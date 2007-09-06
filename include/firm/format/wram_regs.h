/*---------------------------------------------------------------------------*
  Project:  TwlFirm - MI - include
  File:     wram_regs.h

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
#ifndef FIRM_FORMAT_WRAM_REGS_H_
#define FIRM_FORMAT_WRAM_REGS_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	MI_WRAM_A_OFFSET_0KB   = 0,
	MI_WRAM_A_OFFSET_64KB  = 1,
	MI_WRAM_A_OFFSET_128KB = 2,
	MI_WRAM_A_OFFSET_192KB = 3
} MIWramAOffset;

typedef enum
{
	MI_WRAM_BC_OFFSET_0KB   = 0,
	MI_WRAM_BC_OFFSET_32KB  = 1,
	MI_WRAM_BC_OFFSET_64KB  = 2,
	MI_WRAM_BC_OFFSET_96KB  = 3,
	MI_WRAM_BC_OFFSET_128KB = 4,
	MI_WRAM_BC_OFFSET_160KB = 5,
	MI_WRAM_BC_OFFSET_192KB = 6,
	MI_WRAM_BC_OFFSET_224KB = 7
} MIWramBCOffset;


#define MI_WRAM_MAP_NULL        HW_WRAM_AREA

#define REG_WRAM_MAP_CONV_ADDR( regno, abc, border, addr ) \
( \
    ((((addr) - HW_WRAM_AREA) / MI_WRAM_##abc##_SLOT_SIZE) & \
	 (REG_MI_MBK##regno##_W##abc##_##border##_MASK >> \
	  REG_MI_MBK##regno##_W##abc##_##border##_SHIFT)) \
)


// WRAMマッピングレジスタ
typedef struct
{
    u8      main_wrambnk_a[4];         // ARM9 SCFG_MBK1
    u8      main_wrambnk_b[8];         // ARM9 SCFG_MBK2-3
    u8      main_wrambnk_c[8];         // ARM9 SCFG_MBK4-5

    u32     main_wrammap_a;            // ARM9 SCFG_MBK6
    u32     main_wrammap_b;            // ARM9 SCFG_MBK7
    u32     main_wrammap_c;            // ARM9 SCFG_MBK8

    u32     sub_wrammap_a;             // ARM7 SCFG_MBK6
    u32     sub_wrammap_b;             // ARM7 SCFG_MBK7
    u32     sub_wrammap_c;             // ARM7 SCFG_MBK8

    u8      sub_wramlock[3];           // ARM7 SCFG_MBK9

    u8      main_wrambnk_01:2;         // ARM9 RBKCNT1_H
    u8      main_vrambnk_c:3;          // ARM9 RBKCNT0_H
    u8      main_vrambnk_d:3;          // ARM9 RBKCNT0_H
}
MIHeader_WramRegs;

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_FORMAT_WRAM_REGS_H_ */
#endif
