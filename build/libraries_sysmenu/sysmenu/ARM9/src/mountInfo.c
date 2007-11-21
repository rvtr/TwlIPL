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
#define PRV_SAVE_DATA_MOUNT_INDEX		5			// �v���C�x�[�g�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X
#define PUB_SAVE_DATA_MOUNT_INDEX		6			// �p�u���b�N�@�Z�[�u�f�[�^�� s_defaultMountInfo ���X�g�C���f�b�N�X

// extern data-----------------------------------------------------------------

// function's prototype--------------------------------------------------------
void SYSM_SetBootSRLPath( NAMTitleId titleID );
void SYSM_SetMountInfo( NAMTitleId titleID );

static void SYSMi_ModifySaveDataMount( NAMTitleId titleID );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// �f�t�H���g�}�E���g��񃊃X�g
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
	{ 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, 0,                               0, 0, "nand",    "/" },	// ���[�U�[�͂��̃A�[�J�C�u���g���Ȃ�(RW�s��)
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, 0,                               0, 0, "nand2",   "/" },	// ���[�U�[�͂��̃A�[�J�C�u���g���Ȃ�(RW�s��)
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
};


// ============================================================================
//
// �}�E���g���Z�b�g
//
// ============================================================================

// �N��SRL�p�X���V�X�e���̈�ɃZ�b�g
void SYSM_SetBootSRLPath( NAMTitleId titleID )
{
	char path[ FS_FILE_NAME_MAX ];
	
	// �^�C�g��ID��"0"�̎��́AROM�Ɣ��f����iDS�_�E�����[�h�v���C�̎��̋����͖������B�B�B�j
	if( titleID ) {
	    NAM_GetTitleBootContentPath( path, titleID );
	}else {
		STD_StrCpy( path, (const char*)"rom:" );
	}
	
	STD_CopyLStringZeroFill( (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, path, OS_MOUNT_PATH_LEN );
	OS_TPrintf( "boot path : %s\n", (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF );
}


// �V�X�e���̈�ɁA�u�[�g����A�v���̃}�E���g����o�^����
void SYSM_SetMountInfo( NAMTitleId titleID )
{
	// �}�E���g���̐���
	SYSMi_ModifySaveDataMount( titleID );	// �Z�[�u�f�[�^
	// SD�͑S�A�v������ŗǂ��H
	// PHOTO�͑S�A�v���ɉ���ŗǂ��H
	
	// �}�E���g�����V�X�e���̈�ɏ�������
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

// �^�C�g��ID�����ƂɃZ�[�u�f�[�^�L���𔻒肵�āA�}�E���g����ҏW����B
static void SYSMi_ModifySaveDataMount( NAMTitleId titleID )
{
	int i;
	OSMountInfo *pMountTgt = &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ];
	
	if( titleID ) {
		// �^�C�g��ID���w�肳��Ă���NAND�A�v���̏ꍇ�́A�Z�[�u�f�[�^�L���𔻒肵�āA�p�X���Z�b�g
		char saveFilePath[ 2 ][ FS_FILE_NAME_MAX ];
		
		// �Z�[�u�f�[�^�̃t�@�C���p�X���擾
		NAM_GetTitleSaveFilePath( saveFilePath[ 1 ], saveFilePath[ 0 ], titleID );
		
		// ���ʂ����Ƀ}�E���g����ҏW�B
		for( i = 0; i < 2; i++ ) {
			FSFile  file[1];
			FS_InitFile( file );
			// �����݂́A�Z�[�u�t�@�C�����J���邩�ǂ����ŃZ�[�u�t�@�C���L�����m�F�B
			//   �ŏI�I�ɂ�TMD��������ROM�w�b�_�̒l���Q�ƁBROM�w�b�_�̕����ȒP�ő������H
			if( FS_OpenFileEx( file, saveFilePath[ i ], FS_FILEMODE_R) ) {
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
