/*---------------------------------------------------------------------------*
  Project:  TwlFirm - DS - include
  File:     dsemu.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef FIRM_DS_COMMON_DS_H_
#define FIRM_DS_COMMON_DS_H_

#include <sysmenu/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif


#define IPL2_CLONE_BOOT_MODE				1
#define IPL2_OTHER_BOOT_MODE				2


void DS_CheckROMCloneBoot( void );
void DS_InsertWLPatch( void );
void DSi_SetEntry( void* entry );
void DSi_SetComponentSize( size_t size );


#ifdef __cplusplus
} /* extern "C" */

#endif

/* FIRM_DS_COMMON_DS_H_ */
#endif
