/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     boot.c

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

#include <twl.h>
#include <sysmenu/boot/common/boot.h>
#include <sysmenu/mmap.h>
//#include "loader.h"
//#include "mb_child.h"

// define data-------------------------------------------------------

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void Venner_R1( void );
static void ClearMemory( void );
static void ClearBankregAndStack( void );
static void CpuClear32Byte( void );

#ifdef ISDBG_MB_CHILD_
// モニタAPI番号
typedef enum { MONAPI_ONLOADCHILD = 0 } APITYPE;
static u32 _isdcallmon( APITYPE nIndex, void *pParam );
static void	_ISDbgLib_OnLoadChildBinary( void );
#endif // ISDBG_MB_CHILD_

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------

#include <nitro/code32.h>

//-----------------------------------------------------------------------
// メモリクリア
//-----------------------------------------------------------------------
static asm void ClearMemory( void )
{
		mov			r11, lr
#if 0
		ldr			r0, = 0x02280000								// SYSMENU-ARM7 MMEMのクリア
		ldr			r1, = 0x02380000
		bl			CpuClear32Byte
		
//		ldr			r0, = 0x02800000								// SYSMENU-ARM9 MMEMのクリア
//		ldr			r1, = 0x02e80000
//		bl			CpuClear32Byte
		
		ldr			r0, = HW_WRAM_A_LTD								// ARM7-WRAMのクリア( LTDのマッピング )
		ldr			r1, = BOOTCORE_ARM7_ADDR
		bl			CpuClear32Byte
#endif
		bx			r11
}


//-----------------------------------------------------------------------
// バンクレジスタ リセット ＆ スタック領域 クリア
//-----------------------------------------------------------------------
static asm void ClearBankREG_Stack( void )
{
		mov			r12, lr
		
#ifndef ISDBG_MB_CHILD_
		mov			r0, #0xc0 | HW_PSR_SVC_MODE		// SVCモードへ切り換え  & IRQ/FIQ不許可
		msr			cpsr_cxsf, r0
		ldr			sp, =HW_PRV_WRAM_SVC_STACK_END	// SP のセット
		mov			lr,	#0
		msr			spsr_csxf, lr
		
		mov			r0, #0xc0 | HW_PSR_IRQ_MODE		// IRQモードへ切り換え  & IRQ/FIQ不許可
		msr			cpsr_cxsf, r0
		ldr			r2, =HW_PRV_WRAM_IRQ_STACK_END
		mov			sp, r2							// SP のセット
		mov			lr,	#0
		msr			spsr_cxsf, lr
		
		mov			r0, #0xc0 | HW_PSR_SYS_MODE		// システムモードへ切り換え & IRQ/FIQ不許可
		msr			cpsr_cxsf, r0
		mov			r1, #0x200						// SDK_IRQ_STACKSIZE （実際はリンカスクリプト内で定義している。）
		sub			r1, r2, r1
		mov			sp, r1							// SP のセット
		
		ldr			r2, = HW_PRV_WRAM_END
		sub			r0, r2, #0x600
		mov			r1, r2
		bl			CpuClear32Byte
#else  // ISDBG_MB_CHILD_
		ldr			r2, = HW_PRV_WRAM_END
		sub			r0, r2, #0x600
		sub			r1, r2, #HW_PRV_WRAM_SYSRV_SIZE
		bl			CpuClear32Byte
		add			r0, r0, #0x20
		mov			r1, r2
		bl			CpuClear32Byte
#endif // ISDBG_MB_CHILD_
		
		sub			r0, r2, #( HW_PRV_WRAM_END - RETURN_FROM_MAIN_ARM7_FUNCP )
		ldr			r1, = ClearMemory
		bl			CpuClear32Byte
		
		ldr			r2, = 0x027ff000
		mov			r0, r2
		add			r1, r2, #0x800
		bl			CpuClear32Byte
		
		add			r0, r2, #0xda0					// HW_ARENA_INFO_BUF (アリーナ情報構造体)　※デバッガモニタ用ハンドラはクリアしない。
		add			r1, r0, #0x60
		bl			CpuClear32Byte
		
		add			r0, r2, #0xf80
		add			r1, r0, #0x80
		bl			CpuClear32Byte
		
		bx			r12
}


