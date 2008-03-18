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
#include "internal_api.h"

// define data-----------------------------------------------------------------
#define DEFAULT_MOUNT_LIST_NUM				9
#define NAND_MOUNT_INDEX			0
#define NAND2_MOUNT_INDEX			1
#define CONTENT_MOUNT_INDEX			2
#define SHARED1_MOUNT_INDEX			3
#define PRV_SAVE_DATA_MOUNT_INDEX			6			// プライベートセーブデータの s_defaultMountInfo リストインデックス
#define PUB_SAVE_DATA_MOUNT_INDEX			7			// パブリック　セーブデータの s_defaultMountInfo リストインデックス

#define TITLEID_APP_SYS_FLAG_SHIFT		( 32 + 0 )
#define TITLEID_NOT_LAUNCH_FLAG_SHIFT	( 32 + 1 )
#define TITLEID_MEDIA_NAND_FLAG_SHIFT	( 32 + 2 )
#define TITLEID_APP_SYS_FLAG			( 1ULL << TITLEID_APP_SYS_FLAG_SHIFT )
#define TITLEID_NOT_LAUNCH_FLAG			( 1ULL << TITLEID_NOT_LAUNCH_FLAG_SHIFT )
#define TITLEID_MEDIA_NAND_FLAG			( 1ULL << TITLEID_MEDIA_NAND_FLAG_SHIFT )

// extern data-----------------------------------------------------------------
// function's prototype--------------------------------------------------------
static void SYSMi_SetBootSRLPath( LauncherBootType bootType, NAMTitleId titleID );
static void SYSMi_SetMountInfoCore( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc );
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );
static void SYSMi_ModifySaveDataMountForLauncher( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

// デフォルトマウント情報リスト
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ユーザーアプリはこのアーカイブではR/W不可
	{ 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ユーザーアプリはこのアーカイブではR/W不可
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R), 0, 0, "content", NULL },			// Write不可
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R), 0, 0, "shared1", "nand:/shared1" },	// Write不可
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand:/shared2" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  1, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },	// NANDにセーブデータがないアプリの場合は、マウントされない。
	{ 'H', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },	// NANDにセーブデータがないアプリの場合は、マウントされない。
	{ 'I', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
};


// ============================================================================
//
// マウント情報セット
//
// ============================================================================

/*
	要確認
	カードブート時のBootSRLPathは、"rom:"ではなく、""でいく。
	"nand:" と "nand1:"のuserPermissionは"OS_MOUNT_USR_R"で良いのか？
*/
// ランチャーのマウント情報セット
void SYSMi_SetLauncherMountInfo( void )
{
	NAMTitleId titleID = TITLE_ID_LAUNCHER;
	
	// ※とりあえず自身はROMブートで。[TODO:]後で修正
//	SYSMi_SetBootSRLPath( LAUNCHER_BOOTTYPE_NAND, titleID );		// ※SDK2623では、BootSRLPathを"rom:"としたらFSi_InitRomArchiveでNANDアプリ扱いされてアクセス例外で落ちる。
	
	// セーブデータ有無によるマウント情報の編集
	// ※このタイミングではFSは動かせないので、FSを使わない特別版で対応。
	SYSMi_ModifySaveDataMountForLauncher( LAUNCHER_BOOTTYPE_NAND,
										  titleID,
										  &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );

	// マウント情報のセット
	SYSMi_SetMountInfoCore( LAUNCHER_BOOTTYPE_NAND,
							titleID,
							&s_defaultMountList[0] );
}


// システム領域に、ブートするアプリのマウント情報を登録する
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle )
{
	// アプリがTWL対応でない場合は、何もセットせずにリターン
	if( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) == 0 ) {
		return;
	}
	
	// 起動アプリのSRLパスをセット
