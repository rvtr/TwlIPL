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
#include    <twl/code32.h>
#include    <firm.h>
#include    <twl/hw/ARM7/mmap_wramEnv.h>

void    _start(void);
void    _start_AutoloadDoneCallback(void *argv[]);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

// volatile parameters in IPL's work memory
#define IPL_PARAM_CARD_ROM_HEADER      0x023FE940
#define IPL_PARAM_DOWNLOAD_PARAMETER   0x023FE904

/* 外部関数参照定義 */
extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);

static void INITi_DoAutoload(void);
static void INITi_ShelterLtdBinary(void);
static void detect_main_memory_size(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32* ptr);
static void INITi_CallStaticInitializers(void);
#endif

/* リンカスクリプトにより定義されるシンボル参照 */
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
    (void*)0,       // SDK_VERSION_ID   // SDK version info /* [TODO] ビルドを通すため */
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

        //---- read reset flag from pmic
#ifdef FIRM_DISABLE_CR_AT_WARMBOOT
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
        bl              PXI_SendByIntf
        mov             r0, #FIRM_PXI_ID_INIT_MMEM
        bl              PXI_WaitByIntf
#endif // SDK_TS
#endif // FIRM_DISABLE_CR_AT_WARMBOOT

        //---- wait for main memory mode into burst mode
        ldr             r3, =REG_EXMEMCNT_L_ADDR
        mov             r1, #REG_MI_EXMEMCNT_L_ECE2_MASK
@1:
        ldrh            r2, [r3]
        tst             r2, r1
        beq             @1

#if 0
        // move parameters from IPL's work memory to shared area
        ldr             r0, =IPL_PARAM_CARD_ROM_HEADER
        ldr             r1, =HW_CARD_ROM_HEADER
        add             r2, r1, #HW_CARD_ROM_HEADER_SIZE
@1_1:
        ldr             r3, [r0], #4
        str             r3, [r1], #4
        cmp             r1, r2
        bmi             @1_1
        ldr             r0, =IPL_PARAM_DOWNLOAD_PARAMETER
        add             r2, r1, #HW_DOWNLOAD_PARAMETER_SIZE
@1_2:
        ldr             r3, [r0], #4
        str             r3, [r1], #4
        cmp             r1, r2
        bmi             @1_2
#endif

        //---- load autoload block and initialize bss
        //bl              INITi_DoAutoload
#ifndef SDK_FINALROM    // for IS-TWL-DEBUGGER
        bl          _start_AutoloadDoneCallback
