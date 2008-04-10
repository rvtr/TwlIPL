/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_nandfirm.c

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
#include <twl/nam.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_nandfirm.h"
#include "process_import.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

#include "TWLHWInfo_api.h"
#include <firm/format/firm_common.h>
#include <../build/libraries/spi/ARM9/include/spi.h>

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/

// NANDファーム書き込みの際にNVRAMの未割り当て領域＋予約領域を０クリアする場合は定義します（開発用）
//#define CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE		8
#define CHAR_OF_MENU_SPACE		1
#define CURSOR_ORIGIN_X			32
#define CURSOR_ORIGIN_Y			40

#define FILE_NUM_MAX			16

#define NAND_BLOCK_BYTE 			0x200
#define NAND_FIRM_START_OFFSET    	0x200

#define NVRAM_PAGE_SIZE 0x100
#define NVRAM_NORFIRM_RESERVED_ADDRESS     0x200
#define NVRAM_NORFIRM_NANDBOOT_FLAG_OFFSET 0xff
#define NVRAM_NORFIRM_NANDBOOT_FLAG        0x80

#define NVRAM_NON_ASIGNED_AREA_ADDRESS     0x300

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;
static u8 sNvramPageSizeBuffer[NVRAM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);	// ARM7からアクセスするためスタックでは駄目
static u32 sReservedAreaEndAddress;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void MakeFullPathForSD(char* file_name, char* full_path);
static BOOL WriteNandfirm(char* file_name);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess0(void)
{
    FSFile    dir;
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import NandFirm from SD");
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
	kamiFontFillChar( 0, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 1, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 2, BG_COLOR_GREEN, BG_COLOR_TRANS );

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

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ nand file list -----\n");

		// .nand を探してファイル名を保存しておく
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
					if (!STD_CompareString( pExtension, ".nand") || !STD_CompareString( pExtension, ".NAND"))
					{
						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

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

		kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");
    }

	// メニュー一覧
	kamiFontPrintf((s16)3, (s16)4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf((s16)3, (s16)(5+sFileNum+1), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad ファイルリストを表示
	for (i=0;i<sFileNum; i++)
	{
		// ファイル名追加
		kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// 最後にリターンを追加
	kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( NandfirmProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess1(void)
{
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return NandfirmProcess2;
	}

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
		return NandfirmProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return NandfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         プロセス2

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess2(void)
{
	BOOL ret;

	if (STD_GetStringLength(sFilePath[sMenuSelectNo]))
	{
		ret = WriteNandfirm(sFilePath[sMenuSelectNo]);
	}
	else
	{
		if (gAutoFlag)	{ FADE_OUT_RETURN( AutoProcess2 ); 		}
		else 			{ FADE_OUT_RETURN( TopmenuProcess0 );	}
	}

	// 今回の結果を表示
	if ( ret == TRUE )
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG");
	}

	// Auto用
	if (gAutoFlag)
	{
		if (ret) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2 ); }
	}

	return NandfirmProcess1;
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
static BOOL WriteNandfirm(char* file_name)
{
    FSFile  file;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

    BOOL    open_is_ok;
	BOOL    read_is_ok;
	u8* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	u32 write_size;
	BOOL result = TRUE;
	u16 crc_w1, crc_w2;
	u16 crc_r1, crc_r2;
	u16 crc_norfirm_reserved_area_w, crc_norfirm_reserved_area_r;
#ifdef CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL
	u32 write_offset;
#endif

	// .nandのフルパスを作成
	MakeFullPathForSD(file_name, full_path);

	// .nandファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, full_path);
    OS_Printf("FS_OpenFile(\"%s\") ... %s!\n", full_path, open_is_ok ? "OK" : "ERROR");

	// サイズチェック
	file_size  = FS_GetFileLength(&file) ;
	if (file_size > (800*1024))
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
		kamiFontPrintfConsoleEx(1, "Fail Alloc()!\n");
		FS_CloseFile(&file);
		return FALSE;		
	}

	// .nandファイルリード
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	DC_FlushRange(pTempBuf, file_size);
	if (!read_is_ok)
	{
		kamiFontPrintfConsoleEx(1, "Fail FS_ReadFile!\n");
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
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}

	// CRCを計算するので念のためにクリアしてからリードする
	MI_CpuFill8( pTempBuf, 0xee, sizeof(NORHeaderDS) );
	DC_FlushRange(pTempBuf, sizeof(NORHeaderDS));

	// CRCチェックのためNvramからリード
	if (kamiNvramRead(0, pTempBuf, sizeof(NORHeaderDS) ) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "Fail kamiNvramRead()!\n");
	}

	// 書き込み後のCRCを計算
	crc_r1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );

	// NVRAM先頭部分のCRC比較
	if ( crc_w1 != crc_r1 )
	{
		OS_Free(pTempBuf);
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w1, crc_r1);
		return FALSE;
	}

	// nandfirm 起動フラグを立てる
	MI_CpuClear8( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	sNvramPageSizeBuffer[NVRAM_NORFIRM_NANDBOOT_FLAG_OFFSET] = NVRAM_NORFIRM_NANDBOOT_FLAG;

	// NORファームリザーブ領域の書き込みデータのCRCを計算
	crc_norfirm_reserved_area_w = SVC_GetCRC16( 0xffff, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	if (kamiNvramWrite(NVRAM_NORFIRM_RESERVED_ADDRESS, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}

	// CRCを計算するので念のためにクリアしてからリードする
	MI_CpuFill8( sNvramPageSizeBuffer, 0xee, NVRAM_PAGE_SIZE );

	// 読み込みはARM7が直接メモリに書き出すため
	DC_InvalidateRange(sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);

	if (kamiNvramRead(NVRAM_NORFIRM_RESERVED_ADDRESS, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramRead()\n");
		result = FALSE;
	}

	// 書き込み後のCRCを計算
	crc_norfirm_reserved_area_r = SVC_GetCRC16( 0xffff, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	// NORファームリザーブ領域のCRC比較
	if ( crc_norfirm_reserved_area_w != crc_norfirm_reserved_area_r )
	{
		kamiFontPrintfConsoleEx(1, "Fail! Norfirm Reserved Area CRC check %x!=%x\n", crc_norfirm_reserved_area_w, crc_norfirm_reserved_area_r);
		result = FALSE;
	}

#ifdef CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL
	DC_InvalidateRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	// 未割り当て領域＋予約領域を０クリアします（開発用）
	if (kamiNvramRead(NVRAM_CONFIG_DATA_OFFSET_ADDRESS, &sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramRead()\n");
		result = FALSE;
	}
    sReservedAreaEndAddress = (u32)(*(u16 *)sNvramPageSizeBuffer << NVRAM_CONFIG_DATA_OFFSET_SHIFT) - 0xA00;// TWL WiFi設定 + NTR WiFi設定 を差し引く
	//OS_Printf("end = %x\n", sReservedAreaEndAddress);

	MI_CpuFill8( sNvramPageSizeBuffer, 0x00, NVRAM_PAGE_SIZE );
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	for (write_offset=NVRAM_NON_ASIGNED_AREA_ADDRESS; write_offset < sReservedAreaEndAddress; write_offset += NVRAM_PAGE_SIZE)
	{
		if (kamiNvramWrite(write_offset, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
		{
			kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
			result = FALSE;
		}
	}
	//OS_Printf("write_offset = %x\n", write_offset);
#else
	// 未割り当て領域先頭256byte＋予約領域を０クリアします

	MI_CpuFill8( sNvramPageSizeBuffer, 0x00, NVRAM_PAGE_SIZE );
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	if (kamiNvramWrite(NVRAM_NON_ASIGNED_AREA_ADDRESS, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}

	DC_InvalidateRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	if (kamiNvramRead(NVRAM_CONFIG_DATA_OFFSET_ADDRESS, &sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramRead()\n");
		result = FALSE;
	}
    sReservedAreaEndAddress = (u32)(*(u16 *)sNvramPageSizeBuffer << NVRAM_CONFIG_DATA_OFFSET_SHIFT) - 0xA00;// TWL WiFi設定 + NTR WiFi設定 を差し引く

	MI_CpuFill8( sNvramPageSizeBuffer, 0x00, NVRAM_PAGE_SIZE );
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	if (kamiNvramWrite(sReservedAreaEndAddress - 0x100, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}
#endif

	// NANDログ情報のクリア
	if (kamiClearNandErrorLog() != KAMI_RESULT_SUCCESS)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiClearNandErrorLog()\n");
		result = FALSE;		
	}

	kamiFontPrintfConsoleEx(0, "NAND Firm Import Start!\n");

	// NAND書き込み
	write_size = file_size/NAND_BLOCK_BYTE + (file_size % NAND_BLOCK_BYTE != 0);
	kamiNandWrite( NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE, pTempBuf+NAND_FIRM_START_OFFSET, write_size );	// ブロック単位、バイト単位、ブロック単位

	kamiFontPrintfConsoleEx(0, "Start CRC check\n");
	kamiFontLoadScreenData();
	
	// CRCを計算するので念のためにクリアしてからリードする
	MI_CpuClear8( pTempBuf, file_size );
	DC_FlushRange(pTempBuf, file_size);

	// CRCチェックのためNandからリード
	if (kamiNandRead(0, pTempBuf, file_size/512 ) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
	}
	DC_FlushRange(pTempBuf, file_size);

	// 書き込み後のCRCを計算
	crc_r2 = SVC_GetCRC16( 0xffff, pTempBuf+512, file_size-512 );

	// NAND部分についてのCRCチェック
	if (crc_w2 == crc_r2)
	{
		kamiFontPrintfConsoleEx(0, "Success! CRC check %x==%x\n", crc_w2, crc_r2);
	}
	else
	{
		result = FALSE;
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w2, crc_r2);
	}

	// メモリ解放
	OS_Free(pTempBuf);

	return result;
}



