/*---------------------------------------------------------------------------*
  Project:  TwlFirmSDK - AES - include
  File:     aes_ids.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_AES_IDS_H_
#define TWL_AES_AES_IDS_H_

#define AES_IDS_ID0_A   (('N' << 0) | ('i' << 8) | ('n' << 16) | ('t' << 24))
#define AES_IDS_ID0_B   (('e' << 0) | ('n' << 8) | ('d' << 16) | ('o' << 24))
#define AES_IDS_ID0_C(c)    (((unsigned long)c[0] << 0) | ((unsigned long)c[1] << 8) | ((unsigned long)c[2] << 16) | ((unsigned long)c[3] << 24))
#define AES_IDS_ID0_D(c)    (((unsigned long)c[3] << 0) | ((unsigned long)c[2] << 8) | ((unsigned long)c[1] << 16) | ((unsigned long)c[0] << 24))

#define AES_IDS_ID1_A(c)    (((unsigned long)c[3] << 0) | ((unsigned long)c[1] << 8) | ((unsigned long)c[2] << 16) | ((unsigned long)c[0] << 24))
#define AES_IDS_ID1_B(c)    (((unsigned long)c[0] << 0) | ((unsigned long)c[2] << 8) | ((unsigned long)c[1] << 16) | ((unsigned long)c[3] << 24))
#define AES_IDS_ID1_C   (*(unsigned long*)0x04004d04)
#define AES_IDS_ID1_D   (*(unsigned long*)0x04004d00)

#define AES_IDS_ID2_A   (('N' << 0) | ('i' << 8) | ('n' << 16) | ('t' << 24))
#define AES_IDS_ID2_B   (('e' << 0) | ('n' << 8) | ('d' << 16) | ('o' << 24))
#define AES_IDS_ID2_C   ((' ' << 0) | ('D' << 8) | ('S' << 16) | ('\0' << 24))
#define AES_IDS_ID2_D   ((0x01 << 0) | (0x23 << 8) | (0x21 << 16) | (0x00 << 24))

#define AES_IDS_ID3_A   (*(unsigned long*)0x04004d00)
#define AES_IDS_ID3_B   (('N' << 0) | ('I' << 8) | ('N' << 16) | ('T' << 24))
#define AES_IDS_ID3_C   (('E' << 0) | ('N' << 8) | ('D' << 16) | ('O' << 24))
#define AES_IDS_ID3_D   (*(unsigned long*)0x04004d04)

/* TWL_AES_AES_IDS_H_ */
#endif
