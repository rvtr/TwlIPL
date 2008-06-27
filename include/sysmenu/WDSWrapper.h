//**********************************************************************
//
//	@file		WDSWrapper.h
//	@brief		WDSライブラリのラッパー
//
//	@author		S.Nakata
//	@date		2008/06/21
//	@version	0.0
//
//*********************************************************************/
#ifndef	__WDSWRAPPER_H__
#define	__WDSWRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------
//	Include
//-----------------------------------------------------
#include <sysmenu/WDS.h>

//-----------------------------------------------------
//	Macros
//-----------------------------------------------------
#define WDSWRAPPER_STACKSIZE	4096	///< WDSラッパーが使用するスレッドスタックの推奨サイズ
#define WDSWRAPPER_SCANPERIOD	3000	///< WDSラッパーがスキャンを継続する時間[msec]
#define WDSWRAPPER_WAITPERIOD	3000	///< WDSラッパーがスキャンを中断する時間[msec]

//#define WDSWRAPPER_DEBUGPRINT	///< デバッグ表示フラグ

//-----------------------------------------------------
//	Types of WDS Wrapper Library
//-----------------------------------------------------

/**
	@brief	処理結果をあらわす列挙型
*/
typedef enum WDSWrapperErrCode
{
	WDSWRAPPER_ERRCODE_SUCCESS		= 0,	///< 処理成功
	WDSWRAPPER_ERRCODE_FAILURE		= 1,	///< 処理失敗
	WDSWRAPPER_ERRCODE_OPERATING	= 2,	///< 処理実行中
	WDSWRAPPER_ERRCODE_INITIALIZED	= 3,	///< WDSラッパーライブラリ初期化済み
	WDSWRAPPER_ERRCODE_UNINITIALIZED= 4		///< WDSラッパーライブラリ未初期化
} WDSWrapperErrCode;

/**
	@brief	コールバック発生要因をあらわす列挙型
*/
typedef enum WDSWrapperCallback
{
	WDSWRAPPER_CALLBACK_INITIALIZE	= 0,	//!< WDS_WrapperInitialize関数によるコールバック
	WDSWRAPPER_CALLBACK_CLEANUP		= 1,	//!< WDS_WrapperCleanup関数によるコールバック
	WDSWRAPPER_CALLBACK_STARTSCAN	= 2,	//!< WDS_WrapperStartScan関数によるコールバック
	WDSWRAPPER_CALLBACK_STARTSCAN2	= 3,	//!< WDS_WrapperStartScan関数によるコールバック(間欠スキャン1回終了)
	WDSWRAPPER_CALLBACK_STOPSCAN	= 4		//!< WDS_WrapperStopScan関数によるコールバック
} WDSWrapperCallback;

/**
	@brief	WDSラッパーののステートをあらわす列挙型
*/
typedef enum WDSWrapperStateThreadState {
	WDSWRAPPER_STATE_BEFOREINIT,			//!< 初期化開始待ちステート
	WDSWRAPPER_STATE_INIT,				//!< 初期化開始ステート
	WDSWRAPPER_STATE_WAITINIT,			//!< 初期化完了待ちステート
	WDSWRAPPER_STATE_SCAN,				//!< スキャン開始ステート
	WDSWRAPPER_STATE_WAITSCAN,			//!< スキャン完了待ちステート
	WDSWRAPPER_STATE_ENDSCAN,			//!< スキャン中断ステート
	WDSWRAPPER_STATE_WAITENDSCAN,		//!< スキャン中断完了待ちステート
	WDSWRAPPER_STATE_END,				//!< WDS解放開始ステート
	WDSWRAPPER_STATE_WAITEND,			//!< WDS解放完了待ちステート
	WDSWRAPPER_STATE_IDLE,				//!< アイドルステート
	WDSWRAPPER_STATE_TERMINATE			//!< WDSラッパー解放ステート
} WDSWrapperStateThreadState;

/**
	@brief	WDSWrapperライブラリが使用するコールバック関数型
	@note
		・引数argは必ずWDSWrapperCallback型変数へのポインタ
*/
typedef void ( *WDSWrapperCallbackFunc )( void *arg );

/**
	@brief	WDSラッパー内部で使用するアロケーター型
*/
typedef void *( *WDSWrapperAlloc )( u32 size );

/**
	@brief	WDSラッパー内部で使用するアロケーター型
*/
typedef void ( *WDSWrapperFree )( void *ptr );

//-----------------------------------------------------
//	Structs of WDS Wrapper Library
//-----------------------------------------------------

/**
	@brief	WDS_WrapperInitialize関数に与える初期化パラメータ
*/
typedef struct WDSWrapperInitializeParam
{
	u32						threadprio;	///< WDSコントロールスレッドの優先度
	u16						dmano;		///< WMライブラリが使用するDMA番号
	
	WDSWrapperCallbackFunc	callback;	///< WDSコントロールスレッドから呼び出されるコールバック関数へのポインタ
	WDSWrapperAlloc			alloc;		///< WDSラッパーが使用するアロケータ
	WDSWrapperFree			free;		///< WDSラッパーが使用するアロケータ
} WDSWrapperInitializeParam;

/**
	@brief	WDSラッパーが呼び出すコールバック関数に与えられる引数用の構造体
*/
typedef struct WDSWrapperCallbackParam
{
	WDSWrapperCallback	callback;	///< コールバック発生要因
	WDSWrapperErrCode	errcode;	///< エラーコード
} WDSWrapperCallbackParam;

//-----------------------------------------------------
//	Prototypes of WDS Wrapper Library
//-----------------------------------------------------

WDSWrapperErrCode WDS_WrapperInitialize( WDSWrapperInitializeParam param );
WDSWrapperErrCode WDS_WrapperCleanup( void );
WDSWrapperErrCode WDS_WrapperStartScan( void );
WDSWrapperErrCode WDS_WrapperStopScan( void );
WDSWrapperErrCode WDS_WrapperCheckValidBeacon( void );
WDSWrapperErrCode WDS_WrapperSetArgumentParam( void );
WDSWrapperErrCode WDS_WrapperCheckThreadRunning( void );
OSThread *WDS_WrapperGetOSThread( void );

#ifdef __cplusplus
}
#endif

#endif	//EOF __WDSWRAPPER_H__
