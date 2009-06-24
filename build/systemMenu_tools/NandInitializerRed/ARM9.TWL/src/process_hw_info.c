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
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_hw_info.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "hwi.h"

//
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
//
/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	MENU_REGION_JAPAN = 0,
	MENU_REGION_AMERICA,
	MENU_REGION_EUROPE,
	MENU_REGION_AUSTRALIA,
	MENU_REGION_CHINA,
	MENU_REGION_KOREA,
	MENU_RETURN,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CHAR_OF_MENU_SPACE    2
#define MENU_TOP_LINE         5
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      40

#define NANDINITIALIZER_SETTING_FILE_PATH_IN_SD  "sdmc:/nandinitializer.ini"

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8   sMenuSelectNo;
static BOOL sWirelessForceOff;
static BOOL sLogoDemoSkipForce;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL WriteHWNormalInfoFile( void );
static BOOL WriteHWSecureInfoFile( u8 region );
//static BOOL DeleteHWInfoFile( void );

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
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  5, FONT_COLOR_BLACK, "l   REGION JAPAN     l    l");
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   REGION AMERICA   l    l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   REGION EUROPE    l    l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l   REGION AUSTRALIA l    l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l   REGION CHINA     l    l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "l   REGION KOREA     l    l");
	kamiFontPrintf(3, 16, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 17, FONT_COLOR_BLACK, "l   RETURN           l    l");
	kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "+--------------------+----+");

	// 現在のリージョンに"now"と表示
	kamiFontPrintf(26, (s16)(MENU_TOP_LINE+OS_GetRegion()*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "now");

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
#ifndef NAND_INITIALIZER_LIMITED_MODE
	// オート実行用
	if (gAutoFlag)
	{
		return HWInfoProcess2;
	}
#endif

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

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// オート実行用
	if (gAutoFlag)
	{
		// SDカードのnandinitializer.iniより設定を取得
		if (!GetNandInitializerSetting((u8 *)&sMenuSelectNo, (u8 *)&sWirelessForceOff, (u8 *)&sLogoDemoSkipForce))
		{
			// 設定の取得に失敗した場合はデフォルト設定(REGION_JAPAN/WIRELESS_ENABLE)
			sMenuSelectNo = 0;
			sWirelessForceOff = FALSE;
			sLogoDemoSkipForce = FALSE;
		}
	}
	else
#endif
	{
		sWirelessForceOff = LCFG_THW_IsForceDisableWireless();
	}

	switch( sMenuSelectNo )
	{
	case MENU_REGION_JAPAN:
	case MENU_REGION_AMERICA:
	case MENU_REGION_EUROPE:
	case MENU_REGION_AUSTRALIA:
	case MENU_REGION_CHINA:
	case MENU_REGION_KOREA:

		result = WriteHWInfoFile( (u8)sMenuSelectNo, sWirelessForceOff, sLogoDemoSkipForce );

		// 全リージョンの結果をクリア
		for (i=0;i<NUM_OF_MENU_SELECT;i++)
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+i*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "   ");
		}
		// 今回の結果を表示
		if ( result == TRUE )
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK ");
		}
		else
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG ");
		}
		break;

	case MENU_RETURN:
		FADE_OUT_RETURN( TopmenuProcess0 );
		break;
	}


#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto用
	if (gAutoFlag)
	{
		if (result) 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_HARDWARE_INFO] = AUTO_PROCESS_RESULT_SUCCESS; 
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{
			gAutoProcessResult[AUTO_PROCESS_MENU_HARDWARE_INFO] = AUTO_PROCESS_RESULT_FAILURE;  
			FADE_OUT_RETURN( AutoProcess2 ); 
		}
	}
#endif

	return HWInfoProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         HW情報全体のライト

  Description:  

  Arguments:    region : 

  Returns:      None.
 *---------------------------------------------------------------------------*/

