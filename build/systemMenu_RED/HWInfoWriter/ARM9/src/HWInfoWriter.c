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

// define data------------------------------------------
#define WRITER_ELEMENT_NUM					7
#define MSG_X								3
#define MSG_Y								19

// extern data------------------------------------------
const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

// function's prototype declaration---------------------
static void ReadTWLSettings( void );
static void ModifyLanguage( u8 region );
static void ReadPrivateKey( void );
static void ReadHWInfoFile( void );
static void WriteHWInfoFile( u8 region );
static BOOL WriteHWNormalInfoFile( void );
static BOOL WriteHWSecureInfoFile( u8 region );
static void DeleteHWInfoFile( void );
static void VerifyHWInfo( void );
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len );
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

static const u32 s_langBitmapList[ LCFG_TWL_REGION_MAX ] = {
	LCFG_TWL_LANG_BITMAP_JAPAN,
	LCFG_TWL_LANG_BITMAP_AMERICA,
	LCFG_TWL_LANG_BITMAP_EUROPE,
	LCFG_TWL_LANG_BITMAP_AUSTRALIA,
	LCFG_TWL_LANG_BITMAP_CHINA,
	LCFG_TWL_LANG_BITMAP_KOREA,
};

static char *strLanguage[] = {
	(char *)"LANG_JAPANESE",
	(char *)"LANG_ENGLISH",
	(char *)"LANG_FRENCH",
	(char *)"LANG_GERMAN",
	(char *)"LANG_ITALIAN",
	(char *)"LANG_SPANISH",
	(char *)"LANG_CHINESE",
	(char *)"LANG_KOREAN",
};

static const char *strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPE",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

static const char *strLauncherGameCode[] = {
	"LNCJ",
	"LNCE",
	"LNCP",
	"LNCO",
	"LNCC",
	"LNCK",
};

//======================================================
// HW情報ライター
//======================================================

// HW情報ライターの初期化
void HWInfoWriterInit( void )
{
	u8 gameCode[ 5 ] = { 0, 0, 0, 0, 0 };
	
	GX_DispOff();
 	GXS_DispOff();
	
	InitBG();
	
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"HW Info Writer");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	ACSign_SetAllocFunc( Alloc, Free );
	ReadTWLSettings();
	ReadPrivateKey();
	ReadHWInfoFile();
//	VerifyHWInfo();
	OS_TPrintf( "region = %d\n", LCFG_THW_GetRegion() );
	PrintfSJISSub( 2 * 8, 16 * 8, TXT_COLOR_BLACK, "Region   = %s", strRegion[ LCFG_THW_GetRegion() ] );
	PrintfSJISSub( 2 * 8, 18 * 8, TXT_COLOR_BLACK, "SerialNo = %s", LCFG_THW_GetSerialNoPtr() );
//	LCFG_THW_GetLauncherGameCode( gameCode );
//	PrintfSJISSub( 2 * 8, 20 * 8, TXT_COLOR_BLACK, "LauncherGameCode = %s", gameCode );
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
	
	// 実行
	if( pad.trg == PAD_BUTTON_A ) {
		if( s_csr == WRITER_ELEMENT_NUM - 1 ) {
			OS_TPrintf( "Delete start.\n" );
			(void)DeleteHWInfoFile();
		}else {
			OS_TPrintf( "Write start.\n" );
			WriteHWInfoFile( (u8)s_csr );
		}
	}
	
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


// TWL設定データのリード
static void ReadTWLSettings( void )
{
	s_isReadTSD = LCFGi_TSD_ReadSettings();
	if( s_isReadTSD ) {
		OS_TPrintf( "TSD read succeeded.\n" );
	}else {
		OS_TPrintf( "TSD read failed.\n" );
	}
}


// 言語コードをリージョン値に合わせて修正する。
static void ModifyLanguage( u8 region )
{
	u32 langBitmap = s_langBitmapList[ region ];
	u8  nowLanguage = LCFG_TSD_GetLanguage();
	
	// TSDが読み込めていないなら、何もせずリターン
	if( !s_isReadTSD ) {
		return;
	}
	
	if( langBitmap & ( 0x0001 << nowLanguage ) ) {
		OS_TPrintf( "Language no change.\n" );
	}else {
		int i;
		for( i = 0; i < LCFG_TWL_LANG_CODE_MAX; i++ ) {
			if( langBitmap & ( 0x0001 << i ) ) {
				break;
			}
		}
		LCFG_TSD_SetLanguage( (LCFGTWLLangCode)i );
		LCFG_TSD_SetFlagCountry( FALSE );				// ※ついでに国コードもクリアしておく。
		LCFG_TSD_SetCountry( LCFG_TWL_COUNTRY_UNDEFINED );
		LCFGi_TSD_WriteSettings();
		OS_TPrintf( "Language Change \"%s\" -> \"%s\"\n",
					strLanguage[ nowLanguage ], strLanguage[ LCFG_TSD_GetLanguage() ] );
	}
}


