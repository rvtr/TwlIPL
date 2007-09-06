/*---------------------------------------------------------------------------*
  Project:  TwlFirm - MI - include
  File:     wram_regs.h

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
#ifndef FIRM_FORMAT_WRAM_REGS_H_
#define FIRM_FORMAT_WRAM_REGS_H_


#ifdef __cplusplus
extern "C" {
#endif

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
