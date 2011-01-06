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
#include <nitro/os.h>
#include <nitro/code32.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
#include <sysmenu/ds.h>
#include <firm/hw/ARM7/mmap_firm.h>
#include <firm/mi/mainMemory.h>
#include <firm/format/from_firm.h>
#include <firm/aes/ARM7/aes_init.h>
#include "reboot.h"
#include "internal_api.h"
#include "../../../hotsw/ARM7/include/hotswTypes.h"
#include "targetCode.h"

#include <twl/mcu/ARM7/control.h>

extern void SPI_Lock(u32 id);
extern void SPI_Unlock(u32 id);

// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#define PRE_CLEAR_NUM_MAX		(7*2)
#define COPY_NUM_MAX			(6*3)
#define COPY_HEADER_NUM_MAX		(1*3)
#define POST_CLEAR_NUM_MAX		(12 + 6*2)

#define CLRLIST_REBOOT_STACK_PAD_SIZE_IDX	(2*3+1)

#define TITLE_ID_NAND_INITIALIZER		0x00030011304E4941ULL	// 0NIA

// 起動制限をかけるタイトル一覧
typedef struct TitleBlackList {
	OSTitleId	titleID;
	int			rom_version;
}TitleBlackList;

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end );
static void BOOTi_CheckTitleBlackList( ROM_Header_Short *pROMH );
static void BOOTi_RebootCallback( void** entryp, void* mem_list, REBOOTTarget* target );
static void BOOTi_SetMainMemModeForNTR( void );
void BOOTi_SetMainMemModeForNTRCore( u32 addr );

static u32 SearchBinaryArm( void );
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
static u32 SearchBinaryThumb( void );
#endif

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static REBOOTTarget target;

// メモリリスト
static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + COPY_HEADER_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
{
	// pre clear
	HW_WRAM_B_OR_C_MIRROR,   SYSM_OWN_ARM7_WRAM_ADDR_END - HW_WRAM_B_OR_C_MIRROR, // 0x03780000 - 0x0380f000 SYSM_OWN_ARM7_WRAM_ADDRとHW_WRAM_A, HW_WRAM_Bをまとめてクリア
	SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
	OS_BOOT_CODE_BUF_END, 1,     // REBOOTコアコードとスタックの隙間サイズはメモリリスト完成後に差し替える（NULLではREBOOT_GetCoreStackSizeが失敗する）
	HW_LAUNCHER_DELIVER_PARAM_BUF_END, OS_BOOT_CODE_BUF - (HW_LAUNCHER_DELIVER_PARAM_BUF_END + (HW_WRAM_AREA_END - HW_PRV_WRAM_END)), // ランチャーのスタック領域
	HW_WRAM_BASE, HW_WRAM_SIZE,  // 共有WRAM　　Launcherの特殊配置なので、BASEからサイズぶん
	HW_WRAM_C, HW_WRAM_C_SIZE,   // 0x03700000 - 0x03740000
	NULL,
	// copy forward
	NULL,
	// copy backward
	NULL,
	// post clear
	NULL,
};

static u32 twl_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	SYSM_TWL_ARM7_LOAD_MMEM_END, OS_BOOT_A9CODE_BUF,
	OS_BOOT_A9CODE_BUF + OS_BOOT_CODE_SIZE, SYSM_TWL_ARM9_LTD_LOAD_MMEM,
	SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9例外ベクタ
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7例外ベクタ
	NULL,
};

static u32 nitro_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	SYSM_NTR_ARM7_LOAD_MMEM_END, OS_BOOT_A9CODE_BUF,
	SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, SYSM_DBG_NTR_SYSTEM_BUF,
	SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9例外ベクタ
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7例外ベクタ
	NULL,
};

// const data--------------------------------------------------------

// 起動制限をかけるタイトル一覧
static const TitleBlackList s_blackList[] = {
	{ TITLE_ID_NAND_INITIALIZER, 0 },
	{ 0ULL, -1 },
};


void BOOT_Init( void )
{
    *( REGType16v *)0x04000180 = 0x0000;
//	reg_PXI_MAINPINTF = 0x0000;
}


