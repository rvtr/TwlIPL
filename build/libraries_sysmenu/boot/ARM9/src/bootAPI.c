/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     bootAPI.c

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
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>
#include <firm/format/wram_regs.h>
#include "reboot.h"


// define data-------------------------------------------------------
#define SUBP_RECV_IF_ENABLE     0x4000

#define C1_DTCM_ENABLE          0x00010000      // データＴＣＭ イネーブル
#define C1_EXCEPT_VEC_UPPER     0x00002000      // 例外ベクタ 上位アドレス（こちらに設定して下さい）
#define C1_SB1_BITSET           0x00000078      // レジスタ１用１固定ビット列（後期アボートモデル、DATA32構成シグナル制御、PROG32構成シグナル制御、ライトバッファイネーブル）

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------
static void BOOTi_ClearREG_RAM( void );
static void BOOTi_StartBOOT( void );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------
void BOOT_Init( void )
{
    reg_PXI_SUBPINTF = 0x0000;
}

static void ie_subphandler( void )
{
    OS_TPrintf( "INTR SUBP!!\n" );
    OS_SetIrqCheckFlag( OS_IE_SUBP );
}

// ブート準備をして、ARM7からの通知を待つ。
void BOOT_Ready( void )
{
    int i;

    // エントリアドレスの正当性をチェックし、無効な場合は無限ループに入る。
//  SYSMi_CheckEntryAddress();

    for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {                // DMAの停止
        MI_StopDma( (u16)i );
    }

//  FinalizeCardPulledOut();                                // カード抜け検出終了処理
    BOOTi_ClearREG_RAM();                                   // レジスタ＆RAMクリア
    (void)GX_VBlankIntr( FALSE );
    (void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
    OS_EnableInterrupts();
    (void)OS_SetIrqMask( OS_IE_SUBP );                      // サブプロセッサ割り込みのみを許可。
    reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;        // ARM9ステートを "0x0f" に
                                                            // ※もうFIFOはクリア済みなので、使わない。
    // ARM7からの通知待ち
    OS_WaitIrq( 1, OS_IE_SUBP );

    OS_TPrintf( "INTR SUBP passed!!\n" );
    // 割り込みをクリアして最終ブートシーケンスへ。
    reg_PXI_SUBPINTF &= 0x0f00;                             // サブプロセッサ割り込み許可フラグをクリア
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u16)~0 );

    // WRAMの配置
    {
        ROM_Header_Short *pROMH = (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
        MIHeader_WramRegs *pWRAMREGS = (MIHeader_WramRegs *)pROMH->main_wram_config_data;
        reg_GX_VRAMCNT_C    = pWRAMREGS->main_vrambnk_c;
        reg_GX_VRAMCNT_D    = pWRAMREGS->main_vrambnk_d;
        reg_GX_VRAMCNT_WRAM = pWRAMREGS->main_wrambnk_01;
    }
	
	// ROMヘッダバッファをコピー
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	
    // SDK共通リブート
	{
	    // メモリリストの設定
	    static u32 mem_list[] =
	    {
            // pre clear
	        HW_ITCM, HW_ITCM_SIZE,
	        //HW_DTCM, HW_DTCM_SIZE,
	        NULL,
            // copy forward
	        NULL,
            // copy backward
	        NULL,
            // post clear
	        NULL,
	    };
	    
	    // [TODO]再配置リストの作成と設定（ほぼARM7側でやるのでこちらは空）
	    static u32 relocate_list[] =
	    {
			NULL
		};
	    
		REBOOTTarget target = REBOOT_TARGET_TWL_SECURE_SYSTEM;
		ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
		
		// アプリケーション選択
		if ( dh->s.platform_code )
		{
//			target = REBOOT_TARGET_TWL_APP;
		}
		else
		{
			target = REBOOT_TARGET_DS_APP;
		}
		
		// 起動するターゲットの種類を指定する必要あり
		OS_Boot( dh->s.main_entry_address, mem_list, target );
	}
}


// 使用したレジスタ＆メモリのクリア
static void BOOTi_ClearREG_RAM( void )
{
    // 最後がサブプロセッサ割り込み待ちなので、IMEはクリアしない。
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u16)~0 );

	// レジスタクリアは基本的に OS_Boot で行う
}