//-----------------------------------------------------------------------
// ブートコア
//-----------------------------------------------------------------------
asm void BOOT_Core( void )
{
		//---------------------------------------
		// ARM9との同期をとる（subp_stateを2にしてから、mainp_stateが2になるのを待つ。）
		//---------------------------------------
		ldr			r1, =REG_MAINPINTF_ADDR
		mov			r0, #0x0200
		strh		r0, [r1]						// メインプロセッサインターフェースレジスタ
@0		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0002
		bne			@0
		
		//---------------------------------------
		// メモリクリア
		//---------------------------------------
		bl			ClearMemory
		
		//---------------------------------------
		// ARM9との同期をとる（subp_stateを3にしてから、mainp_stateが3になるのを待つ。）
		//---------------------------------------
		ldr			r1, =REG_MAINPINTF_ADDR
		mov			r0, #0x0300
		strh		r0, [r1]						// メインプロセッサインターフェースレジスタ
@1		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0003
		bne			@1
		
		//---------------------------------------
		// ARM7 WRAMバンク設定 MBK6-MBK9			※ARM7がMBK9でWRAMロックをする前にARM9でWRAM設定を済ませておく必要がある。
		//---------------------------------------
		ldr		r0, =HW_TWL_ROM_HEADER_BUF
        add     r10, r0, #0x1a0     // rom_header->s.sub_wram_config_data
        // r10- => r9-r2
        ldr     r9, =REG_MBK6_ADDR
        add     r2, r9, #15
@2      ldr     r3, [r10], #4
        str     r3, [r9], #4
        cmp     r9, r2
        blt     @2
		
		//---------------------------------------
		// 無線マルチブート用ローダー起動
		//---------------------------------------
//		ldr			r1, =LOADER_Start
//		bl			Venner_R1
		
		//---------------------------------------
		// デバッガ動作フラグ格納
		//---------------------------------------
#ifdef __IS_DEBUGGER_BUILD
		ldr			r3, =HW_MAIN_MEM_EX_END
		sub			r0, r3, #0x400
		ldrh		r11, [r0, #0x14]				// r11 <- GetMovedInfoFromIPL1Addr()->isOnDebugger
#endif
		//---------------------------------------
		// ARM9との同期をとる（subp_stateを1にしてから、mainp_stateが1になるのを待つ。）
		//---------------------------------------
		ldr			r1, =REG_MAINPINTF_ADDR
		mov			r0, #0x0100
		strh		r0, [r1]						// メインプロセッサインターフェースレジスタ
@3		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0001
		bne			@3
		
		//---------------------------------------
		// ISデバッガでのDL子機プログラムのデバッグ情報設定
		//---------------------------------------
#ifdef ISDBG_MB_CHILD_
		mov			r0, #MONAPI_ONLOADCHILD
		mov			r1, #0
		bl			_isdcallmon
#endif // ISDBG_MB_CHILD_
		
		//---------------------------------------
		// バンクレジスタ＆スタッククリア
		//---------------------------------------
		bl			ClearBankREG_Stack
		
		//---------------------------------------
		// ARM9との最終同期をとる(subp_stateを0にして、mainp_stateが0になるのを待つ。）
		//---------------------------------------
		ldr			r1, =REG_MAINPINTF_ADDR
		mov			r0, #0
		strh		r0, [r1]						// メインプロセッサインターフェースレジスタ
@4		ldrh		r0, [r1]
		cmp			r0, #0x0001
		beq			@4
		
		//---------------------------------------
		// R11の値をもとにブートアドレス取得
		//---------------------------------------
		ldr			r3, =HW_MAIN_MEM_EX_END			// ゲーム・エントリポイント 獲得
		ldr			r12, [r3, #-(0x200 - 0x34)]		// rmhp->arm7->entryAddr
		mov			lr, r12
		
#ifdef __IS_DEBUGGER_BUILD
		cmp			r11, #1							// if (!GetMovedInfoFromIPL1Addr()->isOnDebugger)
		ldreq		r12, [r3, #-(0x200 - 0x16c)]	// デバッガ動作なら、デバッガ・エントリポイント 獲得
#endif
		//---------------------------------------
		// 汎用レジスタクリア
		//---------------------------------------
		mov			r11, #HW_REG_BASE
		ldmdb		r11, {r0-r10}
		mov			r11, #0
		
		//---------------------------------------
		// ゲームブート
		//---------------------------------------
		bx			r12
}

static asm void Venner_R1( void )
{
		bx		r1
}


// 32byte単位のメモリクリア  r0 = tgtp, r1 = lastAddr
static asm void CpuClear32Byte( void )
{
		mov			r3, #0
		mov			r4, r3
		mov			r5, r3
		mov			r6, r3
		mov			r7, r3
		mov			r8, r3
		mov			r9, r3
		mov			r10, r3
@0		cmp			r0, r1							// クリア終了
		stmltia		r0!, {r3-r10}
		blt			@0
		bx  	    lr
}




#ifdef ISDBG_MB_CHILD_
///////////////////////////////////////////////////////////////////////
//  Project:  モニタの呼び出し - ISデバッグライブラリ内コード相当の実装
//  Copyright (C)2007 INTELIGENT SYSTEMS Co.,Ltd.
///////////////////////////////////////////////////////////////////////

#ifdef SDK_ARM9
// ARM9の場合定義
#define	NITRO9
#endif	// SDK_ARM9

///////////////////////////////////////////////////////////////////////

#define	MON_ENTRY_SIGNATURE_OFFSET				0x00010
#define	MON_INTAPI_FUNC_OFFSET					0x002A0
#define	MON_INTAPI_FUNC_SIGN					0x4346564f		// OVFC
#define	MON_INTAPI_OFFSET_VERSION				0x04
#define	MON_INTAPI_OFFSET_COMMONENTRY_A9		0x20
#define	MON_INTAPI_OFFSET_COMMONENTRY_A7		0x24

#define	PERM_MASK								0x0F
#define	PERM_SHIFT_REGION7						28
#define	PERM_SYS_RW_USER_RW						0x03

///////////////////////////////////////////////////////////////////////

// メモリ保護関連
typedef struct tagMemoryProtection {
	OSIntrMode				modeIntr;
#ifdef	NITRO9
	u32						nCP15R7State;
	u32						nInstProtection;
	u32						nDataProtection;
#endif	// NITRO9
} MemoryProtection;

///////////////////////////////////////////////////////////////////////

static asm OSIntrMode OSm_DisableInterrupts_IrqAndFiq( void )
{
    mrs     r0, cpsr
    orr     r1, r0, #HW_PSR_IRQ_FIQ_DISABLE
    msr     cpsr_c, r1
    and     r0, r0, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}
static asm OSIntrMode OSm_RestoreInterrupts_IrqAndFiq( register OSIntrMode state )
{
    mrs     r1, cpsr
    bic     r2, r1, #HW_PSR_IRQ_FIQ_DISABLE
    orr     r2, r2, r0
    msr     cpsr_c, r2
    and     r0, r1, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}
#ifdef	NITRO9
static asm u32  OSm_SetIPermissionsForProtectionRegionEx( u32 nVal )
{
    mcr     p15, 0, r0, c5, c0, 3
    bx      lr
}

static asm u32  OSm_SetDPermissionsForProtectionRegionEx( u32 nVal )
{
    mcr     p15, 0, r0, c5, c0, 2
    bx      lr
}
static asm void OSm_SetProtectionRegion7( u32 param )
{
    mcr     p15, 0, r0, c6, c7, 0
    bx      lr
}
static asm u32  OSm_GetIPermissionsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c5, c0, 3
    bx      lr
}
static asm u32  OSm_GetDPermissionsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c5, c0, 2
    bx      lr
}
static asm u32  OSm_GetProtectionRegion7( void )
{
    mrc     p15, 0, r0, c6, c7, 0
    bx      lr
}
#endif	// NITRO9

///////////////////////////////////////////////////////////////////////
// プロテクトを解除します
///////////////////////////////////////////////////////////////////////
#ifdef	NITRO9
static void	_isdcallmon_unlockProtect( MemoryProtection *pParam, u32 dwRegion7 )
{
	// 割り込みを強制的に止める処理
	pParam->modeIntr	= OSm_DisableInterrupts_IrqAndFiq();

	// プロテクションユニットのリージョン7の設定を保存
	pParam->nCP15R7State = OSm_GetProtectionRegion7();
	pParam->nInstProtection = OSm_GetIPermissionsForProtectionRegion();
	pParam->nDataProtection = OSm_GetDPermissionsForProtectionRegion();

	// リージョン7にオーバーレイ転送情報領域からメインメモリ終了域のプロテクト設定を行う
	if( dwRegion7 ) OSm_SetProtectionRegion7( dwRegion7 ) ;

	// ユーザー・システム下で書き込めるように修正
	OSm_SetIPermissionsForProtectionRegionEx( (pParam->nInstProtection & (~(PERM_MASK << PERM_SHIFT_REGION7))) | ( PERM_SYS_RW_USER_RW << PERM_SHIFT_REGION7) );
	OSm_SetDPermissionsForProtectionRegionEx( (pParam->nDataProtection & (~(PERM_MASK << PERM_SHIFT_REGION7))) | ( PERM_SYS_RW_USER_RW << PERM_SHIFT_REGION7) );
}
		// NITRO9
#else
		// NITRO7
static void	_isdcallmon_unlockProtect( MemoryProtection *pParam )
{
	// 割り込みを強制的に止める処理
	pParam->modeIntr	= OSm_DisableInterrupts_IrqAndFiq();
}
#endif	// NITRO7


///////////////////////////////////////////////////////////////////////
// プロテクト状態を元に戻します
///////////////////////////////////////////////////////////////////////
static void	_isdcallmon_restoreProtect( const MemoryProtection *pParam )
{
#ifdef	NITRO9
	// ユーザー・システムの保護状態を設定
	OSm_SetDPermissionsForProtectionRegionEx( pParam->nDataProtection );
	OSm_SetIPermissionsForProtectionRegionEx( pParam->nInstProtection );

	// プロテクションユニットのリージョン7の設定を元に戻す
	OSm_SetProtectionRegion7( pParam->nCP15R7State );
#endif	// NITRO9

	// 割り込みを戻す処理
	(void)OSm_RestoreInterrupts_IrqAndFiq( pParam->modeIntr );
}


////////////////////////////////////////////////////////////////////////
// モニタコールバック
//
//  ・呼び出す前に_ISDbgLib_Initializeを行なっておく必要はありません。
//  ・子機バイナリダウンロード後に呼び出してください。
//  ・loadrun等のISNITRO.dllを用いたダウンロード実行環境や
//    v1.75以前の古いデバッガ環境では何もせずに戻ります。
//
////////////////////////////////////////////////////////////////////////
static u32		_isdcallmon( APITYPE nIndex, void *pParam )
{
	MemoryProtection	prot;
	u32					nMonitorEntry;
	u32					nResult = (u32)-1;

	// モニタエントリのチェック
	nMonitorEntry = *(vu32*)0x027FFF68;
	if ((nMonitorEntry >= HW_MAIN_MEM) && (nMonitorEntry < (HW_MAIN_MEM+0x00800000))) {

#ifdef	NITRO9
		u32		dwRegion7;

		// Unlockする領域の切り分け
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00780000) ) {
			dwRegion7 = 0x02780000 | HW_C6_PR_512KB | HW_C6_PR_ENABLE;
		} else
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00700000) ) {
			dwRegion7 = 0x02700000 | HW_C6_PR_1MB | HW_C6_PR_ENABLE;
		} else
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00600000) ) {
			dwRegion7 = 0x02600000 | HW_C6_PR_2MB | HW_C6_PR_ENABLE;
		} else
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00400000) ) {
			dwRegion7 = 0x02400000 | HW_C6_PR_4MB | HW_C6_PR_ENABLE;
		} else {
			dwRegion7 = 0x00000000;
		} 

		_isdcallmon_unlockProtect( &prot, dwRegion7 );
