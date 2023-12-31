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
#include <twl/os/common/format_rom_scfg.h>
#include <sysmenu.h>
#include "internal_api.h"

//#define USE_SHARED2_FILE_OLD_FORMAT

#ifndef SDK_ARM9

// define data-----------------------------------------------------------------
#define DEFAULT_MOUNT_LIST_NUM				9
#define NAND_MOUNT_INDEX			0
#define NAND2_MOUNT_INDEX			1
#define CONTENT_MOUNT_INDEX			2
#define SHARED1_MOUNT_INDEX			3
#define SHARED2_MOUNT_INDEX			4
#define PRV_SAVE_DATA_MOUNT_INDEX			6			// プライベートセーブデータの s_defaultMountInfo リストインデックス
#define PUB_SAVE_DATA_MOUNT_INDEX			7			// パブリック　セーブデータの s_defaultMountInfo リストインデックス
#define SDMC_MOUNT_INDEX					8

#define TITLEID_APP_SYS_FLAG_SHIFT		( 32 + 0 )
#define TITLEID_NOT_LAUNCH_FLAG_SHIFT	( 32 + 1 )
#define TITLEID_MEDIA_NAND_FLAG_SHIFT	( 32 + 2 )
#define TITLEID_APP_SYS_FLAG			( 1ULL << TITLEID_APP_SYS_FLAG_SHIFT )
#define TITLEID_NOT_LAUNCH_FLAG			( 1ULL << TITLEID_NOT_LAUNCH_FLAG_SHIFT )
#define TITLEID_MEDIA_NAND_FLAG			( 1ULL << TITLEID_MEDIA_NAND_FLAG_SHIFT )

// extern data-----------------------------------------------------------------
// function's prototype--------------------------------------------------------
static void SYSMi_SetBootSRLPath( LauncherBootType bootType, NAMTitleId titleID );
static void SYSMi_SetMountInfoCore( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc, OSMountInfo *pDst );
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt, ROM_Header_Short *pROMH );
static void SYSMi_ModifyShared2FileMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );

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
#ifdef USE_SHARED2_FILE_OLD_FORMAT
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", NULL },	// アプリ間共有ファイル
#else
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand:/shared2" },	// アプリ間共有ファイル
#endif
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

// ランチャーのマウント情報セット
void SYSMi_SetLauncherMountInfo( void )
{
	OSMountInfo mountListBuffer[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4);
	ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	NAMTitleId titleID = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->titleID;
	
	// デフォルトリストをバッファにコピー
	MI_CpuCopyFast( s_defaultMountList, mountListBuffer, sizeof(s_defaultMountList) );
	
	// bootSRLパスの設定は、ランチャーが自分で設定するのは厄介なので、NANDファームから引き渡してもらう
	
	// SDカードアクセス要求がない場合は、sdmcをマウントしない。
	if( header->access_control.sd_card_access == 0 ) {
		mountListBuffer[ SDMC_MOUNT_INDEX ].drive[ 0 ] = 0;
	}
	
	// セーブデータ有無によるマウント情報の編集
	// ※このタイミングではFSは動かせないので、FSを使わない特別版で対応。
	SYSMi_ModifySaveDataMount( LAUNCHER_BOOTTYPE_NAND,
										  titleID,
										  &mountListBuffer[ PRV_SAVE_DATA_MOUNT_INDEX ],
										  ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF
										);
#ifdef USE_SHARED2_FILE_OLD_FORMAT
	// 2008.06.20 shared2仕様変更に伴い、無効にする
	// Shared2のアプリ間共有ファイルセット(LAUNCHERで使うかどうかは微妙)
	SYSMi_ModifyShared2FileMount( LAUNCHER_BOOTTYPE_NAND,
										  titleID,
										  &mountListBuffer[ SHARED2_MOUNT_INDEX ] );
#endif

	// マウント情報のセット
	SYSMi_SetMountInfoCore( LAUNCHER_BOOTTYPE_NAND,
							titleID,
							&mountListBuffer[0],
							(OSMountInfo *)header->sub_mount_info_ram_address );
}