static u32 SearchBinaryArm( void )
{
    u32 target_command_address = 0;
    u32 elem[TARGET_ARM_CODE_NUM];
    u32 i;
    
    OS_TPrintf("=====================================\n");
    for( i = 0; i < TARGET_ARM_CODE_NUM; i++ )
    {
        u32 count = 0;
        u32 *p;

        elem[i] = 0;
        p = (u32 *)MI_CpuFind32( target_code_list_arm[i], 0x0, TARGET_ARM_CODE_MAX_SIZE * sizeof(u32) );
        if( p )
        {
            elem[i] = (u32)(p - target_code_list_arm[i]) * sizeof(u32);
        }
    }

    for( i = 0; i < TARGET_ARM_CODE_NUM; i++ )
    {
        u32 search_size = MAJIKON_APP_ARM7_STATIC_BUFFER_SIZE;
        u32 current     = MAJIKON_APP_ARM7_STATIC_BUFFER;
        u32 *codep      = target_code_list_arm[i];
        u32 hit         = 0;
        BOOL isFinish   = FALSE;

        while( search_size >= elem[i] || hit )
	    {
            if( *(u32 *)current != *codep )
            {
                current     += sizeof(u32);
                search_size -= sizeof(u32);
                continue;
            }
            
            while( *(u32 *)current == *codep )
            {
                hit += sizeof(u32);
                
	            if( *(u32 *)current == MAJIKON_APP_TARGET_COMMAND_ARM )
	            {
	                OS_TPrintf("*** Target Command Find!!\n");
	                target_command_address = current;
	            }

                if( hit == elem[i] )
                {
                    isFinish = TRUE;
                    break;
                }

                codep++;
                current += sizeof(u32);
                search_size -= sizeof(u32);
            }
            
            if( isFinish )
            {
                OS_TPrintf("*** Target Code Find!!\n");
                break;
            }
            target_command_address = 0;
            hit                    = 0;
            codep                  = target_code_list_arm[i];
	    }

        if( isFinish )
        {
            OS_TPrintf("Match!!\n");
            break;
        }
    }

    OS_TPrintf("\ntarget address : 0x%08x\n", target_command_address);
    OS_TPrintf("=====================================\n");
    
    return target_command_address;
}


#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
static u32 SearchBinaryThumb( void )
{
    u32 target_command_address = 0;
    u32 elem[TARGET_THUMB_CODE_NUM];
    u32 i;
    
    OS_TPrintf("=====================================\n");
    for( i = 0; i < TARGET_THUMB_CODE_NUM; i++ )
    {
        u32 count = 0;
        u32 *p;

        elem[i] = 0;
        p = (u32 *)MI_CpuFind32( target_code_list_thumb[i], 0x0, TARGET_THUMB_CODE_MAX_SIZE * sizeof(u16) );
        if( p )
        {
            elem[i] = (u32)((u16 *)p - target_code_list_thumb[i]) * sizeof(u16);
        }
    }

    for( i = 0; i < TARGET_THUMB_CODE_NUM; i++ )
    {
        u32 search_size = MAJIKON_APP_ARM7_STATIC_BUFFER_SIZE;
        u32 current     = MAJIKON_APP_ARM7_STATIC_BUFFER;
        u32 hit         = 0;
        BOOL isFinish   = FALSE;
        u16 *codep      = target_code_list_thumb[i];

        OS_TPrintf("search code %d start\n", i);
        while( search_size >= elem[i] || hit )
	    {
            if( *(u16 *)current != *codep )
            {
                current     += sizeof(u16);
                search_size -= sizeof(u16);
                continue;
            }
            
            while( *(u16 *)current == *codep )
            {
                hit += sizeof(u16);

	            if( *(u16 *)current == MAJIKON_APP_TARGET_COMMAND_THUMB )
	            {
	                OS_TPrintf("*** Target Command Find!!\n");
	                target_command_address = current;
	            }

                if( hit == elem[i] )
                {
                    isFinish = TRUE;
                    break;
                }

                codep++;
                current += sizeof(u16);
                search_size -= sizeof(u16);
            }
            
            if( isFinish )
            {
                OS_TPrintf("*** Target Code Find!!\n");
                break;
            }
            target_command_address = 0;
            hit                    = 0;
            codep                  = target_code_list_thumb[i];
	    }

        if( isFinish )
        {
            OS_TPrintf("Match!!\n");
            break;
        }
    }

    OS_TPrintf("\ntarget address : 0x%08x\n", target_command_address);
    OS_TPrintf("=====================================\n");

    return target_command_address;
}
#endif


BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
        u32 target_address_arm;
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
        u32 target_address_thumb;
