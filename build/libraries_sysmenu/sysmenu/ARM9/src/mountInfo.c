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
void SYSMi_SetLauncherMountInfo( void );
void SYSM_SetBootAppMountInfo( NAMTitleId titleID );

static void SYSMi_ModifySaveDataMount( NAMTitleId titleID );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static void SYSMi_SetBootSRLPath( NAMTitleId titleID );
static void SYSMi_SetMountInfoCore( const OSMountInfo *pSrc );

// const data------------------------------------------------------------------

/*

	�����݁ASDK��FATFS�o�O����̂��߁A�ˊт�"nand:"��"F:"�h���C�u�ɂ��Ă���B
	�iNAND2KB���[�h���΍􂪁A"F"�h���C�u�݂̂ł̑Ή��ɂȂ��Ă��邽�߁B�j

*/


// �f�t�H���g�}�E���g��񃊃X�g
OSMountInfo s_defaultMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
//	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R),                0, 0, "nand",    "/" },	// ���[�U�[�͂��̃A�[�J�C�u�ł�Write�s��
//	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R),                0, 0, "nand2",   "/" },	// ���[�U�[�͂��̃A�[�J�C�u�ł�Write�s��
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// ���[�U�[�͂��̃A�[�J�C�u�ł�Write�s��
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ���[�U�[�͂��̃A�[�J�C�u�ł�Write�s��
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{ 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
	{ 'H', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
};

// �����`���[�}�E���g���
const OSMountInfo s_launcherMountList[ DEFAULT_MOUNT_LIST_NUM ] ATTRIBUTE_ALIGN(4) = {
//  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
	{ 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
	{ 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },	// �����`���[�͂������A�N�Z�X��
	{ 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand2",   "/" },	// ����
	{ 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
	{ 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
	{   0, OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
	{   0, OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
};



// ============================================================================
//
// �}�E���g���Z�b�g
//
// ============================================================================

// �����`���[�̃}�E���g���Z�b�g
void SYSMi_SetLauncherMountInfo( void )
{
	// ���Ƃ肠�������g��ROM�u�[�g�ŁB�i��ŏC���j
//	SYSMi_SetBootSRLPath( 0 );						// ��SDK2623�ł́ABootSRLPath��"rom:"�Ƃ�����FSi_InitRomArchive��NAND�A�v����������ăA�N�Z�X��O�ŗ�����B
	SYSMi_SetMountInfoCore( &s_launcherMountList[0] );
}


// �V�X�e���̈�ɁA�u�[�g����A�v���̃}�E���g����o�^����
void SYSM_SetBootAppMountInfo( NAMTitleId titleID )
{
	SYSMi_SetBootSRLPath( titleID );				// �N���A�v����SRL�p�X���Z�b�g
	SYSMi_ModifySaveDataMount( titleID );			// �Z�[�u�f�[�^�L���ɂ��}�E���g���̕ҏW
	/*
		�����@���Ӂ@����
		MountInfo�́AFS�Œ��ڎQ�Ƃ��ăA�N�Z�X����Ԃ𔻒肵�Ă��邽�߁A�����ɃA�v���p�̃f�[�^���Z�b�g����ƁA
		���̌�̓p�[�~�b�V�����̓s����FS���C�u���������FS���g�p����ES��NAM���C�u�������S���g�p�ł��Ȃ��Ȃ�B�i����p�[�~�b�V�����d�l�ɂ��Ă͕ύX�����\������j
		����āA������FS���C�u�������g�p���鏈���́A�{�����̑O�Ɋ������Ă����K�v������B
	*/
	SYSMi_SetMountInfoCore( (const OSMountInfo *)&s_defaultMountList[0] );	// �}�E���g���̃Z�b�g
}


// �N��SRL�p�X���V�X�e���̈�ɃZ�b�g
static void SYSMi_SetBootSRLPath( NAMTitleId titleID )
{
	static char path[ FS_FILE_NAME_MAX ];
	
	MI_CpuClear8( path, FS_FILE_NAME_MAX );
	
	// �^�C�g��ID��"0"�̎��́AROM�Ɣ��f����iDS�_�E�����[�h�v���C�̎��̋����͖������B�B�B�j
	if( titleID ) {
		if( NAM_GetTitleBootContentPath( path, titleID ) != NAM_OK ) {
			OS_TPrintf( "ERROR: BootContentPath Get failed.\n" );
		}
	}else {
//		STD_StrCpy( path, (const char*)"rom:" );		// ��SDK2623�ł́ABootSRLPath��"rom:"�Ƃ�����FSi_InitRomArchive��NAND�A�v����������ăA�N�Z�X��O�ŗ�����B
	}
	
	STD_CopyLStringZeroFill( (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, path, OS_MOUNT_PATH_LEN );
//	OS_TPrintf( "boot path : %s\n", (char *)HW_TWL_FS_BOOT_SRL_PATH_BUF );	// ������OS_Init�O�ŌĂ΂��̂ŁAPrintf�ł��Ȃ��B
}


// �}�E���g�����V�X�e���̈�ɏ�������
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


/*
static void SYSMi_ModifySaveDataMount2( NAMTitleId titleID, ROM_Header_Short *pROMH )
{
	int i;
	OSMountInfo *pMountTgt = &s_defaultMountList[ PRV_SAVE_DATA_MOUNT_INDEX ];
	u32 *pROMHSaveDataSize = &pROMH->public_save_data_size;
	
	if( titleID ) {
		// �^�C�g��ID���w�肳��Ă���NAND�A�v���̏ꍇ�́A�Z�[�u�f�[�^�L���𔻒肵�āA�p�X���Z�b�g
		char saveFilePath[ 2 ][ FS_FILE_NAME_MAX ];
		
		// �Z�[�u�f�[�^�̃t�@�C���p�X���擾
		NAM_GetTitleSaveFilePath( saveFilePath[ 1 ], saveFilePath[ 0 ], titleID );
		
		// ���ʂ����Ƀ}�E���g����ҏW�B
		for( i = 0; i < 2; i++ ) {
			BOOL isFind = FALSE;
			// ROM�w�b�_�ɃZ�[�u�f�[�^�T�C�Y�̋L�ڂ�����Ȃ�
			if( *pROMHSaveDataSize++ ) {
				FSFile  file[1];
				FS_InitFile( file );
				// �Z�[�u�t�@�C�����J����Ȃ�OK�B
				if( FS_OpenFileEx( file, saveFilePath[ i ], FS_FILEMODE_R) ) {
					FS_CloseFile( file );
					isFind = TRUE;
				}
				// �������`���[�ŃZ�[�u�f�[�^�t�@�C���̃��J�o���܂ł��H
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
		// �^�C�g��ID�w��Ȃ��̃J�[�h�A�v���̏ꍇ�́A�Z�[�u�f�[�^����
		for( i = 0; i < 2; i++ ) {
			pMountTgt->drive[ 0 ] = 0;
		}
	}
}
*/
