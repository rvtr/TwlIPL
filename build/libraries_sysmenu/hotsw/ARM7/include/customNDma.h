/*---------------------------------------------------------------------------*
  Project:  TwlIPL - HOTSW - include
  File:     customNDma.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_CUSTOM_NDMA_H__
#define __HOTSW_CUSTOM_NDMA_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif


static inline void HOTSW_WaitCardCtrl(void)
{
	while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){}
}

#ifdef USE_NEW_DMA
static inline void HOTSW_WaitNDmaCtrl(u32 ndmaNo)
{
	while( MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) & MI_NDMA_ENABLE_MASK ){}
}
#else
static inline void HOTSW_WaitDmaCtrl(u32 dmaNo)
{
	while( MI_DMA_REG( dmaNo, MI_DMA_REG_CNT_WOFFSET ) & MI_DMA_ENABLE ){}
}
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================
#ifdef USE_NEW_DMA
// New DMA
void HOTSW_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size);
void HOTSW_NDmaPipe_Card(u32 ndmaNo, const void *src, void *dest, u32 size);
#else
// Old DMA
void HOTSW_DmaCopy32_Card(u32 dmaNo, const void *src, void *dest, u32 size);
void HOTSW_DmaPipe32_Card(u32 dmaNo, const void *src, void *dest, u32 size);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_CUSTOM_NDMA_H__