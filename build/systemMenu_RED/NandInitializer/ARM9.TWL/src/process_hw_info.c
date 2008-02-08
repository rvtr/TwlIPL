/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_hw_info.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <twl/lcfg.h>
#include <nitro/card.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_hw_info.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

#include <sysmenu/acsign.h>
//#include <sysmenu/settings/common/TWLHWInfo.h>
//#include <sysmenu/settings/common/TWLSettings.h>

//
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
//
/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT    8
#define DOT_OF_MENU_SPACE    16
#define CHAR_OF_MENU_SPACE    2
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

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

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

static u8 *s_pPrivKeyBuffer = NULL;
static LCFGReadResult (*s_pReadSecureInfoFunc)( void );
static BOOL s_isReadTSD;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

void HWInfoWriterInit( void );
static void ReadTWLSettings( void );
static void ModifyLanguage( u8 region );
static void ReadPrivateKey( void );
static void ReadHWInfoFile( void );
static void VerifyHWInfo( void );
static BOOL WriteHWInfoFile( u8 region );
static BOOL WriteHWNormalInfoFile( void );
static BOOL WriteHWSecureInfoFile( u8 region );
static BOOL DeleteHWInfoFile( void );

const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         HWInfo プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* HWInfoProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write Hardware Information ");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   REGION JAPAN     l    l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   REGION AMERICA   l    l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l   REGION EUROPE    l    l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l   REGION AUSTRALIA l    l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "l   REGION CHINA     l    l");
	kamiFontPrintf(3, 16, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 17, FONT_COLOR_BLACK, "l   REGION KOREA     l    l");
	kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 19, FONT_COLOR_BLACK, "l   DELETE           l    l");
	kamiFontPrintf(3, 20, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 21, FONT_COLOR_BLACK, "l   RETURN           l    l");
	kamiFontPrintf(3, 22, FONT_COLOR_BLACK, "+--------------------+----+");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_PURPLE, BG_COLOR_PURPLE );
	kamiFontFillChar( 1, BG_COLOR_PURPLE, BG_COLOR_PURPLE );
	kamiFontFillChar( 2, BG_COLOR_PURPLE, BG_COLOR_TRANS );

	// カーソル除外
	SetCursorPos((u16)200, (u16)200);

	// 前準備
	HWInfoWriterInit();

	FADE_IN_RETURN( HWInfoProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         HWInfo プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* HWInfoProcess1(void)
{
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return HWInfoProcess2;
	}

	// 選択メニューの変更
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = NUM_OF_MENU_SELECT -1;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo >= NUM_OF_MENU_SELECT) sMenuSelectNo = 0;
	}

	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return HWInfoProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return HWInfoProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         HWInfo プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* HWInfoProcess2(void)
{
	int i;
	BOOL result;

	switch( sMenuSelectNo )
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		OS_TPrintf( "Write Start.\n" );
		result = WriteHWInfoFile( (u8)sMenuSelectNo );
		break;
	case 6:
		OS_TPrintf( "Delete start.\n" );
		result = DeleteHWInfoFile();
		break;
	case 7:
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	// 全結果をクリア
	for (i=0;i<NUM_OF_MENU_SELECT;i++)
	{
		kamiFontPrintf(26,  (s16)(7+i*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "  ");
	}
	// 今回の結果を表示
	if ( result == TRUE )
	{
		kamiFontPrintf(26,  (s16)(7+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf(26,  (s16)(7+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG");
	}

	// Auto用
	if (gAutoFlag)
	{
		if (result) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2 ); }
	}

	return HWInfoProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         HW情報ライターの初期化

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void HWInfoWriterInit( void )
{
//	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"HW Info Writer");
//	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	ACSign_SetAllocFunc( OS_AllocFromMain, OS_FreeToMain );
	ReadTWLSettings();
	ReadPrivateKey();
	ReadHWInfoFile();
//	VerifyHWInfo();
	OS_Printf("region = %d\n", LCFG_THW_GetRegion() );
	
//	s_csr = 0;
//	DrawMenu( s_csr, &s_writerParam );
	
//	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
//	GX_DispOn();
//	GXS_DispOn();
}

/*---------------------------------------------------------------------------*
  Name:         TWL設定データのリード

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ReadTWLSettings( void )
{
	s_isReadTSD = LCFGi_TSD_ReadSettings();
	if( s_isReadTSD ) {
		OS_TPrintf( "TSD read succeeded.\n" );
	}else {
		OS_TPrintf( "TSD read failed.\n" );
	}
}

/*---------------------------------------------------------------------------*
  Name:         言語コードをリージョン値に合わせて修正する。

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*
  Name:         秘密鍵のリード

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ReadPrivateKey( void )
{
	BOOL result = FALSE;
	u32 keyLength;
	FSFile file;
	OSTick start = OS_GetTick();
	
	FS_InitFile( &file );
	if( !FS_OpenFileEx( &file, "rom:key/private_HWInfo.der", FS_FILEMODE_R ) ) {
		kamiFontPrintfConsoleEx(CONSOLE_RED, "PrivateKey read failed.\n" );
	}else {
		keyLength = FS_GetFileLength( &file );
		if( keyLength > 0 ) {
			s_pPrivKeyBuffer = OS_Alloc( keyLength );
			if( FS_ReadFile( &file, s_pPrivKeyBuffer, (s32)keyLength ) == keyLength ) {
				OS_TPrintf( "PrivateKey read succeeded.\n" );
				result = TRUE;
			}else {
				kamiFontPrintfConsoleEx(CONSOLE_RED, "PrivateKey read failed.\n" );
			}
		}
		FS_CloseFile( &file );
	}
	
	if( !result && s_pPrivKeyBuffer ) {
		OS_Free( s_pPrivKeyBuffer );
		s_pPrivKeyBuffer = NULL;
	}
	OS_TPrintf( "PrivKey read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

#ifdef USE_PRODUCT_KEY
	// 製品用秘密鍵が有効なら、署名ありのアクセス
	s_pReadSecureInfoFunc = LCFGi_THW_ReadSecureInfo;
#else
	// そうでないなら、署名なしのアクセス
	s_pReadSecureInfoFunc = LCFGi_THW_ReadSecureInfo_NoCheck;
//	PutStringUTF16( 14 * 8, 0 * 8, TXT_COLOR_RED, (const u16 *)L"[No Signature MODE]" );
#endif
}

/*---------------------------------------------------------------------------*
  Name:         HW情報全体のリード

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ReadHWInfoFile( void )
{
	LCFGReadResult retval;
	OSTick start = OS_GetTick();
	
	retval = LCFGi_THW_ReadNormalInfo();
	if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		OS_Printf("HW Normal Info read succeeded.\n" );
	}else {
		kamiFontPrintfConsoleEx(0, "HW Normal Info read failed.\n" );
	}
	
	OS_TPrintf( "HW Normal Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
	start = OS_GetTick();
	retval = s_pReadSecureInfoFunc();
	if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		OS_Printf("HW Secure Info read succeeded.\n" );
	}else {
		kamiFontPrintfConsoleEx(0, "HW Secure Info read failed.\n" );
	}
	OS_TPrintf( "HW Secure Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
}

/*---------------------------------------------------------------------------*
  Name:         HW情報全体のライト

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL WriteHWInfoFile( u8 region )
{
	static const u16 *pMsgNormalWriting  = (const u16 *)L"Writing Normal File...";
	static const u16 *pMsgSecureWriting  = (const u16 *)L"Writing Secure File...";
	static const u16 *pMsgSucceeded = (const u16 *)L"Succeeded!";
	static const u16 *pMsgFailed = (const u16 *)L"Failed!";
	BOOL result = TRUE;

	// ノーマルファイルのライト
//	(void)PutStringUTF16( MSG_X * 8, MSG_Y * 8, TXT_COLOR_BLACK, pMsgNormalWriting );
	
	if( WriteHWNormalInfoFile() ) {
//		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, MSG_Y * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
//		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, MSG_Y * 8, TXT_COLOR_RED, pMsgFailed );
		result = FALSE;
	}
	
	// セキュアファイルのライト
//	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLACK, pMsgSecureWriting );
	
	if( WriteHWSecureInfoFile( region ) ) {
//		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
//		(void)PutStringUTF16( ( MSG_X + 18 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_RED, pMsgFailed );
		result = FALSE;
	}
	
	ModifyLanguage( region );
	
	// メッセージを一定時間表示して消去
//	DispMessage( 0, 0, TXT_COLOR_NULL, NULL );
//	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_WHITE,
//								MSG_X * 8 , MSG_Y * 8, ( 32 - MSG_X ) * 8, ( MSG_Y + 4 ) * 8 );

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         HWノーマルInfoファイルのライト

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL WriteHWNormalInfoFile( void )
{
	BOOL isWrite = TRUE;
	LCFGReadResult result;
	
	result = LCFGi_THW_ReadNormalInfo();
	if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		if( !LCFGi_THW_RecoveryNormalInfo( result ) ) {
			kamiFontPrintfConsoleEx(CONSOLE_RED, "HW Normal Info Recovery failed.\n" );
			isWrite = FALSE;
		}
	}
	if( isWrite &&
		!LCFGi_THW_WriteNormalInfo() ) {
		kamiFontPrintfConsoleEx(CONSOLE_RED, "HW Normal Info Write failed.\n" );
	}
	
	return isWrite;
}

/*---------------------------------------------------------------------------*
  Name:         HWセキュアInfoファイルのライト

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL WriteHWSecureInfoFile( u8 region )
{
	BOOL isWrite = TRUE;
	LCFGReadResult result;
	
	// ファイルのリード
	result = s_pReadSecureInfoFunc();
	
	// リードに失敗したらリカバリ
	if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		if( !LCFGi_THW_RecoverySecureInfo( result ) ) {
			kamiFontPrintfConsoleEx(CONSOLE_RED, "HW Secure Info Recovery failed.\n" );
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
	
	// ランチャーTitleID_Loのセット
	{
		int i;
		u8 titleID_Lo[4];
		for( i = 0; i < 4; i++ ) titleID_Lo[ i ] = (u8)strLauncherGameCode[ region ][ 4 - i - 1 ];
		LCFG_THW_SetLauncherTitleID_Lo( (const u8 *)titleID_Lo );
	}

	// ライト
	if( isWrite &&
		!LCFGi_THW_WriteSecureInfo( s_pPrivKeyBuffer ) ) {
		isWrite = FALSE;
		kamiFontPrintfConsoleEx(CONSOLE_RED, "HW Secure Info Write failed.\n" );
	}
	
	return isWrite;
}

/*---------------------------------------------------------------------------*
  Name:         HWInfoファイルの削除

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL DeleteHWInfoFile( void )
{
	static const u16 *pMsgNormalDeleting  = (const u16 *)L"Deleting Normal File...";
	static const u16 *pMsgSecureDeleting  = (const u16 *)L"Deteting Secure File...";
	static const u16 *pMsgSucceeded = (const u16 *)L"Succeeded!";
	static const u16 *pMsgFailed = (const u16 *)L"Failed!";
	BOOL result = TRUE;

	// ノーマルファイル
//	(void)PutStringUTF16( MSG_X * 8, MSG_Y * 8, TXT_COLOR_BLACK, pMsgNormalDeleting );
	if( FS_DeleteFile( (char *)LCFG_TWL_HWINFO_NORMAL_PATH ) ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
//		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, MSG_Y * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
//		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, MSG_Y * 8, TXT_COLOR_RED, pMsgFailed );
		result = FALSE;
	}
	
	// セキュアファイル
//	(void)PutStringUTF16( MSG_X * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLACK, pMsgSecureDeleting );
	if( FS_DeleteFile( (char *)LCFG_TWL_HWINFO_SECURE_PATH ) ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
//		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_BLUE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
//		(void)PutStringUTF16( ( MSG_X + 19 ) * 8, ( MSG_Y + 2 ) * 8, TXT_COLOR_RED, pMsgFailed );
		result = FALSE;
	}
//	DispMessage( 0, 0, TXT_COLOR_NULL, NULL );
//	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_WHITE,
//								MSG_X * 8 , MSG_Y * 8, ( 32 - MSG_X ) * 8, ( MSG_Y + 4 ) * 8 );

	return result;
}
