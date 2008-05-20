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
static int  SYSMi_IsValidCard( void );
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


// FATALエラーかどうかをセット
void SYSM_SetFatalError( BOOL isFatalError )
{
	SYSMi_GetWork()->flags.common.isFatalError = isFatalError;
}


// FATALエラーか？
BOOL SYSM_IsFatalError( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isFatalError;
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


// ISデバッガ上で動作しているか？
BOOL SYSM_IsRunOnDebugger( void )
{
	return SYSMi_GetWork()->flags.hotsw.isOnDebugger;
}


// ISデバッガのバナービューモード起動かどうか？
BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
#ifdef __IS_DEBUGGER_BUILD
	return ( SYSM_IsRunOnDebugger() &&
			 SYSMi_IsValidCard() &&
			 SYSM_GetCardRomHeader()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif	// __IS_DEBUGGER_BUILD
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


// NTR,TWLカード存在チェック 		「リターン　1：カード認識　0：カードなし」
static int SYSMi_IsValidCard( void )
{
	if( ( SYSM_GetCardRomHeader()->nintendo_logo_crc16 == 0xcf56 ) &&
	    ( SYSM_GetCardRomHeader()->header_crc16 == SYSMi_GetWork()->cardHeaderCrc16 ) ) {
		return TRUE;												// NTR,TWLカードあり（NintendoロゴCRC、カードヘッダCRCが正しい場合）
																	// ※Nintendoロゴデータのチェックは、特許の都合上、ロゴ表示ルーチン起動後に行います。
	}else {
		return FALSE;												// NTR,TWLカードなし
	}
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
