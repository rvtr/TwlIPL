/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS
  File:     os_boot.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/
#include <firm/os.h>
#include <firm/mi.h>
#ifdef SDK_ARM9
#include <twl/os/ARM9/os_cache_tag.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         OSi_Finalize

  Description:  finalize

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_Finalize(void)
{
    (void)OS_DisableIrq();
    reg_OS_IE = 0;
    reg_OS_IF = 0xffffffff;
#ifdef SDK_ARM7
    reg_OS_IE2 = 0;
    reg_OS_IF2 = 0xffffffff;
#else // SDK_ARM9
    (void)OS_DisableInterrupts();
    DC_Disable();
    DC_FlushAll();
    DC_WaitWriteBufferEmpty();
    IC_Disable();
    IC_InvalidateAll();

    // clear cache
    IC_ClearTagAll();
    IC_ClearInstructionAll();
    DC_ClearTagAll();
    DC_ClearDataAll();

    OS_DisableProtectionUnit();
#endif // SDK_ARM9
}

