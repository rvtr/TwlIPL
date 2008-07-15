/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     status.c

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
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------


// ランチャーパラメータの取得
const LauncherParamBody *SYSM_GetLauncherParamBody( void )
{
	return (const LauncherParamBody *)&SYSMi_GetWork()->launcherParam.body;
}


// ホットスタートか？
BOOL SYSM_IsHotStart( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isHotStart;
}


// ロゴデモスキップかどうかをセット
void SYSM_SetLogoDemoSkip( BOOL skip )
{
	SYSMi_GetWork()->flags.common.isLogoSkip = skip;
}


// ロゴデモスキップか？
BOOL SYSM_IsLogoDemoSkip( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isLogoSkip;
}


// ISデバッガのバナービューモード起動かどうか？
BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
#if 0
	return ( SYSM_IsRunOnDebugger() &&
			 SYSMi_IsExistCard() &&
			 SYSM_GetCardRomHeader()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif
}


// TSD有効/無効をセット
void SYSM_SetValidTSD( BOOL valid )
{
	SYSMi_GetWork()->flags.common.isValidTSD = valid;
}


// TSD有効？
BOOL SYSM_IsValidTSD( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isValidTSD;
}


// 有効なTWL/NTRカードが差さっているか？
BOOL SYSM_IsExistCard( void )
{
	return (BOOL)SYSMi_GetWork()->flags.hotsw.isExistCard;
}


// 検査用カードが差さっているか？
BOOL SYSM_IsInspectCard( void )
{
	return ( SYSM_IsExistCard() && SYSMi_GetWork()->flags.hotsw.isInspectCard );
}


// 有効なTWLカードが差さっているか？
BOOL SYSM_IsTWLCard( void );
BOOL SYSM_IsTWLCard( void )
{
	return ( SYSM_IsExistCard() && ( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) );
}


// 有効なNTRカードが差さっているか？
BOOL SYSM_IsNTRCard( void );
BOOL SYSM_IsNTRCard( void )
{
	return ( SYSM_IsExistCard() && ( SYSM_GetCardRomHeader()->platform_code == PLATFORM_CODE_NTR ) );
}


// エントリアドレスの正当性チェック
BOOL SYSMi_CheckEntryAddress( void )
{
	// エントリアドレスがROM内登録エリアかAGBカートリッジエリアなら、無限ループに入る。
	if( !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= HW_MAIN_MEM ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  HW_TWL_MAIN_MEM_SHARED )
		 ) ||
		!( ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_MAIN_MEM ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  HW_TWL_MAIN_MEM_SHARED ) ) ||
		   ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_WRAM_BASE ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_NTR_ARM7_LOAD_WRAM_END ) )
		 )
	 ) {
		OS_TPrintf("entry address invalid.\n");
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}
