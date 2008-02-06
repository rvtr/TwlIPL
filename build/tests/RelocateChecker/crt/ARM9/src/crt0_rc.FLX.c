/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - init - ARM9.TWL
  File:     crt0_rc.FLX.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-26#$
  $Rev: 2650 $
  $Author: nakasima $
 *---------------------------------------------------------------------------*/

#include    <nitro/types.h>
#include    <nitro/hw/common/armArch.h>
#include    <nitro/mi/uncompress.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap.h>
#include    <twl/misc.h>
#include    "boot_sync.h"

/*---------------------------------------------------------------------------*/
void    _start(void);
void    _start_AutoloadDoneCallback(void* argv[]);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

/* 外部関数参照定義 */
extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);

/* 内部関数プロトタイプ定義 */
static void INITi_CpuClear32(register u32 data, register void* destp, register u32 size);
static void INITi_InitCoprocessor(void);
static void INITi_InitRegion(void);
static void INITi_DoAutoload(void);
static void INITi_ShelterLtdBinary(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32* ptr);
static void INITi_CallStaticInitializers(void);
#endif

/* リンカスックリプトにより定義されるシンボル参照 */
extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);

void* const _start_ModuleParams[]   =
{
    (void*)SDK_AUTOLOAD_LIST,
    (void*)SDK_AUTOLOAD_LIST_END,
    (void*)SDK_AUTOLOAD_START,
    (void*)SDK_STATIC_BSS_START,
    (void*)SDK_STATIC_BSS_END,
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
  Name:         ShakeHand
  Description:  ARM7 の ShakeHand 関数と同期を取る。
                メインメモリでないメモリ空間で実行される必要がある。
  Arguments:    r0  -   ARM9 同期用変数へのポインタ。
                r1  -   ARM7 同期用変数へのポインタ。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static const u32    microcode_ShakeHand[10] =
{
    0xe1d120b0, /*      ldrh    r2, [r1]    ; 同期変数７を共有メモリから読む */
    0xe1d030b0, /*      ldrh    r3, [r0]    ; 同期変数９を共有メモリから読む */
    0xe2833001, /*  @1: add     r3, r3, #1  ; 同期変数９++ */
    0xe1c030b0, /*      strh    r3, [r0]    ; 同期変数９を共有メモリに書く */
    0xe1d1c0b0, /*      ldrh    r12, [r1]   ; 同期変数７の現状を共有メモリから読む */
    0xe152000c, /*      cmp     r2, r12     ; 同期変数７の変化を判定する */
    0x0afffffa, /*      beq     @1          ; 変化していなければループ */
    0xe2833001, /*      add     r3, r3, #1  ; 同期変数９++ */
    0xe1c030b0, /*      strh    r3, [r0]    ; 同期変数９を共有メモリに書く */
    0xe12fff1e  /*      bx      lr          ; ハンドシェイク完了 */
};

/*---------------------------------------------------------------------------*
  Name:         WaitAgreement
  Description:  ARM7 の起動ベクタが特定の状態になるまで待つ。
                メインメモリでないメモリ空間で実行される必要がある。
  Arguments:    r0  -   同期用フェーズ管理変数へのポインタ。
                r1  -   待機するフェーズ番号。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static const u32    microcode_WaitAgreement[7]  =
{
    0xe1d020b0, /*  @1: ldrh    r2, [r0]    ; フェーズ管理変数を共有メモリから読む */
    0xe1510002, /*      cmp     r1, r2      ; 待機フェーズ番号と比較 */
    0x012fff1e, /*      bxeq    lr          ; 一致していれば待機完了 */
    0xe3a03010, /*      mov     r3, #16     ; 空ループ回数初期化 */
    0xe2533001, /*  @2: subs    r3, r3, #1  ; 空ループ回数 -- */
    0x1afffffd, /*      bne     @2          ; 16 回ループ */
    0xeafffff8  /*      b       @1          ; 先頭に戻る */
};

