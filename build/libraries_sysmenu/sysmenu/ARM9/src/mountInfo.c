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
#define DEFAULT_MOUNT_LIST_NUM				7
#define PRV_SAVE_DATA_MOUNT_INDEX			5			// プライベートセーブデータの s_defaultMountInfo リストインデックス
#define PUB_SAVE_DATA_MOUNT_INDEX			6			// パブリック　セーブデータの s_defaultMountInfo リストインデックス

#define TITLEID_HI_APP_SYS_FLAG_SHIFT		0
#define TITLEID_HI_NOT_LAUNCH_FLAG_SHIFT	1
#define TITLEID_HI_MEDIA_NAND_FLAG_SHIFT	2
#define TITLEID_HI_APP_SYS_FLAG				( 1 << TITLEID_HI_APP_SYS_FLAG_SHIFT )
#define TITLEID_HI_NOT_LAUNCH_FLAG			( 1 << TITLEID_HI_NOT_LAUNCH_FLAG_SHIFT )
#define TITLEID_HI_MEDIA_NAND_FLAG			( 1 << TITLEID_HI_MEDIA_NAND_FLAG_SHIFT )


// extern data-----------------------------------------------------------------

// function's prototype--------------------------------------------------------
void SYSMi_SetLauncherMountInfo( void );
void SYSM_SetBootAppMountInfo( TitleProperty *pBootTitle );

static void SYSMi_SetBootSRLPath( NAMTitleId titleID, TitleMedia bootMedia );
static void SYSMi_SetMountInfoCore( const OSMountInfo *pSrc );
static void SYSMi_ModifySaveDataMount( NAMTitleId titleID, OSMountInfo *pMountTgt );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// デフォルトマウント情報リスト
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
	{ 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ユーザーアプリはこのアーカイブではWrite不可
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ユーザーアプリはこのアーカイブではWrite不可
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },	// NANDにセーブデータがないアプリの場合は、マウントされない。
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },	// NANDにセーブデータがないアプリの場合は、マウントされない。
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
	// ※とりあえず自身はROMブートで。（後で修正）
//	SYSMi_SetBootSRLPath(  );						// ※SDK2623では、BootSRLPathを"rom:"としたらFSi_InitRomArchiveでNANDアプリ扱いされてアクセス例外で落ちる。
	SYSMi_SetMountInfoCore( &s_defaultMountList[0] );
}


// システム領域に、ブートするアプリのマウント情報を登録する
void SYSM_SetBootAppMountInfo( TitleProperty *pBootTitle )
{
	u32 titleID_Hi = (u32)( pBootTitle->titleID >> 32 );		// u64で論理演算はできない？
	// 起動アプリのSRLパスをセット
	SYSMi_SetBootSRLPath( pBootTitle->titleID, (TitleMedia)pBootTitle->flags.media );
	
	// ユーザーアプリの場合、"nand:", "nand2:"アーカイブを変更。
	if( ( titleID_Hi & TITLEID_HI_APP_SYS_FLAG ) == 0 ) {
		s_defaultMountList[ 1 ].userPermission = 0;	// "nand:"
		s_defaultMountList[ 2 ].userPermission = 0;	// "nand2:"
	}
	
	// セーブデータ有無によるマウント情報の編集
	SYSMi_ModifySaveDataMount( pBootTitle->titleID, &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
	
	/*
		※※　注意　※※
		MountInfoは、FSで直接参照してアクセス許可状態を判定しているため、ここにアプリ用のデータをセットすると、
		その後はパーミッションの都合上FSライブラリおよびFSを使用したESやNAMライブラリが全く使用できなくなる。（今後パーミッション仕様については変更される可能性あり）
		よって、内部でFSライブラリを使用する処理は、本処理の前に完了しておく必要がある。
	*/
	SYSMi_SetMountInfoCore( (const OSMountInfo *)&s_defaultMountList[0] );	// マウント情報のセット
}

// 起動SRLパスをシステム領域にセット
static void SYSMi_SetBootSRLPath( NAMTitleId titleID, TitleMedia bootMedia )
{
	static char path[ FS_ENTRY_LONGNAME_MAX ];
	MI_CpuClear8( path, FS_ENTRY_LONGNAME_MAX );
	
	if( bootMedia == TITLE_MEDIA_NAND ) {
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
static void SYSMi_ModifySaveDataMount( NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	u32 titleID_Hi = (u32)( titleID >> 32 );		// u64で論理演算はできない？
	
	// ※カードからブートされた場合でも、titleIDが"NANDアプリ"の場合は、セーブデータをマウントするようにしている。
	
	// タイトルIDが"NANDアプリ"の場合は、セーブデータ有無を判定して、パスをセット
	if( titleID_Hi & TITLEID_HI_MEDIA_NAND_FLAG ) {
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

