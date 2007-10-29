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
#include "main.h"
#include "logoDemo.h"
#include "DS_Setting.h"
#include "DS_DownloadPlay.h"
#include "DS_Chat.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void InitAllocator( NNSFndAllocator* pAllocator );
static void InitAllocSystem( void );
static BOOL CheckBootStatus( void );
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------
NNSFndAllocator g_allocator;

// static variable-------------------------------------------------------------
static BannerFile banner;											// バナーデータ

// const data------------------------------------------------------------------

#if 0
typedef struct CardStatus {
	u16		primarySlot;		// PULLOUT, DETECT, VALID, INVALID
	u16		secondarySlot;		// 同上。
}CardStatus;

typedef struct TitleProperty {	// この情報は、ランチャー時には認証通ってないけど、起動時には認証通すので大丈夫だろう。
	u64		titleID;		// アプリケーション識別ID
	u32		platform;		// NTR, TWL  (HYBLIDはTWLを返す）
	void	*pBanner;		// 固定長フォーマットなら偽造されても大丈夫だろう。
}TitleProperty;


void TwlMain( void )
{
	u32 state = START;
	u32 filter_flag;
	TitleProperty *pBootTitle = NULL;

	// 初期化
	SYSM_Init();	// SYSM_CreateCardThread();も含む
	
	// 本体設定データのリード
	SYSM_ReadTWLSetting( pTWLSetting );
	
	// リセットパラメータの取得。（PMICの値＆メインメモリの値）
	SYSM_GetResetParam( pResetParam );
	if( pResetParam->pBootTitle ) {							// アプリ直接起動の指定があったらロゴデモを飛ばして起動
		pBootTitle = pResetParam->pBootTitle;
		state = BOOT;
	}
	
	// NANDアプリリストの取得
	filter_flag = ALL_APP;
	SYSM_GetNandTitleList( pTitleList_Nand, filter_flag );	// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, 等の条件を指定してタイトルリストを取得する。
															// return : *TitleProperty Array
	// コンテント（リソース）ファイルのリード
	SYSM_ReadContentFile( ContentID );
	// 共有コンテントファイルのリード
	SYSM_ReadSharedContentFile( ContentID );
	
	while( 1 ) {
		CardStatus cardStatus = SYSM_GetCardTitleList( pTitleList_Card );		// カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
		
		switch( state ) {
		case START:
			LogoInit();
			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoDemo() ) {
				LauncherInit( pTitleList_Nand, pTitleList_Card );
				state = LAUNCHER;
			}
			break;
		case LAUNCHER:
			pBootTitle = Launcher( pTitleList_Card, cardStatus );
			if( pBootTitle ) {
				state = BOOT;
			}
			break;
		case BOOT:
			if( pBootTitle ) {
				if( SYSM_CheckTitlePointer( pBootTitle ) &&		// ポインタチェック
					SYSM_AuthAndLoadTitle ( pBootTitle ) ) {	// ROMヘッダ認証
					SYSM_Finalize();							// 終了処理
					return;
				}
				state = STOP;
			}
			break;
		case STOP:
			break;
		}
	}
#endif

extern void SampleMain(void);