// 秘密鍵のリード
static void ReadPrivateKey( void )
{
	BOOL result = FALSE;
	u32 keyLength;
	FSFile file;
	OSTick start = OS_GetTick();
	
	FS_InitFile( &file );
	if( !FS_OpenFileEx( &file, "rom:key/private_HWInfo.der", FS_FILEMODE_R ) ) {
		OS_TPrintf( "PrivateKey read failed.\n" );
	}else {
		keyLength = FS_GetFileLength( &file );
		if( keyLength > 0 ) {
			s_pPrivKeyBuffer = Alloc( keyLength );
			if( FS_ReadFile( &file, s_pPrivKeyBuffer, (s32)keyLength ) == keyLength ) {
				OS_TPrintf( "PrivateKey read succeeded.\n" );
				result = TRUE;
			}else {
				OS_TPrintf( "PrivateKey read failed.\n" );
			}
		}
		FS_CloseFile( &file );
	}
	
	if( !result && s_pPrivKeyBuffer ) {
		Free( s_pPrivKeyBuffer );
		s_pPrivKeyBuffer = NULL;
	}
	OS_TPrintf( "PrivKey read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
#ifdef USE_PRODUCT_KEY
	// 製品用秘密鍵が有効なら、署名ありのアクセス
	s_pReadSecureInfoFunc = LCFGi_THW_ReadSecureInfo;
#else
	// そうでないなら、署名なしのアクセス
	s_pReadSecureInfoFunc = LCFGi_THW_ReadSecureInfo_NoCheck;
	PutStringUTF16( 14 * 8, 0 * 8, TXT_COLOR_RED, (const u16 *)L"[No Signature MODE]" );
#endif
}


// HW情報全体のリード
static void ReadHWInfoFile( void )
{
	LCFGReadResult retval;
	OSTick start = OS_GetTick();
	
	retval = LCFGi_THW_ReadNormalInfo();
	if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		OS_TPrintf( "HW Normal Info read succeeded.\n" );
	}else {
		OS_TPrintf( "HW Normal Info read failed.\n" );
	}
	
	OS_TPrintf( "HW Normal Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
	start = OS_GetTick();
	retval = s_pReadSecureInfoFunc();
	if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		OS_TPrintf( "HW Secure Info read succeeded.\n" );
	}else {
		OS_TPrintf( "HW Secure Info read failed.\n" );
	}
	OS_TPrintf( "HW Secure Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
}


// HW情報全体のライト
static void WriteHWInfoFile( u8 region )
{
	static const u16 *pMsgNormalWriting  = (const u16 *)L"Writing Normal File...";
	static const u16 *pMsgSecureWriting  = (const u16 *)L"Writing Secure File...";
	static const u16 *pMsgSucceeded = (const u16 *)L"Succeeded!";
	static const u16 *pMsgFailed = (const u16 *)L"Failed!";
	
	// ノーマルファイルのライト
	(void)PutStringUTF16( MSG_X * 8, MSG_Y * 8, TXT_COLOR_BLACK, pMsgNormalWriting );
	
	if( WriteHWNormalInfoFile() ) {
		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, MSG_Y * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, MSG_Y * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// セキュアファイルのライト
	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLACK, pMsgSecureWriting );
	
	if( WriteHWSecureInfoFile( region ) ) {
		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	ModifyLanguage( region );
	
	// メッセージを一定時間表示して消去
	DispMessage( 0, 0, TXT_COLOR_NULL, NULL );
	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_WHITE,
								MSG_X * 8 , MSG_Y * 8, ( 32 - MSG_X ) * 8, ( MSG_Y + 4 ) * 8 );
	
	PrintfSJISSub( 2 * 8, 16 * 8, TXT_COLOR_WHITE, "Region   = %s", strRegion[ s_region_old ] );
	PrintfSJISSub( 2 * 8, 16 * 8, TXT_COLOR_BLACK, "Region   = %s", strRegion[ LCFG_THW_GetRegion() ] );
	s_region_old = LCFG_THW_GetRegion();
}


// HWノーマルInfoファイルのライト
static BOOL WriteHWNormalInfoFile( void )
{
	BOOL isWrite = TRUE;
	LCFGReadResult result;
	
	result = LCFGi_THW_ReadNormalInfo();
	if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		if( !LCFGi_THW_RecoveryNormalInfo( result ) ) {
			OS_TPrintf( "HW Normal Info Recovery failed.\n" );
			isWrite = FALSE;
		}
	}
	if( isWrite &&
		!LCFGi_THW_WriteNormalInfo() ) {
		OS_TPrintf( "HW Normal Info Write failed.\n" );
	}
	
	return isWrite;
}