/*---------------------------------------------------------------------------*
  Name:         SwitchCpuClock
  Description:  ARM9 CPU コアの動作クロックを変更する。
                I-TCM 上で実行される必要がある。
  Arguments:    r0  -   切り替える速度モード。
                        ( 0: 等速 / 0以外: 倍速 )
                r1  -   クロック変更後待機するサイクル数。
  Returns:      r0  -   変更前の速度モードを返す。
                        ( 0: 等速 / 1: 倍速 )
 *---------------------------------------------------------------------------*/
static const u32    microcode_SwitchCpuClock[13]    =
{
    0xe3500000, /*      cmp     r0, #0          ; 第１引数を評価 */
    0xe59f3024, /*      ldr     r3, [pc, #36]   ; REG_CLK_ADDR 定数を読み込み */
    0xe1d300b0, /*      ldrh    r0, [r3]        ; REG_CLK_ADDR 内容を読み込み */
    0x03c02001, /*      biceq   r2, r0, #1      ; 等速への変更時は REG_SCFG_CLK_CPUSPD_MASK フラグを下げる */
    0x13802001, /*      orrne   r2, r0, #1      ; 倍速への変更時は REG_SCFG_CLK_CPUSPD_MASK フラグを上げる */
    0xe1500002, /*      cmp     r0, r2          ; REG_CLK_ADDR 内容を変更する必要性を評価 */
    0xe2000001, /*      and     r0, r0, #1      ; 関数戻り値を編集 */
    0x012fff1e, /*      bxeq    lr              ; 変更する必要がない場合は関数終了 */
    0xe1c320b0, /*      strh    r2, [r3]        ; REG_CLK_ADDR に変更内容を書き込み */
    0xe2511004, /*  @1: subs    r1, r1, #4      ; 1 cycle ; 空ループ回数 -- */
    0xaafffffd, /*      bge     @1              ; 3 cycles or 1 cycle ; (待機サイクル数 / 4) 回ループ ; クロック変更完了 */
    0xe12fff1e, /*      bx      lr              ; 2 cycle目で分岐先から命令フェッチ */
    0x04004004  /*      REG_CLK_ADDR            ; REG_CLK_ADDR 定数定義 */
};

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
        /* 割り込み禁止 */
        mov             r12, #HW_REG_BASE
        str             r12, [r12, #REG_IME_OFFSET]     // Use that LSB of HW_REG_BASE is 0b0
        
        /* システム制御コプロセッサ初期化 */
        bl              INITi_InitCoprocessor

        // IPLの初期状態をカバーするための諸処理

        // (1) カードのリセット信号を再初期化
        ldr             r1, =REG_MCCNT1_ADDR
        mov             r0, #REG_MI_MCCNT1_A_RESB_MASK
        str             r0, [r1]

        // (2) shared領域のゼロクリア
        mov             r0, #0
        ldr             r1, =HW_PXI_SIGNAL_PARAM_ARM9
        ldr             r2, =(HW_MAIN_MEM_SYSTEM+HW_MAIN_MEM_SYSTEM_SIZE-HW_PXI_SIGNAL_PARAM_ARM9)
        bl              INITi_CpuClear32

        /* ハンドシェイク用マイクロコードを ITCM にコピー */
        ldr             r1, =microcode_ShakeHand
        ldr             r2, =HW_ITCM
        add             r3, r2, #40
@001:   ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @001
        
        /* ITCM 上のコードで ARM7 とハンドシェイク1 */
        ldr             r0, =HW_BOOT_SYNC_PHASE
        mov             r1, #BOOT_SYNC_PHASE_1
        strh            r1, [r0]
        ldr             r0, =HW_BOOT_SHAKEHAND_9
        ldr             r1, =HW_BOOT_SHAKEHAND_7
        ldr             r2, =HW_ITCM
        blx             r2

        /* ARM7 との同期待ち用マイクロコードを ITCM に上書きコピー */
        ldr             r1, =microcode_WaitAgreement
        ldr             r2, =HW_ITCM
        add             r3, r2, #28
@002:   ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @002

@003:
        /* ARM7 と同期 */
        ldr             r0, =HW_BOOT_SYNC_PHASE
        mov             r1, #BOOT_SYNC_PHASE_4
        ldr             r2, =HW_ITCM
        blx             r2

@010:
        /* TWL ハードウェア上で動作しているかどうかを調査 */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        beq             @020

        /* 倍速モードへの変更用マイクロコードを ITCM にコピー */
        ldr             r1, =microcode_SwitchCpuClock
        ldr             r2, =HW_ITCM
        add             r2, r2, #28
        mov             r3, #52
@011:   subs            r3, r3, #4
        ldr             r0, [r1, r3]
        str             r0, [r2, r3]
        bgt             @011
        /* CPU クロック倍速モードへ変更 */
        mov             r0, #REG_SCFG_CLK_CPUSPD_MASK
        mov             r1, #8
        blx             r2

        /* [TODO] ARM9 側でしか設定できない追加 I/O レジスタの初期設定を行う */

@020:
        /* リージョン初期設定 */
        bl              INITi_InitRegion

        /* スタックポインタ設定 */
        mov             r0, #HW_PSR_SVC_MODE    // SuperVisor mode
        msr             cpsr_c, r0
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE
        sub             sp, r1, #HW_DTCM_SYSRV_SIZE
        sub             r1, sp, #HW_SVC_STACK_SIZE
        mov             r0, #HW_PSR_IRQ_MODE    // IRQ mode
        msr             cpsr_c, r0
        sub             sp, r1, #4              // 4 bytes for stack check code
        tst             sp, #4
        subeq           sp, sp, #4              /* IRQ ハンドラにジャンプした時点で sp が 8byte アラインになるように調整 */
        ldr             r0, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r0
        mov             r0, #HW_PSR_SYS_MODE    // System mode
        msr             cpsr_csfx, r0
        sub             sp, r1, #4              // 4 bytes for stack check code
        tst             sp, #4
        subne           sp, sp, #4              /* Main 関数にジャンプした時点で sp が 8byte アラインになるように調整 */

        /* スタック領域をクリア */
        mov             r0, #0
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        mov             r2, #HW_DTCM_SIZE
        bl              INITi_CpuClear32

        /* VRAM を 0 クリア */
        mov             r0, #0
        ldr             r1, =HW_PLTT
        mov             r2, #HW_PLTT_SIZE
        bl              INITi_CpuClear32
        mov             r0, #0
        ldr             r1, =HW_OAM
        mov             r2, #HW_OAM_SIZE
        bl              INITi_CpuClear32

        /* Autoload を実施 */
        bl              INITi_DoAutoload

        /* STATIC ブロックの .bss セクションを 0 クリア */
        mov             r0, #0
        ldr             r3, =_start_ModuleParams
        ldr             r1, [r3, #12]       // SDK_STATIC_BSS_START
        ldr             r2, [r3, #16]       // SDK_STATIC_BSS_END
        sub             r2, r2, r1
        bl              INITi_CpuClear32

        /* 割り込みベクタ設定 */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE
        add             r1, r1, #HW_DTCM_SYSRV_OFS_INTR_VECTOR
        ldr             r0, =OS_IrqHandler
        str             r0, [r1]

#ifndef SDK_NOINIT
        /* c++ 用初期化 */
        bl              _fp_init
        bl              TwlStartUp
        bl              __call_static_initializers
        bl              INITi_CallStaticInitializers
#endif

        /* V カウント調整 */
        ldr             r1, =REG_VCOUNT_ADDR
@021:   ldrh            r0, [r1]
        cmp             r0, #0
        bne             @021

@030:
        /* Main 関数へジャンプ */
        ldr             r1, =TwlMain
        ldr             lr, =HW_RESET_VECTOR
        bx              r1
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CpuClear32
  Description:  32 bit 単位でバッファのクリアを行う。
  Arguments:    r0  -   クリアする値。
                r1  -   クリア先へのポインタ。
                r2  -   連続してクリアするバッファ長。
  Returns:      なし。
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
  Name:         INITi_InitCoprocessor
  Description:  システム制御コプロセッサを初期化する。
                同時に、I-TCM 及び D-TCM を使用可能な状態にする。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static asm void
INITi_InitCoprocessor(void)
{
        /* コプロセッサの状態取得 */
        mrc             p15, 0, r0, c1, c0, 0

        tst             r0, #HW_C1_PROTECT_UNIT_ENABLE
        beq             @010
        tst             r0, #HW_C1_DCACHE_ENABLE
        beq             @003

        /* D-Cache 内容をメモリにライトバック */
        mov             r1, #0
@001:   mov             r2, #0
@002:   orr             r3, r1, r2
        mcr             p15, 0, r3, c7, c10, 2
        add             r2, r2, #HW_CACHE_LINE_SIZE
        cmp             r2, #HW_DCACHE_SIZE / 4
        blt             @002
        adds            r1, r1, #1 << HW_C7_CACHE_SET_NO_SHIFT
        bne             @001

@003:   /* ライトバッファが空になるのを待つ */
        mov             r1, #0
        mcr             p15, 0, r1, c7, c10, 4

@010:   /* コプロセッサの状態を初期化 */
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

        /* I-TCM のサイズを設定 */
        mov             r1, #HW_C9_TCMR_32MB
        mcr             p15, 0, r1, c9, c1, 1
        /* D-TCM のサイズ及び領域ベースアドレスを設定 */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        orr             r1, r1, #HW_C9_TCMR_16KB
        mcr             p15, 0, r1, c9, c1, 0

        /* I-TCM / D-TCM 使用許可設定 */
        mov             r1, #HW_C1_ITCM_ENABLE | HW_C1_DTCM_ENABLE
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_InitRegion
  Description:  リージョン初期設定を行う。
  Arguments:    なし。
  Returns:      なし。
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
;// Region 2:  ARM7_RESERVE: Base = 0x027e0000, Size = 128KB, I:NC NB    / D:NC NB,     I:NA / D:NA
;//            (* When hardware is not debugger, base will be moved to 0x023e0000 in OS_InitArena() )
; Region 2:  SHARED_WORK:  Base = 0x027ff000, Size = 4KB,   I:NC NB    / D:NC NB,     I:NA / D:RW
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

        /* (0) I/O レジスタ及び VRAM 等 */
        SET_PROTECTION_A(c0, HW_IOREG, 64MB)
        SET_PROTECTION_B(c0, HW_IOREG, 64MB)

        /* (4) D-TCM */
        ldr             r0, =SDK_AUTOLOAD_DTCM_START
        orr             r0, r0, #HW_C6_PR_16KB | HW_C6_PR_ENABLE
        SET_PROTECTION_B(c4, SDK_AUTOLOAD_DTCM_START, 16KB)

        /* (5) I-TCM */
        SET_PROTECTION_A(c5, HW_ITCM_IMAGE, 16MB)
        SET_PROTECTION_B(c5, HW_ITCM_IMAGE, 16MB)

        /* (6) システムコール ROM */
        SET_PROTECTION_A(c6, HW_BIOS, 32KB)
        SET_PROTECTION_B(c6, HW_BIOS, 32KB)

        /* TWL ハードウェア上で動作しているかどうかを調査 */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        beq             @002

@001:   /* ハードウェアが TWL の場合 */
        /* (1) メインメモリ及び WRAM */
        SET_PROTECTION_A(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)
        SET_PROTECTION_B(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)

        /* (2) ARM7 専用メインメモリ空間 */
        SET_PROTECTION_A(c2, HW_TWL_MAIN_MEM_SUB, 512KB)
        SET_PROTECTION_B(c2, HW_TWL_MAIN_MEM_SUB, 512KB)

        /* (3) 拡張メインメモリ */
        SET_PROTECTION_A(c3, HW_TWL_MAIN_MEM_EX, 16MB)
        SET_PROTECTION_B(c3, HW_TWL_MAIN_MEM_EX, 16MB)

        /* (7) ARM9/ARM7 共有メインメモリ空間 */
        SET_PROTECTION_A(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)
        SET_PROTECTION_B(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)

        /* 命令キャッシュ許可 */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 1

        /* データキャッシュ許可 */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 0

        /* ライトバッファ許可 */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 0, 0)
        mcr             p15, 0, r0, c3, c0, 0

        /* 命令アクセス許可 */
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA)
        mcr             p15, 0, r0, c5, c0, 3

        /* データアクセス許可 */
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, RW, RW, RO, RW)
        mcr             p15, 0, r0, c5, c0, 2

        b               @003

@002:   /* ハードウェアが NITRO の場合 */
        /* (1) メインメモリ */
		//SET_PROTECTION_A(c1, HW_MAIN_MEM_MAIN, 8MB)
		//SET_PROTECTION_B(c1, HW_MAIN_MEM_MAIN, 8MB)
        SET_PROTECTION_A(c1, HW_MAIN_MEM_MAIN, 32MB)
        SET_PROTECTION_B(c1, HW_MAIN_MEM_MAIN, 32MB)
        /* Size will be arranged in OS_InitArena(). */

        /* (2) ARM7 専用メインメモリ空間 */
		//SET_PROTECTION_A(c2, (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE - HW_MAIN_MEM_SUB_SIZE), 128KB)
		//SET_PROTECTION_B(c2, (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE - HW_MAIN_MEM_SUB_SIZE), 128KB)
        SET_PROTECTION_A(c2, (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE), 4KB)
        SET_PROTECTION_B(c2, (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE), 4KB)
        /* Base address will be moved in OS_InitArena(). */

        /* (3) カートリッジ */
		//SET_PROTECTION_A(c3, HW_CTRDG_ROM, 128MB)
		//SET_PROTECTION_B(c3, HW_CTRDG_ROM, 128MB)
        SET_PROTECTION_A(c3, HW_CTRDG_ROM, 32MB)
        SET_PROTECTION_B(c3, HW_CTRDG_ROM, 32MB)

        /* (7) ARM9/ARM7 共有メインメモリ空間 */
        SET_PROTECTION_A(c7, HW_MAIN_MEM_SHARED, 4KB)
        SET_PROTECTION_B(c7, HW_MAIN_MEM_SHARED, 4KB)

        /* 命令キャッシュ許可 */
		//mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0)
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 1

        /* データキャッシュ許可 */
		//mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0)
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 0

        /* ライトバッファ許可 */
		//mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 0, 0)
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 0, 0)
        mcr             p15, 0, r0, c3, c0, 0

        /* 命令アクセス許可 */
		//ldr             r0, =REGION_ACC(RW, RW, NA, NA, NA, RW, RO, NA)
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA)
        mcr             p15, 0, r0, c5, c0, 3

        /* データアクセス許可 */
        ldr             r0, =REGION_ACC(RW, RW, RW, RW, RW, RW, RO, RW)
        mcr             p15, 0, r0, c5, c0, 2

