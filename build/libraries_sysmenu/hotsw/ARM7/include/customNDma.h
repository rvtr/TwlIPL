#ifndef __CUSTOM_NDMA_H__
#define __CUSTOM_NDMA_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================
// ÉJÅ[Éh
void HOTSW_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __CUSTOM_NDMA_H__