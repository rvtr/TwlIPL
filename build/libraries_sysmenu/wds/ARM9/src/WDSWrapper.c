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
	OSMutex						mutex;							//!< WDSラッパーが使用するmutex
	
	u8							*wdswork;						//!< WDSが使用するワークエリア
	
	WDSBriefApInfo				briefapinfo[WDS_APINFO_MAX];	//!< WDSラッパーがWDSを使用した結果を格納する領域
	int							briefapinfonum;					//!< WDSラッパーがWDSを使用した結果を格納する領域
	
	WDSWrapperInitializeParam	initparam;						//!< 初期化時パラメータのコピー
	WDSWrapperStateThreadState	state;							//!< WDSラッパーのステート
	OSTick						tickstart;						//!< 各種時間測定用
	
	BOOL						terminate;						//!< 解放開始フラグ
	BOOL						idle;							//!< 間欠スキャン中断フラグ
	BOOL						restart;						//!< 間欠スキャン再開フラグ
	
	OSDeliverArgInfo			deliverinfo;					//!< TWL用アプリ間引数ワークエリア
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
}

//--------------------------------------------------------------------------------
/**	スキャン開始前ウェイトステート関数
		@param	なし
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperBeforeInitState( void )
{
	if( g_wdswrapperwork->restart == TRUE ) {
		// 待っている途中だがすぐに受信を開始
		g_wdswrapperwork->restart = FALSE;

		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
		
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
		// すでにWDSが解放されているので直接ステートを変化させる
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		return;
	}
	
	// 十分長い時間待ったかを確認
	if( g_wdswrapperwork->tickstart + OS_MilliSecondsToTicks( WDSWRAPPER_WAITPERIOD ) < OS_GetTick() ) {
		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
		
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
			@LI 処理成功時は自動的にスキャン開始ステートへ
			@LI 処理失敗時は自動的にWDSラッパー解放ステートへ
*///------------------------------------------------------------------------------
static void WDS_WrapperInitialize_CB( void *arg )
{
	WMCallback *callback = (WMCallback *)arg;
	WDSWrapperCallbackParam param;
	
	// ステート処理と重複処理しないため
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
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
	g_wdswrapperwork->initparam.callback( &param );
	
	// 返り値に基づいてステートを変更
	if( callback->errcode != WM_ERRCODE_SUCCESS )
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	else {
		// スキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_SCAN;
	
		// 連続スキャン開始時間の記録
		g_wdswrapperwork->tickstart = OS_GetTick();
	}
	
	// ステート処理と重複処理しないため
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
}

//--------------------------------------------------------------------------------
/**	スキャン開始ステート関数
		@param	なし
		@return	なし
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
		g_wdswrapperwork->briefapinfonum = 0;
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

	// ステート処理と重複処理しないため
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperStartScan_CB\n");
#endif

	// 外部からの停止等の理由でコールバック待ちステートでない場合には何もしない
	if( g_wdswrapperwork->state != WDSWRAPPER_STATE_WAITSCAN ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("state != WDSWRAPPER_STATE_WAITSCAN\n");
#endif
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
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
		g_wdswrapperwork->initparam.callback( &param );
		
		// ただちにスキャン停止ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
		
		// 最終的にTERMINATEステートに入るよう設定
		g_wdswrapperwork->terminate = TRUE;
		
		// ステート処理と重複処理しないため
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		
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
		g_wdswrapperwork->initparam.callback( &param );
		
		// 十分長い時間スキャンしたのでスキャン中断ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else {
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
			
		// コールバック関数の呼び出し
		g_wdswrapperwork->initparam.callback( &param );

		// ただちにスキャン開始ステートへ
		g_wdswrapperwork->state = WDSWRAPPER_STATE_SCAN;
	}
	
	// ステート処理と重複処理しないため
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
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
			@LI 処理成功時は自動的にWDS解放ステートへ
			@LI 処理失敗時は自動的に再度スキャン停止ステートへ
*///------------------------------------------------------------------------------
static void WDS_WrapperEndScan_CB(void *arg)
{
	WMCallback *callback = (WMCallback *)arg;
	
	// ステート処理と重複処理しないため
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
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
	
	// ステート処理と重複処理しないため
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
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
	
	// ステート処理と重複処理しないため
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_WrapperEnd_CB\n");
#endif
	
	// WDSを終了したタイミングを記録しておく
	g_wdswrapperwork->tickstart = OS_GetTick();
	
	// WDS_End後のステートを各種フラグに基づいて変更
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		
		// コールバックパラメータの設定
		param.callback	= WDSWRAPPER_CALLBACK_STOPSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// コールバック関数の呼び出し
		g_wdswrapperwork->initparam.callback( &param );
	}
	else {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_BEFOREINIT;
	}
	
	
	// ステート処理と重複処理しないため
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
}

