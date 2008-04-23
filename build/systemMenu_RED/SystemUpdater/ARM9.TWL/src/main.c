/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     main.c

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
#include "kami_pxi.h"
#include "kami_font.h"
#include "kami_write_nandfirm.h"
#include "import.h"
#include "hw_info.h"
#include "graphics.h"
#include "hwi.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/

static const char* ImportTadFileList[] =
{
	"rom:/data/HNAA.tad",
	"rom:/data/HNBA.tad",
	"rom:/data/HNCA.tad"
};

static const char* NandFirmPath = "rom:/data/menu_launcher.nand";

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static NAMTitleId   titleId;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static BOOL IgnoreRemoval(void);
static void DrawWaitButtonA(void);
static void DrawAlready(void);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
TwlMain()
{
	BOOL result;
	int tadNum;
	int i;

    OS_Init();
    OS_InitArena();
    PXI_Init();
    OS_InitLock();
    OS_InitArenaEx();
    OS_InitIrqTable();
    OS_SetIrqStackChecker();
    MI_Init();
    OS_InitVAlarm();
    OSi_InitVramExclusive();
    OS_InitThread();
    OS_InitReset();
    GX_Init();
    FX_Init();
    SND_Init();
    TP_Init();
    RTC_Init();

	InitAllocation();

    KamiPxiInit();   /* �Ǝ�PXI������ */

    // V�u�����N���荞�ݐݒ�
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

	// NAM���C�u����������
	NAM_Init( OS_AllocFromMain, OS_FreeToMain);

    // �\���֘A������
    InitGraphics();
	kamiFontInit();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	// ���O�����݂���Ȃ�V�X�e���X�V�ς݂Ɣ���
	{
		FSFile file;
		FS_InitFile( &file );
		if (FS_OpenFileEx(&file, "nand:/sys/log/updater.log", FS_FILEMODE_R) == TRUE)
		{
			FS_CloseFile(&file);
			DrawAlready();
		}
	}

	// �`�{�^���҂�
	DrawWaitButtonA();

	// HWInfo�֘A�̑O����
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
		OS_Warning(" Fail! : HWI_INIT()");
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
		break;
	}

	result = TRUE;

	// �S�n�[�h�E�F�A���̍X�V
	if (WriteHWInfoFile(OS_GetRegion(), OS_IsForceDisableWireless()) == FALSE)
	{
		result = FALSE;
		kamiFontPrintfConsole(CONSOLE_RED, "Hardware Update Failure!\n");		
	}

	// TAD�̃C���|�[�g�J�n
	tadNum = sizeof(ImportTadFileList)/sizeof(ImportTadFileList[0]);

	for (i=0; i<tadNum; i++)
	{
		s32  nam_result = kamiImportTad(i+1, tadNum, ImportTadFileList[i]);

		if ( nam_result == NAM_OK)
		{
			kamiFontPrintf( 0, (s16)i, FONT_COLOR_GREEN, "List : %d Update Success.", i+1 );			
		}
		else
		{
			kamiFontPrintf( 0, (s16)i, FONT_COLOR_RED, "Error: %d : RetCode = %d", i+1, nam_result );
			result = FALSE;
		}
	}

	// NAND�t�@�[���̃C���X�g�[���J�n
	if( kamiWriteNandfirm(NandFirmPath, OS_AllocFromMain, OS_FreeToMain))
	{
			kamiFontPrintf( 0, (s16)i, FONT_COLOR_GREEN, "Firm Update Success.");			
	}
	else
	{
		kamiFontPrintf( 0, (s16)i, FONT_COLOR_RED, "Firm Update Failure!");
		result = FALSE;
	}
	kamiFontLoadScreenData();

	// �����ɕs�ւȂ̂ňꎞ�I�ɍ폜
/*
	// �X�V���O���c���čĎ��s��h��
	if (result)
	{
		(void)FS_CreateFileAuto("nand:/sys/log/updater.log", FS_PERMIT_R | FS_PERMIT_W);
	}
*/
	// ���ʕ\��
	while(1)
	{
		DrawResult(result);
	    OS_WaitVBlankIntr();
	};
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank���荞�ݏ���

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocation

  Description:  �q�[�v�̏�����.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocation(void)
{
    void   *tmp;
    OSHeapHandle hh;

    /* �A���[�i�̏����� */
    tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tmp);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
        OS_Panic("ARM9: Fail to create heap...\n");
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         DrawWaitButtonA

  Description:  A�{�^���҂���\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawWaitButtonA(void)
{
	kamiFontPrintfMain( 5,  3, 8, "--- System Updater ---");

	kamiFontPrintfMain( 5,  8, 3, "<Push A: Start Update>");
	kamiFontPrintfMain( 3, 10, 1, "--------------------------");
	kamiFontPrintfMain( 3, 11, 1, "Do not turn off power");
	kamiFontPrintfMain( 3, 12, 1, "while update is processing");
	kamiFontPrintfMain( 3, 13, 1, "--------------------------");
	kamiFontLoadScreenData();

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  50, 246, 120, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

		kamiPadRead();
		if (kamiPadIsTrigger(PAD_BUTTON_A))
		{
			kamiFontClearMain();
			break;
		}
	    OS_WaitVBlankIntr();
	}
}

/*---------------------------------------------------------------------------*
  Name:         DrawAlready

  Description:  Already��\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawAlready(void)
{
	kamiFontPrintfMain( 8,  8, 3, "<Infomation>");
	kamiFontPrintfMain( 3, 10, 1, "--------------------------");
	kamiFontPrintfMain( 3, 11, 1, "This machine has already");
	kamiFontPrintfMain( 3, 12, 1, "been updated.");
	kamiFontPrintfMain( 3, 13, 1, "--------------------------");
	kamiFontLoadScreenData();

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  50, 246, 120, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	    OS_WaitVBlankIntr();
	}
}

