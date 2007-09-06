/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenandfirm
  File:     wram_regs.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl/mi.h>
#include <firm/format/wram_regs.h>

MIHeader_WramRegs wram_regs =
{
    // ARM9
    {
        REG_WRAM_A_BNK_PACK(0, MI_WRAM_A_ARM7, MI_WRAM_A_OFS_0KB,   TRUE),
        REG_WRAM_A_BNK_PACK(1, MI_WRAM_A_ARM7, MI_WRAM_A_OFS_64KB,  TRUE),
        REG_WRAM_A_BNK_PACK(2, MI_WRAM_A_ARM7, MI_WRAM_A_OFS_128KB, TRUE),
        REG_WRAM_A_BNK_PACK(3, MI_WRAM_A_ARM7, MI_WRAM_A_OFS_192KB, TRUE),
    },
    {
        REG_WRAM_B_BNK_PACK(0, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_0KB,   TRUE),
        REG_WRAM_B_BNK_PACK(1, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_32KB,  TRUE),
        REG_WRAM_B_BNK_PACK(2, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_64KB,  TRUE),
        REG_WRAM_B_BNK_PACK(3, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_96KB,  TRUE),
        REG_WRAM_B_BNK_PACK(4, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_128KB, TRUE),
        REG_WRAM_B_BNK_PACK(5, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_160KB, TRUE),
        REG_WRAM_B_BNK_PACK(6, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_192KB, TRUE),
        REG_WRAM_B_BNK_PACK(7, MI_WRAM_B_ARM7, MI_WRAM_B_OFS_224KB, TRUE),
    },
    {
        REG_WRAM_C_BNK_PACK(0, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_0KB,   TRUE),
        REG_WRAM_C_BNK_PACK(1, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_32KB,  TRUE),
        REG_WRAM_C_BNK_PACK(2, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_64KB,  TRUE),
        REG_WRAM_C_BNK_PACK(3, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_96KB,  TRUE),
        REG_WRAM_C_BNK_PACK(4, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_128KB, TRUE),
        REG_WRAM_C_BNK_PACK(5, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_160KB, TRUE),
        REG_WRAM_C_BNK_PACK(6, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_192KB, TRUE),
        REG_WRAM_C_BNK_PACK(7, MI_WRAM_C_ARM9, MI_WRAM_C_OFS_224KB, TRUE),
    },
    REG_WRAM_A_MAP_PACK(MI_WRAM_MAP_NULL,
                        MI_WRAM_MAP_NULL,
                        MI_WRAM_A_IMG_128KB
                        ),
    REG_WRAM_B_MAP_PACK(MI_WRAM_MAP_NULL,
                        MI_WRAM_MAP_NULL,
                        MI_WRAM_B_IMG_128KB
                        ),
    REG_WRAM_C_MAP_PACK(HW_WRAM_AREA_HALF,
                        HW_WRAM_AREA_HALF + 0x00020000,
                        MI_WRAM_C_IMG_128KB
                        ),

    // ARM7
    REG_WRAM_A_MAP_PACK(HW_WRAM_AREA_HALF,
                        HW_WRAM_AREA_HALF + 0x00020000,
                        MI_WRAM_A_IMG_128KB
                        ),
    REG_WRAM_B_MAP_PACK(HW_WRAM_AREA_HALF + 0x00020000,
                        HW_WRAM_AREA_HALF + 0x00040000,
                        MI_WRAM_B_IMG_128KB
                        ),
    REG_WRAM_C_MAP_PACK(MI_WRAM_MAP_NULL,
                        MI_WRAM_MAP_NULL,
                        MI_WRAM_C_IMG_128KB
                        ),
    // WRAM Lock
    {
        0,
        0,
        0,
    },

    // WRAM-0/1
    MI_WRAM_ARM7_ALL,

    // VRAM-C
    7,
    // VRAM-D
    7,
};
