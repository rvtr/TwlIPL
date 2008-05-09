/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - init
  File:     crt0_jtag.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/
#include        <firm.h>

#include        <twl/code32.h>

void    _start(void);

void* const _start_ModuleParams[];
void* const _start_LtdModuleParams[];

/*---------------------------------------------------------------------------*
  Name:         _start
  Description:  起動ベクタ。
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/
void _start( void );

asm void _start( void )
{
#ifdef SDK_ARM7
        ldr     r3, =REG_JTAG_ADDR
        ldr     r0, =REG_SCFG_JTAG_DSPJE_MASK | REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK
        strh    r0, [r3]
#endif

@10:
        b       @10

        // link for compstatic.TWL
        ldr     r0, =_start_ModuleParams
        ldr     r0, =_start_LtdModuleParams
}

#include    <twl/codereset.h>
