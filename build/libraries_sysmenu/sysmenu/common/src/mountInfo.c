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

#ifndef SDK_ARM9

// define data-----------------------------------------------------------------
#define DEFAULT_MOUNT_LIST_NUM				9
#define NAND_MOUNT_INDEX			0
#define NAND2_MOUNT_INDEX			1
#define CONTENT_MOUNT_INDEX			2
#define SHARED1_MOUNT_INDEX			3
#define SHARED2_MOUNT_INDEX			4
#define PRV_SAVE_DATA_MOUNT_INDEX			6			// �v���C�x�[�g�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X
#define PUB_SAVE_DATA_MOUNT_INDEX			7			// �p�u���b�N�@�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X
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
static void SYSMi_ModifySaveDataMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );
static void SYSMi_ModifySaveDataMountForLauncher( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );
static void SYSMi_ModifyShared2FileMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );

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
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", NULL },	// �A�v���ԋ��L�t�@�C��
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

// �����`���[�̃}�E���g���Z�b�g
void SYSMi_SetLauncherMountInfo( void )
{
	OSMountInfo mountListBuffer[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4);
	ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	NAMTitleId titleID = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->titleID;
	
	// �f�t�H���g���X�g���o�b�t�@�ɃR�s�[
	MI_CpuCopyFast( s_defaultMountList, mountListBuffer, sizeof(s_defaultMountList) );
	
	if( ( *(u8 *)HW_NAND_FIRM_HOTSTART_FLAG & 0x80 ) == 0 ) {
		MI_CpuClearFast( (u8 *)header->sub_mount_info_ram_address, SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN );
	}
	
	// bootSRL�p�X�̐ݒ�́A�����`���[�������Őݒ肷��͖̂��Ȃ̂ŁANAND�t�@�[����������n���Ă��炤
	
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	// �����̃^�C�~���O�ł�FS�͓������Ȃ��̂ŁAFS���g��Ȃ����ʔłőΉ��B
	SYSMi_ModifySaveDataMountForLauncher( LAUNCHER_BOOTTYPE_NAND,
										  titleID,
										  &mountListBuffer[ PRV_SAVE_DATA_MOUNT_INDEX ] );

	// Shared2�̃A�v���ԋ��L�t�@�C���Z�b�g(LAUNCHER�Ŏg�����ǂ����͔���)
	SYSMi_ModifyShared2FileMount( LAUNCHER_BOOTTYPE_NAND,
										  titleID,
										  &mountListBuffer[ SHARED2_MOUNT_INDEX ] );

	// �}�E���g���̃Z�b�g
	SYSMi_SetMountInfoCore( LAUNCHER_BOOTTYPE_NAND,
							titleID,
							&mountListBuffer[0],
							(OSMountInfo *)header->sub_mount_info_ram_address );
}


// SYSM_TWL_MOUNT_INFO_TMP_BUFFER�ɁA�u�[�g����A�v���̃}�E���g����o�^����
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle )
{
	OSMountInfo mountListBuffer[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4);
	ROM_Header_Short *pROMH = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	// �A�v����TWL�Ή��łȂ��ꍇ�́A�����Z�b�g�����Ƀ��^�[��
	if( ( pROMH->platform_code ) == 0 ) {
		return;
	}
	
	// �N���A�v����SRL�p�X���Z�b�g
//	SYSMi_SetBootSRLPath( (LauncherBootType)pBootTitle->flags.bootType,
//						  pBootTitle->titleID );

	STD_CopyLStringZeroFill( (char *)(SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE),
							SYSMi_GetWork2()->bootContentPath, OS_MOUNT_PATH_LEN );
	
	// �f�t�H���g���X�g���o�b�t�@�ɃR�s�[
	MI_CpuCopyFast( s_defaultMountList, mountListBuffer, sizeof(s_defaultMountList) );

	// SD�J�[�h�A�N�Z�X�v�����Ȃ��ꍇ�́Asdmc�h���C�u���}�E���g���Ȃ��B
	if( pROMH->access_control.sd_card_access == 0 ) {
		mountListBuffer[ SDMC_MOUNT_INDEX ].drive[ 0 ] = 0;
	}
	
	// �Z�L���A�A�v���łȂ��ꍇ�A"nand:", "nand2:"�A�[�J�C�u��NA�ɕύX�B
	if( ( pBootTitle->titleID & TITLE_ID_SECURE_FLAG_MASK ) == 0 ) {
		mountListBuffer[ NAND_MOUNT_INDEX ].userPermission = 0;	// "nand:"
		mountListBuffer[ NAND2_MOUNT_INDEX ].userPermission = 0;	// "nand2:"
	}
	
	// �Z�L���A�A�v���łȂ��J�[�h�A�v���́A�}�E���g�����N���A����B
	// �A���ASDIO[1]�A�N�Z�X���L���Ȃ�A�������̃A�[�J�C�u���c���B�iNAND�ɃA�N�Z�X�������J�[�h�A�v�����o�Ă������̂��߂̏����B�j
	if( ( ( pBootTitle->titleID & TITLE_ID_SECURE_FLAG_MASK ) == 0 ) &&
		( ( pBootTitle->titleID & TITLEID_MEDIA_NAND_FLAG ) == 0 ) ) {
		int i;
		u16 mask = 0;
		if( pROMH->arm7_scfg_ext & ROM_SCFG_EXT_SD1_MASK ) {
			mask = 0x013b;		// SDIO[1]�A�N�Z�X���L���ȃA�v���́Anand:/, nand2:/, shared1:/, shared2:/, photo:/, sdmc:/���c���B
		}else {
			mask = 0;			// �S�}�E���g���N���A
		}
		for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
			if( ( mask & ( 0x0001 << i ) ) == 0 ) {
				mountListBuffer[ i ].drive[ 0 ] = 0;
			}
		}
	}
	
	// TMP�W�����v����content�̓}�E���g���Ȃ�
	if( (LauncherBootType)pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP )
	{
		mountListBuffer[CONTENT_MOUNT_INDEX].drive[ 0 ] = 0;
	}
	
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	// ��ARM7�ł�NAM�͓������Ȃ��̂ŁANAM���g��Ȃ��o�[�W�����őΉ��B
	SYSMi_ModifySaveDataMountForLauncher( LAUNCHER_BOOTTYPE_NAND,
										  pBootTitle->titleID,
										  &mountListBuffer[ PRV_SAVE_DATA_MOUNT_INDEX ] );
