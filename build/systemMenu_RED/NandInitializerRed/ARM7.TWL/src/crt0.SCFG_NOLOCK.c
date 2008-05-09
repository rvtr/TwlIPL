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
    外部参照
 *---------------------------------------------------------------------------*/
/* リンカスクリプトにより定義されるシンボル参照 */
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

/* 外部関数参照 */
extern void OS_IrqHandler(void);
#ifndef SDK_NOINIT
extern void _fp_init(void);
extern void __call_static_initializers(void);
#endif

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621
#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

/* .rodata セクションに配置するロードに必要な情報 */
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
  Description:  ARM9 の ShakeHand 関数と同期を取る。
                メインメモリでないメモリ空間で実行される必要がある。
  Arguments:    r0  -   ARM9 同期用変数へのポインタ。
                r1  -   ARM7 同期用変数へのポインタ。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static const u32    microcode_ShakeHand[10] =
{
    0xe1d020b0, /*      ldrh    r2, [r0]    ; 同期変数９を共有メモリから読む */
    0xe1d130b0, /*      ldrh    r3, [r1]    ; 同期変数７を共有メモリから読む */
    0xe2833001, /*  @1: add     r3, r3, #1  ; 同期変数７ ++ */
    0xe1c130b0, /*      strh    r3, [r1]    ; 同期変数７を共有メモリに書く */
    0xe1d0c0b0, /*      ldrh    r12, [r0]   ; 同期変数９の現状を共有メモリから読む */
    0xe152000c, /*      cmp     r2, r12     ; 同期変数９の変化を判定する */
    0x0afffffa, /*      beq     @1          ; 変化していなければループ */
    0xe2833001, /*      add     r3, r3, #1  ; 同期変数７ ++ */
    0xe1c130b0, /*      strh    r3, [r1]    ; 同期変数７を共有メモリに書く */
    0xe12fff1e  /*      bx      lr          ; ハンドシェイク完了 */
};

/*---------------------------------------------------------------------------*
  Name:         Stop
  Description:  プログラムを停止する。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static const u32    microcode_Stop[2] =
{
    0xef000006, /*  @1: swi     #6          ; SVC_Halt */
    0xeafffffd  /*      b       @1          ; ループ */
};

/*---------------------------------------------------------------------------*
  Name:         GotoMain
  Description:  ARM9 に特定の状態になったことを伝えた上で、Main 関数へジャンプ
                する。同時に指定バッファの 0 クリアを行う。スタック内で動作させ
                ることを想定している為、スタックを一切使用しない。
  Arguments:    r0  -   0 クリアするバッファの先頭アドレス。
                        4 バイトアラインされたアドレスである必要がある。
                r1  -   上位  8 bit: ARM9 に伝えるフェーズ番号。
                        下位 24 bit: 0 クリアするバッファのサイズ。
                                     4 の倍数である必要がある。
                r2  -   同期用フェーズ管理変数へのポインタ。
                r3  -   Main 関数のポインタ。Main 関数が Thumb コードである場合
                        には、bx 命令でジャンプするため最下位ビットが 1 になって
                        いる必要がある。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static const u32    microcode_GotoMain[10]   =
{
    0xe59fc01c, /*  @1: ldr     r12, [pc, #28]  ; 下位 24 bit マスクを用意 */
    0xe111000c, /*      tst     r1, r12         ; クリアバッファ残サイズをチェック */
    0xe3a0c000, /*      mov     r12, #0         ; クリア用変数を用意 */
    0x1480c004, /*      strne   r12, [r0], #4   ; 4 バイトクリア */
    0x12411004, /*      subne   r1, r1, #4      ; クリアバッファ残サイズ -= 4 */
    0x1afffff9, /*      bne     @1              ; 残サイズが 0 になるまでループ */
    0xe1a01c21, /*      mov     r1, r1, LSR #24 ; フェーズ管理変数の更新値を用意 */
    0xe1c210b0, /*      strh    r1, [r2]        ; フェーズ管理変数を更新 */
    0xe12fff13, /*      bx      r3              ; Main 関数へジャンプ */
    0x00fffffc  /*      <DATA>  0x00fffffc */
};


