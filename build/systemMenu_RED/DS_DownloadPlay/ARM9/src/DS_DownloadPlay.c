/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_DownloadPlay.c

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

// ※※※ mb_private.hのPRINT_DEBUGを"1"にしていると、ダウンロード後に正常にブートできなくなっているので、注意。

#include <twl.h>
#include <nitro/mb.h>		// ここは<nitro/mb.h>とするのはOK?
#include <sysmenu.h>
#include "mb_child.h"
#include "mb_private.h"


#include "misc.h"
#include "DS_Setting.h"
#include "DS_DownloadPlay.h"

// define data------------------------------------------
#define PARENT_VIEW_NUM						4						// 親機情報リストの一画面表示数
#define PLIST_X								1						// 親機情報リストの表示X位置
#define PLIST_Y								4						// 親機情報リストの表示Y位置
#define	MB_DMA_NO							2						// マルチブート用DMA番号
#define PRG_NEXT_SEQ_QUE_NUM				4						// 次のプログラムシーケンスを格納するキューの数
#define MY_GGID								0x00000000				// マルチブート用GGID
#define G_INTRO_X							1						// ゲーム内容説明表示X位置
#define G_INTRO_Y							4						// ゲーム内容説明表示Y位置
#define G_MEMBER_X							1						// 通信メンバー表示X位置
#define G_MEMBER_Y							10						// 通信メンバー表示Y位置
#define MSG_VIEW_COUNT						75						// メッセージ表示Vカウント数
#define MSG_BLINK_COUNT						12						// メッセージ点滅カウント数

#define NINLOGO_LOAD_1D_CHAR											// Nintendoロゴデータを1Dマッピングキャラクタでロードする。

	// プログラムシーケンス
typedef enum PrgSeq {
	PSEQ_READY = 0,
	PSEQ_INIT,
	PSEQ_INITIALIZING,
	PSEQ_ENDING,
	PSEQ_END,
	PSEQ_CANCELLED,
	PSEQ_DISCONNECTED_BY_PARENT,
	PSEQ_SCANNING,
	PSEQ_CONNECTING,
	PSEQ_CONNECT_SUCCEEDED,
	PSEQ_CONNECT_FAILED,
	PSEQ_REQ_REFUSED,
	PSEQ_MEMBER_FULL,
	PSEQ_DOWNLOAD_READY,
	PSEQ_DOWNLOADING,
	PSEQ_DOWNLOAD_COMPLETED,
	PSEQ_DOWNLOAD_FAILED,
	PSEQ_BOOT_REQ_WAIT,
	PSEQ_BOOT_READY,
	PSEQ_BOOT_START,
	PSEQ_BOOT_FAILED
}PrgSeq;

static char *str_prgSeq[] = {
	"PSEQ_READY",
	"PSEQ_INIT",
	"PSEQ_INITIALIZING",
	"PSEQ_ENDING",
	"PSEQ_END",
	"PSEQ_CANCELLED",
	"PSEQ_DISCONNECTED_BY_PARENT",
	"PSEQ_SCANNING",
	"PSEQ_CONNECTING",
	"PSEQ_CONNECT_SUCCEEDED",
	"PSEQ_CONNECT_FAILED",
	"PSEQ_REQ_REFUSED",
	"PSEQ_MEMBER_FULL",
	"PSEQ_DOWNLOAD_READY",
	"PSEQ_DOWNLOADING",
	"PSEQ_DOWNLOAD_COMPLETED",
	"PSEQ_DOWNLOAD_FAILED",
	"PSEQ_BOOT_REQ_WAIT",
	"PSEQ_BOOT_READY",
	"PSEQ_BOOT_START",
	"PSEQ_BOOT_FAILED",
};


	// 親機情報発見時間データ
typedef struct FindTime {
	BOOL		find;												// 発見したかどうか
	int 		vcount;												// 発見時間
}FindTime;

	// メッセージ表示ステータス
typedef struct MsgViewStatus {
	u16			handle;
	u16			vcount;
	u16			flag;
	u16			color;
	const u8	*str;
	int			(*nextProcessp)(void);
}MsgViewStatus;

	// 次プログラムシーケンスを格納用キュー
typedef struct PrgNextSeqQue {
	BOOL		lock;
	u8			pad;
	u8			num;
	u8			top;
	u8			bottom;
	PrgSeq		seq[ PRG_NEXT_SEQ_QUE_NUM ];
}PrgNextSeqQue;


// extern data------------------------------------------
extern void MBw_SetMaxScanTime( u16 time );
extern 	int MBw_GetScanChannel( void );
extern void UnCompNintendoLogo( u16 *NintendoLogoDatap, u16 *dstp, u32 *temp );

// function's prototype declaration---------------------
	// メインシーケンス
static void SEQ_DSDL_init( void );
static int  SEQ_DSDL_Connect( void );
static int  SEQ_DSDL_Download( void );
static BOOL CheckNextSeq_Connect( void );
static BOOL CheckNextSeq_Download( void );
	// 無線コールバック
static void CallbackChild_MB( u32 status, void *arg );					// マルチブート用コールバック関数
	// 表示系
static void DispTopScreen( void );
static void DispScanStatus( void );
static void DispParentNum( void );
static void DispParentList( void );
static void DispGameIntroduction( void );
static void DispPlayMember( void );
static void DispParentGameInfo( u32 view_no, u32 listNo, BOOL drawFixedDataFlag );
static void ClearDispParentGameInfo( u32 view_no, u32 listNo );
static void DispGameIntroduction_Core( u32 listNo );
static void DispPlayMember_Core( u32 listNo );
static void ClearDispGameIntroduction( void );
static void ClearDispPlayMember( void );
static void DecimalToString( u8 *dstp, const void *valuep, u8 drawLength, u8 size );
static void InitParentFindTime( void );
static void CountParentFindTime( void );
static BOOL CheckParentFindTime( u32 index );
static void CountAndDispTotalTime( void );
static void SetIconOBJ( u32 index );
static void ClearIconOBJ( u32 index );
static void SetDispMessage( u16 color, const u8 *str );
static BOOL DispMessage( void );
static void SetBlinkMessage( u16 color, const u8 *str );
static void DispBlinkMessage( void );
	// Nintendoロゴ表示
static void InitDispNintendoLogo( void );
static BOOL DispNintendoLogo( void );
	// その他
static void SetMyUserInfo( MBUserInfo *my );
static BOOL IsScanLock( void );
	// 割り込み
static void VBlankIntr_WDL( void );
	// シーケンスキュー処理
static void InitPrgNextSeqQue( void );
static BOOL SetPrgNextSeqQue( PrgSeq seq );
static BOOL GetPrgNextSeqQue( PrgSeq *seqp );
static void LockPrgNextSeqQue( void );
static void UnlockPrgNextSeqQue( void );
static BOOL IsEmptyPrgNextSeqQue( void );

void SearchPatchAddress( void );

// global variable -------------------------------------

// static variable -------------------------------------
static int			(*wdProcess)( void );							// 現在実行中のプロセスへのポインタ
static int			csrC;											// メニューのカーソル位置
static int  		csrC_old;										// メニューの前カーソル位置
static PrgSeq		prgSeq;											// プログラムシーケンス
static PrgNextSeqQue nextSeq;										// 次のプログラムシーケンスを格納するキュー
static BOOL			end_flag;										// 終了フラグ
static MsgViewStatus msgStat;										// メッセージ表示ステータス
static MBUserInfo	myUser;											// 自分のユーザー情報
static u16			tgid = 0;
static u16			tp_touch_count = 0;
static u32			gameInfoLostCount = 0;
// 親機リストの表示に使用
static int			unrenewalFlag;
static int			total_vcount;
static int			time_ms;										// 親機発見時間ms
static int			time_sec;										// 親機発見時間sec
static u16			msgHandle;										// メッセージ表示ハンドル
static u16			dispGameInfoFlag;								// 既に表示したゲーム情報をビットで示す。
static u16			dispIntroFlag;									// ゲームの詳細データ描画フラグ
static u16			dispMemberFlag;									// ゲームの詳細データ描画フラグ
static u16			gameName[ MB_GAME_NAME_LENGTH + 1 ];			// 親機ゲームネーム
static u16			userName[ MB_USER_NAME_LENGTH + 1 ];			// 親機ユーザーネーム
static u16			gameIntroduction[2][ MB_GAME_INTRO_LENGTH / 2 + 1 ];
																	// 親機ゲーム内容説明
