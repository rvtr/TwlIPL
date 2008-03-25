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
#define PRV_SAVE_DATA_MOUNT_INDEX			6			// �v���C�x�[�g�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X
#define PUB_SAVE_DATA_MOUNT_INDEX			7			// �p�u���b�N�@�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X

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
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );
static void SYSMi_ModifySaveDataMountForLauncher( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

// �f�t�H���g�}�E���g��񃊃X�g
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ���[�U�[�A�v���͂��̃A�[�J�C�u�ł�R/W�s��
	{ 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ���[�U�[�A�v���͂��̃A�[�J�C�u�ł�R/W�s��
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R), 0, 0, "content", NULL },			// Write�s��
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R), 0, 0, "shared1", "nand:/shared1" },	// Write�s��
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand:/shared2" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  1, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },	// NAND�ɃZ�[�u�f�[�^���Ȃ��A�v���̏ꍇ�́A�}�E���g����Ȃ��B
	{ 'H', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },	// NAND�ɃZ�[�u�f�[�^���Ȃ��A�v���̏ꍇ�́A�}�E���g����Ȃ��B
	{ 'I', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
};


// ============================================================================
//
// �}�E���g���Z�b�g
//
// ============================================================================

/*
	�v�m�F
	�J�[�h�u�[�g����BootSRLPath�́A"rom:"�ł͂Ȃ��A""�ł����B
	"nand:" �� "nand1:"��userPermission��"OS_MOUNT_USR_R"�ŗǂ��̂��H
*/
// �����`���[�̃}�E���g���Z�b�g
void SYSMi_SetLauncherMountInfo( void )
{
	ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	NAMTitleId titleID = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->titleID;
	
	// bootSRL�p�X��ݒ�i�����`���[�������Őݒ肷��͖̂��Ȃ̂ŁANAND�t�@�[������HW_TWL_FS_BOOT_SRL_PATH_BUF�o�R��
	// �����n���Ă��炤
	{
#define BOOT_SRL_PATH_OFFSET	0x3c0
		u8 *pMountInfoAddr = ( ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_mount_info_ram_address;
		if( *(pMountInfoAddr + BOOT_SRL_PATH_OFFSET ) == 0 ) {
			MI_CpuCopyFast( (void *)HW_TWL_FS_BOOT_SRL_PATH_BUF, pMountInfoAddr + BOOT_SRL_PATH_OFFSET, 0x40 );
		}
	}
	
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	// �����̃^�C�~���O�ł�FS�͓������Ȃ��̂ŁAFS���g��Ȃ����ʔłőΉ��B
	SYSMi_ModifySaveDataMountForLauncher( LAUNCHER_BOOTTYPE_NAND,
										  titleID,
										  &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );

	// �}�E���g���̃Z�b�g
	SYSMi_SetMountInfoCore( LAUNCHER_BOOTTYPE_NAND,
							titleID,
							&s_defaultMountList[0],
							(OSMountInfo *)header->sub_mount_info_ram_address );
}


// SYSM_TWL_MOUNT_INFO_TMP_BUFFER�ɁA�u�[�g����A�v���̃}�E���g����o�^����
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle )
{
	// �A�v����TWL�Ή��łȂ��ꍇ�́A�����Z�b�g�����Ƀ��^�[��
	if( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) == 0 ) {
		return;
	}
	
	// �N���A�v����SRL�p�X���Z�b�g
//	SYSMi_SetBootSRLPath( (LauncherBootType)pBootTitle->flags.bootType,
//						  pBootTitle->titleID );

	STD_CopyLStringZeroFill( (char *)(SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE),
							SYSMi_GetWork2()->bootContentPath, OS_MOUNT_PATH_LEN );
	
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	// ��ARM7�ł�NAM�͓������Ȃ��̂ŁANAM���g��Ȃ��o�[�W�����őΉ��B
	SYSMi_ModifySaveDataMountForLauncher( LAUNCHER_BOOTTYPE_NAND,
										  pBootTitle->titleID,
										  &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
/*
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	SYSMi_ModifySaveDataMount( (LauncherBootType)pBootTitle->flags.bootType,
							   pBootTitle->titleID,
							   &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
*/
	// �}�E���g���̃Z�b�g
	SYSMi_SetMountInfoCore( (LauncherBootType)pBootTitle->flags.bootType,
							pBootTitle->titleID,
							&s_defaultMountList[0],
							(OSMountInfo *)SYSM_TWL_MOUNT_INFO_TMP_BUFFER );
	
	/*
		�����@���Ӂ@����
		MountInfo�́AFS�Œ��ڎQ�Ƃ��ăA�N�Z�X����Ԃ𔻒肵�Ă��邽�߁A�����ɃA�v���p�̃f�[�^���Z�b�g����ƁA
		���̌�̓p�[�~�b�V�����̓s����FS���C�u���������FS���g�p����ES��NAM���C�u�������S���g�p�ł��Ȃ��Ȃ�B�i����p�[�~�b�V�����d�l�ɂ��Ă͕ύX�����\������j
		����āA������FS���C�u�������g�p���鏈���́A�{�����̑O�Ɋ������Ă����K�v������B
	*/
}

// �}�E���g�����w�肳�ꂽ�A�h���X�ɏ�������
static void SYSMi_SetMountInfoCore( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc, OSMountInfo *pDst )
{
#pragma unused(bootType)
	
	int i;
	char contentpath[ FS_ENTRY_LONGNAME_MAX ];

	// �^�C�g��ID����content�̃t�@�C���p�X���Z�b�g
	STD_TSNPrintf( contentpath, FS_ENTRY_LONGNAME_MAX,
				   "nand:/title/%08x/%08x/content", (u32)( titleID >> 32 ), titleID );
	STD_CopyLStringZeroFill( pSrc[CONTENT_MOUNT_INDEX].path, contentpath, OS_MOUNT_PATH_LEN );
	
	MI_CpuClearFast( (void *)pDst, SYSM_MOUNT_INFO_SIZE );
	
	// �Z�L���A�A�v���łȂ��ꍇ�A"nand:", "nand2:"�A�[�J�C�u��ύX�B
	if( ( titleID & TITLE_ID_SECURE_FLAG_MASK ) == 0 ) {
		pSrc[ NAND_MOUNT_INDEX ].userPermission = 0;	// "nand:"
		pSrc[ NAND2_MOUNT_INDEX ].userPermission = 0;	// "nand2:"
	}
	
	// �Z�b�g
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


// �^�C�g��ID�����ƂɃZ�[�u�f�[�^�L���𔻒肵�āA�}�E���g����ҏW����B
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	
	// ���J�[�h����u�[�g���ꂽ�ꍇ�ł��AtitleID��"NAND�A�v��"�̏ꍇ�́A�Z�[�u�f�[�^���}�E���g����悤�ɂ��Ă���B
	
	// �Z�[�u�f�[�^�L���𔻒肵�āA�p�X���Z�b�g
	if( ( ( bootType == LAUNCHER_BOOTTYPE_NAND ) &&				// NAND�A�v����NAND����u�[�g���ꂽ��
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == LAUNCHER_BOOTTYPE_ROM ) &&				// IS�f�o�b�K��ŁANAND�A�v����ROM ����u�[�g���ꂽ��
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) &&
		  ( SYSMi_GetWork()->flags.hotsw.isOnDebugger ) )
		) {
		char saveFilePath[ 2 ][ FS_ENTRY_LONGNAME_MAX ];
		u32 saveDataSize[ 2 ];
		saveDataSize[ 0 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->private_save_data_size;
		saveDataSize[ 1 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->public_save_data_size;
		
		// �Z�[�u�f�[�^�̃t�@�C���p�X���擾
		NAM_GetTitleSaveFilePath( saveFilePath[ 1 ], saveFilePath[ 0 ], titleID );
		
		// "ROM�w�b�_��NAND�Z�[�u�t�@�C���T�C�Y > 0" ���� ���̃t�@�C�����J����ꍇ�̂݃}�E���g����o�^
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
		// �^�C�g��ID�w��Ȃ��̃J�[�h�A�v���̏ꍇ�́A�Z�[�u�f�[�^����
		for( i = 0; i < 2; i++ ) {
			pMountTgt->drive[ 0 ] = 0;
		}
	}
}


// �^�C�g��ID�����ƂɃZ�[�u�f�[�^�L���𔻒肵�āA�}�E���g����ҏW����B
static void SYSMi_ModifySaveDataMountForLauncher( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	
	// ���J�[�h����u�[�g���ꂽ�ꍇ�ł��AtitleID��"NAND�A�v��"�̏ꍇ�́A�Z�[�u�f�[�^���}�E���g����悤�ɂ��Ă���B
	
	// �Z�[�u�f�[�^�L���𔻒肵�āA�p�X���Z�b�g
	if( ( ( bootType == LAUNCHER_BOOTTYPE_NAND ) &&			// NAND�A�v����NAND����u�[�g���ꂽ��
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == LAUNCHER_BOOTTYPE_ROM ) &&			// IS�f�o�b�K��ŁANAND�A�v����ROM ����u�[�g���ꂽ��
		  ( titleID & TITLEID_MEDIA_NAND_FLAG ) &&
		  ( SYSMi_GetWork()->flags.hotsw.isOnDebugger ) )
		) {
		char saveFilePath[ 2 ][ FS_ENTRY_LONGNAME_MAX ];
		u32 saveDataSize[ 2 ];
		saveDataSize[ 0 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->private_save_data_size;
		saveDataSize[ 1 ] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->public_save_data_size;
		
		// �Z�[�u�f�[�^�̃t�@�C���p�X���擾
		STD_TSNPrintf( saveFilePath[ 0 ], FS_ENTRY_LONGNAME_MAX,
					   "nand:/title/%08x/%08x/data/private.sav", (u32)( titleID >> 32 ), titleID );
		STD_TSNPrintf( saveFilePath[ 1 ], FS_ENTRY_LONGNAME_MAX,
					   "nand:/title/%08x/%08x/data/public.sav", (u32)( titleID >> 32 ), titleID );
		
		// "ROM�w�b�_��NAND�Z�[�u�t�@�C���T�C�Y > 0" ���� ���̃t�@�C�����J����ꍇ�̂݃}�E���g����o�^
		for( i = 0; i < 2; i++ ) {
			if( saveDataSize[ i ] ) {
				STD_CopyLStringZeroFill( pMountTgt->path, saveFilePath[ i ], OS_MOUNT_PATH_LEN );
			}else {
				pMountTgt->drive[ 0 ] = 0;
			}
			pMountTgt++;
		}
	}else {
		// �^�C�g��ID�w��Ȃ��̃J�[�h�A�v���̏ꍇ�́A�Z�[�u�f�[�^����
		for( i = 0; i < 2; i++ ) {
			pMountTgt->drive[ 0 ] = 0;
		}
	}
}
