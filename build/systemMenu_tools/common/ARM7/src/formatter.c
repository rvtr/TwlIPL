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
    型定義
 *---------------------------------------------------------------------------*/

typedef struct FileProperty {
	u32			length;
	const char *path;
}FileProperty;

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define ERROR_RETURN()  { OS_TPrintf("FATAL ERROR OCCURED %s %s\n", __FILE__, __LINE__); return 0; }

//------- 重要 --------
#define PARTITION_RAW_SIZE		1024		// 1*1024
#define PARTITION_0_SIZE		210944		// 206*1024
#define PARTITION_1_SIZE		33536		// 32.75*1024
#define NAND_FAT_PARTITION_NUM	3			// FATパーティション数（RAWパーティションを除く）
											// 実際には PARTITION 0&1 の2つだけですが3を指定します。
											// 最終PARTISIONのサイズ指定は無視され残りサイズが適用されるためです。
											// PARTISION2 には残りサイズが適用されますが使用しないため
											// フォーマットは行いません。

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
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL CreateDirectory( const char *pDrive, const char **ppDirList );
static BOOL CheckDirectory ( const char *pDrive, const char **ppDirList );
static BOOL CreateFile( const FileProperty *pFileList );
static BOOL CheckFile ( const FileProperty *pFileList );
static int  MY_ReadFile( FATFSFileHandle file, void *pBuffer, int length );

