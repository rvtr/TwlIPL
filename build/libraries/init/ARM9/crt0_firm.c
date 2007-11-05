/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - init
  File:     crt0.c

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
#include        <nitro/code32.h>
#include        <firm.h>

void    _start(void);
void    _start_AutoloadDoneCallback(void *argv[]);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);
extern void TwlMain(void);

static void INITi_CpuClear32(register u32 data, register void *destp, register u32 size);
static void INITi_InitCoprocessor(void);
static void INITi_InitRegion(void);
static void INITi_DoAutoload(void);
static void INITi_ShelterLtdBinary(void);

// from LCF
extern unsigned long SDK_IRQ_STACKSIZE[];

extern void SDK_AUTOLOAD_LIST(void);        // start pointer to autoload information
extern void SDK_AUTOLOAD_LIST_END(void);    // end pointer to autoload information
extern void SDK_AUTOLOAD_START(void);       // autoload data will start from here
extern void SDK_STATIC_BSS_START(void);     // static bss start address
extern void SDK_STATIC_BSS_END(void);       // static bss end address
extern void SDK_LTDAUTOLOAD_LIST(void);     // start pointer to autoload information
extern void SDK_LTDAUTOLOAD_LIST_END(void); // end pointer to autoload information
extern void SDK_LTDAUTOLOAD_START(void);    // autoload data will start from here

/*---------------------------------------------------------------------------*
  Name:         _start_ModuleParams

  Description:  autoload/compress/arguments data block

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
//#include        <nitro/version.h>
#define         SDK_VERSION_ID   ((u32)SDK_VERSION_MAJOR<<24|\
                                  (u32)SDK_VERSION_MINOR<<16|\
                                  (u32)SDK_VERSION_RELSTEP)

void   *const _start_ModuleParams[]     =
{
    (void *)SDK_AUTOLOAD_LIST,
    (void *)SDK_AUTOLOAD_LIST_END,
    (void *)SDK_AUTOLOAD_START,
    (void *)SDK_STATIC_BSS_START,
    (void *)SDK_STATIC_BSS_END,
    (void *)0,                         // CompressedStaticEnd
    (void *)SDK_VERSION_ID,            // SDK version info
    (void *)SDK_NITROCODE_BE,          // Checker 1
    (void *)SDK_NITROCODE_LE,          // Checker 2
};
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
#define INITi_HW_DTCM   SDK_AUTOLOAD_DTCM_START

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
        ldr             r0, =INITi_HW_DTCM
        add             r0, r0, #0x3fc0
        mov             sp, r0

        // IRQ mode
        mov             r0, #HW_PSR_IRQ_MODE
        msr             cpsr_c, r0
        ldr             r0, =INITi_HW_DTCM
        add             r0, r0, #0x3fc0
        sub             r0, r0, #HW_SVC_STACK_SIZE
        sub             sp, r0, #4 // 4byte for stack check code
        tst             sp, #4
        subeq           sp, sp, #4 // for 8byte-alignment

        // System mode
        ldr             r1, =SDK_IRQ_STACKSIZE
        sub             r1, r0, r1
        mov             r0, #HW_PSR_SYS_MODE
        msr             cpsr_csfx, r0
        sub             sp, r1, #4 // 4byte for stack check code

        //---- read reset flag from pmic
#ifdef TWL_PLATFORM_TS
@0:     bl              PXI_RecvByIntf
        cmp             r0, #FIRM_PXI_ID_COLDBOOT
        cmpne           r0, #FIRM_PXI_ID_WARMBOOT
        bne             @0

        //---- initialize Main Memory
        cmp             r0, #FIRM_PXI_ID_COLDBOOT
        moveq           r0, #TRUE
        movne           r0, #FALSE
        bl              MIi_InitMainMemCR

        mov             r0, #FIRM_PXI_ID_INIT_MMEM
        bl              PXI_SendByIntf

#else // TWL_PLATFORM_BB
        //---- initialize Main Memory
        bl              MIi_InitMainMemCR

#endif // TWL_PLATFORM_BB

        /* �V�X�e������R�v���Z�b�T������ */
        bl              INITi_InitCoprocessor

        /* ���[�W���������ݒ� */
        bl              INITi_InitRegion

        //---- clear memory
        // DTCM (16KB)
        mov             r0, #0
        ldr             r1, =INITi_HW_DTCM
        mov             r2, #HW_DTCM_SIZE
        bl              INITi_CpuClear32

        // BG/OBJ palette (1KB)
        mov             r0, #0
        ldr             r1, =HW_PLTT
        mov             r2, #HW_PLTT_SIZE
        bl              INITi_CpuClear32

        // OAM (1KB)
        mov             r0, #0x0200
        ldr             r1, =HW_OAM
        mov             r2, #HW_OAM_SIZE
        bl              INITi_CpuClear32

        //---- load autoload block and initialize bss
