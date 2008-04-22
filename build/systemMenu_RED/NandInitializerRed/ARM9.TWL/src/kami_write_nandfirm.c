/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_write_nandfirm.c

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
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include "kami_font.h"
#include "kami_pxi.h"

#include <firm/format/firm_common.h>
#include "kami_write_nandfirm.h"

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

static u8 sNvramPageSizeBuffer[NVRAM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);	// ARM7からアクセスするためスタックでは駄目
static u32 sReservedAreaEndAddress;

/*---------------------------------------------------------------------------*
  Name:         kamiWriteNandfirm

  Description:  

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL kamiWriteNandfirm(const char* pFullPath, NAMAlloc allocFunc, NAMFree freeFunc)
{
    FSFile  file;

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

	// .nandファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, pFullPath);
//  OS_Printf("FS_OpenFile(\"%s\") ... %s!\n", pFullPath, open_is_ok ? "OK" : "ERROR");

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
	pTempBuf = allocFunc( alloc_size );
	if (pTempBuf == NULL)
	{
		kamiFontPrintfConsoleEx(1, "Fail Alloc()!\n");
		FS_CloseFile(&file);
		return FALSE;		
	}

	// .nandファイルリード
	DC_InvalidateRange(pTempBuf, alloc_size);
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	DC_StoreRange(pTempBuf, file_size);
	if (!read_is_ok)
	{
		kamiFontPrintfConsoleEx(1, "Fail FS_ReadFile!\n");
		FS_CloseFile(&file);
		freeFunc(pTempBuf);
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
	DC_StoreRange(pTempBuf, sizeof(NORHeaderDS));

	// 書き込み後のCRCを計算
	crc_r1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );

	// NVRAM先頭部分のCRC比較
	if ( crc_w1 != crc_r1 )
	{
		freeFunc(pTempBuf);
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w1, crc_r1);
		return FALSE;
	}

	// nandfirm 起動フラグを立てる
	MI_CpuClear8( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	sNvramPageSizeBuffer[NVRAM_NORFIRM_NANDBOOT_FLAG_OFFSET] = NVRAM_NORFIRM_NANDBOOT_FLAG;
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);

	// NORファームリザーブ領域の書き込みデータのCRCを計算
	crc_norfirm_reserved_area_w = SVC_GetCRC16( 0xffff, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	if (kamiNvramWrite(NVRAM_NORFIRM_RESERVED_ADDRESS, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}

	// CRCを計算するので念のためにクリアしてからリードする
	MI_CpuFill8( sNvramPageSizeBuffer, 0xee, NVRAM_PAGE_SIZE );

	// 読み込みはARM7が直接メモリに書き出すため
	DC_FlushRange(sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);

	if (kamiNvramRead(NVRAM_NORFIRM_RESERVED_ADDRESS, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramRead()\n");
		result = FALSE;
	}

	// 書き込み後のCRCを計算
	DC_StoreRange(sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);
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

//	kamiFontPrintfConsoleEx(0, "NAND Firm Import Start!\n");

	// NAND書き込み
	write_size = file_size/NAND_BLOCK_BYTE + (file_size % NAND_BLOCK_BYTE != 0);
	kamiNandWrite( NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE, pTempBuf+NAND_FIRM_START_OFFSET, write_size );	// ブロック単位、バイト単位、ブロック単位

//	kamiFontPrintfConsoleEx(0, "Start CRC check\n");
	kamiFontLoadScreenData();
	
	// CRCを計算するので念のためにクリアしてからリードする
	MI_CpuClear8( pTempBuf, file_size );
	DC_FlushRange(pTempBuf, file_size);

	// CRCチェックのためNandからリード
	if (kamiNandRead(0, pTempBuf, file_size/512 ) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
	}
	DC_StoreRange(pTempBuf, file_size);

	// 書き込み後のCRCを計算
	crc_r2 = SVC_GetCRC16( 0xffff, pTempBuf+512, file_size-512 );

	// NAND部分についてのCRCチェック
	if (crc_w2 == crc_r2)
	{
//		kamiFontPrintfConsoleEx(0, "Success! CRC check %x==%x\n", crc_w2, crc_r2);
	}
	else
	{
		result = FALSE;
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w2, crc_r2);
	}

	// メモリ解放
	freeFunc(pTempBuf);

	return result;
}



