/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - init - ARM7.TWL
  File:     crt0.SCR.c

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

#include    <nitro/types.h>
#include    <nitro/hw/common/armArch.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap_sp.h>
#include    <twl/hw/common/mmap_wramEnv.h>
#include    <nitro/os/common/emulator.h>
#ifndef SDK_FINALROM
#include    <nitro/os/common/printf.h>
#endif
#include    "boot_sync.h"

/*---------------------------------------------------------------------------*/
void    _start(void);
void    _start_AutoloadDoneCallback(void* argv[]);

/*---------------------------------------------------------------------------*
    �O���Q��
 *---------------------------------------------------------------------------*/
/* �����J�X�N���v�g�ɂ���`�����V���{���Q�� */
extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);
extern void SDK_LTDAUTOLOAD_LIST(void);
extern void SDK_LTDAUTOLOAD_LIST_END(void);
extern void SDK_LTDAUTOLOAD_START(void);
extern void SDK_WRAM_ARENA_LO(void);

/* �O���֐��Q�� */
extern void OS_IrqHandler(void);
#ifndef SDK_NOINIT
extern void _fp_init(void);
extern void __call_static_initializers(void);
#endif

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621
#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

/* .rodata �Z�N�V�����ɔz�u���郍�[�h�ɕK�v�ȏ�� */
void* const _start_ModuleParams[]   =
{
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)SDK_STATIC_BSS_START,
    (void*)SDK_STATIC_BSS_END,
    (void*)0,
    (void*)0,
    (void*)SDK_NITROCODE_BE,
    (void*)SDK_NITROCODE_LE,
};

void* const _start_LtdModuleParams[]    =
{
    (void*)SDK_LTDAUTOLOAD_LIST,
    (void*)SDK_LTDAUTOLOAD_LIST_END,
    (void*)SDK_LTDAUTOLOAD_START,
    (void*)0,
    (void*)SDK_TWLCODE_BE,
    (void*)SDK_TWLCODE_LE,
};

/*---------------------------------------------------------------------------*
  Name:         ShakeHand
  Description:  ARM9 �� ShakeHand �֐��Ɠ��������B
                ���C���������łȂ���������ԂŎ��s�����K�v������B
  Arguments:    r0  -   ARM9 �����p�ϐ��ւ̃|�C���^�B
                r1  -   ARM7 �����p�ϐ��ւ̃|�C���^�B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static const u32    microcode_ShakeHand[10] =
{
    0xe1d020b0, /*      ldrh    r2, [r0]    ; �����ϐ��X�����L����������ǂ� */
    0xe1d130b0, /*      ldrh    r3, [r1]    ; �����ϐ��V�����L����������ǂ� */
    0xe2833001, /*  @1: add     r3, r3, #1  ; �����ϐ��V ++ */
    0xe1c130b0, /*      strh    r3, [r1]    ; �����ϐ��V�����L�������ɏ��� */
    0xe1d0c0b0, /*      ldrh    r12, [r0]   ; �����ϐ��X�̌�������L����������ǂ� */
    0xe152000c, /*      cmp     r2, r12     ; �����ϐ��X�̕ω��𔻒肷�� */
    0x0afffffa, /*      beq     @1          ; �ω����Ă��Ȃ���΃��[�v */
    0xe2833001, /*      add     r3, r3, #1  ; �����ϐ��V ++ */
    0xe1c130b0, /*      strh    r3, [r1]    ; �����ϐ��V�����L�������ɏ��� */
    0xe12fff1e  /*      bx      lr          ; �n���h�V�F�C�N���� */
};

/*---------------------------------------------------------------------------*
  Name:         Stop
  Description:  �v���O�������~����B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static const u32    microcode_Stop[2] =
{
    0xef000006, /*  @1: swi     #6          ; SVC_Halt */
    0xeafffffd  /*      b       @1          ; ���[�v */
};