//	SYSMi_SetBootSRLPath( (LauncherBootType)pBootTitle->flags.bootType,
//						  pBootTitle->titleID );

	STD_CopyLStringZeroFill( (char *)((u32)((( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_mount_info_ram_address) + SYSM_MOUNT_INFO_SIZE),
							SYSMi_GetWork2()->bootContentPath, OS_MOUNT_PATH_LEN );
	
	// セーブデータ有無によるマウント情報の編集
	// ※ARM7ではNAMは動かせないので、NAMを使わないバージョンで対応。
	SYSMi_ModifySaveDataMountForLauncher( LAUNCHER_BOOTTYPE_NAND,
										  pBootTitle->titleID,
										  &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
/*
	// セーブデータ有無によるマウント情報の編集
	SYSMi_ModifySaveDataMount( (LauncherBootType)pBootTitle->flags.bootType,
							   pBootTitle->titleID,
							   &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
*/
	// マウント情報のセット
	SYSMi_SetMountInfoCore( (LauncherBootType)pBootTitle->flags.bootType,
							pBootTitle->titleID,
							&s_defaultMountList[0] );
	
	/*
		※※　注意　※※
		MountInfoは、FSで直接参照してアクセス許可状態を判定しているため、ここにアプリ用のデータをセットすると、
		その後はパーミッションの都合上FSライブラリおよびFSを使用したESやNAMライブラリが全く使用できなくなる。（今後パーミッション仕様については変更される可能性あり）
		よって、内部でFSライブラリを使用する処理は、本処理の前に完了しておく必要がある。
	*/
}

/*
// 起動SRLパスをシステム領域にセット
static void SYSMi_SetBootSRLPath( LauncherBootType bootType, NAMTitleId titleID  )
{
	static char path[ FS_ENTRY_LONGNAME_MAX ];
	ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	
	switch( bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		if( NAM_GetTitleBootContentPathFast( path, titleID ) != NAM_OK ) {
			OS_TPrintf( "ERROR: BootContentPath Get failed.\n" );
		}
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		STD_TSNPrintf( path, FS_ENTRY_LONGNAME_MAX, OS_TMP_APP_PATH, titleID );
		break;
	default:
		path[ 0 ] = 0;
//		STD_StrCpy( path, (const char*)"rom:" );		// ※SDK2623では、BootSRLPathを"rom:"としたらFSi_InitRomArchiveでNANDアプリ扱いされてアクセス例外で落ちる。
		break;
	}
	
	if( path[ 0 ] ) {
		STD_CopyLStringZeroFill( (char *)((u32)header->sub_mount_info_ram_address + SYSMi_MOUNT_INFO_SIZE), path, OS_MOUNT_PATH_LEN );
	}else {
		MI_CpuClearFast( (char *)((u32)header->sub_mount_info_ram_address + SYSMi_MOUNT_INFO_SIZE), OS_MOUNT_PATH_LEN );
	}
	OS_TPrintf( "boot SRL path : %s\n", (char *)((u32)header->sub_mount_info_ram_address + SYSMi_MOUNT_INFO_SIZE) );	// ※OS_Init前で呼ぶとPrintfできないので注意。
}
*/

// マウント情報をシステム領域に書き込み
static void SYSMi_SetMountInfoCore( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc )
{
#pragma unused(bootType)
	
	int i;
	ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	OSMountInfo *pDst   = (OSMountInfo *)header->sub_mount_info_ram_address;
	char contentpath[ FS_ENTRY_LONGNAME_MAX ];

	// タイトルIDからcontentのファイルパスをセット
	STD_TSNPrintf( contentpath, FS_ENTRY_LONGNAME_MAX,
				   "nand:/title/%08x/%08x/content", (u32)( titleID >> 32 ), titleID );
	STD_CopyLStringZeroFill( pSrc[CONTENT_MOUNT_INDEX].path, contentpath, OS_MOUNT_PATH_LEN );
	
	MI_CpuClearFast( (void *)pDst, SYSM_MOUNT_INFO_SIZE );
	
	// セキュアアプリでない場合、"nand:", "nand2:"アーカイブを変更。
	if( ( titleID & TITLE_ID_HI_SECURE_FLAG_MASK ) == 0 ) {
		pSrc[ NAND_MOUNT_INDEX ].userPermission = 0;	// "nand:"
		pSrc[ NAND2_MOUNT_INDEX ].userPermission = 0;	// "nand2:"
	}
	
	// セット
	for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
		if( pSrc->drive[ 0 ] ) {
			MI_CpuCopyFast( pSrc, pDst, sizeof(OSMountInfo) );
			pDst++;
		}
		pSrc++;
	}
	
#if 0
	pDst   = (OSMountInfo *)pDst;
	for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
		OS_TPrintf( "mount path : %s\n", pDst->path );
		pDst++;
	}
#endif
}


