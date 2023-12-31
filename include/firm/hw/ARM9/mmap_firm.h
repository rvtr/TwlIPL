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

//------------------------------------- FIRM_RESET_BUF
#define HW_FIRM_RESET_BUF               HW_MAIN_MEM
#define HW_FIRM_RESET_BUF_END           (HW_FIRM_RESET_BUF + HW_FIRM_RESET_BUF_SIZE)
//#define HW_FIRM_RESET_BUF_SIZE          0x400  // 12KB
#define HW_FIRM_RESET_BUF_SIZE          0x800000    // 8MB

//------------------------------------- FIRM_RSA_BUF
#define HW_FIRM_RSA_BUF                 (HW_FIRM_RSA_BUF_END - HW_FIRM_RSA_BUF_SIZE)
#define HW_FIRM_RSA_BUF_END             HW_FIRM_ES_BUF
#define HW_FIRM_RSA_BUF_SIZE            0x3000  // 12KB

//------------------------------------- FIRM_ES_BUF
#define HW_FIRM_ES_BUF                  (HW_FIRM_ES_BUF_END - HW_FIRM_ES_BUF_SIZE)
#define HW_FIRM_ES_BUF_END              HW_FIRM_FROM_BROM_BUF
#define HW_FIRM_ES_BUF_SIZE             0x1000  // 4KB

//------------------------------------- FIRM_FROM_BROM_BUF
#define HW_FIRM_FROM_BROM_BUF           (HW_FIRM_FROM_BROM_BUF_END - HW_FIRM_FROM_BROM_BUF_SIZE)
#define HW_FIRM_FROM_BROM_BUF_END       (HW_ITCM_END - 0x1000)  //  END - 4KB
#define HW_FIRM_FROM_BROM_BUF_SIZE      0x3000  // 12KB

//------------------------------------- FIRM_FROM_FIRM_BUF
#define HW_FIRM_FROM_FIRM_BUF           (HW_FIRM_FROM_FIRM_BUF_END - HW_FIRM_FROM_FIRM_BUF_SIZE)
#define HW_FIRM_FROM_FIRM_BUF_END       (HW_ITCM_END - 0x1000)  //  END - 4KB
#define HW_FIRM_FROM_FIRM_BUF_SIZE      0x2C00  // 11KB


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_HW_MMAP_FIRM_H_ */
#endif
