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

#include <twl.h>
#include <sysmenu/WDS.h>
#include <sysmenu.h>
#include "scanWDS.h"

#define	WDS_THREAD_PRIO		10
#define	WDS_STACK_SIZE		1024
#define WDS_MESG_DEPTH		1
static OSThread s_thread;
u64 s_stack[ WDS_STACK_SIZE / sizeof(u64) ];
OSMessage s_msgBuffer[ WDS_MESG_DEPTH ];
OSMessageQueue s_msgQueue;

// WDSスキャンスレッド
static void ScanWDSThread( void *arg );
// アクセスポイント情報のデバッグ表示用関数
static void DumpWDSApInfo( WDSApInfo *apinfo );

// WDS非同期関数のコールバック関数たち
// コールバック関数内ではアプリケーションのステート変数s_appstateを変更する
void WDS_Initialize_CB(void *arg);
void WDS_StartScan_CB(void *arg);
void WDS_EndScan_CB(void *arg);
void WDS_End_CB(void *arg);

// アプリケーションを制御するステートの列挙型
typedef enum AppState {
	APP_STATE_WDSINIT,
	APP_STATE_WDSWAITINIT,
	APP_STATE_WDSSCAN,
	APP_STATE_WDSWAITSCAN,
	APP_STATE_WDSCOMPLETESCAN,
	APP_STATE_WDSENDSCAN,
	APP_STATE_WDSWAITENDSCAN,
	APP_STATE_WDSCOMPLETEENDSCAN,
	APP_STATE_WDSWAITEND,
	APP_STATE_WDSCOMPLETEEND
} AppState;


// 受け取ったビーコン情報を格納する変数(この配列をランチャー経由でホットスポットチャンネルに渡す)
static WDSBriefApInfo briefapinfo[WDS_APINFO_MAX];



// WDSスキャンスレッドの起動
void StartScanWDS( void )
{
	u8 *wdsSysBuf;
	
	// WDSライブラリが使用するバッファを確保(32バイトアラインメントしている必要がある)
	wdsSysBuf = SYSM_Alloc( WDS_GetWorkAreaSize() );
	if( wdsSysBuf == NULL)
	{
		OS_Panic("OS_Alloc Failed");
	}
	// WDSスレッドの起動
	OS_InitMessageQueue( &s_msgQueue, &s_msgBuffer[0], WDS_MESG_DEPTH );
    OS_CreateThread( &s_thread, ScanWDSThread, (void *)wdsSysBuf, s_stack + WDS_STACK_SIZE / sizeof(u64), WDS_STACK_SIZE, WDS_THREAD_PRIO );
    OS_WakeupThreadDirect( &s_thread );
	
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSINIT, OS_MESSAGE_BLOCK );
}


