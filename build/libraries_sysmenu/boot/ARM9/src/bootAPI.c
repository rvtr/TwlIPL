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
#include <twl/dsp.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
#include <sysmenu/ds.h>
#include <firm/format/wram_regs.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include <firm/format/from_firm.h>
#include "reboot.h"
#include "../../../hotsw/ARM7/include/hotswTypes.h"


// define data-------------------------------------------------------
#define SUBP_RECV_IF_ENABLE     		0x4000

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------
static void BOOTi_ClearREG_RAM( void );
static void BOOTi_RebootCallback( void** entryp, void* mem_list, REBOOTTarget* target );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static REBOOTTarget target;

// メモリリスト
// バッファオーバランのリスク回避のため不要な鍵はpre clearで消す。
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

void BOOT_Ready( void )
{
	// 最適化されるとポインタを初期化しただけでは何もコードは生成されません
	ROM_Header *th = (ROM_Header *)SYSM_APP_ROM_HEADER_BUF;          // TWL拡張ROMヘッダ（キャッシュ領域、DSアプリには無い）
	ROM_Header *dh = (ROM_Header *)(SYSMi_GetWork()->romHeaderNTR);  // DS互換ROMヘッダ（非キャッシュ領域）

	// HOTSW終了処理待ち
#ifndef SYSM_NO_LOAD
	while( ! HOTSW_isFinalized() ) {
		OS_Sleep( 1 );
	}
#endif

	// リブート
	REBOOTi_SetTwlRomHeaderAddr( th );
	REBOOTi_SetRomHeaderAddr( dh );
	REBOOTi_SetPostFinalizeCallback( BOOTi_RebootCallback );
	OS_Boot( OS_BOOT_ENTRY_FROM_ROMHEADER, mem_list, target );
}

// ブート準備をして、ARM7からの通知を待つ。
// SDKのFinalize処理後に呼び出される
static void BOOTi_RebootCallback( void** entryp, void* mem_list_v, REBOOTTarget* target )
{
#pragma unused(entryp)
	u32* mem_list = mem_list_v;
	ROM_Header *th = (void*)REBOOTi_GetTwlRomHeaderAddr();
	ROM_Header *dh = (void*)REBOOTi_GetRomHeaderAddr();
    BOOL isNtrMode;

    // エントリアドレスの正当性をチェックし、無効な場合は無限ループに入る。
//  SYSMi_CheckEntryAddress();

//  FinalizeCardPulledOut();                                // カード抜け検出終了処理
	BOOTi_ClearREG_RAM();                                   // レジスタ＆RAMクリア
    (void)GX_VBlankIntr( FALSE );
	DC_StoreAll();

    MI_StopAllDma();                                        // 割り込み禁止状態でDMA停止
    MI_StopAllNDma();

    (void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
    OS_EnableInterrupts();
    (void)OS_SetIrqMask( OS_IE_SUBP );                      // サブプロセッサ割り込みのみを許可。
    MI_SetWramBank(MI_WRAM_ARM7_ALL);                       // WRAM0/1の最終配置はOS_Bootで行う
    reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;        // ARM9ステートを "0x0f" に
                                                            // ※もうFIFOはクリア済みなので、使わない。
    // ARM7からの通知待ち
    // この時点でARM7によるdhへのNTR-ROMヘッダをコピーが保証される
    OS_WaitInterrupt( 1, OS_IE_SUBP );

    OS_TPrintf( "INTR SUBP passed!!\n" );

    // SDKのFinalize処理完了後にブート種別をアプリのものへ変更
    ( (OSBootInfo *)OS_GetBootInfo() )->boot_type = SYSMi_GetWork()->appBootType;

    // 割り込みをクリアして最終ブートシーケンスへ。
    reg_PXI_SUBPINTF &= 0x0f00;                             // サブプロセッサ割り込み許可フラグをクリア
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u32)~0 );

    // TWL/NTRモード判定
    if ( ! dh->s.platform_code
#ifndef SYSM_NO_LOAD
         || (SYSM_IsRunOnDebugger() && ((SYSMRomEmuInfo*)HOTSW_GetRomEmulationBuffer())->isForceNTRMode)
#endif
       )
    {
        isNtrMode = TRUE;
    }
    else
    {
        isNtrMode = FALSE;
    }
	
    // WRAMの配置
    {
        MIHeader_WramRegs *pWRAMREGS = (MIHeader_WramRegs *)th->s.main_wram_config_data;
        int i;
        reg_GX_VRAMCNT_C    = pWRAMREGS->main_vrambnk_c;
        reg_GX_VRAMCNT_D    = pWRAMREGS->main_vrambnk_d;
        // WRAM0/1の最終配置はOS_Bootで行う

        // DSP停止
        DSP_ResetOn();     			// DSPブロック初期化
        DSP_ResetInterfaceCore();   // DSP-A9IFの初期化
        DSP_PowerOff();    			// DSPをOFF

        // TWL拡張WRAM
        // ARM7のrebootでクリア
        for (i=0; i<MI_WRAM_C_MAX_NUM; i++)
        {
            MIi_SetWramBankEnable_B(i, MI_WRAM_ENABLE);
            MIi_SetWramBankEnable_C(i, MI_WRAM_ENABLE);
        }
        MI_SwitchWram_B(MI_WRAM_DSP,  MI_WRAM_ARM7);
        MI_SwitchWram_B(MI_WRAM_ARM9, MI_WRAM_ARM7);
        MI_SwitchWram_C(MI_WRAM_DSP,  MI_WRAM_ARM7);
        MI_SwitchWram_C(MI_WRAM_ARM9, MI_WRAM_ARM7);
    }

    // SDK共通リブート
	{
		*target = REBOOT_TARGET_TWL_SYSTEM;
		
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

		// 鍵は不要になるので、消しておく
		{
			OSFromFirmBuf* fromFirm = (void*)HW_FIRM_FROM_FIRM_BUF;
			MI_CpuClearFast(fromFirm, sizeof(OSFromFirmBuf));
		}
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

