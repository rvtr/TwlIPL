/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_format.c

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
#include <nitro/card.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_format.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

#include <sysmenu/namut.h>

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	MENU_EASY_FORMAT=0,
	MENU_CHECK_DISK,
#ifndef NAND_FORMATTER_MODE
	MENU_NORMAL_FORMAT,
	MENU_FILL_FORMAT,
#endif
	MENU_RETURN,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

// NANDの簡易フォーマットを実行する際に
// ツリー情報を出力する場合は定義します
//#define DUMP_NAND_TREE

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56
#define CHAR_OF_MENU_SPACE    2

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;
static u8 sLock;
static u8 sFormatResult;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void FormatCallback(KAMIResult result, void* arg);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Format プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Format Nand Flash");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l  FORMAT           l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l  CHECK DISK       l     l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+-------------------+-----+");
#ifndef NAND_FORMATTER_MODE
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l  FORMAT <Normal>  l     l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l  FORMAT <Fill FF> l     l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "l  RETURN           l     l");
	kamiFontPrintf(3, 16, FONT_COLOR_BLACK, "+-------------------+-----+");
#else
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l  RETURN           l     l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+-------------------+-----+");
#endif

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_BLUE, BG_COLOR_BLUE );
	kamiFontFillChar( 1, BG_COLOR_BLUE, BG_COLOR_BLUE );
	kamiFontFillChar( 2, BG_COLOR_BLUE, BG_COLOR_TRANS );

	// カーソル除外
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( FormatProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         Format プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess1(void)
{
#ifndef NAND_FORMATTER_MODE
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = MENU_NORMAL_FORMAT;
		return FormatProcess2;
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
		return FormatProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return FormatProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Format プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess2(void)
{
	if (sLock == FALSE)
	{
		s16 y_pos = (s16)(7 + sMenuSelectNo * CHAR_OF_MENU_SPACE);

		switch( sMenuSelectNo )
		{
		case MENU_EASY_FORMAT:	// 簡易フォーマット
#ifdef DUMP_NAND_TREE
			NAMUT_DrawNandTree();
#endif
			kamiFontPrintf(24, y_pos, FONT_COLOR_BLACK, " WAIT");
			kamiFontLoadScreenData();

			if (NAMUT_Format())
			{
				kamiFontPrintf(24, y_pos, FONT_COLOR_GREEN, " OK  ");
			}
			else
			{
				kamiFontPrintf(24, y_pos, FONT_COLOR_RED, " NG  ");
			}
#ifdef DUMP_NAND_TREE
			OS_Printf("\n");
			NAMUT_DrawNandTree();
#endif
			return FormatProcess1;
		case MENU_CHECK_DISK: // チェックディスク
			{
				FATFSDiskInfo info;
				BOOL result = FALSE;

				kamiFontPrintf(24,  y_pos, FONT_COLOR_BLACK, " WAIT");
				kamiFontLoadScreenData();

//				FATFS_UnmountDrive("F:");
//				FATFS_UnmountDrive("G:");
	            // 指定のNANDパーティション0をFドライブにマウント
//	            if (FATFS_MountDrive("F", FATFS_MEDIA_TYPE_NAND, 0))
				{
					// チェックディスク実行
					if (FATFS_CheckDisk("nand:", &info, TRUE, TRUE, TRUE))
					{
			            // 指定のNANDパーティション1をGドライブにマウント
//			            if (FATFS_MountDrive("G", FATFS_MEDIA_TYPE_NAND, 1))
						{
							// チェックディスク実行
							if (FATFS_CheckDisk("nand2:", &info, TRUE, TRUE, TRUE))
							{
								result = TRUE;
							}
						}
					}
				}

				// デフォルトマウント状態に戻しておく
//				FATFS_UnmountDrive("G:");
//				FATFS_MountDrive("G", FATFS_MEDIA_TYPE_SD, 0);

				if (result == TRUE) { kamiFontPrintf(24,  y_pos, FONT_COLOR_GREEN, " OK  "); }
				else                { kamiFontPrintf(24,  y_pos, FONT_COLOR_RED,   " NG  "); }

				return FormatProcess1;
			}
#ifndef NAND_FORMATTER_MODE
		case MENU_NORMAL_FORMAT:	// ノーマルフォーマット
			sLock = TRUE;
			ExeFormatAsync(FORMAT_MODE_QUICK, FormatCallback);
			kamiFontPrintf(24,  y_pos, FONT_COLOR_BLACK, "     ");
			return FormatProcess3;
		case MENU_FILL_FORMAT: // フルフォーマット
			sLock = TRUE;
			ExeFormatAsync(FORMAT_MODE_FULL, FormatCallback);
			kamiFontPrintf(24,  y_pos, FONT_COLOR_BLACK, "     ");
			return FormatProcess3;
#endif
		case MENU_RETURN:
			FADE_OUT_RETURN( TopmenuProcess0 );
		}
	}

	return FormatProcess1;
}

static void FormatCallback(KAMIResult result, void* /*arg*/)
{
	s16 y_pos = (s16)(7 + sMenuSelectNo * CHAR_OF_MENU_SPACE);

	if ( result == KAMI_RESULT_SUCCESS_TRUE )
	{
		kamiFontPrintf(24,  y_pos, FONT_COLOR_GREEN, " OK  ");
		sFormatResult = TRUE;
	}
	else
	{
		kamiFontPrintf(24,  y_pos, FONT_COLOR_RED,   " NG  ");
		sFormatResult = FALSE;
	}

	// ロック解除
	sLock = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         Format プロセス３

  Description:  フォーマット完了待ちプロセス

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess3(void)
{
	static s32 progress;
	s16 y_pos = (s16)(7 + sMenuSelectNo * CHAR_OF_MENU_SPACE);

	// 処理終了判定
	if (sLock == FALSE)
	{
		progress = 0;

#ifndef NAND_FORMATTER_MODE
		// Auto用
		if (gAutoFlag)
		{
			if (sFormatResult == TRUE) { FADE_OUT_RETURN( AutoProcess1 ); }
			else { FADE_OUT_RETURN( AutoProcess2 ); }
		}
#endif

		return FormatProcess1;
	}

	// 進捗表示更新
	if ( ++progress >= 30*5 ) 
	{
		progress = 0;
		kamiFontPrintf(24, y_pos, FONT_COLOR_BLACK, "     ");
	}

	kamiFontPrintf((s16)(24 + (progress / 30)),  y_pos, FONT_COLOR_BLACK, "*");

	return FormatProcess3;
}
