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
#include "hw_info.h"
#include "TWLHWInfo_api.h"
#include "graphics.h"
#include "kami_global.h"
#include "font.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

static const char* sDirectoryNameRegion[] =
{
	"japan",
	"america",
	"europe",
	"australia"
};

static const char* sDirectoryNameConsole[] =
{
	"debugger",	  // IS_TWL_DEBUGGER
	"standalone", // IS_TWL_CAPTURE
	"standalone", // TWL
	""			  // UNKNOWN
};

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

static s32 kamiImportTad(const char* path, BOOL erase);
static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressDraw(f32 ratio);
BOOL ImportDirectoryTad(char* directory);

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessImport

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessImport(void)
{
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char directory[FS_ENTRY_LONGNAME_MAX+6];
	s32 i=0;
	s32 j=0;

	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();
	NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");

	while(!FadeInTick())
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	    OS_WaitVBlankIntr();
	}

	STD_TSNPrintf(directory, sizeof(directory), "rom:/data/%s/%s/", sDirectoryNameConsole[GetConsole()], sDirectoryNameRegion[gRegion]);
	result &= ImportDirectoryTad(directory);

	STD_TSNPrintf(directory, sizeof(directory), "rom:/data/common/%s/", sDirectoryNameRegion[gRegion]);
	result &= ImportDirectoryTad(directory);

	STD_TSNPrintf(directory, sizeof(directory), "rom:/data/common/");
	result &= ImportDirectoryTad(directory);

	while (!FadeOutTick())
	{
	    OS_WaitVBlankIntr();
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ImportDirectoryTad

  Description:  �w�肵���f�B���N�g���ɂ���TAD�𖳏�����Import���܂��B

  Arguments:    path

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ImportDirectoryTad(char* directory)
{
    FSFile  dir;
    FSDirectoryEntryInfo   info[1];
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	static s32 listNo=0;
	s32 j=0;

	FS_InitFile(&dir);
	if (!FS_OpenDirectory(&dir, directory, FS_FILEMODE_R))
	{
    	kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory()\n");
		return FALSE;
	}

	// tad�t�@�C�����������ăC���|�[�g
    while (FS_ReadDirectory(&dir, info))
    {
		s32  nam_result;
		char string1[256];
		u16  string2[256];

		MI_CpuClear8(string2, sizeof(string2));

        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
        {
			char* pExtension;

			// �g���q�̃`�F�b�N
			pExtension = STD_SearchCharReverse( info->longname, '.');
			if (pExtension)
			{
				if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
				{

					STD_TSPrintf(string1, "List %d ", ++listNo);
					STD_ConvertStringSjisToUnicode(string2, NULL, string1, NULL, NULL);

					NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_WHITE, 0, 60, 256, 20);
					NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
						TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");
					NNS_G2dTextCanvasDrawText(&gTextCanvas, 135, 60,
						TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)string2);

					STD_TSNPrintf(full_path, sizeof(full_path), "%s/%s", directory, info->longname);
//            		kamiFontPrintfConsole(CONSOLE_GREEN, "  %s\n", full_path);

					// MAX_RETRY_COUNT�܂Ń��g���C����
					for (j=0; j<MAX_RETRY_COUNT; j++)
					{	
						nam_result = kamiImportTad(full_path, j);
						if (nam_result == NAM_OK)
						{
							break;
						}
						else
						{
							kamiFontPrintfConsole(CONSOLE_GREEN, "Import %d Retry!\n", listNo);
						}
					}

					if ( nam_result == NAM_OK)
					{
						kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d Import Success.\n", listNo);			
					}
					else
					{
						kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d : RetCode = %d\n", listNo, nam_result );
						result = FALSE;
					}

					kamiFontLoadScreenData();
				}
			}
        }
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad �t�@�C���C���|�[�g

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static s32 kamiImportTad(const char* path, BOOL erase)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	// tad�t�@�C���̏��擾
	nam_result = NAM_ReadTadInfo(&tadInfo, path);
	if ( nam_result != NAM_OK )
	{
		return nam_result;
	}

	// ES�̎d�l�ŌÂ� e-ticket ������ƐV���� e-ticket ���g�����C���|�[�g���ł��Ȃ�
	// �b��Ή��Ƃ��ĊY���^�C�g�������S�폜���Ă���C���|�[�g����
	if (erase)
	{
		nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
		if ( nam_result != NAM_OK )
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
			return FALSE;
		}
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

	// ������ƕ\������
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
    OS_WaitVBlankIntr();

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
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
        OS_WaitVBlankIntr();

		// 3D������
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
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

	// �O���[���o�[
	DrawQuadWithColors( 30,  86,   x,  95, GX_RGB(22, 31, 22), GX_RGB(12, 25, 12));

	// �O���[�o�[
	DrawQuad( 30,  86, 226,  95, GX_RGB(28, 28, 28));
}
