/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Chat.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
#include "misc.h"
#include "HWInfoWriter.h"
#include "hwi.h"

// define data------------------------------------------
#define WRITER_ELEMENT_NUM					7
#define MSG_X								3
#define MSG_Y								18

// extern data------------------------------------------

// function's prototype declaration---------------------
static void WriteHWInfoFile( u8 region, BOOL isDisableWireless );
static void DeleteHWInfoFile( void );
static void DispMessage( int x, int y, u16 color, const u16 *pMsg );

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------
static u16 s_csr;
static u8 *s_pPrivKeyBuffer = NULL;
static LCFGReadResult (*s_pReadSecureInfoFunc)( void );
static BOOL s_isReadTSD;
static u8 s_region_old;
static BOOL s_isDisableWireless;

// const data  -----------------------------------------
static const u16 *const s_pStrWriter[ WRITER_ELEMENT_NUM ] = {
	(const u16 *)L"Write HW Info REGION=JAPAN",
	(const u16 *)L"Write HW Info REGION=AMERICA",
	(const u16 *)L"Write HW Info REGION=EUROPE",
	(const u16 *)L"Write HW Info REGION=AUSTRALIA",
	(const u16 *)L"Write HW Info REGION=CHINA",
	(const u16 *)L"Write HW Info REGION=KOREA",
	(const u16 *)L"Delete HW Info",
};

static MenuPos s_writerPos[] = {
	{ TRUE,  3 * 8,   4 * 8 },
	{ TRUE,  3 * 8,   6 * 8 },
	{ TRUE,  3 * 8,   8 * 8 },
	{ TRUE,  3 * 8,  10 * 8 },
	{ TRUE,  3 * 8,  12 * 8 },
	{ TRUE,  3 * 8,  14 * 8 },
	{ TRUE,  3 * 8,  16 * 8 },
};


static const MenuParam s_writerParam = {
	WRITER_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_writerPos[ 0 ],
	(const u16 **)&s_pStrWriter,
};

static const char *strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPE",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};


//======================================================
// HW情報ライター
//======================================================
const char *pWireless[] = {
	"Enable Wireless",
	"Force disable Wireless",
};

// HW情報ライターの初期化
void HWInfoWriterInit( void )
{
	
	GX_DispOff();
 	GXS_DispOff();
	
	InitBG();
	
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"HW Info Writer");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );

	{
		char *pMode = NULL;
		switch ( HWI_Init( Alloc, Free ) ) {
		case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
			pMode = "No";
			break;
		case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
			pMode = "Pro";
			break;
		case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
			pMode = "Dev";
			break;
		}
		PrintfSJIS( 14 * 8, 0 * 8, TXT_COLOR_RED, "[%s Signature MODE]", pMode );
	}
	
	// 無線強制ON/OFF情報の表示
	s_isDisableWireless = LCFG_THW_IsForceDisableWireless();
	PrintfSJIS( 3 * 8, 2 * 8, TXT_COLOR_BLACK, pWireless[ s_isDisableWireless ] );
	
	OS_TPrintf( "region = %d\n", LCFG_THW_GetRegion() );
	PrintfSJISSub( 2 * 8, 16 * 8, TXT_COLOR_BLACK, "Region   = %s", strRegion[ LCFG_THW_GetRegion() ] );
	PrintfSJISSub( 2 * 8, 18 * 8, TXT_COLOR_BLACK, "SerialNo = %s", LCFG_THW_GetSerialNoPtr() );
	if ( 1 )
	{
		int i;
		u8 titleID_Lo[ 4 ];
		u8 gameCode[ 5 ] = { 0, 0, 0, 0, 0 };
		LCFG_THW_GetLauncherTitleID_Lo( titleID_Lo );
		for( i = 0; i < 4; i++ ) gameCode[ i ] = titleID_Lo[ 4 - i - 1 ];
		PrintfSJISSub( 2 * 8, 20 * 8, TXT_COLOR_BLACK, "LauncherTitleID_Lo = %s", gameCode );
	}
	s_region_old = LCFG_THW_GetRegion();
	s_csr = 0;
	DrawMenu( s_csr, &s_writerParam );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// HW情報ライターのメインループ
