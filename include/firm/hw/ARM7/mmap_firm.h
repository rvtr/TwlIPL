/*---------------------------------------------------------------------------*
  Project:  TwlFirm - HW - include
  File:     mmap_firm.h

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
#ifndef FIRM_HW_MMAP_FIRM_H_
#define FIRM_HW_MMAP_FIRM_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------- FIRM
#define HW_FIRM                         HW_WRAM
#define HW_FIRM_END                     (HW_FIRM + HW_FIRM_SIZE)
#define HW_FIRM_SIZE                    (HW_WRAM_0_SIZE + HW_WRAM_1_SIZE + HW_WRAM_A_SIZE_MAX + HW_WRAM_B_SIZE_MAX)

//------------------------------------- FIRM_SVC_STACK
#define HW_FIRM_SVC_STACK               (HW_FIRM_SVC_STACK_END-HW_FIRM_SVC_STACK_SIZE)
#define HW_FIRM_SVC_STACK_END           (HW_WRAM_AREA_END - HW_PRV_WRAM_SYSRV_SIZE)
#define HW_FIRM_SVC_STACK_SIZE          HW_SVC_STACK_SIZE

//------------------------------------- FIRM_IRQ_STACK
#define HW_FIRM_IRQ_STACK               (HW_FIRM_IRQ_STACK_END-HW_FIRM_IRQ_STACK_SIZE)
#define HW_FIRM_IRQ_STACK_END           HW_FIRM_SVC_STACK
#define HW_FIRM_IRQ_STACK_SIZE          SDK_IRQ_STACKSIZE

//------------------------------------- HW_FIRM_FROM_BROM_BUF
#define HW_FIRM_FROM_BROM_BUF           (HW_FIRM_FROM_BROM_BUF_END - HW_FIRM_FROM_BROM_BUF_SIZE)
#define HW_FIRM_FROM_BROM_BUF_END       (HW_WRAM_AREA_END - 0x1000)  // END - 4KB
#define HW_FIRM_FROM_BROM_BUF_SIZE      0x3000  // 12KB

//------------------------------------- HW_FIRM_FROM_FIRM_BUF
#define HW_FIRM_FROM_FIRM_BUF           (HW_FIRM_FROM_FIRM_BUF_END - HW_FIRM_FROM_FIRM_BUF_SIZE)
#define HW_FIRM_FROM_FIRM_BUF_END       (HW_WRAM_AREA_END - 0x1000)  // END - 4KB
#define HW_FIRM_FROM_FIRM_BUF_SIZE      0x2C00  // 11KB

//------------------------------------- HW_FIRM_APP_BUF
#define HW_FIRM_APP_BUF                 (HW_MAIN_MEM_HI_EX_END - HW_FIRM_APP_BUF_SIZE)
#define HW_FIRM_APP_BUF_END             (HW_FIRM_APP_BUF + HW_FIRM_APP_BUF_SIZE)
#define HW_FIRM_APP_BUF_SIZE            0x00800000  // 8MB

//------------------------------------- HW_FIRM_BOOT_CORE
#define HW_FIRM_BOOT_CORE               HW_FIRM_FROM_BROM_BUF_END
#define HW_FIRM_BOOT_CORE_END           (HW_FIRM_BOOT_CORE + HW_FIRM_BOOT_CORE_SIZE)
#define HW_FIRM_BOOT_CORE_SIZE          0x200  // 512B


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_HW_MMAP_FIRM_H_ */
#endif
