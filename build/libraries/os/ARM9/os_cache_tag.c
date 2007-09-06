/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS
  File:     os_cache_tag.c

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
#include <twl/os/ARM9/os_cache_tag.h>
#include <nitro/code32.h>


//===========================================================================
//              DATA CACHE (for specified range)
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_ClearTagAll

  Description:  clear tag in data cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DC_ClearTagAll( void )
{
    DC_FillTagAll( 0 );
}

/*---------------------------------------------------------------------------*
  Name:         DC_FillTagAll

  Description:  clear tag in data cache

  Arguments:    data :  fill data

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_FillTagAll( u32 data )
{
    mov         r12, #0
@1:
    mov         r2, #0
@2:
    orr         r3, r2, r12
    mcr         p15, 3, r3, c15, c0, 0  // set index
    mcr         p15, 3, r0, c15, c2, 0  // clear tag
    add         r2, r2, #HW_CACHE_LINE_SIZE
    cmp         r2, #HW_DCACHE_SIZE/4
    blt         @2

    add         r12, r12, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r12, #0
    bne         @1

    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_ClearDataAll

  Description:  clear data in data cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DC_ClearDataAll( void )
{
    DC_FillDataAll( 0 );
}

/*---------------------------------------------------------------------------*
  Name:         DC_FillDataAll

  Description:  fill data in data cache

  Arguments:    data :  fill data

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_FillDataAll( u32 data )
{
    mov         r12, #0
@1:
    mov         r2, #0
@2:
    orr         r3, r2, r12
    mcr         p15, 3, r3, c15, c0, 0  // set index
    mcr         p15, 3, r0, c15, c4, 0  // clear data
    add         r2, r2, #4
    cmp         r2, #HW_DCACHE_SIZE/4
    blt         @2

    add         r12, r12, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r12, #0
    bne         @1

    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_GetTagAndDataAll

  Description:  get tag and data in data cache

  Arguments:    tag           tag address
                data          data address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_GetTagAndDataAll( void* tag, void* data )
{
    mov         r12, #0
@1:
    mov         r2, #0
@2:
    orr         r3, r2, r12
    mcr         p15, 3, r3, c15, c0, 0  // set index
    tst         r2, #HW_CACHE_LINE_SIZE - 1
    mrceq       p15, 3, r3, c15, c2, 0  // get tag
    streq       r3, [r0], #4
    mrc         p15, 3, r3, c15, c4, 0  // get data
    str         r3, [r1], #4
    add         r2, r2, #4
    cmp         r2, #HW_DCACHE_SIZE/4
    blt         @2

    add         r12, r12, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r12, #0
    bne         @1

    bx          lr
}


//===========================================================================
//              INSTRUCTION CACHE
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         IC_ClearTagAll

  Description:  clear tag in instruction cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IC_ClearTagAll( void )
{
    IC_FillTagAll( 0 );
}

/*---------------------------------------------------------------------------*
  Name:         IC_FillTagAll

  Description:  fill tag in instruction cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_FillTagAll( u32 data )
{
    mov         r12, #0
@1:
    mov         r2, #0
@2:
    orr         r3, r2, r12
    mcr         p15, 3, r3, c15, c0, 0  // set index
    mcr         p15, 3, r0, c15, c1, 0  // clear tag
    add         r2, r2, #HW_CACHE_LINE_SIZE
    cmp         r2, #HW_ICACHE_SIZE/4
    blt         @2

    add         r12, r12, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r12, #0
    bne         @1

    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         IC_ClearInstructionAll

  Description:  clear instruction in instruction cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IC_ClearInstructionAll( void )
{
    IC_FillInstructionAll( 0 );
}

/*---------------------------------------------------------------------------*
  Name:         IC_FillInstructionAll

  Description:  fill instruction in instruction cache

  Arguments:    data :  fill data

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_FillInstructionAll( u32 data )
{
    mov         r12, #0
@1:
    mov         r2, #0
@2:
    orr         r3, r2, r12
    mcr         p15, 3, r3, c15, c0, 0  // set index
    mcr         p15, 3, r0, c15, c3, 0  // clear data
    add         r2, r2, #4
    cmp         r2, #HW_ICACHE_SIZE/4
    blt         @2

    add         r12, r12, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r12, #0
    bne         @1

    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         IC_GetTagAndInstructionAll

  Description:  get tag and instruction in instruction cache

  Arguments:    tag           tag address
                inst          instruction address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_GetTagAndInstructionAll( void* tag, void* inst )
{
    mov         r12, #0
@1:
    mov         r2, #0
@2:
    orr         r3, r2, r12
    mcr         p15, 3, r3, c15, c0, 0  // set index
    tst         r2, #HW_CACHE_LINE_SIZE - 1
    mrceq       p15, 3, r3, c15, c1, 0  // get tag
    streq       r3, [r0], #4
    mrc         p15, 3, r3, c15, c3, 0  // get data
    str         r3, [r1], #4
    add         r2, r2, #4
    cmp         r2, #HW_ICACHE_SIZE/4
    blt         @2

    add         r12, r12, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r12, #0
    bne         @1

    bx          lr
}


#include <nitro/codereset.h>