/*
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	SYSMi_ModifySaveDataMount( (LauncherBootType)pBootTitle->flags.bootType,
							   pBootTitle->titleID,
							   &mountListBuffer[ PRV_SAVE_DATA_MOUNT_INDEX ] );
*/

	// Shared2�̃A�v���ԋ��L�t�@�C���Z�b�g
	SYSMi_ModifyShared2FileMount( LAUNCHER_BOOTTYPE_NAND,
										  pBootTitle->titleID,
										  &mountListBuffer[ SHARED2_MOUNT_INDEX ] );

	// �}�E���g���̃Z�b�g
	SYSMi_SetMountInfoCore( (LauncherBootType)pBootTitle->flags.bootType,
							pBootTitle->titleID,
							&mountListBuffer[0],
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

#define KB		( 1024 )
#define MB		( 1024 * 1024 )
#define SHARED2FILE_SIZE_VALUE_TABLE_LENGTH		9
static u32 shared2FileSizeValueTable[] = {
    16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB, 512 * KB,
	1 * MB, 2 * MB, 4 * MB
};

// shared2�t�@�C���̃}�E���g����ҏW����B
static void SYSMi_ModifyShared2FileMount( LauncherBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
#pragma unused(bootType,titleID)
	int l;
	BOOL sizeok = FALSE;
	
	// NAND�A�N�Z�X�\��shared2_file�r�b�g�������Ă���΃}�E���g
	if( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->access_control.nand_access &&
		(( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->access_control.shared2_file ) {
		char shared2FilePath[ FS_ENTRY_LONGNAME_MAX ];
		u32 shared2DataSize = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->shared2_file_size;
		
		// �t�@�C���p�X���擾
		STD_TSNPrintf( shared2FilePath, FS_ENTRY_LONGNAME_MAX,
					   "nand:/shared2/%04X", (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->shared2_file_index);
		
		//[TODO:]���ۂɃt�@�C���̃T�C�Y�����ē������ǂ������`�F�b�N
		// �T�C�Y�`�F�b�N���ă}�E���g���o�^
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
		// �\�łȂ����shared2�}�E���g����
		pMountTgt->drive[ 0 ] = 0;
	}
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
		saveDataSize[ 0 ] = (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->private_save_data_size;
		saveDataSize[ 1 ] = (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->public_save_data_size;
		
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
		
		//[TODO:]���ۂɃt�@�C�����J���Ă݂āA�J���邩�ǂ����`�F�b�N
		
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
//		STD_StrCpy( path, (const char*)"rom:" );		// ��SDK2623�ł́ABootSRLPath��"rom:"�Ƃ�����FSi_InitRomArchive��NAND�A�v����������ăA�N�Z�X��O�ŗ�����B
		break;
	}
	
	if( path[ 0 ] ) {
		STD_CopyLStringZeroFill( SYSMi_GetWork2()->bootContentPath, path, OS_MOUNT_PATH_LEN );
	}else {
		MI_CpuClearFast( SYSMi_GetWork2()->bootContentPath, OS_MOUNT_PATH_LEN );
	}
	OS_TPrintf( "boot SRL path : %s\n", SYSMi_GetWork2()->bootContentPath );	// ��OS_Init�O�ŌĂԂ�Printf�ł��Ȃ��̂Œ��ӁB
}

#endif //#ifndef SDK_ARM9