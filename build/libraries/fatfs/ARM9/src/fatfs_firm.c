/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fatfs
  File:     fatfs_firm.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <firm.h>
#include <twl/types.h>

void FATFS_InitFIRM( void )
{
    FATFSiCommandBuffer = (void*)FIRM_FATFS_COMMAND_BUFFER;
    FATFS_Init();
}

void FATFSi_GetUnicodeConversionTable(const u8 **u2s, const u16 **s2u)
{
    *u2s = NULL;
    *s2u = NULL;
}
