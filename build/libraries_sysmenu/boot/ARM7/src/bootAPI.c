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
#include <twl/aes/ARM7/lo.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
#include <sysmenu/ds.h>
#include <firm/hw/ARM7/mmap_firm.h>
#include <firm/format/from_firm.h>
#include <firm/aes/ARM7/aes_init.h>
#include <firm/os/common/boot.h>
#include "reboot.h"
#include "internal_api.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#define PRE_CLEAR_NUM_MAX		(5*2)
#define COPY_NUM_MAX			(5*3)
#define POST_CLEAR_NUM_MAX		(12 + 6*2)

#define CLRLIST_OWN_ARM7_WRAM_SIZE_IDX		1
#define CLRLIST_REBOOT_STACK_PAD_SIZE_IDX	(2*3+1)

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static u32 twl_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_MAIN_MEM_PARAMETER_BUF_END, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, OS_BOOT_A9CODE_BUF,
	OS_BOOT_A9CODE_BUF + OS_BOOT_CODE_SIZE, SYSM_OWN_ARM9_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9例外ベクタ
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7例外ベクタ
	NULL,
};

static u32 nitro_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_MAIN_MEM_PARAMETER_BUF, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, OS_BOOT_A9CODE_BUF,
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_DBG_NTR_SYSTEM_BUF,
	SYSM_OWN_ARM9_MMEM_ADDR_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9例外ベクタ
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7例外ベクタ
	NULL,
};

// const data--------------------------------------------------------

// dev_commonKeyはベタで持っていい。
static const u8 dev_commonKey[] = {
	0xA1, 0x60, 0x4A, 0x6A, 0x71, 0x23, 0xB5, 0x29,
	0xAE, 0x8B, 0xEC, 0x32, 0xC8, 0x16, 0xFC, 0xAA
};

static const u8 dev_seedES[] = {
	0x2D, 0xD4, 0x03, 0x98, 0xA7, 0x6B, 0x03, 0x28,
	0xCE, 0x61, 0x04, 0xBB, 0x0A, 0xBB, 0x03, 0x5B,
};

static const u8 dev_seedNAM[] = {
	0x4D, 0x04, 0xA4, 0x7F, 0xE3, 0x02, 0x30, 0x2E,
	0x2A, 0x07, 0x06, 0xE6, 0xD9, 0x06, 0x47, 0x76,
};

static const u8 dev_seedSlotC[] = {
	0x3B, 0x06, 0x86, 0x57, 0x33, 0x04, 0x88, 0x11,
	0x49, 0x04, 0x6B, 0x33, 0x12, 0x02, 0xAC, 0xF3,
};


void BOOT_Init( void )
{
	reg_PXI_MAINPINTF = 0x0000;
}
		
BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
		// 最適化されるとポインタを初期化しただけでは何もコードは生成されません
		ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL拡張ROMヘッダ（DSアプリには無い）
		ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
		BOOL isNtrMode;

		// ヘッダ情報再配置
		if( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL )
		{
			// TWL-ROMヘッダ情報の再配置
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}else {
			// NTRモード
			// TWL-ROMヘッダ情報の再配置
			//   ランチャーのROMヘッダが残っている非コピー領域もクリア
			MI_CpuClearFast( (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}

		(void)OS_DisableIrq();							// ここで割り込み禁止にしないとダメ。
		(void)OS_SetIrqMask(0);							// SDKバージョンのサーチに時間がかかると、ARM9がHALTにかかってしまい、ARM7のサウンドスレッドがARM9にFIFOでデータ送信しようとしてもFIFOが一杯で送信できない状態で無限ループに入ってしまう。
		(void)OS_SetIrqMaskEx(0);
		
		// マウント情報を一時的にSYSM_TWL_MOUNT_INFO_TMP_BUFFERに登録
		// ここまでにHW_TWL_ROM_HEADER_BUFのヘッダが次のアプリのものに変更されている必要あり
		SYSMi_SetBootAppMountInfo( &SYSMi_GetWork2()->bootTitleProperty );
		
		// FSによってshared領域にコピーされたランチャー自身のマウントパスのクリア
		MI_CpuClearFast((char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, OS_MOUNT_PATH_LEN);
		
		BOOTi_ClearREG_RAM();							// ARM7側のメモリ＆レジスタクリア。
		reg_MI_MBK9 = 0;								// 全WRAMのロック解除
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9に対してブートするようIRQで要求＋ARM7のステートを１にする。
		
        // TWL/NTRモード判定
        if ( ! dh->s.platform_code ||
             (SYSM_IsRunOnDebugger() && ((SYSMRomEmuInfo*)HOTSW_GetRomEmulationBuffer())->isForceNTRMode) )
        {
            isNtrMode = TRUE;
        }
        else
        {
            isNtrMode = FALSE;
        }
		
		// 鍵情報の引渡しを行う。
		// ブートアプリのROMヘッダのaccessKeyControl情報を見て判定
		// 引渡しは、IRQスタック領域を使うので、割り込みを禁止してからセットする。
		{
			BOOL isClearSlotB = TRUE;
			BOOL isClearSlotC = TRUE;
			
			MI_CpuClearFast( (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, HW_LAUNCHER_DELIVER_PARAM_BUF_SIZE );
			if( ! isNtrMode ) {
				if( th->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK ) {
					// commonClientKey
					if( th->s.access_control.common_client_key ) {
						void *pCommonKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											OSi_GetFromFirmAddr()->aes_key[ 0 ] : (void *)dev_commonKey;
						MI_CpuCopy8( pCommonKey, (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, AES_BLOCK_SIZE );
					}
					// commonClientKeyForDebugger
					else if( th->s.access_control.common_client_key_for_debugger_sysmenu ) {
						MI_CpuCopy8( OSi_GetFromFirmAddr()->aes_key[ 1 ], (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, AES_BLOCK_SIZE );
					}
					
					// HW AES Slot B
					if( th->s.access_control.hw_aes_slot_B ) {
						void *pSeedES  =  ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0 ] ) : (void *)dev_seedES;
						MI_CpuCopy8( pSeedES,  (void *)( HW_LAUNCHER_DELIVER_PARAM_BUF + 0x10 ), AES_BLOCK_SIZE );
						isClearSlotB = FALSE;
						// AESスロットのデフォルト値セットは不要
					}
					// HW AES Slot C
					if( th->s.access_control.hw_aes_slot_C ) {
						void *pSeedNAM =  ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x10 ] ) : (void *)dev_seedNAM;
						void *pSeedSlotC = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x20 ] ) : (void *)dev_seedSlotC;
						MI_CpuCopy8( pSeedNAM, (void *)( HW_LAUNCHER_DELIVER_PARAM_BUF + 0x20 ), AES_BLOCK_SIZE );
						isClearSlotC = FALSE;
						// AESスロットのデフォルト値セット
						AES_Lock();
						AES_SetKeySeedC( pSeedSlotC );
						AES_Unlock();
					}
				}
			}
			// ブートするアプリに応じて、AESキースロットのクリアを行う。
			AESi_ResetAesKeyA();
			if( isClearSlotB ) AESi_ResetAesKeyB();
			if( isClearSlotC ) AESi_ResetAesKeyC();
			
			// 鍵は不要になるので、消しておく
			{
				OSFromFirmBuf* fromFirm = (void*)HW_FIRM_FROM_FIRM_BUF;
				MI_CpuClearFast(fromFirm, sizeof(OSFromFirmBuf));
			}
		}
		// SDK共通リブート
		{
			REBOOTTarget target = REBOOT_TARGET_TWL_SYSTEM;
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			u32 *post_clear_list;
			// メモリリストの設定
			// [TODO:] ショップアプリで鍵を残す場合、NANDファーム引数の領域（WRAMにある）を消さないように注意。
			//         WRAMリマップ後の消し漏れやバッファオーバランの懸念回避のため不要な鍵はpre clearで消す。
			// [TODO:] DSPの使っているWRAMをどこからどこまで消すか？
			static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
			{
				// pre clear
				SYSM_OWN_ARM7_WRAM_ADDR, NULL, // SYSM_OWN_ARM7_WRAM_ADDR（SDK_AUTOLOAD_WRAM_START）はリンカから与えられるので定数でない
				SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
				SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
				OS_BOOT_CODE_BUF_END, 1,     // REBOOTコアコードとスタックの隙間サイズはメモリリスト完成後に差し替える（NULLではREBOOT_GetCoreStackSizeが失敗する）
				HW_WRAM_BASE, HW_WRAM_SIZE,  // 共有WRAM　　Launcherの特殊配置なので、BASEからサイズぶん
				NULL,
				// copy forward
				NULL,
				// copy backward
				NULL,
				// post clear
				NULL,
			};
			
			mem_list[CLRLIST_OWN_ARM7_WRAM_SIZE_IDX] = SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR;
			
			// copy forwardリスト設定
			mem_list[list_count++] = SYSM_TWL_MOUNT_INFO_TMP_BUFFER;
			mem_list[list_count++] = (u32)th->s.sub_mount_info_ram_address;
			mem_list[list_count++] = SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN;
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
			if ( ! isNtrMode )
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
			// REBOOTコアコードとスタックの隙間サイズを算出
			mem_list[CLRLIST_REBOOT_STACK_PAD_SIZE_IDX] = OS_BOOT_STACK_TOP - REBOOT_GetCoreStackSize(mem_list) - OS_BOOT_CODE_BUF_END;
			
			// サウンド停止
			SND_Shutdown();
			
			// アプリケーション選択
			if ( ! isNtrMode )
			{
				if ( th->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK )
				{
					if ( th->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK )
					{
						target = REBOOT_TARGET_TWL_SECURE;
					}
					else
					{
						target = REBOOT_TARGET_TWL_SYSTEM;
					}
				}
				else
				{
					target = REBOOT_TARGET_TWL_APP;
				}
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

			// 外部デポップ回路を有効にする
			CDC_EnableExternalDepop();

			// I2S停止（MCLKは動作継続）
			reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;

            if ( isNtrMode || th->s.codec_mode == OS_CODECMODE_NITRO )
            {
				// （CODEC-DSモード）
				CDC_GoDsMode();
            }
			else
			{
				// 再初期化（CODEC-TWLモード）
				CDC_Init();
			}

			// I2S再開
			// DSサウンド：DSP = 8:0
			// 32KHz
			reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
							  REG_SND_SMX_CNT_E_MASK;

			// デポップ期間のあと外部デポップ回路を無効にする
			// 単純なウェイトになるため、将来的にはコンポーネントでデポップを
			// 無効にするように変更する予定（TODO)
			OS_SpinWait(OS_MilliSecondsToTicks(100) * 64);
			CDC_DisableExternalDepop();

#ifdef SDK_ARM7
            // デバッガではTWLカードスロット２を電源ON
            if ( SYSM_IsRunOnDebugger() )
            {
                HOTSWi_TurnCardPowerOn( 2 );
            }
#endif // SDK_ARM7

#if defined(FIRM_USE_TWLSDK_KEYS) || defined(SYSMENU_DISABLE_RETAIL_BOOT)
            // TwlSDK内の鍵を使っている時は製品用CPUではTWLアプリはブートしない
            if ( ! (*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK) )
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
		reg_MI_MC_SWP ^= 0x80;											// カードスロットのスワップ
#endif
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = SYSMi_GetWork()->nCardID;		// カード抜けチェックバッファにカードIDをセット
	}else {
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = 0;
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
