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

#define SYSM_HOTSW_ENABLE_ROMEMU

// Function prototype -------------------------------------------------------
// 活栓挿抜処理の初期化
void HOTSW_Init(void);

// カードの存在判定
 BOOL HOTSW_IsCardExist(void);

// Boot Segment バッファの指定
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment バッファの指定
void HOTSW_SetSecureSegmentBuffer(void* buf, u32 size);

// Romエミュレーション情報を格納しているバッファのポインタを返す
#ifdef SDK_ARM7
void* HOTSW_GetRomEmulationBuffer(void);
#else // SDK_ARM9
SDK_INLINE void* HOTSW_GetRomEmulationBuffer(void)
{
	return (void*)HW_ISD_RESERVED;
}
#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __MY_CARD_H__