/*---------------------------------------------------------------------------*
  Name:         GotoMain
  Description:  ARM9 �ɓ���̏�ԂɂȂ������Ƃ�`������ŁAMain �֐��փW�����v
                ����B�����Ɏw��o�b�t�@�� 0 �N���A���s���B�X�^�b�N���œ��삳��
                �邱�Ƃ�z�肵�Ă���ׁA�X�^�b�N����؎g�p���Ȃ��B
  Arguments:    r0  -   0 �N���A����o�b�t�@�̐擪�A�h���X�B
                        4 �o�C�g�A���C�����ꂽ�A�h���X�ł���K�v������B
                r1  -   ���  8 bit: ARM9 �ɓ`����t�F�[�Y�ԍ��B
                        ���� 24 bit: 0 �N���A����o�b�t�@�̃T�C�Y�B
                                     4 �̔{���ł���K�v������B
                r2  -   �����p�t�F�[�Y�Ǘ��ϐ��ւ̃|�C���^�B
                r3  -   Main �֐��̃|�C���^�BMain �֐��� Thumb �R�[�h�ł���ꍇ
                        �ɂ́Abx ���߂ŃW�����v���邽�ߍŉ��ʃr�b�g�� 1 �ɂȂ���
                        ����K�v������B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static const u32    microcode_GotoMain[10]   =
{
    0xe59fc01c, /*  @1: ldr     r12, [pc, #28]  ; ���� 24 bit �}�X�N��p�� */
    0xe111000c, /*      tst     r1, r12         ; �N���A�o�b�t�@�c�T�C�Y���`�F�b�N */
    0xe3a0c000, /*      mov     r12, #0         ; �N���A�p�ϐ���p�� */
    0x1480c004, /*      strne   r12, [r0], #4   ; 4 �o�C�g�N���A */
    0x12411004, /*      subne   r1, r1, #4      ; �N���A�o�b�t�@�c�T�C�Y -= 4 */
    0x1afffff9, /*      bne     @1              ; �c�T�C�Y�� 0 �ɂȂ�܂Ń��[�v */
    0xe1a01c21, /*      mov     r1, r1, LSR #24 ; �t�F�[�Y�Ǘ��ϐ��̍X�V�l��p�� */
    0xe1c210b0, /*      strh    r1, [r2]        ; �t�F�[�Y�Ǘ��ϐ����X�V */
    0xe12fff13, /*      bx      r3              ; Main �֐��փW�����v */
    0x00fffffc  /*      <DATA>  0x00fffffc */
};


/*---------------------------------------------------------------------------*
    �����֐��v���g�^�C�v
 *---------------------------------------------------------------------------*/
static void     INITi_CheckSysConfig(void);
static void     INITi_DetectMainMemorySize(void);
static void     INITi_Stop(void);
static void     INITi_DoAutoload(void);
#ifndef SDK_NOINIT
static void     INITi_ShelterStaticInitializer(u32* ptr);
static void     INITi_CallStaticInitializers(void);
#endif

static void*    INITi_Copy32(void* dst, void* src, u32 size);
static void*    INITi_Fill32(void* dst, u32 value, u32 size);

/*---------------------------------------------------------------------------*/
#include    <twl/code32.h>

