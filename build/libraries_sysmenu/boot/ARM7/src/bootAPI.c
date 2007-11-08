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
//#include "loader.h"
//#include "mb_child.h"

// define data-------------------------------------------------------
#define MAINP_SEND_IF		0xff00

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------

void BOOT_Init( void )
{
	reg_PXI_MAINPINTF = 0x0000;
}


BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
		(void)OS_DisableIrq();							// ここで割り込み禁止にしないとダメ。
		(void)OS_SetIrqMask(0);							// SDKバージョンのサーチに時間がかかると、ARM9がHALTにかかってしまい、ARM7のサウンドスレッドがARM9にFIFOでデータ送信しようとしてもFIFOが一杯で送信できない状態で無限ループに入ってしまう。
/*
#ifdef ISDBG_MB_CHILD_
		if( ( GetIpl2WorkAddr()->ipl2_type != 0xff ) && ( GetIpl2WorkAddr()->ipl2_type & 0x28 ) )
#endif // ISDBG_MB_CHILD_								// 	USG or NATなら無線パッチを当てる
		{
			InsertWLPatch();
		}
*/
		BOOTi_ClearREG_RAM();							// ARM7側のメモリ＆レジスタクリア。
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9に対してブートするようIRQで要求＋ARM7のステートを１にする。
		BOOT_Core();									// never return
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	int i ;
	
//	SVC_CpuClearFast(0x0000, IPL2_PRV_WRAM_ARENA_LO, RETURN_FROM_MAIN_ARM7_FUNCP - IPL2_PRV_WRAM_ARENA_LO );
																	// アリーナのクリア
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {						// DMAの停止
		MI_StopDma( (u16)i );
	}
	// レジスタのクリア
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x0b0), (0x13c - 0x0b0) );
																		// DMA0SAD  ～ RCNT1
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x400), 0x104 );	// SG0CNT_L ～ SGMCNT
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x508), 0x14 );		// SGRVCNT  ～ SGRV1CLEN
	reg_GX_DISPSTAT			= 0;
	reg_SPI_SPICNT			= 0;
	reg_PXI_MAINP_FIFO_CNT	= 0x4008;
	// クリアしていないレジスタは、VCOUNT, JOY, PIFCNT, MC-, EXMEMCNT, IME, PAUSE, POWLCDCNT, 他セキュリティ系です。
	(void)OS_ResetRequestIrqMask((u16)~0);
}
