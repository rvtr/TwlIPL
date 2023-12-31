/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     WDS.c

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

//**********************************************************************
/**
//	@file		WDSWrapper.c
//	@brief		WDSライブラリのラッパー
//
//	@author		S.Nakata
//	@date		2008/06/24
//	@version	01.00
//
***********************************************************************/
#include <sysmenu/WDSWrapper.h>
#ifdef WDS_WITHDWC
#include <dwc.h>
#endif

//-----------------------------------------------------
//	Structs
//-----------------------------------------------------

/**
	@brief	WDSラッパーのワーク領域
*/
typedef struct WDSWrapperWork
{
	u8							*stack;							//!< WDSラッパーが使用するスタック
	OSThread					thread;							//!< WDSラッパーが使用するスレッド構造体
	
	u8							*wdswork;						//!< WDSが使用するワークエリア
	
	WDSBriefApInfo				briefapinfo[WDS_APINFO_MAX];			//!< WDSラッパーがWDSを使用した結果を格納する領域
	WDSBriefApInfo				briefapinfo_previous[WDS_APINFO_MAX];	//!< 直前のWDSを使用した結果を格納する領域
	int							briefapinfonum;							//!< WDSラッパーがWDSを使用した結果を格納する領域
	int							briefapinfonum_previous;				//!< 直前のWDSを使用した結果を格納する領域
	
	WDSWrapperInitializeParam	initparam;						//!< 初期化時パラメータのコピー
	WDSWrapperStateThreadState	state;							//!< WDSラッパーのステート
	OSTick						tickstart;						//!< 各種時間測定用
	
	BOOL						terminate;						//!< 解放開始フラグ
	BOOL						idle;							//!< 間欠スキャン中断フラグ
	BOOL						restart;						//!< 間欠スキャン再開フラグ
	
#ifdef SDK_TWL
	OSDeliverArgInfo			deliverinfo;					//!< TWL用アプリ間引数ワークエリア
#endif
	
	BOOL						callingback;					//!< コールバック関数呼び出し中はTRUE
} WDSWrapperWork;

//-----------------------------------------------------
//	Variables
//-----------------------------------------------------
static WDSWrapperWork	*g_wdswrapperwork	= NULL;

//-----------------------------------------------------
//	Internal Functions
//-----------------------------------------------------
static void WDS_WrapperInitialize_CB( void *arg );
static void WDS_WrapperStartScan_CB( void *arg );
static void WDS_WrapperEndScan_CB(void *arg);
static void WDS_WrapperEnd_CB( void *arg );
static void WDS_WrapperCallUserCallback( void *arg );

//--------------------------------------------------------------------------------
/**	ユーザー指定のコールバック関数を呼び出すユーティリティ関数
		@param arg コールバック関数に与えるパラメータ
		@return なし
*///------------------------------------------------------------------------------
static void WDS_WrapperCallUserCallback( void *arg )
{
	g_wdswrapperwork->callingback = TRUE;
	g_wdswrapperwork->initparam.callback( arg );
	g_wdswrapperwork->callingback = FALSE;
}