static u16			playMember[ MB_MEMBER_MAX_NUM ][ MB_USER_NAME_LENGTH + 1];
																	// 通信メンバー名リスト
static u8			parentListNo[ PARENT_VIEW_NUM ][ 4 ] ATTRIBUTE_ALIGN(2);
																	// 親機リスト番号
static u32			objVramBuff[ (MB_ICON_DATA_SIZE * PARENT_VIEW_NUM + 0x20) / sizeof(u32) ] ATTRIBUTE_ALIGN(32);
static FindTime		findTime[ MB_GAME_INFO_RECV_LIST_NUM ];			// 親機発見時間データ
static GXOamAttr	oamBakM[ 128 ]    ATTRIBUTE_ALIGN(32);			// OAM バックアップ

static const MbBeaconRecvStatus *mbrsp;

// MBライブラリ用ワーク
static u32			mbwork[ MB_CHILD_SYSTEM_BUF_SIZE / sizeof(u32) ];

// Nintendoロゴ表示
static int loop_count;
static u32 ninLogoBuff[ 0x700 / sizeof(u32) ];

static MBDownloadFileInfo dlfileinfo;
static u8 output_buffer[ AUTH_BUFFER_LEN ];

// const data  -----------------------------------------
	// 表示用文字列
static const u8 str_sura[]   ATTRIBUTE_ALIGN(2)	= "/";
static const u8 str_period[] ATTRIBUTE_ALIGN(2)	= ".";
static const u8 str_sec[]    ATTRIBUTE_ALIGN(2)	= "sec";
static const u8 str_disconnected[] ATTRIBUTE_ALIGN(2)		= "disconnected by parent.";
static const u8 str_connect_succeeded[] ATTRIBUTE_ALIGN(2)	= "connect succeeded.";
static const u8 str_connect_failed[] ATTRIBUTE_ALIGN(2)		= "connect failed.";
static const u8 str_req_refused[] ATTRIBUTE_ALIGN(2)		= "req refused.";
static const u8 str_member_full[] ATTRIBUTE_ALIGN(2)		= "member full.";
static const u8 str_downloading[] ATTRIBUTE_ALIGN(2)		= "DOWNLOADING....";
static const u8 str_download_completed[] ATTRIBUTE_ALIGN(2) = "download completed.";
static const u8 str_download_cancelled[] ATTRIBUTE_ALIGN(2) = "download cancelled.";

	// メニュー関数用　親機リスト№文字列リスト
static u8 *str_parentListNo[] ATTRIBUTE_ALIGN(2) = {
	parentListNo[0],
	parentListNo[1],
	parentListNo[2],
	parentListNo[3],
};

	// メニュー関数用　メニュー構成データ
static const MenuComponent childModeSel = {
	PARENT_VIEW_NUM,												// 項目数
	PLIST_X,														// X位置（キャラ数）
	PLIST_Y,														// Y位置（　〃　　）
	0,																// 次の項目へのX位置キャラ数
	4,																// 次の項目へのY位置キャラ数
	3,
	WHITE,															// 非選択色
	HIGHLIGHT_Y,													// 選択色
	(const u8 **)&str_parentListNo,									// メニュー項目文字列リスト
};


//======================================================
// メインループ
//======================================================

// 無線マルチブートのメインループ
int DS_DownloadPlayMain(void)
{
	SEQ_DSDL_init();
	
	wdProcess = SEQ_DSDL_Connect;
	
	OS_TPrintf("MbBeaconRecvStatus:%d\n", sizeof(MbBeaconRecvStatus) );
	
	while(1) {
		OS_WaitIrq(1, OS_IE_V_BLANK);
		ReadKeyPad();
		mf_KEYPAD_rapid();
		
		mbrsp = MB_GetBeaconRecvStatus();
		
		if( wdProcess != NULL ) {
			if(wdProcess()) {
				return 0;
			}
		}
		
		//// ここから ////
		{
			static int touch = 0;
			TPData tmp1 ATTRIBUTE_ALIGN(32);
			TPData tmp2 ATTRIBUTE_ALIGN(32);
			
			while (TP_RequestRawSampling(&tmp1) != 0) {}
			TP_GetCalibratedPoint(&tmp2, &tmp1);
			
			if (tmp2.touch != touch)
			{
				tp_touch_count++;
				(void)DrawDecimalSJIS( 16, 0, RED, &tp_touch_count, 4, 2 );
				OS_Printf("touch change %d  validity = %d\n", tmp2.touch, tmp2.validity);
				touch = tmp2.touch;
			}
		}
		//// ここまで ////
		
//		if (SYSM_IsCardPulledOut()) {								// カード抜け検出
		if ( 0 ) {
			OS_Printf("Card is pulled out.\n");
#ifdef __DEBUG
			OS_Terminate();
#endif
		}
		
		if (PAD_DetectFold() == TRUE) {								// スリープモードへの遷移
			SYSM_GoSleepMode();
		}
		
		OS_PrintServer();											// ARM7からのプリントデバッグを処理する
	}
	return 0;
}


//======================================================
// 無線マルチブート（初期化）
//======================================================

// 無線マルチブートの初期化
static void SEQ_DSDL_init(void)
{
    GXS_SetVisiblePlane( GX_PLANEMASK_NONE );
    GX_SetVisiblePlane ( GX_PLANEMASK_NONE );
	
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr_WDL);
	GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);
	
	// NITROカードのROM内登録データのクリア
	MI_CpuClearFast( (void *)HW_ROM_HEADER_BUF, 0x160 );
	
	// サブスクリーンのクリア
	MI_CpuClearFast( bgBakS, sizeof(bgBakS) );
	MI_CpuFillFast ( (void *)HW_OAM, 192 , HW_OAM_SIZE );
	MI_CpuFillFast ( (void *)oamBakS, 192 , sizeof(oamBakS) );		// 192でフィルしないと、未使用OBJが(0, 0)にキャラNo.0で全部表示されてしまう。
	
	// メインスクリーンのクリア
	MI_CpuClearFast( (void *)HW_BG_VRAM,  0x10000 );
	MI_CpuClearFast( (void *)HW_OBJ_VRAM, 0x8000 );
	MI_CpuClearFast( bgBakM, sizeof(bgBakM) );
	MI_CpuFillFast ( (void *)oamBakM, 192 , sizeof(oamBakM) );		// 192でフィルしないと、未使用OBJが(0, 0)にキャラNo.0で全部表示されてしまう。
	MI_CpuFillFast ( (void *)HW_OAM,  192 , HW_OAM_SIZE );
	MI_CpuCopyFast ( myPlttData, (void *)HW_BG_PLTT, sizeof(myPlttData) );	//  BGパレット   セット
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS  (  1, 0, WHITE,  (const u8 *)"NICKNAME=");
	(void)DrawStringSJIS  (  1, 2, LIGHTGREEN, (const u8 *)"PARENT NUM  =");
//	(void)DrawStringSJIS  (  1, 22, LIGHTGREEN, (const u8 *)"GINFO LOST COUNT =");
	(void)DrawStringSJISEx( 26, 2, CYAN, str_period,	PARENT_VIEW_NUM);
	(void)DrawStringSJISEx( 29, 2, CYAN, str_sec,		PARENT_VIEW_NUM);
	
	InitPrgNextSeqQue();
	unrenewalFlag		= 0;
	end_flag			= FALSE;
	prgSeq				= PSEQ_INIT;
	msgHandle			= 0x8000;
	dispGameInfoFlag	= 0;
	dispIntroFlag		= 0;
	dispMemberFlag		= 0;
	csrC_old			= 2;										// 最初はわざと違う値にしておく
	csrC				= 0;
	{
		int i;
		for( i = 0; i < PARENT_VIEW_NUM; i++ ) {
			parentListNo[i][2] = '.';
			parentListNo[i][3] = 0x00;
		}
	}
	SetMyUserInfo( &myUser );										// MBUserInfoをNITRO設定データから読み出してセット
	{
		// ニックネーム表示
		u16 nickname[ MB_USER_NAME_LENGTH + 1 ];
		ExUTF16_LEtoSJIS_BE( (u8 *)nickname, (u16 *)myUser.name, myUser.nameLength );
		nickname[ myUser.nameLength ] = 0;
		(void)DrawStringSJIS( 8, 0, WHITE, (const u8 *)nickname );
	}
	
	InitParentFindTime();											// 親機発見時間変数のクリア
	
	while( (SYSM_GetBootFlag() & BFLG_WM_INITIALIZED) == 0 ) {}
	
	LOADER_Init( NULL );
	
    GXS_SetVisiblePlane( GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1 );
    GX_SetVisiblePlane ( GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1 );
}


