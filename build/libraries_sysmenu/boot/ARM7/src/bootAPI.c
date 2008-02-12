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
#define POST_CLEAR_NUM_MAX		(12 + 4*2)

#define SYSMi_ARM9_BOOT_CODE_BUF	0x023fee00

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static u32 twl_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_PARAM_RESERVED_END, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, SYSMi_ARM9_BOOT_CODE_BUF,
	SYSMi_ARM9_BOOT_CODE_BUF + OS_BOOT_CODE_SIZE, SYSM_OWN_ARM9_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR_END, HW_TWL_MAIN_MEM_SHARED,
	NULL,
};

static u32 nitro_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_END,
	HW_PARAM_RESERVED_END, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, SYSMi_ARM9_BOOT_CODE_BUF,
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_DBG_NTR_SYSTEM_BUF,
	SYSM_OWN_ARM9_MMEM_ADDR_END, HW_TWL_MAIN_MEM_SHARED,
	NULL,
};

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
			REBOOTTarget target = REBOOT_TARGET_TWL_SYSTEM;
            BOOL ds = FALSE;
			ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL拡張ROMヘッダ（DSアプリには無い）
			ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			u32 *post_clear_list;
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
			if ( dh->s.platform_code )
			{
				post_clear_list = twl_post_clear_list;
				BOOTi_CutAwayRegionList( post_clear_list, (u32)th->s.main_ltd_ram_address, (u32)th->s.main_ltd_ram_address + th->s.main_ltd_size);
				BOOTi_CutAwayRegionList( post_clear_list, (u32)th->s.sub_ltd_ram_address, (u32)th->s.sub_ltd_ram_address + th->s.sub_ltd_size);
			}else
			{
				post_clear_list = nitro_post_clear_list;
			}
			BOOTi_CutAwayRegionList( post_clear_list, (u32)dh->s.main_ram_address, (u32)dh->s.main_ram_address + dh->s.main_size);
			BOOTi_CutAwayRegionList( post_clear_list, (u32)dh->s.sub_ram_address, (u32)dh->s.sub_ram_address + dh->s.sub_size);
			for( l=0; post_clear_list[l]!=NULL ; l+=2 )
			{
				mem_list[list_count++] = post_clear_list[l];
				mem_list[list_count++] = post_clear_list[l+1] - post_clear_list[l];
			}
			mem_list[list_count] = NULL;
			
			// サウンド停止
			SND_Disable();
			
			// アプリケーション選択
			if ( dh->s.platform_code )
			{
//				target = REBOOT_TARGET_TWL_APP;
#ifdef SYSMENU_DISABLE_TWL_BOOT
                while (1)
                {
                }
#endif // SYSMENU_DISABLE_TWL_BOOT
			}
			else
			{
				target = REBOOT_TARGET_DS_APP;
			}
			
            if ( target == REBOOT_TARGET_DS_APP || target == REBOOT_TARGET_DS_WIFI )
            {
                ds = TRUE;
            }

            if ( ds || th->s.codec_mode == OS_CODECMODE_NITRO )
            {
				// I2S停止（MCLKは動作継続）
				reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;
				CDC_GoDsMode();
				// DSサウンド：DSP = 8:0
				// 32KHz
				reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
								  REG_SND_SMX_CNT_E_MASK;
            }

#if defined(FIRM_USE_TWLSDK_KEYS) || defined(SYSMENU_DISABLE_RETAIL_BOOT)
            // TwlSDK内の鍵を使っている時は製品用CPUではTWLアプリはブートしない
            if ( ! (*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK) && !ds )
            {
                OS_Terminate();
            }
#endif // FIRM_USE_SDK_KEYS || SYSMENU_DISABLE_RETAIL_BOOT

			// リブート
			OS_Boot( dh->s.sub_entry_address, mem_list, target );
		}
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	if( SYSMi_GetWork()->flags.common.isCardBoot ) {
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

// 単純リスト要素削除
static void BOOTi_DeliteElementFromList( u32 *list, u32 index )
{
	int l;
	for( l=(int)index; list[l]!=NULL; l++ )
	{
		list[l] = list[l+1];
	}
}

// 単純リスト要素追加
static void BOOTi_InsertElementToList( u32 *list, u32 index, u32 value )
{
	int l = (int)index;
	while(list[l]!=NULL)
	{
		l++;
	}
	list[l+1] = NULL;
	for( ; index<l; l-- )
	{
		list[l] = list[l-1];
	}
	list[l] = value;
}

// {first1, last1, first2, last2, ... , NULL}という形式の領域リストから
// {start, end}の領域を切り取ったリストを返す関数
// 引数に与えるリストは要素が最大2追加されるため、十分な大きさが必要
// また、領域リストの要素は、最後尾のNULL以外昇順に並んでいる必要がある。
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end )
{
	int l, m, n;
	if( end <= start ) return;
	for( l=0; regionlist[l]!=NULL; l++ )
	{
		if( regionlist[l] >= start )
		{
			break;
		}
	}
	for( m=l; regionlist[m]!=NULL; m++ )
	{
		if( regionlist[m] > end )
		{
			break;
		}
	}
	// この時点でregionlist[l]およびregionlist[m]は、start <= regionlist[l], end < regionlist[m]で、且つ最も小さな値
	
	if( m % 2 == 1 )
	{
		BOOTi_InsertElementToList( regionlist, (u32)m, end );
		// endをリストに追加した場合、mは追加した要素を指すように
	}
	if( l % 2 == 1 )
	{
		BOOTi_InsertElementToList( regionlist, (u32)l, start );
		m++;
		// startをリストに追加した場合、mは1増える
		l++;
		// startをリストに追加した場合、lは追加した要素の次の要素を指すように
	}
	
	// regionlist[l]からregionlist[m-1]までの要素を消す
	for( n=l; l<m; l++ )
	{
		BOOTi_DeliteElementFromList( regionlist, (u32)n );
	}
}
