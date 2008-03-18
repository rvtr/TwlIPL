/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     formatter.c

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

#include	<nitro/types.h>
#include	<twl/init/crt0.h>
#include	<twl/memorymap_sp.h>
#include	<twl/os.h>
#include	<twl/spi.h>
#include	<twl/fatfs.h>
#include	<nitro/pad.h>
#include	<nitro/std.h>
#include	<nitro/card.h>
#include    "formatter.h"

extern BOOL FATFSi_nandFillPartition( int partition_no, u32* buf, u32 blocks);

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

typedef struct FileProperty {
	u32			length;
	const char *path;
}FileProperty;

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define ERROR_RETURN()  { OS_TPrintf("FATAL ERROR OCCURED %s %s\n", __FILE__, __LINE__); return 0; }

// ������ҏW������ASDK��FATFS_Init���̓��p�����[�^���C������K�v������B
#define NAND_SIZE				245			// 256MB mobiNAND�ł̎g�p�\�T�C�Y�iiNAND�ł͈Ⴄ�l�ɂȂ�B����B�j
#define PARTITION_RAW_SIZE		4
#define PARTITION_0_SIZE		213
#define PARTITION_1_SIZE		( NAND_SIZE - PARTITION_RAW_SIZE - PARTITION_0_SIZE )
#define NAND_FAT_PARTITION_NUM	2			// FAT�p�[�e�B�V�������iRAW�p�[�e�B�V�����������j
		

// const data--------------------------------------------------------
//#define NAND_SEPARATE_READ
#define FS_READ_BLOCK_SIZE			(  2 * 1024 )
#define FATFS_CLUSTER_SIZE			( 16 * 1024 )

// �t�@�C������t�@�C���T�C�Y�ύX�ւ̒Ǐ]����ԂȂ̂�
// HWInfo�̃��C�g����LCFG���C�u�����Ń��J�o����������悤�ɂ��܂�

// FATFS�̃N���X�^�T�C�Y��16KB�Ȃ̂ŁA�f�[�^�T�C�Y�����܂��Ă��Ȃ����̂́A�]�T����������16KB�ɂ��Ă���
//static const FileProperty s_fileList[] = {
//	{  128,                "nand:/sys/ID.sgn"                     },	// ����A�S���T�C�Y�͓K���B���g����B
//	{  LCFG_TWL_HWINFO_FILE_LENGTH, LCFG_TWL_HWINFO_NORMAL_PATH   },
//	{  LCFG_TWL_HWINFO_FILE_LENGTH, LCFG_TWL_HWINFO_SECURE_PATH   },
//	{  FATFS_CLUSTER_SIZE, "nand:/shared1/TWLCFG0.dat"  },
//	{  FATFS_CLUSTER_SIZE, "nand:/shared1/TWLCFG1.dat"  },	// �~���[
//	{  0, NULL },
//};

static const char *s_pDirList0[] = {
	(const char *)"nand:/sys",
	(const char *)"nand:/title",
	(const char *)"nand:/ticket",
	(const char *)"nand:/shared1",
	(const char *)"nand:/shared2",
	(const char *)"nand:/import",
	(const char *)"nand:/tmp",
	NULL,
	};

static const char *s_pDirList1[] = {
	(const char *)"nand2:/photo",
	NULL,
};

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL CreateDirectory( const char *pDrive, const char **ppDirList );
static BOOL CheckDirectory ( const char *pDrive, const char **ppDirList );
static BOOL CreateFile( const FileProperty *pFileList );
static BOOL CheckFile ( const FileProperty *pFileList );
static int  MY_ReadFile( FATFSFileHandle file, void *pBuffer, int length );

