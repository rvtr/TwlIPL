/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     bootAPI.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

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
#include <twl/camera.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>
#include <sysmenu/ds.h>
#include <firm/format/wram_regs.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include <firm/format/from_brom.h>
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

//  FinalizeCardPulledOut();                                // カード抜け検出終了処理
    BOOTi_ClearREG_RAM();                                   // レジスタ＆RAMクリア
    (void)GX_VBlankIntr( FALSE );

    for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {                // 割り込み禁止状態でDMA停止
        MI_StopDma( (u16)i );
        MI_StopNDma( (u16)i );
    }

    (void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
    OS_EnableInterrupts();
    (void)OS_SetIrqMask( OS_IE_SUBP );                      // サブプロセッサ割り込みのみを許可。
    MI_SetWramBank(MI_WRAM_ARM7_ALL);                       // WRAM0/1の最終配置はOS_Bootで行う
    reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;        // ARM9ステートを "0x0f" に
                                                            // ※もうFIFOはクリア済みなので、使わない。
    // ARM7からの通知待ち
    OS_WaitIrq( 1, OS_IE_SUBP );

    OS_TPrintf( "INTR SUBP passed!!\n" );
    // 割り込みをクリアして最終ブートシーケンスへ。
    reg_PXI_SUBPINTF &= 0x0f00;                             // サブプロセッサ割り込み許可フラグをクリア
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u32)~0 );

    // WRAMの配置
    {
        ROM_Header_Short *pROMH = (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
        MIHeader_WramRegs *pWRAMREGS = (MIHeader_WramRegs *)pROMH->main_wram_config_data;
        reg_GX_VRAMCNT_C    = pWRAMREGS->main_vrambnk_c;
        reg_GX_VRAMCNT_D    = pWRAMREGS->main_vrambnk_d;
        // WRAM0/1の最終配置はOS_Bootで行う
    }

    // SDK共通リブート
	{
	    // メモリリストの設定
		// [TODO:] ショップアプリで鍵を残す場合、NANDファーム引数の領域(ITCMにある)を消さないように注意。
		//         バッファオーバランの懸念回避のため不要な鍵はpre clearで消す。
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
	    
	    // 再配置リストの作成と設定（ほぼARM7側でやるのでこちらは空）
	    static u32 relocate_list[] =
	    {
			NULL
		};
	    
		REBOOTTarget target = REBOOT_TARGET_TWL_SYSTEM;
        BOOL ds = FALSE;
		ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL拡張ROMヘッダ（DSアプリには無い）
		ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
		
		// アプリケーション選択
		if ( dh->s.platform_code )
		{
			if ( th->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK )
			{
				if ( th->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK )
				{
					target = REBOOT_TARGET_TWL_SECURE_SYSTEM;
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
            OS_Terminate();
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

#if defined(FIRM_USE_TWLSDK_KEYS) || defined(SYSMENU_DISABLE_RETAIL_BOOT)
        // TwlSDK内の鍵を使っている時は製品用CPUではTWLアプリはブートしない
        if ( ! (*(u8*)OS_CHIPTYPE_DEBUGGER_ADDR & OS_CHIPTYPE_DEBUGGER_MASK) && !ds )
        {
            OS_Terminate();
        }
#endif // FIRM_USE_SDK_KEYS || SYSMENU_DISABLE_RETAIL_BOOT

        // USG以前のDSアプリには無線パッチを適用
        // （キャッシュ領域の排他制御簡略化のためARM9で行う）
        if ( REBOOT_TARGET_DS_APP )
        {
            DS_InsertWLPatch();
        }

        // デバッガによるROMエミュレーション時はNTR-ROMヘッダバッファの
        // ゲームコマンドパラメータをスクランブルOFF設定に書き換える
        dh->s.game_cmd_param = SYSMi_GetWork()->gameCommondParam;

		// セキュアシステム以外は鍵を消しておく
		if ( target != REBOOT_TARGET_TWL_SECURE_SYSTEM )
		{
			MI_CpuClearFast((void*)HW_FIRM_FROM_BROM_BUF, HW_FIRM_FROM_BROM_BUF_SIZE);
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
    (void)OS_ResetRequestIrqMask( (u32)~0 );

	// レジスタクリアは基本的に OS_Boot で行う
}

