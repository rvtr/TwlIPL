/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - gcdfirm
  File:     wram_regs.c

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
#include <twl/mi.h>
#include <firm/format/wram_regs.h>

MIHeader_WramRegs wram_regs_init =
{
    // ARM9
    {
        REG_MI_MBK_A0_FIELD( 1, MI_WRAM_A_OFFSET_0KB  , MI_WRAM_ARM9 ),
        REG_MI_MBK_A1_FIELD( 1, MI_WRAM_A_OFFSET_64KB , MI_WRAM_ARM9 ),
        REG_MI_MBK_A2_FIELD( 1, MI_WRAM_A_OFFSET_128KB, MI_WRAM_ARM9 ),
        REG_MI_MBK_A3_FIELD( 1, MI_WRAM_A_OFFSET_192KB, MI_WRAM_ARM9 ),
    },
    {
        REG_MI_MBK_B0_FIELD( 1, MI_WRAM_BC_OFFSET_0KB  , MI_WRAM_ARM7 ),
        REG_MI_MBK_B1_FIELD( 1, MI_WRAM_BC_OFFSET_32KB , MI_WRAM_ARM7 ),
        REG_MI_MBK_B2_FIELD( 1, MI_WRAM_BC_OFFSET_64KB , MI_WRAM_ARM7 ),
        REG_MI_MBK_B3_FIELD( 1, MI_WRAM_BC_OFFSET_96KB , MI_WRAM_ARM7 ),
        REG_MI_MBK_B4_FIELD( 1, MI_WRAM_BC_OFFSET_128KB, MI_WRAM_ARM7 ),
        REG_MI_MBK_B5_FIELD( 1, MI_WRAM_BC_OFFSET_160KB, MI_WRAM_ARM7 ),
        REG_MI_MBK_B6_FIELD( 1, MI_WRAM_BC_OFFSET_192KB, MI_WRAM_ARM7 ),
//        REG_MI_MBK_B7_FIELD( 1, MI_WRAM_BC_OFFSET_224KB, MI_WRAM_ARM7 ),
        REG_MI_MBK_B7_FIELD( 1, MI_WRAM_BC_OFFSET_224KB, MI_WRAM_ARM9 ),
    },
    {
        REG_MI_MBK_C0_FIELD( 1, MI_WRAM_BC_OFFSET_0KB  , MI_WRAM_ARM7 ),
        REG_MI_MBK_C1_FIELD( 1, MI_WRAM_BC_OFFSET_32KB , MI_WRAM_ARM7 ),
        REG_MI_MBK_C2_FIELD( 1, MI_WRAM_BC_OFFSET_64KB , MI_WRAM_ARM7 ),
        REG_MI_MBK_C3_FIELD( 1, MI_WRAM_BC_OFFSET_96KB , MI_WRAM_ARM7 ),
        REG_MI_MBK_C4_FIELD( 1, MI_WRAM_BC_OFFSET_128KB, MI_WRAM_ARM7 ),
        REG_MI_MBK_C5_FIELD( 1, MI_WRAM_BC_OFFSET_160KB, MI_WRAM_ARM7 ),
        REG_MI_MBK_C6_FIELD( 1, MI_WRAM_BC_OFFSET_192KB, MI_WRAM_ARM7 ),
        REG_MI_MBK_C7_FIELD( 1, MI_WRAM_BC_OFFSET_224KB, MI_WRAM_ARM7 ),
    },
    REG_MI_MBK6_FIELD(  REG_WRAM_MAP_CONV_ADDR( 6, A, EADDR, HW_WRAM_AREA_HALF ),
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 6, A, SADDR, HW_WRAM_AREA_HALF - HW_WRAM_A_SIZE )
                        ),
/*
    REG_MI_MBK7_FIELD(  REG_WRAM_MAP_CONV_ADDR( 7, B, EADDR, HW_WRAM_AREA + HW_WRAM_B_SIZE ),
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 7, B, SADDR, HW_WRAM_AREA )
                        ),
*/
    REG_MI_MBK7_FIELD(  REG_WRAM_MAP_CONV_ADDR( 7, B, EADDR, HW_WRAM_AREA_HALF - HW_WRAM_A_SIZE ),  // joint to WRAM-A
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 7, B, SADDR, HW_WRAM_AREA )
                        ),
    REG_MI_MBK8_FIELD(  REG_WRAM_MAP_CONV_ADDR( 8, C, EADDR, MI_WRAM_MAP_NULL ),
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 8, C, SADDR, MI_WRAM_MAP_NULL )
                        ),

    // ARM7
    REG_MI_MBK6_FIELD(  REG_WRAM_MAP_CONV_ADDR( 6, A, EADDR, MI_WRAM_MAP_NULL ),
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 6, A, SADDR, MI_WRAM_MAP_NULL )
                        ),
    REG_MI_MBK7_FIELD(  REG_WRAM_MAP_CONV_ADDR( 7, B, EADDR, HW_WRAM_AREA + HW_WRAM_B_SIZE ),
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 7, B, SADDR, HW_WRAM_AREA )
                        ),
    REG_MI_MBK8_FIELD(  REG_WRAM_MAP_CONV_ADDR( 8, C, EADDR, HW_WRAM_AREA_HALF - HW_WRAM_SIZE ),
                        MI_WRAM_IMAGE_256KB,
                        REG_WRAM_MAP_CONV_ADDR( 8, C, SADDR, HW_WRAM_AREA_HALF - HW_WRAM_SIZE - HW_WRAM_C_SIZE )
                        ),

    // WRAM Lock
    {
        0,
        0,
        0,
    },

    // WRAM-0/1
    3,

    // VRAM-C
    7,
    // VRAM-D
    7,
};

