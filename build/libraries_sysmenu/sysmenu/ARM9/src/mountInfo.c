/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include <twl/nam.h>
#include <sysmenu.h>

// define data-----------------------------------------------------------------
#define DEFAULT_MOUNT_LIST_NUM			7
#define PRV_SAVE_DATA_MOUNT_INDEX		5			// プライベートセーブデータの s_defaultMountInfo リストインデックス
#define PUB_SAVE_DATA_MOUNT_INDEX		6			// パブリック　セーブデータの s_defaultMountInfo リストインデックス

// extern data-----------------------------------------------------------------

// function's prototype--------------------------------------------------------
void SYSM_SetBootSRLPath( NAMTitleId titleID );
void SYSM_SetMountInfo( NAMTitleId titleID );

static void SYSMi_ModifySaveDataMount( NAMTitleId titleID );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// デフォルトマウント情報リスト
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
	{ 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, 0,                               0, 0, "nand",    "/" },	// ユーザーはこのアーカイブを使えない(RW不可)
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, 0,                               0, 0, "nand2",   "/" },	// ユーザーはこのアーカイブを使えない(RW不可)
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
};


// ============================================================================
//
// マウント情報セット
//
// ============================================================================

// 起動SRLパスをシステム領域にセット
void SYSM_SetBootSRLPath( NAMTitleId titleID )
{
	char path[ FS_FILE_NAME_MAX ];
	
	// タイトルIDが"0"の時は、ROMと判断する（DSダウンロードプレイの時の挙動は未実装。。。）
	if( titleID ) {
	    NAM_GetTitleBootContentPath( path, titleID );
	}else {
		STD_StrCpy( path, (const char*)"rom:" );
	}
	
	STD_CopyLStringZeroFill( (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, path, OS_MOUNT_PATH_LEN );
	OS_TPrintf( "boot path : %s\n", (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF );
}


// システム領域に、ブートするアプリのマウント情報を登録する
void SYSM_SetMountInfo( NAMTitleId titleID )
{
	// マウント情報の整理
	SYSMi_ModifySaveDataMount( titleID );	// セーブデータ
	// SDは全アプリ解放で良い？
	// PHOTOは全アプリに解放で良い？
	
	// マウント情報をシステム領域に書き込み
	{
		OSMountInfo *pSrc = s_defaultMountList;
		OSMountInfo *pDst   = (OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
		int i;
		for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
			if( pSrc->drive[ 0 ] ) {
				MI_CpuCopyFast( pSrc, pDst, sizeof(OSMountInfo) );
				pDst++;
			}
			pSrc++;
		}
#if 0
		pDst   = (OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
		for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
			OS_TPrintf( "mount path : %s\n", pDst->path );
			pDst++;
		}
#endif
	}
}

// タイトルIDをもとにセーブデータ有無を判定して、マウント情報を編集する。
static void SYSMi_ModifySaveDataMount( NAMTitleId titleID )
{
	int i;
	OSMountInfo *pMountTgt = &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ];
	
	if( titleID ) {
		// タイトルIDが指定されているNANDアプリの場合は、セーブデータ有無を判定して、パスをセット
		char saveFilePath[ 2 ][ FS_FILE_NAME_MAX ];
		
		// セーブデータのファイルパスを取得
		NAM_GetTitleSaveFilePath( saveFilePath[ 1 ], saveFilePath[ 0 ], titleID );
		
		// 結果を元にマウント情報を編集。
		for( i = 0; i < 2; i++ ) {
			FSFile  file[1];
			FS_InitFile( file );
			// ※現在は、セーブファイルを開けるかどうかでセーブファイル有無を確認。
			//   最終的にはTMDもしくはROMヘッダの値を参照。ROMヘッダの方が簡単で速いか？
			if( FS_OpenFileEx( file, saveFilePath[ i ], FS_FILEMODE_R) ) {
				FS_CloseFile( file );
				STD_CopyLStringZeroFill( pMountTgt->path, saveFilePath[ i ], OS_MOUNT_PATH_LEN );
			}else {
				pMountTgt->drive[ 0 ] = 0;
			}
			pMountTgt++;
		}
	}else {
		// タイトルID指定なしのカードアプリの場合は、セーブデータ無効
		for( i = 0; i < 2; i++ ) {
			pMountTgt->drive[ 0 ] = 0;
		}
	}
}