// TOPスクリーンへの表示
static void DispTopScreen( void )
{
	SetTargetScreenSJIS( TOP_SCREEN );
	ClearAllStringSJIS();
	(void)DrawStringSJIS(  1, 0, YELLOW, (const u8 *)"WIRELESS DOWNLOAD");
	(void)DrawStringSJIS(  G_INTRO_X,  G_INTRO_Y,  LIGHTGREEN, (const u8 *)"GAME INTRODUCTION");
	(void)DrawStringSJIS(  G_MEMBER_X, G_MEMBER_Y, LIGHTGREEN, (const u8 *)"MEMBER LIST");
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


//======================================================
// 無線マルチブート（親機サーチ ～ 接続）
//======================================================

// 無線マルチブートの接続シーケンス
static int SEQ_DSDL_Connect(void)
{
	// 現在のprgSeqに応じた処理
	BOOL getSeqFlag = CheckNextSeq_Connect();
	
	switch ( prgSeq ) {
	  case PSEQ_INIT:												// 子機モードの開始（ライブラリの初期化～StartScanまでの流れを実行）
		if ( MB_Init( mbwork, &myUser, MY_GGID, tgid, MB_DMA_NO ) != WM_ERRCODE_SUCCESS ) {
			OS_Printf("MB init failed.\n");
			break;
		}
		MB_CommSetChildStateCallback( CallbackChild_MB );
		if ( MB_StartChild() != WM_ERRCODE_SUCCESS ) {
			OS_Printf("MB start failed.\n");
			break;
		}
		tgid++;
		DispTopScreen();
		prgSeq = PSEQ_INITIALIZING;
		break;
		
	  case PSEQ_INITIALIZING:
		break;
		
	  case PSEQ_SCANNING:											// スキャン中は随時、次シーケンスチェックを行う。
	  case PSEQ_ENDING:
		if( !getSeqFlag ) {
			return 0;
		}
		break;
		
	  case PSEQ_CONNECTING:											// 接続中はキー入力を受け付けずにリターンする。（次シーケンスチェックは行う）
		return 0;
		
	  case PSEQ_CONNECT_SUCCEEDED:									// メッセージ表示後、ダウンロードシーケンスに移行。
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// シーケンスロック解除
			wdProcess = SEQ_DSDL_Download;
		}
		return 0;
		
	  case PSEQ_CONNECT_FAILED:										// メッセージ表示後、スキャン再開。（スキャン再開そのものは、ライブラリが自動で行う。）
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// シーケンスロック解除
			prgSeq = PSEQ_SCANNING;
		}
		break;
		
	  case PSEQ_DISCONNECTED_BY_PARENT:								// 切断検出メッセージ表示設定。
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// シーケンスロック解除
			prgSeq = PSEQ_INIT;
		}
		break;
		
	  case PSEQ_CANCELLED:											// 終了
		return 0;
		
	  default:
		break;
	}
	
	
	//  カーソル移動
	if(pad.trg & PAD_KEY_DOWN){										// カーソルの移動
		if( ++csrC == MB_GAME_INFO_RECV_LIST_NUM ){
			csrC = 0;
		}
	}
	if(pad.trg & PAD_KEY_UP){
		if( --csrC & 0x80 ){
			csrC = MB_GAME_INFO_RECV_LIST_NUM - 1;
		}
	}
	DrawMenu((u16)(csrC & (PARENT_VIEW_NUM - 1)), &childModeSel);	// リスト番号（カーソル）の表示
	DispParentNum();												// 発見親機数の表示
	DispParentList();												// 親機リストの表示
	DispScanStatus();												// WMスキャン状態の表示
	CountAndDispTotalTime();										// トータル時間のカウント＆表示
	CountParentFindTime();											// 親機発見時間のカウント
	
	
	// 親機への接続
	if(pad.trg & PAD_BUTTON_A) {
		if( mbrsp->usefulGameInfoFlag & (0x0001 << csrC) ) {
			if( MB_CommDownloadRequest( csrC ) == WM_ERRCODE_SUCCESS ) {	// 接続開始
				OS_Printf("start conntct to parent.\n");
				prgSeq = PSEQ_CONNECTING;
			}else {
				OS_Printf("this parent info is invalid.\n");
			}
		}
	}else if(pad.trg & PAD_BUTTON_B) {								// 終了開始
		if( prgSeq == PSEQ_SCANNING ) {
			OS_Printf("MB End start\n");
			prgSeq	 = PSEQ_ENDING;
//			end_flag = TRUE;
			MB_End();
		}
	}else if( pad.trg & PAD_BUTTON_R ) {
		unrenewalFlag ^= 0x01;
//		MB_SetUnrenewalGameInfoFlag( (BOOL)unrenewalFlag );
	}
	
	csrC_old = csrC;												// 旧カーソル位置の退避
	return 0;
}


// 次シーケンスのチェック（コールバックでメインループとは非同期に返される次シーケンスへの移行をメインループのプログラム進行に合わせる。）
static BOOL CheckNextSeq_Connect(void)
{
	PrgSeq seq;
	// MBコールバックによって、シーケンスが移行していた場合の処理
	if( GetPrgNextSeqQue( &seq) ) {									// MBコールバックによって、シーケンスが移行していた場合の処理
		prgSeq = seq;
		OS_Printf("prgSeq = %s\n", str_prgSeq[ prgSeq ]);
		
		switch( seq ) {
		  case PSEQ_SCANNING:
			{
/*
				u8 *ver = MBw_GetWLVersion();
				(void)DrawStringSJIS( 21, 0, WHITE, (const u8 *)"WL ver=" );
				(void)DrawStringSJIS( 26, 0, WHITE, (const u8 *)ver );
*/
			}
			break;
			
		  case PSEQ_CONNECT_SUCCEEDED:								// 接続成功メッセージ表示設定。
			SetDispMessage( YELLOW, str_connect_succeeded );
			LockPrgNextSeqQue();									// prgSeqをロックする。
			return FALSE;
			
		  case PSEQ_CONNECT_FAILED:									// 接続失敗メッセージ表示設定。
			SetDispMessage( RED, str_connect_failed);
			LockPrgNextSeqQue();									// prgSeqをロックする。
			return FALSE;
			
		  case PSEQ_DISCONNECTED_BY_PARENT:							// 切断検出メッセージ表示設定。
			SetDispMessage( RED, str_disconnected);
			LockPrgNextSeqQue();									// prgSeqをロックする。
			return FALSE;
			
		  case PSEQ_CANCELLED:										// 終了
			{
				u32 i;
				for( i = 0; i < PARENT_VIEW_NUM; i++ ) {
					ClearIconOBJ( i );								// 全アイコンの消去
				}
//				if( end_flag ) {
//					ClearAllStringSJIS();
//					SetTargetScreenSJIS( TOP_SCREEN );
//					ClearAllStringSJIS();
//					SetTargetScreenSJIS( BOTTOM_SCREEN );
//					(void)DrawStringSJIS( 11, 11, WHITE, (const u8 *)"しゅうりょうしました。");
//				}else {
					prgSeq = PSEQ_INIT;
//				}
			}
			return FALSE;
		}
	}
	return TRUE;													// シーケンス移行がなかった場合もしくはFALSEリターンのシーケンスでなかった場合はTRUEリターン。
}


//======================================================
// 無線マルチブート（ダウンロード）
//======================================================

