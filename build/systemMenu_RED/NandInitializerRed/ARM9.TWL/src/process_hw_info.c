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

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT    7
#define DOT_OF_MENU_SPACE    16
#define CHAR_OF_MENU_SPACE    2
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

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

static BOOL WriteHWNormalInfoFile( void );
static BOOL WriteHWSecureInfoFile( u8 region );
//static BOOL DeleteHWInfoFile( void );

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
//	kamiFontPrintf(3, 19, FONT_COLOR_BLACK, "l   DELETE           l    l");
//	kamiFontPrintf(3, 20, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 19, FONT_COLOR_BLACK, "l   RETURN           l    l");
	kamiFontPrintf(3, 20, FONT_COLOR_BLACK, "+--------------------+----+");

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
		sMenuSelectNo = 0;
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
//	case 6:
//		OS_TPrintf( "Delete start.\n" );
//		result = DeleteHWInfoFile();
//		break;
	case 6:
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

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto用
	if (gAutoFlag)
	{
		if (result) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2 ); }
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

BOOL WriteHWInfoFile( u8 region )
{
	static const char *pMsgNormalWriting  	= "Writing Normal File...";
	static const char *pMsgSecureWriting  	= "Writing Secure File...";
	static const char *pMsgSignWriting  	= "Writing Sign   File...";
	static const char *pMsgSucceeded 		= "Success!\n";
	static const char *pMsgFailed 			= "Failed!\n";
	u32 installedSoftBoxCount = 0;
	BOOL result = TRUE;

	// ノーマルファイルのライト
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgNormalWriting );
	
	if( HWI_WriteHWNormalInfoFile() ) {
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// セキュアファイルのライト
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSecureWriting );
	
	if( HWI_WriteHWSecureInfoFile( region, NULL, FALSE ) ) {	// とりあえず無線は有効で。
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// HWID署名ファイルのライト
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSignWriting );
	
	if( HWI_WriteHWIDSignFile() ) {
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

	// InstalledSoftBoxCount, FreeSoftBoxCount の値を現在のNANDの状態に合わせて更新します。
	UpdateNandBoxCount();
	
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
  Name:         UpdateNandBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount の値を
				現在のNANDの状態に合わせて更新します。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void UpdateNandBoxCount( void )
{
	u32 installedSoftBoxCount;
	u32 freeSoftBoxCount;

	// InstalledSoftBoxCount, FreeSoftBoxCount を数えなおす
	installedSoftBoxCount = NAMUT_SearchInstalledSoftBoxCount();
	freeSoftBoxCount = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - installedSoftBoxCount;

//	OS_Printf("installedSoftBoxCount = %d\n", installedSoftBoxCount);
//	OS_Printf("freeSoftBoxCount      = %d\n", freeSoftBoxCount);

	// LCFGライブラリの静的変数に対する更新
    LCFG_TSD_SetInstalledSoftBoxCount( (u8)installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( (u8)freeSoftBoxCount );

	// LCFGライブラリの静的変数の値をNANDに反映
    {
        u8 *pBuffer = OS_Alloc( LCFG_WRITE_TEMP );
        if( pBuffer ) {
            (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
            OS_Free( pBuffer );
        }
    }
}