// WDSスキャンスレッド
static void ScanWDSThread( void *arg )
{
#pragma unused(arg)
	OSTick wdsScanBeginTick = 0;
	u8 *wdsSysBuf = arg;
	
	// メインループ
	while( 1 ) {
		OSTick now;
		int i;
		// アプリケーションを制御するステート変数
		AppState appstate;
		
        (void)OS_ReceiveMessage( &s_msgQueue, (OSMessage)&appstate, OS_MESSAGE_BLOCK );
		
		switch( appstate )
		{
		case APP_STATE_WDSINIT:
			// イニシャルステート
			
			// WDSライブラリの初期化関数を呼び出し、その非同期処理の完了を待つ
			OS_Printf("*** WDS_Initialize\n");
			if( WDS_Initialize( wdsSysBuf, WDS_Initialize_CB, 0 ) == 0 )
			{
				OS_Printf("WDS_Initialize successed\n");
			}
			else {
				OS_Panic("WDS_Initialize failed");
			}
			break;
			OS_Printf("*** WDS_Initialize waiting asyncronous process\n");
		case APP_STATE_WDSWAITINIT:
		case APP_STATE_WDSWAITSCAN:
		case APP_STATE_WDSWAITENDSCAN:
		case APP_STATE_WDSWAITEND:
			// 非同期処理の完了を待つステート群
			
			// コールバック関数が呼び出され、ステートが変更されるのを待てばよい
			break;
		case APP_STATE_WDSSCAN:
			// 初期化が完了した直後か、ビーコンスキャン完了時に引き続きスキャンを行う場合に入ってくるステート
			
			//OS_Printf("*** WDS_StartScan\n");
			// ビーコンスキャン非同期処理を開始する
			if( WDS_StartScan( WDS_StartScan_CB ) == 0 )
			{
				if( wdsScanBeginTick == 0 )
					wdsScanBeginTick = OS_GetTick();
			}
			else {
				OS_Panic("WDS_StartScan failed");
			}
			break;
		case APP_STATE_WDSCOMPLETESCAN:
			// スキャン完了後に入ってくるステート
			
			// 一回のスキャンではビーコンを取れないことが多いので、2秒間ビーコン受信を繰り返す
			now = OS_GetTick();
			if( OS_TicksToMilliSeconds(now - wdsScanBeginTick) < 2000 )
			{
				// 再スキャンのためにスキャン開始ステートに移行
				(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSSCAN, OS_MESSAGE_NOBLOCK );
			}
			else {
				// スキャン終了ステートに移行
				(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSENDSCAN, OS_MESSAGE_NOBLOCK );
			}
			break;
		case APP_STATE_WDSENDSCAN:
			// スキャンを終了させる際に入ってくるステート
			OS_Printf("*** WDS_EndScan\n");
			
			// スキャンを終了させる非同期処理を開始する
			if( WDS_EndScan( WDS_EndScan_CB ) == 0 )
			{
				OS_Printf("WDS_EndScan successed\n");
			}
			else {
				OS_Panic("WDS_EndScan failed");
			}
			break;
		case APP_STATE_WDSCOMPLETEENDSCAN:
			// スキャン終了非同期処理が終わった際に入って来るステート
			OS_Printf("*** WDS_GetApInfoAll\n");
			if( WDS_GetApInfoAll( briefapinfo ) != 0 )
			{
				OS_Panic("WDS_GetApInfoAll failed\n");
			}
			for( i = 0 ; i < WDS_APINFO_MAX ; i++ )
			{
				if( briefapinfo[i].isvalid == TRUE )
				{
					OS_TPrintf("rssi: %d\n", briefapinfo[i].rssi);
					DumpWDSApInfo( &briefapinfo[i].apinfo );
					*(u16 *)0x0500003c = 0x03ff;
				}
			}
			
			// WDSライブラリを終了し、無線ハードの電源を落とす非同期処理を開始する
			OS_Printf("*** WDS_End\n");
			if( WDS_End( WDS_End_CB ) == 0 )
			{
				OS_Printf("WDS_End successed\n");
			}
			else {
				OS_Panic("WDS_End failed");
			}
			break;
		case APP_STATE_WDSCOMPLETEEND:
			// WDSライブラリの解放処理が完了した際に入って来るステート
			SYSM_Free( wdsSysBuf );
			OS_TPrintf("WDS test successfully completed\n");
			return;
		}
	}
}

// アクセスポイント情報のデバッグ表示用関数
static void DumpWDSApInfo( WDSApInfo *apinfo )
{
	int i;
	char buf[256];
	
	OS_TPrintf( "================================\n" );
	// SSID
	MI_CpuCopy8( apinfo->ssid, buf, WDS_SSID_BUF_SIZE) ;
	buf[WDS_SSID_BUF_SIZE] = 0x00;
	OS_TPrintf( "SSID: %s\n", buf );
	
	// APNUM
	MI_CpuCopy8( apinfo->apnum, buf, WDS_APNUM_BUF_SIZE) ;
	buf[WDS_APNUM_BUF_SIZE] = 0x00;
	OS_TPrintf( "APNUM: %s\n", buf );
	
	// CHANNEL
	OS_TPrintf( "channel: %d\n", apinfo->channel );
	
	// ENCRYPTFLAG
	OS_TPrintf( "encryptmethod: %d\n", apinfo->encryptflag);
	
	// WEPKEY
	OS_TPrintf( "WEPKEY: " );
	for( i = 0 ; i < WDS_WEPKEY_BUF_SIZE ; i++ )
	{
		OS_TPrintf( "%02x", apinfo->wepkey[i] );
	}
	OS_TPrintf( "\n" );
	OS_TPrintf( "================================\n" );
}

static void WDS_Initialize_CB(void *arg)
{
#pragma unused(arg)
	OS_TPrintf("WDS_Initialize_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSSCAN, OS_MESSAGE_NOBLOCK );
}

static void WDS_StartScan_CB(void *arg)
{
#pragma unused(arg)
//	OS_TPrintf("WDS_StartScan_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSCOMPLETESCAN, OS_MESSAGE_NOBLOCK );
}

static void WDS_EndScan_CB(void *arg)
{
#pragma unused(arg)
	OS_TPrintf("WDS_EndScan_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSCOMPLETEENDSCAN, OS_MESSAGE_NOBLOCK );
}

static void WDS_End_CB(void *arg)
{
#pragma unused(arg)
	OS_TPrintf("WDS_End_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSCOMPLETEEND, OS_MESSAGE_NOBLOCK );
}
