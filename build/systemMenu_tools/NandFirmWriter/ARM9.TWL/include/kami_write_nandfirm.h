/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_write_nandfirm.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-22#$
  $Rev: 1212 $
  $Author: kamikawa $
 *---------------------------------------------------------------------------*/

#ifndef KAMI_WRITE_NAND_FIRM
#define KAMI_WRITE_NAND_FIRM

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    å^íËã`
 *---------------------------------------------------------------------------*/

typedef void* (*KAMIAlloc)(u32 size);
typedef void  (*KAMIFree)(void* ptr);

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/
BOOL NandfirmProcess(void);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_WRITE_NAND_FIRM */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
