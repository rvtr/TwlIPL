/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS - include
  File:     systemCall.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef FIRM_OS_SYSTEMCALL_H_
#define FIRM_OS_SYSTEMCALL_H_

#include <twl/os/common/systemCall.h>

#ifdef __cplusplus
extern "C" {
#endif


#define SVC_ID_PREMASK              0x3f
#define SVC_ID_SHIFT                1

#define SVC_ID_SEMIHOST_ARM         0x12
#define SVC_ID_SEMIHOST_THUMB       0xab


typedef enum
{
    // TWL
    SVC_ID_INIT_SIGN_HEAP = 32,
    SVC_ID_DEC_RSA = 33,
    SVC_ID_DEC_SIGN = 34,
    SVC_ID_DEC_SIGN_DER = 35,
    SVC_ID_SHA1_INIT = 36,
    SVC_ID_SHA1_UPDATE = 37,
    SVC_ID_SHA1_FINAL = 38,
    SVC_ID_CALC_SHA1 = 39,
    SVC_ID_CMP_SHA1 = 40,
    SVC_ID_RAND_SHA1 = 41,
    SVC_ID_LZ8_DEV = 1,
    SVC_ID_LZ16_DEV_IMG = 2,

    // reserve 0x2b for semihosting (0xab & 0x3f == 0x2b)

    // DS compatible
    SVC_ID_SOFT_RESET = 0,

    SVC_ID_WAIT_BY_LOOP = 3,
    SVC_ID_WAIT_INTR = 4,
    SVC_ID_WAIT_VB_INTR = 5,
    SVC_ID_HALT = 6,
    SVC_ID_SLEEP = 7,
    SVC_ID_SND_BIAS = 8,
    SVC_ID_DIV = 9,

    SVC_ID_CPU_SET = 11,
    SVC_ID_CPU_SET_FAST = 12,
    SVC_ID_SQRT = 13,
    SVC_ID_CRC16 = 14,
    SVC_ID_IS_MEMEX = 15,
    SVC_ID_UNPACKBITS_DEV = 16,
    SVC_ID_LZ8 = 17,
    // overlap semihosting ((0x123456>>16) & 0x3f == 0x12)
    SVC_ID_LZ16_DEV = 18,
    SVC_ID_HUFF_DEV = 19,
    SVC_ID_RL8 = 20,
    SVC_ID_RL16_DEV = 21,
    SVC_ID_DF8 = 22,

    SVC_ID_DF16 = 24,

    SVC_ID_SND_SIN = 26,
    SVC_ID_SND_PITCH = 27,
    SVC_ID_SND_VOL = 28,
    SVC_ID_DS_IPL2 = 29,

    SVC_ID_PAUSE_HI = 31
}
OSSvcID;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_OS_SYSTEMCALL_H_ */
#endif
