/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     gameBoot.c

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
#include <sysmenu/memorymap.h>

// define data-------------------------------------------------------
#define C1_DTCM_ENABLE          0x00010000		// データＴＣＭ イネーブル
#define C1_EXCEPT_VEC_UPPER     0x00002000		// 例外ベクタ 上位アドレス（こちらに設定して下さい）
#define C1_SB1_BITSET           0x00000078		// レジスタ１用１固定ビット列（後期アボートモデル、DATA32構成シグナル制御、PROG32構成シグナル制御、ライトバッファイネーブル）

#define INITi_HW_DTCM   SDK_AUTOLOAD_DTCM_START

// extern data-------------------------------------------------------

// from LCF
extern  u32 SDK_IRQ_STACKSIZE[];

// function's prototype----------------------------------------------
void ReturnFromMain(void);
void ResetCP15(void);
void ClearBankregAndStack(void);
void CpuClear32Byte(void);
void BootFuncEnd(void);

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------


#include <twl/code32.h>		// このソースはデフォルトではARMでコンパイルされる。

//-----------------------------------------------------------------------
// メインルーチンからのリターン
//-----------------------------------------------------------------------

/*
	※ReturnFromMainをRamの後方にコピーする方法が、今はReturnFromMainアドレスから、
　　　(BootFuncEnd - ReturnFromMain)サイズ分だけコピーとしているが、
　　　この方法だと、コンパイラの最適化仕様で関数並びが変わってしまった時におかしくなる。
　　　何か他にいい方法はないか？
*/


