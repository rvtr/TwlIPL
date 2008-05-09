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
#include <twl/fatfs/common/api.h>

void FATFS_InitFIRM( void )
{
    MI_CpuClearFast( (void*)HW_FIRM_FATFS_COMMAND_BUFFER, HW_FIRM_FATFS_COMMAND_BUFFER_SIZE );
    MI_CpuClearFast( (void*)HW_FIRM_FATFS_ARCHNAME_LIST, HW_FIRM_FATFS_ARCHNAME_LIST_SIZE );
    FATFSiCommandBuffer = (void*)HW_FIRM_FATFS_COMMAND_BUFFER;
    FATFSiArcnameList = (void*)HW_FIRM_FATFS_ARCHNAME_LIST;
    FATFS_Init();
}

void FATFSi_GetUnicodeConversionTable(const u8 **u2s, const u16 **s2u)
{
    *u2s = NULL;
    *s2u = NULL;
}
