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
#define DEFAULT_MOUNT_LIST_NUM				7
#define PRV_SAVE_DATA_MOUNT_INDEX			5			// プライベートセーブデータの s_defaultMountInfo リストインデックス
#define PUB_SAVE_DATA_MOUNT_INDEX			6			// パブリック　セーブデータの s_defaultMountInfo リストインデックス

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

void SYSMi_SetBootSRLPathToWork2( TitleProperty *pBootTitle )
{
	static char path[ FS_ENTRY_LONGNAME_MAX ];
	
	switch( pBootTitle->flags.bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		if( NAM_GetTitleBootContentPathFast( path, pBootTitle->titleID ) != NAM_OK ) {
			OS_TPrintf( "ERROR: BootContentPath Get failed.\n" );
		}
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		STD_TSNPrintf( path, FS_ENTRY_LONGNAME_MAX, OS_TMP_APP_PATH, pBootTitle->titleID );
		break;
	default:
		path[ 0 ] = 0;
//		STD_StrCpy( path, (const char*)"rom:" );		// ※SDK2623では、BootSRLPathを"rom:"としたらFSi_InitRomArchiveでNANDアプリ扱いされてアクセス例外で落ちる。
		break;
	}
	
	if( path[ 0 ] ) {
		STD_CopyLStringZeroFill( SYSMi_GetWork2()->bootContentPath, path, OS_MOUNT_PATH_LEN );
	}else {
		MI_CpuClearFast( SYSMi_GetWork2()->bootContentPath, OS_MOUNT_PATH_LEN );
	}
	OS_TPrintf( "boot SRL path : %s\n", SYSMi_GetWork2()->bootContentPath );	// ※OS_Init前で呼ぶとPrintfできないので注意。
}
