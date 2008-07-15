/*---------------------------------------------------------------------------*
  Project:  TwlSDK - components - armadillo.TWL
  File:     overwriteRtc.c

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

#include    <twl/types.h>
#include    <nitro/rtc/common/fifo.h>
#include 	<twl/memorymap_sp.h>
#include 	<twl/hw/common/mmap_wramEnv.h>
#include 	<twl/code32.h>

extern BOOL RTC_IsAvailablePxiCommand(u16 command);

/*---------------------------------------------------------------------------*
  Name:         RTC_IsAvailablePxiCommand

  Description:  PXI コマンドが有効な RTC 操作コマンドであるかどうか調査する。

  Arguments:    command -   調査するコマンド。

  Returns:      BOOL    -   有効なコマンドである場合に TRUE を返す。処理できな
                            い無効なコマンドである場合には FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL
RTC_IsAvailablePxiCommand(u16 command)
{
    switch (command)
    {
    case RTC_PXI_COMMAND_RESET:
    case RTC_PXI_COMMAND_SET_HOUR_FORMAT:
    case RTC_PXI_COMMAND_READ_DATETIME:
    case RTC_PXI_COMMAND_READ_DATE:
    case RTC_PXI_COMMAND_READ_TIME:
    case RTC_PXI_COMMAND_READ_PULSE:
    case RTC_PXI_COMMAND_READ_ALARM1:
    case RTC_PXI_COMMAND_READ_ALARM2:
    case RTC_PXI_COMMAND_READ_STATUS1:
    case RTC_PXI_COMMAND_READ_STATUS2:
    case RTC_PXI_COMMAND_READ_ADJUST:
    case RTC_PXI_COMMAND_READ_FREE:
    case RTC_PXI_COMMAND_WRITE_DATETIME:
    case RTC_PXI_COMMAND_WRITE_DATE:
    case RTC_PXI_COMMAND_WRITE_TIME:
    case RTC_PXI_COMMAND_WRITE_PULSE:
    case RTC_PXI_COMMAND_WRITE_ALARM1:
    case RTC_PXI_COMMAND_WRITE_ALARM2:
    case RTC_PXI_COMMAND_WRITE_STATUS1:
    case RTC_PXI_COMMAND_WRITE_STATUS2:
    case RTC_PXI_COMMAND_WRITE_ADJUST:
    case RTC_PXI_COMMAND_WRITE_FREE:
        return TRUE;
    }
    return FALSE;
}

