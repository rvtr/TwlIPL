/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     hotsw.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef __SYSMENU_HOTSW_COMMON_HOTSW_H__
#define __SYSMENU_HOTSW_COMMON_HOTSW_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define -------------------------------------------------------------------
#define SYSM_HOTSW_ENABLE_ROMEMU
#define USE_WRAM_LOAD
#define INITIAL_KEYTABLE_PRELOAD
#define HOTSW_FINAL_VERSION
//#define USE_NEW_DMA
//#define HOTSW_NO_MESSAGE					// Printf抑制スイッチ

#ifndef SDK_FINALROM
	#ifdef  HOTSW_NO_MESSAGE
		#define HOTSW_TPrintf( ... )        ((void)0)
		#define HOTSW_PutString( ... )      ((void)0)
	#else
		#define HOTSW_TPrintf				OS_TPrintf
		#define HOTSW_PutString				OS_PutString
	#endif
#else
	#define HOTSW_TPrintf( ... )        	((void)0)
	#define HOTSW_PutString( ... )      	((void)0)
#endif

// enum   -------------------------------------------------------------------
// スレッドに送るメッセージのステート
typedef enum HotSwMessageType{
	HOTSW_INSERT = 0,
    HOTSW_PULLOUT,
    HOTSW_CONTROL
} HotSwMessageType;

typedef enum ModeType{
	HOTSW_MODE1 = 0,
    HOTSW_MODE2
} ModeType;

typedef enum HotSwCardState{
    HOTSW_CARD_STATE_POWER_OFF = 0,
    HOTSW_CARD_STATE_NORMAL_MODE,
    HOTSW_CARD_STATE_GAME_MODE,
    HOTSW_CARD_STATE_KEEP
} HotSwCardState; // 旧 HotSwApliType

// union  -------------------------------------------------------------------
// PXI用メッセージ
typedef union HotSwPxiMessageForArm7{
    struct {
    	u32		value	 :1;
    	u32		ctrl	 :1;
        u32		finalize :1;
        u32		read	 :1;
        u32		cardState:8;
    	u32		:20;
    } msg;
    u32 data;
} HotSwPxiMessageForArm7;

typedef union HotSwPxiMessageForArm9{
    struct {
    	u32		mode	:1;
    	u32		insert	:1;
        u32		pullout :1;
        u32		read	:1;
        u32		result  :8;
    	u32		:20;
    } msg;
    u32 data;
} HotSwPxiMessageForArm9;

// struct -------------------------------------------------------------------
// スレッド用メッセージ
typedef struct HotSwMessageForArm7{
    u32				 value;
    BOOL			 ctrl;
    BOOL			 finalize;
    BOOL			 read;
	HotSwMessageType type;
    HotSwCardState   state;
} HotSwMessageForArm7;

typedef struct HotSwMessageForArm9{
    BOOL			 	isGameMode;
    BOOL			 	isInsert;
    BOOL			 	isPulledOut;
    BOOL				isReadComplete;
	CardDataReadState 	result;
} HotSwMessageForArm9;


// Function prototype -------------------------------------------------------
// --- ARM9
#ifdef SDK_ARM9
// PXI通信でARM7に活線挿抜有効／無効を通知。カード関連のフラグ処理も行う(非同期版)
void HOTSW_EnableHotSWAsync( BOOL enable );

// PXI通信でARM7に活線挿抜有効／無効を通知。カード関連のフラグ処理も行う(同期版)
void HOTSW_EnableHotSW( BOOL enable );

// PXI通信でARM7に活線挿抜無効を通知。(非同期版)
void HOTSW_InvalidHotSWAsync( void );

// PXI通信でARM7に活線挿抜無効を通知。(同期版)
void HOTSW_InvalidHotSW( void );

// PXI通信でARM7に活線挿抜Finalize処理を通知
void HOTSW_FinalizeHotSWAsync( HotSwCardState cardState );

// 活線挿抜の許可/抑制の状態を返す
BOOL HOTSW_isEnableHotSW(void);

// カードアプリのロードが完了しているかを返す
BOOL HOTSW_isCardLoadCompleted(void);

//  終了処理が完了したかを返す
BOOL HOTSW_isFinalized(void);

// 活線挿抜処理中かどうかを返す
BOOL HOTSW_isBusyHotSW(void);

#ifdef USE_WRAM_LOAD
// 活栓挿抜処理の初期化
void HOTSW_Init();

// カードデータを読み出す関数(同期版)
CardDataReadState HOTSW_ReadCardData(void* src, void* dest, u32 size);

// カードデータを読み出す関数(非同期版)
CardDataReadState HOTSW_ReadCardDataAsync(void* src, void* dest, u32 size);

// カードがGameモードになった時のコールバック関数を設定
void HOTSW_SetGameModeCallBackFunction(OSIrqFunction function);

// カードが刺さった時のコールバック関数を設定
void HOTSW_SetCardInsertCallBackFunction(OSIrqFunction function);

// カードが抜けた時のコールバック関数を設定
void HOTSW_SetCardPullOutCallBackFunction(OSIrqFunction function);

// カードがゲームモードになったかどうか
BOOL HOTSW_isGameMode(void);

// Gameモードのデータ読み関数
CardDataReadState HOTSW_ReadCardDataOnGameMode(const void* src, void* dest, u32 size);
#endif

// --- ARM7
#else
// 活栓挿抜処理の初期化
void HOTSW_Init(u32 threadPrio);

#ifdef INITIAL_KEYTABLE_PRELOAD
// DS互換BlowfishテーブルをWRAM経由でローカルにコピーする
void HOTSW_CopyInitialKeyTable(void);
#endif

// カードの存在判定
BOOL HOTSW_IsCardExist(void);

// カードにアクセスできる状態か判定
BOOL HOTSW_IsCardAccessible(void);

// Boot Segment バッファの指定
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment バッファの指定
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);
// 上記のバッファ非クリアバージョン
void HOTSWi_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

// ROMをエミュレーションしているか？
BOOL HOTSWi_IsRomEmulation(void);

// デバッガ通信用にカードスロットの電源をONにする。
void HOTSWi_TurnCardPowerOn(u32 slot);

// カードワーク取得
void *HOTSWi_GetCardBootData(void);

#ifdef USE_WRAM_LOAD
// NANDアプリ用KeyTableの生成
void HOTSWi_MakeBlowfishTableDSForNAND(void);
// 引数で与えられたバッファから2KB分の領域をBlowfishで復号化する
BOOL HOTSW_DecryptObjectFile(void* dest);
#endif
#endif

// Romエミュレーション情報を格納しているバッファのポインタを返す
SDK_INLINE void* HOTSW_GetRomEmulationBuffer(void)
{
	return (void*)&SYSMi_GetWork()->romEmuInfo;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __SYSMENU_HOTSW_COMMON_HOTSW_H__
