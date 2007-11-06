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
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"

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

#if 1

typedef struct CardStatus {
	u16		primarySlot;		// PULLOUT, DETECT, VALID, INVALID
	u16		secondarySlot;		// 同上。
}CardStatus;


void TwlMain( void )
{
	enum {
		START = 0,
		LOGODEMO = 1,
		LAUNCHER_INIT = 2,
		LAUNCHER = 3,
		AUTHENTICATE = 4,
		BOOT = 5,
		STOP = 6
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty pTitleList[TITLE_PROPERTY_NUM];
	
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	
	// 割り込み許可--------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// システムの初期化----------------
	InitAllocator( &g_allocator );
	CMN_InitFileSystem( &g_allocator );
	
	// システムメニュー初期化----------
	SYSM_Init( Alloc, Free );											// OS_Initの後でコール。
	
	// リセットパラメータの取得--------
	if( SYSM_GetResetParam()->isLogoSkip ) {
		if( SYSM_GetResetParam()->bootTitleID ) {							// アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
			pBootTitle = (TitleProperty *)SYSM_GetResetParam();
			state = AUTHENTICATE;
		}else {																// それ以外の場合は、ロゴデモを飛ばしてランチャー起動
			state = LAUNCHER_INIT;
		}
	}
	
	// コンテント（リソース）ファイルのリード
//	FS_ReadContentFile( ContentID );
	
	// 共有コンテントファイルのリード
//	FS_ReadSharedContentFile( ContentID );
	
	// NANDアプリリストの取得----------
	(void)SYSM_GetNandTitleList( pTitleList, TITLE_PROPERTY_NUM );
	
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// Vブランク割り込み待ち
		
		ReadKeyPad();											// キー入力の取得
		ReadTpData();											// TP入力の取得
		
		(void)SYSM_GetCardTitleList( pTitleList );				// カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
		
		switch( state ) {
		case START:
			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoMain() ) {
				state = LAUNCHER_INIT;
			}
			break;
		case LAUNCHER_INIT:
			InitBG();										// BG初期化
			LauncherInit( pTitleList );
			state = LAUNCHER;
			break;
		case LAUNCHER:
			pBootTitle = LauncherMain( pTitleList );
			if( pBootTitle ) {
				state = AUTHENTICATE;
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_LoadAndAuthenticateTitle( pBootTitle ) ) {	// アプリロード＆認証
			case AUTH_PROCESSING:
				break;
			case AUTH_RESULT_SUCCEEDED:
				state = BOOT;
				break;
			case AUTH_RESULT_TITLE_POINTER_ERROR:
			case AUTH_RESULT_AUTHENTICATE_FAILED:
			case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
				state = STOP;
				break;
			}
			break;
		case BOOT:
			SYSM_Finalize();									// 終了処理
			return;
		case STOP:												// 停止
			break;
		}
	}
}
#endif


// ============================================================================
// function's description
// ============================================================================
/*
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
//	if( SYSM_WaitARM7Init() ) {										// ARM7側の初期化が終わるのを待ってからメインループ開始
//		return;														// TRUEが返されたら、デバッガブートなのでリターン
//	}
	
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
*/
	

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

    NNS_FndInitAllocatorForExpHeap(pAllocator, heapHandle, 32);
}


// メモリ割り当て
void *Alloc( u32 size )
{
	return NNS_FndAllocFromAllocator( &g_allocator, size );
}


// メモリ解放
void Free( void *pBuffer )
{
	NNS_FndFreeToAllocator( &g_allocator, pBuffer );
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
	if( !TSD_IsSetTP() ||
		!TSD_IsSetLanguage() ||
		!TSD_IsSetDateTime() ||
		!TSD_IsSetUserColor() ||
		!TSD_IsSetNickname() ) {									// TP,言語,RTC,ニックネームがセットされていなければ、ロゴ表示もゲームロードも行わず、ブートメニューをショートカット起動。
		
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
	if( !other_shortcut_off
//		&& !TSD_IsAutoBoot()
		) {
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
//	if( TSD_IsAutoBoot() ) {
	if( 0 ) {
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

