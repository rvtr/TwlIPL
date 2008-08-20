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

//------- �d�v --------
#define PARTITION_RAW_SIZE		1024		// 1*1024
#define PARTITION_0_SIZE		210944		// 206*1024
#define PARTITION_1_SIZE		33536		// 32.75*1024
#define NAND_FAT_PARTITION_NUM	3			// FAT�p�[�e�B�V�������iRAW�p�[�e�B�V�����������j
											// ���ۂɂ� PARTITION 0&1 ��2�����ł���3���w�肵�܂��B
											// �ŏIPARTISION�̃T�C�Y�w��͖�������c��T�C�Y���K�p����邽�߂ł��B
											// PARTISION2 �ɂ͎c��T�C�Y���K�p����܂����g�p���Ȃ�����
											// �t�H�[�}�b�g�͍s���܂���B

// const data--------------------------------------------------------
#define FS_READ_BLOCK_SIZE			(  2 * 1024 )
#define FATFS_CLUSTER_SIZE			( 16 * 1024 )

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
	const int INIT_DATA_BUFFER_SIZE = 512*16;
    int     nand_fat_partition_num;

	init_datbuf = OS_AllocFromSubPrivWram( INIT_DATA_BUFFER_SIZE );
	if( init_datbuf == NULL ) {
		OS_TPrintf( "memory allocate error.\n" );
		ERROR_RETURN();
	}

    MI_CpuFill8( init_datbuf, 0xFF, INIT_DATA_BUFFER_SIZE);

    // NAND���t�H�[�}�b�g
    {
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

		// nand&nand2�̃h���C�u���蓖�Ă𒲂ׂ�
	    {
	        const OSMountInfo  *info;

	        // �����`���[����N�����Ă��Ȃ��ꍇ��shared�̈���Q�ƁB
	        if (*(const u8 *)HW_TWL_RED_LAUNCHER_VER == 0)
			{
				info = OS_GetMountInfo();
			}
	        // �����V�����`���[�ֈڍs���Ă���Ȃ炻������Q�ƁB
			else
			{
	            extern const u8 SDK_MOUNT_INFO_TABLE[];
	            info = (const OSMountInfo *)SDK_MOUNT_INFO_TABLE;
			}

	        for (; *info->drive; ++info)
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
        if (FATFSi_SetNANDPartitionsKiroBytes(partition_MB_size, nand_fat_partition_num))
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
                else if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( 0, init_datbuf, INIT_DATA_BUFFER_SIZE))
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
                    if(!FATFS_MountDrive(drive_nand2_path, FATFS_MEDIA_TYPE_NAND, (u32)1))
                    {
						return FALSE;
                    }
                    if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( 1, init_datbuf, INIT_DATA_BUFFER_SIZE))
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

	// ���������
	OS_FreeToSubPrivWram( init_datbuf );

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
	pBuffer = OS_AllocFromSubPrivWram( length );
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
		if( !FATFS_CreateFile( pFileList->path, TRUE, "rw\0rw\0rw\0" ) ) {
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
	OS_FreeToSubPrivWram( pBuffer );

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
		pBuffer = OS_AllocFromSubPrivWram( pFileList->length );
		if( pBuffer == NULL ) {
			OS_TPrintf( "memory allocate error.\n" );
			ERROR_RETURN();
		}
		// �t�@�C���ǂݍ���
		if(
			FATFS_ReadFile( file, pBuffer, (int)pFileList->length )
			!= pFileList->length ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// �t�@�C���x���t�@�C
		for( i = 0; i < pFileList->length / sizeof(u32); i++ ) {
			if( pBuffer[ i ] != 0 ) {
				OS_TPrintf( "ng.\n" );
				ERROR_RETURN();
			}
		}
		// ���������
		OS_FreeToSubPrivWram( pBuffer );
		// �t�@�C���N���[�Y
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}

	return TRUE;
}
