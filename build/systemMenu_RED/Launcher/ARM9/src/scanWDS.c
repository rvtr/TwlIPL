/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     scanWDS.c

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

#include "scanWDS.h"

#define WDS_THREAD_PRIO			15
#define WDS_DMA_NO				3

char *callbackstring[] = {
	"WDSWRAPPER_CALLBACK_INITIALIZE",
	"WDSWRAPPER_CALLBACK_CLEANUP",
	"WDSWRAPPER_CALLBACK_STARTSCAN",
	"WDSWRAPPER_CALLBACK_STARTSCAN2",
	"WDSWRAPPER_CALLBACK_STOPSCAN",
};

static BOOL s_isStarted = FALSE;
static BOOL s_isClearnup = FALSE;

// WDSWrapper終了？
BOOL IsClearnupWDSWrapper( void )
{
	if( s_isStarted ) {
		return s_isClearnup;
	}else {
		return TRUE;
	}
}


// WDSWrapper用コールバック関数
void Callback_WDSWrapper( void *ptr )
{
	WDSWrapperCallbackParam *callback = (WDSWrapperCallbackParam *)ptr;
	WDS_PRINTF("Callback_WDSWrapper: %s %d ", callbackstring[callback->callback], callback->errcode );
	
	switch( callback->callback ) {
	case WDSWRAPPER_CALLBACK_INITIALIZE:
		s_isStarted = TRUE;
		s_isClearnup = FALSE;
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN2:
		// n秒間隔のビーコン間欠スキャン一回分が完了
		// ビーコンを受け取っていないという結果が出た場合、強調表示を消します
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			WDS_PRINTF( "強調表示を消します" );
		else {
			WDS_PRINTF( "強調表示をつけます" );
		}
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN:
		// n秒間隔のビーコン間欠スキャン一回分が完了
		// ビーコンを受け取っている場合のみ強調表示を付けます
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_SUCCESS )
			WDS_PRINTF( "強調表示をつけます" );
		break;
	case WDSWRAPPER_CALLBACK_CLEANUP:
		s_isStarted = FALSE;
		s_isClearnup = TRUE;
		break;
	}
	
	WDS_PRINTF( "\n" );
}

// Sleepコールバック関数
void Callback_WDSPreSleep( void *ptr )
{
#pragma unused( ptr )
	WDS_WrapperCleanup();
	while ( WDS_WrapperCheckThreadRunning() == WDSWRAPPER_ERRCODE_SUCCESS )
	{
		OS_Sleep(1);
	}
}

void Callback_WDSPostSleep( void *ptr )
{
#pragma unused( ptr )
	InitializeWDS();
}

// 初期化関数
void InitializeWDS( void )
{
	static BOOL	isInitialized = FALSE;

	{
		WDSWrapperInitializeParam param;
		param.threadprio = WDS_THREAD_PRIO;
		param.dmano      = WDS_DMA_NO;
		param.callback   = Callback_WDSWrapper;
		param.alloc      = SYSM_Alloc;
		param.free       = SYSM_Free;
		(void)WDS_WrapperInitialize( param );		// 初期化と動作開始を兼ねている。（失敗しても止まりはしないので、気にしない）
	}

	if ( ! isInitialized )
	{
		static PMSleepCallbackInfo preCbInfo;
		static PMSleepCallbackInfo postCbInfo;
		PM_SetSleepCallbackInfo( &preCbInfo, Callback_WDSPreSleep, NULL );
		PM_PrependPreSleepCallback( &preCbInfo );
		PM_SetSleepCallbackInfo( &postCbInfo, Callback_WDSPostSleep, NULL );
		PM_AppendPostSleepCallback( &postCbInfo );
	}
	isInitialized = TRUE;
}