/*---------------------------------------------------------------------------*
  Name:         ExeFormat

  Description:  Nandのフォーマットを行います

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

    // NANDをフォーマット
    {
		u8 drive_nand;
		u8 drive_nand2;
		char drive_nand_path[4];
		char drive_nand2_path[4];

        /* パーティションサイズをプロンプトから設定 */
        u32     partition_MB_size[5];
        partition_MB_size[0] = PARTITION_RAW_SIZE;		// RAW領域
        partition_MB_size[1] = PARTITION_0_SIZE;		// FAT0領域
        partition_MB_size[2] = PARTITION_1_SIZE;		// FAT1領域
        nand_fat_partition_num = NAND_FAT_PARTITION_NUM;

		// nand&nand2のドライブ割り当てを調べる
	    {
	        const OSMountInfo  *info;

	        // ランチャーから起動していない場合はshared領域を参照。
	        if (*(const u8 *)HW_TWL_RED_LAUNCHER_VER == 0)
			{
				info = OS_GetMountInfo();
			}
	        // 環境が新ランチャーへ移行しているならそちらを参照。
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

		// nand nand2 ドライブアンマウント
		FATFS_UnmountDrive( drive_nand_path );
		FATFS_UnmountDrive( drive_nand2_path );

		// NANDのパーティションを指定
        // sizeInMB : パーティションサイズをメガバイト単位で格納した配列
        // partitions : パーティション総数
        if (FATFSi_SetNANDPartitionsKiroBytes(partition_MB_size, nand_fat_partition_num))
        {
            // マウント
            if (FATFS_MountDrive(drive_nand_path, FATFS_MEDIA_TYPE_NAND, 0))
            {
				// デフォルトドライブ設定
                if (!FATFS_SetDefaultDrive(drive_nand_path))
                {
                    return FALSE;
                }
				// 指定のパスが指すドライブを含むメディア全体を初期化
                else if(!FATFS_FormatMedia(drive_nand_path))
                {
                    return FALSE;
                }
				// パーティション内を指定バッファの内容でフィルする
                else if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( 0, init_datbuf, INIT_DATA_BUFFER_SIZE))
                {
                    return FALSE;
                }
				// 指定のパスが指すドライブ全体を初期化
                else if(!FATFS_FormatDrive(drive_nand_path))
                {
                    return FALSE;
                }
                else
                {
					// FAT1パーティションの初期化
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

	// メモリ解放
	OS_FreeToSubPrivWram( init_datbuf );

	// ディレクトリ生成＆チェック
	if (!CreateDirectory( "nand:", s_pDirList0 )) { return FALSE; }
	if (!CheckDirectory ( "nand:", s_pDirList0 )) { return FALSE; }
	if (!CreateDirectory( "nand2:", s_pDirList1 )) { return FALSE; }
	if (!CheckDirectory ( "nand2:", s_pDirList1 )) { return FALSE; }
	
	// ファイル生成＆チェック
//	if (!CreateFile( &s_fileList[0] )) { return FALSE; }
//	if (!CheckFile ( &s_fileList[0] )) { return FALSE; }

	// 成功
	return TRUE;
}


// ディレクトリ作成
static BOOL CreateDirectory( const char *pDrive, const char **ppDirList )
{
	// デフォルトドライブの指定
	OS_TPrintf( "\nCreate directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        ERROR_RETURN();
	}

	// 指定されたディレクトリをルートに作成
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


// ディレクトリ存在チェック
static BOOL CheckDirectory( const char *pDrive, const char **ppDirList )
{
	// デフォルトドライブの指定
	OS_TPrintf( "\nCheck directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        ERROR_RETURN();
	}
	
	// 指定されたディレクトリをチェック
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


// ファイル作成
static BOOL CreateFile( const FileProperty *pFileList )
{
	const FileProperty *pTemp = pFileList;
	u32 length = 0;
	u8 *pBuffer;
	
	// ファイルリスト中の最大サイズのバッファを確保し、"0"埋め
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
	
	// 指定されたファイルを作成して、"0"埋め
	while( pFileList->path ) {
		FATFSFileHandle file;
		
		OS_TPrintf( "  %s, %dbytes...", pFileList->path, pFileList->length );
		// ファイル生成
		if( !FATFS_CreateFile( pFileList->path, TRUE, "rw\0rw\0rw\0" ) ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// ファイルオープン
		file = FATFS_OpenFile( pFileList->path, "w" );
		if( !file ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// ファイル長変更
		if( !FATFS_SetFileLength( file, (int)pFileList->length ) ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// ファイル書き込み
		if( !FATFS_WriteFile( file, pBuffer, (int)pFileList->length ) ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// ファイルクローズ
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}
	// メモリ解放
	OS_FreeToSubPrivWram( pBuffer );

	return TRUE;
}


// ファイルチェック
static BOOL CheckFile( const FileProperty *pFileList )
{
	// デフォルトドライブの指定
	OS_TPrintf( "\nCheck File :\n" );
	
	// 指定されたディレクトリをルートに作成
	while( pFileList->path ) {
		FATFSFileHandle file;
		u32 *pBuffer;
		int i;
		
		OS_TPrintf( "  %s, %dbytes...", pFileList->path, pFileList->length );
		// ファイルオープン
		file = FATFS_OpenFile( pFileList->path, "r+" );
		if( !file ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// ファイル長チェック
		if( FATFS_GetFileLength( file ) != pFileList->length ) {
			OS_TPrintf( "ng. length = %d\n", FATFS_GetFileLength( file )  );
			ERROR_RETURN();
		}
		// バッファ メモリ確保
		pBuffer = OS_AllocFromSubPrivWram( pFileList->length );
		if( pBuffer == NULL ) {
			OS_TPrintf( "memory allocate error.\n" );
			ERROR_RETURN();
		}
		// ファイル読み込み
		if(
			FATFS_ReadFile( file, pBuffer, (int)pFileList->length )
			!= pFileList->length ) {
			OS_TPrintf( "ng.\n" );
			ERROR_RETURN();
		}
		// ファイルベリファイ
		for( i = 0; i < pFileList->length / sizeof(u32); i++ ) {
			if( pBuffer[ i ] != 0 ) {
				OS_TPrintf( "ng.\n" );
				ERROR_RETURN();
			}
		}
		// メモリ解放
		OS_FreeToSubPrivWram( pBuffer );
		// ファイルクローズ
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}

	return TRUE;
}
