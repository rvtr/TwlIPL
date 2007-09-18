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
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include <firm/os.h>
#include <firm/mi.h>
#include <firm/pxi.h>
#ifdef SDK_ARM9
#include <twl/os/ARM9/os_cache_tag.h>
#else
#include <twl/aes/ARM7/lo.h>
#endif

void OSi_BootCore( OSEntryPoint p );

/*---------------------------------------------------------------------------*
  Name:         OSi_Boot

  Description:  boot firm

  Arguments:    entry :    entry point
                w     :    wram settings

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_Boot( void* entry, MIHeader_WramRegs* w )
{
    OSEntryPoint p = (OSEntryPoint)entry;

    (void)OS_DisableInterrupts();
    OSi_Finalize();

#ifdef SDK_ARM9

    MI_CpuCopy8( w, (void*)REG_MBK1_ADDR, 32 ); // set MBK1 - 8
    //reg_RBKCNT1_H_ADDR = w->sub_wramlock[4];  // set RBKCNT01

    // request hiding secure rom
    PXI_NotifyID( FIRM_PXI_ID_DONE_WRAM_SETTING );

    OSi_ClearWorkArea();

#else // SDK_ARM7

    // wait request of hiding secure rom
    PXI_WaitID( FIRM_PXI_ID_DONE_WRAM_SETTING );

    MI_CpuCopy8( &w->sub_wrammap_a, (void*)REG_MBK6_ADDR, 13 ); // set MBK6 - MBK_C_LOCK

    AESi_SetKeySeedA( (AESKeySeed*)OSi_GetFromBromAddr()->aes_key[2] );    // erase

    OSi_ClearWorkArea();

#endif // SDK_ARM7

    OSi_BootCore( p );
}

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
    reg_OS_IF2 = 0xffff;
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

extern void SDK_STATIC_DATA_START(void); // static data start address
extern void SDK_STATIC_BSS_END(void);  // static bss end address

/*---------------------------------------------------------------------------*
  Name:         OSi_ClearWorkArea

  Description:  clear work area

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
asm void OSi_ClearWorkArea( void )
{
        mov     r11, lr

        // clear stack with r4-r9
        mov     r0, #0
        ldr     r1, =SDK_STATIC_DATA_START
        ldr     r2, =SDK_STATIC_BSS_END
        sub     r2, r2, r1
        bl      MIi_CpuClearFast

        bx      r11
}

asm void OSi_BootCore( OSEntryPoint p )
{
        mov     r11, r0

        // clear stack with r4-r9
        mov     r0, #0
#if 0
        ldr     r1, =HW_FIRM_STACK
        ldr     r2, =HW_FIRM_STACK_SIZE
#endif
        bl      MIi_CpuClearFast

        mov     lr, r11

        // clear registers
#if 0
        ldr     sp, =HW_FIRM_STACK
#endif
        ldmia   sp, {r0-r12,sp}

        bx      lr
}