// 無線マルチブートのファイルダウンロードシーケンス
static int SEQ_DSDL_Download(void)
{
	static int count_old = 0;
	// 現在のprgSeqに応じた処理
	BOOL getSeqFlag = CheckNextSeq_Download();
	
	switch(prgSeq) {
		
	  case PSEQ_REQ_REFUSED:
	  case PSEQ_MEMBER_FULL:										// メッセージ表示後、一旦終了して初期化からやり直し。
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// シーケンスロック解除
			prgSeq		= PSEQ_ENDING;
			wdProcess	= SEQ_DSDL_Connect;
			MB_End();
		}
		break;
		
		// ダウンロードファイル情報を受信し、ダウンロード準備ができた状態
	  case PSEQ_DOWNLOAD_READY:
//		if (pad.trg & PAD_BUTTON_A) {								// ダウンロード開始
			OS_Printf("Start download.\n");
			ClearStringSJIS_handle(msgHandle);
			(void)MB_CommStartDownload();
			SetBlinkMessage( YELLOW, str_downloading );
//		}
		break;
		
		// ダウンロード中
	  case PSEQ_DOWNLOADING:
		if( getSeqFlag ) {
			DispBlinkMessage();
			msgHandle = msgStat.handle;
			
			if( 1 ) {
				u16 percent = MB_GetChildProgressPercentage();
				msgHandle = DrawDecimalSJIS( 16, 21, WHITE, &percent ,3, 2 );
			}
		}
		break;
		
		// ダウンロード完了
	  case PSEQ_DOWNLOAD_COMPLETED:
		if( DispMessage() ) {
			{
				msgHandle = DrawStringSJIS( 4, 21,  LIGHTGREEN, "Sign Digest....");
				if ( ACSignDigest ( output_buffer, &dlfileinfo ) ) {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  LIGHTGREEN, "Digest SUCCEEDED.");
				}else {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  RED,        "Digest FAILED.");
				}
				{
					int i;
					for ( i = 0; i < 120; i++ ) {
						SVC_WaitVBlankIntr();
					}
				}
				ClearStringSJIS_handle( msgHandle );
			}
			UnlockPrgNextSeqQue();									// シーケンスロック解除
			prgSeq = PSEQ_BOOT_REQ_WAIT;
			msgHandle = DrawStringSJIS( 4, 21,  WHITE, "Waiting BOOT-REQ from parent.");
		}
		break;
		
		// 親機からのブート許可待ち
	  case PSEQ_BOOT_REQ_WAIT:
		break;
		
		// ダウンロードが完了し、ブート準備ができた状態
//	  case PSEQ_BOOT_READY:
//		(void)LOADER_Start();										// ここでARM7によって、ダウンロードプログラムの再配置を行う。
//		return 1;													// "1"でリターンすることで、無線マルチブートのメインループから抜けてIPL2に処理を戻す。
		
		// 親機からのブート要求受信により、起動可否チェックを行う。（Nintendoロゴ表示＆チェック）
	  case PSEQ_BOOT_READY:
		if( DispNintendoLogo() ) {
			if( SYSM_CheckNinLogo( (u16 *)SYSM_GetCardRomHeader()->nintendo_logo ) ) {
				prgSeq = PSEQ_BOOT_START;
			}else {
				SetDispMessage( RED, (const u8 *)"Illegal game data.");
				prgSeq = PSEQ_BOOT_FAILED;
			}
		}
		break;
		
		// ブート開始
	  case PSEQ_BOOT_START:
		ClearStringSJIS_handle(msgHandle);
		
		SearchPatchAddress();
		
		(void)LOADER_Start();										// ここでARM7によって、ダウンロードプログラムの再配置を行う。
		return 1;													// "1"でリターンすることで、無線マルチブートのメインループから抜けてIPL2に処理を戻す。
		
		// ブート失敗
		// 親機によって、接続が切断された状態
		// キャンセルをした場合
	  case PSEQ_BOOT_FAILED:
	  case PSEQ_DISCONNECTED_BY_PARENT:
	  case PSEQ_CANCELLED:
		if( DispMessage() ) {										// 指定メッセージを一定期間表示
			UnlockPrgNextSeqQue();									// シーケンスロック解除
			ClearStringSJIS_handle( msgHandle );
			prgSeq		= PSEQ_INIT;
			wdProcess	= SEQ_DSDL_Connect;
		}
		break;
		
	  default:
		break;
	}
	
	// Bボタンで終了処理を行う
	if( pad.trg & PAD_BUTTON_B ) {
		MB_End();													// キャンセル時は、親機サーチ状態からやり直す。
	}
	
	DrawMenu((u16)(csrC & (PARENT_VIEW_NUM - 1)), &childModeSel);	// リスト番号（カーソル）の表示
	DispParentNum();												// 発見親機数の表示
	DispParentList();												// 親機リストの表示
	DispScanStatus();												// WMスキャン状態の表示
	CountAndDispTotalTime();										// トータル時間のカウント＆表示
	CountParentFindTime();											// 親機発見時間のカウント
	return 0;
}


// 次シーケンスのチェック（コールバックでメインループとは非同期に返される次シーケンスへの移行をメインループのプログラム進行に合わせる。）
static BOOL CheckNextSeq_Download(void)
{
	PrgSeq seq;
	
	// MBコールバックによって、シーケンスが移行していた場合の処理
	if( GetPrgNextSeqQue( &seq) ) {
		prgSeq = seq;
		OS_Printf("prgSeq = %s\n", str_prgSeq[ prgSeq ]);
		
		switch( seq ) {
			
		  case PSEQ_REQ_REFUSED:									// 接続拒否メッセージ表示設定
			SetDispMessage( RED, str_req_refused);
			LockPrgNextSeqQue();									// prgSeqをロックする。
			return FALSE;
			
		  case PSEQ_MEMBER_FULL:									// メンバーFULLメッセージ表示設定。
			SetDispMessage( RED, str_member_full);
			LockPrgNextSeqQue();									// prgSeqをロックする。
			return FALSE;
			
		  case PSEQ_DOWNLOAD_READY:									// ダウンロード準備完了
			ClearStringSJIS_handle( msgHandle );
			{
				msgHandle = DrawStringSJIS( 4, 21,  WHITE, "Sign Decript....");
				if ( ACSignDecrpto( output_buffer, &dlfileinfo ) ) {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  LIGHTGREEN, "Decript SUCCEEDED.");
				}else {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  RED,        "Decript FAILED.");
				}
				{
					int i;
					for ( i = 0; i < 120; i++ ) {
						SVC_WaitVBlankIntr();
					}
				}
				ClearStringSJIS_handle( msgHandle );
			}
			
			msgHandle = DrawStringSJIS( 4, 21,  WHITE, "PUSH [A] TO DOWNLOAD.");
			break;
			
		  case PSEQ_DOWNLOAD_COMPLETED:								// ダウンロード完了
			ClearStringSJIS_handle( msgHandle );
			SetDispMessage( YELLOW, str_download_completed );
			LockPrgNextSeqQue();									// シーケンスロック
			return FALSE;
			
//		  case PSEQ_BOOT_READY:										// ブート準備完了
//			ClearStringSJIS_handle( msgHandle );
//			break;
			
		  case PSEQ_BOOT_READY:										// ブート準備完了
			ClearStringSJIS_handle( msgHandle );
			InitDispNintendoLogo();
			break;
			
		  case PSEQ_CANCELLED:
			ClearStringSJIS_handle( msgHandle );
			SetDispMessage( RED, str_download_cancelled );
			LockPrgNextSeqQue();									// シーケンスロック
			return FALSE;
			
		  case PSEQ_DISCONNECTED_BY_PARENT:							// 切断検出メッセージ表示設定。
			ClearStringSJIS_handle( msgHandle );
			SetDispMessage( RED, str_disconnected );
			LockPrgNextSeqQue();									// prgSeqをロックする。
			return FALSE;
			
		  default:
			break;
		}
	}
	return TRUE;
}


//======================================================
// コールバック
//======================================================

