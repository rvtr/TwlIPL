/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - PXI
  File:     regname.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef FIRM_PXI_COMMON_REGNAME_EX_H_
#define FIRM_PXI_COMMON_REGNAME_EX_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  SDK_ARM9

#define REG_PXI_FIFO_CNT_SEND_FULL_SHIFT REG_PXI_SUBP_FIFO_CNT_SEND_FULL_SHIFT
#define REG_PXI_FIFO_CNT_RECV_EMP_SHIFT  REG_PXI_SUBP_FIFO_CNT_RECV_EMP_SHIFT

#else  // SDK_ARM7

#define REG_PXI_FIFO_CNT_SEND_FULL_SHIFT REG_PXI_MAINP_FIFO_CNT_SEND_FULL_SHIFT
#define REG_PXI_FIFO_CNT_RECV_EMP_SHIFT  REG_PXI_MAINP_FIFO_CNT_RECV_EMP_SHIFT

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* FIRM_PXI_COMMON_REGNAME_EX_H_ */
#endif
