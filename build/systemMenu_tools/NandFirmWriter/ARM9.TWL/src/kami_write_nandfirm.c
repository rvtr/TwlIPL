/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_write_nandfirm.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-08#$
  $Rev: 2427 $
  $Author: kamikawa $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <nitro/snd.h>
#include <twl/nam.h>
#include <nitro/nvram.h>
#include <../build/libraries/spi/ARM9/include/spi.h>

#include "kami_write_nandfirm.h"
#include "kami_pxi.h"

/*---------------------------------------------------------------------------*
    �}�N����`
 *---------------------------------------------------------------------------*/

// NAND�t�@�[���������݂̍ۂ�NVRAM�̖����蓖�ė̈�{�\��̈���O�N���A����ꍇ�͒�`���܂��i�J���p�j
//#define CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define NAND_BLOCK_BYTE 			       	0x200
#define NAND_FIRM_START_OFFSET    	       	0x200
#define NAND_FIRM_START_OFFSET_IN_FILE     	0x200

#define NVRAM_PAGE_SIZE 0x100
#define NVRAM_NORFIRM_RESERVED_ADDRESS     	0x200
#define NVRAM_NORFIRM_NANDBOOT_FLAG_OFFSET 	0xff
#define NVRAM_NORFIRM_NANDBOOT_FLAG        	0x80

#define NVRAM_NON_ASIGNED_AREA_ADDRESS     	0x300


#define DOT_OF_MENU_SPACE					8
#define CHAR_OF_MENU_SPACE					1
#define CURSOR_ORIGIN_X						32
#define CURSOR_ORIGIN_Y						40

#define FILE_NUM_MAX						16

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static u8 sNvramPageSizeBuffer[NVRAM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);	// ARM7����A�N�Z�X���邽�߃X�^�b�N�ł͑ʖ�
static u32 sReservedAreaEndAddress;

static s32 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static void kamiEraseNandfirmVersion( u32 nandfirmsize );
BOOL GetNandFirmBinPath(void);
BOOL kamiWriteNandfirm(const char* pFullPath, NAMAlloc allocFunc, NAMFree freeFunc);

