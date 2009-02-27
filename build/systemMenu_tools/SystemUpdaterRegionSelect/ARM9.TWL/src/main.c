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
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>
#include <twl/nam.h>
#include "kami_pxi.h"
#include "kami_font.h"
#include "kami_copy_file.h"
#include "graphics.h"
#include "hwi.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "font.h"
#include "kami_global.h"

#define SCRAMBLE_MASK 0x00406000

extern void InitFont(void);

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

s32       gLockId;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static BOOL IgnoreRemoval(void);

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

    // ���i�r���h�����`���[���f�o�b�K��ł̋N���Ή�
    if ( OS_GetRunningConsoleType() & OS_CONSOLE_TWLDEBUGGER )
    {
        ROM_Header *dh = (void *)HW_ROM_HEADER_BUF;
        dh->s.game_cmd_param &= ~SCRAMBLE_MASK;
    }

    OS_Init();
	OS_InitThread();
	OS_InitTick();
	OS_InitAlarm();
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
	NAM_Init( OS_AllocFromMain, OS_FreeToMain );
	NAMUT_Init( OS_AllocFromMain, OS_FreeToMain );	// SoftBoxCount�̌v�Z�ɕK�v

    // �\���֘A������
    InitGraphics();
	kamiFontInit();
	kamiFontPrintfConsole(FONT_COLOR_GREEN, "Log Window:\n");

	// ���C���X���b�h�̃J�[�h���b�NID�擾
	gLockId = OS_GetLockID();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	// �t�H���g������
	InitFont();

	// �R���\�[���`�F�b�N
	ProcessCheckConsole();

	// ���O���m�F
	ProcessCheckLog();

	// ���[�W�����I��
	ProcessSelectRegion();

	// Note�\��
	ProcessNote();

	// TWL�̍X�V���������s���ł�
	CARD_LockRom((u16)gLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_NOW_UPDATE);
	CARD_UnlockRom((u16)gLockId);

	// IS�f�o�b�K�̃n�[�h�E�F�A���Z�b�g���֎~����
    DEBUGGER_HwResetDisable();

	// HWInfo�֘A�̑O����
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
		kamiFontPrintfConsole(FONT_COLOR_RED, " Fail! : HWI_INIT()\n");
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
		break;
	}

	result = TRUE;

	// NAND�̃t�H�[�}�b�g���Â���΃t�H�[�}�b�g
	result &= ProcessFormat();

	// �S�n�[�h�E�F�A���̍X�V
	result &= ProcessHwinfo();

	// �t�H���g�̏�������
	result &= ProcessWriteFont();

	// cert.sys�̏�������
	result &= ProcessWriteCert();
	
	// �_�~�[�t�@�C���̐���
	result &= ProcessWriteDummy();

	// TAD�̃C���|�[�g�J�n
	result &= ProcessImport( OS_AllocFromMain, OS_FreeToMain );

	// �I�����[�W�����ȊO��SystemMenu�̏������s��
	result &= ProcessDeleteOtherResionSysmenu();

	// NAND�t�@�[���̃C���X�g�[���J�n
	result &= ProcessNandfirm();

	// �{�̏��������s��
	result &= ProcessNamutFormat();

	// �X�V���O���쐬����VersionDown��h��
	if (result)
	{
		ProcessLog();
	}

	// IS�f�o�b�K�̃n�[�h�E�F�A���Z�b�g��������
    DEBUGGER_HwResetEnable();

	// ���ʕ\��
	ProcessFinish( result );
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
//	kamiFontLoadScreenData();
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