// 子機モードでのコールバック
static void CallbackChild_MB( u32 status, void *arg )
{
	switch (status)
	{
	  case MB_COMM_CSTATE_INIT_COMPLETE:							// 初期化完了
		OS_Printf("CB: init completed.\n");
		(void)SetPrgNextSeqQue( PSEQ_SCANNING );
		break;
		
//	  case MB_COMM_CSTATE_END_COMPLETE:								// マルチブート終了
//		OS_Printf("CB: end completed.\n");
//		(void)SetPrgNextSeqQue( PSEQ_END );
//		break;

	  case MB_COMM_CSTATE_REQ_REFUSED:								// 親機からのエントリー拒否
	    OS_Printf("CB: request refused.\n");
		(void)SetPrgNextSeqQue( PSEQ_REQ_REFUSED );
	    break;

	  case MB_COMM_CSTATE_MEMBER_FULL:								// ゲームが定員に達した
	    OS_Printf("CB: member full.\n");
		(void)SetPrgNextSeqQue( PSEQ_MEMBER_FULL );
	    break;
		
	  case MB_COMM_CSTATE_CONNECT:									// 親機への接続成功
		OS_Printf("CB: connected.\n");
		(void)SetPrgNextSeqQue( PSEQ_CONNECT_SUCCEEDED );
		break;
		
	  case MB_COMM_CSTATE_CONNECT_FAILED:							// 親機への接続失敗
		OS_Printf("CB: connect failed!\n");
		(void)SetPrgNextSeqQue( PSEQ_CONNECT_FAILED );
		break;
		
	  case MB_COMM_CSTATE_DLINFO_ACCEPTED:							// 親機からダウンロード情報を受信
		OS_Printf("CB: dlinfo accepted.\n");
		(void)SetPrgNextSeqQue( PSEQ_DOWNLOAD_READY );
		MI_CpuCopy16( (void *)arg, (void *)&dlfileinfo, sizeof( MBDownloadFileInfo ) ) ;
		break;
		
	  case MB_COMM_CSTATE_RECV_PROCEED:								// 受信開始
		OS_Printf("CB: recv proceed.\n");
		(void)SetPrgNextSeqQue( PSEQ_DOWNLOADING );
		break;
		
	  case MB_COMM_CSTATE_RECV_COMPLETE:							// 受信成功
		OS_Printf("CB: recv completed.\n");
		(void)SetPrgNextSeqQue( PSEQ_DOWNLOAD_COMPLETED );
		break;
		
	  case MB_COMM_CSTATE_BOOT_READY:								// ブート準備完了
		OS_Printf("CB: boot ready.\n");
//		SYSM_GetWork()->mb_flag = 1;
//		SYSM_GetWork()->mb_ggid = *(u32 *)( MB_GetBeaconRecvStatus()->list[ pCwork->connectTargetNo ].bssDesc.gameInfo.ggid );
//		SYSM_GetWork()->mb_ggid = MB_GetBeaconRecvStatus()->list[ pCwork->connectTargetNo ].gameInfo.ggid;
		(void)SetPrgNextSeqQue( PSEQ_BOOT_READY );
		break;
		
	  case MB_COMM_CSTATE_CANCELLED:								// ダウンロードキャンセル
		OS_Printf("CB: download cancel.\n");
		(void)SetPrgNextSeqQue( PSEQ_CANCELLED );
		break;
		
	  case MB_COMM_CSTATE_DISCONNECTED_BY_PARENT:					// 親機からの切断通知
		OS_Printf("CB: disconnected by parent!\n");
		(void)SetPrgNextSeqQue( PSEQ_DISCONNECTED_BY_PARENT );
		break;
		
	  case MB_COMM_CSTATE_GAMEINFO_LOST:
		gameInfoLostCount++;
		break;
	}
}


//======================================================
// 表示系サブルーチン
//======================================================

// スキャン状態の表示
static void DispScanStatus(void)
{
#if 0
	int channel = MBw_GetScanChannel();
	
	(void)DrawHexSJIS( 13, 2, WHITE, &channel, 2);
	if( IsScanLock() ) {											// スキャンロック中なら、その時間を表示
		(void)DrawStringSJIS( 15, 2, RED,   (const u8 *)"LOCK");
		(void)DrawHexSJIS   ( 19, 2, WHITE, &mbrsp->nowLockTimeCount, 4);
	}else {
		(void)DrawStringSJIS( 15, 2, RED, (const u8 *)"          ");
	}
#endif
}


// 発見した親機数の表示
static void DispParentNum(void)
{
	int i;
	int num = 0;
	
	for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++) {				// 親機数のカウント
		if( mbrsp->usefulGameInfoFlag & (0x0001 << i) )	num++;
	}
	(void)DrawDecimalSJIS( 10, 2, LIGHTGREEN, &num, 2, 4);
}


// 親機情報リストの表示
static void DispParentList(void)
{
	u32 i;
	u32 listNo		= (u32)( csrC     & ~(PARENT_VIEW_NUM - 1) );	// 現在表示中のリストの先頭を算出（~表示数でマスク）
	u32 listNo_old	= (u32)( csrC_old & ~(PARENT_VIEW_NUM - 1) );	// 前回表示したリストの先頭を算出
	
//	(void)DrawDecimalSJISEx( 15, 22, WHITE, &gameInfoLostCount, 8, 4, 0);
	
	// ゲーム内容説明の表示
	DispGameIntroduction();
	
	// 通信メンバーの表示
	DispPlayMember();
	
	// 親機リストの表示
	for( i = 0; i < PARENT_VIEW_NUM; i++ ) {
		if( (listNo != listNo_old) || !(mbrsp->usefulGameInfoFlag & (0x0001 << listNo)) ){
			if( dispGameInfoFlag & (0x0001 << listNo_old) ) {		// 表示リスト位置が変更 or 該当親機が有効でなくなった場合、前回表示データが既に表示済みならクリアする。
				ClearDispParentGameInfo( i, listNo_old);
				dispGameInfoFlag ^= 0x0001 << listNo_old;
			}
		}
		
		if( mbrsp->usefulGameInfoFlag & (0x0001 << listNo) ) {		// 該当リスト№のデータが有効で、かつ未表示なら表示する。
			DispParentGameInfo( i, listNo , !(dispGameInfoFlag & (0x0001 << listNo)) );
			dispGameInfoFlag |= 0x0001 << listNo;
		}
		if( csrC != csrC_old ) {
			DecimalToString( parentListNo[i], &listNo, 2, 2);		// ビュー画面の先頭リストナンバーからを文字列バッファに入れる。
		}
		listNo++;
		listNo_old++;
	}
}


// ゲーム内容説明の表示
static void DispGameIntroduction( void )
{
	BOOL clear = FALSE;
	
	if( ( ( csrC != csrC_old ) && ( dispIntroFlag & (0x0001 << csrC_old) ) )
	 || ( !(mbrsp->usefulGameInfoFlag & (0x0001 << csrC)) && ( dispIntroFlag & (0x0001 << csrC) ) ) ) {
		ClearDispGameIntroduction();								// カーソルが動いて、前カーソル位置の詳細表示を行っていたか、
		dispIntroFlag	= 0;
		clear			= TRUE;
	}
	
	if( (mbrsp->usefulGameInfoFlag & (0x0001 << csrC)) && !( dispIntroFlag & (0x0001 << csrC) ) ) {
		if ( !clear ){												// 現在カーソル位置のゲーム情報が有効で、まだ詳細表示を行っていなかったら表示。
			ClearDispGameIntroduction();
		}
		DispGameIntroduction_Core( (u32)csrC );
		dispIntroFlag	= (u16)( 0x0001 << csrC );					// カーソルが移動された時か、現在位置のゲーム情報が未表示の時にのみ表示 or クリアを行う。
	}
}


