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

#include <firm.h>
#include <firm/format/from_brom.h>
#include <firm/format/from_firm.h>
#include <twl/os/common/format_rom.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_BootWithRomHeaderFromFIRM

  Description:  boot with ROM header

  Arguments:    rom_header  :  ROM header

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_BootWithRomHeaderFromFIRM( ROM_Header* rom_header );

/*---------------------------------------------------------------------------*
  Name:         OS_BootDefault

  Description:  boot system menu using ROM_Header in HW_TWL_ROM_HEADER_BUF

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OS_BootFromFIRM( void )
{
    OS_BootWithRomHeaderFromFIRM( (ROM_Header*)HW_TWL_ROM_HEADER_BUF );
}

/*---------------------------------------------------------------------------*
  Name:         OSi_FromBromToMenu

  Description:  convert OSFromBromBuf to OSFromFirmBuf

  Arguments:    None

  Returns:      FALSE if FromBrom is broken
 *---------------------------------------------------------------------------*/
BOOL OSi_FromBromToMenu( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_GetFromBromAddr

  Description:  data address from bootrom to firm

  Arguments:    None

  Returns:      address
 *---------------------------------------------------------------------------*/
static inline OSFromBromBuf* OSi_GetFromBromAddr( void )
{
    return  (OSFromBromBuf*)HW_FIRM_FROM_BROM_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetFromFirmAddr

  Description:  data address from firm to menu

  Arguments:    None

  Returns:      address
 *---------------------------------------------------------------------------*/
static inline OSFromFirmBuf* OSi_GetFromFirmAddr( void )
{
    return  (OSFromFirmBuf*)HW_FIRM_FROM_FIRM_BUF;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_OS_BOOT_H_ */
#endif
