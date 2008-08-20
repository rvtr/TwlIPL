/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
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
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "process_format.h"
#include "process_topmenu.h"
#include "graphics.h"
#include "keypad.h"
#include "kami_pxi.h"
#include "sd_event.h"
#include "process_fade.h"
#include "hwi.h"

#define SCRAMBLE_MASK 0x00406000

extern void HWInfoWriterInit( void );

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static Process sProcess;
static FSEventHook  sSDHook;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
TwlMain()
{
    // ���i�r���h�����`���[���f�o�b�K��ł̋N���Ή�
    if ( OS_GetRunningConsoleType() & OS_CONSOLE_TWLDEBUGGER )
    {
        ROM_Header *dh = (void *)HW_ROM_HEADER_BUF;
        dh->s.game_cmd_param &= ~SCRAMBLE_MASK;
    }

    OS_Init();
	OS_InitTick();
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
	SNDEX_Init();
    TP_Init();
    RTC_Init();

#ifndef NAND_INITIALIZER_LIMITED_MODE
    KamiPxiInit();   /* �Ǝ�PXI������ */
#endif

    // V�u�����N���荞�ݐݒ�
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
	
    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);
	// SD�J�[�h�̑}���C�x���g�Ď��R�[���o�b�N�ݒ�
//  FS_RegisterEventHook("sdmc", &sSDHook, SDEvents, NULL);

	// FS_Init�̌�̕����ǂ��͗l
	InitAllocation();


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

	// �����V�[�P���X�ݒ�
	sProcess = TopmenuProcess0;

	kamiFontPrintfConsole( CONSOLE_ORANGE, "How to \n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "+---------------------------+\n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "l A Button    : Select Menu l\n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "l Up/Down Key : Change Menu l\n");
#ifndef NAND_INITIALIZER_LIMITED_MODE
	kamiFontPrintfConsole( CONSOLE_ORANGE, "l L&R Button  : Auto Init   l\n");
#endif
	kamiFontPrintfConsole( CONSOLE_ORANGE, "+---------------------------+\n");

#ifdef AUTO_FORMAT_MODE
//  �����H���ł�NAND������������Ă��Ȃ������̏�Ԃ�FAT�ɃA�N�Z�X�����
//  ��肪���邽�ߋ����I�Ƀt�H�[�}�b�g���s��
	ExeFormat(FORMAT_MODE_QUICK);
#endif

	// NAM���C�u����������
	NAM_Init( OS_AllocFromMain, OS_FreeToMain);
	NAMUT_Init( OS_AllocFromMain, OS_FreeToMain);

	// HWInfo�֘A�̑O����
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
		kamiFontPrintfConsoleEx(CONSOLE_RED, "HWI_INIT() Failure!\n" );
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, "[PRO Signature MODE]\n" );
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, "[DEV Signature MODE]\n" );
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
		kamiFontPrintfConsoleEx(CONSOLE_RED, "[No Signature MODE]\n" );
		break;
	}


    while (1)
    {
		kamiPadRead();

        // �R�}���h�t���b�V��
//      (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

		// V�u�����N�҂�
        OS_WaitVBlankIntr();

        // �`�q�l�V�R�}���h������M
//      while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
//      {
//      }

		// �t�H���g�X�N���[���f�[�^���[�h
		kamiFontLoadScreenData();

		sProcess = sProcess();
    }
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