// 通信メンバーのクリア＆表示
static void DispPlayMember( void )
{
	BOOL clear = FALSE;
	
	if( ( ( csrC != csrC_old ) && ( dispMemberFlag & (0x0001 << csrC_old) ) )
	 || ( !(mbrsp->validGameInfoFlag & (0x0001 << csrC)) && ( dispMemberFlag & (0x0001 << csrC) ) ) ) {
		ClearDispPlayMember();										// カーソル位置のゲーム詳細情報を表示していて、そのゲーム情報が無効になったらクリア。
		dispMemberFlag	= 0;
		clear			= TRUE;
	}
	
	if( (mbrsp->validGameInfoFlag & (0x0001 << csrC)) && !( dispMemberFlag & (0x0001 << csrC) ) ) {
		if ( !clear ){												// 現在カーソル位置のゲーム情報が有効で、まだ詳細表示を行っていなかったら表示。
			ClearDispPlayMember();
		}
		DispPlayMember_Core( (u32)csrC );
		dispMemberFlag	= (u16)( 0x0001 << csrC );					// カーソルが移動された時か、現在位置のゲーム情報が未表示の時にのみ表示 or クリアを行う。
	}
}


// 指定された親機情報の表示
static void DispParentGameInfo( u32 view_no, u32 listNo, BOOL drawFixedDataFlag)
{
	if(drawFixedDataFlag) {											// 変化しない情報の表示
		
		MI_CpuClear16( gameName, MB_GAME_NAME_LENGTH * 2 );
		MI_CpuClear16( userName, MB_USER_NAME_LENGTH * 2 );
		ExUTF16_LEtoSJIS_BE( (u8 *)gameName, (u16 *)mbrsp->list[ listNo ].gameInfo.fixed.gameName, MB_GAME_NAME_LENGTH * 2 );
		ExUTF16_LEtoSJIS_BE( (u8 *)userName, (u16 *)mbrsp->list[ listNo ].gameInfo.fixed.parent.name, (u16)( mbrsp->list[ listNo ].gameInfo.fixed.parent.nameLength * 2));
//		MI_CpuCopy16( mbrsp->list[ listNo ].gameInfo.fixed.gameName, gameName, MB_GAME_NAME_LENGTH * 2);
//		MI_CpuCopy16( mbrsp->list[ listNo ].gameInfo.fixed.parent.name, userName, mbrsp->list[ listNo ].gameInfo.fixed.parent.nameLength * 2);
		gameName[ MB_GAME_NAME_LENGTH ] = 0;
		userName[ mbrsp->list[ listNo ].gameInfo.fixed.parent.nameLength ] = 0;
		
		(void)ClearStringSJISEx( gameName, view_no);
		(void)ClearStringSJISEx( userName, view_no);
		(void)DrawStringSJISEx(  9, (PLIST_Y + (int)view_no*4),     WHITE, gameName, view_no);
		(void)DrawStringSJISEx(  9, (PLIST_Y + (int)view_no*4 + 2), WHITE, userName, view_no);
		
		(void)DrawStringSJISEx( 26, (PLIST_Y + (int)view_no*4),     YELLOW, str_sura, view_no);
		(void)DrawDecimalSJIS ( 27, (PLIST_Y + (int)view_no*4),     YELLOW, &mbrsp->list[ listNo ].gameInfo.fixed.maxPlayerNum, 2, 1);
		
		// 親機発見時間の表示
		(void)CheckParentFindTime( listNo );						// ここで親機を発見しているかを再確認しておく。
		time_ms		= (int)findTime[listNo].vcount * 17;
		time_sec	= time_ms / 1000;
		time_ms		= (time_ms % 1000) / 10;
		(void)DrawDecimalSJISEx( 23, (PLIST_Y + (int)view_no*4 + 2), CYAN, &time_sec, 4, 4, view_no);
		(void)DrawDecimalSJISEx( 27, (PLIST_Y + (int)view_no*4 + 2), CYAN, &time_ms,  2, 4, view_no);
		(void)DrawStringSJISEx ( 26, (PLIST_Y + (int)view_no*4 + 2), CYAN, str_period, view_no);
		(void)DrawStringSJISEx ( 29, (PLIST_Y + (int)view_no*4 + 2), CYAN, str_sec, view_no);
		
		SetIconOBJ( listNo );										// アイコンの表示
	}
	{	// 変化する情報（現メンバー数・親機寿命カウントの表示）
		(void)DrawDecimalSJIS( 24, (PLIST_Y + (int)view_no*4),     YELLOW, &mbrsp->list[ listNo ].gameInfo.volat.nowPlayerNum, 2, 1);
		(void)DrawHexSJIS    ( 19, (PLIST_Y + (int)view_no*4 + 2), CYAN,   &mbrsp->list[ listNo ].lifetimeCount, 4);
		{
			u16 color = RED;
			if ( mbrsp->validGameInfoFlag & ( 0x0001 << listNo ) ) {
				color = CYAN;
			}
			(void)DrawHexSJIS    ( 19, (PLIST_Y + (int)view_no*4), color,   &mbrsp->list[ listNo ].gameInfo.seqNoVolat, 2);
		}
	}
}


// 指定された親機情報の表示クリア
static void ClearDispParentGameInfo( u32 view_no, u32 listNo)
{
	// ※これらの変数のアドレスは、DispParentGameInfo側のものと一致してないと、表示をクリアすることができないので注意。（自動変数で表示するときはアドレスが変わってしまう）
	// （これはDrawStringSJIS関数の仕様）
	ClearStringSJISEx( gameName,   view_no );						// ゲーム名
	ClearStringSJISEx( userName,   view_no );						// ユーザー名
	ClearStringSJISEx( &time_sec,  view_no );						// 親機発見時間sec
	ClearStringSJISEx( &time_ms,   view_no );						// 親機発見時間ms
	ClearStringSJISEx( (void *)str_sura,   view_no );				// "/"
	ClearStringSJISEx( (void *)str_period, view_no );				// "."
	ClearStringSJISEx( (void *)str_sec,    view_no );				// "sec"
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].lifetimeCount );				// 親機寿命カウント
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].gameInfo.volat.nowPlayerNum );	// プレイ人数
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].gameInfo.fixed.maxPlayerNum );	// 最大プレイ人数
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].gameInfo.seqNoVolat );	// 最大プレイ人数
	
	ClearIconOBJ( listNo );											// アイコンの消去
}


