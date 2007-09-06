/*---------------------------------------------------------------------------*
  Project:  TwlFirm - HW - include
  File:     mmap_firm.h

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
#ifndef FIRM_HW_MMAP_FIRM_H_
#define FIRM_HW_MMAP_FIRM_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------- NORFIRM
#define HW_NORFIRM                      HW_WRAM
#define HW_NORFIRM_END                  (HW_NORFIRM + HW_NORFIRM_SIZE)
#define HW_NORFIRM_SIZE                 (HW_WRAM_0_SIZE + HW_WRAM_1_SIZE + HW_WRAM_A_SIZE_MAX + HW_WRAM_B_SIZE_MAX)

//------------------------------------- NORFIRM_WRAM_ABC
#define HW_NORFIRM_WRAM_A_MAP_END       (HW_WRAM_AREA_END - HW_PRV_WRAM_SIZE)
#define HW_NORFIRM_WRAM_B_MAP_END       HW_NORFIRM_WRAM_A_MAP_END
#define HW_NORFIRM_WRAM_C_MAP_END       HW_NORFIRM_WRAM_A_MAP_END

//------------------------------------- HW_NORFIRM_FROM_BROM_BUF
#define HW_NORFIRM_FROM_BROM_BUF        (HW_NORFIRM_FROM_BROM_BUF_END - HW_NORFIRM_FROM_BROM_BUF_SIZE)
#define HW_NORFIRM_FROM_BROM_BUF_END    (HW_WRAM_AREA_END - 0x1000)  // END - 4KB
#define HW_NORFIRM_FROM_BROM_BUF_SIZE   0x3000  // 12KB


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_HW_MMAP_FIRM_H_ */
#endif