/*---------------------------------------------------------------------------*
  Name:         �v���Z�X

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/
BOOL NandfirmProcess(void)
{
	BOOL ret = FALSE;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	if( !GetNandFirmBinPath() )
    {
        OS_PutString("Failed GetNandFirmBinPath()\n");
		return FALSE;
    }
    
	// sMenuSelectNo�̏����������Ă���
	sMenuSelectNo = 0;
    
	if (STD_GetStringLength(sFilePath[sMenuSelectNo]))
	{
		// .nand�̃t���p�X���쐬
		MakeFullPathForSD(sFilePath[sMenuSelectNo], full_path);
		ret = kamiWriteNandfirm(full_path, OS_AllocFromMain, OS_FreeToMain);
	}

	// ����̌��ʂ�\��
	if ( ret == TRUE )
	{
        OS_PutString("NandFirmWrite Success\n");
	}
	else
	{
        OS_PutString("NandFirmWrite Fail\n");
	}
    
    return ret;
}


/*---------------------------------------------------------------------------*
  Name:         GetNandFirmBinPath

  Description:  

  Arguments:    no

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL GetNandFirmBinPath(void)
{
    FSFile    dir;
	BOOL ret = TRUE;

	// �z��N���A
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// �t�@�C����������
	sFileNum = 0;

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        ret = FALSE;
        OS_TPrintf("Error FS_OpenDirectory(sdmc:/)\n");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		//kamiFontPrintfConsole(CONSOLE_ORANGE, "------ nand file list -----\n");

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
						//kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

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
    }

    return ret;
}


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
	u32 nandfirm_size;
	u32 alloc_size;
	u32 write_block;
	BOOL result = TRUE;
	u16 crc_w1, crc_w2;
	u16 crc_r1, crc_r2;
	u16 crc_norfirm_reserved_area_w, crc_norfirm_reserved_area_r;
#ifdef    CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL
	u32 write_offset;
#endif // CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL

	// .nand�t�@�C���I�[�v��
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, pFullPath);
	if (!open_is_ok)
	{
		OS_Warning("Failure! FS_OpenFile");
		return FALSE;
	}

	// �T�C�Y�`�F�b�N
	file_size  = FS_GetFileLength(&file) ;
	if (file_size > (800*1024))
	{
		//kamiFontPrintfConsoleEx(1, "too big file size!\n");
		FS_CloseFile(&file);
		return FALSE;
	}

	nandfirm_size = file_size - NAND_FIRM_START_OFFSET_IN_FILE;

	// �o�b�t�@�m��
	// �������݂��u���b�N�P��(512byte)�ł��邱�Ƃ��l����512�A���C�����g���m��
	alloc_size = MATH_ROUNDUP(file_size, 512);
	pTempBuf = allocFunc( alloc_size );
	if (pTempBuf == NULL)
	{
		//kamiFontPrintfConsoleEx(1, "Fail Alloc()!\n");
		FS_CloseFile(&file);
		return FALSE;		
	}

	MI_CpuClear8( pTempBuf, alloc_size );

	// .nand�t�@�C�����[�h
	DC_FlushRange(pTempBuf, alloc_size);
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	DC_FlushRange(pTempBuf, file_size);
	if (!read_is_ok)
	{
		//kamiFontPrintfConsoleEx(1, "Fail FS_ReadFile!\n");
		FS_CloseFile(&file);
		freeFunc(pTempBuf);
		return FALSE;
	}

	// �t�@�C���N���[�Y
	FS_CloseFile(&file);

	// �������ݑO��CRC���v�Z
	crc_w1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );
	crc_w2 = SVC_GetCRC16( 0xffff, pTempBuf+NAND_FIRM_START_OFFSET_IN_FILE, nandfirm_size );

	// �܂�NORHeaderDS�̈���������ށi40byte?�j
	if (NVRAMi_Write(0, sizeof(NORHeaderDS), (void*)pTempBuf) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Write()\n");
		result = FALSE;
	}

	// CRC���v�Z����̂ŔO�̂��߂ɃN���A���Ă��烊�[�h����
	MI_CpuClear8( pTempBuf, sizeof(NORHeaderDS) );
	DC_FlushRange(pTempBuf, sizeof(NORHeaderDS));

	// CRC�`�F�b�N�̂���Nvram���烊�[�h
	if (NVRAMi_Read(0, sizeof(NORHeaderDS), pTempBuf) != NVRAM_RESULT_SUCCESS)
	{
        OS_PutString("Fail NVRAMi_Read()!\n");
	    //kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Read()!\n");
	}
	DC_FlushRange(pTempBuf, sizeof(NORHeaderDS));

	// �������݌��CRC���v�Z
	crc_r1 = SVC_GetCRC16( 0xffff, pTempBuf, sizeof(NORHeaderDS) );

	// NVRAM�擪������CRC��r
	if ( crc_w1 != crc_r1 )
	{
		freeFunc(pTempBuf);
		//kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w1, crc_r1);
		return FALSE;
	}

	// nandfirm �N���t���O�𗧂Ă�
	MI_CpuClear8( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	sNvramPageSizeBuffer[NVRAM_NORFIRM_NANDBOOT_FLAG_OFFSET] = NVRAM_NORFIRM_NANDBOOT_FLAG;
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);

	// NOR�t�@�[�����U�[�u�̈�̏������݃f�[�^��CRC���v�Z
	crc_norfirm_reserved_area_w = SVC_GetCRC16( 0xffff, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	if (NVRAMi_Write(NVRAM_NORFIRM_RESERVED_ADDRESS, NVRAM_PAGE_SIZE, sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Write()\n");
		result = FALSE;
	}

	// CRC���v�Z����̂ŔO�̂��߂ɃN���A���Ă��烊�[�h����
	MI_CpuClear8( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	// �ǂݍ��݂�ARM7�����ڃ������ɏ����o������
	DC_FlushRange(sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);

	if (NVRAMi_Read(NVRAM_NORFIRM_RESERVED_ADDRESS, NVRAM_PAGE_SIZE, sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Read()\n");
		result = FALSE;
	}

	// �������݌��CRC���v�Z
	DC_FlushRange(sNvramPageSizeBuffer, NVRAM_PAGE_SIZE);
	crc_norfirm_reserved_area_r = SVC_GetCRC16( 0xffff, sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	// NOR�t�@�[�����U�[�u�̈��CRC��r
	if ( crc_norfirm_reserved_area_w != crc_norfirm_reserved_area_r )
	{
		//kamiFontPrintfConsoleEx(1, "Fail! Norfirm Reserved Area CRC check %x!=%x\n", crc_norfirm_reserved_area_w, crc_norfirm_reserved_area_r);
		result = FALSE;
	}

#ifdef CLEAR_NON_ASIGNED_AREA_AND_RESERVED_AREA_ALL
	DC_InvalidateRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	// �����蓖�ė̈�{�\��̈���O�N���A���܂��i�J���p�j
	if (NVRAMi_Read(NVRAM_CONFIG_DATA_OFFSET_ADDRESS, NVRAM_PAGE_SIZE, &sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Read()\n");
		result = FALSE;
	}
    sReservedAreaEndAddress = (u32)(*(u16 *)sNvramPageSizeBuffer << NVRAM_CONFIG_DATA_OFFSET_SHIFT) - 0xA00;// TWL WiFi�ݒ� + NTR WiFi�ݒ� ����������
	//OS_Printf("end = %x\n", sReservedAreaEndAddress);

	MI_CpuFill8( sNvramPageSizeBuffer, 0x00, NVRAM_PAGE_SIZE );
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	for (write_offset=NVRAM_NON_ASIGNED_AREA_ADDRESS; write_offset < sReservedAreaEndAddress; write_offset += NVRAM_PAGE_SIZE)
	{
		if (NVRAMi_Write(write_offset, NVRAM_PAGE_SIZE, sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
		{
			//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Write()\n");
			result = FALSE;
		}
	}
	//OS_Printf("write_offset = %x\n", write_offset);
#else
	// �����蓖�ė̈�擪256byte�{�\��̈���O�N���A���܂�

	MI_CpuFill8( sNvramPageSizeBuffer, 0x00, NVRAM_PAGE_SIZE );
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	if (NVRAMi_Write(NVRAM_NON_ASIGNED_AREA_ADDRESS, NVRAM_PAGE_SIZE, sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Write()\n");
		result = FALSE;
	}

	DC_InvalidateRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );
	if (NVRAMi_Read(NVRAM_CONFIG_DATA_OFFSET_ADDRESS, NVRAM_PAGE_SIZE, &sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Read()\n");
		result = FALSE;
	}
    sReservedAreaEndAddress = (u32)(*(u16 *)sNvramPageSizeBuffer << NVRAM_CONFIG_DATA_OFFSET_SHIFT) - 0xA00;// TWL WiFi�ݒ� + NTR WiFi�ݒ� ����������

	MI_CpuFill8( sNvramPageSizeBuffer, 0x00, NVRAM_PAGE_SIZE );
	DC_FlushRange( sNvramPageSizeBuffer, NVRAM_PAGE_SIZE );

	if (NVRAMi_Write(sReservedAreaEndAddress - 0x100, NVRAM_PAGE_SIZE, sNvramPageSizeBuffer) != NVRAM_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail NVRAMi_Write()\n");
		result = FALSE;
	}
#endif

	// NAND���O���̃N���A
	if (kamiClearNandErrorLog() != KAMI_RESULT_SUCCESS)
	{
		//kamiFontPrintfConsoleEx(1, "Fail kamiClearNandErrorLog()\n");
		result = FALSE;		
	}

	// nandfirm�o�[�W�����̏����i�f�o�b�O�p�j
	kamiEraseNandfirmVersion(nandfirm_size);

//	kamiFontPrintfConsoleEx(0, "NAND Firm Import Start!\n");

	// NAND��������
	write_block = nandfirm_size/NAND_BLOCK_BYTE + (nandfirm_size % NAND_BLOCK_BYTE != 0);
	kamiNandWrite( NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE, pTempBuf+NAND_FIRM_START_OFFSET, write_block );	// �u���b�N�P�ʁA�o�C�g�P�ʁA�u���b�N�P��
//	kamiFontLoadScreenData();
	
	// CRC���v�Z����̂ŔO�̂��߂ɃN���A���Ă��烊�[�h����
	MI_CpuClear8( pTempBuf, nandfirm_size );
	DC_FlushRange(pTempBuf, nandfirm_size);

	// CRC�`�F�b�N�̂���Nand���烊�[�h
	if (kamiNandRead(NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE, pTempBuf, write_block ) == KAMI_RESULT_SEND_ERROR)
	{
	    //kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
	}
	DC_FlushRange(pTempBuf, nandfirm_size);

	// �������݌��CRC���v�Z
	crc_r2 = SVC_GetCRC16( 0xffff, pTempBuf, nandfirm_size );

	// NAND�����ɂ��Ă�CRC�`�F�b�N
	if (crc_w2 == crc_r2)
	{
//		kamiFontPrintfConsoleEx(0, "Success! CRC check %x==%x\n", crc_w2, crc_r2);
	}
	else
	{
		result = FALSE;
		//kamiFontPrintfConsoleEx(1, "Fail! CRC check %x!=%x\n", crc_w2, crc_r2);
	}

	// ���������
	freeFunc(pTempBuf);

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         kamiEraseNandfirmVersion

  Description:  nandfirm�̃o�[�W���������������܂��B�i�f�o�b�O�p�j

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
void kamiEraseNandfirmVersion( u32 nandfirmsize )
{
	u8 buffer[NAND_BLOCK_BYTE];
	u32 blockNo;

	if ((nandfirmsize % NAND_BLOCK_BYTE)==0)
	{
		blockNo = NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE + nandfirmsize/NAND_BLOCK_BYTE;
		MI_CpuClear8( buffer, NAND_BLOCK_BYTE );
		DC_FlushRange(buffer, NAND_BLOCK_BYTE);
		kamiNandWrite( blockNo, buffer, 1 );	// �u���b�N�P�ʁA�o�C�g�P�ʁA�u���b�N�P��
	}
}
