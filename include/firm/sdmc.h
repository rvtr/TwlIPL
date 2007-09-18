/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - sdmc
  File:     sdmc.h

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

#ifndef FIRM_SDMC_H_
#define FIRM_SDMC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  SDK_ARM9
#else  // SDK_ARM7
#include <firm/devices/firm_sdmc/ARM7/sdif_ip.h>
#include <firm/devices/firm_sdmc/ARM7/sdif_reg.h>
#include <firm/devices/firm_sdmc/ARM7/sdmc.h>
#include <firm/devices/firm_sdmc/ARM7/sdmc_config.h>
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_SDMC_H_ */
#endif
