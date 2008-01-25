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
  $Rev:$
  $Author:$
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

// ここを編集したら、SDKのFATFS_Init内の同パラメータも修正する必要がある。
#define NAND_SIZE				245			// 256MB mobiNANDでの使用可能サイズ（iNANDでは違う値になる。未定。）
#define PARTITION_RAW_SIZE		4
#define PARTITION_0_SIZE		213
#define PARTITION_1_SIZE		( NAND_SIZE - PARTITION_RAW_SIZE - PARTITION_0_SIZE )
#define NAND_FAT_PARTITION_NUM	2			// FATパーティション数（RAWパーティションを除く）
		

// const data--------------------------------------------------------
//#define NAND_SEPARATE_READ
#define FS_READ_BLOCK_SIZE			(  2 * 1024 )
#define FATFS_CLUSTER_SIZE			( 16 * 1024 )
// FATFSのクラスタサイズは16KBなので、データサイズが決まっていないものは、余裕を持たせて16KBにしておく
static const FileProperty s_fileList[] = {
	{  128,                "F:/sys/ID.sgn"           },	// 現状、全部サイズは適当。中身も空。
	{  FATFS_CLUSTER_SIZE, "F:/sys/HWINFO.dat"       },
	{  FATFS_CLUSTER_SIZE, "F:/shared1/TWLCFG0.dat"  },
	{  FATFS_CLUSTER_SIZE, "F:/shared1/TWLCFG1.dat"  },	// ミラー
//	{  FATFS_CLUSTER_SIZE, "F:/shared1/WIFICFG0.dat" },
//	{  FATFS_CLUSTER_SIZE, "F:/shared1/WIFICFG1.dat" },	// ミラー
	{  0, NULL },
};

static const char *s_pDirList0[] = {
	(const char *)"sys",
	(const char *)"title",
	(const char *)"ticket",
	(const char *)"shared1",
	(const char *)"import",
	(const char *)"tmp",
	NULL,
	};

static const char *s_pDirList1[] = {
	(const char *)"photo",
	(const char *)"shared2",
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
    int     nand_fat_partition_num;

	init_datbuf = OS_Alloc( 512*16 );
	if( init_datbuf == NULL ) {
		OS_TPrintf( "memory allocate error.\n" );
		ERROR_RETURN();
	}

    MI_CpuFill8( init_datbuf, 0xFF, 512*16);

    // NANDをフォーマット
    {
        int     i;

        /* パーティションサイズをプロンプトから設定 */
        u32     partition_MB_size[5];
        partition_MB_size[0] = PARTITION_RAW_SIZE;		// RAW領域
        partition_MB_size[1] = PARTITION_0_SIZE;		// FAT0領域
        partition_MB_size[2] = PARTITION_1_SIZE;		// FAT1領域
        nand_fat_partition_num = NAND_FAT_PARTITION_NUM;

		// F G ドライブアンマウント
        for( i=0; i<nand_fat_partition_num; i++) {
            char    drive[4];
            STD_TSPrintf( drive, "%c:", 'F'+i);
            FATFS_UnmountDrive( drive);
        }

		// NANDのパーティションを指定
        // sizeInMB : パーティションサイズをメガバイト単位で格納した配列
        // partitions : パーティション総数
        if (FATFSi_SetNANDPartitions(partition_MB_size, nand_fat_partition_num))
        {
            // マウント
            if (FATFS_MountDrive("F", FATFS_MEDIA_TYPE_NAND, 0))
            {
                const char     *path = "F:";        // "F:"がFAT0パーティションになる。

				// デフォルトドライブ設定
                if (!FATFS_SetDefaultDrive(path))
                {
                    return FALSE;
                }
				// 指定のパスが指すドライブを含むメディア全体を初期化
                else if(!FATFS_FormatMedia(path))
                {
                    return FALSE;
                }
				// パーティション内を指定バッファの内容でフィルする
                else if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( 0, init_datbuf, 16))
                {
                    return FALSE;
                }
				// 指定のパスが指すドライブ全体を初期化
                else if(!FATFS_FormatDrive(path))
                {
                    return FALSE;
                }
                else
                {
					// F以降のドライブの初期化
                    for (i = 1; i < nand_fat_partition_num; ++i)
                    {
                        char    drive[4];
                        STD_TSPrintf(drive, "%c:", 'F' + i);
                        if(!FATFS_MountDrive(drive, FATFS_MEDIA_TYPE_NAND, (u32)i))
                        {
							return FALSE;
                        }
                    }
                    for (i = 1; i < nand_fat_partition_num; ++i)
                    {
                        char    drive[4];
                        STD_TSPrintf(drive, "%c:", 'F' + i);
                        if (format_mode == FORMAT_MODE_FULL && !FATFSi_nandFillPartition( i, init_datbuf, 16))
                        {
                            return FALSE;
                        }
                        else if (!FATFS_FormatDrive(drive))
                        {
							return FALSE;
                        }
                    }
                }
            }
        }
    }

	// メモリ解放
	OS_Free( init_datbuf );

	// ディレクトリ生成＆チェック
	if (!CreateDirectory( "F:", s_pDirList0 )) { return FALSE; }
	if (!CheckDirectory ( "F:", s_pDirList0 )) { return FALSE; }
	if (!CreateDirectory( "G:", s_pDirList1 )) { return FALSE; }
	if (!CheckDirectory ( "G:", s_pDirList1 )) { return FALSE; }
	
	// ファイル生成＆チェック
	if (!CreateFile( &s_fileList[0] )) { return FALSE; }
	if (!CheckFile ( &s_fileList[0] )) { return FALSE; }

	// F,Gドライブアンマウント
    {
        int i;
        for( i=0; i<nand_fat_partition_num; i++) {
            char    drive[4];
            STD_TSPrintf( drive, "%c:", 'F'+i);
            FATFS_UnmountDrive( drive);
        }
    }

	// デフォルトマウント状態に戻しておく
	FATFS_MountDrive("F", FATFS_MEDIA_TYPE_NAND, 0);
	FATFS_MountDrive("G", FATFS_MEDIA_TYPE_SD, 0);

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
	pBuffer = OS_Alloc( length );
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
		if( !FATFS_CreateFile( pFileList->path, TRUE, "rwxrwxrwx" ) ) {
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
	OS_Free( pBuffer );

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
		OSTick start;
		
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
		pBuffer = OS_Alloc( pFileList->length );
		if( pBuffer == NULL ) {
			OS_TPrintf( "memory allocate error.\n" );
			ERROR_RETURN();
		}
		start = OS_GetTick();
		// ファイル読み込み
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
		// ファイルベリファイ
		for( i = 0; i < pFileList->length / sizeof(u32); i++ ) {
			if( pBuffer[ i ] != 0 ) {
				OS_TPrintf( "ng.\n" );
				ERROR_RETURN();
			}
		}
		OS_TPrintf( " [VerifyTime : %dms] ", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
		// メモリ解放
		OS_Free( pBuffer );
		// ファイルクローズ
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}

	return TRUE;
}

#ifdef NAND_SEPARATE_READ
// NAND不具合をARM7側のFSは吸収しきれいていないので、自前で2KB単位に分割してリード
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
