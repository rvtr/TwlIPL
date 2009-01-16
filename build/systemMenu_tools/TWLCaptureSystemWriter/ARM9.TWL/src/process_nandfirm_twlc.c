/*---------------------------------------------------------------------------*
  Project:  TwlIPL - Tools - TWLCaptureSystemWriter
  File:     process_nandfirm_twlc.c

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

// ���O�t�@�C���̏����o�����I�������ɁAROM�̎w�肳�ꂽ�ʒu�Ɋi�[���ꂽnandfirm��{�̂ɏ����߂�
// NandInitializerRed����process_nandfirm.c���㏑��

#include <twl.h>
#include <twl/nam.h>
#include "kami_pxi.h"
#include "kami_font.h"
#include "process_topmenu.h"
#include "process_nandfirm_twlc.h"
#include "process_import_twlc.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "kami_write_nandfirm.h"
#include "common_utility.h"

#define NAND_BLOCK_BYTE 			       0x200
#define NAND_START_OFFSET					0x200
#define NANDFIRM_FILE_START_OFFSET			0x200

// nandfirm�i�[��V���{��
extern void* nandfirm_begin;
extern void* nandfirm_end;

static BOOL writebackFirm( void );

void* NandfirmProcessTWLC0( void )
{
	int i;
	
	kamiFontClear();
	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import NandFirm from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 1, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 2, BG_COLOR_GREEN, BG_COLOR_TRANS );

	// ���j���[�ꗗ
	kamiFontPrintf((s16)3, (s16)4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf((s16)3, (s16)5, FONT_COLOR_BLACK, "l   %-16.16s l    l", "included_file");
	kamiFontPrintf((s16)3, (s16)6, FONT_COLOR_BLACK, "l   RETURN           l    l");
	kamiFontPrintf((s16)3, (s16)7, FONT_COLOR_BLACK, "+--------------------+----+");

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( NandfirmProcessTWLC1 );
}

void* NandfirmProcessTWLC1(void)
{
	BOOL ret;
	
	ret = writebackFirm();
	
	if( ret )
	{
		gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_NANDFIRM] = AUTO_PROCESS_RESULT_SUCCESS;
		kamiFontPrintfConsole(CONSOLE_ORANGE, "nandfirm write succeeded.") ;
		FADE_OUT_RETURN(AutoProcess1);
	}
	else
	{
		gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_NANDFIRM] = AUTO_PROCESS_RESULT_FAILURE;  
		kamiFontPrintfConsole(CONSOLE_RED, "nandfirm write failed.") ;
		FADE_OUT_RETURN(AutoProcess2);
	}
	
	
}


static BOOL writebackFirm( void )
{
	u8 *pBuf;
	u8 *pVerifyBuf;
	u32 allocSize;
	u32 fileSize = (u32)(&nandfirm_end) - (u32)(&nandfirm_begin);
	u32 nandfirmSize = fileSize - NANDFIRM_FILE_START_OFFSET;
	u32 writeBlock;
	u16 crc1,crc2;
	BOOL ret = TRUE;
	
	if( 800*1024 < fileSize ) 
	{
		kamiFontPrintfConsole(CONSOLE_RED, "too large file size." );
		return FALSE;
	}
		
	// �������݃T�C�Y��512�o�C�g�̃u���b�N�P�ʂȂ̂ł���ɔz��
	allocSize = MATH_ROUNDUP(fileSize, 512); 
	pBuf = OS_Alloc(allocSize);
	if(pBuf == NULL)
	{
		kamiFontPrintfConsole(CONSOLE_RED, "Alloc failed." );
		return FALSE;
	}
	
	// �f�[�^�̓ǂݏo��
	MI_CpuClear8(pBuf, allocSize);
	DC_FlushRange(pBuf, allocSize);
	MI_CpuCopy8( &nandfirm_begin, pBuf, fileSize);
	
	// �f�[�^�̏����o��	
	writeBlock = fileSize/NAND_BLOCK_BYTE + (fileSize % NAND_BLOCK_BYTE != 0);
	DC_FlushRange(pBuf, allocSize);
	kamiFontPrintfConsole(CONSOLE_ORANGE, "nandfirm writing..." );	
	kamiNandWrite( NAND_START_OFFSET/NAND_BLOCK_BYTE, pBuf+NANDFIRM_FILE_START_OFFSET, writeBlock);
	/*
	// �������݌��CRC�`�F�b�N���ꉞ����Ă���
	pVerifyBuf = OS_Alloc(allocSize);
	if (kamiNandRead(NAND_START_OFFSET/NAND_BLOCK_BYTE, pVerifyBuf, writeBlock ) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
	}
	DC_FlushRange(pVerifyBuf, allocSize);

	crc1 = SVC_GetCRC16( 0xffff, pBuf, fileSize );
	crc2 = SVC_GetCRC16( 0xffff, pVerifyBuf, fileSize );
	if(crc1 != crc2)
	{
		ret = FALSE;
	}
	
	OS_Free(pVerifyBuf);*/
	OS_Free(pBuf);

	return ret;
}