// ============================================================================
// function's description
// ============================================================================
void TwlMain(void)
{
	typedef enum PrgState {
		STATE_START = 1,
		STATE_LOGO_DISP,
		STATE_LOGO_MENU,
		STATE_WAIT_BOOT
	}PrgState;
	
	PrgState prg_state     = STATE_START;
	BOOL     boot_decision = FALSE;
	
	// 初期化----------------------------------
	SYSM_Init();													// システムメニュー関連データの初期化（TwlMainの先頭でコールして下さい。）
	
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	GX_SetPower(GX_POWER_ALL);										// 各ロジック パワーON
	
	// 割り込み許可----------------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// デバイス初期化-------------------------------
#ifndef __TP_OFF
	TP_Init();
#endif
	(void)RTC_Init();
	
	// システムの初期化------------------
	InitAllocator( &g_allocator );
	CMN_InitFileSystem( &g_allocator );

//	InitAllocSystem();
	
	// ARM7初期化待ち--------------------------
	if( SYSM_WaitARM7Init() ) {										// ARM7側の初期化が終わるのを待ってからメインループ開始
		return;														// TRUEが返されたら、デバッガブートなのでリターン
	}
	
	// メインループ----------------------------
	while(1){
		OS_WaitIrq(1, OS_IE_V_BLANK);								// Vブランク割り込み待ち
		ReadKeyPad();												// キー入力の取得
		
		if(SYSM_IsTPReadable()) {
			ReadTpData();											// TP入力の取得
		}
		
//		if(SYSM_Main()) {											// IPL2システムのメイン
//			return;													// TRUEが帰ってきたらメインループからリターン（NITROゲーム起動等）
//		}
		
		switch(prg_state) {
		  case STATE_START:
			boot_decision = CheckBootStatus();						// ブート状態をチェックする。（ショートカット起動やコンパイルスイッチによる強制起動）
//			if( !SYSM_GetBannerFile( &banner ) ) {					// バナーデータのリード
//				OS_Printf("ROM banner data read failed.\n");
//			}
			prg_state = STATE_LOGO_DISP;
			break;
			
			//-----------------------------------
			// NITROロゴ表示
			//-----------------------------------
		  case STATE_LOGO_DISP:
			// 自動起動ONの時のキーショートカット処理
			if( GetNCDWork()->option.autoBootFlag ) {				// TPタッチされるか、Bボタンが押下されたら今回の自動起動をOFFにする。
				
				ReadTpDataLogoDirectBootCancel();					// ※red_ipl2特有の処理。red_ipl2のTP仕様は特許にひっかかっているので使えないが、ここだけは有効にしたいので、無理やり実装。
				
				if( (tpd.disp.touch) || (pad.trg & PAD_BUTTON_B) ) {
					boot_decision = 0;
					SYSM_ClearBootFlag( BFLG_BOOT_NITRO | BFLG_BOOT_AGB | BFLG_BOOT_BMENU );
				}
			}
			
			if( LogoMain() ) {										// ロゴ表示ルーチン（※BFLG_GAMEBOY_LOGO_OFFの時は即終了）
				InitBG();										// BG初期化
				LauncherInit();									// ブート未決定時のみロゴメニューを初期化する。
				
				prg_state = STATE_LOGO_MENU;
			}
			break;													// ※NITROカードが正当でない場合は、このまま無限ループ。
			
			//-----------------------------------
			// ロゴメニューで起動モード選択
			//-----------------------------------
		  case STATE_LOGO_MENU:
			{
				IPL2BootType command = LauncherMain( boot_decision );
				
				switch(command) {
				case BOOT_TYPE_UNSOLVED:
					break;
					
				case BOOT_TYPE_NITRO:
//					if( !SYSM_BootNITRO() ) {
//						(void)DrawStringSJIS( 4,  20, RED, (const u8 *)"This NITRO card is invalid!!");
//					}
					break;
					
				case BOOT_TYPE_PICT_CHAT:
//					(void)SYSM_BootPictChat();
					break;
					
				case BOOT_TYPE_WIRELESS_BOOT:
//					(void)SYSM_BootDSDownloadPlay();
					break;
					
				case BOOT_TYPE_BMENU:
//					(void)SYSM_BootMachineSetting();
					break;
					
				default:
					OS_Panic( "ERROR: boot code failed : %d\n", command );
				}
				if(command) {
					prg_state = STATE_WAIT_BOOT;
				}
			}
			break;
		  case STATE_WAIT_BOOT:
//			SYSM_PermitToBootSelectedTarget();
			break;
		}
		
		if ( PAD_DetectFold() == TRUE ) {							// スリープモードへの遷移
//			SYSM_FinalizeCardPulledOut();
			SYSM_GoSleepMode();
//			(void)SYSM_IsCardPulledOut();							// カード抜け検出コマンド発行
//			SYSM_FinalizeCardPulledOut();
																	// カード抜け検出
//			if ( SYSM_IsCardPulledOut() ) {
			if ( 0 ) {
				(void)PM_ForceToPowerOff();
			}
		}
		
//		if (SYSM_IsCardPulledOut()) {								// カード抜け検出
		if ( 0 ) {
			OS_Printf("Card is pulled out.\n");
			OS_Terminate();
		}
		
		OS_PrintServer();											// ARM7からのプリントデバッグを処理する
		
		//---- BG-VRAMの更新
//		DC_FlushRange ( bgBakS,  sizeof(bgBakS) );
//		MI_CpuCopyFast( bgBakS, (void*)(HW_DB_BG_VRAM+0xf000), sizeof(bgBakS) );
	}
}


// アロケータの初期化
static void InitAllocator( NNSFndAllocator* pAllocator )
{
    u32   arenaLow      = MATH_ROUNDUP  ((u32)OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 16);
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo(heapSize, 16);
    NNSFndHeapHandle    heapHandle;
    SDK_NULL_ASSERT( pAllocator );

    heapHandle = NNS_FndCreateExpHeap(heapMemory, heapSize);
    SDK_ASSERT( heapHandle != NNS_FND_HEAP_INVALID_HANDLE );

    NNS_FndInitAllocatorForExpHeap(pAllocator, heapHandle, 4);
}