// HWセキュアInfoファイルのライト
static BOOL WriteHWSecureInfoFile( u8 region )
{
	BOOL isWrite = TRUE;
	LCFGReadResult result;
	
	// ファイルのリード
	result = s_pReadSecureInfoFunc();
	
	// リードに失敗したらリカバリ
	if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		if( !LCFGi_THW_RecoverySecureInfo( result ) ) {
			OS_TPrintf( "HW Secure Info Recovery failed.\n" );
			isWrite = FALSE;
		}
	}
	
	// リージョンのセット
	LCFG_THW_SetRegion( region );
	
	// 対応言語ビットマップのセット
	LCFG_THW_SetValidLanguageBitmap( s_langBitmapList[ region ] );
	
	// [TODO:]量産工程でないとシリアルNo.は用意できないので、ここではMACアドレスをもとに適当な値をセットする。
	// シリアルNo.のセット
	{
		u8 buffer[ 12 ] = "SERIAL";		// 適当な文字列をMACアドレスと結合してSHA1を取り、仮SerialNoとする。
		u8 serialNo[ SVC_SHA1_DIGEST_SIZE ];
		int i;
		int len = ( LCFG_THW_GetRegion() == LCFG_TWL_REGION_AMERICA ) ?
					LCFG_TWL_HWINFO_SERIALNO_LEN_AMERICA : LCFG_TWL_HWINFO_SERIALNO_LEN_OTHERS;
		OS_GetMacAddress( buffer + 6 );
		SVC_CalcSHA1( serialNo, buffer, sizeof(buffer) );
		for( i = 3; i < SVC_SHA1_DIGEST_SIZE; i++ ) {
			serialNo[ i ] = (u8)( ( serialNo[ i ] % 10 ) + 0x30 );
		}
		MI_CpuCopy8( "SRN", serialNo, 3 );
		MI_CpuClear8( &serialNo[ len ], sizeof(serialNo) - len );
		OS_TPrintf( "serialNo : %s\n", serialNo );
		LCFG_THW_SetSerialNo( serialNo );
	}
	
	// ランチャーゲームコード
	LCFG_THW_SetLauncherGameCode( (const u8 *)strLauncherGameCode[ region ] );
	
	// ライト
	if( isWrite &&
		!LCFGi_THW_WriteSecureInfo( s_pPrivKeyBuffer ) ) {
		isWrite = FALSE;
		OS_TPrintf( "HW Secure Info Write failed.\n" );
	}
	
	return isWrite;
}


// HWInfoファイルの削除
static void DeleteHWInfoFile( void )
{
	static const u16 *pMsgNormalDeleting  = (const u16 *)L"Deleting Normal File...";
	static const u16 *pMsgSecureDeleting  = (const u16 *)L"Deteting Secure File...";
	static const u16 *pMsgSucceeded = (const u16 *)L"Succeeded!";
	static const u16 *pMsgFailed = (const u16 *)L"Failed!";
	
	// ノーマルファイル
	(void)PutStringUTF16( MSG_X * 8, MSG_Y * 8, TXT_COLOR_BLACK, pMsgNormalDeleting );
	if( FS_DeleteFile( (char *)LCFG_TWL_HWINFO_NORMAL_PATH ) ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, MSG_Y * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, MSG_Y * 8, TXT_COLOR_RED, pMsgFailed );
	}
	
	// セキュアファイル
	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLACK, pMsgSecureDeleting );
	if( FS_DeleteFile( (char *)LCFG_TWL_HWINFO_SECURE_PATH ) ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_RED, pMsgFailed );
	}
	DispMessage( 0, 0, TXT_COLOR_NULL, NULL );
	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_WHITE,
								MSG_X * 8 , MSG_Y * 8, ( 32 - MSG_X ) * 8, ( MSG_Y + 4 ) * 8 );
}


// HWInfoファイルのベリファイ
static void VerifyHWInfo( void )
{
	if( VerifyData(	(const u8 *)LCFG_THW_GetNormalInfo(), (const u8 *)LCFG_THW_GetDefaultNormalInfo(), sizeof(LCFGTWLHWNormalInfo) ) ) {
		OS_TPrintf( "HW normal Info verify succeeded.\n" );
	}else {
		OS_TPrintf( "HW normal Info verify failed.\n" );
	}
	if( VerifyData(	(const u8 *)LCFG_THW_GetSecureInfo(), (const u8 *)LCFG_THW_GetDefaultSecureInfo(), sizeof(LCFGTWLHWSecureInfo) ) ) {
		OS_TPrintf( "HW secure Info verify succeeded.\n" );
	}else {
		OS_TPrintf( "HW secure Info verify failed.\n" );
	}

}


// メモリ上のデータベリファイ
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len )
{
	while( len-- ) {
		if( *pTgt++ != *pOrg++ ) {
			return FALSE;
		}
	}
	return TRUE;
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
