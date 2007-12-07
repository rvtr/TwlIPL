/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - init - ARM7.TWL
  File:     crt0.LTD.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-28#$
  $Rev: 2690 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#include    <nitro/types.h>
#include    <nitro/hw/common/armArch.h>
#include	<nitro/os/common/emulator.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap_sp.h>
#include    <twl/hw/ARM7/mmap_wramEnv.h>
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
static void INITi_SetHMACSHA1ToAppParam(void);
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

#ifdef	DEBUG_WRAM_SETTING
u32 const wramMapping[12] = {
	0x8d898581, 0x8c888480, 0x9c989490, 0x8c888480,
	0x9c989490, 0x00000000, 0x09403900, 0x09803940,
	0x080037c0, 0x09403900, 0x09803940, 0x0000000f
};
#endif

#define	DIGEST_HASH_BLOCK_SIZE_SHA1					(512/8)
static const u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = 
{
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24,
};

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

        /* 専用 WRAM 上のコードで ARM9 とハンドシェイク1 */
        ldr             r0, =HW_BOOT_SHAKEHAND_9
        ldr             r1, =HW_BOOT_SHAKEHAND_7
        ldr             r2, =HW_PRV_WRAM
        mov             lr, pc
        bx              r2

		// ロードされたアプリのダイジェストを計算してアプリ間パラメータに格納
        bl              INITi_SetHMACSHA1ToAppParam
        
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
        
        /* TWL ハードウェア上で動作しているかどうかを調査 */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        /* TWL ハードウェア上で動作していない場合は永久ループ */
@012:   beq             @012

        /* ダウンロードパラメータ情報を共有エリアに退避 */
        ldr             r1, =HW_DOWNLOAD_PARAMETER
        ldr             r2, =TWLIPL_PARAM_DOWNLOAD_PARAMETER
        add             r3, r2, #HW_DOWNLOAD_PARAMETER_SIZE
@013:   ldr             r0, [r2], #4
        str             r0, [r1], #4
        cmp             r2, r3
        blt             @013

        /* 旧無線の送受信機能を動作させる設定を行う */
        ldr             r1, =REG_WL_ADDR
        ldrh            r0, [r1]
        orr             r0, r0, #REG_SCFG_WL_OFFB_MASK
        strh            r0, [r1]
        /* ARM7 側の DMA には新 DMA 回路を採用し、サウンドにも新 DMA 回路を利用 */
        ldr             r1, =REG_EXT_ADDR
        ldr             r0, [r1]
        orr             r0, r0, #REG_SCFG_EXT_DMAC_MASK
        orr             r0, r0, #REG_SCFG_EXT_DSEL_MASK
        str             r0, [r1]
        /* [TODO] ARM7 側でしか設定できない追加 I/O レジスタの初期設定を行う */
        

        /* Autoload を実施 */
        bl              INITi_DoAutoload
        
@020:
        /* STATIC ブロックの .bss セクションを 0 クリア */
        ldr             r0, =_start_ModuleParams
        ldr             r1, [r0, #12]       // SDK_STATIC_BSS_START
        ldr             r2, [r0, #16]       // SDK_STATIC_BSS_END
        mov             r0, #0
@021:   cmp             r1, r2
        strlt           r0, [r1], #4
        blt             @021

        //---- detect main memory size
        bl              detect_main_memory_size
		
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

#include <nitro/mi/stream.h>
#include <twl/os/common/systemCall.h>
#include <nitro/mi.h>
/*---------------------------------------------------------------------------*
  Name:         INITi_SetHMACSHA1ToAppParam
  Description:  ROMがロードされた各アプリ領域のHMACSHA1を計算し、特定のアドレ
                スに保存
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
#define UNDEF_CODE			0xe7ffdeff
#define ENCRYPT_DEF_SIZE	0x800
#define DGT_TGT_ADDR		( HW_MAIN_MEM + 0x0200 )

static void INITi_SetHMACSHA1ToAppParam(void)
{
	u32 *arm9_flx_addr = (u32 *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028));
	u32 *p_arm9encryObjVerify = (u32 *)(DGT_TGT_ADDR + 4 * 32);
	int l;
	SVCHMACSHA1Context *pCon = ( SVCHMACSHA1Context * ) 0x037c0000;
	
	// arm9_flx
	*p_arm9encryObjVerify = TRUE;
	for( l=0; l<ENCRYPT_DEF_SIZE/4; l++ )
	{
		if(arm9_flx_addr[l] != UNDEF_CODE)
		{
			if((u32)p_arm9encryObjVerify < 0x2000400)
			{
				*p_arm9encryObjVerify = arm9_flx_addr[l];
				p_arm9encryObjVerify++;
			}
		}
	}
	MI_CpuClear8( (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028)), ENCRYPT_DEF_SIZE);// 折角MI使えるので、4バイト境界で困らないように8で
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x02c)));
	SVC_HMACSHA1GetHash(pCon, (void *)DGT_TGT_ADDR);
	// arm7_flx
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x038)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x03c)));
	SVC_HMACSHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 32));
	// arm9_ltd
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1c8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1cc)));
	SVC_HMACSHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 2 * 32));
	// arm7_ltd
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1d8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1dc)));
	SVC_HMACSHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 3 * 32));

/*
	SVCSHA1Context *pCon = ( SVCSHA1Context * ) 0x037c0000;
	// arm9_flx
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)( (*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028)) + 0x800 ), ( *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x02c)) - 0x800 ) );
	SVC_SHA1GetHash(pCon, (void *)DGT_TGT_ADDR);
	// arm7_flx
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x038)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x03c)));
	SVC_SHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 32));
	// arm9_ltd
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1c8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1cc)));
	SVC_SHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 64));
	// arm7_ltd
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1d8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1dc)));
	SVC_SHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 96));
*/
}
/*
static asm void INITi_SetHMACSHA1ToAppParam(void)
{
        mov             r0, #HW_MAIN_MEM
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        ldr				r1, =s_digestDefaultKey
        mov				r2, #(DIGEST_HASH_BLOCK_SIZE_SHA1)
        bl				SVC_HMACSHA1Init
        
        mov             r1, #HW_WRAM_AREA
        sub             r3, r1, 0x2000
        ldr				r1, [r3, 0x028]
        ldr				r2, [r3, 0x02c]
        bl				SVC_HMACSHA1Update
        
        mov             r1, #HW_MAIN_MEM
        bl				SVC_HMACSHA1GetHash
        
        bx				lr
}
*/


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
        ldmia           ap!, {r0-r3, r12}
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
//################ temp: this process will be done in IPL
	    // SCFG enable?
	    ldr     r2, =REG_EXT_ADDR
		ldr     r0, [r2]
		tst     r0, #0x80000000
		beq     @9

        ldr     r2, =HW_PRV_WRAM_SYSRV
		//OPT(bonding option)
        ldr     r3, =REG_OP_ADDR
        ldrh    r0, [r3]
		strh    r0, [r2, #8]
		//OPT(JTAG info)
		ldr  	r3, =REG_JTAG_ADDR
		ldrb	r0, [r3]
		//CLK(only wram clock)
		ldr     r3, =REG_CLK_ADDR
		ldrh    r1, [r3]
		and     r1, r1, #0x80
		orr     r0, r0, r1, LSR 1
		strb	r0, [r2, #9]
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
		tst		r12, #HWi_WSYS09_CLK_WRAMHCLK_MASK
        moveq   r0, #OS_CONSOLE_SIZE_8MB
		beq		@4

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
  Name:         NitroSpStartUp
  Description:  Hook for user start up.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void
NitroSpStartUp(void)
{
}

#include    <twl/codereset.h>