// SYSM_TWL_MOUNT_INFO_TMP_BUFFERに、ブートするアプリのマウント情報を登録する
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle )
{
	OSMountInfo mountListBuffer[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4);
	ROM_Header_Short *pROMH = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	// アプリがTWL対応でない場合は、何もセットせずにリターン
	if( ( pROMH->platform_code ) == 0 ) {
		return;
	}
	
	// 起動アプリのSRLパスをセット
//	SYSMi_SetBootSRLPath( (LauncherBootType)pBootTitle->flags.bootType,
//						  pBootTitle->titleID );

	STD_CopyLStringZeroFill( (char *)(SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE),
							SYSMi_GetWork2()->bootContentPath, OS_MOUNT_PATH_LEN );
	
	// デフォルトリストをバッファにコピー
	MI_CpuCopyFast( s_defaultMountList, mountListBuffer, sizeof(s_defaultMountList) );

	// セキュアアプリでない場合、"nand:", "nand2:"アーカイブをNAに変更。
	if( ( pBootTitle->titleID & TITLE_ID_SECURE_FLAG_MASK ) == 0 ) {
		mountListBuffer[ NAND_MOUNT_INDEX ].userPermission = 0;	// "nand:"
		mountListBuffer[ NAND2_MOUNT_INDEX ].userPermission = 0;	// "nand2:"
	}

	// 2008.06.26 contentとshared2はマウントしない方針とする。
	mountListBuffer[ CONTENT_MOUNT_INDEX ].drive[ 0 ] = 0;
	mountListBuffer[ SHARED2_MOUNT_INDEX ].drive[ 0 ] = 0;

	{
		int i;
		u16 notMount = 0x0000;
		
		// カードアプリは、content, dataPub, dataPrv をマウントしない。
		if( ( pBootTitle->titleID & TITLEID_MEDIA_NAND_FLAG ) == 0 ) {
			notMount |= ( 0x0001 << CONTENT_MOUNT_INDEX ) |
						( 0x0001 << PRV_SAVE_DATA_MOUNT_INDEX ) |
						( 0x0001 << PUB_SAVE_DATA_MOUNT_INDEX );
		}
		
		// TMPジャンプアプリは、contentをマウントしない
		if( (LauncherBootType)pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP )
		{
			notMount |= ( 0x0001 << CONTENT_MOUNT_INDEX );
		}
		
		// SDカードアクセス要求がない場合は、sdmcをマウントしない。
		if( pROMH->access_control.sd_card_access == 0 ) {
			notMount |= ( 0x0001 << SDMC_MOUNT_INDEX );
		}
		
		// NANDアクセス要求がない場合は、NAND関係のドライブを全てマウントしない。
		if( pROMH->access_control.nand_access == 0 ) {
			notMount |= 0x00ff;
		}
		
		// アクセス不可なドライブを無効にする。
		for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
			if( notMount & ( 0x0001 << i ) ) {
				mountListBuffer[ i ].drive[ 0 ] = 0;
			}
		}
	}
	
	// セーブデータ有無によるマウント情報の編集
	// ※ARM7ではNAMは動かせないので、NAMを使わないバージョンで対応。
	SYSMi_ModifySaveDataMount( LAUNCHER_BOOTTYPE_NAND,
										  pBootTitle->titleID,
										  &mountListBuffer[ PRV_SAVE_DATA_MOUNT_INDEX ],
										  ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF
							   );

#if 0
	// 2008.06.20 shared2仕様変更に伴い、無効にする
	// Shared2のアプリ間共有ファイルセット
	SYSMi_ModifyShared2FileMount( LAUNCHER_BOOTTYPE_NAND,
										  pBootTitle->titleID,
										  &mountListBuffer[ SHARED2_MOUNT_INDEX ] );
#endif

	// マウント情報のセット
	SYSMi_SetMountInfoCore( (LauncherBootType)pBootTitle->flags.bootType,
							pBootTitle->titleID,
							&mountListBuffer[0],
							(OSMountInfo *)SYSM_TWL_MOUNT_INFO_TMP_BUFFER );
	
	/*
		※※　注意　※※
		MountInfoは、FSで直接参照してアクセス許可状態を判定しているため、ここにアプリ用のデータをセットすると、
		その後はパーミッションの都合上FSライブラリおよびFSを使用したESやNAMライブラリが全く使用できなくなる。（今後パーミッション仕様については変更される可能性あり）
		よって、内部でFSライブラリを使用する処理は、本処理の前に完了しておく必要がある。
	*/
}

