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

void OSi_BootCore( OSEntryPoint p, MIHeader_WramRegs* w );


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
    void (*OSBootCore)( OSEntryPoint p, MIHeader_WramRegs* w );

    (void)OS_DisableInterrupts();
    OSi_Finalize();

#ifdef SDK_ARM9
    OSBootCore = (void*)HW_ITCM;
#else // SDK_ARM7
    OSBootCore = (void*)HW_EXT_WRAM;
#endif // SDK_ARM7

    MI_CpuCopyFast( OSi_BootCore, OSBootCore, 0x200 );
    OSBootCore(p, w);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_Finalize

  Description:  finalize

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_Finalize(void)
{
    (void)OS_DisableInterrupts();
    (void)OS_DisableIrq();
    reg_OS_IE = 0;
    reg_OS_IF = 0xffffffff;
#ifdef SDK_ARM7
    reg_OS_IE2 = 0;
    reg_OS_IF2 = 0xffff;
    // set init check flag by bootrom
    SVC_CpuClear( REG_OS_PAUSE_CHK_MASK, (void*)REG_PAUSE_ADDR, sizeof(u16), 16 );
#else // SDK_ARM9
    // set init check flag
    reg_OS_PAUSE = REG_OS_PAUSE_CHK_MASK;
    *(u64*)HW_INIT_LOCK_BUF = 0;

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

asm void OSi_BootCore( OSEntryPoint p, MIHeader_WramRegs* w )
{
        mov     r11, r0
        mov     r10, r1

#ifdef SDK_ARM9
        // wait for request of wram map
        ldr     r3, =REG_SUBPINTF_ADDR
        mov     r2, #REG_PXI_SUBPINTF_A7STATUS_MASK
@0:
        ldr     r0, [r3]
        and     r0, r0, r2
        cmp     r0, r2
        bne     @0

        // r10- => r9-r2
        ldr     r9, =REG_MBK1_ADDR
        add     r2, r9, #32
@1:
        ldr     r3, [r10], #4
        str     r3, [r9], #4
        cmp     r9, r2
        blt     @1

        // notify wram map
        ldr     r3, =REG_SUBPINTF_ADDR
        mov     r0, #REG_PXI_SUBPINTF_A9STATUS_MASK
        str     r0, [r3]
        // wait for finalizing pxi
        ldr     r3, =REG_SUBPINTF_ADDR
@2:
        ldr     r0, [r3]
        and     r0, r0, #REG_PXI_SUBPINTF_A7STATUS_MASK
        cmp     r0, #0
        bne     @2
        // finalize pxi
        mov     r0, #0
        str     r0, [r3]
#else
        // request wram map
        ldr     r3, =REG_MAINPINTF_ADDR
        mov     r0, #REG_PXI_MAINPINTF_A7STATUS_MASK
        str     r0, [r3]
        // wait for wram map
        mov     r2, #REG_PXI_MAINPINTF_A9STATUS_MASK
@0:
        ldr     r0, [r3]
        and     r0, r0, r2
        cmp     r0, r2
        bne     @0
        // finalize pxi
        mov     r0, #0
        str     r0, [r3]

        // r10- => r9-r2
        add     r10, r10, #32
        ldr     r9, =REG_MBK6_ADDR
        add     r2, r9, #13
@1:
        ldr     r3, [r10], #4
        str     r3, [r9], #4
        cmp     r9, r2
        blt     @1
#endif

        // clear stack with r4-r9
        mov     r0, #0
#if 0
        ldr     r1, =HW_FIRM_STACK
        ldr     r2, =HW_FIRM_STACK_SIZE
        bl      MIi_CpuClearFast
#endif

        mov     lr, r11

        // clear registers
#if 0
        ldr     sp, =HW_FIRM_STACK
        ldmia   sp, {r0-r12,sp}
#endif

        bx      lr
}