#else
		_isdcallmon_unlockProtect( &prot );
#endif

		// シグネチャの確認とジャンプ
		{
			u32	nSignature = nMonitorEntry + MON_ENTRY_SIGNATURE_OFFSET;
			u32 nOVFC = nMonitorEntry + MON_INTAPI_FUNC_OFFSET;

			if(    *(u64 *)nSignature == (u64)0x47494c4c45544e49LL // INTELLIG
				&& *(u32 *)nOVFC == MON_INTAPI_FUNC_SIGN 
				&& *(u32 *)(nOVFC + MON_INTAPI_OFFSET_VERSION) >= 2
			){
				// 通知関数のアドレス診断
				u32		nAddr;
#ifdef	NITRO9
				nAddr = *(u32 *)(nOVFC + MON_INTAPI_OFFSET_COMMONENTRY_A9);
#else
				nAddr = *(u32 *)(nOVFC + MON_INTAPI_OFFSET_COMMONENTRY_A7);
#endif
				if ((nAddr >= HW_MAIN_MEM) && (nAddr < (HW_MAIN_MEM+0x00800000))) {
					// 通知関数へジャンプする
					// (bx or blxでブランチしているか確認すること)
					nResult = (*(u32(*)( u32, void*))nAddr)( nIndex, pParam );
				}
			}
		}

		// メモリ保護の復元
		_isdcallmon_restoreProtect( &prot );
	}

	return nResult;
}

#if 0
// 子機バイナリダウンロード後のコールバック処理
static void	_ISDbgLib_OnLoadChildBinary( void )
{
	(void)_isdcallmon( MONAPI_ONLOADCHILD, NULL );
}
#endif // 0

#endif // ISDBG_MB_CHILD_


#include <nitro/codereset.h>		// ここまで。

