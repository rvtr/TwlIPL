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

#define DHT_MAGIC_CODE_EX   (('N' << 0)|('D' << 8)|('H' << 16)|('X' << 24))

#define DHT_MAGIC_CODE_ADHOC    (('N' << 0)|('D' << 8)|('H' << 16)|('I' << 24))

#include <twl/types.h>
#include <twl/os/common/banner.h>

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

#define DHTHeaderEx DHTHeader

typedef struct DHTDatabase
{
    u8 game_code[4];
    u8 rom_version;
    u8 reserved[3]; // for 4B alignment DHTDatabase array
    u8 hash[2][20];
}
DHTDatabase;

typedef struct DHTDatabaseEx
{
    u8 game_code[4];
    u8 rom_version;
    u8 reserved[3]; // for 4B alignment DHTDatabase array
    u8 banner_hash[20];
}
DHTDatabaseEx;

/*
    ad-hoc対処用
*/
#define DHT_INDIVIDUAL_ENTRY_MAX        8
typedef struct DHTDatabaseAdHoc
{
    u8 game_code[4];
    u8 rom_version;
    u8 reserved[3]; // for 4B alignment DHTDatabase array
    struct
    {
        u32 offset;
        u32 length;
    } entry[DHT_INDIVIDUAL_ENTRY_MAX];
    u8 hash[20];
}
DHTDatabaseAdHoc;

typedef struct DHTFile
{
    DHTHeader   header;
    DHTDatabase database[];
}
DHTFile;

typedef struct DHTFileEx
{
    DHTHeader       header;
    DHTDatabaseEx   database[];
}
DHTFileEx;

typedef struct DHTFileAdHoc
{
    DHTHeader           header;
    DHTDatabaseAdHoc    database[];
}
DHTFileAdHoc;

#define DHT_HMAC_KEY    { \
    0x61, 0xbd, 0xdd, 0x72, 0x7e, 0x72, 0xbe, 0xde, 0xad, 0x3a, 0xdf, 0x7f, 0x3d, 0x2d, 0xf7, 0xa5, \
    0x16, 0x7e, 0xb4, 0xc9, 0x7c, 0x6c, 0x00, 0x7c, 0x57, 0xbb, 0x94, 0x8a, 0x64, 0xcd, 0x4e, 0x1c, \
    0x51, 0x6b, 0xbd, 0xdb, 0x1d, 0xeb, 0x54, 0xe9, 0x34, 0x27, 0xf9, 0x31, 0x51, 0x5e, 0x89, 0x4e, \
    0x7f, 0xd9, 0x7c, 0xe9, 0x92, 0x44, 0x0f, 0xef, 0x6b, 0xb6, 0x12, 0x21, 0x68, 0x88, 0xd8, 0xee  \
}

#define DHT_HMAC_KEY2 { \
    0x85, 0x29, 0x48, 0xf3, 0xa1, 0xbb, 0x13, 0x30, 0x93, 0x5d, 0xb8, 0xc9, 0xa5, 0x9a, 0xe8, 0x30, \
    0xc4, 0xd0, 0x4a, 0xdd, 0xa4, 0x92, 0x81, 0xfd, 0x4f, 0xa1, 0x32, 0xfa, 0x46, 0x05, 0xde, 0x68, \
    0x7b, 0xa7, 0xd7, 0x5b, 0xc9, 0x3a, 0xc8, 0x8d, 0xcd, 0x25, 0x3a, 0x17, 0x3c, 0xc2, 0xd6, 0xe0, \
    0xd2, 0xe5, 0xb9, 0xfb, 0x49, 0xf9, 0x4d, 0x05, 0x70, 0x10, 0x29, 0x51, 0x7a, 0xc5, 0x89, 0x49, \
}

/*
    ユーティリティ
        hp: メモリ上のデータベースの先頭アドレス
*/
#define DHT_GET_SIGN_TARGET_ADDR(hp)    (&((DHTHeader*)hp)->nums)
#define DHT_GET_SIGN_TARGET_SIZE(hp)    (((DHTHeader*)hp)->nums * sizeof(DHTDatabase) + sizeof(u32))
#define DHT_GET_SIGN_TARGET_OFFSET      (int)DHT_GET_SIGN_TARGET_ADDR(0)

#define DHT_GET_SIGN_TARGET_ADDR_EX(hp) DHT_GET_SIGN_TARGET_ADDR(hp)
#define DHT_GET_SIGN_TARGET_SIZE_EX(hp) (((DHTHeader*)hp)->nums * sizeof(DHTDatabaseEx) + sizeof(u32))
#define DHT_GET_SIGN_TARGET_OFFSET_EX   DHT_GET_SIGN_TARGET_OFFSET

#define DHT_GET_SIGN_TARGET_ADDR_ADHOC(hp) DHT_GET_SIGN_TARGET_ADDR(hp)
#define DHT_GET_SIGN_TARGET_SIZE_ADHOC(hp) (((DHTHeader*)hp)->nums * sizeof(DHTDatabaseAdHoc) + sizeof(u32))
#define DHT_GET_SIGN_TARGET_OFFSET_ADHOC   DHT_GET_SIGN_TARGET_OFFSET

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //SYSMENU_DHT_FORMAT_H_
