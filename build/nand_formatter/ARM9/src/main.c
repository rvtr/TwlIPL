/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::             $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/


#include <twl.h>
#include <twl/fatfs.h>
#include <nitro/card.h>


//================================================================================


/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    //---- check interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_InitArena();
    PXI_Init();
    OS_InitLock();
    OS_InitArenaEx();
    OS_InitIrqTable();
    OS_SetIrqStackChecker();
    MI_Init();
    OS_InitVAlarm();
    OSi_InitVramExclusive();
    OS_InitThread();
    OS_InitReset();
    OS_Init();
    GX_Init();

    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    FATFS_Init();

    // ARM7からのフォーマット完了通知待ち (てっとりばやく強引な方法で通知している)
    OS_TPrintf("now formatting NAND partitions...\n");
    {
        const CARDRomHeader    *header = CARD_GetOwnRomHeader();
        while (*(u32 *)header->main_ram_address != 0x00000000)
        {
            OS_WaitVBlankIntr();
        }
    }

    OS_TPrintf("format has completed!\n");
    OS_Terminate();
}


/*====== End of main.c ======*/