#endif

        //---- fill static static bss with 0
        ldr             r0, =_start_ModuleParams
        ldr             r1,  [r0, #12]   // BSS segment start
        ldr             r2,  [r0, #16]   // BSS segment end
        mov             r0, #0
@2:     cmp             r1, r2
        strcc           r0, [r1], #4
        bcc             @2

        //---- detect main memory size
        bl              detect_main_memory_size

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
  Name:         INITi_DoAutoload
  Description:  リンク情報に沿って、各オートロードブロックの固定データ部の展開
                及び変数部の 0 クリアを行う。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
/*
 * < 二段階オートロード >
 * 0x02f88000 に crt0 及び一段目ロード元バイナリが配置されている。
 *  NITRO と共有可能な WRAM 上に配置されるべきバイナリデータを 0x037c0000 にロードする。
 *  TWL でしか動作しない WRAM 上に配置されるべきバイナリデータを続きのアドレスにロードする。
 * 0x02e80000 に二段目ロード元バイナリが配置されている。
 *  0x04000 バイト分はカード ROM から再読み出し不可なので、0x02f84000 - 0x02f88000 に退避する。
 *  NITRO と共有可能な MAIN 上に配置されるべきバイナリデータを 0x02f88000 + sizeof(crt0) にロードする。
 *  TWL でしか動作しない MAIN 上に配置されるべきバイナリデータを続きのアドレスにロードする。
 */
static asm void
INITi_DoAutoload(void)
{
@000:
        stmdb           sp!, {lr}
        /* WRAM 用ブロックをオートロード */
        ldr             r1, =_start_ModuleParams
        ldr             r12, [r1]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_AUTOLOAD_START
@001:   cmp             r12, r0
        bge             @010
        /* 固定セクションをロード */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@002:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @002
        /* static initializer テーブル情報を読み出し */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           sp!, {r0-r3, r12}
#endif
        /* .bss セクションを 0 クリア */
        mov             r0, #0
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@003:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @003
@004:   ldmia           sp!, {r0}
        b               @001

@010:   /* メインメモリ用ブロックの存在を確認 */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1dc  /* ARM7 用拡張常駐モジュール ROM サイズ */
        ldr             r0, [r1]
        cmp             r0, #0
        beq             @020

        /* 再読み出し不可部分を退避 */
        bl              INITi_ShelterLtdBinary

        /* メインメモリ用ブロックをオートロード */
        ldr             r1, =_start_LtdModuleParams
        ldr             r12, [r1]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_LTDAUTOLOAD_START
@011:   cmp             r12, r0
        bge             @020
        /* 固定セクションをロード */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@012:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @012
        /* static initializer テーブル情報を読み出し */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           sp!, {r0-r3, r12}
#endif
        /* .bss セクションを 0 クリア */
        mov             r0, #0
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@013:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @013
@014:   ldmia           sp!, {r0}
        b               @011

@020:   /* オートロード完了コールバック関数呼び出し */
        ldr             r0, =_start_ModuleParams
        ldr             r1, =_start_LtdModuleParams
        ldmia           sp!, {lr}
        b               _start_AutoloadDoneCallback
}

/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterLtdBinary
  Description:  TWL 専用のオートロード元バイナリデータの内、カード ROM から
                再読み出しできない領域のデータを退避エリアに退避する。
                再読み出しできない領域のデータは ARM7 用と ARM9 用の拡張常駐
                モジュールの２つに分かれている可能性があるので、冗長ではあるが
                両方の先頭から 0x4000 分をそれぞれ退避する。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static asm void
INITi_ShelterLtdBinary(void)
{
        /* 退避元・先アドレスを調査 */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1d8  /* ARM7 用拡張常駐モジュール RAM アドレス */
        ldr             r1, [r1]
        ldr             r3, =HW_TWL_ROM_HEADER_BUF + 0x038  /* ARM7 用常駐モジュール RAM アドレス */
        ldr             r3, [r3]
        sub             r2, r3, #0x4000                     /* 再読み出し不可領域サイズ */ /* ARM7 用退避エリア */

        /* コピー */
@loop:  ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @loop

        bx              lr
}

#ifndef SDK_NOINIT
/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterStaticInitializer
  Description:  各オートロードセグメント内の static initializer へのポインタ
                テーブルを IRQ スタックの最上部に退避する。
  Arguments:    ptr     -   セグメント内のポインタテーブルへのポインタ。
                            テーブルは NULL で終端されている必要がある。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static asm void
INITi_ShelterStaticInitializer(u32* ptr)
{
        /* 引数確認 */
        cmp             r0, #0
        bxeq            lr

        /* 退避場所先頭アドレスを計算 */
        ldr             r1, =HW_FIRM_IRQ_STACK_END
        ldr             r2, =HW_FIRM_IRQ_STACK_SIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* 退避場所先頭から空き場所を調査 */
@001:   ldr             r2, [r1]
        cmp             r2, #0
        addne           r1, r1, #4
        bne             @001

        /* 空き場所にテーブルをコピー */
@002:   ldr             r2, [r0], #4
        str             r2, [r1], #4
        cmp             r2, #0
        bne             @002

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CallStaticInitializers
  Description:  各オートロードセグメント内の static initializer を呼び出す。
                オートロード処理によって IRQ スタックの最上部に退避されている
                関数ポインタテーブルを一つずつ呼び出す。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static asm void
INITi_CallStaticInitializers(void)
{
        stmdb           sp!, {lr}

        /* テーブル退避場所先頭アドレスを計算 */
        ldr             r1, =HW_FIRM_IRQ_STACK_END
        ldr             r2, =HW_FIRM_IRQ_STACK_SIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* テーブルに管理されているポインタを一つずつ呼び出し */
@001:   ldr             r0, [r1]
        cmp             r0, #0
        beq             @002
        stmdb           sp!, {r1}
        mov             lr, pc
        bx              r0
        ldmia           sp!, {r1}
        /* 一旦呼び出したポインタはゼロクリア (IRQスタックを間借りしている為) */
        mov             r0, #0
        str             r0, [r1], #4
        b               @001

@002:   ldmia           sp!, {lr}
        bx              lr
}
#endif

/*---------------------------------------------------------------------------*
  Name:         _start_AutoloadDoneCallback
  Description:  オートロード完了コールバック。
  Arguments:    argv    -   オートロードパラメータを保持している配列。
                    argv[0] =   SDK_AUTOLOAD_LIST
                    argv[1] =   SDK_AUTOLOAD_LIST_END
                    argv[2] =   SDK_AUTOLOAD_START
                    argv[3] =   SDK_STATIC_BSS_START
                    argv[4] =   SDK_STATIC_BSS_END
  Returns:      なし。
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start_AutoloadDoneCallback(void* argv[])
{
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         detect_main_memory_size

  Description:  detect main memory size.
                result is written into (u32*)HW_MMEMCHECKER_SUB.
                value is [OS_CONSOLE_SIZE_4MB|OS_CONSOLE_SIZE_8MB|
                OS_CONSOLE_SIZE_16MB|OS_CONSOLE_SIZE_32MB]

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define OSi_IMAGE_DIFFERENCE  0x400000
#define OSi_IMAGE_DIFFERENCE2 0xb000000
#define OSi_DETECT_NITRO_MASK  (REG_SND_SMX_CNT_E_MASK | REG_SND_SMX_CNT_FSEL_MASK)
#define OSi_DETECT_NITRO_VAL   (REG_SND_SMX_CNT_E_MASK)

static asm void detect_main_memory_size( void )
{
//################ this process is required before IPL
#if 0
        // SCFG enable?
        ldr     r2, =REG_EXT_ADDR
        ldr     r0, [r2]
        tst     r0, #0x80000000
        beq     @9
#endif
        ldr     r2, =HW_PRV_WRAM_SYSRV
        //OPT(bonding option)
        ldr     r3, =REG_OP_ADDR
        ldrh    r0, [r3]
        strh    r0, [r2, #8]
        //OPT(JTAG info)
        ldr     r3, =REG_JTAG_ADDR
        ldrb    r0, [r3]
        //CLK(only wram clock)
        ldr     r3, =REG_CLK_ADDR
        ldrh    r1, [r3]
        and     r1, r1, #0x80
        orr     r0, r0, r1, LSR 1
        strb    r0, [r2, #9]
@9:
//################

        //---- copy scfg setting
        ldr     r2, =HW_PRV_WRAM_SYSRV
        ldr     r3, =HW_SYS_CONF_BUF
        ldr     r0, [r2, #HWi_WSYS04_WRAMOFFSET]
        str     r0, [r3, #HWi_WSYS04_OFFSET]
        ldrh    r0, [r2, #HWi_WSYS08_WRAMOFFSET]
        strh    r0, [r3, #HWi_WSYS08_OFFSET]

        //---- detect memory size
#if 0   // NITRO hardware is not supported
        mov     r0, #OS_CONSOLE_SIZE_4MB
        mov     r1, #0

        ldr     r2, =HW_MMEMCHECKER_SUB
        sub     r3, r2, #OSi_IMAGE_DIFFERENCE
@1:
        strh    r1, [r2]
        ldrh    r12, [r3]
        cmp     r1, r12
        bne     @2

        add     r1, r1, #1
        cmp     r1, #2 // check 2 loop
        bne     @1

        //---- 4MB
        b       @4

        //---- 8MB or 16MB or 32MB
@2:
        // check if running on twl/nitro
        ldr     r1, =HW_SYS_CONF_BUF
        ldrb    r12, [r1,#HWi_WSYS09_OFFSET]
        tst     r12, #HWi_WSYS09_CLK_WRAMHCLK_MASK
        moveq   r0, #OS_CONSOLE_SIZE_8MB
        beq     @4
#else
        ldr     r2, =HW_MMEMCHECKER_SUB
#endif

        //---- 16MB or 32MB
        mov     r1, #0
        add     r3, r2, #OSi_IMAGE_DIFFERENCE2
@3:
        strh    r1, [r2]
        ldrh    r12, [r3]
        cmp     r1, r12

        movne   r0, #OS_CONSOLE_SIZE_32MB
        bne     @4

        add     r1, r1, #1
        cmp     r1, #2 // check 2 loop
        bne     @3
        mov     r0, #OS_CONSOLE_SIZE_16MB
@4:

        //---- check SMX_CNT
        ldr     r3, =REG_SMX_CNT_ADDR
        ldrh    r1, [r3]
        and     r1, r1, #OSi_DETECT_NITRO_MASK
        cmp     r1, #OSi_DETECT_NITRO_VAL
        orreq   r0, r0, #OS_CHIPTYPE_SMX_MASK

        strb    r0, [r2]

        bx      lr
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