//--------------------------------------------------------------------------------
/**	アイドルステート関数
		@param	なし
		@return	なし
*///------------------------------------------------------------------------------
static void WDS_WrapperIdleState( void )
{
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
	else if( g_wdswrapperwork->restart == TRUE )
	{
		g_wdswrapperwork->idle = FALSE;
		g_wdswrapperwork->restart = FALSE;
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
	
	// ステートの初期化
	g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	
	while( 1 ) {
		OS_Sleep(20);
		
		// ステートにより処理を分岐(ステート処理中はmutexによりlockが行われる)
		OS_LockMutex( &g_wdswrapperwork->mutex );
		if( g_wdswrapperwork->state == WDSWRAPPER_STATE_TERMINATE )
			break;
		
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
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
	}
	
	// コールバックパラメータの設定
	param.callback	= WDSWRAPPER_CALLBACK_CLEANUP;
	param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
	
	// コールバック関数の呼び出し
	g_wdswrapperwork->initparam.callback( &param );
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
	
	// mutexの初期化
	OS_InitMutex( &g_wdswrapperwork->mutex );
	
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
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	// 解放処理実行中をチェック
	if( g_wdswrapperwork->terminate == TRUE ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->terminate = TRUE;
	
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
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
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	// ステートがIDLEかBEFOREINITであることを確認
	if( g_wdswrapperwork->state != WDSWRAPPER_STATE_IDLE && g_wdswrapperwork->state != WDSWRAPPER_STATE_BEFOREINIT ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	// 再スタート処理実行中をチェック
	if( g_wdswrapperwork->restart == TRUE ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->restart = TRUE;
	
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
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
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	// 間欠受信停止処理実行中をチェック
	if( g_wdswrapperwork->idle == TRUE ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->idle = TRUE;
	
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	直前に完了した間欠受信で有効な親機ビーコンを受け取ったか確認します
		@param	なし
		@return	WDSWRAPPER_ERRCODE_SUCCESS: 直前に完了した間欠受信で有効な親機ビーコンを受け取った
		@return	WDSWRAPPER_ERRCODE_FAILURE: 直前に完了した間欠受信で有効な親機ビーコンを受け取っていない
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDSラッパーライブラリが初期化されていない
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCheckValidBeacon( void )
{
	int i;
	WDSWrapperErrCode ret = WDSWRAPPER_ERRCODE_FAILURE;
	
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	if( g_wdswrapperwork->briefapinfonum > 0 ) {
		for( i = 0; i < g_wdswrapperwork->briefapinfonum; i++ ) {
			if( g_wdswrapperwork->briefapinfo[i].isvalid == TRUE ) {
				if( g_wdswrapperwork->briefapinfo[i].apinfo.infoflag & WDS_INFOFLAG_NOTIFY ) {
					ret = WDSWRAPPER_ERRCODE_SUCCESS;
					break;
				}
			}
		}
	}
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
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
WDSWrapperErrCode WDS_WrapperSetArgumentParam( void )
{
	int err;
	
	// 初期化済みをチェック
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;

	// TWL用のArgumentシステムを初期化
	OS_InitDeliverArgInfo( &g_wdswrapperwork->deliverinfo, sizeof( g_wdswrapperwork->briefapinfo ) );
	
	// Argument領域に書き込み
	err = OS_SetBinaryToDeliverArg( g_wdswrapperwork->briefapinfo, sizeof( g_wdswrapperwork->briefapinfo ) );
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
