/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS
  File:     os_init_firm.c

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

/*---------------------------------------------------------------------------*
  Name:         OS_InitNOR

  Description:  initialize sdk os for firm

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void OS_InitFIRM(void)
{
#ifdef SDK_ARM9
    //---- system shared area check
    SDK_ASSERT((u32)&(OS_GetSystemWork()->command_area) == HW_CMD_AREA);

    //----------------------------------------------------------------
    // for ARM9

#ifdef SDK_ENABLE_ARM7_PRINT
    // Init PrintServer for ARM7 (if specified)
    OS_InitPrintServer();
#endif

    //---- Init Arena (arenas except SUBPRIV-WRAM)
    OS_InitArena();

    //---- Init interProcessor I/F
    //  Sync with ARM7 to enable OS_GetConsoleType()
    //  PXI_Init() must be called before OS_InitArenaEx()
    PXI_Init();

    //---- Init Spinlock
    OS_InitLock();

    //---- Init Arena (extended main)
    OS_InitArenaEx();

    //---- Init IRQ Table
    OS_InitIrqTable();

    //---- Init IRQ Stack checker
    OS_SetIrqStackChecker();

    //---- Init Exception System
    OS_InitException();

    //---- Init MI (Wram bank and DMA0 arranged)
    MI_Init();

    //---- Init VCountAlarm
    OS_InitVAlarm();

    //---- Init VRAM exclusive System
    OSi_InitVramExclusive();

    //---- Init Thread System
#ifndef SDK_NO_THREAD
    OS_InitThread();
#endif
    //---- Init Reset System
#ifndef SDK_SMALL_BUILD
    OS_InitReset();
#endif

    //---- Init Cartridge
#ifndef SDK_TEG
//    CTRDG_Init();
#endif

    //---- Init Card
#ifndef SDK_SMALL_BUILD
//    CARD_Init();
#endif

    //---- Init Power Manager
#ifndef SDK_TEG
//    PM_Init();
#endif

    //---- adjust VCOUNT
//    OSi_WaitVCount0();

#else  // SDK_ARM9
    //----------------------------------------------------------------
    // for ARM7

    //---- Init Arena (SUBPRIV-WRAM arena)
    OS_InitArena();

    //---- Init interProcessor I/F
    PXI_Init();

    //---- Init Spinlock
    OS_InitLock();

    //---- Init IRQ Table
    OS_InitIrqTable();

#define SDK_EXCEPTION_BUG
#ifndef SDK_EXCEPTION_BUG
    //---- Init Exception System
    OS_InitException();
#endif

    //---- Init Tick
    OS_InitTick();

    //---- Init Alarm System
    OS_InitAlarm();

    //---- Init Thread System
    OS_InitThread();

    //---- Init Reset System
#ifndef SDK_SMALL_BUILD
    OS_InitReset();
#endif

    //---- Init Cartridge
#ifndef SDK_TEG
//    CTRDG_Init();
#endif

#endif // SDK_ARM9
}

#pragma profile reset
