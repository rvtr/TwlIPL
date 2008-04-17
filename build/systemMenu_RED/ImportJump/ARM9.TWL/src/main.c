/*---------------------------------------------------------------------------*
  Project:  ImportJump
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
#include "kami_font.h"
#include "import.h"
#include "graphics.h"
#include "hwi.h"
#include "ImportJump.h"

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static NAMTitleId   titleId;
char sTadPath[FS_ENTRY_LONGNAME_MAX];

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
	// OS_Init���O�Ɏ��s����
	{
		// SRL�̌���ɔz�u����TAD�t�@�C���ɃA�N�Z�X�\�ɂ��邽�߂�
		// �J�[�h�A�N�Z�X�̃n�b�V���`�F�b�N�𖳌�������
	    const CARDRomHeaderTWL *header = (const CARDRomHeaderTWL *)HW_TWL_ROM_HEADER_BUF;
        *(u32 *)header->digest_tabel2_digest = 0x00000000;
		// �f�o�b�K����ǂݎ�邽�ߊg����������L���ɂ���
		OS_EnableMainExArena();
	}

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

	// magicCode���قȂ�ꍇ�͒�~
	if (STD_CompareNString( (char *)&GetImportJumpSetting()->magicCode, "TWLD", 4 ))
	{
		OS_Warning(" Magic Code Wrong!\n");
		while(1){};
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
	if (kamiImportTad())
	{
		// �C���|�[�g�ɐ��������Ȃ�A�v���W�����v
		OS_DoApplicationJump( GetImportJumpSetting()->bootTitleID, OS_APP_JUMP_NORMAL );
	}

	// �A�v���W�����v�ɐ��������Ȃ炱���ւ͓��B���Ȃ�
	while(1){};
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
  Name:         IgnoreRemoval

  Description:  �J�[�h�����R�[���o�b�N

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
// �����_�ł̓J�[�h���荞�݂�HIO�ʒm�Ɏg�p���Ă���悤�Ȃ̂�
// �J�[�h�����댟�o�𖳎�����悤�ɂ��ăe�X�g���쐬���Ă���B
static BOOL IgnoreRemoval(void)
{
    OS_TWarning("detected CARD-removal!(miss-notification from debugger Host-I/O)\n");
    return FALSE;
}