//        bl              INITi_DoAutoload
#ifndef SDK_FINALROM    // for IS-TWL-DEBUGGER
        bl          _start_AutoloadDoneCallback
#endif

        //---- fill static static bss with 0
        mov             r0, #0
        ldr             r3, =_start_ModuleParams
        ldr             r1, [r3, #12]       // SDK_STATIC_BSS_START
        ldr             r2, [r3, #16]       // SDK_STATIC_BSS_END
        sub             r2, r2, r1
        bl              INITi_CpuClear32

        //---- flush static bss region
        //     (r0 == #0, r3 == _start_ModuleParams::BSS_segment_start)
        bic                r1, r3, #HW_CACHE_LINE_SIZE - 1
@cacheflush:
        mcr                p15, 0, r0, c7, c10, 4         // wait writebuffer empty
        mcr                p15, 0, r1, c7,  c5, 1         // ICache
        mcr                p15, 0, r1, c7, c14, 1         // DCache
        add                r1, r1, #HW_CACHE_LINE_SIZE
        cmp                r1, r2
        blt                @cacheflush

        // print buffer (used for ARM7's printing)
        ldr             r1, =HW_COMPONENT_PARAM
        str             r0, [r1, #0]

        //---- set interrupt vector
        ldr             r1, =INITi_HW_DTCM
        add             r1, r1, #0x3fc0
        add             r1, r1, #HW_DTCM_SYSRV_OFS_INTR_VECTOR
        ldr             r0, =OS_IrqHandler
        str             r0, [r1, #0]

#ifndef SDK_NOINIT
        //---- for C++
        bl             _fp_init
        bl             TwlStartUp
        bl             __call_static_initializers
#endif
        //---- start (to 16bit code)
        ldr             r1, =TwlMain
        ldr             lr, =HW_RESET_VECTOR

        tst             sp, #4
        subne           sp, sp, #4 // for 8byte-alignment
        bx              r1
}


/*---------------------------------------------------------------------------*
  Name:         INITi_CpuClear32

  Description:  fill memory with specified data.
                32bit version

  Arguments:    data  : fill data
                destp : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static asm void  INITi_CpuClear32( register u32 data, register void *destp, register u32 size )
{
        add     r12, r1, r2             // r12: destEndp = destp + size
@20:
        cmp     r1, r12                 // while (destp < destEndp)
        stmltia r1!, {r0}               // *((vu32 *)(destp++)) = data
        blt     @20
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_DoAutoload
  Description:  �����N���ɉ����āA�e�I�[�g���[�h�u���b�N�̌Œ�f�[�^���̓W�J
                �y�ѕϐ����� 0 �N���A���s���B4M bytes ���z���� PSRAM ���������
                �ɔz�u�����I�[�g���[�h�u���b�N�̓W�J�́A�n�[�h�E�F�A�� TWL ��
                ����ꍇ�ɂ����s���B�I�[�g���[�h���f�[�^�ƃI�[�g���[�h�悪�ꕔ
                �d�Ȃ�ꍇ������̂ŁA�������W�J���s���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
/*
 * < ��i�K�I�[�g���[�h >
 * 0x02000000 �� Static �Z�O�����g�y�ш�i�ڃ��[�h���o�C�i����(�K�v�ɉ����Č㔼�����k�����)�z�u����Ă���B
 *  ���k����Ă���ꍇ�́A�܂� 0x02000000 �Ɍ������㏑�����𓀂���B
 *  NITRO �Ƌ��L�\�� ITCM ��ɔz�u�����ׂ��o�C�i���f�[�^�� 0x01ff8000 �Ƀ��[�h����B
 *  NITRO �Ƌ��L�\�� DTCM ��ɔz�u�����ׂ��o�C�i���f�[�^�� 0x02fe0000 �Ƀ��[�h����B
 * 0x02400000 �ɓ�i�ڃ��[�h���o�C�i����(�K�v�ɉ����đS�Ĉ��k�����)�z�u����Ă���B
 *  0x04000 �o�C�g���̓J�[�h ROM ����ēǂݏo���s�Ȃ̂ŁA0x02f80000 - 0x02f84000 �ɑޔ�����B
 *  ���k����Ă���ꍇ�́A�܂� 0x02400000 �Ɍ������㏑�����𓀂���B
 *  TWL �ł������삵�Ȃ� WRAM ��ɔz�u�����ׂ��o�C�i���f�[�^�����ꂼ��w��A�h���X�Ƀ��[�h����B
 *  TWL �ł������삵�Ȃ����C����������ɔz�u�����ׂ��o�C�i���f�[�^��O������R�s�[���邱�ƂŃ��[�h����B
 *  ����́ANITRO �Ƌ��L�\�ȃ��C����������ɔz�u�����f�[�^�� 0x02400000 ���z���Ȃ��͂��ł��邽�߁A
 *  �z�u���ׂ��A�h���X�� 0x02400000 ��菬�����A�h���X�ɂȂ�͂��ł���ׁB
 *  �܂��A�I�[�g���[�h��񃊃X�g�̎��̂����C���������ւ̃I�[�g���[�h�u���b�N�� .bss �Z�N�V�����̃N���A�̉ߒ���
 *  �j�󂳂��\�������邪�A��A�̃I�[�g���[�h�����̍Ō�̒i�K�Ȃ̂ŁA�j�󂳂�Ă����Ȃ��B
 */
static asm void
INITi_DoAutoload(void)
{
@000:
        stmdb           sp!, {lr}
        /* NITRO ���p�u���b�N�̉� */
        ldr             r1, =_start_ModuleParams
        ldr             r0, [r1, #20]       // r0 = bottom of compressed data
        bl              MIi_UncompressBackward

@010:
        /* NITRO ���p�u���b�N���I�[�g���[�h */
        ldr             r1, =_start_ModuleParams
        ldr             r12, [r1]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_AUTOLOAD_START
@011:   cmp             r12, r0
        bge             @020
        /* �Œ�Z�N�V���������[�h */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        stmdb           sp!, {r2}
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@012:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @012
        /* .bss �Z�N�V������ 0 �N���A */
        mov             r0, #0              // r0 = number to fill .bss section
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@013:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @013
        /* �L���b�V���𒲐� */
        ldmia           sp!, {r2}           // r2 = start address of destination range
        mov             r0, #HW_ITCM_IMAGE
        cmp             r2, r0
        addge           r0, r0, #HW_ITCM_SIZE
        cmpge           r0, r2
        bgt             @015                // If I-TCM autoload block, skip cache control logic.
        ldr             r0, =SDK_AUTOLOAD_DTCM_START
        cmp             r2, r0
        addge           r0, r0, #HW_DTCM_SIZE
        cmpge           r0, r2
        bgt             @015                // If D-TCM autoload block, skip cache control logic.
        bic             r2, r2, #HW_CACHE_LINE_SIZE - 1     // RoundDown32
@014:   cmp             r2, r3
        bge             @015
        mcr             p15, 0, r2, c7, c14, 1      // Store and Invalidate D-Cache
        mcr             p15, 0, r2, c7, c5, 1       // Invalidate I-Cache
        add             r2, r2, #HW_CACHE_LINE_SIZE
        b               @014
@015:   ldmia           sp!, {r0}
        b               @011

@020:
        /* TWL �n�[�h�E�F�A��œ��삵�Ă��邩�ǂ����𒲍� */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        beq             @030

        /* TWL ��p�u���b�N�̑��݂��m�F */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1cc  /* ARM9 �p�g���풓���W���[�� ROM �T�C�Y */
        ldr             r0, [r1]
        cmp             r0, #0
        beq             @030

        /* �ēǂݏo���s������ޔ� */
        bl              INITi_ShelterLtdBinary

        /* TWL ��p�u���b�N�̉� */
        ldr             r1, =_start_LtdModuleParams
        ldr             r0, [r1, #12]
        bl              MIi_UncompressBackward

        /* TWL ��p�u���b�N���I�[�g���[�h */
        ldr             r1, =_start_LtdModuleParams
        ldr             r12, [r1]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_LTDAUTOLOAD_START
@021:   cmp             r12, r0
        bge             @030
        /* �Œ�Z�N�V���������[�h */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        stmdb           sp!, {r2}
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@022:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @022
        /* .bss �Z�N�V������ 0 �N���A */
        mov             r0, #0              // r0 = number to fill .bss section
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@023:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @023
        /* �L���b�V���𒲐� */
        ldmia           sp!, {r2}           // r2 = start address of destination range
        mov             r0, #HW_ITCM_IMAGE
        cmp             r2, r0
        addge           r0, r0, #HW_ITCM_SIZE
        cmpge           r0, r2
        bgt             @025                // If I-TCM autoload block, skip cache control logic.
        ldr             r0, =SDK_AUTOLOAD_DTCM_START
        cmp             r2, r0
        addge           r0, r0, #HW_DTCM_SIZE
        cmpge           r0, r2
        bgt             @025                // If D-TCM autoload block, skip cache control logic.
        bic             r2, r2, #HW_CACHE_LINE_SIZE - 1     // RoundDown32
@024:   cmp             r2, r3
        bge             @025
        mcr             p15, 0, r2, c7, c14, 1      // Store and Invalidate D-Cache
        mcr             p15, 0, r2, c7, c5, 1       // Invalidate I-Cache
        add             r2, r2, #HW_CACHE_LINE_SIZE
        b               @024
@025:   ldmia           sp!, {r0}
        b               @021

@030:   /* ���C�g�o�b�t�@����ɂȂ�̂�҂� */
        mov             r0, #0
        mcr             p15, 0, r0, c7, c10, 4

        /* �I�[�g���[�h�����R�[���o�b�N�֐��Ăяo�� */
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
        /* ARM7 ��p���C����������ԕی샊�[�W��������U�A�N�Z�X�\�ɕύX */
        mrc             p15, 0, r0, c5, c0, 3
        mrc             p15, 0, r1, c5, c0, 2
        stmdb           sp!, {r0, r1}
        bic             r0, r0, #(0xf << 8)
        orr             r0, r0, #(0x1 << 8)
        bic             r1, r1, #(0xf << 8)
        orr             r1, r1, #(0x1 << 8)
        mcr             p15, 0, r0, c5, c0, 3
        mcr             p15, 0, r1, c5, c0, 2

        /* �ޔ����E��A�h���X�𒲍� */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1c8  /* ARM9 �p�g���풓���W���[�� RAM �A�h���X */
        ldr             r1, [r1]
        ldr             r3, =HW_TWL_ROM_HEADER_BUF + 0x038  /* ARM7 �p�풓���W���[�� RAM �A�h���X */
        ldr             r3, [r3]
        sub             r3, r3, #0x4000                     /* �ēǂݏo���s�̈�T�C�Y */ /* ARM7 �p�ޔ��G���A */
        sub             r2, r3, #0x4000                     /* �ēǂݏo���s�̈�T�C�Y */ /* ARM9 �p�ޔ��G���A */

        /* �R�s�[ */
@loop:  ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @loop

        /* ARM7 ��p���C����������ԕی샊�[�W�����ݒ�����ɖ߂� */
        ldmia           sp!, {r0, r1}
        mcr             p15, 0, r0, c5, c0, 3
        mcr             p15, 0, r1, c5, c0, 2
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_UncompressBackward
  Description:  Uncompress special archive for module compression.
  Arguments:    bottom         = Bottom adrs of packed archive + 1
                bottom[-8..-6] = offset for top    of compressed data
                                 inp_top = bottom - bottom[-8..-6]
                bottom[-5]     = offset for bottom of compressed data
                                 inp     = bottom - bottom[-5]
                bottom[-4..-1] = offset for bottom of original data
                                 outp    = bottom + bottom[-4..-1]
                typedef struct
                {
                   u32         bufferTop:24;
                   u32         compressBottom:8;
                   u32         originalBottom;
                }  CompFooter;
  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void
MIi_UncompressBackward(register void* bottom)
{
#define data            r0
#define inp_top         r1
#define outp            r2
#define inp             r3
#define outp_save       r4
#define flag            r5
#define count8          r6
#define index           r7
#define len             r12

        cmp             bottom, #0
        beq             @exit
        stmfd           sp!,    {r4-r7}
        ldmdb           bottom, {r1-r2}
        add             outp,    bottom,  outp
        sub             inp,     bottom,  inp_top, LSR #24
        bic             inp_top, inp_top, #0xff000000
        sub             inp_top, bottom,  inp_top
        mov             outp_save, outp
@loop:
        cmp             inp, inp_top            // exit if inp==inp_top
        ble             @end_loop
        ldrb            flag, [inp, #-1]!       // r4 = compress_flag = *--inp
        mov             count8, #8
@loop8:
        subs            count8, count8, #1
        blt             @loop
        tst             flag, #0x80
        bne             @blockcopy
@bytecopy:
        ldrb            data, [inp, #-1]!
        strb            data, [outp, #-1]!      // Copy 1 byte
        b               @joinhere
@blockcopy:
        ldrb            len,   [inp, #-1]!
        ldrb            index, [inp, #-1]!
        orr             index, index, len, LSL #8
        bic             index, index, #0xf000
        add             index, index, #0x0002
        add             len,   len,   #0x0020
@patterncopy:
        ldrb            data,  [outp, index]
        strb            data,  [outp, #-1]!
        subs            len,   len,   #0x0010
        bge             @patterncopy

@joinhere:
        cmp             inp, inp_top
        mov             flag, flag, LSL #1
        bgt             @loop8
@end_loop:

        // DC_FlushRange & IC_InvalidateRange
        mov             r0, #0
        bic             inp,  inp_top, #HW_CACHE_LINE_SIZE - 1
@cacheflush:
        mcr             p15, 0, r0, c7, c10, 4          // wait writebuffer empty
        mcr             p15, 0, inp, c7,  c5, 1         // ICache
        mcr             p15, 0, inp, c7, c14, 1         // DCache
        add             inp, inp, #HW_CACHE_LINE_SIZE
        cmp             inp, outp_save
        blt             @cacheflush

        ldmfd           sp!, {r4-r7}
@exit   bx              lr
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
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_InitCoprocessor
  Description:  �V�X�e������R�v���Z�b�T������������B
                �����ɁAI-TCM �y�� D-TCM ���g�p�\�ȏ�Ԃɂ���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_InitCoprocessor(void)
{
        /* �R�v���Z�b�T�̏�Ԏ擾 */
        mrc             p15, 0, r0, c1, c0, 0

        tst             r0, #HW_C1_PROTECT_UNIT_ENABLE
        beq             @010
        tst             r0, #HW_C1_DCACHE_ENABLE
        beq             @003

        /* D-Cache ���e���������Ƀ��C�g�o�b�N */
        mov             r1, #0
@001:   mov             r2, #0
@002:   orr             r3, r1, r2
        mcr             p15, 0, r3, c7, c10, 2
        add             r2, r2, #HW_CACHE_LINE_SIZE
        cmp             r2, #HW_DCACHE_SIZE / 4
        blt             @002
        adds            r1, r1, #1 << HW_C7_CACHE_SET_NO_SHIFT
        bne             @001

@003:   /* ���C�g�o�b�t�@����ɂȂ�̂�҂� */
        mov             r1, #0
        mcr             p15, 0, r1, c7, c10, 4

@010:   /* �R�v���Z�b�T�̏�Ԃ������� */
        ldr             r1, = HW_C1_ITCM_LOAD_MODE          \
                            | HW_C1_DTCM_LOAD_MODE          \
                            | HW_C1_ITCM_ENABLE             \
                            | HW_C1_DTCM_ENABLE             \
                            | HW_C1_LD_INTERWORK_DISABLE    \
                            | HW_C1_ICACHE_ENABLE           \
                            | HW_C1_DCACHE_ENABLE           \
                            | HW_C1_PROTECT_UNIT_ENABLE
        bic             r0, r0, r1
        ldr             r1, = HW_C1_SB1_BITSET              \
                            | HW_C1_EXCEPT_VEC_UPPER
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        /* I-TCM �̃T�C�Y��ݒ� */
        mov             r1, #HW_C9_TCMR_32MB
        mcr             p15, 0, r1, c9, c1, 1
        /* D-TCM �̃T�C�Y�y�ї̈�x�[�X�A�h���X��ݒ� */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        orr             r1, r1, #HW_C9_TCMR_16KB
        mcr             p15, 0, r1, c9, c1, 0

        /* I-TCM / D-TCM �g�p���ݒ� */
        mov             r1, #HW_C1_ITCM_ENABLE | HW_C1_DTCM_ENABLE
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_InitRegion
  Description:  ���[�W���������ݒ���s���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
/* When hardware is TWL
; Region G:  BACK_GROUND:  Base = 0x0,        Size = 4GB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 0:  IO_VRAM:      Base = 0x04000000, Size = 64MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 1:  MAINMEM_WRAM: Base = 0x02000000, Size = 32MB,  I:Cach Buf / D:Cach Buf,  I:RW / D:RW
; Region 2:  ARM7_RESERVE: Base = 0x02f80000, Size = 512KB, I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 3:  EX_MAINMEM:   Base = 0x0d000000, Size = 16MB,  I:Cach Buf / D:Cach Buf,  I:RW / D:RW
; Region 4:  DTCM:         Base = 0x02fe0000, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 5:  ITCM:         Base = 0x01000000, Size = 16MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 6:  BIOS:         Base = 0xffff0000, Size = 32KB,  I:Cach NB  / D:Cach NB,   I:RO / D:RO
; Region 7:  SHARED_WORK:  Base = 0x02ffc000, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
*/
/* When hardware is NITRO
; Region G:  BACK_GROUND:  Base = 0x0,        Size = 4GB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 0:  IO_VRAM:      Base = 0x04000000, Size = 64MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 1:  MAIN_MEM:     Base = 0x02000000, Size = 8MB*,  I:Cach Buf / D:Cach Buf,  I:RW / D:RW
;            (* When hardware is not debugger, size will be reduced to 4MB in OS_InitArena() )
; Region 2:  ARM7_RESERVE: Base = 0x027e0000, Size = 128KB, I:NC NB    / D:NC NB,     I:NA / D:NA
;            (* When hardware is not debugger, base will be moved to 0x023e0000 in OS_InitArena() )
; Region 3:  CARTRIDGE:    Base = 0x08000000, Size = 128MB, I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 4:  DTCM:         Base = 0x02fe0000, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 5:  ITCM:         Base = 0x01000000, Size = 16MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 6:  BIOS:         Base = 0xffff0000, Size = 32KB,  I:Cach NB  / D:Cach NB,   I:RO / D:RO
; Region 7:  SHARED_WORK:  Base = 0x02fff000, Size = 4KB,   I:NC NB    / D:NC NB,     I:NA / D:RW
*/

static asm void
INITi_InitRegion(void)
{
#define SET_PROTECTION_A(id, adr, siz)      ldr r0, =(adr|HW_C6_PR_##siz|HW_C6_PR_ENABLE)
#define SET_PROTECTION_B(id, adr, siz)      mcr p15, 0, r0, c6, id, 0
#define REGION_BIT(a, b, c, d, e, f, g, h)  (((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | ((e) << 4) | ((f) << 5) | ((g) << 6) | ((h) << 7))
#define REGION_ACC(a, b, c, d, e, f, g, h)  (((a) << 0) | ((b) << 4) | ((c) << 8) | ((d) << 12) | ((e) << 16) | ((f) << 20) | ((g) << 24) | ((h) << 28))
#define NA      0
#define RW      1
#define RO      5

        /* (0) I/O ���W�X�^�y�� VRAM �� */
        SET_PROTECTION_A(c0, HW_IOREG, 64MB)
        SET_PROTECTION_B(c0, HW_IOREG, 64MB)

        /* (4) D-TCM */
        ldr             r0, =SDK_AUTOLOAD_DTCM_START
        orr             r0, r0, #HW_C6_PR_16KB | HW_C6_PR_ENABLE
        SET_PROTECTION_B(c4, SDK_AUTOLOAD_DTCM_START, 16KB)

        /* (5) I-TCM */
        SET_PROTECTION_A(c5, HW_ITCM_IMAGE, 16MB)
        SET_PROTECTION_B(c5, HW_ITCM_IMAGE, 16MB)

        /* (6) �V�X�e���R�[�� ROM */
        SET_PROTECTION_A(c6, HW_BIOS, 32KB)
        SET_PROTECTION_B(c6, HW_BIOS, 32KB)

        /* TWL �n�[�h�E�F�A��œ��삵�Ă��邩�ǂ����𒲍� */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        beq             @002

@001:   /* �n�[�h�E�F�A�� TWL �̏ꍇ */
        /* (1) ���C���������y�� WRAM */
        SET_PROTECTION_A(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)
        SET_PROTECTION_B(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)

        /* (2) ARM7 ��p���C����������� */
        SET_PROTECTION_A(c2, HW_TWL_MAIN_MEM_SUB, 512KB)
        SET_PROTECTION_B(c2, HW_TWL_MAIN_MEM_SUB, 512KB)

        /* (3) �g�����C�������� */
        SET_PROTECTION_A(c3, HW_TWL_MAIN_MEM_EX, 16MB)
        SET_PROTECTION_B(c3, HW_TWL_MAIN_MEM_EX, 16MB)

        /* (7) ARM9/ARM7 ���L���C����������� */
        SET_PROTECTION_A(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)
        SET_PROTECTION_B(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)

        /* ���߃L���b�V������ */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 1

        /* �f�[�^�L���b�V������ */
//        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mov             r0, #REGION_BIT(0, 1, 1, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 0

        /* ���C�g�o�b�t�@���� */
//        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 0, 0)
        mov             r0, #REGION_BIT(0, 1, 1, 1, 0, 0, 0, 0)
        mcr             p15, 0, r0, c3, c0, 0

        /* ���߃A�N�Z�X���� */
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA)
        mcr             p15, 0, r0, c5, c0, 3

        /* �f�[�^�A�N�Z�X���� */
//        ldr             r0, =REGION_ACC(RW, RW, NA, RW, RW, RW, RO, RW)
        ldr             r0, =REGION_ACC(RW, RW, RW, RW, RW, RW, RO, RW)
        mcr             p15, 0, r0, c5, c0, 2
//        b               @003

@002:   /* �n�[�h�E�F�A�� NITRO �̏ꍇ */
#if 0
        /* (1) ���C�������� */
        SET_PROTECTION_A(c1, HW_MAIN_MEM_MAIN, 8MB)
        SET_PROTECTION_B(c1, HW_MAIN_MEM_MAIN, 8MB)
        /* Size will be arranged in OS_InitArena(). */

        /* (2) ARM7 ��p���C����������� */
        SET_PROTECTION_A(c2, (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE - HW_MAIN_MEM_SUB_SIZE), 128KB)
        SET_PROTECTION_B(c2, (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE - HW_MAIN_MEM_SUB_SIZE), 128KB)
        /* Base address will be moved in OS_InitArena(). */

        /* (3) �J�[�g���b�W */
        SET_PROTECTION_A(c3, HW_CTRDG_ROM, 128MB)
        SET_PROTECTION_B(c3, HW_CTRDG_ROM, 128MB)

        /* (7) ARM9/ARM7 ���L���C����������� */
        SET_PROTECTION_A(c7, HW_MAIN_MEM_SHARED, 4KB)
        SET_PROTECTION_B(c7, HW_MAIN_MEM_SHARED, 4KB)

        /* ���߃L���b�V������ */
        mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 1

        /* �f�[�^�L���b�V������ */
        mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 0

        /* ���C�g�o�b�t�@���� */
        mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 0, 0)
        mcr             p15, 0, r0, c3, c0, 0

        /* ���߃A�N�Z�X���� */
        ldr             r0, =REGION_ACC(RW, RW, NA, NA, NA, RW, RO, NA)
        mcr             p15, 0, r0, c5, c0, 3

        /* �f�[�^�A�N�Z�X���� */
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, RW, RW, RO, RW)
        mcr             p15, 0, r0, c5, c0, 2
#endif
@003:   /* �v���e�N�V�������j�b�g�y�уL���b�V���g�p���ݒ� */
        mrc             p15, 0, r0, c1, c0, 0
        ldr             r1, = HW_C1_ICACHE_ENABLE       \
                            | HW_C1_DCACHE_ENABLE       \
                            | HW_C1_CACHE_ROUND_ROBIN   \
                            | HW_C1_PROTECT_UNIT_ENABLE
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        /* �L���b�V���̓��e��j�� */
        mov             r1, #0
        mcr             p15, 0, r1, c7, c6, 0
        mcr             p15, 0, r1, c7, c5, 0

        bx              lr
}


/*---------------------------------------------------------------------------*
  Name:         TwlStartUp

  Description:  hook for user start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void TwlStartUp(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         OSi_ReferSymbol

  Description:  used by SDK_REFER_SYMBOL macro to avoid dead-strip.

  Arguments:    symbol            unused

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_ReferSymbol(void *symbol)
{
#pragma unused(symbol)
}
