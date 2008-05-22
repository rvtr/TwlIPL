/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
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
#include "hw_info.h"
#include "TWLHWInfo_api.h"
#include "graphics.h"

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

static u32  sCurrentProgress;
static vu8 sNowImport = FALSE;
static vu8 sProgress  = FALSE;
static u8  sStack[THREAD_STACK_SIZE];

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressDraw(f32 ratio);

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad �t�@�C���C���|�[�g

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/

s32 kamiImportTad(int no, int total, const char* path)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	kamiFontPrintfMain( 4, 9, 8, "Now Updating...  %d / %d", no, total );
	kamiFontLoadScreenData();

	// tad�t�@�C���̏��擾
	nam_result = NAM_ReadTadInfo(&tadInfo, path);
	if ( nam_result != NAM_OK )
	{
		return nam_result;
	}

	// ES�̎d�l�ŌÂ� e-ticket ������ƐV���� e-ticket ���g�����C���|�[�g���ł��Ȃ�
	// �b��Ή��Ƃ��ĊY���^�C�g�������S�폜���Ă���C���|�[�g����
	nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
	if ( nam_result != NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
		return FALSE;
	}

	// �C���|�[�g�J�n�t���O�𗧂Ă�
	sNowImport = TRUE;

    // �i���X���b�h�쐬
	MI_CpuClear8(sStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)sStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
    OS_WakeupThreadDirect(&thread);

	// Import�J�n
	nam_result = NAM_ImportTad( path );

	// �C���|�[�g�J�n�t���O��������
	sNowImport = FALSE;

	// �i���X���b�h�̎��͏I����҂�
	while (sProgress){};

	// InstalledSoftBoxCount, FreeSoftBoxCount �̒l�����݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B
	(void)NAMUT_UpdateSoftBoxCount();

	return nam_result;
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

	sProgress = TRUE;

    while (sNowImport)
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

	sProgress = FALSE;
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
  Name:         DrawResult

  Description:  �������ʂ�\�����܂��B

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/

void DrawResult(BOOL result)
{
	// 3D������
	G3X_Reset();
	G3_Identity();
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

	// "Now Updating.." ������
	kamiFontPrintfMain( 0, 9, 7, "                                ");

	if (result)
	{
		kamiFontPrintfMain( 9, 10, 7, "Update Success!");
		// �O���[���_�C�A���O
		DrawQuad( 50,  50, 206, 120, GX_RGB(12, 25, 12));
	}
	else
	{
		kamiFontPrintfMain( 9, 10, 7, "Update Failure!");
		// ���b�h�_�C�A���O
		DrawQuad( 50,  50, 206, 120, GX_RGB(31,  0,  0));
	}

	kamiFontLoadScreenData();

	// 3D�X���b�v
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

