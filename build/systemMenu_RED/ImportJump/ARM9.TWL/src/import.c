/*---------------------------------------------------------------------------*
  Project:  ImportJump
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
#include "graphics.h"
#include "ImportJump.h"

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
static ImportJump sImportJumpSetting;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* arg);
static void Destructor(void* arg);
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

BOOL kamiImportTad(NAMTitleId* pTitleID)
{
	NAMTadInfo tadInfo;
	NAMTitleInfo titleInfoTmp;
	OSThread thread;
	s32  nam_result;
	FSFile file;
	char savePublicPath[FS_ENTRY_LONGNAME_MAX];
	char savePrivatePath[FS_ENTRY_LONGNAME_MAX];
	char subBannerPath[FS_ENTRY_LONGNAME_MAX];

	// ���i�pCPU�ł̓C���|�[�g�s��
	if ( !((*(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK)) )
	{
		OS_Warning(" Fail : Production CPU\n");
		return FALSE;
    }

	// �t�@�C��������
	FS_InitFile(&file);

	// CARD-ROM �̈���ꎞ�I�ȃt�@�C���Ƃ݂Ȃ����̃t�@�C�����J���܂��B
	if (!FS_CreateFileFromRom(&file, GetImportJumpSetting()->tadRomOffset, GetImportJumpSetting()->tadLength))
	{
		OS_Warning(" Fail : FS_CreateFileFromRom\n");
		return FALSE;
	}

	// tad�t�@�C���̏��擾
	if (NAM_ReadTadInfoWithFile(&tadInfo, &file) != NAM_OK)
	{
		OS_Warning(" Fail! : NAM_ReadTadInfo\n");
		return FALSE;
	}

	// titleID��ۑ����Ă���
	*pTitleID = tadInfo.titleInfo.titleId;

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

	// TAD�t�@�C�����X�V����Ă���ꍇ�Ɍ���C���|�[�g�������s��
	// NandInitializer�ɂ���ď�������Ă���\��������̂Ŋm�F����
	if (GetImportJumpSetting()->importTad == 1 || NAM_ReadTitleInfo(&titleInfoTmp, tadInfo.titleInfo.titleId) != NAM_OK)
	{
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
		nam_result = NAM_ImportTadWithFile( &file );

		// �i���X���b�h�̎��͏I����҂�
		while (sNowImport){};

		if ( nam_result == NAM_OK )
		{
			// InstalledSoftBoxCount, FreeSoftBoxCount �̒l�����݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B
			UpdateNandBoxCount();
		}
		else
		{
			OS_Warning(" Fail! : NAM Result Code = 0x%x\n", nam_result);
			return FALSE;
		}
	}

	// �Z�[�u�f�[�^�N���A����
	if (GetImportJumpSetting()->clearPublicSaveData || GetImportJumpSetting()->clearPrivateSaveData)
	{
		// �Z�[�u�t�@�C���p�X�擾
		if ( NAM_GetTitleSaveFilePath(savePublicPath, savePrivatePath, tadInfo.titleInfo.titleId) != NAM_OK )
		{
			OS_Warning(" Fail! NAM_GetTitleSaveFilePath\n");
			return FALSE;
		}
		
		// public�Z�[�u�f�[�^FF�N���A���t�H�[�}�b�g
		if (GetImportJumpSetting()->clearPublicSaveData && tadInfo.titleInfo.publicSaveSize > 0)
		{
			if (NAMUTi_ClearSavedataPublic(savePublicPath, tadInfo.titleInfo.titleId) == FALSE)
			{
				OS_Warning(" Fail! NAMUTi_ClearSavedataPublic\n");
				return FALSE;
			}
		}

		// private�Z�[�u�f�[�^FF�N���A���t�H�[�}�b�g
		if (GetImportJumpSetting()->clearPrivateSaveData && tadInfo.titleInfo.privateSaveSize > 0)
		{
			if (NAMUTi_ClearSavedataPublic(savePrivatePath, tadInfo.titleInfo.titleId) == FALSE)
			{
				OS_Warning(" Fail! NAMUTi_ClearSavedataPrivate\n");
				return FALSE;
			}
		}
	}

	// �T�u�o�i�[�N���A����
	if (GetImportJumpSetting()->clearSaveBannerFile)
	{
		// �T�u�o�i�[�p�X�擾
		if ( NAM_GetTitleBannerFilePath(subBannerPath, tadInfo.titleInfo.titleId) != NAM_OK )
		{
			OS_Warning(" Fail! NAM_GetTitleBannerFilePath\n");
			return FALSE;
		}

		// �T�u�o�i�[�j��
		{
			if (NAMUTi_DestroySubBanner(subBannerPath) == FALSE)
			{
				OS_Warning(" Fail! NAMUTi_DestroySubBanner\n");
				return FALSE;
			}
		}	
	}

	return TRUE;
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

	kamiFontPrintfMain( 4, 9, 8, "Now Importing...");
	kamiFontLoadScreenData();

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
  Name:         ProgressDraw

  Description:  �C���|�[�g�̐i����\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressDraw(f32 ratio)
{
	s16 x = (s16)(30 + (226 - 30)*ratio);

	// 3D������
	G3X_Reset();
	G3_Identity();
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

	// �O���[���o�[
	DrawQuad( 30,  90,   x,  95, GX_RGB(12, 25, 12));

	// �O���[�o�[
	DrawQuad( 30,  90, 226,  95, GX_RGB(28, 28, 28));

	// �O���[�_�C�A���O
	DrawQuad( 20,  60, 236, 110, GX_RGB(25, 25, 25));

	// 3D�X���b�v
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
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

/*---------------------------------------------------------------------------*
  Name:         GetImportJumpSetting

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

ImportJump* GetImportJumpSetting( void )
{
    static BOOL inited = FALSE;

    if ( ! inited )
    {
        // �J���pCPU�ł̂݃��[�h
        if ( *(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK )
        {
            u16 id = (u16)OS_GetLockID();
            CARD_LockRom( id );
            CARD_ReadRom( MI_DMA_NOT_USE, (void*)IMPORT_JUMP_SETTING_OFS, &sImportJumpSetting, sizeof(ImportJump) );
            CARD_UnlockRom( id );
        }
        inited = TRUE;
    }

    return &sImportJumpSetting;
}
