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
#include <twl/sea.h>
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include <sysmenu/util.h>
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
	
    SEA_Init();
	
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
	
	// NAMライブラリ初期化
	NAM_Init( Alloc, Free );        // NAMUTライブラリがNAMライブラリを使用している
	NAMUT_Init( Alloc, Free );
	
	// ※本来ならランチャーからのパラメータチェックを行い、
	//   初回起動シーケンスに入るパスがある
	
	{
		OS_TPrintf( "LCFGTWLOwnerInfo       : 0x%04x\n", sizeof(LCFGTWLOwnerInfo) );
		OS_TPrintf( "LCFGTWLParentalControl : 0x%04x\n", sizeof(LCFGTWLParentalControl) );
		OS_TPrintf( "LCFGTWLSettingsData    : 0x%04x\n", sizeof(LCFGTWLSettingsData) );
	}
	
	// ::::::::::::::::::::::::::::::::::::::::::::::
	// TWL設定データファイルの読み込み
	// ::::::::::::::::::::::::::::::::::::::::::::::
	g_isValidTSD = TRUE;
    {
        u8 *pBuffer = Alloc( LCFG_READ_TEMP );
        if( pBuffer ) {
			// NANDからTWL本体設定データをリード
			BOOL isRead = LCFG_ReadTWLSettings( (u8 (*)[LCFG_READ_TEMP])pBuffer );
			
			// リード失敗ファイルが存在する場合は、ファイルをリカバリ
			if( LCFG_RecoveryTWLSettings() ) {
				if( isRead ) {
					// ミラーデータのうち、一方でもリードできていたなら何もしない。
				}else {
					// リードに完全に失敗していた場合は、フラッシュ壊れシーケンスへ。
					LCFG_TSD_SetFlagFinishedBrokenTWLSettings( FALSE );
					(void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );	// LCFG_READ_TEMP > LCFG_WRITE_TEMP なので、pBufferをそのまま流用
				}
			}else {
				// リカバリ失敗時は、FALTALエラー
				g_isValidTSD = FALSE;
			}
            Free( pBuffer );
        }else {
			// メモリ確保ができなかった時は、FATALエラー
			g_isValidTSD = FALSE;
		}
	}
	
	UTL_CaribrateTP( LCFG_TSD_GetTPCalibrationPtr() );
	
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