/*---------------------------------------------------------------------------*
  Name:         _start
  Description:  �N���x�N�^�B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start(void)
{
@000:
        /* �����݋֎~ */
        mov         r12, #HW_REG_BASE
        str         r12, [r12, #REG_IME_OFFSET]

        /* SCFG �ݒ���m�F */
        bl          INITi_CheckSysConfig

        /* �����`���[����n���ꂽ����ޔ� */
        ldr         r3, =SDK_WRAM_ARENA_LO
        sub         r2, r3, #0x40
        ldr         r1, =HW_LAUNCHER_DELIVER_PARAM_BUF
@001:
        cmp         r2, r3
        ldrlt       r0, [r1], #4
        strlt       r0, [r2], #4
        blt         @001

        /* �n���h�V�F�C�N�p�}�C�N���R�[�h���p WRAM �ɃR�s�[ */
        ldr         r1, =microcode_ShakeHand
        ldr         r2, =HW_PRV_WRAM
        add         r3, r2, #40     // sizeof(microcode_ShakeHand)
@002:   ldr         r0, [r1], #4
        str         r0, [r2], #4
        cmp         r2, r3
        blt         @002

        /* ��p WRAM ��̃R�[�h�� ARM9 �ƃn���h�V�F�C�N */
        ldr         r0, =HW_BOOT_SHAKEHAND_9
        ldr         r1, =HW_BOOT_SHAKEHAND_7
        ldr         r2, =HW_PRV_WRAM
        mov         lr, pc
        bx          r2

@010:
        /* �X�^�b�N�|�C���^�ݒ� */
        mov         r0, #HW_PSR_SVC_MODE        // SuperVisor mode
        msr         cpsr_c, r0
        ldr         sp, =HW_PRV_WRAM_SVC_STACK_END
        mov         r0, #HW_PSR_IRQ_MODE        // IRQ mode
        msr         cpsr_c, r0
        ldr         sp, =HW_PRV_WRAM_IRQ_STACK_END
        ldr         r1, =SDK_IRQ_STACKSIZE
        sub         r1, sp, r1
        sub         sp, sp, #4                  // 4 bytes for IRQ stack check code
        mov         r0, #HW_PSR_SYS_MODE        // System mode
        msr         cpsr_csfx, r0
        sub         sp, r1, #4                  // 4 bytes for SYS stack check code

        /* �X�^�b�N�̈���N���A */
        ldr         r0, =SDK_SYS_STACKSIZE
        sub         r1, r1, r0
        ldr         r2, =HW_PRV_WRAM_IRQ_STACK_END
        mov         r0, #0
@011:   cmp         r1, r2
        strlt       r0, [r1], #4
        blt         @011

        /* Autoload �����{ */
        bl          INITi_DoAutoload

        /* STATIC �Z�O�����g�� .bss �Z�N�V������ 0 �N���A */
        mov         r1, #0          // r1 = clear value for STATIC bss section
        ldr         r3, =_start_ModuleParams
        ldr         r0, [r3, #12]   // r0 = start address of STATIC bss section
        ldr         r2, [r3, #16]
        subs        r2, r2, r0      // r2 = size of STATIC bss section
        blgt        INITi_Fill32

        /* ���C���������T�C�Y�𒲍� */
        bl          INITi_DetectMainMemorySize

#ifndef SDK_FINALROM
        /* �f�o�b�O�o�̓E�B���h�E��ݒ� */
        ldr         r1, =HW_PRINT_OUTPUT_ARM9
        mov         r0, #OS_PRINT_OUTPUT_DEFAULT_ARM9
        orr         r0, r0, #(OS_PRINT_OUTPUT_DEFAULT_ARM7 << 8)
        strh        r0, [r1]
        mov         r0, #OS_PRINT_OUTPUT_DEFAULT_ARM9ERR
        orr         r0, r0, #(OS_PRINT_OUTPUT_DEFAULT_ARM7ERR << 8)
        strh        r0, [r1, #2]
#endif

        /* IRQ �����݃x�N�^�ݒ� */
        ldr         r1, =HW_INTR_VECTOR_BUF
        ldr         r0, =OS_IrqHandler
        str         r0, [r1]

#ifndef SDK_NOINIT
        /* c++ �p������ */
        bl          _fp_init
        bl          TwlSpStartUp
        bl          __call_static_initializers
        bl          INITi_CallStaticInitializers
#endif

@0f0:
        /* Main �֐��ւ̃W�����v�p�}�C�N���R�[�h���X�^�b�N�̒�ɃR�s�[ */
        ldr         r1, =microcode_GotoMain
        sub         r2, sp, #40
        mov         r3, sp
@0f1:   cmp         r2, r3
        ldrlt       r0, [r1], #4
        strlt       r0, [r2], #4
        blt         @0f1

        /* �}�C�N���R�[�h���o�R���� Main �֐��փW�����v */
        ldr         r0, =SDK_STATIC_START
        bic         r0, r0, #0x00000003
        ldr         r1, =SDK_STATIC_BSS_END
        sub         r1, r1, r0
        add         r1, r1, #3
        bic         r1, r1, #0x00000003
        bic         r1, r1, #0xff000000
        mov         r2, #BOOT_SYNC_PHASE_4
        mov         r2, r2, LSL #24
        orr         r1, r1, r2
        ldr         r2, =HW_BOOT_SYNC_PHASE
        ldr         r3, =TwlSpMain
        ldr         lr, =HW_RESET_VECTOR
        sub         r12, sp, #40
        bx          r12
        /* never return */
}

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
        bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CheckSysConfig
  Description:  SCFG �֘A I/O ���W�X�^�̓��e���m�F����B
                A7-SCFG �u���b�N���A�N�Z�X�L���ȏꍇ�ɂ́A���[�_�[���s���ݒ菈
                �����s������ŃA�N�Z�X�𖳌�������B
                ARM7 ��p WRAM ���Ƀ��[�_�[���W�J���Ă��� I/O ���W�X�^�����m
                �F������Ń��C����������̋��L�̈�ɃR�s�[����B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_CheckSysConfig(void)
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
        ldr         r1, =REG_EXT_ADDR
        ldr         r0, [r1]
/*      bic         r0, r0, #REG_SCFG_EXT_CFG_MASK */
        str         r0, [r1]

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
        ldr     	r2, =HW_MMEMCHECKER_SUB
        add     	r3, r2, #OSi_IMAGE_DIFFERENCE2
        mov     	r0, #OS_CONSOLE_SIZE_16MB

		/* OSi_CHECKNUM1 (0x55) �������݃e�X�g */
		mov			r1, #OSi_CHECKNUM1
		strb		r1, [r3]

		ldr			r2, =OSi_100usWAIT
@1		subs		r2, r2, #4
		bcs			@1

		ldrb		r1, [r3]
		cmp			r1, #OSi_CHECKNUM1
		bne			@check_smix

		/* OSi_CHECKNUM2 (0xaa) �� �������݃e�X�g */
        mov     	r1, #OSi_CHECKNUM2
        strb    	r1, [r3]

		ldr     	r2, =OSi_100usWAIT
@2		subs    	r2, r2, #4
        bcs     	@2

        ldrb    	r1, [r3]
        cmp     	r1, #OSi_CHECKNUM2
        moveq   	r0, #OS_CONSOLE_SIZE_32MB

@check_smix:
        /* SMIX ���W�X�^�𒲍� */
        ldr         r3, =REG_SMX_CNT_ADDR
        ldrh        r1, [r3]
        and         r1, r1, #OSi_DETECT_NITRO_MASK
        cmp         r1, #OSi_DETECT_NITRO_VAL
        orreq       r0, r0, #OS_CHIPTYPE_SMX_MASK

        /* �������ʂ��i�[ */
        ldr 	    r2, =HW_MMEMCHECKER_SUB
        strb        r0, [r2]
        bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_Stop
  Description:  �v���O�������~����BARM7 ��p WRAM �� Halt ���J��Ԃ����[�v
                �֐����R�s�[���A�R�s�[�����֐������s����B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_Stop(void)
{
        ldr         r1, =microcode_Stop
        ldr         r2, =HW_PRV_WRAM
        add         r3, r2, #8      // sizeof(microcode_Stop)
@copy_loop:
        ldr         r0, [r1], #4
        str         r0, [r2], #4
        cmp         r2, r3
        blt         @copy_loop

@stop_loop:
        ldr         r0, =HW_PRV_WRAM
        mov         lr, pc
        bx          r0
        b           @stop_loop
        /* never return */
}

/*---------------------------------------------------------------------------*
  Name:         INITi_DoAutoload
  Description:  �����N���ɉ����āA�e�I�[�g���[�h�u���b�N�̌Œ�f�[�^���̓W�J
                �y�ѕϐ����� 0 �N���A���s���B
                NITRO �݊��I�[�g���[�h�Z�O�����g�͑��݂����ATWL ��p�Z�O�����g
                �݂̂����݂���Ƃ����O��B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/

static asm void
INITi_DoAutoload(void)
{
        stmfd       sp!, {lr}

        /* �I�[�g���[�h�����{ */
@000:
        ldr         r0, =_start_LtdModuleParams
        ldr         r12, [r0]       // r12 = SDK_LTDAUTOLOAD_LIST
        ldr         r3, [r0, #4]    // r3 = SDK_LTDAUTOLOAD_LIST_END
        ldr         r1, [r0, #8]    // r1 = SDK_LTDAUTOLOAD_START
@001:
        cmp         r12, r3
        bge         @010
        /* �Œ�Z�N�V���� */
        ldr         r0, [r12], #4   // r0 = start address of destination range of fixed section
        ldr         r2, [r12], #4   // r2 = size of fixed section
        bl          INITi_Copy32
        stmfd       sp!, {r0, r1}
        /* static initializer �e�[�u�� */
        ldr         r0, [r12], #4
#ifndef SDK_NOINIT
        stmfd       sp!, {r3, r12}
        bl          INITi_ShelterStaticInitializer
        ldmfd       sp!, {r3, r12}
#endif
        /* bss �Z�N�V���� */
        ldmfd       sp!, {r0}       // r0 = start address of destination range of bss section
        mov         r1, #0          // r1 = clear value for bss section
        ldr         r2, [r12], #4   // r2 = sizeo of bss section
        bl          INITi_Fill32
        ldmfd       sp!, {r1}
        b           @001

        /* �I�[�g���[�h���X�g�E�I�[�g���[�h���o�b�t�@���N���A */
@010:
        ldr         r1, =_start_LtdModuleParams
        ldr         r12, [r1]       // r12 = SDK_LTDAUTOLOAD_LIST
        ldr         r3, [r1, #4]    // r3 = SDK_LTDAUTOLOAD_LIST_END
        ldr         r0, [r1, #8]    // r0 = SDK_LTDAUTOLOAD_START
@011:
        cmp         r12, r3
        bge         @0f0
        mov         r1, #0          // r1 = clear value
        str         r1, [r12], #4
        ldr         r2, [r12]       // r2 = size of fixed section
        str         r1, [r12], #4
        str         r1, [r12], #4
        str         r1, [r12], #4
        bl          INITi_Fill32
        b           @011

@0f0:
        /* �I�[�g���[�h�����R�[���o�b�N�Ăяo�� */
        ldr         r0, =_start_ModuleParams
        ldr         r1, =_start_LtdModuleParams
        ldmfd       sp!, {lr}
        b           _start_AutoloadDoneCallback
}

#ifndef SDK_NOINIT

static asm void
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
        cmp         r0, #0
        bxeq        lr

        /* �ޔ��ꏊ�擪�A�h���X���v�Z */
        ldr         r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr         r2, =SDK_IRQ_STACKSIZE
        sub         r1, r1, r2
        add         r1, r1, #4
@001:
        /* �ޔ��ꏊ�擪����󂫏ꏊ�𒲍� */
        ldr         r2, [r1]
        cmp         r2, #0
        addne       r1, r1, #4
        bne         @001
@002:
        /* �󂫏ꏊ�Ƀe�[�u�����R�s�[ */
        ldr         r2, [r0], #4
        str         r2, [r1], #4
        cmp         r2, #0
        bne         @002

        bx          lr
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
        stmdb       sp!, {lr}

        /* �e�[�u���ޔ��ꏊ�擪�A�h���X���v�Z */
        ldr         r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr         r2, =SDK_IRQ_STACKSIZE
        sub         r1, r1, r2
        add         r1, r1, #4
@001:
        /* �e�[�u���ɊǗ�����Ă���|�C���^������Ăяo�� */
        ldr         r0, [r1]
        cmp         r0, #0
        beq         @002
        stmfd       sp!, {r1}
        mov         lr, pc
        bx          r0
        ldmfd       sp!, {r1}
        /* ��U�Ăяo�����|�C���^�̓[���N���A (IRQ�X�^�b�N���Ԏ؂肵�Ă����) */
        mov         r0, #0
        str         r0, [r1], #4
        b           @001
@002:
        ldmia       sp!, {lr}
        bx          lr
}
#endif

/*---------------------------------------------------------------------------*
  Name:         INITi_Copy32
  Description:  32 bit �P�ʂŃR�s�[���s���B�X�^�b�N�� 36 �o�C�g����邪�A
                ���W�X�^ r3 - r12 �͔j�󂵂Ȃ��B
  Arguments:    r0  -   �R�s�[��ւ̃|�C���^ ( 4 �o�C�g�A���C�� )�B
                r1  -   �R�s�[���ւ̃|�C���^ ( 4 �o�C�g�A���C�� )�B
                r2  -   �R�s�[���钷�����o�C�g�P�ʂŎw�� ( 4 �̔{�� )�B
  Returns:      r0  -   �R�s�[��̃R�s�[��ւ̃|�C���^ ( r0 + r2 )�B
                r1  -   �R�s�[��̃R�s�[���ւ̃|�C���^ ( r1 + r2 )�B
 *---------------------------------------------------------------------------*/
static asm void*
INITi_Copy32(void* dst, void* src, u32 size)
{
        stmfd       sp!, {r3-r11}

        bics        r3, r2, #0x0000001f
        beq         @next
        add         r3, r0, r3
@loop:
        ldmia       r1!, {r4-r11}
        stmia       r0!, {r4-r11}
        cmp         r3, r0
        bgt         @loop
@next:
        tst         r2, #0x00000010
        ldmneia     r1!, {r4-r7}
        stmneia     r0!, {r4-r7}
        tst         r2, #0x00000008
        ldmneia     r1!, {r4-r5}
        stmneia     r0!, {r4-r5}
        tst         r2, #0x00000004
        ldmneia     r1!, {r4}
        stmneia     r0!, {r4}

        ldmfd       sp!, {r3-r11}
        bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_Fill32
  Description:  32 bit �P�ʂŃo�b�t�@���e���w��f�[�^�Ŗ��߂�B�X�^�b�N�� 36
                �o�C�g����邪�A���W�X�^ r3 - r12 �͔j�󂵂Ȃ��B
  Arguments:    r0  -   �o�b�t�@�ւ̃|�C���^ ( 4 �o�C�g�A���C�� )�B
                r1  -   �o�b�t�@�𖄂߂���e�� 32 bit �l�Ŏw��B
                r2  -   �o�b�t�@�𖄂߂钷�����o�C�g�P�ʂŎw�� ( 4 �̔{�� )�B
  Returns:      r0  -   ������̃o�b�t�@�ւ̃|�C���^ ( r0 + r2 )�B
 *---------------------------------------------------------------------------*/
static asm void*
INITi_Fill32(void* dst, u32 value, u32 size)
{
        stmfd       sp!, {r3-r11}

        mov         r4, r1
        mov         r5, r1
        mov         r6, r1
        mov         r7, r1
        mov         r8, r1
        mov         r9, r1
        mov         r10, r1
        mov         r11, r1
        bics        r3, r2, #0x0000001f
        beq         @next
        add         r3, r0, r3
@loop:
        stmia       r0!, {r4-r11}
        cmp         r3, r0
        bgt         @loop
@next:
        tst         r2, #0x00000010
        stmneia     r0!, {r4-r7}
        tst         r2, #0x00000008
        stmneia     r0!, {r4-r5}
        tst         r2, #0x00000004
        stmneia     r0!, {r4}

        ldmfd       sp!, {r3-r11}
        bx          lr
}

#include    <twl/codereset.h>
