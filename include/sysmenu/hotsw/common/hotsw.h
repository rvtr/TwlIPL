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

// enum   -------------------------------------------------------------------
// スレッドに送るメッセージのステート
typedef enum HotSwMessageType{
	HOTSW_INSERT = 0,
    HOTSW_PULLOUT,
    HOTSW_CONTROL
} HotSwMessageType;

typedef enum ModeType{
	HOTSW_MODE1,
    HOTSW_MODE2
} ModeType;

// union  -------------------------------------------------------------------
// PXI用メッセージ
typedef union HotSwPxiMessage{
    struct {
    	u32		value	:1;
    	u32		ctrl	:1;
    	u32		:30;
    } msg;
    u32 data;
} HotSwPxiMessage;

// struct -------------------------------------------------------------------
// スレッド用メッセージ
typedef struct HotSwMessage{
    u32				 value;
    BOOL			 ctrl;
	HotSwMessageType type;
} HotSwMessage;


// Function prototype -------------------------------------------------------
// 活栓挿抜処理の初期化
void HOTSW_Init(void);

// カードの存在判定
BOOL HOTSW_IsCardExist(void);

// カードにアクセスできる状態か判定
BOOL HOTSW_IsCardAccessible(void);

// Boot Segment バッファの指定
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment バッファの指定
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

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

#endif  // __SYSMENU_HOTSW_COMMON_HOTSW_H__