@003:   /* プロテクションユニット及びキャッシュ使用許可設定 */
        mrc             p15, 0, r0, c1, c0, 0
        ldr             r1, = HW_C1_ICACHE_ENABLE       \
                            | HW_C1_DCACHE_ENABLE       \
                            | HW_C1_CACHE_ROUND_ROBIN   \
                            | HW_C1_PROTECT_UNIT_ENABLE
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        /* キャッシュの内容を破棄 */
        mov             r1, #0
        mcr             p15, 0, r1, c7, c6, 0
        mcr             p15, 0, r1, c7, c5, 0

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_DoAutoload
  Description:  リンク情報に沿って、各オートロードブロックの固定データ部の展開
                及び変数部の 0 クリアを行う。4M bytes を越える PSRAM メモリ空間
                に配置されるオートロードブロックの展開は、ハードウェアが TWL で
                ある場合にだけ行う。オートロード元データとオートロード先が一部
                重なる場合もあるので、後方から展開を行う。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
/*
 * < 二段階オートロード >
 * 0x02000000 に Static セグメント及び一段目ロード元バイナリが(必要に応じて後半が圧縮されて)配置されている。
 *  圧縮されている場合は、まず 0x02000000 に後方から上書きしつつ解凍する。
 *  NITRO と共有可能な ITCM 上に配置されるべきバイナリデータを 0x01ff8000 にロードする。
 *  NITRO と共有可能な DTCM 上に配置されるべきバイナリデータを 0x02fe0000 にロードする。
 * 0x02400000 に二段目ロード元バイナリが(必要に応じて全て圧縮されて)配置されている。
 *  0x04000 バイト分はカード ROM から再読み出し不可なので、0x02f80000 - 0x02f84000 に退避する。
 *  圧縮されている場合は、まず 0x02400000 に後方から上書きしつつ解凍する。
 *  TWL でしか動作しない WRAM 上に配置されるべきバイナリデータをそれぞれ指定アドレスにロードする。
 *  TWL でしか動作しないメインメモリ上に配置されるべきバイナリデータを前方からコピーすることでロードする。
 *  これは、NITRO と共有可能なメインメモリ上に配置されるデータが 0x02400000 を越えないはずであるため、
 *  配置すべきアドレスは 0x02400000 より小さいアドレスになるはずである為。
 *  また、オートロード情報リストの実体がメインメモリへのオートロードブロックの .bss セクションのクリアの過程で
 *  破壊される可能性があるが、一連のオートロード処理の最後の段階なので、破壊されても問題ない。
 */
