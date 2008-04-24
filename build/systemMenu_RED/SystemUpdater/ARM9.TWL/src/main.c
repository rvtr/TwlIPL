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

// ���g���C��
#define MAX_RETRY_COUNT   10

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
static void DrawAlready(char* date);

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
	BOOL hw_info_result;
	BOOL nand_firm_result;
	int tadNum;
	int i,j;

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

    KamiPxiInit();   /* �Ǝ�PXI������ */

    // V�u�����N���荞�ݐݒ�
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

	InitAllocation();

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
			char date[sizeof(__DATE__)];
			if (FS_ReadFile(&file, date, sizeof(__DATE__)) != sizeof(__DATE__))
			{
				OS_Warning("Failure! FS_ReadFile");
			}
			FS_CloseFile(&file);
			DrawAlready(date);
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
	for (i=0;i<MAX_RETRY_COUNT;i++)
	{
		hw_info_result = WriteHWInfoFile(OS_GetRegion(), OS_IsForceDisableWireless());
		if (hw_info_result)
		{
			kamiFontPrintf( 0, (s16)0, FONT_COLOR_GREEN, "Write Hardware Info Success.");			
			break;
		}
		else
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Write Hardware Info Retry!\n");
		}
	}
	if ( hw_info_result == FALSE)
	{
		result = FALSE;
		kamiFontPrintf( 0, (s16)0, FONT_COLOR_RED, "Write Hardware Info Failure!");			
	}

	// TAD�̃C���|�[�g�J�n
	tadNum = sizeof(ImportTadFileList)/sizeof(ImportTadFileList[0]);

	for (i=0; i<tadNum; i++)
	{
		s32  nam_result;
	
		// MAX_RETRY_COUNT�܂Ń��g���C����
		for (j=0; j<MAX_RETRY_COUNT; j++)
		{	
			nam_result = kamiImportTad(i+1, tadNum, ImportTadFileList[i]);
			if (nam_result == NAM_OK)
			{
				break;
			}
			else
			{
				kamiFontPrintfConsole(CONSOLE_RED, "Import %d Retry!\n", i+1);
			}
		}

		if ( nam_result == NAM_OK)
		{
			kamiFontPrintf( 0, (s16)(i+1), FONT_COLOR_GREEN, "List : %d Update Success.", i+1 );			
		}
		else
		{
			kamiFontPrintf( 0, (s16)(i+1), FONT_COLOR_RED, "Error: %d : RetCode = %d", i+1, nam_result );
			result = FALSE;
		}
	}

	// NAND�t�@�[���̃C���X�g�[���J�n
	for (j=0;j<MAX_RETRY_COUNT;j++)
	{
		nand_firm_result = kamiWriteNandfirm(NandFirmPath, OS_AllocFromMain, OS_FreeToMain);
		if (nand_firm_result)
		{
			kamiFontPrintf( 0, (s16)(i+1), FONT_COLOR_GREEN, "Firm Update Success.");			
			break;
		}
		else
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Write Firm Retry!\n");
		}
	}
	if ( nand_firm_result == FALSE)
	{
		result = FALSE;
		kamiFontPrintf( 0, (s16)(i+1), FONT_COLOR_RED, "Firm Update Failure!");
	}

	// �����ɕs�ւȂ̂ňꎞ�I�ɍ폜

	// �X�V���O���쐬���čĎ��s��h��
	if (result)
	{
		if (FS_CreateFileAuto("nand:/sys/log/updater.log", FS_PERMIT_R | FS_PERMIT_W))
		{
			FSFile file;
			FS_InitFile( &file );
			if (FS_OpenFileEx(&file, "nand:/sys/log/updater.log", FS_FILEMODE_W))
			{
				char date[sizeof(__DATE__)];
				if (STD_CopyLString( date, __DATE__, sizeof(__DATE__) ) != (sizeof(__DATE__)-1))
				{
					OS_Warning("Failure! STD_CopyLString\n");
				}
				if (FS_WriteFile(&file, (void*)date, sizeof(__DATE__) ) == -1)
				{
					OS_Warning("Failure : FS_WriteFile\n");
				}
				FS_CloseFile(&file);
			}
		}
	}

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
	kamiFontLoadScreenData();
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
	kamiFontPrintfMain( 5,  3, 8, "    System Updater    ");
	kamiFontPrintfMain( 5,  5, 8, " --- %s ---", __DATE__);

	kamiFontPrintfMain( 5,  8, 3, "<Push A: Start Update>");
	kamiFontPrintfMain( 3, 10, 1, "--------------------------");
	kamiFontPrintfMain( 3, 11, 1, "Do not turn off power");
	kamiFontPrintfMain( 3, 12, 1, "while update is processing");
	kamiFontPrintfMain( 3, 13, 1, "--------------------------");

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  54, 246, 120, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

		kamiPadRead();
		if (kamiPadIsTrigger(PAD_BUTTON_A))
		{
			kamiFontClearMain();
			break;
		}
	    OS_WaitVBlankIntr();
	}

	G3X_Reset();
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	OS_WaitVBlankIntr();
}

/*---------------------------------------------------------------------------*
  Name:         DrawAlready

  Description:  Already��\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawAlready(char* date)
{
	kamiFontPrintfMain( 3,  8, 1, "--------------------------");
	kamiFontPrintfMain( 3,  9, 1, "This machine has already");
	kamiFontPrintfMain( 3, 10, 1, "been updated.");
	kamiFontPrintfMain( 3, 12, 1, "version: %s", date);
	kamiFontPrintfMain( 3, 13, 1, "--------------------------");

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  50, 246, 128, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	    OS_WaitVBlankIntr();
	}
}
