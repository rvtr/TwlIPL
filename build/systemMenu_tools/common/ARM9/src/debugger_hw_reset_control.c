/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     debugger_hw_reset_control.c

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
#include "debugger_hw_reset_control.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/

#define OS_THREAD_PRIORITY_IS_TWL_DEBUGGER_HW_RESET_CONTROL  15

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

vu8       sHwResetEnable = TRUE;
OSThread  sThread;
u32       sStack[1024];
static s32 sLockId;

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/
static void CardAccessThread(void* arg);

/*---------------------------------------------------------------------------*
  Name:         CardAccessThread

  Description:  5行毎にダミーのカードアクセスを行います。

  Arguments:    arg -   使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CardAccessThread(void* arg)
{
#pragma unused(arg)

    while (!sHwResetEnable)
    {
		CARD_LockRom((u16)sLockId);
		(void)CARDi_ReadRomID();
		CARD_UnlockRom((u16)sLockId);

		// 5秒間スリープ
		OS_Sleep(5000);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetDisable

  Description:  IS-TWL-DEBUGGERでのハードウェアリセットを禁止します。
                この機能はデバッガディゼーブルフラグを指定したSRL 
				でのみ有効です。内部動作としては、5秒毎にカードアクセスを
				行うスレッドを生成起動しています。IS-TWL-DEBUGGERは
				カードアクセスを監視していて10秒間カードアクセスがない
				場合にハードウェアリセットを許可する仕組みになっています。

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetDisable( void )
{
	if (sLockId == 0)
	{
		sLockId = OS_GetLockID();
	}

	if (sHwResetEnable)
	{
		sHwResetEnable = FALSE;

	    OS_CreateThread(&sThread, CardAccessThread, NULL,
	        			(void*)((u32)sStack + sizeof(sStack)), sizeof(sStack), 
						OS_THREAD_PRIORITY_IS_TWL_DEBUGGER_HW_RESET_CONTROL);
	    OS_WakeupThreadDirect(&sThread);
	}
}

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetEnable

  Description:  IS-TWL-DEBUGGERでのハードウェアリセットを許可します。
                この機能はデバッガディゼーブルフラグを指定したSRL 
				でのみ有効です。実際にハードウェアリセットが可能になるには
				最大で10秒かかります。

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetEnable( void )
{
	if (!sHwResetEnable)
	{
    	sHwResetEnable = TRUE;
		OS_WakeupThreadDirect(&sThread);
		while (!OS_IsThreadTerminated(&sThread)){}
	}
}
