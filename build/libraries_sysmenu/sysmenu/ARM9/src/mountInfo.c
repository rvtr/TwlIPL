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
#define PRV_SAVE_DATA_MOUNT_INDEX			5			// �v���C�x�[�g�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X
#define PUB_SAVE_DATA_MOUNT_INDEX			6			// �p�u���b�N�@�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X

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

static void SYSMi_SetBootSRLPath( OSBootType bootType, NAMTitleId titleID );
static void SYSMi_SetMountInfoCore( OSBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc );
static void SYSMi_ModifySaveDataMount( OSBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );
static void SYSMi_ModifySaveDataMountForLauncher( OSBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// �f�t�H���g�}�E���g��񃊃X�g
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
	{ 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ���[�U�[�A�v���͂��̃A�[�J�C�u�ł�Write�s��
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ���[�U�[�A�v���͂��̃A�[�J�C�u�ł�Write�s��
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },	// NAND�ɃZ�[�u�f�[�^���Ȃ��A�v���̏ꍇ�́A�}�E���g����Ȃ��B
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },	// NAND�ɃZ�[�u�f�[�^���Ȃ��A�v���̏ꍇ�́A�}�E���g����Ȃ��B
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
	NAMTitleId titleID = TITLE_ID_LAUNCHER;
	
	// �}�E���g���̃N���A
	MI_CpuClearFast( (void *)HW_TWL_FS_MOUNT_INFO_BUF, HW_TWL_ROM_HEADER_BUF - HW_TWL_FS_MOUNT_INFO_BUF );
	
	// ���Ƃ肠�������g��ROM�u�[�g�ŁB[TODO:]��ŏC��
//	SYSMi_SetBootSRLPath( OS_BOOTTYPE_NAND, titleID );		// ��SDK2623�ł́ABootSRLPath��"rom:"�Ƃ�����FSi_InitRomArchive��NAND�A�v����������ăA�N�Z�X��O�ŗ�����B
	
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	// �����̃^�C�~���O�ł�FS�͓������Ȃ��̂ŁAFS���g��Ȃ����ʔłőΉ��B
	SYSMi_ModifySaveDataMountForLauncher( OS_BOOTTYPE_NAND,
										  titleID,
										  &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
	
	// �}�E���g���̃Z�b�g
	SYSMi_SetMountInfoCore( OS_BOOTTYPE_NAND,
							titleID,
							&s_defaultMountList[0] );
}


// �V�X�e���̈�ɁA�u�[�g����A�v���̃}�E���g����o�^����
void SYSM_SetBootAppMountInfo( TitleProperty *pBootTitle )
{
	// �}�E���g���̃N���A
	MI_CpuClearFast( (void *)HW_TWL_FS_MOUNT_INFO_BUF, HW_TWL_ROM_HEADER_BUF - HW_TWL_FS_MOUNT_INFO_BUF );
	
	// �A�v����TWL�Ή��łȂ��ꍇ�́A�����Z�b�g�����Ƀ��^�[��
	if( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) == 0 ) {
		return;
	}
	
	// �N���A�v����SRL�p�X���Z�b�g
	SYSMi_SetBootSRLPath( (OSBootType)pBootTitle->flags.bootType,
						  pBootTitle->titleID );
	
	// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	SYSMi_ModifySaveDataMount( (OSBootType)pBootTitle->flags.bootType,
							   pBootTitle->titleID,
							   &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ] );
	// �}�E���g���̃Z�b�g
	SYSMi_SetMountInfoCore( (OSBootType)pBootTitle->flags.bootType,
							pBootTitle->titleID,
							&s_defaultMountList[0] );
	
	/*
		�����@���Ӂ@����
		MountInfo�́AFS�Œ��ڎQ�Ƃ��ăA�N�Z�X����Ԃ𔻒肵�Ă��邽�߁A�����ɃA�v���p�̃f�[�^���Z�b�g����ƁA
		���̌�̓p�[�~�b�V�����̓s����FS���C�u���������FS���g�p����ES��NAM���C�u�������S���g�p�ł��Ȃ��Ȃ�B�i����p�[�~�b�V�����d�l�ɂ��Ă͕ύX�����\������j
		����āA������FS���C�u�������g�p���鏈���́A�{�����̑O�Ɋ������Ă����K�v������B
	*/
}


// �N��SRL�p�X���V�X�e���̈�ɃZ�b�g
static void SYSMi_SetBootSRLPath( OSBootType bootType, NAMTitleId titleID  )
{
	static char path[ FS_ENTRY_LONGNAME_MAX ];
	
	switch( bootType )
	{
	case OS_BOOTTYPE_NAND:
		if( NAM_GetTitleBootContentPathFast( path, titleID ) != NAM_OK ) {
			OS_TPrintf( "ERROR: BootContentPath Get failed.\n" );
		}
		break;
	case OS_BOOTTYPE_TEMP:
		STD_TSNPrintf( path, 31, "nand:/tmp/%.16llx.srl", titleID );
		break;
	default:
		path[ 0 ] = 0;
//		STD_StrCpy( path, (const char*)"rom:" );		// ��SDK2623�ł́ABootSRLPath��"rom:"�Ƃ�����FSi_InitRomArchive��NAND�A�v����������ăA�N�Z�X��O�ŗ�����B
		break;
	}
	
	if( path[ 0 ] ) {
		STD_CopyLStringZeroFill( (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, path, OS_MOUNT_PATH_LEN );
	}
//	OS_TPrintf( "boot path : %s\n", (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF );	// ������OS_Init�O�ŌĂ΂��̂ŁAPrintf�ł��Ȃ��B
}


// �}�E���g�����V�X�e���̈�ɏ�������
static void SYSMi_SetMountInfoCore( OSBootType bootType, NAMTitleId titleID, OSMountInfo *pSrc )
{
#pragma unused(bootType)
	
	int i;
	u32 titleID_Hi = (u32)( titleID >> 32 );		// u64�Ř_�����Z�͂ł��Ȃ��H
	OSMountInfo *pDst   = (OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
	
	// ���[�U�[�A�v���̏ꍇ�A"nand:", "nand2:"�A�[�J�C�u��ύX�B
	if( ( titleID_Hi & TITLEID_HI_APP_SYS_FLAG ) == 0 ) {
		pSrc[ 1 ].userPermission = 0;	// "nand:"
		pSrc[ 2 ].userPermission = 0;	// "nand2:"
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
	pDst   = (OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
	for( i = 0; i < DEFAULT_MOUNT_LIST_NUM; i++ ) {
		OS_TPrintf( "mount path : %s\n", pDst->path );
		pDst++;
	}
#endif
}


// �^�C�g��ID�����ƂɃZ�[�u�f�[�^�L���𔻒肵�āA�}�E���g����ҏW����B
static void SYSMi_ModifySaveDataMount( OSBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	u32 titleID_Hi = (u32)( titleID >> 32 );		// u64�Ř_�����Z�͂ł��Ȃ��H
	
	// ���J�[�h����u�[�g���ꂽ�ꍇ�ł��AtitleID��"NAND�A�v��"�̏ꍇ�́A�Z�[�u�f�[�^���}�E���g����悤�ɂ��Ă���B
	
	// �Z�[�u�f�[�^�L���𔻒肵�āA�p�X���Z�b�g
	if( ( ( bootType == OS_BOOTTYPE_NAND ) &&				// NAND�A�v����NAND����u�[�g���ꂽ��
		  ( titleID_Hi & TITLEID_HI_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == OS_BOOTTYPE_ROM ) &&				// IS�f�o�b�K��ŁANAND�A�v����ROM ����u�[�g���ꂽ��
		  ( titleID_Hi & TITLEID_HI_MEDIA_NAND_FLAG ) &&
		  ( SYSMi_GetWork()->isOnDebugger ) )
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
static void SYSMi_ModifySaveDataMountForLauncher( OSBootType bootType, NAMTitleId titleID, OSMountInfo *pMountTgt )
{
	int i;
	u32 titleID_Hi = (u32)( titleID >> 32 );		// u64�Ř_�����Z�͂ł��Ȃ��H
	
	// ���J�[�h����u�[�g���ꂽ�ꍇ�ł��AtitleID��"NAND�A�v��"�̏ꍇ�́A�Z�[�u�f�[�^���}�E���g����悤�ɂ��Ă���B
	
	// �Z�[�u�f�[�^�L���𔻒肵�āA�p�X���Z�b�g
	if( ( ( bootType == OS_BOOTTYPE_NAND ) &&				// NAND�A�v����NAND����u�[�g���ꂽ��
		  ( titleID_Hi & TITLEID_HI_MEDIA_NAND_FLAG ) ) ||
		( ( bootType == OS_BOOTTYPE_ROM ) &&				// IS�f�o�b�K��ŁANAND�A�v����ROM ����u�[�g���ꂽ��
		  ( titleID_Hi & TITLEID_HI_MEDIA_NAND_FLAG ) &&
		  ( SYSMi_GetWork()->isOnDebugger ) )
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

