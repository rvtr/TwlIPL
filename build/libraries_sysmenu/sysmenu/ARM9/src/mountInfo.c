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
void SYSMi_SetLauncherMountInfo( void );
void SYSM_SetBootAppMountInfo( NAMTitleId titleID );

static void SYSMi_ModifySaveDataMount( NAMTitleId titleID );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static void SYSMi_SetBootSRLPath( NAMTitleId titleID );
static void SYSMi_SetMountInfoCore( const OSMountInfo *pSrc );

// const data------------------------------------------------------------------

/*

	※現在、SDKのFATFSバグ回避のため、突貫で"nand:"を"F:"ドライブにしている。
	（NAND2KBリード問題対策が、"F"ドライブのみでの対応になっているため。）

*/


// デフォルトマウント情報リスト
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
//	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R),                0, 0, "nand",    "/" },	// ユーザーはこのアーカイブではWrite不可
//	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R),                0, 0, "nand2",   "/" },	// ユーザーはこのアーカイブではWrite不可
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ユーザーはこのアーカイブではWrite不可
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ユーザーはこのアーカイブではWrite不可
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
	{ 'H', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
};

// ランチャーマウント情報
const OSMountInfo s_launcherMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ランチャーはここもアクセス可
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// 同上
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{   0, OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
	{   0, OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
};



// ============================================================================
//
// マウント情報セット
//
// ============================================================================

// ランチャーのマウント情報セット
void SYSMi_SetLauncherMountInfo( void )
{
	// ※とりあえず自身はROMブートで。（後で修正）
//	SYSMi_SetBootSRLPath( 0 );						// ※SDK2623では、BootSRLPathを"rom:"としたらFSi_InitRomArchiveでNANDアプリ扱いされてアクセス例外で落ちる。
	SYSMi_SetMountInfoCore( &s_launcherMountList[0] );
}


// システム領域に、ブートするアプリのマウント情報を登録する
void SYSM_SetBootAppMountInfo( NAMTitleId titleID )
{
	SYSMi_SetBootSRLPath( titleID );				// 起動アプリのSRLパスをセット
	SYSMi_ModifySaveDataMount( titleID );			// セーブデータ有無によるマウント情報の編集
	/*
		※※　注意　※※
		MountInfoは、FSで直接参照してアクセス許可状態を判定しているため、ここにアプリ用のデータをセットすると、
		その後はパーミッションの都合上FSライブラリおよびFSを使用したESやNAMライブラリが全く使用できなくなる。（今後パーミッション仕様については変更される可能性あり）
		よって、内部でFSライブラリを使用する処理は、本処理の前に完了しておく必要がある。
	*/
	SYSMi_SetMountInfoCore( (const OSMountInfo *)&s_defaultMountList[0] );	// マウント情報のセット
}


// 起動SRLパスをシステム領域にセット
static void SYSMi_SetBootSRLPath( NAMTitleId titleID )
{
	static char path[ FS_FILE_NAME_MAX ];
	
	MI_CpuClear8( path, FS_FILE_NAME_MAX );
	
	// タイトルIDが"0"の時は、ROMと判断する（DSダウンロードプレイの時の挙動は未実装。。。）
	if( titleID ) {
		if( NAM_GetTitleBootContentPath( path, titleID ) != NAM_OK ) {
			OS_TPrintf( "ERROR: BootContentPath Get failed.\n" );
		}
	}else {
//		STD_StrCpy( path, (const char*)"rom:" );		// ※SDK2623では、BootSRLPathを"rom:"としたらFSi_InitRomArchiveでNANDアプリ扱いされてアクセス例外で落ちる。
	}
	
	STD_CopyLStringZeroFill( (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, path, OS_MOUNT_PATH_LEN );
//	OS_TPrintf( "boot path : %s\n", (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF );	// ※今はOS_Init前で呼ばれるので、Printfできない。
}


// マウント情報をシステム領域に書き込み
static void SYSMi_SetMountInfoCore( const OSMountInfo *pSrc )
{
	OSMountInfo *pDst   = (OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
	int i;
	
	MI_CpuClearFast( (void *)HW_TWL_FS_MOUNT_INFO_BUF, HW_TWL_FS_BOOT_SRL_PATH_BUF - HW_TWL_FS_MOUNT_INFO_BUF );
	
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


/*
static void SYSMi_ModifySaveDataMount2( NAMTitleId titleID, ROM_Header_Short *pROMH )
{
	int i;
	OSMountInfo *pMountTgt = &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ];
	u32 *pROMHSaveDataSize = &pROMH->public_save_data_size;
	
	if( titleID ) {
		// タイトルIDが指定されているNANDアプリの場合は、セーブデータ有無を判定して、パスをセット
		char saveFilePath[ 2 ][ FS_FILE_NAME_MAX ];
		
		// セーブデータのファイルパスを取得
		NAM_GetTitleSaveFilePath( saveFilePath[ 1 ], saveFilePath[ 0 ], titleID );
		
		// 結果を元にマウント情報を編集。
		for( i = 0; i < 2; i++ ) {
			BOOL isFind = FALSE;
			// ROMヘッダにセーブデータサイズの記載があるなら
			if( *pROMHSaveDataSize++ ) {
				FSFile  file[1];
				FS_InitFile( file );
				// セーブファイルを開けるならOK。
				if( FS_OpenFileEx( file, saveFilePath[ i ], FS_FILEMODE_R) ) {
					FS_CloseFile( file );
					isFind = TRUE;
				}
				// ※ランチャーでセーブデータファイルのリカバリまでやる？
#if 0
				else if( FS_CreateFile( saveFilePath[ i ], FS_PERMIT_R | FS_PERMIT_W ) &&
						 FS_SetFileLength( file, *pROMHSaveDataSize )  ) {
					FS_CloseFile( file );
					isFind = TRUE;
				}
#endif
			}
			
			if( isFind ) {
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
*/
