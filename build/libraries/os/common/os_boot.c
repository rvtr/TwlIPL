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
#include <firm/os/ARM9/os_cache_tag.h>
#else
#include <twl/aes/ARM7/lo.h>
#endif

#if 0
    課題:
        OSi_BootCoreのコピー先決定

        いろいろクリア (ITCM,DTCM,STACK,TEXT,RODATA,DATAなど (OSi_BootCoreは残す))
            なので、OSi_BootCoreは次のプログラムですぐに壊されそうなところを使う
#endif

void OSi_BootCore( ROM_Header* rom_header );

/*---------------------------------------------------------------------------*
  Name:         OSi_Boot

  Description:  boot firm

  Arguments:    rom_header  :  ROM header

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_Boot( ROM_Header* rom_header )
{
    void (*OSBootCore)( ROM_Header* rom_header );

    (void)OS_DisableInterrupts();
    OSi_Finalize();
    OSBootCore = (void*)HW_FIRM_BOOT_CORE;
    MI_CpuCopyFast( OSi_BootCore, OSBootCore, HW_FIRM_BOOT_CORE_SIZE );

    OSBootCore(rom_header);
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

extern void SDK_STATIC_START(void); // static start address
extern void SDK_STATIC_END(void);   // static end address

#include <twl/code32.h>

asm void OSi_BootCore( ROM_Header* rom_header )
{

#ifdef SDK_ARM9
        add     r10, r0, #0x180     // rom_header->s.main_wram_config_data
        ldr     r11, [r0, #0x24]    // rom_header->s.main_entry_address

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

#else   // ARM7
        add     r10, r0, #0x1a0     // rom_header->s.sub_wram_config_data
        ldr     r11, [r0, #0x34]    // rom_header->s.sub_entry_address

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
        ldr     r9, =REG_MBK6_ADDR
        add     r2, r9, #15
@1:
        ldr     r3, [r10], #4
        str     r3, [r9], #4
        cmp     r9, r2
        blt     @1

#endif

        // clear something all
        mov     r0, #0
#if 0
        // clear stack
        ldr     r1, =HW_FIRM_STACK
        ldr     r2, =HW_FIRM_STACK_SIZE
@10:    cmp     r1, r2
        strcc   r0, [r1], #4
        bcc     @10

        // clear static text, data, and bss
        ldr     r1, =SDK_STATIC_START
        ldr     r2, =SDK_STATIC_END
@20:    cmp     r1, r2
        strcc   r0, [r1], #4
        bcc     @20
#endif

        mov     lr, r11

        // clear registers
#if 0
        ldr     sp, =HW_FIRM_STACK
        ldmia   sp, {r0-r12,sp}
#endif

        bx      lr
}
