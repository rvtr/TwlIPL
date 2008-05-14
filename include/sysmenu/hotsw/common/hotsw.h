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

#define SYSM_HOTSW_ENABLE_ROMEMU
//#define USE_WRAM_LOAD

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

typedef enum HotSwApliType{
    HOTSW_APLITYPE_CARD = 0,
    HOTSW_APLITYPE_NTR_NAND,
    HOTSW_APLITYPE_TWL_NAND
} HotSwApliType;

// union  -------------------------------------------------------------------
// PXI用メッセージ
typedef union HotSwPxiMessage{
    struct {
    	u32		value	:1;
    	u32		ctrl	:1;
        u32		finalize:1;
        u32		read	:1;
        u32		bootType:8;
    	u32		:20;
    } msg;
    u32 data;
} HotSwPxiMessage;

// struct -------------------------------------------------------------------
// スレッド用メッセージ
typedef struct HotSwMessage{
    u32				 value;
    BOOL			 ctrl;
    BOOL			 finalize;
    BOOL			 read;
	HotSwMessageType type;
    HotSwApliType    apli;
} HotSwMessage;


// Function prototype -------------------------------------------------------
// --- ARM9
#ifdef SDK_ARM9
// PXI通信でARM7に活線挿抜有効／無効を通知
void HOTSW_EnableHotSWAsync( BOOL enable );

// PXI通信でARM7に活線挿抜Finalize処理を通知
void HOTSW_FinalizeHotSWAsync( HotSwApliType apliType );

// 活線挿抜の許可/抑制の状態を返す
BOOL HOTSW_isEnableHotSW(void);

// カードアプリのロードが完了しているかを返す
BOOL HOTSW_isCardLoadCompleted(void);

#ifdef USE_WRAM_LOAD
// カードデータを読み出す関数
CardDataReadState HOTSW_ReadCardData(void* src, void* dest, u32 size);

// カードがゲームモードになったかどうか
BOOL HOTSW_isGameMode(void);
#endif

// --- ARM7
#else
// 活栓挿抜処理の初期化
void HOTSW_Init(u32 threadPrio);

// カードの存在判定
BOOL HOTSW_IsCardExist(void);

// カードにアクセスできる状態か判定
BOOL HOTSW_IsCardAccessible(void);

// Boot Segment バッファの指定
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment バッファの指定
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

// ISデバッガ上で動作しているか？
BOOL HOTSWi_IsRunOnDebugger(void);

// ROMをエミュレーションしているか？
BOOL HOTSWi_IsRomEmulation(void);

// デバッガ通信用にカードスロットの電源をONにする。
void HOTSWi_TurnCardPowerOn(u32 slot);
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