//--------------------------------------------------------------------------------
/**	アクセスポイント情報のデバッグ用表示関数
		@param apinfo デバッグ表示を行うWDSApInfoへのポインタ
		@return なし
*///------------------------------------------------------------------------------
static void DumpWDSApInfo( WDSApInfo *apinfo )
{
	int i;
	char buf[256];
	
	// SSID
	MI_CpuCopy8( apinfo->ssid, buf, WDS_SSID_BUF_SIZE) ;
	buf[WDS_SSID_BUF_SIZE] = 0x00;
	OS_TPrintf( "SSID: %s\n", buf );
	
	// APNUM
	MI_CpuCopy8( apinfo->apnum, buf, WDS_APNUM_BUF_SIZE) ;
	buf[WDS_APNUM_BUF_SIZE] = 0x00;
	OS_TPrintf( "APNUM: %s\n", buf );
	
	// HOTSPOTID
	OS_TPrintf( "hotspotid: %d\n", apinfo->hotspotid );
	
	// HOTSPOTNAME
	OS_TPrintf( "hotspotname: " );
	for( i = 0 ; i < WDS_HOTSPOTNAME_BUF_SIZE ; i++ )
	{
		OS_TPrintf( "%02x", apinfo->hotspotname[i] );
	}
	OS_TPrintf( "\n" );
	
	// WEPKEY
	OS_TPrintf( "wepkey: " );
	for( i = 0 ; i < WDS_WEPKEY_BUF_SIZE ; i++ )
	{
		OS_TPrintf( "%02x", apinfo->wepkey[i] );
	}
	OS_TPrintf( "\n" );
	
	// CHANNEL
	OS_TPrintf( "channel: %d\n", apinfo->channel );
	
	// ENCRYPTFLAG
	OS_TPrintf( "encryptmethod: %d\n", apinfo->encryptflag);
	
	// INFOFLAG
	OS_TPrintf( "infoflag: %02d\n", apinfo->infoflag);
	
	// RESERVE
	OS_TPrintf( "reserve: " );
	for( i = 0 ; i < 5 ; i++ )
	{
		OS_TPrintf( "%02x", apinfo->reserve[i] );
	}
	OS_TPrintf( "\n" );
	
	// MTU
	OS_TPrintf( "mtu: %d\n", apinfo->mtu);
	
	// CRC
	OS_TPrintf( "crc: %04x\n", apinfo->crc);
}

