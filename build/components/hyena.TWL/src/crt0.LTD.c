/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - init - ARM7.TWL
  File:     crt0.LTD.c

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

#include    <nitro/types.h>
#include    <nitro/hw/common/armArch.h>
#include	<nitro/os/common/emulator.h>
#include	<nitro/os/common/printf.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap_sp.h>
#include    <twl/hw/common/mmap_wramEnv.h>
#include    <twl/os/common/format_rom.h>
#include    "boot_sync.h"

/*---------------------------------------------------------------------------*/
void    _start(void);
void    _start_AutoloadDoneCallback(void* argv[]);
void    INIT_InitializeScfg(void);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

/* 外部関数参照定義 */
extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);

/* 内部関数プロトタイプ定義 */
static void INITi_DoAutoload(void);
static BOOL INITi_IsRunOnTwl( void );
static void INITi_CopySysConfig( void );
static void INITi_DetectMainMemorySize(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32* ptr);
static void INITi_CallStaticInitializers(void);
#endif
static void* INITi_Copy32(void* dst, void* src, u32 size);
static void* INITi_Fill32(void* dst, u32 value, u32 size);

/* リンカスクリプトにより定義されるシンボル参照 */
extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);

#ifdef USE_HYENA_COMPONENT
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
#else
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
#endif

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


#ifdef	DEBUG_WRAM_SETTING
u32 const wramMapping[12] = {
	0x8d898581, 0x8c888480, 0x9c989490, 0x8c888480,
	0x9c989490, 0x00000000, 0x09403900, 0x09803940,
	0x080037c0, 0x09403900, 0x09803940, 0x0000000f
};
#endif

/* [TODO] 
 * 現状 TWL のマルチブートローダーがどこに情報を残してくれるか未定のため、
 * ビルドを通すためだけのでたらめな定義です。*/
#define TWLIPL_PARAM_DOWNLOAD_PARAMETER     0x02ffb000

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

#ifdef	DEBUG_WRAM_SETTING
/*---------------------------------------------------------------------------*
  Name:         WaitAgreement
  Description:  ARM9 の起動ベクタが特定の状態になるまで待つ。
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
#endif

/*---------------------------------------------------------------------------*
  Name:         GotoMain
  Description:  ARM9 に特定の状態になったことを伝えた上で、Main 関数へジャンプ
                する。メインメモリでないメモリ空間で実行される必要がある。
  Arguments:    r0  -   Main 関数のポインタ。Main 関数が Thumb コードである場合
                        には、bx 命令でジャンプするため最下位ビットが 1 になって
                        いる必要がある。
                r1  -   同期用フェーズ管理変数へのポインタ。
                r2  -   ARM9 に伝えるフェーズ番号。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static const u32    microcode_GotoMain[2]   =
{
    0xe1c120b0, /*      strh    r2, [r1]    ; フェーズ管理変数を更新 */
    0xe12fff10  /*      bx  r0              ; Main 関数へジャンプ */
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

        /* ハンドシェイク用マイクロコードを専用 WRAM にコピー */
        ldr             r1, =microcode_ShakeHand
        ldr             r2, =HW_PRV_WRAM
        add             r3, r2, #40
@001:   ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @001

        /* 専用 WRAM 上のコードで ARM9 とハンドシェイク */
        ldr             r0, =HW_BOOT_SHAKEHAND_9
        ldr             r1, =HW_BOOT_SHAKEHAND_7
        ldr             r2, =HW_PRV_WRAM
        mov             lr, pc
        bx              r2

@010:
        /* スタックポインタ設定 */
        mov             r0, #HW_PSR_SVC_MODE        // SuperVisor mode
        msr             cpsr_c, r0
        ldr             sp, =HW_PRV_WRAM_SVC_STACK_END
        mov             r0, #HW_PSR_IRQ_MODE        // IRQ mode
        msr             cpsr_c, r0
        ldr             sp, =HW_PRV_WRAM_IRQ_STACK_END
        ldr             r1, =SDK_IRQ_STACKSIZE
        sub             r1, sp, r1
        sub             sp, sp, #4                  // 4 bytes for stack check code
        mov             r0, #HW_PSR_SYS_MODE
        msr             cpsr_csfx, r0
        sub             sp, r1, #4                  // 4 bytes for stack check code

        /* スタック領域をクリア */
        ldr             r0, =SDK_SYS_STACKSIZE
        sub             r1, r1, r0
        ldr             r2, =HW_PRV_WRAM_IRQ_STACK_END
        mov             r0, #0
