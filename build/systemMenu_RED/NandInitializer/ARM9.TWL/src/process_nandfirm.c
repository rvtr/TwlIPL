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
  $Rev:$
  $Author:$
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
#include "cursor.h"
#include "keypad.h"

#include "TWLHWInfo_api.h"
#include <firm/format/firm_common.h>
#include <../build/libraries/spi/ARM9/include/spi.h>

/*---------------------------------------------------------------------------*
    �}�N����`
 *---------------------------------------------------------------------------*/
#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE		8
#define CHAR_OF_MENU_SPACE		1
#define CURSOR_ORIGIN_X			32
#define CURSOR_ORIGIN_Y			40

#define FILE_NUM_MAX			16

#define NAND_BLOCK_BYTE 			0x200
#define NAND_BOOT_FLAG_ADDRESS    	0x2ff
#define NAND_FIRM_START_OFFSET    	0x200

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;
static u8 sNandBootFlag = 0xff;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static void MakeFullPathForSD(char* file_name, char* full_path);
static BOOL WriteNandfirm(char* file_name);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess0(void)
{
    FSFile    dir;
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import NandFirm from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// �z��N���A
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// �t�@�C����������
	sFileNum = 0;

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 1, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 2, BG_COLOR_GREEN, BG_COLOR_TRANS );

    // SD�J�[�h�̃��[�g�f�B���N�g��������
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

		// .nand ��T���ăt�@�C������ۑ����Ă���
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

				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".nand") || !STD_CompareString( pExtension, ".NAND"))
					{
						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

						// �ő�16�ŏI��
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

	// ���j���[�ꗗ
	kamiFontPrintf((s16)3, (s16)4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf((s16)3, (s16)(5+sFileNum+1), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad �t�@�C�����X�g��\��
	for (i=0;i<sFileNum; i++)
	{
		// �t�@�C�����ǉ�
		kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// �Ō�Ƀ��^�[����ǉ�
	kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

	return NandfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess1(void)
{
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return NandfirmProcess2;
	}

	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = sFileNum;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo > sFileNum) sMenuSelectNo = 0;
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return NandfirmProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		return TopmenuProcess0;
	}

	return NandfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         �v���Z�X2

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
		if (gAutoFlag)	{ return AutoProcess2; 		}
		else 			{ return TopmenuProcess0;	}
	}

	// ����̌��ʂ�\��
	if ( ret == TRUE )
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG");
	}

	// Auto�p
	if (gAutoFlag)
	{
		if (ret) return AutoProcess1;
		else return AutoProcess2;
	}

	return NandfirmProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MakeFullPathForSD

  Description:  

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeFullPathForSD(char* file_name, char* full_path)
{
	// �t���p�X���쐬
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

	// .nand�̃t���p�X���쐬
	MakeFullPathForSD(file_name, full_path);

	// .nand�t�@�C���I�[�v��
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, full_path);
    OS_Printf("FS_OpenFile(\"%s\") ... %s!\n", full_path, open_is_ok ? "OK" : "ERROR");

	// �T�C�Y�`�F�b�N
	file_size  = FS_GetFileLength(&file) ;
	if (file_size > (800*1024))
	{
		kamiFontPrintfConsoleEx(1, "too big file size!\n");
		FS_CloseFile(&file);
		return FALSE;
	}

	// �o�b�t�@�m��
	alloc_size = ROUND_UP(file_size, 32) ;
	pTempBuf = OS_Alloc( alloc_size );
	if (pTempBuf == NULL)
	{
		kamiFontPrintfConsoleEx(1, "Fail Alloc()!\n");
		FS_CloseFile(&file);
		return FALSE;		
	}

	// .nand�t�@�C�����[�h
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	DC_FlushRange(pTempBuf, file_size);
	if (!read_is_ok)
	{
		kamiFontPrintfConsoleEx(1, "Fail FS_ReadFile!\n");
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// �t�@�C���N���[�Y
	FS_CloseFile(&file);

	// �������ݑO��CRC���v�Z
	crc_w1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );
	crc_w2 = SVC_GetCRC16( 0xffff, pTempBuf+512, file_size-512 );

	// �܂�NORHeaderDS�̈���������ށi40byte?�j
	if (kamiNvramWrite(0, (void*)pTempBuf, sizeof(NORHeaderDS)) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}

	// CRC���v�Z����̂ŔO�̂��߂ɃN���A���Ă��烊�[�h����
	MI_CpuFill8( pTempBuf, 0xee, sizeof(NORHeaderDS) );
	DC_FlushRange(pTempBuf, sizeof(NORHeaderDS));

	// CRC�`�F�b�N�̂���Nvram���烊�[�h
	if (kamiNvramRead(0, pTempBuf, sizeof(NORHeaderDS) ) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "Fail kamiNvramRead()!\n");
	}

	// �ǂݍ��݂�ARM7�����ڃ������ɏ����o������
	DC_InvalidateRange(pTempBuf, sizeof(NORHeaderDS));
	// �������݌��CRC���v�Z
	crc_r1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );

	// NVRAM�擪������CRC��r
	if ( crc_w1 != crc_r1 )
	{
		OS_Free(pTempBuf);
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w1, crc_r1);
		return FALSE;
	}

	// nandfirm �N���t���O�𗧂Ă�
	if (kamiNvramWrite(NAND_BOOT_FLAG_ADDRESS, &sNandBootFlag, 1) == KAMI_RESULT_SEND_ERROR)
	{
		kamiFontPrintfConsoleEx(1, "Fail kamiNvramWrite()\n");
		result = FALSE;
	}

	kamiFontPrintfConsoleEx(0, "NAND Firm Import Start!\n");

	// NAND��������
	write_size = file_size/NAND_BLOCK_BYTE + (file_size % NAND_BLOCK_BYTE != 0);
	kamiNandWrite( NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE, pTempBuf+NAND_FIRM_START_OFFSET, write_size );	// �u���b�N�P�ʁA�o�C�g�P�ʁA�u���b�N�P��

	kamiFontPrintfConsoleEx(0, "Start CRC check\n");
	kamiFontLoadScreenData();
	
	// CRC���v�Z����̂ŔO�̂��߂ɃN���A���Ă��烊�[�h����
	MI_CpuClear8( pTempBuf, file_size );
	DC_FlushRange(pTempBuf, file_size);

	// CRC�`�F�b�N�̂���Nand���烊�[�h
	if (kamiNandRead(0, pTempBuf, file_size/512 ) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
	}
	DC_FlushRange(pTempBuf, file_size);

	// �������݌��CRC���v�Z
	crc_r2 = SVC_GetCRC16( 0xffff, pTempBuf+512, file_size-512 );

	// NAND�����ɂ��Ă�CRC�`�F�b�N
	if (crc_w2 == crc_r2)
	{
		kamiFontPrintfConsoleEx(0, "Success! CRC check %x==%x\n", crc_w2, crc_r2);
	}
	else
	{
		result = FALSE;
		kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w2, crc_r2);
	}

	// ���������
	OS_Free(pTempBuf);

	return result;
}