#endif
        
		// 最適化されるとポインタを初期化しただけでは何もコードは生成されません
		ROM_Header *th = (ROM_Header *)SYSM_APP_ROM_HEADER_BUF;          // TWL拡張ROMヘッダ（キャッシュ領域、DSアプリには無い）
		ROM_Header *dh = (ROM_Header *)(SYSMi_GetWork()->romHeaderNTR);  // DS互換ROMヘッダ（非キャッシュ領域）
		// リブート
		REBOOTi_SetTwlRomHeaderAddr( th );
		REBOOTi_SetRomHeaderAddr( dh );
		REBOOTi_SetPostFinalizeCallback( BOOTi_RebootCallback );

        // ARM7バッファ( 0x0238_0000 )から特定バイナリをサーチ
        target_address_arm = SearchBinaryArm();

#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
        if( target_address_arm == 0x0 )
        {
            target_address_thumb = SearchBinaryThumb();
        }
#endif
        
        if( target_address_arm )
        {
            // パッチ埋め込み
            MI_CpuCopy8( patch_core_arm, (u32 *)MAJIKON_PATCH_ADDR, sizeof(patch_core_arm));
            
            // パッチに飛ばす処理埋め込み
            // 埋め込むコードは2命令あるので、1つ前のアドレスを返す
            MI_CpuCopy8( patch_jump_arm, (u32 *)(target_address_arm - sizeof(u32)), sizeof(patch_jump_arm));
        }
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
        else if( target_address_thumb )
        {
            // パッチ埋め込み
            MI_CpuCopy8( patch_core_arm, (u32 *)MAJIKON_PATCH_ADDR, sizeof(patch_core_arm));
            
            // パッチに飛ばす処理埋め込み
            // dcd部分が4バイトアライメントとれてないとダメなので調整。
            if( !(target_address_thumb % 0x4) )
            {
                target_address_thumb -= 0x2;
            }
            MI_CpuCopy8( patch_jump_thumb, (u32 *)target_address_thumb, sizeof(patch_jump_thumb));
        }
#endif

		OS_Boot( OS_BOOT_ENTRY_FROM_ROMHEADER, mem_list, target );
	}
	return FALSE;
}

