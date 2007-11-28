/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     Card.h
 *---------------------------------------------------------------------------*/
#ifndef __MY_CARD_H__
#define __MY_CARD_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototype -------------------------------------------------------
// 活栓挿抜処理の初期化
void HOTSW_Init(void);

// カード起動。Normalモード→Secureモード→Gameモードを行う
BOOL HOTSW_Boot(void);

// ARM7,9の常駐モジュールを展開する関数
void HOTSW_LoadStaticModule(void);

// Boot Segment バッファの指定
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment バッファの指定
void HOTSW_SetSecureSegmentBuffer(void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __MY_CARD_H__