/*---------------------------------------------------------------------------*
  Name:         ExeFormat

  Description:  Nand�̃t�H�[�}�b�g���s���܂�

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL
ExeFormat(FormatMode format_mode)
{
	u32 *init_datbuf;
    int     nand_fat_partition_num;

	init_datbuf = OS_Alloc( 512*16 );
	if( init_datbuf == NULL ) {
		OS_TPrintf( "memory allocate error.\n" );
		ERROR_RETURN();
	}

    MI_CpuFill8( init_datbuf, 0xFF, 512*16);

    // NAND���t�H�[�}�b�g
    {
        int     i;
		u8 drive_nand;
		u8 drive_nand2;
		char drive_nand_path[4];
		char drive_nand2_path[4];

        /* �p�[�e�B�V�����T�C�Y���v�����v�g����ݒ� */
        u32     partition_MB_size[5];
        partition_MB_size[0] = PARTITION_RAW_SIZE;		// RAW�̈�
        partition_MB_size[1] = PARTITION_0_SIZE;		// FAT0�̈�
        partition_MB_size[2] = PARTITION_1_SIZE;		// FAT1�̈�
        nand_fat_partition_num = NAND_FAT_PARTITION_NUM;

		// OSMountInfo���nand&nand2�̃h���C�u���蓖�Ă𒲂ׂ�
	    {
	        const OSMountInfo  *info;
	        for (info = OS_GetMountInfo(); *info->drive; ++info)
	        {
				if (!STD_CompareNString( "nand2", info->archiveName, 5 ))
				{
					drive_nand2 = *(info->drive);
                    STD_TSPrintf(drive_nand2_path, "%c:", drive_nand2);
				}
				else if (!STD_CompareNString( "nand", info->archiveName, 4 ))
				{
					drive_nand = *(info->drive);
                    STD_TSPrintf(drive_nand_path, "%c:", drive_nand);
				}
			}
		}

		// nand nand2 �h���C�u�A���}�E���g
		FATFS_UnmountDrive( drive_nand_path );
		FATFS_UnmountDrive( drive_nand2_path );

		// NAND�̃p�[�e�B�V�������w��
        // sizeInMB : �p�[�e�B�V�����T�C�Y�����K�o�C�g�P�ʂŊi�[�����z��
        // partitions : �p�[�e�B�V��������
        if (FATFSi_SetNANDPartitions(partition_MB_size, nand_fat_partition_num))
        {
            // �}�E���g
            if (FATFS_MountDrive(drive_nand_path, FATFS_MEDIA_TYPE_NAND, 0))
            {
				// �f�t�H���g�h���C�u�ݒ�
                if (!FATFS_SetDefaultDrive(drive_nand_path))
                {
                    return FALSE;
                }
				// �w��̃p�X���w���h���C�u���܂ރ��f�B�A�S�̂�������
                else if(!FATFS_FormatMedia(drive_nand_path))
                {
                    return FALSE;
                }
				// �p�[�e�B�V���������w��o�b�t�@�̓��e�Ńt�B������
                else if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( 0, init_datbuf, 16))
                {
                    return FALSE;
                }
				// �w��̃p�X���w���h���C�u�S�̂�������
                else if(!FATFS_FormatDrive(drive_nand_path))
                {
                    return FALSE;
                }
                else
                {
					// FAT1�p�[�e�B�V�����̏�����
                    for (i = 1; i < nand_fat_partition_num; ++i)
                    {
                        if(!FATFS_MountDrive(drive_nand2_path, FATFS_MEDIA_TYPE_NAND, (u32)i))
                        {
							return FALSE;
                        }
                    }
                    for (i = 1; i < nand_fat_partition_num; ++i)
                    {
                        if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( i, init_datbuf, 16))
                        {
                            return FALSE;
                        }
                        else if (!FATFS_FormatDrive(drive_nand2_path))
                        {
							return FALSE;
                        }
                    }
                }
            }
        }
    }

	// ���������
	OS_Free( init_datbuf );

	// �f�B���N�g���������`�F�b�N
	if (!CreateDirectory( "nand:", s_pDirList0 )) { return FALSE; }
	if (!CheckDirectory ( "nand:", s_pDirList0 )) { return FALSE; }
	if (!CreateDirectory( "nand2:", s_pDirList1 )) { return FALSE; }
	if (!CheckDirectory ( "nand2:", s_pDirList1 )) { return FALSE; }
	
	// �t�@�C���������`�F�b�N
//	if (!CreateFile( &s_fileList[0] )) { return FALSE; }
//	if (!CheckFile ( &s_fileList[0] )) { return FALSE; }

	// ����
	return TRUE;
}


// �f�B���N�g���쐬
static BOOL CreateDirectory( const char *pDrive, const char **ppDirList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCreate directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        ERROR_RETURN();
	}

	// �w�肳�ꂽ�f�B���N�g�������[�g�ɍ쐬
	while( *ppDirList ) {
		OS_TPrintf( "  %s...", *ppDirList );
		if( !FATFS_CreateDirectory( *ppDirList, "rwxrwxrwx") ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		OS_TPrintf( "ok.\n" );
		ppDirList++;
	}

	return TRUE;
}


// �f�B���N�g�����݃`�F�b�N
static BOOL CheckDirectory( const char *pDrive, const char **ppDirList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCheck directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        ERROR_RETURN();
	}
	
	// �w�肳�ꂽ�f�B���N�g�����`�F�b�N
	while( *ppDirList ) {
	    FATFSDirectoryHandle dir = FATFS_OpenDirectory( *ppDirList, "rw");
		OS_TPrintf( "  %s...", *ppDirList );
		if( dir ) {
			OS_TPrintf( "ok.\n" );
    	    (void)FATFS_CloseDirectory( dir );
		}else {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		ppDirList++;
	}

	return TRUE;
}