#if 0
// mallocシステムの初期化
static void InitAllocSystem(void)
{
	void*			tempLo;
	OSHeapHandle	hh;
	
	tempLo	= OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 16);
	OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
	OS_TPrintf( "ArenaLo : %08x  ArenaHi : %08x\n", OS_GetMainArenaLo(), OS_GetMainArenaHi() );
	
	hh		= OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
	if(hh < 0) {
		OS_Panic("ARM9: Fail to create heap...\n");
	}
	hh		= OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}
#endif

// ブート状態を確認し、ロゴ表示有無を判断する-------
static BOOL CheckBootStatus(void)
{
	BOOL boot_decision		= FALSE;								// 「ブート内容未定」に
	BOOL other_shortcut_off	= FALSE;
	
	//-----------------------------------------------------
	// デバッグ用コンパイルスイッチによる挙動
	//-----------------------------------------------------
	{
#ifdef __FORCE_BOOT_BMENU											// ※ブートメニュー強制起動スイッチがONか？
		SYSM_SetBootFlag( BFLG_BOOT_BMENU );
		return TRUE;												// 「ブート内容決定」でリターン
#endif /* __FORCE_BOOT_BMENU */
		
#ifdef __LOGO_SKIP													// ※デバッグ用ロゴスキップ
		SetLogoEnable( FALSE );										// ロゴ表示スキップ
#endif /* __LOGO_SKIP */
	}
	
	
	//-----------------------------------------------------
	// NITRO設定データ未入力時の設定メニューショートカット起動
	//-----------------------------------------------------
#ifdef __DIRECT_BOOT_BMENU_ENABLE									// ※NITRO設定データ未入力時のブートメニュー直接起動スイッチがONか？
	if( ( (GetNCDWork()->option.input_tp == 0)
		||(GetNCDWork()->option.input_language == 0)
		||(GetNCDWork()->option.input_rtc == 0)
		||(GetNCDWork()->option.input_favoriteColor == 0)
		||(GetNCDWork()->option.input_nickname == 0) ) ) {		// TP,言語,RTC,ニックネームがセットされていなければ、ロゴ表示もゲームロードも行わず、ブートメニューをショートカット起動。
		
		if( ( pad.cont & PAD_PRODUCTION_NITRO_SHORTCUT ) == PAD_PRODUCTION_NITRO_SHORTCUT ) {
			other_shortcut_off = TRUE;								// 量産工程用のキーショートカットが押されていたら、設定メニュー起動はなし。
		}else if( !SYSM_IsInspectNITROCard() )  {					// 但し、量産用のキーショートカットが押されている時か、NITRO検査カードがささっている時は、ブートメニューへのショートカット起動は行わない。
			SYSM_SetBootFlag( BFLG_BOOT_BMENU );
			SetLogoEnable( FALSE );
			return TRUE;											// 「ブート内容決定」でリターン
		}
	}
#endif /* __DIRECT_BOOT_BMENU_ENABLE */
	
	
	//-----------------------------------------------------
	// キーショートカット起動
	//-----------------------------------------------------
	if( !other_shortcut_off && !GetNCDWork()->option.autoBootFlag ) {
																	// 他ショートカットONかつオート起動OFFの時
		u32 nowBootFlag = 0;
		
		if(pad.cont & PAD_BUTTON_R){								// Rボタン押下起動なら、ロゴ表示なしでAGBゲームへ
			SetLogoEnable( FALSE );
			nowBootFlag = BFLG_BOOT_AGB;
		}else if(pad.cont & PAD_BUTTON_L){							// Lボタン押下起動なら、ロゴ表示後にNITROゲームへ
			nowBootFlag = BFLG_BOOT_NITRO;
		}else if(pad.cont & PAD_BUTTON_B){							// Bボタン押下起動なら、ロゴ表示後にブートメニューへ
			nowBootFlag = BFLG_BOOT_BMENU;
		}
		if( nowBootFlag ) {
			SYSM_SetBootFlag( nowBootFlag );
			return TRUE;											// 「ブート内容決定」でリターン
		}
	}
	
	
	//-----------------------------------------------------
	// 自動起動オプション有効時の挙動
	//-----------------------------------------------------
#ifndef __SYSM_DEBUG
	if( GetNCDWork()->option.autoBootFlag ) {
		if ( SYSM_IsNITROCard() ) {									// NITROカードのみの時はNITRO起動
			SYSM_SetBootFlag( BFLG_BOOT_NITRO );
			return TRUE;											// 「ブート内容決定」でリターン
		}
	}
#endif /* __SYSM_DEBUG */
	
	return FALSE;													// 「ブート内容未定」でリターン
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