BOOL WriteHWInfoFile( u8 region, BOOL wirelessForceOff, BOOL logoDemoSkipForce )
{
	static const char *pMsgSecureWriting  	= "Writing Secure File...";
	static const char *pMsgNormalWriting  	= "Writing Normal File...";
	static const char *pMsgSignWriting  	= "Writing Sign   File...";
	static const char *pMsgSucceeded 		= "Success!\n";
	static const char *pMsgFailed 			= "Failed!\n";
	u32 installedSoftBoxCount = 0;
	BOOL result = TRUE;

	// HWID署名ファイルのライト
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSignWriting );
	
	if( HWI_WriteHWIDSignFile() ) {
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}

	// セキュアファイルのライト
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSecureWriting );
	
	if( HWI_WriteHWSecureInfoFile( region, NULL, wirelessForceOff, logoDemoSkipForce ) ) {	
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// ノーマルファイルのライト(移行可能なユニークIDのためにセキュアファイルの後で書き込む）
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgNormalWriting );
	
	if( HWI_WriteHWNormalInfoFile() ) {
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// CFGデータの修正
	if (!HWI_ModifyLanguage( region ))
	{
		kamiFontPrintfConsoleEx(CONSOLE_RED, "Fail! Write TWLSettings\n" );	
		result = FALSE;
	}
	
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         HWInfoファイルの削除

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL DeleteHWInfoFile( void )
{
	static const char *pMsgNormalDeleting  	= "Deleting Normal File...";
	static const char *pMsgSecureDeleting  	= "Deteting Secure File...";
	static const char *pMsgSucceeded 		= "Success!\n";
	static const char *pMsgFailed 			= "Failed!\n";
	BOOL result = TRUE;

	// ノーマルファイル
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgNormalDeleting );
	if( HWI_DeleteHWNormalInfoFile() ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// セキュアファイル
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSecureDeleting );
	if( HWI_DeleteHWSecureInfoFile() ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         GetNandInitializerSetting

  Description:  SDカードのnandinitializer.iniの設定を確認します

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL GetNandInitializerSetting(u8* region, u8* wireless, u8* logodemoskip)
{
    FSFile  file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	char* pStr;
	u8    temp_region;
	u8    temp_wireless;
	u8    temp_logodemoskip;
	u32 file_size;
	u32 alloc_size;

	// ROMファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, NANDINITIALIZER_SETTING_FILE_PATH_IN_SD);
	if (!open_is_ok)
	{
    	OS_Printf("%s is not exist.\n", NANDINITIALIZER_SETTING_FILE_PATH_IN_SD);
		return FALSE;
	}

	// ROMファイルリード
	file_size  = FS_GetFileLength(&file) ;
	alloc_size = ROUND_UP(file_size, 32) ;
	pTempBuf = OS_Alloc( alloc_size );
	SDK_NULL_ASSERT(pTempBuf);
	DC_InvalidateRange(pTempBuf, alloc_size);
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	if (!read_is_ok)
	{
	    OS_Printf("%s could not be read.\n", NANDINITIALIZER_SETTING_FILE_PATH_IN_SD);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROMファイルクローズ
	FS_CloseFile(&file);
	
	// 強制ロゴデモスキップ設定を読み取る
	pStr = STD_SearchString( pTempBuf, "LOGO_DEMO_SKIP_FORCE:");
	if (pStr != NULL)
	{

		pStr += STD_GetStringLength("LOGO_DEMO_SKIP_FORCE:");
		temp_logodemoskip = (u8)(*pStr - '0');

		if ( temp_logodemoskip == 1 )
		{ 
			*logodemoskip = temp_logodemoskip; 
		}
		else
		{
			*logodemoskip = 0; 
		}
	}else
	{
		*logodemoskip = 0;
	}

	// REGION: を読み取る
	pStr = STD_SearchString( pTempBuf, "REGION:");
	if (pStr == NULL)
	{
		OS_Free(pTempBuf);
		return FALSE;
	}

	pStr += STD_GetStringLength("REGION:");
	temp_region = (u8)(*pStr - '0');

	if (OS_TWL_REGION_JAPAN <= temp_region && temp_region < OS_TWL_REGION_MAX)
	{
		*region = temp_region;
	}
	else
	{
		OS_Free(pTempBuf);
		return FALSE;		
	}

	// 強制ワイヤレスOFF設定を読み取る
	pStr = STD_SearchString( pTempBuf, "WIRELESS_FORCE_OFF:");
	if (pStr == NULL)
	{
		OS_Free(pTempBuf);
		return FALSE;
	}

	pStr += STD_GetStringLength("WIRELESS_FORCE_OFF:");
	temp_wireless = (u8)(*pStr - '0');

	if (0 <= temp_wireless && temp_wireless <= 1)
	{ 
		*wireless = temp_wireless; 
	}
	else
	{
		OS_Free(pTempBuf);
		return FALSE;
	}

	OS_Free(pTempBuf);
	return TRUE;
}