asm void ReturnFromMain(void)
{
		//---------------------------------------
		// データキャッシュを全て無効に。（DC_InvalidateAllを抜き出して実装）
		//---------------------------------------
	    mov         r0, #0
    	mcr         p15, 0, r0, c7, c6, 0
		
		//---------------------------------------
		// ARM7との同期をとる（subp_stateが2になるのを待って、mainp_stateを2にする。）
		//---------------------------------------
		ldr			r1, =REG_SUBPINTF_ADDR
@0		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0002
		bne			@0
		mov			r0, #0x0200
		strh		r0, [r1]
		
		//---------------------------------------
		// ISデバッガ動作フラグの格納
		//---------------------------------------
#ifdef __IS_DEBUGGER_BUILD
		ldr			r3, =HW_MAIN_MEM_EX_END
		sub			r0, r3, #0x400
		ldrh		r11, [r0, #0x14]					// r11 =  GetMovedInfoFromIPL1Addr()->isOnDebugger
#endif
		//---------------------------------------
		// ARM7との同期をとる（subp_stateが1になるのを待って、mainp_stateを1にする。）
		//---------------------------------------
		ldr			r1, =REG_SUBPINTF_ADDR
@1		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0001
		bne			@1
		mov			r0, #0x0100
		strh		r0, [r1]
		
		//---------------------------------------
		// バンクレジスタ＆スタッククリア
		//---------------------------------------
		bl			ClearBankregAndStack
		
		//---------------------------------------
		// プロテクションユニットの解除
		//---------------------------------------
		bl			ResetCP15
		
		//---------------------------------------
		// ARM7との最終同期をとる(subp_stateが0になるのを待って、mainp_stateを0にする）
		//---------------------------------------
		ldr			r1, =REG_SUBPINTF_ADDR
@2		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0001
		beq			@2
		ldr			r3, =REG_VCOUNT_ADDR			// Vカウンタを全IPL2バージョンで同一値(=0)でアプリに引き渡すようにする。
@3		ldrh			r0, [r3]
		cmp			r0, #0
		bne			@3
//		mov			r0, #0						// R0に読んだVカウント値が"0"なので、これはいらない
		strh			r0, [r1]
		
		//---------------------------------------
		// R11の値をもとにブートアドレス取得
		//---------------------------------------
		ldr			r3, =HW_MAIN_MEM_EX_END			// ゲーム・エントリポイント 獲得
		ldr			r12, [r3, #-(0x200 - 0x24)]		// rmhp->arm9->entryAddr
		mov			lr, r12
		
#ifdef __IS_DEBUGGER_BUILD
		cmp			r11, #1							// if (!GetMovedInfoFromIPL1Addr()->isOnDebugger)
		ldreq		r12, [r3, #-(0x200 - 0x168)]	// デバッガ・エントリポイント 獲得
#endif
		
		//---------------------------------------
		// 汎用レジスタクリア
		//---------------------------------------
		ldr			r11, =INITi_HW_DTCM				// クリアしたDTCMからデータを読み出して、汎用レジスタをクリアする。
		ldmia		r11, {r0-r10}
		mov			r11, #0
		
		//---------------------------------------
		// ゲームブート
		//---------------------------------------
		bx			r12
}


//-----------------------------------------------------------------------
// システム制御コプロセッサ リセット
//-----------------------------------------------------------------------
asm void ResetCP15(void)
{
		// プロテクションユニット＆キャッシュ＆ITCM無効。DTCMは有効（スタックをクリアするため）
		ldr     	r0, = C1_DTCM_ENABLE  | C1_EXCEPT_VEC_UPPER | C1_SB1_BITSET
		mcr     	p15, 0, r0, c1, c0, 0
		
		// ITCMの割り当てを解除
		mov			r0, #0
		mcr			p15, 0, r0, c6, c5, 0
		
		// DTCMの割り当てを解除
//		mov			r0,#0
//		mcr			p15, 0, r0, c9, c1, 0
		
		// キャッシュ無効化
		mov     	r0, #0
		mcr     	p15, 0, r0, c7, c5, 0       	// 命令キャッシュ
		mcr     	p15, 0, r0, c7, c6, 0       	// データキャッシュ
		
		// ライトバッファ エンプティ待ち
		mcr			p15, 0, r0, c7, c10, 4
		
		bx			lr
}


//-----------------------------------------------------------------------
// バンクレジスタ リセット ＆ スタック領域 クリア
//-----------------------------------------------------------------------
asm void ClearBankregAndStack(void)
{
		mov			r12, lr
		
#ifndef IPL2_ONLYMULTIBOOT
		mov			r0, #0xc0 | HW_PSR_SVC_MODE		// SVCモードへ切り換え  & IRQ/FIQ不許可
		msr			cpsr_cxsf, r0
		ldr			r0, =INITi_HW_DTCM
		add			r0, r0, #0x3fc0
		mov			sp, r0							// SP のセット
		mov			lr,	#0
		msr			spsr_csxf, lr
		
		mov			r0, #0xc0 | HW_PSR_IRQ_MODE		// IRQモードへ切り換え  & IRQ/FIQ不許可
		msr			cpsr_cxsf, r0
		ldr			r0, =INITi_HW_DTCM
		add			r0, r0, #0x3fc0
		sub			r0, r0, #HW_SVC_STACK_SIZE
		mov			sp, r0							// SP のセット
		mov			lr,	#0
		msr			spsr_cxsf, lr
		
        ldr			r1, =SDK_IRQ_STACKSIZE
        sub			r1, r0, r1
		mov			r0, #0xc0 | HW_PSR_SYS_MODE		// システムモードへ切り換え & IRQ/FIQ不許可
		msr			cpsr_cxsf, r0
        sub			sp, r1, #4						// SP のセット & 4byte for stack check code
#endif // IPL2_ONLYMULTIBOOT
		
		ldr			r0, =HW_ITCM					// ITCMのクリア
		mov			r1, #HW_ITCM_SIZE
		bl			CpuClear32Byte
		
		ldr			r0, =INITi_HW_DTCM				// スタックを含めたDTCMのクリア
		mov			r1, #HW_DTCM_SIZE
		bl			CpuClear32Byte
		
		bx			r12
}


// 32byte単位のメモリクリア  r0 dstp,r1 byteSize
asm void CpuClear32Byte(void)
{
		add			r2, r0, r1						// 終了アドレスの算出
		mov			r1, r1, lsr #5					// サイズは32byte単位
		mov			r3, #0
		mov			r4, r3
		mov			r5, r3
		mov			r6, r3
		mov			r7, r3
		mov			r8, r3
		mov			r9, r3
		mov			r10, r3
@0		cmp			r0, r2							// クリア終了？
		stmltia		r0!, {r3-r10}
		blt			@0
		bx  	    lr
}


void BootFuncEnd(void)
{
}
#include <twl/codereset.h>		// ここまで。

