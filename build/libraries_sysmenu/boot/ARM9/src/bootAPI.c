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
#include <sysmenu/boot/common/boot.h>
#include <firm/format/wram_regs.h>
//#include <nitro/mb.h>
//#include "IPL2_work.h"
//#include "define.h"

// define data-------------------------------------------------------
#define SUBP_RECV_IF_ENABLE		0x4000

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
//	SYSMi_CheckEntryAddress();
	
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {				// DMAの停止
		MI_StopDma( (u16)i );
	}
	
//	FinalizeCardPulledOut();								// カード抜け検出終了処理
	BOOTi_ClearREG_RAM();									// レジスタ＆RAMクリア
	(void)GX_VBlankIntr( FALSE );
	(void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
	OS_EnableInterrupts();
	(void)OS_SetIrqMask( OS_IE_SUBP );						// サブプロセッサ割り込みのみを許可。
	reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;		// ARM9ステートを "0x0f" に
															// ※もうFIFOはクリア済みなので、使わない。
	// ARM7からの通知待ち
	OS_WaitIrq( 1, OS_IE_SUBP );
	
	OS_TPrintf( "INTR SUBP passed!!\n" );
	// 割り込みをクリアして最終ブートシーケンスへ。
	reg_PXI_SUBPINTF &= 0x0f00;								// サブプロセッサ割り込み許可フラグをクリア
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
	
	BOOT_Core();			// never return
}


// 使用したレジスタ＆メモリのクリア
static void BOOTi_ClearREG_RAM( void )
{
	// 最後がサブプロセッサ割り込み待ちなので、IMEはクリアしない。
	(void)OS_SetIrqMask( 0 );
	(void)OS_ResetRequestIrqMask( (u16)~0 );
	
	// メモリクリア
	GX_SetBankForLCDC( GX_VRAM_LCDC_ALL );							// VRAM     クリア
	MI_CpuClearFast( (void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
	(void)GX_DisableBankForLCDC();
//	MI_CpuClearFast( (void *)HW_ITCM,	HW_ITCM_SIZE );			// ITCM     クリア  ※ITCMにはSDKのコードが入っているので、BOOT_Coreでクリアする。
//	MI_CpuClearFast( (void *)HW_DTCM,	HW_DTCM_SIZE - 0x800 );	// DTCM     クリア	※DTCMはスタック&SDK変数入りなので、最後にBOOT_Coreでクリアする。
	MI_CpuClearFast( (void *)HW_OAM,	HW_OAM_SIZE );			// OAM      クリア
	MI_CpuClearFast( (void *)HW_PLTT,	HW_PLTT_SIZE );			// パレット クリア
	MI_CpuClearFast( (void *)HW_DB_OAM,	HW_DB_OAM_SIZE );		// OAM      クリア
	MI_CpuClearFast( (void *)HW_DB_PLTT,HW_DB_PLTT_SIZE );		// パレット クリア
	
	// レジスタクリア
	MI_CpuClearFast( (void*)( HW_REG_BASE + 0x8 ),    0x12c );	// BG0CNT    ～ KEYCNT
	MI_CpuClearFast( (void*)( HW_REG_BASE + 0x280 ),  0x40 );	// DIVCNT    ～ SQRTD3
	MI_CpuClearFast( (void*)( HW_REG_BASE + 0x1000 ), 0x6e );	// DISP1CNT1 ～ DISPBRTCNT1
	CP_SetDiv32_32( 0, 1 );
	reg_PXI_SUBP_FIFO_CNT = 0x4008;
	reg_GX_DISPCNT  = 0;
	reg_GX_DISPSTAT = 0;										// ※ reg_GX_VCOUNTはベタクリアできないので、この先頭部分のクリアを分離する。
	
	// クリアしていないレジスタは、VCOUNT, PIFCNT, MC-, EXMEMCNT, IME, RBKCNT1, PAUSE, POWLCDCNT, 全3D系。
}