void HWInfoWriterMain( void )
{
	// カーソル移動
	if( pad.trg & PAD_KEY_DOWN ){
		if( ++s_csr == WRITER_ELEMENT_NUM ) {
			s_csr = 0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x8000 ) {
			s_csr = WRITER_ELEMENT_NUM - 1;
		}
	}
	DrawMenu( s_csr, &s_writerParam );

	if( pad.trg & PAD_BUTTON_START ) {
		PrintfSJIS( 3 * 8, 2 * 8, TXT_COLOR_WHITE, pWireless[ s_isDisableWireless ] );
		s_isDisableWireless ^= 0x01;
		PrintfSJIS( 3 * 8, 2 * 8, TXT_COLOR_BLACK, pWireless[ s_isDisableWireless ] );
	}

	// 実行
	if( pad.trg == PAD_BUTTON_A ) {
		if( s_csr == WRITER_ELEMENT_NUM - 1 ) {
			OS_TPrintf( "Delete start.\n" );
			(void)DeleteHWInfoFile();
		}else {
			OS_TPrintf( "Write start.\n" );
			WriteHWInfoFile( (u8)s_csr, s_isDisableWireless );
		}
	}
	
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


// HW情報全体のライト
static void WriteHWInfoFile( u8 region, BOOL isDisableWireless )
{
	static const u16 *pMsgNormalWriting  = (const u16 *)L"Writing Normal File...";
	static const u16 *pMsgSecureWriting  = (const u16 *)L"Writing Secure File...";
	static const u16 *pMsgHWIDSignWriting = (const u16 *)L"Writing HWID Sign File...";
	static const u16 *pMsgSucceeded = (const u16 *)L"Succeeded!";
	static const u16 *pMsgFailed = (const u16 *)L"Failed!";
	
	// -------------------------------------
	// ノーマルファイルのライト
	// -------------------------------------
	(void)PutStringUTF16( MSG_X * 8, MSG_Y * 8, TXT_COLOR_BLACK, pMsgNormalWriting );
	
	if( HWI_WriteHWNormalInfoFile() ) {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, MSG_Y * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, MSG_Y * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// -------------------------------------
	// セキュアファイルのライト
	// -------------------------------------
	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLACK, pMsgSecureWriting );
	
	if( HWI_WriteHWSecureInfoFile( region, NULL, isDisableWireless ) ) {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// -------------------------------------
	// HWID署名ファイルのライト
	// -------------------------------------
	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 4 ) * 8, TXT_COLOR_BLACK, pMsgHWIDSignWriting );
	
	if( HWI_WriteHWIDSignFile() ) {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 4 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 4 ) * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// リージョンの更新を言語コードに反映させる。（必ずセキュアファイルのライト後に実行）
	HWI_ModifyLanguage( region );
	
	// メッセージを一定時間表示して消去
	DispMessage( 0, 0, TXT_COLOR_NULL, NULL );
	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_WHITE,
								MSG_X * 8 , MSG_Y * 8, ( 32 - MSG_X ) * 8, ( MSG_Y + 4 ) * 8 );
	
	PrintfSJISSub( 2 * 8, 16 * 8, TXT_COLOR_WHITE, "Region   = %s", strRegion[ s_region_old ] );
	PrintfSJISSub( 2 * 8, 16 * 8, TXT_COLOR_BLACK, "Region   = %s", strRegion[ LCFG_THW_GetRegion() ] );
	s_region_old = LCFG_THW_GetRegion();
}

// HWInfoファイルの削除
static void DeleteHWInfoFile( void )
{
	static const u16 *pMsgNormalDeleting  = (const u16 *)L"Deleting Normal File...";
	static const u16 *pMsgSecureDeleting  = (const u16 *)L"Deteting Secure File...";
	static const u16 *pMsgHWIDSignDeleting = (const u16 *)L"Deteting HWID Sign File.";
	static const u16 *pMsgSucceeded = (const u16 *)L"Succeeded!";
	static const u16 *pMsgFailed = (const u16 *)L"Failed!";
	
	// -------------------------------------
	// ノーマルファイルの削除
	// -------------------------------------
	(void)PutStringUTF16( MSG_X * 8, MSG_Y * 8, TXT_COLOR_BLACK, pMsgNormalDeleting );
	if( HWI_DeleteHWNormalInfoFile() ) {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, MSG_Y * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, MSG_Y * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// -------------------------------------
	// セキュアファイルの削除
	// -------------------------------------
	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLACK, pMsgSecureDeleting );
	if( HWI_DeleteHWSecureInfoFile() ) {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// -------------------------------------
	// HWID署名ファイルの削除
	// -------------------------------------
	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 4 ) * 8, TXT_COLOR_BLACK, pMsgHWIDSignDeleting );
	if( HWI_DeleteHWIDSignFile() ) {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 4 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 20 ) * 8, ( MSG_Y + 4 ) * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	DispMessage( 0, 0, TXT_COLOR_NULL, NULL );
	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_WHITE,
								MSG_X * 8 , MSG_Y * 8, ( 32 - MSG_X ) * 8, ( MSG_Y + 4 ) * 8 );
}

// メッセージ表示
static void DispMessage( int x, int y, u16 color, const u16 *pMsg )
{
	OSTick start = OS_GetTick();
	// メッセージ表示
	if( pMsg ) {
		(void)PutStringUTF16( x, y, color, pMsg );
	}
	// ウェイト
	while( OS_TicksToSeconds( OS_GetTick() - start ) < 2 ) {
		OS_SpinWait( 0x1000 );
	}
	// メッセージ消去
	if( pMsg ) {
		(void)PutStringUTF16( x, y, TXT_COLOR_WHITE, pMsg );
	}
}
