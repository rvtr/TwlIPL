/*---------------------------------------------------------------------------*
  Project:  TwlFirm - include - AES
  File:     aes.h

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
#ifndef FIRM_AES_H_
#define FIRM_AES_H_

#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

#include <twl/aes/common/types.h>

#ifdef SDK_ARM7
#include <twl/aes/ARM7/hi.h>
#include <twl/aes/ARM7/lo.h>
#include <firm/aes/ARM7/aes_init.h>
#include <firm/aes/ARM7/aes_ids.h>
#else // !SDK_ARM7
#include <firm/aes/ARM9/aes_init.h>
#endif // !SDK_ARM7

/* FIRM_AES_H_ */
#endif
