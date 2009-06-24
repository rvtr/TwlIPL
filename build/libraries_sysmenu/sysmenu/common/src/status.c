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

/*** フラグセット、割り込み禁止つき arm9 ****/
void SYSM_SetHeaderLoadCompleted( BOOL comp )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isHeaderLoadCompleted = comp;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetLoadFinished( BOOL finish )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isLoadFinished = finish;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetLoadSucceeded( BOOL succeed )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isLoadSucceeded = succeed;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetCardBoot( BOOL card )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isCardBoot = card;
	OS_RestoreInterrupts( mode );
}

// ロゴデモスキップかどうかをセット
void SYSM_SetLogoDemoSkip( BOOL skip )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isLogoSkip = skip;
	OS_RestoreInterrupts( mode );
}

// TSD有効/無効をセット
void SYSM_SetValidTSD( BOOL valid )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isValidTSD = valid;
	OS_RestoreInterrupts( mode );
}

/*** フラグセット、割り込み禁止つき arm7 ****/
// 必要なさげ

void SYSM_SetHotStart( BOOL hot )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isHotStart = hot;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetValidLauncherParam( BOOL valid )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isValidLauncherParam = valid;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetResetRTC( BOOL reset )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isResetRTC = reset;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetNANDFatalError( BOOL fatal )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isNANDFatalError = fatal;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetARM9Start( BOOL start )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isARM9Start = start;
	OS_RestoreInterrupts( mode );
}

/*** ここまで フラグセット関数 ***/

// ホットスタートか？
BOOL SYSM_IsHotStart( void )
{
	return (BOOL)SYSMi_GetWork()->flags.arm7.isHotStart;
}

// ロゴデモスキップか？
BOOL SYSM_IsLogoDemoSkip( void )
{
	return (BOOL)SYSMi_GetWork()->flags.arm9.isLogoSkip;
}

// ISデバッガのバナービューモード起動かどうか？
BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
	return ( SYSM_IsRunOnDebugger() &&
			 SYSM_IsExistCard() &&
			 SYSMi_GetWork()->romEmuInfo.isForceBannerViewMode );
}

// TSD有効？
BOOL SYSM_IsValidTSD( void )
{
	return (BOOL)SYSMi_GetWork()->flags.arm9.isValidTSD;
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
	// エントリアドレスがMMEMもしくはWRAMのロード可能領域外なら、不正と判定。
	if( !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= HW_MAIN_MEM ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  HW_TWL_MAIN_MEM_SHARED )
		 ) ||
		!( ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_MAIN_MEM ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  HW_TWL_MAIN_MEM_SHARED ) ) ||
		   ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_WRAM_BASE ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_NTR_ARM7_LOAD_WRAM_END ) )
		 ) ||
	    !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= (u32)SYSM_GetCardRomHeader()->main_ram_address ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  (u32)SYSM_GetCardRomHeader()->main_ram_address + SYSM_GetCardRomHeader()->main_size )
		 ) ||
	    !( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address >= (u32)SYSM_GetCardRomHeader()->sub_ram_address ) &&
		   ( (u32)SYSM_GetCardRomHeader()->sub_entry_address <  (u32)SYSM_GetCardRomHeader()->sub_ram_address + SYSM_GetCardRomHeader()->sub_size )
		 )
	 ) {
		OS_TPrintf("entry address invalid.\n");
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}