static asm void
INITi_DoAutoload(void)
{
@000:
        stmdb           sp!, {lr}
        /* NITRO 共用ブロックの解凍 */
        ldr             r1, =_start_ModuleParams
        ldr             r0, [r1, #20]       // r0 = bottom of compressed data
        bl              MIi_UncompressBackward

@010:
        /* NITRO 共用ブロックをオートロード */
        ldr             r1, =_start_ModuleParams
        ldr             r12, [r1]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_AUTOLOAD_START
@011:   cmp             r12, r0
        bge             @020
        /* 固定セクションをロード */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        stmdb           sp!, {r2}
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
        mov             r0, #0              // r0 = number to fill .bss section
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@013:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @013
        /* キャッシュを調整 */
        ldmia           sp!, {r2}           // r2 = start address of destination range
        mov             r0, #HW_ITCM_IMAGE
        cmp             r2, r0
        movge           r0, #HW_ITCM_END
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
        /* TWL ハードウェア上で動作しているかどうかを調査 */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        beq             @030

        /* TWL 専用ブロックの存在を確認 */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1cc  /* ARM9 用拡張常駐モジュール ROM サイズ */
        ldr             r0, [r1]
        cmp             r0, #0
        beq             @030

        /* 再読み出し不可部分を退避 */
        bl              INITi_ShelterLtdBinary

        /* TWL 専用ブロックの解凍 */
        ldr             r1, =_start_LtdModuleParams
        ldr             r0, [r1, #12]
        bl              MIi_UncompressBackward

        /* TWL 専用ブロックをオートロード */
        ldr             r1, =_start_LtdModuleParams
        ldr             r12, [r1]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_LTDAUTOLOAD_START
@021:   cmp             r12, r0
        bge             @030
        /* 固定セクションをロード */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        stmdb           sp!, {r2}
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@022:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @022
        /* static initializer テーブル情報を読み出し */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           sp!, {r0-r3, r12}
#endif
        /* .bss セクションを 0 クリア */
        mov             r0, #0              // r0 = number to fill .bss section
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@023:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @023
        /* キャッシュを調整 */
        ldmia           sp!, {r2}           // r2 = start address of destination range
        mov             r0, #HW_ITCM_IMAGE
        cmp             r2, r0
        movge           r0, #HW_ITCM_END
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

@030:   /* ライトバッファが空になるのを待つ */
        mov             r0, #0
        mcr             p15, 0, r0, c7, c10, 4

        /* オートロード完了コールバック関数呼び出し */
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
        /* ARM7 専用メインメモリ空間保護リージョンを一旦アクセス可能に変更 */
        mrc             p15, 0, r0, c5, c0, 3
        mrc             p15, 0, r1, c5, c0, 2
        stmdb           sp!, {r0, r1}
        bic             r0, r0, #(0xf << 8)
        orr             r0, r0, #(0x1 << 8)
        bic             r1, r1, #(0xf << 8)
        orr             r1, r1, #(0x1 << 8)
        mcr             p15, 0, r0, c5, c0, 3
        mcr             p15, 0, r1, c5, c0, 2

        /* 退避元・先アドレスを調査 */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1c8  /* ARM9 用拡張常駐モジュール RAM アドレス */
        ldr             r1, [r1]
        ldr             r3, =HW_TWL_ROM_HEADER_BUF + 0x038  /* ARM7 用常駐モジュール RAM アドレス */
        ldr             r3, [r3]
        sub             r3, r3, #0x4000                     /* 再読み出し不可領域サイズ */ /* ARM7 用退避エリア */
        sub             r2, r3, #0x4000                     /* 再読み出し不可領域サイズ */ /* ARM9 用退避エリア */

        /* コピー */
@loop:  ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @loop

        /* ARM7 専用メインメモリ空間保護リージョン設定を元に戻す */
        ldmia           sp!, {r0, r1}
        mcr             p15, 0, r0, c5, c0, 3
        mcr             p15, 0, r1, c5, c0, 2
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterStaticInitializer
  Description:  各オートロードセグメント内の static initializer へのポインタ
                テーブルを IRQ スタックの最上部 (から 4 バイトずらした位置)
                に退避する。
  Arguments:    ptr     -   セグメント内のポインタテーブルへのポインタ。
                            テーブルは NULL で終端されている必要がある。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
#ifndef SDK_NOINIT
static asm void
INITi_ShelterStaticInitializer(u32* ptr)
{
        /* 引数確認 */
        cmp             r0, #0
        bxeq            lr

        /* 退避場所先頭アドレスを計算 */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE
        sub             r1, r1, #HW_DTCM_SYSRV_SIZE
        sub             r1, r1, #HW_SVC_STACK_SIZE
        ldr             r2, =SDK_IRQ_STACKSIZE
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
                オートロード処理によって IRQ スタックの最上部 (から 4 バイト
                ずらした位置) に退避されている関数ポインタテーブルを一つずつ
                呼び出す。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static asm void
INITi_CallStaticInitializers(void)
{
        stmdb           sp!, {lr}

        /* テーブル退避場所先頭アドレスを計算 */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE
        sub             r1, r1, #HW_DTCM_SYSRV_SIZE
        sub             r1, r1, #HW_SVC_STACK_SIZE
        ldr             r2, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* テーブルに管理されているポインタを一つずつ呼び出し */
@001:   ldr             r0, [r1]
        cmp             r0, #0
        beq             @002
        stmdb           sp!, {r1}
        blx             r0
        ldmia           sp!, {r1}
        /* 一旦呼び出したポインタはゼロクリア (IRQスタックを間借りしている為) */
        mov             r0, #0
        str             r0, [r1], #4
        b               @001

@002:
        ldmia           sp!, {lr}
        bx              lr
}
#endif

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
  Name:         NitroStartUp
  Description:  Hook for user start up.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void
NitroStartUp(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         OSi_ReferSymbol
  Description:  Used by SDK_REFER_SYMBOL macro to avoid dead-strip.
  Arguments:    symbol  -   unused.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
OSi_ReferSymbol(void* symbol)
{
#pragma unused(symbol)
}

#include    <twl/codereset.h>
