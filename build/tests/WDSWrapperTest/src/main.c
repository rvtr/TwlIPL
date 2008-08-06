/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - WDSWrapperTest
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
#include <sysmenu/WDSWrapper.h>

// Vブランク関数
static void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

// ヒープ初期化関数
static void InitializeAllocateSystem(void)
{
    void *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

// WDSWrapper用アロケータ
static void *Alloc_WDSWrapper( u32 size )
{
	return OS_Alloc( size );
}

static void Free_WDSWrapper( void *ptr )
{
	OS_Free( ptr );
}

char *callbackstring[] = {
	"WDSWRAPPER_CALLBACK_INITIALIZE",
	"WDSWRAPPER_CALLBACK_CLEANUP",
	"WDSWRAPPER_CALLBACK_STARTSCAN",
	"WDSWRAPPER_CALLBACK_STARTSCAN2",
	"WDSWRAPPER_CALLBACK_STOPSCAN",
};

// WDSWrapper用コールバック関数
static void Callback_WDSWrapper( void *ptr )
{
	WDSWrapperCallbackParam *callback = (WDSWrapperCallbackParam *)ptr;
	OS_TPrintf("Callback_WDSWrapper: %s %d ", callbackstring[callback->callback], callback->errcode );
	
	switch( callback->callback ) {
	case WDSWRAPPER_CALLBACK_INITIALIZE:
		OS_TPrintf( "初期化完了" );
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN2:
		// n秒間隔のビーコン間欠スキャン一回分が完了
		// ビーコンを受け取っていないという結果が出た場合、強調表示を消します
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			OS_TPrintf( "強調表示を消します" );
		else {
			OS_TPrintf( "強調表示をつけます" );
		}
		// 受信したビーコン情報データをArgument領域に書き込む
#ifdef SDK_TWL
		OS_TPrintf( "\n" );
		WDS_WrapperSetArgumentParam();
#endif
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN:
		// n秒間隔のビーコン間欠スキャン一回分が完了
		// ビーコンを受け取っている場合のみ強調表示を付けます
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_SUCCESS )
			OS_TPrintf( "強調表示をつけます" );
		break;
	case WDSWRAPPER_CALLBACK_STOPSCAN:
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			OS_TPrintf( "強調表示を消します" );
		else {
			OS_TPrintf( "強調表示をつけます" );
		}
		break;
	case WDSWRAPPER_CALLBACK_CLEANUP:
		OS_TPrintf( "解放完了" );
		break;
	}
	OS_TPrintf( "\n" );
}

// スリープモードに入る前に呼び出されるコールバック関数
static void Callback_WDSPreSleep( void *ptr )
{
#pragma unused( ptr )
	WDS_WrapperCleanup();
	while( WDS_WrapperCheckThreadRunning() == WDSWRAPPER_ERRCODE_SUCCESS )
		OS_Sleep( 100 );
}

// スリープモードから復帰する際に呼び出されるコールバック関数
static void Callback_WDSPostSleep( void *ptr )
{
#pragma unused( ptr )
	WDSWrapperInitializeParam param;
	
	// WDSWrapper初期化と動作開始
	param.threadprio = 20;
	param.dmano = 1;
	
	param.callback = Callback_WDSWrapper;
	param.alloc = Alloc_WDSWrapper;
	param.free = Free_WDSWrapper;
	WDS_WrapperInitialize( param );
}

// メイン関数
void NitroMain(void)
{
	WDSWrapperInitializeParam param;
	u16 lastpad = 0x0000;
	u16 nowpad = 0x0000;
	PMSleepCallbackInfo presleepcallbackinfo, postsleepcallbackinfo;
	
	// 各種初期化処理
	OS_Init();
	OS_InitTick();
	OS_InitAlarm();

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	
	//---- power manager callback
	PM_SetSleepCallbackInfo( &presleepcallbackinfo, Callback_WDSPreSleep, NULL );
	PM_AppendPreSleepCallback( &presleepcallbackinfo );
	PM_SetSleepCallbackInfo( &postsleepcallbackinfo, Callback_WDSPostSleep, NULL );
	PM_AppendPostSleepCallback( &postsleepcallbackinfo );
	
	InitializeAllocateSystem();
	
	OS_TPrintf( "WDSWrapper Sample\n\n" );
	OS_TPrintf( "Xボタン: WDSラッパーとWDSを初期化し、間欠スキャンを開始\n" );
	OS_TPrintf( "Bボタン: スキャンを中断\n" );
	OS_TPrintf( "Aボタン: スキャンを再開\n" );
	OS_TPrintf( "Yボタン: スキャンを中断し、WDSラッパーとWDSを解放\n" );
	OS_TPrintf( "スタートボタン: ヒープをダンプ表示\n" );
	OS_TPrintf( "セレクトボタン: スリープモードin/out\n" );

	// キー入力で中断・再開・解放をやる
	while( 1 ) {
		lastpad = nowpad;
		nowpad = PAD_Read();
		
		// Aボタン: スキャンを再開
		// Bボタン: スキャンを中断
		// Xボタン: ラッパーを初期化
		// Yボタン: ラッパーを解放
		// スタートボタン: ヒープをダンプ表示
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_A && !( nowpad & PAD_BUTTON_A ) )
			WDS_WrapperStartScan();
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_B && !( nowpad & PAD_BUTTON_B ) )
			WDS_WrapperStopScan();
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_X && !( nowpad & PAD_BUTTON_X ) ) {
			// WDSWrapper初期化と動作開始
			param.threadprio = 20;
			param.dmano = 1;
	
			param.callback = Callback_WDSWrapper;
			param.alloc = Alloc_WDSWrapper;
			param.free = Free_WDSWrapper;
			WDS_WrapperInitialize( param );
		}
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_Y && !( nowpad & PAD_BUTTON_Y ) )
			WDS_WrapperCleanup();
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_START && !( nowpad & PAD_BUTTON_START ) )
			OS_DumpHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE );
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_SELECT && !( nowpad & PAD_BUTTON_SELECT ) ) {
			PM_GoSleepMode( PM_TRIGGER_KEY, PM_PAD_LOGIC_OR, PAD_BUTTON_SELECT );
		}
		OS_Sleep( 100 );
	}
}
