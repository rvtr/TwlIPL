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
#include "graphics.h"
#include "hwi.h"

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/

static const char* ImportTadFileList[] =
{
	"rom:/data/HNAA.tad",
	"rom:/data/HNBA.tad",
	"rom:/data/HNCA.tad"
};

static const char* NandFirmPath = "rom:/data/menu_launcher-RELEASE.nand";

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
extern void FS_MountHostIO(const char *basepath);

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

	// HWInfo�֘A�̑O����
	// InstalledSoftBoxCount, FreeSoftBoxCount �̍X�V�̂��߂ɕK�v
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

	// TAD�̃C���|�[�g�J�n
	result = TRUE;
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
