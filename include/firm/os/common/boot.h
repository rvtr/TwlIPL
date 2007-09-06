/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS - include
  File:     boot.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef FIRM_OS_BOOT_H_
#define FIRM_OS_BOOT_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <firm/memorymap.h>
#include <firm/devices/firm_sdmc/ARM7/sdmc.h>  //from_brom.h—p
#include <firm/format/from_brom.h>

#include <nitro/hw/common/armArch.h>

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         OSi_Boot

  Description:  boot firm

  Arguments:    entry :  entry point

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_Boot( void* entry, MIHeader_WramRegs* w, BOOL to_firm );

/*---------------------------------------------------------------------------*
  Name:         OSi_Finalize

  Description:  finalize

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_Finalize(void);

/*---------------------------------------------------------------------------*
  Name:         OSi_GetFromBromAddr

  Description:  data address from bootrom to norfirm

  Arguments:    None

  Returns:      address
 *---------------------------------------------------------------------------*/
static inline OSFromBromBuf* OSi_GetFromBromAddr( void )
{
    return  (OSFromBromBuf*)HW_NORFIRM_FROM_BROM_BUF;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_OS_BOOT_H_ */
#endif
