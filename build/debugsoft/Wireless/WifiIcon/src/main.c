/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - WiFiIcon
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include <twl.h>

static void InitInterrupt(void)
{
    OS_EnableIrq();
    OS_EnableInterrupts();
}

static void InitAlloc(void)
{
    OSHeapHandle hHeap;
    void* lo = OS_GetMainArenaLo();
    void* hi = OS_GetMainArenaHi();

    lo = OS_InitAlloc(OS_ARENA_MAIN, lo, hi, 1);
    OS_SetArenaLo(OS_ARENA_MAIN, lo);

    hHeap = OS_CreateHeap(OS_ARENA_MAIN, lo, hi);
    SDK_ASSERT( hHeap >= 0 );

    OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}
    
static void InitInteruptSystem();


void
TwlStartUp()
{
    OS_Init();
    InitAlloc();
}


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  ���C���֐��ł��B 

  Arguments:    �Ȃ��B 

  Returns:      �Ȃ��B 
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    InitInteruptSystem();

    GX_DispOn();
    GXS_DispOn();
    
    *(u16*)HW_PLTT  =   0x001f << 10;
    *(u16*)HW_DB_PLTT  =   0x001f << 10;
    // �����`���[�ɖ߂��悤�ɁA �I�����Ȃ� 
    for (;;)
    {
        // �t���[���X�V�B 
        {
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem 

  Description:  ���荞�݂����������܂��B 

  Arguments:    �Ȃ��B 

  Returns:      �Ȃ��B 
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // �ʊ��荞�݃t���O��S�ĕs���� 
    (void)OS_SetIrqMask(0);

    // �}�X�^�[���荞�݃t���O������ 
    (void)OS_EnableIrq();

    // IRQ ���荞�݂������܂� 
    (void)OS_EnableInterrupts();
	
	(void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
}
