/*---------------------------------------------------------------------------*
  Project:  TwlIPL - AES - include
  File:     aes_init.h

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

#ifndef TWL_AES_AES_INIT_H_
#define TWL_AES_AES_INIT_H_

#include <firm/pxi.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         AESi_SendSeed

  Description:  send SEED/KEY to ARM7 via PXI.

  Arguments:    pSeed   pointer to seed

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_SendSeed( AESKey *pSeed )
{
    PXI_SendDataByFifo( PXI_FIFO_TAG_DATA, pSeed, AES_BLOCK_SIZE );
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_AES_AES_INIT_H_ */
#endif