@011:   cmp             r1, r2
        strlt           r0, [r1], #4
        blt             @011

        /* SCFG を HW_SYS_CONF_BUF へコピー */
        bl              INIT_InitializeScfg
        bl              INITi_CopySysConfig

        /* TWL ハードウェア上で動作しているかどうかを調査 */
        bl              INITi_IsRunOnTwl
        /* TWL ハードウェア上で動作していない場合は永久ループ */
@012:   bne             @012

        /* ダウンロードパラメータ情報を共有エリアに退避 */
        ldr             r0, =HW_DOWNLOAD_PARAMETER              // r0 = start address of destination range
        ldr             r1, =TWLIPL_PARAM_DOWNLOAD_PARAMETER    // r1 = start address of source range
        mov             r2, #HW_DOWNLOAD_PARAMETER_SIZE         // r2 = size of download parameter
        bl              INITi_Copy32

        /* プリロードパラメータアドレスが格納されていない場合は、デフォルト値をセット */
		ldr				r0, =HW_PRELOAD_PARAMETER_ADDR
		ldr				r1, [r0]
		cmp				r1, #0
		ldreq			r1, =HW_PARAM_TWL_SETTINGS_DATA_DEFAULT
		streq			r1, [r0]

@020:
        /* Autoload を実施 */
        bl              INITi_DoAutoload

        /* STATIC ブロックの .bss セクションを 0 クリア */
        mov             r1, #0          // r1 = clear value for STATIC bss section
        ldr             r3, =_start_ModuleParams
        ldr             r0, [r3, #12]   // r0 = start address of STATIC bss section
        ldr             r2, [r3, #16]
        subs            r2, r2, r0      // r2 = size of STATIC bss section
        blgt            INITi_Fill32

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

        /* 割り込みベクタ設定 */
        ldr             r1, =HW_INTR_VECTOR_BUF
        ldr             r0, =OS_IrqHandler
        str             r0, [r1]

#ifndef SDK_NOINIT
        /* c++ 用初期化 */
        bl              _fp_init
        bl              TwlSpStartUp
        bl              __call_static_initializers
        bl              INITi_CallStaticInitializers
#endif

@030:
        /* Main 関数へのジャンプ用マイクロコードをスタックの底にコピー */
        ldr             r1, =microcode_GotoMain
        ldr             r0, [r1], #4
        str             r0, [sp, #-4]
        ldr             r0, [r1]
        str             r0, [sp]

        /* マイクロコードを経由して Main 関数へジャンプ */
        ldr             r0, =TwlSpMain
        ldr             r1, =HW_BOOT_SYNC_PHASE
        mov             r2, #BOOT_SYNC_PHASE_4
        ldr             lr, =HW_RESET_VECTOR
        sub             r3, sp, #4
        bx              r3
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
 * 0x02380000 に crt0 及び一段目ロード元バイナリが配置されている。
 *  NITRO と共有可能な WRAM 上に配置されるべきバイナリデータを 0x037c0000 にロードする。
 *  TWL でしか動作しない WRAM 上に配置されるべきバイナリデータを続きのアドレスにロードする。
 * 0x02400000 に ARM9 の二段目ロード元バイナリが配置されている。
 *  0x04000 バイト分はカード ROM から再読み出し不可なので、0x02f80000 - 0x02f84000 に退避する。
 * 0x02e80000 に二段目ロード元バイナリが配置されている。
 *  0x04000 バイト分はカード ROM から再読み出し不可なので、0x02f84000 - 0x02f88000 に退避する。
 *  NITRO と共有可能な MAIN 上に配置されるべきバイナリデータを 0x02f88000 にロードする。
 *  TWL でしか動作しない MAIN 上に配置されるべきバイナリデータを続きのアドレスにロードする。
 */
static asm void
INITi_DoAutoload(void)
{
        stmfd           sp!, {lr}

@01_autoload:
        /* WRAM 用ブロックをオートロード */
        ldr             r0, =_start_ModuleParams
        ldr             r12, [r0]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r3, [r0, #4]        // r3 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r0, #8]        // r1 = SDK_AUTOLOAD_START
@01_segments_loop:
        cmp             r12, r3
        bge             @02_start
        /* 固定セクション */
        ldr             r0, [r12], #4       // r0 = start address of destination range of fixed section
        ldr             r2, [r12], #4       // r2 = size of fixed section
        bl              INITi_Copy32
        stmfd           sp!, {r0, r1}
        /* static initializer テーブル */
        ldr             r0, [r12], #4       // r0 = start address of source range of static initializers table
#ifndef SDK_NOINIT
        stmfd           sp!, {r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmfd           sp!, {r3, r12}
#endif
        /* bss セクション */
        ldmfd           sp!, {r0}           // r0 = start address of destination range of bss section
        mov             r1, #0              // r1 = clear value for bss section
        ldr             r2, [r12], #4       // r2 = size of bss section
        bl              INITi_Fill32
        ldmfd           sp!, {r1}           // r1 = end address of source range of the autoload segment
        b               @01_segments_loop

@02_start:
		
        /* メインメモリ用オートロードブロックの存在を確認 */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF
        ldr             r0, [r1, #ROM_Header.s.sub_ltd_size]
        cmp             r0, #0
        beq             @03_start

@02_autoload:
        /* メインメモリ用ブロックをオートロード */
        ldr             r0, =_start_LtdModuleParams
        ldr             r12, [r0]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r3, [r0, #4]        // r3 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r0, #8]        // r1 = SDK_AUTOLOAD_START
@02_segments_loop:
        cmp             r12, r3
        bge             @03_start
        /* 固定セクション */
        ldr             r0, [r12], #4
        ldr             r2, [r12], #4
        bl              INITi_Copy32
        stmfd           sp!, {r0, r1}
        /* static initializer テーブル */
        ldr             r0, [r12], #4
#ifndef SDK_NOINIT
        stmfd           sp!, {r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmfd           sp!, {r3, r12}
#endif
        /* bss セクション */
        ldmfd           sp!, {r0}
        mov             r1, #0
        ldr             r2, [r12], #4
        bl              INITi_Fill32
        ldmfd           sp!, {r1}
        b               @02_segments_loop

@03_start:
        /* オートロード完了コールバック関数呼び出し */
        ldr             r0, =_start_ModuleParams
        ldr             r1, =_start_LtdModuleParams
        ldmfd           sp!, {lr}
        b               _start_AutoloadDoneCallback
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
        ldr             r1, =HW_PRV_WRAM_IRQ_STACK_END
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
        ldr             r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr             r2, =SDK_IRQ_STACKSIZE
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
  Name:         NitroSpStartUp
  Description:  Hook for user start up.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void
NitroSpStartUp(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         INITi_IsRunOnTwl

  Description:  check running platform

                This function is used in only Nitro-TWL hybrid mode.
                (In Nitro mode and TWL limited mode, treated as constant)

  Arguments:    None

  Returns:      TRUE  : running on TWL
                FALSE : running on NITRO
 *---------------------------------------------------------------------------*/
static asm BOOL INITi_IsRunOnTwl( void )
{
		ldr     r0, =HW_PRV_WRAM_SYSRV
		ldrb    r0, [r0,#HWi_WSYS08_WRAMOFFSET]
		and		r0, r0, #HWi_WSYS08_ROM_ARM9SEC_MASK | HWi_WSYS08_ROM_ARM9RSEL_MASK
		cmp		r0, #HWi_WSYS08_ROM_ARM9SEC_MASK
		moveq	r0, #TRUE
		movne	r0, #FALSE

		bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         INIT_InitializeScfg

  Description:  ローダーが行う SCFG ブロックへの設定処理を代行する為の関数。
                処理の実体は crt0 の外部に定義するため、弱シンボルとする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
INIT_InitializeScfg(void)
{
		bx			lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CopySysConfig

  Description:  copy SCFG registers to HW_SYS_CONF_BUF and HW_PRV_WRAM_SYSRV

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void INITi_CopySysConfig( void )
{
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

#define OSi_CHECKNUM1         0x55
#define OSi_CHECKNUM2         0xaa
#define OSi_100usWAIT         3352  // 100us = 3351.4cycles(33.514MHz)

static asm void INITi_DetectMainMemorySize( void )
{
        stmfd   sp!, {lr}

        //---- detect memory size
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
        b       @3

        //---- 8MB or 16MB or 32MB
@2:
        // check if running on twl/nitro
        bl      INITi_IsRunOnTwl
        movne   r0, #OS_CONSOLE_SIZE_8MB
        bne     @3

        //---- 16MB or 32MB
        add     r3, r2, #OSi_IMAGE_DIFFERENCE2
        mov     r0, #OS_CONSOLE_SIZE_16MB
        ldrb    r2, [r3]

        mov     r1, #OSi_CHECKNUM1
        strb    r1, [r3]

		ldr     r12, =OSi_100usWAIT
@4		subs    r12, r12, #4
        bcs     @4

        ldrb    r1, [r3]
        cmp     r1, #OSi_CHECKNUM1
        bne     @3

        mov     r1, #OSi_CHECKNUM2
        strb    r1, [r3]

		ldr     r12, =OSi_100usWAIT
@5		subs    r12, r12, #4
        bcs     @5

        ldrb    r1, [r3]
        cmp     r1, #OSi_CHECKNUM2
        moveq   r0, #OS_CONSOLE_SIZE_32MB

@3:
        //---- check SMX_CNT
		strb    r2, [r3]
        ldr     r3, =REG_SMX_CNT_ADDR
        ldrh    r1, [r3]
        and     r1, r1, #OSi_DETECT_NITRO_MASK
        cmp     r1, #OSi_DETECT_NITRO_VAL
        orreq   r0, r0, #OS_CHIPTYPE_SMX_MASK

        ldr     r2, =HW_MMEMCHECKER_SUB
        strh    r0, [r2]

        ldmfd   sp!, {lr}
        bx      lr
}

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
        stmfd           sp!, {r3-r11}
        bics            r3, r2, #0x0000001f
        beq             @next
        add             r3, r0, r3
@loop:
        ldmia           r1!, {r4-r11}
        stmia           r0!, {r4-r11}
        cmp             r3, r0
        bgt             @loop
@next:
        tst             r2, #0x00000010
        ldmneia         r1!, {r4-r7}
        stmneia         r0!, {r4-r7}
        tst             r2, #0x00000008
        ldmneia         r1!, {r4-r5}
        stmneia         r0!, {r4-r5}
        tst             r2, #0x00000004
        ldmneia         r1!, {r4}
        stmneia         r0!, {r4}
        ldmfd           sp!, {r3-r11}
        bx              lr
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
        stmfd           sp!, {r3-r11}
        mov             r4, r1
        mov             r5, r1
        mov             r6, r1
        mov             r7, r1
        mov             r8, r1
        mov             r9, r1
        mov             r10, r1
        mov             r11, r1
        bics            r3, r2, #0x0000001f
        beq             @next
        add             r3, r0, r3
@loop:
        stmia           r0!, {r4-r11}
        cmp             r3, r0
        bgt             @loop
@next:
        tst             r2, #0x00000010
        stmneia         r0!, {r4-r7}
        tst             r2, #0x00000008
        stmneia         r0!, {r4-r5}
        tst             r2, #0x00000004
        stmneia         r0!, {r4}
        ldmfd           sp!, {r3-r11}
        bx              lr
}

#include    <twl/codereset.h>
