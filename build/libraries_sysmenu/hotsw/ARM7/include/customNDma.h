#ifndef __HOTSW_CUSTOM_NDMA_H__
#define __HOTSW_CUSTOM_NDMA_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif


static inline void HOTSW_WaitCardCtrl(void)
{
	while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){
//		OS_PutString("Card is busy..\n");
    }
}

static inline void HOTSW_WaitDmaCtrl(u32 ndmaNo)
{
	while( MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) & MI_NDMA_ENABLE_MASK ){
//		OS_PutString("Dma is busy..\n");
    }
}

// ===========================================================================
// 	Function Describe
// ===========================================================================
// ÉJÅ[Éh
void HOTSW_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size);
void HOTSW_NDmaPipe_Card(u32 ndmaNo, const void *src, void *dest, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_CUSTOM_NDMA_H__