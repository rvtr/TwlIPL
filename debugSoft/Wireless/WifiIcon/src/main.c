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

  Description:  メイン関数です。 

  Arguments:    なし。 

  Returns:      なし。 
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    InitInteruptSystem();

    GX_DispOn();
    GXS_DispOn();
    
    *(u16*)HW_PLTT  =   0x001f << 10;
    *(u16*)HW_DB_PLTT  =   0x001f << 10;
    // ランチャーに戻れるように、 終了しない 
    for (;;)
    {
        // フレーム更新。 
        {
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem 

  Description:  割り込みを初期化します。 

  Arguments:    なし。 

  Returns:      なし。 
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // 個別割り込みフラグを全て不許可に 
    (void)OS_SetIrqMask(0);

    // マスター割り込みフラグを許可に 
    (void)OS_EnableIrq();

    // IRQ 割り込みを許可します 
    (void)OS_EnableInterrupts();
	
	(void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
}
