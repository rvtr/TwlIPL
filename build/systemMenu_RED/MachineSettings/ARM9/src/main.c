/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

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
#include "misc.h"
#include "MachineSetting.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------
int (*g_pNowProcess)( void );
BOOL g_isValidTSD;
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------


// ============================================================================
// function's description
// ============================================================================
void TwlMain(void)
{
	
	// 初期化----------------------------------
    OS_Init();
	OS_InitTick();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
    GX_Init();
	GX_SetPower(GX_POWER_ALL);										// 各ロジック パワーON
	FS_Init( 3 );
	
	// 割り込み許可----------------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// デバイス初期化-------------------------------
	TP_Init();
	(void)RTC_Init();
	
	// システムの初期化------------------
	InitAllocator();
	
	// ※本来ならランチャーからのパラメータチェックを行い、
	//   初回起動シーケンスに入るパスがある
	
	// TWL設定のリード
	SYSM_SetAllocFunc( Alloc, Free );								// SYSM_ReadTWLSettingsFile()の実行に必要。
	
	// ::::::::::::::::::::::::::::::::::::::::::::::
	// TWL設定データファイルの読み込み
	// ::::::::::::::::::::::::::::::::::::::::::::::
	(void)LCFG_ReadHWSecureInfo();
	g_isValidTSD = LCFG_ReadTWLSettings();
	if( g_isValidTSD ) {
		SYSM_CaribrateTP();
	}
	
	InitBG();
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	MachineSettingInit();
	// メインループ----------------------------
	while ( 1 ) {
		OS_WaitIrq( 1, OS_IE_V_BLANK );								// Vブランク割り込み待ち
		
		ReadKeyPad();												// キー入力の取得
		
		(void)g_pNowProcess();
		
		GetAndDrawRTCData( &g_rtcDraw, FALSE );
	}
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