// SDKのFinalize処理後に呼び出される
static void BOOTi_RebootCallback( void** entryp, void* mem_list_v, REBOOTTarget* target )
{
#pragma unused(entryp)
		u32* mem_list = mem_list_v;
		ROM_Header *th = (void*)REBOOTi_GetTwlRomHeaderAddr();
		ROM_Header *dh = (void*)REBOOTi_GetRomHeaderAddr();
		ROM_Header *ch = (void*)HW_CARD_ROM_HEADER;
		BOOL isNtrMode;
		u32  spiLockId;

		(void)OS_DisableIrq();							// ここで割り込み禁止にしないとダメ。
		(void)OS_SetIrqMask(0);							// SDKバージョンのサーチに時間がかかると、ARM9がHALTにかかってしまい、ARM7のサウンドスレッドがARM9にFIFOでデータ送信しようとしてもFIFOが一杯で送信できない状態で無限ループに入ってしまう。
		(void)OS_SetIrqMaskEx(0);

		MI_StopAllDma();                                // 割り込み禁止状態でDMA停止
		MI_StopAllNDma();

		// 起動アプリNTR-ROMヘッダへのパッチ処理のためコピー
		MI_CpuCopyFast( th, dh, HW_CARD_ROM_HEADER_SIZE );

		// カードNTR-ROMヘッダをNANDアプリやDSダウンロードプレイのためコピー
		MI_CpuCopyFast( (void*)SYSM_CARD_ROM_HEADER_BAK, ch, HW_CARD_ROM_HEADER_SIZE );

    	if ( (((ROM_Header *)SYSM_CARD_ROM_HEADER_BAK)->s.platform_code & PLATFORM_CODE_FLAG_TWL) && (dh->s.platform_code & PLATFORM_CODE_FLAG_TWL) ){
			// カードTWL-ROMヘッダをHW_TWL_CARD_ROM_HEADER_BUFにコピー
			MI_CpuCopyFast( (void*)SYSM_CARD_ROM_HEADER_BAK, (void*)HW_TWL_CARD_ROM_HEADER_BUF, HW_TWL_CARD_ROM_HEADER_BUF_SIZE );
        }

        // デバッガによるROMエミュレーション時はNTR-ROMヘッダバッファの
        // ゲームコマンドパラメータをスクランブルOFF設定に書き換える
        if ( SYSM_IsRunOnDebugger() )
        {
            // NitroSDKバグ対策でブートメディア種別に関わらずROMヘッダを常時書き換え
            dh->s.game_cmd_param &= ~SCRAMBLE_MASK;
        }
        // カードROMヘッダ（非キャッシュ領域）は常時設定
        ch->s.game_cmd_param = SYSMi_GetWork()->gameCommondParam;

		// この処理は、DSダウンロードプレイ側で行う。
		// MI_CpuCopyFast ( ch, (void *)MB_CARD_ROM_HEADER_ADDRESS, 0x160);

		// ブラックリストをチェックし、起動制限をかける
		BOOTi_CheckTitleBlackList( (void*)th );
		
		// マウント情報を一時的にSYSM_TWL_MOUNT_INFO_TMP_BUFFERに登録
		// ここまでにSYSM_APP_ROM_HEADER_BUFのキャッシュがライトバックされている必要あり
		SYSMi_SetBootAppMountInfo( &SYSMi_GetWork2()->bootTitleProperty );
		
		// FSによってshared領域にコピーされたランチャー自身のマウントパスのクリア
		MI_CpuClearFast((char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, OS_MOUNT_PATH_LEN);
		
		// NAND/TMPアプリ用KeyTableの生成
		if( ! SYSMi_GetWork()->flags.arm9.isCardBoot )
		{
			HOTSWi_MakeBlowfishTableDSForNAND();
		}
		// ARM9FLXの先頭2k暗号オブジェクトをデクリプト
		{
			// 再配置情報があればそちらのアドレス、なければヘッダ指定のアドレス
			void *addr = (SYSMi_GetWork()->romRelocateInfo[0].src != NULL) ? SYSMi_GetWork()->romRelocateInfo[0].src : dh->s.main_ram_address;
			HOTSWi_SetSecureSegmentBuffer(HOTSW_MODE1, addr, SECURE_AREA_SIZE);
			HOTSW_DecryptObjectFile( addr );
		}
		
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
		
		// NTRモード時にはARM9やDMAがメインメモリをアクセスしていないタイミングでデータ非保持モードへ切り替え（フリーズ対策）
		if ( isNtrMode )
		{
            // ARM9がOS_Haltに達するまで待つ（0.1msec）
            OS_SpinWait(HW_CPU_CLOCK_ARM7 / 10000);
			BOOTi_SetMainMemModeForNTR();
		}
		
		BOOTi_ClearREG_RAM();							// ARM7側のメモリ＆レジスタクリア。
		reg_MI_MBK9 = 0;								// 全WRAMのロック解除
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9に対してブートするようIRQで要求＋ARM7のステートを１にする。
		
		// 鍵情報の引渡しを行う。
		// ブートアプリのROMヘッダのaccessKeyControl情報を見て判定
		SYSMi_SetAESKeysForAccessControl( isNtrMode, th );
		
		// SDK共通リブート
		{
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			u32 *post_clear_list;
			
			*target = REBOOT_TARGET_TWL_SYSTEM;
			
			// copy forwardリスト設定
			// マウント情報
			if( !isNtrMode )
			{
				mem_list[list_count++] = SYSM_TWL_MOUNT_INFO_TMP_BUFFER;
				mem_list[list_count++] = (u32)th->s.sub_mount_info_ram_address;
				mem_list[list_count++] = SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN;
			}
			// モジュール再配置コピーforward
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
			// モジュール再配置コピーbackward
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL && SYSMi_GetWork()->romRelocateInfo[l].rev )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].src;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].dest;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].length;
				}
			}
			// NTRシステム領域コピー（pre clearの消去範囲へ含まれるようになったため）
			if( isNtrMode )
			{
				mem_list[list_count++] = HW_MAIN_MEM_SHARED;
				mem_list[list_count++] = 0x02800000 - HW_MAIN_MEM_SHARED_SIZE;
				mem_list[list_count++] = HW_MAIN_MEM_SHARED_SIZE;
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
				// NTR-IPLと同様にシステム領域直前をクリアしておく
				// [TODO:] すでにpost_clear_listに含まれている気がする。要確認
				mem_list[list_count++] = 0x02800000 - OS_BOOT_SYS_CLR_SIZE;
				mem_list[list_count++] = OS_BOOT_SYS_CLR_SIZE - HW_MAIN_MEM_SHARED_SIZE;
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
						*target = REBOOT_TARGET_TWL_SECURE;
					}
					else
					{
						*target = REBOOT_TARGET_TWL_SYSTEM;
					}
				}
				else
				{
					*target = REBOOT_TARGET_TWL_APP;
				}
			}
			else
			{
				*target = REBOOT_TARGET_DS_APP;
			}

			// I2S停止（MCLKは動作継続）
			reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;

			spiLockId = (u32)OS_GetLockID();
			if (spiLockId == OS_LOCK_ID_ERROR)
			{
		        OS_Warning("%s: OS_GetLockID failed.\n", __FUNCTION__);
			}
			SPI_Lock(spiLockId); // CODEC用SPI排他ロック

			// CODEC再初期化
			CDC_Init();

			// CODEC-DSモードへの遷移
            if ( isNtrMode || th->s.exFlags.codec_mode == OS_CODECMODE_NITRO )
            {
				// DSタイトルのスピーカ設定を行う
	            if ( *target == REBOOT_TARGET_DS_APP )
	            {
                    DS_SetSpeakerVolume( dh );
				}
				CDC_GoDsMode();
            }

			SPI_Unlock(spiLockId); // CODEC用SPI排他ロック

			// I2S再開
			// DSサウンド：DSP = 8:0
			// 32KHz
			reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
							  REG_SND_SMX_CNT_E_MASK;

            // USG以前のDSアプリには無線パッチを適用
            // ARM9はロードアドレスをキャッシュストアしておく必要あり
            if ( *target == REBOOT_TARGET_DS_APP )
            {
                DS_InsertWLPatch( dh );
            }
		}
}