//--------------------------------------------------------------------------------
/**	スキャン開始前ウェイトステート関数
		@param	なし
		@return	なし
	@note
		@LI このステートに入っている場合、WMからのコールバックが発生する可能性はない
		@LI IF関数からのフラグ変化がイレギュラーな存在としてあるが
		@LI Mutexのロックを行わなくても、処理中のフラグ変化による動作異常はない
*///------------------------------------------------------------------------------
static void WDS_WrapperBeforeInitState( void )
{
	if( g_wdswrapperwork->restart == TRUE ) {
		// 待っている途中だがすぐに受信を開始
		g_wdswrapperwork->restart = FALSE;

		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
		
		// 現スキャン状態のスキャン結果をクリア
		g_wdswrapperwork->briefapinfonum = 0;
		
		// スキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
		return;
	}
	else if( g_wdswrapperwork->terminate == TRUE ) {
		// すでにWDSが解放されているので直接ステートを変化させる
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
		return;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		WDSWrapperCallbackParam param;
		
		// すでにWDSが解放されているのでコールバックを呼び出し、直接ステートを変化させる
		g_wdswrapperwork->idle = FALSE;
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STOPSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// コールバック関数の呼び出し(このコールバックのスレッド優先度はWMスレッドの優先度ではない)
		WDS_WrapperCallUserCallback( &param );
		
		return;
	}
	
	// 十分長い時間待ったかを確認
	if( g_wdswrapperwork->tickstart + OS_MilliSecondsToTicks( WDSWRAPPER_WAITPERIOD ) < OS_GetTick() ) {
		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
		
		// 現スキャン状態のスキャン結果をクリア
		g_wdswrapperwork->briefapinfonum = 0;
		
		// スキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	}
}

//--------------------------------------------------------------------------------
/**	初期化開始ステート関数
		@param	なし
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperInitState( void )
{
	WMErrCode err;
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperInitState\n");
#endif
	
	// 初期化完了待ちステートをあらかじめ設定しておく
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITINIT;
	
	err = (WMErrCode)WDS_Initialize( g_wdswrapperwork->wdswork, WDS_WrapperInitialize_CB, 0 );
	if( err == WM_ERRCODE_SUCCESS || err == WM_ERRCODE_OPERATING ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_Initialize successed\n");
#endif
	}
	else {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_Initialize failed\n");
#endif
		// 初期化開始に失敗したのでWDSラッパー解放ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
}

//--------------------------------------------------------------------------------
/**	初期化コールバック関数
		@param	arg 非同期処理の結果を格納するWMCallback型変数へのポインタ
		@return	なし
		@note
			@LI この関数が呼ばれる際のステートは常にWDSWRAPPER_STATE_WAITINIT
			@LI 何の処理もメインスレッドは行っていない
			@LI 処理成功時は自動的にスキャン開始ステートへ
			@LI 処理失敗時は自動的にWDSラッパー解放ステートへ
*///------------------------------------------------------------------------------
static void WDS_WrapperInitialize_CB( void *arg )
{
	WMCallback *callback = (WMCallback *)arg;
	WDSWrapperCallbackParam param;
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_Initialize_CB\n");
#endif
	
	// 返り値に基づいてコールバックパラメータを設定
	param.callback	= WDSWRAPPER_CALLBACK_INITIALIZE;
	if( callback->errcode != WM_ERRCODE_SUCCESS )
		param.errcode	= WDSWRAPPER_ERRCODE_FAILURE;
	else
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
	
	// コールバック関数の呼び出し
	WDS_WrapperCallUserCallback( &param );
	
	// 返り値に基づいてステートを変更
	if( callback->errcode != WM_ERRCODE_SUCCESS )
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	else {
		// スキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_SCAN;
	
		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
	}
}

//--------------------------------------------------------------------------------
/**	スキャン開始ステート関数
		@param	なし
		@return	なし
		@note
			@LI この関数が呼ばれる際は、WDSは初期化済みだがWDS_StartScan要因以外でコールバック関数は呼ばれない
*///------------------------------------------------------------------------------
static void WDS_WrapperScanState( void )
{
	WMErrCode err;
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf( "*** WDS_WrapperScanState\n" );
#endif
	
	// スキャン完了待ちステートへあらかじめ移行しておく
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITSCAN;
	
	err = (WMErrCode)WDS_StartScan( WDS_WrapperStartScan_CB );
	if( err == WM_ERRCODE_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_StartScan successed\n");
#endif
	}
	else {
		// スキャン開始に失敗したのでWDS解放開始ステートへ
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf( "WDS_StartScan failed\n" );
#endif
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
}

//--------------------------------------------------------------------------------
/**	スキャン完了待ちステート関数
		@param	なし
		@return	なし
		@note
			@LI このステートの間は外部IFからのキャンセルを受け付ける
			@LI この処理の中でフラグが変化しても、動作は結局同じなのでMutexロックはしない
*///------------------------------------------------------------------------------
static void WDS_WrapperWaitScanState( void )
{
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
}

//--------------------------------------------------------------------------------
/**	スキャン開始コールバック関数
		@param	arg 非同期処理の結果を格納するWMCallback型変数へのポインタ
		@return なし
		@note
			@LI ユーザー中断が原因で、ステートがWDSWRAPPER_STATE_WAITSCANではない場合がある
			@LI 処理成功時はスキャン時間に基づいてIDLEかスキャン前ウェイトステートへ
			@LI 処理失敗時はスキャン停止後WDSラッパー解放ステートへ
*///------------------------------------------------------------------------------
static void WDS_WrapperStartScan_CB( void *arg )
{
#pragma unused( arg )
#ifdef WDSWRAPPER_DEBUGPRINT
	int i;
#endif
	WDSWrapperCallbackParam param;

#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperStartScan_CB\n");
#endif
	
	// ライブラリがすでに解放済みである場合に備える
	if( g_wdswrapperwork == NULL )
		return;

	// 外部からの停止等の理由でコールバック待ちステートでない場合には何もしない
	if( g_wdswrapperwork->state != WDSWRAPPER_STATE_WAITSCAN ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("state != WDSWRAPPER_STATE_WAITSCAN\n");
#endif
		return;
	}
	
	// スキャン結果を取得する
	if( WDS_GetApInfoAll( g_wdswrapperwork->briefapinfo ) != 0 ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf( "WDS_GetApInfoAll failed\n" );
#endif
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_FAILURE;
		
		// コールバック関数の呼び出し
		WDS_WrapperCallUserCallback( &param );
		
		// ただちにスキャン停止ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
		
		// 最終的にTERMINATEステートに入るよう設定
		g_wdswrapperwork->terminate = TRUE;
		
		return;
	}
	else {
		// ビーコン数を格納
		g_wdswrapperwork->briefapinfonum = WDS_GetApInfoNum();
	}

	
	if( g_wdswrapperwork->tickstart + OS_MilliSecondsToTicks( WDSWRAPPER_SCANPERIOD ) < OS_GetTick() ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("*** WDS_WrapperCompleteScanState: Scanned more than %u [ms]\n", WDSWRAPPER_SCANPERIOD );
#endif

#ifdef WDSWRAPPER_DEBUGPRINT
		// スキャン結果を取得したのでデバッグ表示
		for( i = 0 ; i < WDS_APINFO_MAX ; i++ ) {
			if( g_wdswrapperwork->briefapinfo[i].isvalid == TRUE ) {
				OS_TPrintf( "================================\n" );
				OS_TPrintf( "rssi: %d\n", g_wdswrapperwork->briefapinfo[i].rssi );
				DumpWDSApInfo( &( g_wdswrapperwork->briefapinfo[i].apinfo ) );
				OS_TPrintf( "================================\n" );
			}
		}
#endif
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN2;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// コールバック関数の呼び出し
		WDS_WrapperCallUserCallback( &param );
		
		// これまでのスキャン結果を直前のスキャン結果を保持しておく場所にコピー
		MI_CpuCopy8( g_wdswrapperwork->briefapinfo, g_wdswrapperwork->briefapinfo_previous, sizeof(g_wdswrapperwork->briefapinfo) );
		g_wdswrapperwork->briefapinfonum_previous = g_wdswrapperwork->briefapinfonum;
			
		// 十分長い時間スキャンしたのでスキャン中断ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else {
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
			
		// コールバック関数の呼び出し
		WDS_WrapperCallUserCallback( &param );

		// ただちにスキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_SCAN;
	}
}

//--------------------------------------------------------------------------------
/**	スキャン停止開始ステート関数
		@param	なし
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperEndScanState( void )
{
	WMErrCode err;
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf( "*** WDS_WrapperEndScanState\n" );
#endif
	
	// スキャン停止待ちステートへあらかじめ移行しておく
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITENDSCAN;
	
	err = (WMErrCode)WDS_EndScan( WDS_WrapperEndScan_CB );
	if( err == WM_ERRCODE_SUCCESS || err == WM_ERRCODE_ILLEGAL_STATE ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan successed\n");
#endif
		// WDS解放ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
	else if( err == WM_ERRCODE_OPERATING ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan asynchronously successed\n");
#endif
	}
	else
	{
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan failed\n");
#endif
		// 再度スキャン停止を試みる
		// (これは処理的に無限ループに入る可能性があるのでまずいが、そもそもスキャンが止まっていないとWM_Endもできないので不可避)
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
}

//--------------------------------------------------------------------------------
/**	スキャン停止コールバック関数
		@param	arg 非同期処理の結果を格納するWMCallback型変数へのポインタ
		@return	なし
		@note
			@LI このコールバックが呼ばれる際は、ステートは必ずWDSWRAPPER_STATE_WAITENDSCAN
			@LI 処理成功時は自動的にWDS解放ステートへ
			@LI 処理失敗時は自動的に再度スキャン停止ステートへ
*///------------------------------------------------------------------------------
static void WDS_WrapperEndScan_CB(void *arg)
{
	WMCallback *callback = (WMCallback *)arg;
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_WrapperEndScan_CB\n");
#endif
	
	if( callback->errcode != WM_ERRCODE_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan success\n");
#endif
		// 再度スキャン停止を試みる
		// (これは処理的に無限ループに入る可能性があるのでまずいが、そもそもスキャンが止まっていないとWM_Endもできないので不可避)
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan success\n");
#endif
		// スキャン停止完了したら自動的にWDS解放ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
}

//--------------------------------------------------------------------------------
/**	解放開始ステート関数
		@param	なし
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperEndState( void )
{
	WMErrCode err;
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperEndState\n");
#endif
	
	
	// 解放待ちステートへあらかじめ移行しておく
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITEND;
	err = (WMErrCode)WDS_End( WDS_WrapperEnd_CB );
	
	if( err == WM_ERRCODE_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_End successed\n");
#endif
	}
	else if( err == WM_ERRCODE_OPERATING ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_End asynchronously successed\n");
#endif
	}
	else {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_End failed\n");
#endif
		// 引き続き解放を試みる
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
}

//--------------------------------------------------------------------------------
/**	解放開始コールバック関数
		@param	arg 非同期処理の結果を格納するWMCallback型変数へのポインタ
		@return	なし
		@note
			@LI WDS_Endは常に成功するため、フラグに基づいてIDLEまたは初期化前ウェイトまたはライブラリ解放ステートへ
*///------------------------------------------------------------------------------
static void WDS_WrapperEnd_CB( void *arg )
{
#pragma unused(arg)
	WDSWrapperCallbackParam	param;
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_WrapperEnd_CB\n");
#endif
	
	// WDSを終了したタイミングを記録しておく
	g_wdswrapperwork->tickstart = OS_GetTick();
	
	// WDS_End後のステートを各種フラグに基づいて変更
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->terminate = FALSE;
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		g_wdswrapperwork->idle = FALSE;
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STOPSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// コールバック関数の呼び出し
		WDS_WrapperCallUserCallback( &param );
	}
	else {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_BEFOREINIT;
	}
}

//--------------------------------------------------------------------------------
/**	アイドルステート関数
		@param	なし
		@return	なし
			@LI このステートの間は外部IFからのスキャン開始とライブラリ解放を受け付ける
			@LI この処理の中でフラグが変化しても、動作は結局同じなのでMutexロックはしない
*///------------------------------------------------------------------------------
static void WDS_WrapperIdleState( void )
{
	WDSWrapperCallbackParam	param;
	
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		// すでにIDLE状態のため、成功コールバックを返す
		g_wdswrapperwork->idle = FALSE;
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STOPSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// コールバック関数の呼び出し
		WDS_WrapperCallUserCallback( &param );
	}
	else if( g_wdswrapperwork->restart == TRUE )
	{
		g_wdswrapperwork->idle = FALSE;
		g_wdswrapperwork->restart = FALSE;

		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
		
		// 現スキャン状態のスキャン結果をクリア
		g_wdswrapperwork->briefapinfonum = 0;
		
		// スキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	}
}

//--------------------------------------------------------------------------------
/**	WDSラッパーライブラリが内部で実行するスレッド関数
		@param	arg 常にNULL
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperThreadFunc( void *arg )
{
#pragma unused( arg )
	WDSWrapperCallbackParam param;
	
	// 間欠受信のビーコン数を初期化
	g_wdswrapperwork->briefapinfonum = 0;
	
	// スキャン開始ステートへ
	g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	
	while( 1 ) {
		OS_Sleep( 20 );
		
		// ステートにより処理を分岐(ステート処理中はmutexによりlockが行われる)
		if( g_wdswrapperwork->state == WDSWRAPPER_STATE_TERMINATE )
			break;
		OS_Sleep( 20 );
		
		switch( g_wdswrapperwork->state ) {
		case WDSWRAPPER_STATE_BEFOREINIT:		WDS_WrapperBeforeInitState();	break;
		case WDSWRAPPER_STATE_INIT:				WDS_WrapperInitState();			break;
		case WDSWRAPPER_STATE_WAITINIT:			break;
		case WDSWRAPPER_STATE_SCAN:				WDS_WrapperScanState();			break;
		case WDSWRAPPER_STATE_WAITSCAN:			WDS_WrapperWaitScanState();		break;
		case WDSWRAPPER_STATE_ENDSCAN:			WDS_WrapperEndScanState();		break;
		case WDSWRAPPER_STATE_WAITENDSCAN:		break;
		case WDSWRAPPER_STATE_END:				WDS_WrapperEndState();			break;
		case WDSWRAPPER_STATE_WAITEND:			break;
		case WDSWRAPPER_STATE_IDLE:				WDS_WrapperIdleState();			break;
		case WDSWRAPPER_STATE_TERMINATE:		break;
		}
	}
	
	// スキャン停止要求を受けたが、その後WDSWrapper解放要求を受け、スキャン停止コールバックが発生せずにここにきた場合の対策コード
	if( g_wdswrapperwork->idle == TRUE ) {
		g_wdswrapperwork->idle = FALSE;
		
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STOPSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// コールバック関数の呼び出し
		WDS_WrapperCallUserCallback( &param );
	}
	
	// コールバックパラメータの設定
	param.callback	= WDSWRAPPER_CALLBACK_CLEANUP;
	param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
	
	// コールバック関数の呼び出し
	WDS_WrapperCallUserCallback( &param );
}

//--------------------------------------------------------------------------------
/**	WDSコントロールスレッドデストラクタ
		@param	arg 常にNULL
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperThreadDestructor( void *arg )
{
#pragma unused( arg )
	if( g_wdswrapperwork != NULL ) {
		if( g_wdswrapperwork->stack != NULL ) {
			g_wdswrapperwork->initparam.free( g_wdswrapperwork->stack );
			g_wdswrapperwork->stack = NULL;
		}
		if( g_wdswrapperwork->wdswork != NULL ) {
			g_wdswrapperwork->initparam.free( g_wdswrapperwork->wdswork );
			g_wdswrapperwork->wdswork = NULL;
		}
		g_wdswrapperwork->initparam.free( g_wdswrapperwork );
		g_wdswrapperwork = NULL;
	}
}

//-----------------------------------------------------
//	External Functions
//-----------------------------------------------------

//--------------------------------------------------------------------------------
/**
	WDSラッパーライブラリ初期化の非同期処理を開始します。<BR>
	初期化処理完了時にコールバックが発生します。<BR>
	コールバック関数でエラー通知を受け取った場合は、ライブラリが自動的に解放されるのを待ってください。
		@param	param WDSラッパー初期化パラメータ
		@return	WDSWRAPPER_ERRCODE_SUCCESS: 初期化処理を開始
		@return	WDSWRAPPER_ERRCODE_INITIALIZED: WDSラッパーライブラリは初期化済み
		@return	WDSWRAPPER_ERRCODE_FAILURE: 初期化に失敗
	@note
		・WDSコントロールスレッドは生成直後から間欠スキャンを実行しています
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperInitialize( WDSWrapperInitializeParam param )
{
	// 初期化済みをチェック
	if( g_wdswrapperwork != NULL )
		return WDSWRAPPER_ERRCODE_INITIALIZED;
	
	// パラメータのサニティチェック
	if( param.callback == NULL || param.alloc == NULL || param.free == NULL )
		return WDSWRAPPER_ERRCODE_FAILURE;
	
	// ワークエリアを初期化
	g_wdswrapperwork = param.alloc( sizeof( WDSWrapperWork ) );
	if( g_wdswrapperwork == NULL )
		goto FAILURE;
	MI_CpuClear8( g_wdswrapperwork, sizeof( WDSWrapperWork ) );
	
	// スタック領域・WDSワークエリアの初期化
	g_wdswrapperwork->stack = param.alloc( WDSWRAPPER_STACKSIZE );
	if( g_wdswrapperwork->stack == NULL )
		goto FAILURE;
	g_wdswrapperwork->wdswork = param.alloc( WDS_GetWorkAreaSize() );
	if( g_wdswrapperwork->wdswork == NULL )
		goto FAILURE;
	
	// パラメータのコピー
	g_wdswrapperwork->initparam = param;
	
	// スレッドの生成
	OS_CreateThread( &g_wdswrapperwork->thread,
					WDS_WrapperThreadFunc,
					NULL,
					g_wdswrapperwork->stack + WDSWRAPPER_STACKSIZE,
					WDSWRAPPER_STACKSIZE,
					g_wdswrapperwork->initparam.threadprio
				   );
	
	// ワークエリア解放用のスレッドデストラクタを設定
	OS_SetThreadDestructor( &g_wdswrapperwork->thread, WDS_WrapperThreadDestructor );
	
	// スレッドの実行
	OS_WakeupThreadDirect( &g_wdswrapperwork->thread );
	
	// 初期化成功
	return WDSWRAPPER_ERRCODE_SUCCESS;

FAILURE:
	if( g_wdswrapperwork != NULL ) {
		if( g_wdswrapperwork->stack != NULL ) {
			param.free( g_wdswrapperwork->stack );
			g_wdswrapperwork->stack = NULL;
		}
		if( g_wdswrapperwork->wdswork != NULL ) {
			param.free( g_wdswrapperwork->wdswork );
			g_wdswrapperwork->wdswork = NULL;
		}
		param.free( g_wdswrapperwork );
		g_wdswrapperwork = NULL;
	}
	return WDSWRAPPER_ERRCODE_FAILURE;
}

//--------------------------------------------------------------------------------
/**
	WDSコントロールスレッドを停止し、WDSラッパーライブラリを解放します。<BR>
	解放処理完了の直前にコールバックが発生します。<BR>
	コールバック関数にエラーが通知されることはありません。
		@param	なし
		@return	WDSWRAPPER_ERRCODE_SUCCESS WDSラッパー解放を開始
		@return	WDSWRAPPER_ERRCODE_OPERATING WDSラッパー解放を実行中
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED WDSラッパーライブラリが初期化されていない
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCleanup( void )
{
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	// 解放処理実行中をチェック
	if( g_wdswrapperwork->terminate == TRUE ) {
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->terminate = TRUE;
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDSコントロールスレッドのステートを間欠受信状態に変更します。<BR>
	ステートの変更に成功すると、以後一回の間欠受信が完了するたびにコールバックが呼び出されます。<BR>
	コールバック関数でエラー通知を受け取った場合は、ライブラリが自動的に解放されるのを待ってください。
		@return	WDSWRAPPER_ERRCODE_SUCCESS: ステート変更に成功(コールバック発生を待ってください)
		@return	WDSWRAPPER_ERRCODE_FAILURE: 間欠受信に移行できるステートでない(現在のステートが維持されます)
   		@return	WDSWRAPPER_ERRCIDE_OPERATING: すでにステートが間欠受信状態
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDSラッパーライブラリが初期化されていない
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperStartScan( void )
{
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	// ステートがIDLEかBEFOREINITであることを確認
	if( g_wdswrapperwork->state != WDSWRAPPER_STATE_IDLE && g_wdswrapperwork->state != WDSWRAPPER_STATE_BEFOREINIT ) {
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	// 再スタート処理実行中をチェック
	if( g_wdswrapperwork->restart == TRUE ) {
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->restart = TRUE;
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDSコントロールスレッドのステートを間欠受信停止状態に設定します。<BR>
	間欠受信が実際に停止した際にコールバックが呼び出されます。<BR>
	コールバックでエラーが通知されることはありません。
		@return	WDSWRAPPER_ERRCODE_SUCCESS: ステート変更に成功
		@return	WDSWRAPPER_ERRCIDE_OPERATING: すでにステートが間欠受信停止状態
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDSラッパーライブラリが初期化されていない
	@note
		ステート変更が行われてから間欠受信が停止するまでには20ms程度の時間がかかります
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperStopScan( void )
{
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	// 間欠受信停止処理実行中をチェック
	if( g_wdswrapperwork->idle == TRUE ) {
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->idle = TRUE;
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	直前に完了した間欠受信で有効な親機ビーコンを受け取ったか確認します
		@param	なし
		@return	WDSWRAPPER_ERRCODE_SUCCESS: 直前に完了した間欠受信で有効な親機ビーコンを受け取った
		@return	WDSWRAPPER_ERRCODE_FAILURE: 直前に完了した間欠受信で有効な親機ビーコンを受け取っていない
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDSラッパーライブラリが初期化されていない
		@note	ライブラリ初期化時に指定したコールバック関数の中でのみ呼び出して下さい
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCheckValidBeacon( void )
{
	int i;
	WDSWrapperErrCode ret = WDSWRAPPER_ERRCODE_FAILURE;
	
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	// コールバック関数からの呼び出しかを確認
	if( g_wdswrapperwork->callingback != TRUE )
		return WDSWRAPPER_ERRCODE_FAILURE;
	
	if( g_wdswrapperwork->briefapinfonum_previous > 0 ) {
		// 直前のWDSの結果が生きていればまずそれをチェックする
		for( i = 0; i < g_wdswrapperwork->briefapinfonum_previous; i++ ) {
			if( g_wdswrapperwork->briefapinfo_previous[i].isvalid == TRUE &&
				g_wdswrapperwork->briefapinfo_previous[i].apinfo.infoflag & WDS_INFOFLAG_NOTIFY ) {
				ret = WDSWRAPPER_ERRCODE_SUCCESS;
				break;
			}
		}
	}
	if( g_wdswrapperwork->briefapinfonum > 0 ) {
		// 現在スキャン中のデータもチェックする
		for( i = 0; i < g_wdswrapperwork->briefapinfonum; i++ ) {
			if( g_wdswrapperwork->briefapinfo[i].isvalid == TRUE &&
				g_wdswrapperwork->briefapinfo[i].apinfo.infoflag & WDS_INFOFLAG_NOTIFY ) {
				ret = WDSWRAPPER_ERRCODE_SUCCESS;
				break;
			}
		}
	}
	
	return ret;
}

//--------------------------------------------------------------------------------
/**
	直前に完了した間欠受信で受け取ったビーコン情報をArgument領域にセットします
		@param	なし
		@return	WDSWRAPPER_ERRCODE_SUCCESS: 親機ビーコン情報がArgument領域にセットされた
		@return	WDSWRAPPER_ERRCODE_FAILURE: 親機ビーコン情報をArgument領域にセットできなかった
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDSラッパーライブラリが初期化されていない
*///------------------------------------------------------------------------------
#ifdef SDK_TWL
WDSWrapperErrCode WDS_WrapperSetArgumentParam( void )
{
	int err;
	
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;

	// TWL用のArgumentシステムに取得したビーコンデータを書き込む
	if( g_wdswrapperwork->briefapinfonum_previous <= 0 && g_wdswrapperwork->briefapinfonum > 0) {
		// 直前のスキャンがビーコンなしだが、現在のスキャン結果にビーコンがある場合は現在スキャン中の情報を使う
		g_wdswrapperwork->briefapinfonum_previous = g_wdswrapperwork->briefapinfonum;
		MI_CpuCopy8( g_wdswrapperwork->briefapinfo, g_wdswrapperwork->briefapinfo_previous, sizeof( g_wdswrapperwork->briefapinfo ) );
	}
	OS_InitDeliverArgInfo( &g_wdswrapperwork->deliverinfo, sizeof( g_wdswrapperwork->briefapinfo_previous ) );
	err = OS_SetBinaryToDeliverArg( g_wdswrapperwork->briefapinfo_previous, sizeof( g_wdswrapperwork->briefapinfo_previous ) );
	
	if( err != OS_DELIVER_ARG_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_TPrintf( "WDS_WrapperSetArgumentParam: failed %d\n", err );
#endif
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	err = OS_EncodeDeliverArg();
	if( err != OS_DELIVER_ARG_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_TPrintf( "OS_EncodeDeliverArg: failed %d\n", err );
#endif
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf( "WDS_WrapperSetArgumentParam: success\n" );
#endif
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}
#endif

//--------------------------------------------------------------------------------
/**
	WDSコントロールスレッドが実行中か確認します
		@param	なし
		@return	WDSWRAPPER_ERRCODE_SUCCESS: WDSコントロールスレッドが実行中
		@return	WDSWRAPPER_ERRCODE_FAILURE: WDSコントロールスレッドは停止している
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDSラッパーライブラリが初期化されていない
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCheckThreadRunning( void )
{
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	if( OS_IsThreadTerminated( &g_wdswrapperwork->thread ) == TRUE )
		return WDSWRAPPER_ERRCODE_FAILURE;
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDSコントロールスレッドが使用しているOSThreadへのポインタを得ます
	@param	なし
	@return	NULL: WDSラッパーライブラリが初期化されていない
	@return	NULL以外: 実行中・あるいは停止中のOSThreadへのポインタ
*///------------------------------------------------------------------------------
OSThread *WDS_WrapperGetOSThread( void )
{
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return NULL;
	
	return &g_wdswrapperwork->thread;
}
