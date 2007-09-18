/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - nandrfirm-loader
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
#include <firm/mi.h>

u8 acHeap[4*1024] __attribute__ ((aligned (32)));
int acPool[3];
int errID;

void TwlMain( void )
{
    OS_TPrintf( "\nNAND Boot time is %d msec.\n", OS_TicksToMilliSecondsBROM32(OS_GetTick()));

    OS_InitFIRM();

    SVC_InitSignHeap( acPool, acHeap, sizeof(acHeap) );

    // load menu
    if ( MI_LoadHeader( acPool ) && MI_LoadMenu() )
    {
        MI_BootMenu();
    }

    OS_Terminate();

}