// ゲーム内容説明の表示（実処理)
static void DispGameIntroduction_Core( u32 listNo )
{
	int i;
	const u16 *strp = mbrsp->list[ listNo ].gameInfo.fixed.gameIntroduction;
	u16 *dstp;
	u16	gameIntroTmp[ MB_GAME_INTRO_LENGTH / 2 + 1 ];
	
	MI_CpuClear16( gameIntroduction[0], (MB_GAME_INTRO_LENGTH / 2 + 1) * sizeof(u16) );
	MI_CpuClear16( gameIntroduction[1], (MB_GAME_INTRO_LENGTH / 2 + 1) * sizeof(u16) );
	
	dstp = gameIntroTmp;
	for ( i = 0; i < MB_GAME_INTRO_LENGTH / 2; i++ ) {				// バイト単位でのコピーなので、1行分は、MB_GAME_INTRO_LENGTH/2*2。
		if( ( *strp == NULL) || ( *strp == 0x000a ) ) {
			*dstp++ = 0x0000;
		}else {
			*dstp++ = *strp++;
		}
	}
	ExUTF16_LEtoSJIS_BE( (u8 *)gameIntroduction[ 0 ], gameIntroTmp, MB_GAME_INTRO_LENGTH / 2 );
	
	if( *strp == 0x000a ) {
		strp++;
	}
	
	dstp = gameIntroTmp;
	for ( i = 0; i < MB_GAME_INTRO_LENGTH / 2; i++ ) {
		if( ( *strp == NULL) || ( *strp == 0x000a ) ) {
			*dstp++ = 0x00;
		}else {
			*dstp++ = *strp++;
		}
	}
	ExUTF16_LEtoSJIS_BE( (u8 *)gameIntroduction[ 1 ], gameIntroTmp, MB_GAME_INTRO_LENGTH / 2 + 1 );
	
	ClearDispGameIntroduction();
	
	SetTargetScreenSJIS( TOP_SCREEN );
	(void)DrawStringSJIS(  ( G_INTRO_X + 1 ), ( G_INTRO_Y + 2 ), WHITE, (const u8 *)gameIntroduction[0] );
	(void)DrawStringSJIS(  ( G_INTRO_X + 1 ), ( G_INTRO_Y + 4 ), WHITE, (const u8 *)gameIntroduction[1] );
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// ゲーム内容説明表示のクリア
static void ClearDispGameIntroduction( void )
{
	SetTargetScreenSJIS( TOP_SCREEN );
	ClearStringSJIS( (void *)gameIntroduction[0] );
	ClearStringSJIS( (void *)gameIntroduction[1] );
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// 通信メンバーの表示（実処理）
static void DispPlayMember_Core( u32 listNo )
{
	int n, count_x;
	u16 nameLength;
	int pos_x, pos_y;
	const MBUserInfo *memberp = &mbrsp->list[ listNo ].gameInfo.volat.member[ 0 ];
	u16 *dstp;
	
	SetTargetScreenSJIS( TOP_SCREEN );
	
	MI_CpuClear16( playMember, sizeof(playMember) );
	count_x	= 0;
	pos_x	= G_MEMBER_X + 1;
	pos_y	= G_MEMBER_Y + 2;
	for ( n = 0; n < MB_MEMBER_MAX_NUM; n++ ) {
		
		if( memberp->nameLength > MB_USER_NAME_LENGTH ) {			// 名前長のチェック
			nameLength = MB_USER_NAME_LENGTH;
		}else {
			nameLength = memberp->nameLength;
		}
		dstp = playMember[ n ];
		if( mbrsp->list[ listNo ].gameInfo.volat.nowPlayerFlag & (0x0002 << n ) ) {
			
			ExUTF16_LEtoSJIS_BE( (u8 *)dstp, (u16 *)memberp->name, nameLength );
			playMember[ n ][ nameLength ] = 0;
			(void)DrawStringSJIS( pos_x, pos_y, WHITE, (const u8 *)playMember[ n ] );
		}else {
			MI_CpuCopy16( (void *)"----------", (void *)playMember[ n ], MB_USER_NAME_LENGTH + 1);
			(void)DrawStringSJIS( pos_x, pos_y, WHITE, playMember[ n ] );
		}
		memberp++;
		
		if( ++count_x == 3 ) {										// 表示位置の算出
			pos_x	-= 20;
			pos_y	+= 2;
			count_x	 = 0;
		}else {
			pos_x	+= 10;
		}
	}
	
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// 通信メンバー表示のクリア
static void ClearDispPlayMember( void )
{
	int i;
	
	SetTargetScreenSJIS( TOP_SCREEN );
	for( i = 0; i < MB_MEMBER_MAX_NUM + 1; i++ ) {
		ClearStringSJIS( (void *)playMember[ i ] );
	}
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// 10進データを文字列に変換
static void DecimalToString(u8 *dstp, const void *valuep, u8 drawLength, u8 size)
{
	u16 count;
	u32 mask, divisor, target;
	
	mask = 0xff;
	while(--size > 0) {
		mask = (mask << 8) | 0xff;
	}
	target=(*(u32 *)valuep) & mask;
	count=10;
	divisor=1000000000;
	while(count) {
		CP_SetDiv32_32(target, divisor);
		if (count <= drawLength) {
			*dstp++ = (u8)(CP_GetDivResult32()+0x0030);
		}
		target=(u32)CP_GetDivRemainder32();
		CP_SetDiv32_32(divisor, 10);
		divisor=(u32)CP_GetDivResult32();
		count--;
	}
}


// 親機発見時間変数のクリア
static void InitParentFindTime( void )
{
	int i;
	for( i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++) {
		findTime[i].find	= FALSE;
		findTime[i].vcount	= 0;
	}
	total_vcount		= 0;
}


// 親機発見時間のカウント
static void CountParentFindTime( void )
{
	u32 i;
	
	for( i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++ ) {
		if( !CheckParentFindTime( i ) ) {
			findTime[i].vcount = total_vcount;						// 親機情報が見つかるまでカウント
		}
	}
}


// 親機情報が揃ったかどうかチェックして、発見時間カウントを停止する。
static BOOL CheckParentFindTime( u32 index )
{
	if( mbrsp->validGameInfoFlag & (0x01 << index) ) {				// 親機情報が発見されたなら、カウントを停止する。
		findTime[ index ].find = TRUE;
	}
	return findTime[ index ].find;
}


// トータル時間のカウント＆表示
static void CountAndDispTotalTime( void )
{
	total_vcount++;
	time_ms		= total_vcount * 17;
	time_sec	= time_ms / 1000;
	time_ms		= (time_ms % 1000) / 10;
	(void)DrawDecimalSJISEx( 23, 2, CYAN, &time_sec, 4, 4,	PARENT_VIEW_NUM);
	(void)DrawDecimalSJISEx( 27, 2, CYAN, &time_ms,  2, 4,	PARENT_VIEW_NUM);
}


// アイコンOBJのロード
static void SetIconOBJ( u32 index )
{
	u16 view_no			= (u16)( index & 0x03 );
	const MBIconInfo *iconp	= &mbrsp->list[ index ].gameInfo.fixed.icon;
	
	GXS_LoadOBJPltt( iconp->palette, (u32)( MB_ICON_PALETTE_SIZE * view_no ), MB_ICON_PALETTE_SIZE );
	MI_CpuCopyFast(  iconp->data,    (void *)((u32)objVramBuff + 0x20 + MB_ICON_DATA_SIZE * view_no), MB_ICON_DATA_SIZE );
	G2_SetOBJAttr( (GXOamAttr*)&oamBakS[ view_no ],					// OAM pointer
					8 * (PLIST_X + 3),								// X position
					8 *  PLIST_Y + (view_no * 32),					// Y position
					0,												// Priority
					GX_OAM_MODE_NORMAL,								// Bitmap mode
					FALSE,											// mosaic off
					GX_OAM_EFFECT_NONE,								// affine off
					GX_OAM_SHAPE_32x32,								// 16x16 size
					GX_OAM_COLOR_16,								// 16 color
					1 + view_no * 16,								// charactor
					view_no,										// palette
					0);												// affine
}

// アイコンOBJのクリア
static void ClearIconOBJ( u32 index )
{
	u16 view_no		= (u16)( index & 0x03 );
	u16 *oamp		= (u16 *)&oamBakS[ view_no ];
	
	*oamp++  = 192;													// Y座標を画面外に。（オール0クリアではダメ）
	*oamp++  = 0;
	*oamp++  = 0;
	*oamp    = 0;
}


// メッセージ表示のセット
static void SetDispMessage( u16 color, const u8 *str )
{
	msgStat.vcount	 = MSG_VIEW_COUNT;
	msgStat.color	 = color;
	msgStat.str		 = str;
}


// メッセージ表示
static BOOL DispMessage(void)
{
	if( msgStat.vcount == MSG_VIEW_COUNT ) {
		msgStat.handle = DrawStringSJIS( 4, 21, msgStat.color, msgStat.str );
	}
	
	if ( --msgStat.vcount == 0 ) {
		ClearStringSJIS_handle( msgStat.handle );
		return TRUE;
	}
	return FALSE;
}


// メッセージ表示のセット（点滅表示）
static void SetBlinkMessage( u16 color, const u8 *str )
{
	msgStat.vcount	 = MSG_BLINK_COUNT;
	msgStat.flag	 = 1;
	msgStat.color	 = color;
	msgStat.str		 = str;
}


// メッセージ表示
static void DispBlinkMessage( void )
{
	if( ( msgStat.vcount == MSG_BLINK_COUNT ) && msgStat.flag ) {
		msgStat.handle = DrawStringSJIS( 4, 21, msgStat.color, msgStat.str );
	}
	
	if ( --msgStat.vcount == 0 ) {
		ClearStringSJIS_handle( msgStat.handle );
		msgStat.flag 	^= 0x01;
		msgStat.vcount	 = MSG_BLINK_COUNT;
	}
}


//=============================================================================
// Nintendoロゴ表示
//=============================================================================

// Nintendoロゴ表示の初期化
static void InitDispNintendoLogo( void )
{
	int i;
	u16 *palettep = (u16 *)(HW_OBJ_PLTT + 0x20 * 15 ) + 1;		// パレット15のカラー1に。
	*palettep++ = 0x7fff;
	*palettep   = 0x1111;
	
	// TOP画面の全表示クリア
	SetTargetScreenSJIS( TOP_SCREEN );
	ClearAllStringSJIS();
	SetTargetScreenSJIS( BOTTOM_SCREEN );
	
	// 画面上に受信したゲームのNintendoロゴを表示する。
#ifdef NINLOGO_LOAD_1D_CHAR
	// 1Dマッピングでのロード＆表示
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
	SYSM_LoadNintendoLogo1D( (u16 *)SYSM_GetCardRomHeader()->nintendo_logo, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
//	SYSM_LoadNintendoLogo1D( (u16 *)SYSROM9_NINLOGO_ADR, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
	
	for ( i = 0; i < 3; i++ ) {
		G2_SetOBJAttr( &oamBakM[ 10 + i ],						// OAM pointer
					72 + 32 * i,								// X position
					88,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_32x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x2 + i * 4,								// charactor
					15,											// palette
					0);											// affine
	}
	{
		G2_SetOBJAttr( &oamBakM[ 13 ],							// OAM pointer
					72 + 32 * 3,								// X position
					88,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_8x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x2 + 3 * 4,								// charactor
					15,											// palette
					0);											// affine
	}
	for ( i = 0; i < 3; i++ ) {
		G2_SetOBJAttr( &oamBakM[ 14 + i ],						// OAM pointer
					72 + 32 * i,								// X position
					96,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_32x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x0f + i * 4,								// charactor
					15,											// palette
					0);											// affine
	}
	{
		G2_SetOBJAttr( &oamBakM[ 17 ],							// OAM pointer
					72 + 32 * 3,								// X position
					96,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_8x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0xf + 3 * 4,								// charactor
					15,											// palette
					0);											// affine
	}
#else
	// 2Dマッピングでのロード＆表示
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_2D );
	SYSM_LoadNintendoLogo2D( (u16 *)SYSM_GetCardRomHeader()->nintendo_logo, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
//	SYSM_LoadNintendoLogo2D( (u16 *)SYSROM9_NINLOGO_ADR, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
	
	for ( i = 0; i < 4; i++ ) {
		G2_SetOBJAttr( &oamBakM[11 + i],						// OAM pointer
					72 + 32 * i,								// X position
					88,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_32x16,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x2 + i * 4,								// charactor
					15,											// palette
					0);											// affine
	}
#endif
	
	loop_count = 120;
}


// Nintendoロゴ表示
static BOOL DispNintendoLogo( void )
{
	if( --loop_count == 0 ) {
		{
			int i;
			for ( i = 0; i < 4; i++ ) {
				G2_SetOBJPosition( &oamBakM[ 11 + i ], 0, 192 );
			}
		}
		return TRUE;
	}
	return FALSE;
}


//=============================================================================
// その他サブルーチン
//=============================================================================

// 自分のユーザー情報をNITRO設定データから読み出してセット
static void SetMyUserInfo( MBUserInfo *my )
{
	NvNickname *nickname = NCD_GetNickname();
	
	MI_CpuCopy16( nickname->str, my->name, (u32)( nickname->length * 2 ) );
	my->nameLength		= nickname->length;
	my->playerNo		= 0;										// とりあえず０にする。
	my->favoriteColor	= NCD_GetFavoriteColor();
}


// 現在スキャンロック中かどうかを取得する。
static BOOL IsScanLock(void)
{
	return mbrsp->nowScanTargetFlag ? TRUE : FALSE;
}


//=============================================================================
// 割り込みルーチン
//=============================================================================

// Vブランク割り込み
static void VBlankIntr_WDL(void)
{
	// OAM, BG-VRAMの更新
	DC_FlushRange (oamBakM,		sizeof(oamBakM));
	DC_FlushRange (oamBakS,		sizeof(oamBakS));
	DC_FlushRange (bgBakM,		sizeof(bgBakM));
	DC_FlushRange (bgBakS,		sizeof(bgBakS));
	DC_FlushRange (objVramBuff,	sizeof(objVramBuff));
	MI_CpuCopyFast(oamBakM,		(void*)HW_OAM,					 sizeof(oamBakM));
	MI_CpuCopyFast(oamBakS,		(void*)HW_DB_OAM,				 sizeof(oamBakS));
	MI_CpuCopyFast(bgBakM,		(void*)(HW_BG_VRAM + 0xf000),    sizeof(bgBakM));
	MI_CpuCopyFast(bgBakS,		(void*)(HW_DB_BG_VRAM + 0xf000), sizeof(bgBakS));
	MI_CpuCopyFast(objVramBuff,	(void*)HW_DB_OBJ_VRAM,			 sizeof(objVramBuff));
	//---- 割り込みチェックフラグ
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


//=============================================================================
// プログラムシーケンスキュー処理
//=============================================================================

// キュー初期化
static void InitPrgNextSeqQue( void )
{
	MI_CpuClear16( &nextSeq, sizeof(nextSeq) );
}


// 次のPrgSeqをキューにセットする。
static BOOL SetPrgNextSeqQue( PrgSeq seq )
{
	BOOL preIRQ = OS_DisableIrq();
	BOOL retval = FALSE;
	
	if( nextSeq.num != PRG_NEXT_SEQ_QUE_NUM ) {
		nextSeq.seq[ nextSeq.bottom ] =seq;
		nextSeq.num++;
		nextSeq.bottom++;
		if( nextSeq.bottom == PRG_NEXT_SEQ_QUE_NUM ) {
			nextSeq.bottom = 0;
		}
		retval = TRUE;
	}
	(void)OS_RestoreIrq( preIRQ );
	return retval;
}


// 次のPrgSeqをキューから取り出す
static BOOL GetPrgNextSeqQue( PrgSeq *seqp )
{
	BOOL preIRQ = OS_DisableIrq();
	BOOL retval = FALSE;
	
	if( ( !nextSeq.lock ) && ( nextSeq.num != 0 ) ) {				// キューがロックされておらず、かつ次Seqが存在するなら、キューから取り出し。
		*seqp = nextSeq.seq[ nextSeq.top ];
		nextSeq.num--;
		nextSeq.top++;
		if( nextSeq.top == PRG_NEXT_SEQ_QUE_NUM ) {
			nextSeq.top = 0;
		}
		retval = TRUE;
	}
	(void)OS_RestoreIrq( preIRQ );
	return retval;
}


// キューをロックする。（ロック中はGetPrgNextSeqQueで次Seqを取り出せなくなります。）
static void LockPrgNextSeqQue(void)
{
	nextSeq.lock = TRUE;
}


// キューのロック解除。
static void UnlockPrgNextSeqQue(void)
{
	nextSeq.lock = FALSE;
}

// キューが空か？
static BOOL IsEmptyPrgNextSeqQue(void)
{
	return nextSeq.num ? FALSE : TRUE;
}



static const u32 searchCode[] = {
	0xe59f1028,
	0xe59f2028,
	0xe1d200b0,
	0xe2100001,
	0x1afffffc,
	0xe1d100b0,
	0xe3500006,
	0x0afffff9,
	0xe1d100b0,
	0xe3500005,
	0x0afffff6,
	0xe12fff1e,
	0x04808214,
	0x0480819c,
};


void SearchPatchAddress( void )
{
	MBDownloadFileInfo *dlfinfo = (MBDownloadFileInfo *)MB_DOWNLOAD_FILEINFO_ADDRESS;
	BOOL find = FALSE;
	u32 *tgtp = (u32 *)MB_ARM7_STATIC_RECV_BUFFER;
	u32 size  = dlfinfo->seg[ 2 ].size >> 2;
	
	while( size-- ) {
		if( *tgtp++ == searchCode[0] ) {
			u32 *srcp   = (u32 *)&searchCode[ 1 ];
			u32 *checkp = tgtp ;
			int i = sizeof( searchCode ) / sizeof(u32) - 1;
			while( i-- ) { 
				if( *srcp++ != *checkp++ ) break;
			}
			if( i < 0 ) {
				u32 addr = (u32)tgtp - 0x04 + 0x18;
				OS_TPrintf("Found! -> 0x%08x\n", addr );
				find = TRUE;
			}
		}
	}
	if( !find ) {
		OS_TPrintf("Not found.\n");
	}
}

