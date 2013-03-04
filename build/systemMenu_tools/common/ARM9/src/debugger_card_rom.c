/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     debugger_card_rom.c

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

#include "debugger_card_rom.h"

/*---------------------------------------------------------------------------*/
/* constants */

#define CARD_COMMAND_ID             0x07000000
#define CARD_COMMAND_MASK           0x07000000
#define CARD_RESET_HI               0x20000000
#define CARD_COMMAND_OP_G_READID    0xB8

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetRomFlag

  Description:  �J�[�h�R�}���h�R���g���[���p�����[�^���擾

  Arguments:    flag       �J�[�h�f�o�C�X�֔��s����R�}���h�̃^�C�v
                           (CARD_COMMAND_PAGE / CARD_COMMAND_ID /
                            CARD_COMMAND_STAT / CARD_COMMAND_REFRESH)

  Returns:      �J�[�h�R�}���h�R���g���[���p�����[�^
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 CARDi_GetRomFlag(u32 flag)
{
    u32     rom_ctrl = *(vu32 *)(HW_CARD_ROM_HEADER + 0x60);
    return (u32)(flag | REG_MI_MCCNT1_START_MASK | CARD_RESET_HI | (rom_ctrl & ~CARD_COMMAND_MASK));
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetRomOp

  Description:  �J�[�h�R�}���h�ݒ�

  Arguments:    command    �R�}���h
                offset     �]���y�[�W��

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SetRomOp(u32 command, u32 offset)
{
    u32     cmd1 = (u32)((offset >> 8) | (command << 24));
    u32     cmd2 = (u32)((offset << 24));
    // �O�̂��ߑO���ROM�R�}���h�̊����҂��B
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK) != 0)
    {
    }
    // �}�X�^�[�C�l�[�u���B
    reg_MI_MCCNT0 = (u16)(REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK |
                          (reg_MI_MCCNT0 & ~REG_MI_MCCNT0_SEL_MASK));
    // �R�}���h�ݒ�B
    reg_MI_MCCMD0 = MI_HToBE32(cmd1);
    reg_MI_MCCMD1 = MI_HToBE32(cmd2);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomIDCoreEx

  Description:  �J�[�h ID �̓ǂݏo���B

  Arguments:    dontCare : gRD_ID�� don't care �r�b�g�ɃZ�b�g����l

  Returns:      �J�[�h ID
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomIDCoreEx(u32 dontCare)
{
    u8 op = CARD_COMMAND_OP_G_READID;

    CARDi_SetRomOp(op, dontCare);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_ID) & ~REG_MI_MCCNT1_L1_MASK);
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_RDY_MASK) == 0)
    {
    }
    return reg_MI_MCD1;
}

