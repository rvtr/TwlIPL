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
#include <twl/mcu.h>
#include <twl/cdc.h>
#include <symbols.h>
#include <twl/rtfs.h>
#include <sysmenu.h>
#include "reboot.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#ifdef	ISDBG_MB_CHILD_
#define PRE_CLEAR_NUM_MAX		(6*2)
#else
#define PRE_CLEAR_NUM_MAX		(4*2)
#endif

#define COPY_NUM_MAX			(4*3)
#define POST_CLEAR_NUM_MAX		(4*2)

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
	char drv;

	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
		
		// unmount drives
		for ( drv = 'A'; drv <= 'Z'; drv++ )
		{
			rtfs_detach( drv );
		}
		
		(void)OS_DisableIrq();							// ここで割り込み禁止にしないとダメ。
		(void)OS_SetIrqMask(0);							// SDKバージョンのサーチに時間がかかると、ARM9がHALTにかかってしまい、ARM7のサウンドスレッドがARM9にFIFOでデータ送信しようとしてもFIFOが一杯で送信できない状態で無限ループに入ってしまう。
		(void)OS_SetIrqMaskEx(0);
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
			REBOOTTarget target = REBOOT_TARGET_TWL_SECURE_SYSTEM;
			ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL拡張ROMヘッダ（DSアプリには無い）
			ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			// メモリリストの設定
			static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
			{
				// pre clear
				SYSM_OWN_ARM7_WRAM_ADDR, NULL, // SYSM_OWN_ARM7_WRAM_ADDR（SDK_AUTOLOAD_WRAM_START）はリンカから与えられる
				SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
				SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
#ifdef	ISDBG_MB_CHILD_
				HW_PRV_WRAM_END - 0x600, (HW_PRV_WRAM_END - HW_PRV_WRAM_SYSRV_SIZE) - (HW_PRV_WRAM_END - 0x600),
				HW_PRV_WRAM_END - 0x600 + 0x20, HW_PRV_WRAM_END - (HW_PRV_WRAM_END - 0x600 + 0x20),
#endif
				HW_MAIN_MEM_SHARED, HW_RED_RESERVED - HW_MAIN_MEM_SHARED,
				NULL,
				// copy forward
				NULL,
				// copy backward
				NULL,
				// post clear
				NULL,
			};
			mem_list[1] = SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR;
			
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
			
			// アプリケーション選択
			if ( dh->s.platform_code )
			{
//				target = REBOOT_TARGET_TWL_APP;
			}
			else
			{
				target = REBOOT_TARGET_DS_APP;
				MCU_GoDsMode();
				CDC_GoDsMode();
				// DSサウンド：DSP = 8:0
				// 32KHz
				reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
								  REG_SND_SMX_CNT_E_MASK;
			}
			
			// リブート
			OS_Boot( dh->s.sub_entry_address, mem_list, target );
		}
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	if( SYSMi_GetWork()->isCardBoot ) {
#ifdef DEBUG_USED_CARD_SLOT_B_
		reg_MI_MC_SWP = 0x80;											// カードスロットのスワップ
#endif
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = SYSMi_GetWork()->nCardID;		// カード抜けチェックバッファにカードIDをセット
	}
	
	*(vu32 *)HW_RESET_PARAMETER_BUF = 0;								// リセットバッファをクリア
	
	// レジスタクリアは基本的に OS_Boot で行う
	
	// クリアしていないレジスタは、VCOUNT, JOY, PIFCNT, MC-, EXMEMCNT, IME, PAUSE, POWLCDCNT, 他セキュリティ系です。
	(void)OS_ResetRequestIrqMask((u32)~0);
	(void)OS_ResetRequestIrqMaskEx((u32)~0);
}