// �t�@�C���쐬
static BOOL CreateFile( const FileProperty *pFileList )
{
	const FileProperty *pTemp = pFileList;
	u32 length = 0;
	u8 *pBuffer;
	
	// �t�@�C�����X�g���̍ő�T�C�Y�̃o�b�t�@���m�ۂ��A"0"����
	while( pTemp->path ) {
		if( length < pTemp->length ) {
			length = pTemp->length;
		}
		pTemp++;
	}
	pBuffer = OS_Alloc( length );
	if( pBuffer == NULL ) {
		OS_TPrintf( "memory allocate error.\n" );
		ERROR_RETURN();
	}
	MI_CpuClearFast( pBuffer, length );
	
	OS_TPrintf( "\nCreate File :\n" );
	
	// �w�肳�ꂽ�t�@�C�����쐬���āA"0"����
	while( pFileList->path ) {
		FATFSFileHandle file;
		
		OS_TPrintf( "  %s, %dbytes...", pFileList->path, pFileList->length );
		// �t�@�C������
		if( !FATFS_CreateFile( pFileList->path, TRUE, "rwxrwxrwx" ) ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// �t�@�C���I�[�v��
		file = FATFS_OpenFile( pFileList->path, "w" );
		if( !file ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// �t�@�C�����ύX
		if( !FATFS_SetFileLength( file, (int)pFileList->length ) ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// �t�@�C����������
		if( !FATFS_WriteFile( file, pBuffer, (int)pFileList->length ) ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// �t�@�C���N���[�Y
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}
	// ���������
	OS_Free( pBuffer );

	return TRUE;
}


// �t�@�C���`�F�b�N
static BOOL CheckFile( const FileProperty *pFileList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCheck File :\n" );
	
	// �w�肳�ꂽ�f�B���N�g�������[�g�ɍ쐬
	while( pFileList->path ) {
		FATFSFileHandle file;
		u32 *pBuffer;
		int i;
		OSTick start;
		
		OS_TPrintf( "  %s, %dbytes...", pFileList->path, pFileList->length );
		// �t�@�C���I�[�v��
		file = FATFS_OpenFile( pFileList->path, "r+" );
		if( !file ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// �t�@�C�����`�F�b�N
		if( FATFS_GetFileLength( file ) != pFileList->length ) {
			OS_TPrintf( "ng. length = %d\n", FATFS_GetFileLength( file )  );
			ERROR_RETURN();
		}
		// �o�b�t�@ �������m��
		pBuffer = OS_Alloc( pFileList->length );
		if( pBuffer == NULL ) {
			OS_TPrintf( "memory allocate error.\n" );
			ERROR_RETURN();
		}
		start = OS_GetTick();
		// �t�@�C���ǂݍ���
		if(
#ifdef NAND_SEPARATE_READ
			MY_ReadFile( file, pBuffer, (int)pFileList->length )
#else
			FATFS_ReadFile( file, pBuffer, (int)pFileList->length )
#endif
			!= pFileList->length ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		OS_TPrintf( " [ReadTime : %dms] ", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
		start = OS_GetTick();
		// �t�@�C���x���t�@�C
		for( i = 0; i < pFileList->length / sizeof(u32); i++ ) {
			if( pBuffer[ i ] != 0 ) {
				OS_TPrintf( "ng.\n" );
				ERROR_RETURN();
			}
		}
		OS_TPrintf( " [VerifyTime : %dms] ", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
		// ���������
		OS_Free( pBuffer );
		// �t�@�C���N���[�Y
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}

	return TRUE;
}

#ifdef NAND_SEPARATE_READ
// NAND�s���ARM7����FS�͋z�������ꂢ�Ă��Ȃ��̂ŁA���O��2KB�P�ʂɕ������ă��[�h
static int MY_ReadFile( FATFSFileHandle file, void *pBuffer, int length )
{
	int length_bak = length;
	while( length ) {
		int rdLength = ( length > FS_READ_BLOCK_SIZE ) ? FS_READ_BLOCK_SIZE : length;
		
		if( FATFS_ReadFile( file, pBuffer, rdLength ) != rdLength ) {
			return -1;
		}
		pBuffer  = (u8 *)pBuffer + rdLength;
		length  -= rdLength;
	}
	return length_bak;
}
#endif
