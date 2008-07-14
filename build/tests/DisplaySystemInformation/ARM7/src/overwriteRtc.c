/*---------------------------------------------------------------------------*
  Project:  TwlSDK - components - armadillo.TWL
  File:     overwriteRtc.c

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

#include    <twl/types.h>
#include    <nitro/rtc/common/fifo.h>
#include 	<twl/memorymap_sp.h>
#include 	<twl/hw/common/mmap_wramEnv.h>
#include 	<twl/code32/j>

extern BOOL RTC_IsAvailablePxiCommand(u16 command);
extern void		INIT_InitializeScfg(void);


/*---------------------------------------------------------------------------*
  Name:         RTC_IsAvailablePxiCommand

  Description:  PXI �R�}���h���L���� RTC ����R�}���h�ł��邩�ǂ�����������B

  Arguments:    command -   ��������R�}���h�B

  Returns:      BOOL    -   �L���ȃR�}���h�ł���ꍇ�� TRUE ��Ԃ��B�����ł���
                            �������ȃR�}���h�ł���ꍇ�ɂ� FALSE ��Ԃ��B
 *---------------------------------------------------------------------------*/
BOOL
RTC_IsAvailablePxiCommand(u16 command)
{
    switch (command)
    {
    case RTC_PXI_COMMAND_RESET:
    case RTC_PXI_COMMAND_SET_HOUR_FORMAT:
    case RTC_PXI_COMMAND_READ_DATETIME:
    case RTC_PXI_COMMAND_READ_DATE:
    case RTC_PXI_COMMAND_READ_TIME:
    case RTC_PXI_COMMAND_READ_PULSE:
    case RTC_PXI_COMMAND_READ_ALARM1:
    case RTC_PXI_COMMAND_READ_ALARM2:
    case RTC_PXI_COMMAND_READ_STATUS1:
    case RTC_PXI_COMMAND_READ_STATUS2:
    case RTC_PXI_COMMAND_READ_ADJUST:
    case RTC_PXI_COMMAND_READ_FREE:
    case RTC_PXI_COMMAND_WRITE_DATETIME:
    case RTC_PXI_COMMAND_WRITE_DATE:
    case RTC_PXI_COMMAND_WRITE_TIME:
    case RTC_PXI_COMMAND_WRITE_PULSE:
    case RTC_PXI_COMMAND_WRITE_ALARM1:
    case RTC_PXI_COMMAND_WRITE_ALARM2:
    case RTC_PXI_COMMAND_WRITE_STATUS1:
    case RTC_PXI_COMMAND_WRITE_STATUS2:
    case RTC_PXI_COMMAND_WRITE_ADJUST:
    case RTC_PXI_COMMAND_WRITE_FREE:
        return TRUE;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         INIT_InitializeScfg

  Description:  ���[�_�[���s�� SCFG �u���b�N�ւ̐ݒ菈�����s����ׂ̊֐��B
                crt0 ���̎�V���{�����㏑������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

asm void
INIT_InitializeScfg(void)
{
        /* A7-SCFG �̃A�N�Z�X�۔��� */
        ldr         r1, =REG_EXT_ADDR
        ldr         r0, [r1]
        ldr         r2, =HW_PRV_WRAM_SYSRV
        tst         r0, #REG_SCFG_EXT_CFG_MASK
        beq         @invalid

@valid:
        /* ARM7 Secure-ROM �؂藣�� */
        ldr         r1, =REG_A7ROM_ADDR
        ldrb        r0, [r1]
        orr         r0, r0, #REG_SCFG_A7ROM_SEC_MASK
        strb        r0, [r1]

        /* ARM9 Secure-ROM �؂藣�� */
        ldr         r1, =REG_A9ROM_ADDR
        ldrb        r0, [r1]
        orr         r0, r0, #REG_SCFG_A9ROM_SEC_MASK
        strb        r0, [r1]

        /* NITRO ������L���� */
        ldr         r1, =REG_WL_ADDR
        ldrh        r0, [r1]
        orr         r0, r0, #REG_SCFG_WL_OFFB_MASK
        strh        r0, [r1]

        /* ROM �ݒ�ANITRO �����ݒ�A�{���f�B���O�I�v�V�������� WRAM �ɓW�J */
        ldr         r3, =REG_OP_ADDR        // SCFG-OP
        ldrb        r1, [r3]
        and         r0, r1, #(REG_SCFG_OP_OP1_MASK | REG_SCFG_OP_OP0_MASK)
        ldr         r3, =REG_A9ROM_ADDR     // SCFG-ROM:0~7
        ldrb        r1, [r3]
        and         r3, r1, #(REG_SCFG_A9ROM_RSEL_MASK | REG_SCFG_A9ROM_SEC_MASK)
        orr         r0, r0, r3, LSL #(HWi_WSYS08_ROM_ARM9RSEL_SHIFT - REG_SCFG_A9ROM_RSEL_SHIFT)
        ldr         r3, =REG_A7ROM_ADDR     // SCFG-ROM:8~15
        ldrb        r1, [r3]
        and         r3, r1, #(REG_SCFG_A7ROM_RSEL_MASK | REG_SCFG_A7ROM_FUSE_MASK)
        orr         r0, r0, r3, LSL #(HWi_WSYS08_ROM_ARM7RSEL_SHIFT - REG_SCFG_A7ROM_RSEL_SHIFT)
        ldr         r3, =REG_WL_ADDR        // SCFG-WL
        ldrb        r1, [r3]
        and         r3, r1, #REG_SCFG_WL_OFFB_MASK
        orr         r0, r0, r3, LSL #(HWi_WSYS08_WL_OFFB_SHIFT - REG_SCFG_WL_OFFB_SHIFT)
        strb        r0, [r2, #HWi_WSYS08_WRAMOFFSET]

        /* �e�V�K�u���b�N�փN���b�N������ */
        ldr         r1, =REG_CLK_ADDR
        ldrh        r0, [r1]
        ldr         r3, =REG_SCFG_CLK_SNDMCLK_MASK | REG_SCFG_CLK_WRAMHCLK_MASK | REG_SCFG_CLK_AESHCLK_MASK | REG_SCFG_CLK_SD2HCLK_MASK | REG_SCFG_CLK_SD1HCLK_MASK
        orr         r0, r0, r3
        strh        r0, [r1]

        /* JTAG ���� WRAM �ɓW�J */
        ldr         r3, =REG_JTAG_ADDR      // SCFG-JTAG
        ldrh        r1, [r3]
        and         r0, r1, #(REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK)
        and         r3, r1, #REG_SCFG_JTAG_DSPJE_MASK
        orr         r0, r0, r3, LSR #(REG_SCFG_JTAG_DSPJE_SHIFT - HWi_WSYS09_JTAG_DSPJE_SHIFT)
        strb        r0, [r2, #HWi_WSYS09_WRAMOFFSET]

        /* �e�g���@�\��L���� */
        ldr         r1, =REG_EXT_ADDR       // SCFG_EXT
        ldr         r0, [r1]
        ldr         r3, =REG_SCFG_EXT_WRAM_MASK | REG_SCFG_EXT_GPIO_MASK | REG_SCFG_EXT_I2C_MASK | REG_SCFG_EXT_I2S_MASK | REG_SCFG_EXT_MIC_MASK | REG_SCFG_EXT_SD2_MASK | REG_SCFG_EXT_SD1_MASK | REG_SCFG_EXT_AES_MASK | REG_SCFG_EXT_DMAC_MASK
        orr         r0, r0, r3
        orr         r0, r0, #(REG_SCFG_EXT_DSEL_MASK | REG_SCFG_EXT_INTC_MASK)
        bic         r0, r0, #REG_SCFG_EXT_MC_B_MASK
        str         r0, [r1]

        /* �e�g���@�\�̐���ݒ���e�� WRAM �ɓW�J */
        str         r0, [r2, #HWi_WSYS04_WRAMOFFSET]

        /* �������[�J�[�h I/F �̃X���b�g�I�� */
        ldr         r1, =REG_MC1_ADDR       // SCFG_MC1
        ldr         r0, [r1]
        bic         r0, r0, #REG_MI_MC1_SWP_MASK
        str         r0, [r1]

        /* WRAM-A/B/C �� ARM7 �Ɋ��蓖�����Ă��邱�Ƃ��m�F */
        ldr         r1, =REG_MBK1_ADDR
        ldr         r0, [r1]
        ldr         r3, =0x8d898581
        cmp         r0, r3
        blne        INITi_Stop
        ldr         r1, =REG_MBK2_ADDR
        ldr         r0, [r1]
        ldr         r3, =0x8d898581
        cmp         r0, r3
        blne        INITi_Stop
        ldr         r1, =REG_MBK3_ADDR
        ldr         r0, [r1]
        ldr         r3, =0x9d999591
        cmp         r0, r3
        blne        INITi_Stop
        ldr         r1, =REG_MBK4_ADDR
        ldr         r0, [r1]
        ldr         r3, =0x8d898581
        cmp         r0, r3
        blne        INITi_Stop
        ldr         r1, =REG_MBK5_ADDR
        ldr         r0, [r1]
        ldr         r3, =0x9d999591
        cmp         r0, r3
        blne        INITi_Stop

        /* WRAM-A/B/C �̃������}�b�v��ݒ� */
        ldr         r1, =REG_MBK6_ADDR
        ldr         r0, =0x080037c0
        str         r0, [r1]
        ldr         r1, =REG_MBK7_ADDR
        ldr         r0, =0x07c03780
        str         r0, [r1]
        ldr         r1, =REG_MBK8_ADDR
        ldr         r0, =0x07803740
        str         r0, [r1]
        ldr         r1, =REG_MBK9_ADDR
        ldr         r0, =0x00ffff0f
        str         r0, [r1]

        /* A7-SCFG �u���b�N�ւ̃A�N�Z�X�𖳌��� */
//        ldr         r1, =REG_EXT_ADDR
//        ldr         r0, [r1]
//        bic         r0, r0, #REG_SCFG_EXT_CFG_MASK
//        str         r0, [r1]

@invalid:
        /* ROM �ݒ�ANITRO �����ݒ���e���m�F */
        ldrb        r0, [r2, #HWi_WSYS08_WRAMOFFSET]
        and         r0, r0, #(HWi_WSYS08_ROM_ARM7RSEL_MASK | HWi_WSYS08_ROM_ARM9RSEL_MASK | HWi_WSYS08_ROM_ARM9SEC_MASK)
        cmp         r0, #HWi_WSYS08_ROM_ARM9SEC_MASK
        blne        INITi_Stop

        /* SCFG ���W�X�^�ݒ�������L�̈�ɃR�s�[ */
        ldr         r2, =HW_PRV_WRAM_SYSRV
        ldr         r3, =HW_SYS_CONF_BUF
        ldr         r0, [r2, #HWi_WSYS04_WRAMOFFSET]
        str         r0, [r3, #HWi_WSYS04_OFFSET]
        ldrh        r0, [r2, #HWi_WSYS08_WRAMOFFSET]
        strh        r0, [r3, #HWi_WSYS08_OFFSET]

        bx          lr
}
