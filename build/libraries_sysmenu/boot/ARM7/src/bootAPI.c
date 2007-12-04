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
#include <sysmenu.h>
#include "reboot.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#ifdef	ISDBG_MB_CHILD_
#define PRE_CLEAR_NUM_MAX		18
#else
#define PRE_CLEAR_NUM_MAX		14
#endif

#define COPY_NUM_MAX			12
#define POST_CLEAR_NUM_MAX		8

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
		reg_MI_MBK9 = 0;								// 全WRAMのロック解除
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9に対してブートするようIRQで要求＋ARM7のステートを１にする。
		
		// SDK共通リブート
		{
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			// メモリリストの設定
			static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
			{
                // pre clear
				SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
				SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
				SYSM_OWN_ARM7_WRAM_ADDR, SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR,
				SYSM_OWN_ARM7_WRAM_ADDR, SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR,
#ifdef	ISDBG_MB_CHILD_
				HW_PRV_WRAM_END - 0x600, (HW_PRV_WRAM_END - HW_PRV_WRAM_SYSRV_SIZE) - (HW_PRV_WRAM_END - 0x600),
				HW_PRV_WRAM_END - 0x600 + 0x20, HW_PRV_WRAM_END - (HW_PRV_WRAM_END - 0x600 + 0x20),
#endif
				HW_MAIN_MEM_SHARED, HW_RED_RESERVED - HW_MAIN_MEM_SHARED,
				HW_ARENA_INFO_BUF, HW_ROM_HEADER_BUF - HW_ARENA_INFO_BUF,
				HW_PXI_SIGNAL_PARAM_ARM9, HW_MAIN_MEM_SYSTEM_END - HW_PXI_SIGNAL_PARAM_ARM9,
				NULL,
                // copy forward
				NULL,
                // copy backward
				NULL,
                // post clear
				NULL,
			};
			
			// copy forwardリスト設定
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL && !SYSMi_GetWork()->romRelocateInfo[l].rev )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].src;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].dest;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].length;
				}
			}
			mem_list[list_count++] = NULL;
			
			// copy backwardリスト設定
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL && SYSMi_GetWork()->romRelocateInfo[l].rev )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].src;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].dest;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].length;
				}
			}
			mem_list[list_count++] = NULL;
			
			// post clearリスト設定
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].post_clear_addr != NULL )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].post_clear_addr;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].post_clear_length;
				}
			}
			mem_list[list_count] = NULL;

			// リブート
			OS_Boot( (void *)*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x34), mem_list, REBOOT_TARGET_TWL_SECURE_SYSTEM );
		}
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	int i ;
	
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {						// DMAの停止
		MI_StopDma( (u16)i );
	}
	
	if( SYSMi_GetWork()->isCardBoot ) {
#ifdef DEBUG_USED_CARD_SLOT_B_
		reg_MI_MC_SWP = 0x80;											// カードスロットのスワップ
#endif
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = SYSMi_GetWork()->nCardID;		// カード抜けチェックバッファにカードIDをセット
		*(u32 *)HW_RED_RESERVED = SYSMi_GetWork()->nCardID;
	}
	
	// レジスタのクリア
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x0b0), (0x13c - 0x0b0) );
																		// DMA0SAD  ～ RCNT1
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x400), 0x104 );	// SG0CNT_L ～ SGMCNT
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x508), 0x14 );		// SGRVCNT  ～ SGRV1CLEN
	reg_GX_DISPSTAT			= 0;
	reg_SPI_SPICNT			= 0;
	reg_PXI_MAINP_FIFO_CNT	= 0x4008;
	
	*(vu32 *)HW_RESET_PARAMETER_BUF = 0;								// リセットバッファをクリア
	
	// クリアしていないレジスタは、VCOUNT, JOY, PIFCNT, MC-, EXMEMCNT, IME, PAUSE, POWLCDCNT, 他セキュリティ系です。
	(void)OS_ResetRequestIrqMask((u16)~0);
}
