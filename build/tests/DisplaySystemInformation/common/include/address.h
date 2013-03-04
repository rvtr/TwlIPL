/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     main.c

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef __DISP_INFO_ADDRESS_H__
#define __DISP_INFO_ADDRESS_H__

#include <twl.h>

#define DISPINFO_SHARED_SCFG_REG_ADDR	(void*)0x02fff000
#define DISPINFO_SHARED_SCFG_REG_SIZE	0x26U
#define DISPINFO_SHARED_SCFG_WRAM_ADDR	(void*)0x02fff030
#define DISPINFO_SHARED_SCFG_WRAM_SIZE	0x06U

#endif