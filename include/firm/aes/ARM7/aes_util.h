/*---------------------------------------------------------------------------*
  Project:  TwlIPL - AES - include
  File:     aes_util.h

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

#ifndef TWL_AES_AES_UTIL_H_
#define TWL_AES_AES_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/
void AESi_AddCounter(AESCounter* pCounter, u32 nums);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_AES_AES_UTIL_H_ */
#endif
