/*---------------------------------------------------------------------------*
  Project:  TwlIPL - wram_regs
  File:     wram_regs.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-12-11#$
  $Rev: 2990 $
  $Author: nakasima $
 *---------------------------------------------------------------------------*/
#include <twl/mi.h>

#define HW_HYENA_WRAM_A_OFS      (HW_WRAM_A_LTD - HW_WRAM_BASE)
#define HW_HYENA_WRAM_A_OFS_END  (HW_WRAM_A_LTD_END - HW_WRAM_BASE)
#define HW_HYENA_WRAM_B_OFS      (HW_HYENA_WRAM_A_OFS - HW_WRAM_B_SIZE)
#define HW_HYENA_WRAM_B_OFS_END  (HW_HYENA_WRAM_A_OFS)
#define HW_HYENA_WRAM_C_OFS      (HW_HYENA_WRAM_B_OFS - HW_WRAM_C_SIZE)
#define HW_HYENA_WRAM_C_OFS_END  (HW_HYENA_WRAM_B_OFS)

// MAP_TS_LTD for hyena
// WRAM-A Lock:ON,  Master:ARM7, Enable:Slot0-3(256Kbytes), Address(7):0x037c0000-0x037fffff, Address(9):None
// WRAM-B Lock:ON,  Master:ARM7, Enable:Slot7  (32Kbytes),  Address(7):0x037b8000-0x037bffff, Address(9):None
// WRAM-B Lock:OFF, Master:ARM9, Enable:Slot0-6(224Kbytes), Address(7):0x03780000-0x037b7fff, Address(9):0x03780000-0x037bffff
// WRAM-C Lock:OFF, Msster:ARM9, Enable:Slot0-7(256Kbytes), Address(7):0x03740000-0x0377ffff, Address(9):0x03740000-0x0377ffff
// WRAM-0           Master:ARM9,               (16Kbytes),  Address(7):0x03040000-0x03043fff, Address(9):0x03040000-0x03043fff
// WRAM-1           Master:ARM9,               (16Kbytes),  Address(7):0x03044000-0x03047fff, Address(9):0x03044000-0x03047fff

// MAP_TS_LTD original
// WRAM-A Lock:ON,  Master:ARM7, Enable:Slot0-3(256Kbytes), Address(7):0x037c0000-0x037fffff, Address(9):None
// WRAM-B Lock:OFF, Master:ARM9, Enable:Slot0-7(256Kbytes), Address(7):0x03900000-0x0393ffff, Address(9):0x03900000-0x0393ffff
// WRAM-C Lock:OFF, Msster:ARM9, Enable:Slot0-7(256Kbytes), Address(7):0x03940000-0x0397ffff, Address(9):0x03940000-0x0397ffff
// WRAM-0           Master:ARM9,               (16Kbytes),  Address(7):0x03040000-0x03043fff, Address(9):0x03040000-0x03043fff
// WRAM-1           Master:ARM9,               (16Kbytes),  Address(7):0x03044000-0x03047fff, Address(9):0x03044000-0x03047fff

u32 HYENA_WramReg[0x30/sizeof(u32)] =
{
    // ARM9

    // WRAM-A
    REG_MI_MBK1_FIELD(
        TRUE, MI_WRAM_OFFSET_192KB/2, MI_WRAM_ARM7,
        TRUE, MI_WRAM_OFFSET_128KB/2, MI_WRAM_ARM7,
        TRUE, MI_WRAM_OFFSET_64KB/2,  MI_WRAM_ARM7,
        TRUE, MI_WRAM_OFFSET_0KB/2,   MI_WRAM_ARM7
    ),
    // WRAM-B
    REG_MI_MBK2_FIELD(
        TRUE, MI_WRAM_OFFSET_96KB,  MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_64KB,  MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_32KB,  MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_0KB,   MI_WRAM_ARM9
    ),
    REG_MI_MBK3_FIELD(
        TRUE, MI_WRAM_OFFSET_224KB, MI_WRAM_ARM7,
        TRUE, MI_WRAM_OFFSET_192KB, MI_WRAM_ARM7,
        TRUE, MI_WRAM_OFFSET_160KB, MI_WRAM_ARM7,
        TRUE, MI_WRAM_OFFSET_128KB, MI_WRAM_ARM9
    ),
    // WRAM-C
    REG_MI_MBK4_FIELD(
        TRUE, MI_WRAM_OFFSET_96KB,  MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_64KB,  MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_32KB,  MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_0KB,   MI_WRAM_ARM9
    ),
    REG_MI_MBK5_FIELD(
        TRUE, MI_WRAM_OFFSET_224KB, MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_192KB, MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_160KB, MI_WRAM_ARM9,
        TRUE, MI_WRAM_OFFSET_128KB, MI_WRAM_ARM9
    ),

    REG_MI_MBK6_FIELD(  NULL >> 16,
                        MI_WRAM_IMAGE_256KB,
                        NULL >> 16
                        ),
    REG_MI_MBK7_FIELD(  HW_HYENA_WRAM_B_OFS_END >> 15,
                        MI_WRAM_IMAGE_256KB,
                        HW_HYENA_WRAM_B_OFS >> 15
                        ),
    REG_MI_MBK8_FIELD(  HW_HYENA_WRAM_C_OFS_END >> 15,
                        MI_WRAM_IMAGE_256KB,
                        HW_HYENA_WRAM_C_OFS >> 15
                        ),

    // ARM7
    REG_MI_MBK6_FIELD(  HW_HYENA_WRAM_A_OFS_END >> 16,
                        MI_WRAM_IMAGE_256KB,
                        HW_HYENA_WRAM_A_OFS >> 16
                        ),
    REG_MI_MBK7_FIELD(  HW_HYENA_WRAM_B_OFS_END >> 15,
                        MI_WRAM_IMAGE_256KB,
                        HW_HYENA_WRAM_B_OFS >> 15
                        ),
    REG_MI_MBK8_FIELD(  HW_HYENA_WRAM_C_OFS_END >> 15,
                        MI_WRAM_IMAGE_256KB,
                        HW_HYENA_WRAM_C_OFS >> 15
                        ),

    // WRAM Lock
    (u32)(
    (0x0F <<  0) |
    (0x80 <<  8) |
    (0x00 << 16) |

    // WRAM-0/1
    (0 << 24) |

    // VRAM-C
    (7 << 26) |
    // VRAM-D
    (7 << 29)
    ),
};

