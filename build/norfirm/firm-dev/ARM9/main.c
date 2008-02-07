/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - norfirm-print
  File:     main.c

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
#include "reboot.h"

//#define PRINT_DEBUG

#ifndef PRINT_DEBUG
#undef  OS_TPrintf
#define OS_TPrintf(...) ((void)0)
#endif // PRINT_DEBUG

void TwlMain( void )
{
    OS_TPrintf( "\nNOR Boot time is %d msec.\n", OS_TicksToMilliSecondsBROM32(OS_GetTick()));

    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );

    OS_InitFIRM();

    OS_TPrintf( "\nARM9 starts.\n" );
    OS_TPrintf( "\nARM9 ends.\n" );

    REBOOT_DisableInterruptsAndProtectionUnit();
    OS_Terminate();
}