/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
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
  Description:  起動ベクタ。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start(void)
{
@000:
        /* 割込み禁止 */
        mov         r12, #HW_REG_BASE
        str         r12, [r12, #REG_IME_OFFSET]

        /* SCFG 設定を確認 */
        bl          INITi_CheckSysConfig

        /* ランチャーから渡された情報を退避 */
        ldr         r3, =SDK_WRAM_ARENA_LO
        sub         r2, r3, #0x40
        ldr         r1, =HW_LAUNCHER_DELIVER_PARAM_BUF
@001:
        cmp         r2, r3
        ldrlt       r0, [r1], #4
        strlt       r0, [r2], #4
        blt         @001

        /* ハンドシェイク用マイクロコードを専用 WRAM にコピー */
        ldr         r1, =microcode_ShakeHand
        ldr         r2, =HW_PRV_WRAM
        add         r3, r2, #40     // sizeof(microcode_ShakeHand)
@002:   ldr         r0, [r1], #4
        str         r0, [r2], #4
        cmp         r2, r3
        blt         @002

        /* 専用 WRAM 上のコードで ARM9 とハンドシェイク */
        ldr         r0, =HW_BOOT_SHAKEHAND_9
        ldr         r1, =HW_BOOT_SHAKEHAND_7
        ldr         r2, =HW_PRV_WRAM
        mov         lr, pc
        bx          r2

@010:
        /* スタックポインタ設定 */
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

        /* スタック領域をクリア */
        ldr         r0, =SDK_SYS_STACKSIZE
        sub         r1, r1, r0
        ldr         r2, =HW_PRV_WRAM_IRQ_STACK_END
        mov         r0, #0
@011:   cmp         r1, r2
        strlt       r0, [r1], #4
        blt         @011

        /* Autoload を実施 */
        bl          INITi_DoAutoload

        /* STATIC セグメントの .bss セクションを 0 クリア */
        mov         r1, #0          // r1 = clear value for STATIC bss section
        ldr         r3, =_start_ModuleParams
        ldr         r0, [r3, #12]   // r0 = start address of STATIC bss section
        ldr         r2, [r3, #16]
        subs        r2, r2, r0      // r2 = size of STATIC bss section
        blgt        INITi_Fill32

        /* メインメモリサイズを調査 */
        bl          INITi_DetectMainMemorySize

#ifndef SDK_FINALROM
        /* デバッグ出力ウィンドウを設定 */
        ldr         r1, =HW_PRINT_OUTPUT_ARM9
        mov         r0, #OS_PRINT_OUTPUT_DEFAULT_ARM9
        orr         r0, r0, #(OS_PRINT_OUTPUT_DEFAULT_ARM7 << 8)
        strh        r0, [r1]
        mov         r0, #OS_PRINT_OUTPUT_DEFAULT_ARM9ERR
        orr         r0, r0, #(OS_PRINT_OUTPUT_DEFAULT_ARM7ERR << 8)
        strh        r0, [r1, #2]
#endif

        /* IRQ 割込みベクタ設定 */
        ldr         r1, =HW_INTR_VECTOR_BUF
        ldr         r0, =OS_IrqHandler
        str         r0, [r1]

#ifndef SDK_NOINIT
        /* c++ 用初期化 */
        bl          _fp_init
        bl          TwlSpStartUp
        bl          __call_static_initializers
        bl          INITi_CallStaticInitializers
#endif

@0f0:
        /* Main 関数へのジャンプ用マイクロコードをスタックの底にコピー */
        ldr         r1, =microcode_GotoMain
        sub         r2, sp, #40
        mov         r3, sp
@0f1:   cmp         r2, r3
        ldrlt       r0, [r1], #4
        strlt       r0, [r2], #4
        blt         @0f1

        /* マイクロコードを経由して Main 関数へジャンプ */
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
        bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CheckSysConfig
  Description:  SCFG 関連 I/O レジスタの内容を確認する。
                A7-SCFG ブロックがアクセス有効な場合には、ローダーが行う設定処
                理を代行した上でアクセスを無効化する。
                ARM7 専用 WRAM 内にローダーが展開している I/O レジスタ情報を確
                認した上でメインメモリ上の共有領域にコピーする。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static asm void
INITi_CheckSysConfig(void)
{
        /* A7-SCFG のアクセス可否判定 */
        ldr         r1, =REG_EXT_ADDR
        ldr         r0, [r1]
        ldr         r2, =HW_PRV_WRAM_SYSRV
        tst         r0, #REG_SCFG_EXT_CFG_MASK
        beq         @invalid

@valid:
        /* ARM7 Secure-ROM 切り離し */
        ldr         r1, =REG_A7ROM_ADDR
        ldrb        r0, [r1]
        orr         r0, r0, #REG_SCFG_A7ROM_SEC_MASK
        strb        r0, [r1]

        /* ARM9 Secure-ROM 切り離し */
        ldr         r1, =REG_A9ROM_ADDR
        ldrb        r0, [r1]
        orr         r0, r0, #REG_SCFG_A9ROM_SEC_MASK
        strb        r0, [r1]

        /* NITRO 無線を有効化 */
        ldr         r1, =REG_WL_ADDR
        ldrh        r0, [r1]
        orr         r0, r0, #REG_SCFG_WL_OFFB_MASK
        strh        r0, [r1]

        /* ROM 設定、NITRO 無線設定、ボンディングオプション情報を WRAM に展開 */
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

        /* 各新規ブロックへクロックを供給 */
        ldr         r1, =REG_CLK_ADDR
        ldrh        r0, [r1]
        ldr         r3, =REG_SCFG_CLK_SNDMCLK_MASK | REG_SCFG_CLK_WRAMHCLK_MASK | REG_SCFG_CLK_AESHCLK_MASK | REG_SCFG_CLK_SD2HCLK_MASK | REG_SCFG_CLK_SD1HCLK_MASK
        orr         r0, r0, r3
        strh        r0, [r1]

        /* JTAG 情報を WRAM に展開 */
        ldr         r3, =REG_JTAG_ADDR      // SCFG-JTAG
        ldrh        r1, [r3]
        and         r0, r1, #(REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK)
        and         r3, r1, #REG_SCFG_JTAG_DSPJE_MASK
        orr         r0, r0, r3, LSR #(REG_SCFG_JTAG_DSPJE_SHIFT - HWi_WSYS09_JTAG_DSPJE_SHIFT)
        strb        r0, [r2, #HWi_WSYS09_WRAMOFFSET]

        /* 各拡張機能を有効化 */
        ldr         r1, =REG_EXT_ADDR       // SCFG_EXT
        ldr         r0, [r1]
        ldr         r3, =REG_SCFG_EXT_WRAM_MASK | REG_SCFG_EXT_GPIO_MASK | REG_SCFG_EXT_I2C_MASK | REG_SCFG_EXT_I2S_MASK | REG_SCFG_EXT_MIC_MASK | REG_SCFG_EXT_SD2_MASK | REG_SCFG_EXT_SD1_MASK | REG_SCFG_EXT_AES_MASK | REG_SCFG_EXT_DMAC_MASK
        orr         r0, r0, r3
        orr         r0, r0, #(REG_SCFG_EXT_DSEL_MASK | REG_SCFG_EXT_INTC_MASK)
        bic         r0, r0, #REG_SCFG_EXT_MC_B_MASK
        str         r0, [r1]

        /* 各拡張機能の制御設定内容を WRAM に展開 */
        str         r0, [r2, #HWi_WSYS04_WRAMOFFSET]

        /* メモリーカード I/F のスロット選択 */
        ldr         r1, =REG_MC1_ADDR       // SCFG_MC1
        ldr         r0, [r1]
        bic         r0, r0, #REG_MI_MC1_SWP_MASK
        str         r0, [r1]

        /* WRAM-A/B/C が ARM7 に割り当たっていることを確認 */
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

        /* WRAM-A/B/C のメモリマップを設定 */
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

        /* A7-SCFG ブロックへのアクセスを無効化 */
        ldr         r1, =REG_EXT_ADDR
        ldr         r0, [r1]
/*      bic         r0, r0, #REG_SCFG_EXT_CFG_MASK */
        str         r0, [r1]

@invalid:
        /* ROM 設定、NITRO 無線設定内容を確認 */
        ldrb        r0, [r2, #HWi_WSYS08_WRAMOFFSET]
        and         r0, r0, #(HWi_WSYS08_ROM_ARM7RSEL_MASK | HWi_WSYS08_ROM_ARM9RSEL_MASK | HWi_WSYS08_ROM_ARM9SEC_MASK)
        cmp         r0, #HWi_WSYS08_ROM_ARM9SEC_MASK
        blne        INITi_Stop

        /* SCFG レジスタ設定情報を共有領域にコピー */
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
  Description:  メインメモリサイズを調査する。
                調査結果は (u16*)HW_MMEMCHECER_SUB に格納される。
                格納される値は [OS_CONSOLE_SIZE_16MB|OS_CONSOLE_SIZE_32B]
    NOTE:       プラットフォームが NITRO の場合は考慮していない。
  Arguments:    なし。
  Returns:      なし。
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

		/* OSi_CHECKNUM1 (0x55) 書き込みテスト */
		mov			r1, #OSi_CHECKNUM1
		strb		r1, [r3]

		ldr			r2, =OSi_100usWAIT
@1		subs		r2, r2, #4
		bcs			@1

		ldrb		r1, [r3]
		cmp			r1, #OSi_CHECKNUM1
		bne			@check_smix

		/* OSi_CHECKNUM2 (0xaa) を 書き込みテスト */
        mov     	r1, #OSi_CHECKNUM2
        strb    	r1, [r3]

		ldr     	r2, =OSi_100usWAIT
@2		subs    	r2, r2, #4
        bcs     	@2

        ldrb    	r1, [r3]
        cmp     	r1, #OSi_CHECKNUM2
        moveq   	r0, #OS_CONSOLE_SIZE_32MB

@check_smix:
        /* SMIX レジスタを調査 */
        ldr         r3, =REG_SMX_CNT_ADDR
        ldrh        r1, [r3]
        and         r1, r1, #OSi_DETECT_NITRO_MASK
        cmp         r1, #OSi_DETECT_NITRO_VAL
        orreq       r0, r0, #OS_CHIPTYPE_SMX_MASK

        /* 調査結果を格納 */
        ldr 	    r2, =HW_MMEMCHECKER_SUB
        strb        r0, [r2]
        bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_Stop
  Description:  プログラムを停止する。ARM7 専用 WRAM に Halt を繰り返すループ
                関数をコピーし、コピーした関数を実行する。
  Arguments:    なし。
  Returns:      なし。
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
  Description:  リンク情報に沿って、各オートロードブロックの固定データ部の展開
                及び変数部の 0 クリアを行う。
                NITRO 互換オートロードセグメントは存在せず、TWL 専用セグメント
                のみが存在するという前提。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/

static asm void
INITi_DoAutoload(void)
{
        stmfd       sp!, {lr}

        /* オートロードを実施 */
@000:
        ldr         r0, =_start_LtdModuleParams
        ldr         r12, [r0]       // r12 = SDK_LTDAUTOLOAD_LIST
        ldr         r3, [r0, #4]    // r3 = SDK_LTDAUTOLOAD_LIST_END
        ldr         r1, [r0, #8]    // r1 = SDK_LTDAUTOLOAD_START
@001:
        cmp         r12, r3
        bge         @010
        /* 固定セクション */
        ldr         r0, [r12], #4   // r0 = start address of destination range of fixed section
        ldr         r2, [r12], #4   // r2 = size of fixed section
        bl          INITi_Copy32
        stmfd       sp!, {r0, r1}
        /* static initializer テーブル */
        ldr         r0, [r12], #4
#ifndef SDK_NOINIT
        stmfd       sp!, {r3, r12}
        bl          INITi_ShelterStaticInitializer
        ldmfd       sp!, {r3, r12}
#endif
        /* bss セクション */
        ldmfd       sp!, {r0}       // r0 = start address of destination range of bss section
        mov         r1, #0          // r1 = clear value for bss section
        ldr         r2, [r12], #4   // r2 = sizeo of bss section
        bl          INITi_Fill32
        ldmfd       sp!, {r1}
        b           @001

        /* オートロードリスト・オートロード元バッファをクリア */
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
        /* オートロード完了コールバック呼び出し */
        ldr         r0, =_start_ModuleParams
        ldr         r1, =_start_LtdModuleParams
        ldmfd       sp!, {lr}
        b           _start_AutoloadDoneCallback
}

#ifndef SDK_NOINIT

static asm void
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
        cmp         r0, #0
        bxeq        lr

        /* 退避場所先頭アドレスを計算 */
        ldr         r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr         r2, =SDK_IRQ_STACKSIZE
        sub         r1, r1, r2
        add         r1, r1, #4
@001:
        /* 退避場所先頭から空き場所を調査 */
        ldr         r2, [r1]
        cmp         r2, #0
        addne       r1, r1, #4
        bne         @001
@002:
        /* 空き場所にテーブルをコピー */
        ldr         r2, [r0], #4
        str         r2, [r1], #4
        cmp         r2, #0
        bne         @002

        bx          lr
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
        stmdb       sp!, {lr}

        /* テーブル退避場所先頭アドレスを計算 */
        ldr         r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr         r2, =SDK_IRQ_STACKSIZE
        sub         r1, r1, r2
        add         r1, r1, #4
@001:
        /* テーブルに管理されているポインタを一つずつ呼び出し */
        ldr         r0, [r1]
        cmp         r0, #0
        beq         @002
        stmfd       sp!, {r1}
        mov         lr, pc
        bx          r0
        ldmfd       sp!, {r1}
        /* 一旦呼び出したポインタはゼロクリア (IRQスタックを間借りしている為) */
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
  Description:  32 bit 単位でコピーを行う。スタックを 36 バイト消費するが、
                レジスタ r3 - r12 は破壊しない。
  Arguments:    r0  -   コピー先へのポインタ ( 4 バイトアライン )。
                r1  -   コピー元へのポインタ ( 4 バイトアライン )。
                r2  -   コピーする長さをバイト単位で指定 ( 4 の倍数 )。
  Returns:      r0  -   コピー後のコピー先へのポインタ ( r0 + r2 )。
                r1  -   コピー後のコピー元へのポインタ ( r1 + r2 )。
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
  Description:  32 bit 単位でバッファ内容を指定データで埋める。スタックを 36
                バイト消費するが、レジスタ r3 - r12 は破壊しない。
  Arguments:    r0  -   バッファへのポインタ ( 4 バイトアライン )。
                r1  -   バッファを埋める内容を 32 bit 値で指定。
                r2  -   バッファを埋める長さをバイト単位で指定 ( 4 の倍数 )。
  Returns:      r0  -   処理後のバッファへのポインタ ( r0 + r2 )。
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
