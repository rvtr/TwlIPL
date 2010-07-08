/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - init
  File:     crt0_firm.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include    <firm.h>
#include    <twl/hw/common/mmap_wramEnv.h>

#include    <twl/code32.h>

void    _start(void);
void    _start_AutoloadDoneCallback(void *argv[]);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

/* �O���֐��Q�ƒ�` */
extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);

static void INITi_CpuClear32(register u32 data, register void *destp, register u32 size);
static void INITi_CpuClearFast(register u32 data, register void* destp, register u32 size);
static void INITi_DoAutoload(void);
static void INITi_ShelterLtdBinary(void);
static void INITi_CopySysConfig( void );
static void INITi_DetectMainMemorySize(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32* ptr);
static void INITi_CallStaticInitializers(void);
#endif

/* �����J�X�N���v�g�ɂ���`�����V���{���Q�� */
extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);

void   *const _start_ModuleParams[]     =
{
    (void *)SDK_AUTOLOAD_LIST,
    (void *)SDK_AUTOLOAD_LIST_END,
    (void *)SDK_AUTOLOAD_START,
    (void *)SDK_STATIC_BSS_START,
    (void *)SDK_STATIC_BSS_END,
    (void*)0,       // CompressedStaticEnd. This fixed number will be updated by compstatic tool.
    (void*)0,       // SDK_VERSION_ID   // SDK version info /* [TODO] �r���h��ʂ����� */
    (void*)SDK_NITROCODE_BE,
    (void*)SDK_NITROCODE_LE,
};

extern void SDK_LTDAUTOLOAD_LIST(void);
extern void SDK_LTDAUTOLOAD_LIST_END(void);
extern void SDK_LTDAUTOLOAD_START(void);

void* const _start_LtdModuleParams[]    =
{
    (void*)SDK_LTDAUTOLOAD_LIST,
    (void*)SDK_LTDAUTOLOAD_LIST_END,
    (void*)SDK_LTDAUTOLOAD_START,
    (void*)0,       // CompressedLtdautoloadEnd. This fixed number will be updated by compstatic tool.
    (void*)SDK_TWLCODE_BE,
    (void*)SDK_TWLCODE_LE,
};

/*---------------------------------------------------------------------------*
  Name:         _start

  Description:  Start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void _start( void )
{
        //---- set IME = 0
        //     ( use that LSB of HW_REG_BASE equal to 0 )
        mov             r12, #HW_REG_BASE
        str             r12, [r12, #REG_IME_OFFSET]

        //---- initialize stack pointer
        // SVC mode
        mov             r0, #HW_PSR_SVC_MODE
        msr             cpsr_c, r0
        ldr             sp, =HW_FIRM_SVC_STACK_END

        // IRQ mode
        mov             r0, #HW_PSR_IRQ_MODE
        msr             cpsr_c, r0
        ldr             r0, =HW_FIRM_IRQ_STACK_END
        mov             sp, r0

        // System mode
        ldr             r1, =HW_FIRM_IRQ_STACK_SIZE
        sub             r1, r0, r1
        mov             r0, #HW_PSR_SYS_MODE
        msr             cpsr_csfx, r0
        sub             sp, r1, #4 // 4byte for stack check code

#ifdef FIRM_DISABLE_CR_AT_WARMBOOT
        //---- read reset flag from mcu
#ifdef SDK_TS
#if 0
        mov             r0, #REG_PMIC_SW_FLAGS_ADDR
        bl              PMi_GetRegister
        ands            r0, r0, #PMIC_SW_FLAGS_WARMBOOT
#else
        mov             r0, #I2C_SLAVE_MICRO_CONTROLLER
        mov             r1, #MCU_REG_TEMP_ADDR
        bl              I2Ci_ReadRegister
        ldr             r2, =HW_RESET_PARAMETER_BUF
        str             r0, [r2]    // store 4 bytes
        cmp             r0, #0
#endif
        movne           r0, #FIRM_PXI_ID_WARMBOOT
        moveq           r0, #FIRM_PXI_ID_COLDBOOT
        bl              PXIi_SendByIntf
#endif // SDK_TS
#endif // FIRM_DISABLE_CR_AT_WARMBOOT

        //---- wait for main memory mode into burst mode
        ldr             r3, =REG_EXMEMCNT_L_ADDR
        mov             r1, #REG_MI_EXMEMCNT_L_EMODE_MASK
@1:
        ldrh            r2, [r3]
        tst             r2, r1
        beq             @1

        mov             r0, #FIRM_PXI_ID_INIT_MMEM
        bl              PXIi_WaitIDByIntf

#ifndef FIRM_FOR_CTR

        //---- clear HW_TWL_MAIN_MEM_SHARED
        mov             r0, #0
        ldr             r1, =HW_TWL_MAIN_MEM_SHARED
        mov             r2, #HW_TWL_MAIN_MEM_SHARED_SIZE
        bl              INITi_CpuClearFast

#endif // FIRM_FOR_CTR

        //---- notify to clear HW_MAIN_MEM_SHARED
        mov             r0, #FIRM_PXI_ID_INIT_MMEM
        bl              PXIi_SendIDByIntf

        /* SCFG �� HW_SYS_CONF_BUF �փR�s�[ */
        bl              INITi_CopySysConfig

        //---- load autoload block and initialize bss
        //bl              INITi_DoAutoload
