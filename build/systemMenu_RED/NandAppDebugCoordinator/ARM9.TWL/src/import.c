/*---------------------------------------------------------------------------*
  Project:  NandAppDebugCoordinator
  File:     process_import.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "import.h"
#include "TWLHWInfo_api.h"


/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static void* spStack;
static u32  sCurrentProgress;
static vu8 sNowImport = FALSE;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressInit(void);
void ProgressDraw(f32 ratio);
static void UpdateNandBoxCount( void );

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad �t�@�C���C���|�[�g

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/

BOOL kamiImportTad(char* path, NAMTitleId* pTitleId)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	// tad�t�@�C���̏��擾
	if (NAM_ReadTadInfo(&tadInfo, path) != NAM_OK)
	{
		OS_Warning(" Fail! : NAM_ReadTadInfo\n");
		return FALSE;
	}

	// ��ŃA�v���W�����v����TitleId�������œǂݎ���Ă���
	*pTitleId = tadInfo.titleInfo.titleId;

	// Not Launch �Ȃ玸�s
	if (tadInfo.titleInfo.titleId & TITLE_ID_NOT_LAUNCH_FLAG_MASK)
	{
		OS_Warning(" Fail! :  NOT_LAUNCH_FLAG is specified in rsf file\n");
		return FALSE;
	}

	// Data Only �Ȃ玸�s
	if (tadInfo.titleInfo.titleId & TITLE_ID_DATA_ONLY_FLAG_MASK)
	{
		OS_Warning(" Fail! : DATA_ONLY_FLAG is specified in rsf file\n");
		return FALSE;
	}

	// NOT_LAUNCH_FLAG �܂��� DATA_ONLY_FLAG �������Ă��Ȃ��^�C�g���̏ꍇ
	// freeSoftBoxCount�ɋ󂫂��Ȃ���΃C���|�[�g���Ȃ�
	if (NAMUT_SearchInstalledSoftBoxCount() == LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX)
	{
		OS_Warning(" Fail! : NAND FreeSoftBoxCount == 0\n");
		return FALSE;
	}

	// �C���|�[�g�J�n�t���O�𗧂Ă�
	sNowImport = TRUE;

    // �i���X���b�h�쐬
	spStack = OS_Alloc(THREAD_STACK_SIZE);
	MI_CpuClear8(spStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
	OS_SetThreadDestructor( &thread, Destructor );
    OS_WakeupThreadDirect(&thread);

	// Import�J�n
	nam_result = NAM_ImportTad( path );

	// �i���X���b�h�̎��͏I����҂�
	while (sNowImport){};

	if ( nam_result == NAM_OK )
	{
		// InstalledSoftBoxCount, FreeSoftBoxCount �̒l�����݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B
		UpdateNandBoxCount();
		return TRUE;
	}
	else
	{
		OS_Warning(" Fail! : NAM Result Code = 0x%x\n", nam_result);
		return FALSE;
	}
}

static void Destructor(void* /*arg*/)
{
	OS_Free(spStack);
}

/*---------------------------------------------------------------------------*
  Name:         ProgressThread

  Description:  .tad �t�@�C���C���|�[�g�̐i����\������X���b�h�B
				�i����100%�ɒB����Ə����𔲂���B

  Arguments:    arg -   �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* /*arg*/)
{
    u32  currentSize;
    u32  totalSize   = 0;
	u32  totalSizeBk = 0;

	ProgressInit();

    while (TRUE)
    {
        NAM_GetProgress(&currentSize, &totalSize);

		if ((totalSize > 0 && totalSize == currentSize) || totalSizeBk > totalSize)
		{
			// ���ɃC���|�[�g���I��
			ProgressDraw((f32)1.0);
			break;	
		}
		else if (totalSize > 0)
		{
			ProgressDraw((f32)currentSize/totalSize);
		}

		totalSizeBk = totalSize;

		// V�u�����N�҂�
        OS_WaitVBlankIntr();
    }

	sNowImport = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressInit

  Description:  �C���|�[�g�̐i����\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressInit(void)
{
	sCurrentProgress = 0;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressDraw

  Description:  �C���|�[�g�̐i����\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressDraw(f32 ratio)
{
	char square[2] = { 0x01, 0x00 };
	u32 temp;
	s32 i;

	temp = (u32)(32 * ratio);
	if (temp > sCurrentProgress)
	{
		s32 diff = (s32)(temp - sCurrentProgress);
		for (i=0;i<diff;i++)
		{
			kamiFontPrintfConsole(2, square);
		}
	}
	sCurrentProgress = temp;

	// �t�H���g�X�N���[���f�[�^���[�h
	kamiFontLoadScreenData();
}

/*---------------------------------------------------------------------------*
  Name:         UpdateNandBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount �̒l��
				���݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void UpdateNandBoxCount( void )
{
	u32 installedSoftBoxCount;
	u32 freeSoftBoxCount;

	// InstalledSoftBoxCount, FreeSoftBoxCount �𐔂��Ȃ���
	installedSoftBoxCount = NAMUT_SearchInstalledSoftBoxCount();
	freeSoftBoxCount = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - installedSoftBoxCount;

	// LCFG���C�u�����̐ÓI�ϐ��ɑ΂���X�V
    LCFG_TSD_SetInstalledSoftBoxCount( (u8)installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( (u8)freeSoftBoxCount );

	// LCFG���C�u�����̐ÓI�ϐ��̒l��NAND�ɔ��f
    {
        u8 *pBuffer = OS_Alloc( LCFG_WRITE_TEMP );
        if( pBuffer ) {
            (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
            OS_Free( pBuffer );
        }
    }
}

