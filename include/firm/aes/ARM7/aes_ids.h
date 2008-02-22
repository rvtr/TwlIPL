/*---------------------------------------------------------------------------*
  Project:  TwlFirmSDK - AES - include
  File:     aes_ids.h

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

#ifndef TWL_AES_AES_IDS_H_
#define TWL_AES_AES_IDS_H_

#define AES_IDS_ID0_C(c)    (((unsigned long)c[0] << 0) | ((unsigned long)c[1] << 8) | ((unsigned long)c[2] << 16) | ((unsigned long)c[3] << 24))
#define AES_IDS_ID0_D(c)    (((unsigned long)c[3] << 0) | ((unsigned long)c[2] << 8) | ((unsigned long)c[1] << 16) | ((unsigned long)c[0] << 24))

#define AES_IDS_ID1_A(c)    (((unsigned long)c[3] << 0) | ((unsigned long)c[1] << 8) | ((unsigned long)c[2] << 16) | ((unsigned long)c[0] << 24))
#define AES_IDS_ID1_B(c)    (((unsigned long)c[0] << 0) | ((unsigned long)c[2] << 8) | ((unsigned long)c[1] << 16) | ((unsigned long)c[3] << 24))

/* TWL_AES_AES_IDS_H_ */
#endif