#ifndef SDK_FINALROM    // for IS-TWL-DEBUGGER
        bl          _start_AutoloadDoneCallback
#endif

        //---- fill static bss with 0
        ldr             r0, =_start_ModuleParams
        ldr             r1,  [r0, #12]   // BSS segment start
        ldr             r2,  [r0, #16]   // BSS segment end
        mov             r0, #0
        sub             r2, r2, r1
        bl              INITi_CpuClearFast

        //---- detect main memory size
        bl              INITi_DetectMainMemorySize

#ifndef SDK_FINALROM
        //---- set debug print window
        ldr     r1, =HW_PRINT_OUTPUT_ARM9
        mov     r0, #OS_PRINT_OUTPUT_DEFAULT_ARM9
        orr     r0, r0, #(OS_PRINT_OUTPUT_DEFAULT_ARM7<<8)
        strh    r0, [r1]
        mov     r0, #OS_PRINT_OUTPUT_DEFAULT_ARM9ERR
        orr     r0, r0, #(OS_PRINT_OUTPUT_DEFAULT_ARM7ERR<<8)
        strh    r0, [r1, #2]
#endif

        //---- set interrupt vector
        ldr             r1, =HW_INTR_VECTOR_BUF
        ldr             r0, =OS_IrqHandler
        str             r0, [r1, #0]

#ifndef SDK_NOINIT
        //---- for C++
        bl              _fp_init
        bl              TwlSpStartUp
        bl              __call_static_initializers
//        bl              INITi_CallStaticInitializers
#endif

        //---- start (to 16bit code)
        ldr             r1, =TwlSpMain
        ldr             lr, =HW_RESET_VECTOR

        bx              r1
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CpuClearFast
  Description:  32 Byte �P�ʂŃo�b�t�@�̃N���A���s���B
  Arguments:    r0  -   �N���A����l�B
                r1  -   �N���A��ւ̃|�C���^�B
                r2  -   �A�����ăN���A����o�b�t�@���B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_CpuClearFast(register u32 data, register void* destp, register u32 size)
{
        stmfd   sp!, {r4-r9}

        add     r9, r1, r2              // r9:  destEndp = destp + size
        mov     r12, r2, lsr #5         // r12: destBlockEndp = destp + size/32*32
        add     r12, r1, r12, lsl #5

        mov     r2, r0
        mov     r3, r2
        mov     r4, r2
        mov     r5, r2
        mov     r6, r2
        mov     r7, r2
        mov     r8, r2

@40:
        cmp     r1, r12                 // while (destp < destBlockEndp)
        stmltia r1!, {r0, r2-r8}        // *((vu32 *)(destp++)) = data
        blt     @40
@41:
        cmp     r1, r9                  // while (destp < destEndp)
        stmltia r1!, {r0}               // *((vu32 *)(destp++)) = data
        blt     @41

        ldmfd   sp!, {r4-r9}
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CpuClear32
  Description:  32 bit �P�ʂŃo�b�t�@�̃N���A���s���B
  Arguments:    r0  -   �N���A����l�B
                r1  -   �N���A��ւ̃|�C���^�B
                r2  -   �A�����ăN���A����o�b�t�@���B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_CpuClear32(register u32 data, register void* destp, register u32 size)
{
        add             r12, r1, r2
@001:   cmp             r1, r12
        strlt           r0, [r1], #4
        blt             @001
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_DoAutoload
  Description:  �����N���ɉ����āA�e�I�[�g���[�h�u���b�N�̌Œ�f�[�^���̓W�J
                �y�ѕϐ����� 0 �N���A���s���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
/*
 * < ��i�K�I�[�g���[�h >
 * 0x02f88000 �� crt0 �y�ш�i�ڃ��[�h���o�C�i�����z�u����Ă���B
 *  NITRO �Ƌ��L�\�� WRAM ��ɔz�u�����ׂ��o�C�i���f�[�^�� 0x037c0000 �Ƀ��[�h����B
 *  TWL �ł������삵�Ȃ� WRAM ��ɔz�u�����ׂ��o�C�i���f�[�^�𑱂��̃A�h���X�Ƀ��[�h����B
 * 0x02e80000 �ɓ�i�ڃ��[�h���o�C�i�����z�u����Ă���B
 *  0x04000 �o�C�g���̓J�[�h ROM ����ēǂݏo���s�Ȃ̂ŁA0x02f84000 - 0x02f88000 �ɑޔ�����B
 *  NITRO �Ƌ��L�\�� MAIN ��ɔz�u�����ׂ��o�C�i���f�[�^�� 0x02f88000 + sizeof(crt0) �Ƀ��[�h����B
 *  TWL �ł������삵�Ȃ� MAIN ��ɔz�u�����ׂ��o�C�i���f�[�^�𑱂��̃A�h���X�Ƀ��[�h����B
 */
static asm void
INITi_DoAutoload(void)
{
@000:
        stmdb           sp!, {lr}
        /* WRAM �p�u���b�N���I�[�g���[�h */
        ldr             r1, =_start_ModuleParams
        ldr             r12, [r1]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_AUTOLOAD_START
@001:   cmp             r12, r0
        bge             @010
        /* �Œ�Z�N�V���������[�h */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@002:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @002
        /* static initializer �e�[�u������ǂݏo�� */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           sp!, {r0-r3, r12}
#endif
        /* .bss �Z�N�V������ 0 �N���A */
        mov             r0, #0
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@003:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @003
@004:   ldmia           sp!, {r0}
        b               @001

@010:   /* ���C���������p�u���b�N�̑��݂��m�F */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1dc  /* ARM7 �p�g���풓���W���[�� ROM �T�C�Y */
        ldr             r0, [r1]
        cmp             r0, #0
        beq             @020

        /* �ēǂݏo���s������ޔ� */
        bl              INITi_ShelterLtdBinary

        /* ���C���������p�u���b�N���I�[�g���[�h */
        ldr             r1, =_start_LtdModuleParams
        ldr             r12, [r1]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_LTDAUTOLOAD_START
@011:   cmp             r12, r0
        bge             @020
        /* �Œ�Z�N�V���������[�h */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@012:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @012
        /* static initializer �e�[�u������ǂݏo�� */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           sp!, {r0-r3, r12}
#endif
        /* .bss �Z�N�V������ 0 �N���A */
        mov             r0, #0
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@013:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @013
@014:   ldmia           sp!, {r0}
        b               @011

@020:   /* �I�[�g���[�h�����R�[���o�b�N�֐��Ăяo�� */
        ldr             r0, =_start_ModuleParams
        ldr             r1, =_start_LtdModuleParams
        ldmia           sp!, {lr}
        b               _start_AutoloadDoneCallback
}

/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterLtdBinary
  Description:  TWL ��p�̃I�[�g���[�h���o�C�i���f�[�^�̓��A�J�[�h ROM ����
                �ēǂݏo���ł��Ȃ��̈�̃f�[�^��ޔ��G���A�ɑޔ�����B
                �ēǂݏo���ł��Ȃ��̈�̃f�[�^�� ARM7 �p�� ARM9 �p�̊g���풓
                ���W���[���̂Q�ɕ�����Ă���\��������̂ŁA�璷�ł͂��邪
                �����̐擪���� 0x4000 �������ꂼ��ޔ�����B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_ShelterLtdBinary(void)
{
        /* �ޔ����E��A�h���X�𒲍� */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1d8  /* ARM7 �p�g���풓���W���[�� RAM �A�h���X */
        ldr             r1, [r1]
        ldr             r3, =HW_TWL_ROM_HEADER_BUF + 0x038  /* ARM7 �p�풓���W���[�� RAM �A�h���X */
        ldr             r3, [r3]
        sub             r2, r3, #0x4000                     /* �ēǂݏo���s�̈�T�C�Y */ /* ARM7 �p�ޔ��G���A */

        /* �R�s�[ */
@loop:  ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @loop

        bx              lr
}

#ifndef SDK_NOINIT
/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterStaticInitializer
  Description:  �e�I�[�g���[�h�Z�O�����g���� static initializer �ւ̃|�C���^
                �e�[�u���� IRQ �X�^�b�N�̍ŏ㕔�ɑޔ�����B
  Arguments:    ptr     -   �Z�O�����g���̃|�C���^�e�[�u���ւ̃|�C���^�B
                            �e�[�u���� NULL �ŏI�[����Ă���K�v������B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_ShelterStaticInitializer(u32* ptr)
{
        /* �����m�F */
        cmp             r0, #0
        bxeq            lr

        /* �ޔ��ꏊ�擪�A�h���X���v�Z */
        ldr             r1, =HW_FIRM_IRQ_STACK_END
        ldr             r2, =HW_FIRM_IRQ_STACK_SIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* �ޔ��ꏊ�擪����󂫏ꏊ�𒲍� */
@001:   ldr             r2, [r1]
        cmp             r2, #0
        addne           r1, r1, #4
        bne             @001

        /* �󂫏ꏊ�Ƀe�[�u�����R�s�[ */
@002:   ldr             r2, [r0], #4
        str             r2, [r1], #4
        cmp             r2, #0
        bne             @002

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CallStaticInitializers
  Description:  �e�I�[�g���[�h�Z�O�����g���� static initializer ���Ăяo���B
                �I�[�g���[�h�����ɂ���� IRQ �X�^�b�N�̍ŏ㕔�ɑޔ�����Ă���
                �֐��|�C���^�e�[�u��������Ăяo���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_CallStaticInitializers(void)
{
        stmdb           sp!, {lr}

        /* �e�[�u���ޔ��ꏊ�擪�A�h���X���v�Z */
        ldr             r1, =HW_FIRM_IRQ_STACK_END
        ldr             r2, =HW_FIRM_IRQ_STACK_SIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* �e�[�u���ɊǗ�����Ă���|�C���^������Ăяo�� */
@001:   ldr             r0, [r1]
        cmp             r0, #0
        beq             @002
        stmdb           sp!, {r1}
        mov             lr, pc
        bx              r0
        ldmia           sp!, {r1}
        /* ��U�Ăяo�����|�C���^�̓[���N���A (IRQ�X�^�b�N���Ԏ؂肵�Ă����) */
        mov             r0, #0
        str             r0, [r1], #4
        b               @001

@002:   ldmia           sp!, {lr}
        bx              lr
}
#endif

/*---------------------------------------------------------------------------*
  Name:         _start_AutoloadDoneCallback
  Description:  �I�[�g���[�h�����R�[���o�b�N�B
  Arguments:    argv    -   �I�[�g���[�h�p�����[�^��ێ����Ă���z��B
                    argv[0] =   SDK_AUTOLOAD_LIST
                    argv[1] =   SDK_AUTOLOAD_LIST_END
                    argv[2] =   SDK_AUTOLOAD_START
                    argv[3] =   SDK_STATIC_BSS_START
                    argv[4] =   SDK_STATIC_BSS_END
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start_AutoloadDoneCallback(void* argv[])
{
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CopySysConfig

  Description:  copy SCFG registers to HW_SYS_CONF_BUF and HW_PRV_WRAM_SYSRV

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void INITi_CopySysConfig( void )
{
//################ temp: this process will be done in IPL
        // SCFG enable?
        ldr     r2, =REG_EXT_ADDR
        ldr     r0, [r2]
        tst     r0, #0x80000000
        beq     @9

        ldr     r2, =HW_PRV_WRAM_SYSRV
        //EXT(extentions)
        ldr     r3, =REG_EXT_ADDR
        ldr     r0, [r3]
        str     r0, [r2, #HWi_WSYS04_WRAMOFFSET]
        //OPT(bonding option)
        ldr     r3, =REG_OP_ADDR
        ldrb    r0, [r3]
        //A9ROM(ARM9 ROM)
        ldr     r3, =REG_A9ROM_ADDR
        ldrb    r1, [r3]
        and     r12,r1, #(REG_SCFG_A9ROM_RSEL_MASK | REG_SCFG_A9ROM_SEC_MASK)
        orr     r0, r0, r12, LSL #(HWi_WSYS08_ROM_ARM9RSEL_SHIFT - REG_SCFG_A9ROM_RSEL_SHIFT)
        //A7ROM(ARM7 ROM)
        ldr     r3, =REG_A7ROM_ADDR
        ldrb    r1, [r3]
        and     r12,r1, #(REG_SCFG_A7ROM_RSEL_MASK | REG_SCFG_A7ROM_FUSE_MASK)
        orr     r0, r0, r12, LSL #(HWi_WSYS08_ROM_ARM7RSEL_SHIFT - REG_SCFG_A7ROM_RSEL_SHIFT)
        //DS-WL(DS wireless)
        ldr     r3, =REG_A7ROM_ADDR
        ldrb    r1, [r3]
        and     r12,r1, #REG_SCFG_WL_OFFB_MASK
        orr     r0, r0, r12, LSL #(HWi_WSYS08_WL_OFFB_SHIFT - REG_SCFG_WL_OFFB_SHIFT)
        strb    r0, [r2, #HWi_WSYS08_WRAMOFFSET]
        //OPT(JTAG info)
        ldr     r3, =REG_JTAG_ADDR
        ldrh    r0, [r3]
        and     r12,r0, #REG_SCFG_JTAG_DSPJE_MASK
        orr     r0, r0, r12, LSR #(REG_SCFG_JTAG_DSPJE_SHIFT - HWi_WSYS09_JTAG_DSPJE_SHIFT)
        //CLK(only wram clock)
        ldr     r3, =REG_CLK_ADDR
        ldrh    r1, [r3]
        and     r12,r1, #(REG_SCFG_CLK_AESHCLK_MASK | REG_SCFG_CLK_SD2HCLK_MASK | REG_SCFG_CLK_SD1HCLK_MASK)
        orr     r0, r0, r12, LSL #(HWi_WSYS09_CLK_SD1HCLK_SHIFT - REG_SCFG_CLK_SD1HCLK_SHIFT)
        and     r12,r1, #(REG_SCFG_CLK_SNDMCLK_MASK | REG_SCFG_CLK_WRAMHCLK_MASK)
        orr     r0, r0, r12, LSR #(REG_SCFG_CLK_WRAMHCLK_SHIFT - HWi_WSYS09_CLK_WRAMHCLK_SHIFT)
        strb    r0, [r2, #HWi_WSYS09_WRAMOFFSET]
@9:
//################

        //---- copy scfg setting
        ldr     r2, =HW_PRV_WRAM_SYSRV
        ldr     r3, =HW_SYS_CONF_BUF
        ldr     r0, [r2, #HWi_WSYS04_WRAMOFFSET]
        str     r0, [r3, #HWi_WSYS04_OFFSET]
        ldrh    r0, [r2, #HWi_WSYS08_WRAMOFFSET]
        strh    r0, [r3, #HWi_WSYS08_OFFSET]

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_DetectMainMemorySize
  Description:  ���C���������T�C�Y�𒲍�����B
                �������ʂ� (u16*)HW_MMEMCHECER_SUB �Ɋi�[�����B
                �i�[�����l�� [OS_CONSOLE_SIZE_16MB|OS_CONSOLE_SIZE_32B]
    NOTE:       �v���b�g�t�H�[���� NITRO �̏ꍇ�͍l�����Ă��Ȃ��B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
#define     OSi_IMAGE_DIFFERENCE2   0xb000000
#define     OSi_DETECT_NITRO_MASK   (REG_SND_SMX_CNT_E_MASK | REG_SND_SMX_CNT_FSEL_MASK)
#define     OSi_DETECT_NITRO_VAL    (REG_SND_SMX_CNT_E_MASK)

#define     OSi_CHECKNUM1           0x55
#define     OSi_CHECKNUM2           0xaa
#define     OSi_100usWAIT           3352  // 100us = 3351.4cycles(33.514MHz)

static asm void
INITi_DetectMainMemorySize(void)
{
        ldr         r2, =HW_MMEMCHECKER_SUB
        add         r3, r2, #OSi_IMAGE_DIFFERENCE2
        mov         r0, #OS_CONSOLE_SIZE_16MB

        /* OSi_CHECKNUM1 (0x55) �������݃e�X�g */
        mov         r1, #OSi_CHECKNUM1
        strb        r1, [r3]

        ldr         r2, =OSi_100usWAIT
@1      subs        r2, r2, #4
        bcs         @1

        ldrb        r1, [r3]
        cmp         r1, #OSi_CHECKNUM1
        bne         @check_smix

        /* OSi_CHECKNUM2 (0xaa) �� �������݃e�X�g */
        mov         r1, #OSi_CHECKNUM2
        strb        r1, [r3]

        ldr         r2, =OSi_100usWAIT
@2      subs        r2, r2, #4
        bcs         @2

        ldrb        r1, [r3]
        cmp         r1, #OSi_CHECKNUM2
        moveq       r0, #OS_CONSOLE_SIZE_32MB

@check_smix:
        /* SMIX ���W�X�^�𒲍� */
        ldr         r3, =REG_SMX_CNT_ADDR
        ldrh        r1, [r3]
        and         r1, r1, #OSi_DETECT_NITRO_MASK
        cmp         r1, #OSi_DETECT_NITRO_VAL
        orreq       r0, r0, #OS_CHIPTYPE_SMX_MASK

        /* �������ʂ��i�[ */
        ldr         r2, =HW_MMEMCHECKER_SUB
        strb        r0, [r2]
        bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpStartUp

  Description:  hook for user start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void TwlSpStartUp(void)
{
}

#include    <twl/codereset.h>
