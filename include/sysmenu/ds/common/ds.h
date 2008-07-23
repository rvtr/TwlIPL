/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ds.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/

#ifndef SYSM_DS_COMMON_DS_H_
#define SYSM_DS_COMMON_DS_H_

#include <sysmenu/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DS_REDRSV_PATCH_FUNC_ADDR			( HW_RED_RESERVED + 0x1c )
#define DS_WLPATCH_SIZE						(5*4)
#define DS_WLPATCH_COPYCODE_SIZE			(10*4)

void DS_InsertWLPatch( void* romHeaderNTR );
void DS_SetSpeakerVolume( void* romHeaderNTR );

#ifdef __cplusplus
} /* extern "C" */

#endif

/* SYSM_DS_COMMON_DS_H_ */
#endif