static void BOOTi_SetMainMemModeForNTR( void )
{
    BOOTi_SetMainMemModeForNTRCore(HW_TWL_MAIN_MEM_END - 2);
    BOOTi_SetMainMemModeForNTRCore(HW_TWL_MAIN_MEM_EX_END - 2);
}

asm void BOOTi_SetMainMemModeForNTRCore( u32 addr )
{
        ldr     r1,  =MMEM_TCR0
        ldr     r2,  =MMEM_TCR1_DS
        ldrh    r3, [r0]
        strh    r3, [r0]
        strh    r3, [r0]
        ldr     r3,  =MMEM_TCR2
        strh    r1, [r0]
        strh    r2, [r0]
        strh    r3, [r0]
        bx      lr
}

static void BOOTi_ClearREG_RAM( void )
{
#ifdef DEBUG_USED_CARD_SLOT_B_
	reg_MI_MC_SWP ^= 0x80;												// カードスロットのスワップ
#endif
																		// カード抜けチェックバッファにカードIDをセット
	((SDKBootCheckInfo*)HW_BOOT_CHECK_INFO_BUF)->nCardID = SYSMi_GetWork()->appCardID;
	
	*(vu32 *)HW_RESET_PARAMETER_BUF = 0;								// リセットバッファをクリア
	
	// レジスタクリアは基本的に OS_Boot で行う
	
	// クリアしていないレジスタは、VCOUNT, JOY, PIFCNT, MC-, EXMEMCNT, IME, PAUSE, POWLCDCNT, 他セキュリティ系です。
	(void)OS_ResetRequestIrqMask((u32)~0);
	(void)OS_ResetRequestIrqMaskEx((u32)~0);
}

// 単純リスト要素削除
static void BOOTi_DeleteElementFromList( u32 *list, u32 index )
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
		BOOTi_DeleteElementFromList( regionlist, (u32)n );
	}
}

// 起動制限をかけるブラックリストTITLEのチェック
static void BOOTi_CheckTitleBlackList( ROM_Header_Short *pROMH )
{
	const TitleBlackList *pBlackList = &s_blackList[ 0 ];
	
	while( pBlackList->rom_version >= 0 ) {
		if( ( pBlackList->titleID ==  pROMH->titleID ) &&
			( pBlackList->rom_version ==  pROMH->rom_version ) ) {
			OS_TPrintf( "Hit black list : %c%c%c%c ver.%d...Terminate.\n",
						pROMH->titleID_Lo[ 3 ], pROMH->titleID_Lo[ 2 ], pROMH->titleID_Lo[ 1 ], pROMH->titleID_Lo[ 0 ],
						pROMH->rom_version );
			OS_Terminate();
		}
		pBlackList++;
	}
}