// マウント情報を指定されたアドレスに書き込み
static void SYSMi_SetMountInfoCore( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc, OSMountInfo *pDst )
{
#pragma unused(bootType)
	
	int i;
	char contentpath[ FS_ENTRY_LONGNAME_MAX ];
	
	// タイトルIDからcontentのファイルパスをセット
	STD_TSNPrintf( contentpath, FS_ENTRY_LONGNAME_MAX,
				   "nand:/title/%08x/%08x/content", (u32)( titleID >> 32 ), titleID );
	STD_CopyLStringZeroFill( pSrc[CONTENT_MOUNT_INDEX].path, contentpath, OS_MOUNT_PATH_LEN );
	
	MI_CpuClearFast( (void *)pDst, SYSM_MOUNT_INFO_SIZE );
	
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

#ifdef USE_SHARED2_FILE_OLD_FORMAT
// 2008.06.20 shared2仕様変更に伴い、無効にする
#define KB		( 1024 )
#define MB		( 1024 * 1024 )
#define SHARED2FILE_SIZE_VALUE_TABLE_LENGTH		9
static u32 shared2FileSizeValueTable[] = {
    16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB, 512 * KB,
	1 * MB, 2 * MB, 4 * MB
};

// shared2ファイルのマウント情報を編集する。
static void SYSMi_ModifyShared2FileMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
#pragma unused(bootType,titleID)
	int l;
	BOOL sizeok = FALSE;
	
	// NANDアクセス可能でshared2_fileビットが立っていればマウント
	if( (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->access_control.nand_access &&
		(( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->access_control.shared2_file ) {
		char shared2FilePath[ FS_ENTRY_LONGNAME_MAX ];
		u32 shared2DataSize = (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->shared2_file_size;
		
		// ファイルパスを取得
		STD_TSNPrintf( shared2FilePath, FS_ENTRY_LONGNAME_MAX,
					   "nand:/shared2/%04X", (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->shared2_file_index);
		
		// サイズチェックしてマウント情報登録
		for(l=0; l<SHARED2FILE_SIZE_VALUE_TABLE_LENGTH; l++)
		{
			if( shared2FileSizeValueTable[l] == shared2DataSize )
			{
				sizeok = TRUE;
				break;
			}
		}
		if( sizeok ) {
			STD_CopyLStringZeroFill( pMountTgt->path, shared2FilePath, OS_MOUNT_PATH_LEN );
		}else {
			pMountTgt->drive[ 0 ] = 0;
		}
		
	}else {
		// 可能でなければshared2マウント無効
		pMountTgt->drive[ 0 ] = 0;
	}
}
#endif


// タイトルIDをもとにセーブデータ有無を判定して、マウント情報を編集する。
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt, ROM_Header_Short *pROMH )
{
	int i;
	
	// ※カードからブートされた場合でも、titleIDが"NANDアプリ"の場合は、セーブデータをマウントするようにしている。
	
	// セーブデータ有無を判定して、パスをセット
	if( ( ( bootType == LAUNCHER_BOOTTYPE_NAND ) &&			// NANDアプリがNANDからブートされた時
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == LAUNCHER_BOOTTYPE_ROM ) &&			// ISデバッガ上で、NANDアプリがROM からブートされた時
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) &&
		    SYSM_IsRunOnDebugger() )
		) {
		char saveFilePath[ 2 ][ FS_ENTRY_LONGNAME_MAX ];
		u32 saveDataSize[ 2 ];
		saveDataSize[ 0 ] = pROMH->private_save_data_size;
		saveDataSize[ 1 ] = pROMH->public_save_data_size;
		
		// セーブデータのファイルパスを取得
		STD_TSNPrintf( saveFilePath[ 0 ], FS_ENTRY_LONGNAME_MAX,
					   "nand:/title/%08x/%08x/data/private.sav", (u32)( titleID >> 32 ), titleID );
		STD_TSNPrintf( saveFilePath[ 1 ], FS_ENTRY_LONGNAME_MAX,
					   "nand:/title/%08x/%08x/data/public.sav", (u32)( titleID >> 32 ), titleID );
		
		// "ROMヘッダのNANDセーブファイルサイズ > 0" の場合のみマウント情報を登録
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

#else //ARM9

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
		STD_TSNPrintf( path, FS_ENTRY_LONGNAME_MAX, "nand:/tmp/jump.app", pBootTitle->titleID );
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

#endif //#ifndef SDK_ARM9