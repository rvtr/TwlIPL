/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht_format.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef SYSMENU_DHT_FORMAT_H_
#define SYSMENU_DHT_FORMAT_H_

#define DHT_MAGIC_CODE      (('N' << 0)|('D' << 8)|('H' << 16)|('T' << 24))
#define DHT_DS_HEADER_SIZE  0x160
#define DHT_OVERLAY_MAX     (512*1024)

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DHTHeader
{
    u32 magic_code; // fixed
    u8  sign[128];  // for nums member and whole DHTDatabase array
    u32 nums;       // number of subsequent DHTDatabase array
}
DHTHeader;

typedef struct DHTDatabase
{
    u8 game_code[4];
    u8 rom_version;
    u8 reserved[3]; // for 4B alignment DHTDatabase array
    u8 hash[2][20];
}
DHTDatabase;

typedef struct DHTFile
{
    DHTHeader   header;
    DHTDatabase database[];
}
DHTFile;

#define DHT_HMAC_KEY    { \
    0x61, 0xbd, 0xdd, 0x72, 0x7e, 0x72, 0xbe, 0xde, 0xad, 0x3a, 0xdf, 0x7f, 0x3d, 0x2d, 0xf7, 0xa5, \
    0x16, 0x7e, 0xb4, 0xc9, 0x7c, 0x6c, 0x00, 0x7c, 0x57, 0xbb, 0x94, 0x8a, 0x64, 0xcd, 0x4e, 0x1c, \
    0x51, 0x6b, 0xbd, 0xdb, 0x1d, 0xeb, 0x54, 0xe9, 0x34, 0x27, 0xf9, 0x31, 0x51, 0x5e, 0x89, 0x4e, \
    0x7f, 0xd9, 0x7c, 0xe9, 0x92, 0x44, 0x0f, 0xef, 0x6b, 0xb6, 0x12, 0x21, 0x68, 0x88, 0xd8, 0xee  \
}


/*
    ユーティリティ
        hp: メモリ上のファイルの先頭アドレス
*/
#define DHT_GET_SIGN_TARGET_ADDR(hp)    (&((DHTHeader*)hp)->nums)
#define DHT_GET_SIGN_TARGET_SIZE(hp)    (((DHTHeader*)hp)->nums * sizeof(DHTDatabase) + sizeof(u32))

#define DHT_GET_SIGN_TARGET_OFFSET      (int)DHT_GET_SIGN_TARGET_ADDR(0)


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //SYSMENU_DHT_FORMAT_H_