// タイトルIDをもとにセーブデータ有無を判定して、マウント情報を編集する。
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	
	// ※カードからブートされた場合でも、titleIDが"NANDアプリ"の場合は、セーブデータをマウントするようにしている。
	
	// セーブデータ有無を判定して、パスをセット
	if( ( ( bootType == LAUNCHER_BOOTTYPE_NAND ) &&				// NANDアプリがNANDからブートされた時
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == LAUNCHER_BOOTTYPE_ROM ) &&				// ISデバッガ上で、NANDアプリがROM からブートされた時
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) &&
		  ( SYSMi_GetWork()->flags.hotsw.isOnDebugger ) )
		) {
		char saveFilePath[ 2 ][ FS_ENTRY_LONGNAME_MAX ];
		u32 saveDataSize[ 2 ];
		saveDataSize[ 0 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->private_save_data_size;
		saveDataSize[ 1 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->public_save_data_size;
		
		// セーブデータのファイルパスを取得
		NAM_GetTitleSaveFilePath( saveFilePath[ 1 ], saveFilePath[ 0 ], titleID );
		
		// "ROMヘッダのNANDセーブファイルサイズ > 0" かつ そのファイルを開ける場合のみマウント情報を登録
		for( i = 0; i < 2; i++ ) {
			FSFile  file[1];
			FS_InitFile( file );
			if( saveDataSize[ i ] &&
				FS_OpenFileEx( file, saveFilePath[ i ], FS_FILEMODE_R) ) {
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


// タイトルIDをもとにセーブデータ有無を判定して、マウント情報を編集する。
static void SYSMi_ModifySaveDataMountForLauncher( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	
	// ※カードからブートされた場合でも、titleIDが"NANDアプリ"の場合は、セーブデータをマウントするようにしている。
	
	// セーブデータ有無を判定して、パスをセット
	if( ( ( bootType == LAUNCHER_BOOTTYPE_NAND ) &&			// NANDアプリがNANDからブートされた時
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == LAUNCHER_BOOTTYPE_ROM ) &&			// ISデバッガ上で、NANDアプリがROM からブートされた時
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) &&
		  ( SYSMi_GetWork()->flags.hotsw.isOnDebugger ) )
		) {
		char saveFilePath[ 2 ][ FS_ENTRY_LONGNAME_MAX ];
		u32 saveDataSize[ 2 ];
		saveDataSize[ 0 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->private_save_data_size;
		saveDataSize[ 1 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->public_save_data_size;
		
		// セーブデータのファイルパスを取得
		STD_TSNPrintf( saveFilePath[ 0 ], FS_ENTRY_LONGNAME_MAX,
					   "nand:/title/%08x/%08x/data/private.sav", (u32)( titleID >> 32 ), titleID );
		STD_TSNPrintf( saveFilePath[ 1 ], FS_ENTRY_LONGNAME_MAX,
					   "nand:/title/%08x/%08x/data/public.sav", (u32)( titleID >> 32 ), titleID );
		
		// "ROMヘッダのNANDセーブファイルサイズ > 0" かつ そのファイルを開ける場合のみマウント情報を登録
		for( i = 0; i < 2; i++ ) {
			if( saveDataSize[ i ] ) {
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

