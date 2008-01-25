/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_norfirm.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <stddef.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_import.h"
#include "process_norfirm.h"
#include "cursor.h"
#include "keypad.h"

#include "TWLHWInfo_api.h"

#include <firm/format/firm_common.h>
#include <firm/format/norfirm.h>
#include <../build/libraries/spi/ARM9/include/spi.h>

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE		8
#define CHAR_OF_MENU_SPACE		1

#define CURSOR_ORIGIN_X			32
#define CURSOR_ORIGIN_Y			40

#define FILE_NUM_MAX			16

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void MakeFullPathForSD(char* file_name, char* full_path);
static BOOL WriteNorfirm(char* file_name);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NorfirmProcess0(void)
{
    FSFile    dir;
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import Norfirm from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// 配列クリア
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// ファイル数初期化
	sFileNum = 0;

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_VIOLET, BG_COLOR_VIOLET );
	kamiFontFillChar( 1, BG_COLOR_VIOLET, BG_COLOR_VIOLET );
	kamiFontFillChar( 2, BG_COLOR_VIOLET, BG_COLOR_TRANS );

    // SDカードのルートディレクトリを検索
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        OS_Printf("Error FS_OpenDirectory(sdmc:/)\n");
		kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "Error FS_OpenDirectory(sdmc:/)");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(0, "------ nor file list -----\n");

		// .nor を探してファイル名を保存しておく
        while (FS_ReadDirectory(&dir, info))
        {
            OS_Printf("  %s", info->longname);
            if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) != 0)
            {
                OS_Printf("/\n");
            }
            else
            {
				char* pExtension;
              OS_Printf(" (%d BYTEs)\n", info->filesize);

				// 拡張子のチェック
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".nor") || !STD_CompareString( pExtension, ".NOR"))
					{
						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(0, "%d:%s\n", sFileNum, info->longname);

						// 最大16個で終了
						if (++sFileNum >= FILE_NUM_MAX)
						{
							break;
						}
					}
				}
            }
        }
        (void)FS_CloseDirectory(&dir);

		kamiFontPrintfConsole(0, "--------------------------\n");
    }

	// メニュー一覧
	kamiFontPrintf((s16)3,  (s16)4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf((s16)3, (s16)(5+sFileNum+1), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad ファイルリストを表示
	for (i=0;i<sFileNum; i++)
	{
		// ファイル名追加
		kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// 最後にリターンを追加
	kamiFontPrintf((s16)3, (s16)(5+CHAR_OF_MENU_SPACE*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

	return NorfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NorfirmProcess1(void)
{
	// 選択メニューの変更
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = sFileNum;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo > sFileNum) sMenuSelectNo = 0;
	}

	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return NorfirmProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		return TopmenuProcess0;
	}

	return NorfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         プロセス2

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NorfirmProcess2(void)
{
	BOOL ret;

	if (STD_GetStringLength(sFilePath[sMenuSelectNo]))
	{
		ret = WriteNorfirm(sFilePath[sMenuSelectNo]);
	}
	else
	{
		// リターン
		return TopmenuProcess0;
	}

	// 今回の結果を表示
	if ( ret == TRUE )
	{
		kamiFontPrintf((s16)26, (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf((s16)26, (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG");
	}

	return NorfirmProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MakeFullPathForSD

  Description:  

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeFullPathForSD(char* file_name, char* full_path)
{
	// フルパスを作成
	STD_CopyString( full_path, "sdmc:/" );
	STD_ConcatenateString( full_path, file_name );
}

/*---------------------------------------------------------------------------*
  Name:         MakeFullPathForSD

  Description:  

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define NVRAM_PAGE_SIZE 256

static BOOL WriteNorfirm(char* file_name)
{
    FSFile  file;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	u8*  pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;
    int nor_addr;
	u16 crc_w1, crc_w2;
	u16 crc_r1, crc_r2;

	// .norのフルパスを作成
	MakeFullPathForSD(file_name, full_path);

	// .norファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, full_path);
    OS_Printf("FS_OpenFile(\"%s\") ... %s!\n", full_path, open_is_ok ? "OK" : "ERROR");

	// サイズチェック
	file_size  = FS_GetFileLength(&file) ;
	if (file_size > 256*1024)
	{
		kamiFontPrintfConsoleEx(1, "too big file size!\n");
		FS_CloseFile(&file);
		return FALSE;
	}

	// バッファ確保
	alloc_size = ROUND_UP(file_size, 32) ;
	pTempBuf = OS_Alloc( alloc_size );
	if (pTempBuf == NULL)
	{
		kamiFontPrintfConsoleEx(1, "Fail Alloc()\n");
		FS_CloseFile(&file);
		return FALSE;		
	}

	// .norファイルリード
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	DC_FlushRange(pTempBuf, file_size);
	if (!read_is_ok)
	{
    	kamiFontPrintfConsoleEx(1, "FS_ReadFile(\"%s\") ... ERROR!\n", full_path);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ファイルクローズ
	FS_CloseFile(&file);

	// 書き込み前のCRCを計算
	crc_w1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );
	crc_w2 = SVC_GetCRC16( 0xffff, pTempBuf+512, file_size-512 );

	// まずNORHeaderDS領域を書き込む（40byte?）
	if (kamiNvramWrite(0, (void*)pTempBuf, sizeof(NORHeaderDS)) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail SPI_NvramPageWrite()\n");
		result = FALSE;
	}

	// CRCチェックのためNvramからリード
	if (kamiNvramRead(0, pTempBuf, sizeof(NORHeaderDS) ) == KAMI_RESULT_SEND_ERROR)
	{
	    OS_Printf("kamiNvramRead ... ERROR!\n");
	}

	// 読み込みはARM7が直接メモリに書き出す
	DC_InvalidateRange(pTempBuf, sizeof(NORHeaderDS));
	// 書き込み後のCRCを計算
	crc_r1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );

	// NVRAM前半部のCRCをチェック
	if ( crc_w1 != crc_r1 )
	{
		OS_Free(pTempBuf);
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w1, crc_r1);
		return FALSE;
	}

	nor_addr = offsetof(NORHeader, l);	// 512byte

	// 進捗メーター初期化
	ProgressInit();
	kamiFontPrintfConsole(0, "NOR Firm Import Start!\n");

	while ( nor_addr < file_size)
	{
		// 書きこみ
		if (kamiNvramWrite((u32)nor_addr, (void*)(pTempBuf + nor_addr), NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
		{
			OS_TPrintf("======= Fail SPI_NvramPageWrite() ======== \n");
			result = FALSE;
			break;
		}
		nor_addr += NVRAM_PAGE_SIZE;

		// 進捗メーター表示
		ProgressDraw((f32)nor_addr/file_size);
	}

	kamiFontPrintfConsoleEx(0, "Start CRC check\n");
	kamiFontLoadScreenData();
	
	// CRCチェックのためNvramからリード
	if (kamiNvramRead(0, pTempBuf, file_size ) == KAMI_RESULT_SEND_ERROR)
	{
	    OS_Printf("kamiNvramRead ... ERROR!\n");
	}

	// 読み込みはARM7が直接メモリに書き出す
	DC_InvalidateRange(pTempBuf, file_size);

	// 書き込み後のCRCを計算
	crc_r2 = SVC_GetCRC16( 0xffff, pTempBuf+512, file_size-512 );

	// CRC比較
	if (crc_w2 != crc_r2)
	{
		result = FALSE;
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w2, crc_r2);
	}

	// メモリ解放
	OS_Free(pTempBuf);

	return result;
}



