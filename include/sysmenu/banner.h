/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     banner.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef BANNER_H_
#define BANNER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef BANNER_ROM_OFFSET
#define	BANNER_ROM_OFFSET	(HW_ROM_HEADER_BUF + 0x68)
#endif

#define BNR_VER_MIN					1
#define BNR_VER_MAX					3
#define BNR_HEADER_RSV_SIZE			24

#define BNR_CHINESE_SUPPORT_VER		2
#define BNR_HANGUL_SUPPORT_VER		3

typedef enum
{
    BNR_JAPANESE = 0,
    BNR_ENGLISH  = 1,
    BNR_FRENCH   = 2,
    BNR_GERMAN   = 3,
    BNR_ITALIAN  = 4,
    BNR_SPANISH  = 5,
    BNR_CHINESE  = 6,
    BNR_HANGUL   = 7,
	BNR_LANG_NUM
} BannerFileLangIdx;

#define BNR_LANG_NUM_V1			6
#define BNR_LANG_NUM_V2			1
#define BNR_LANG_NUM_V3			1


#define BNR_HEADER_SIZE       32
#define BNR_VER_OFFSET        0
#define BNR_V1_CRC16_OFFSET   2
#define BNR_IMAGE_OFFSET      BNR_HEADER_SIZE
#define BNR_IMAGE_SIZE       (32 * 32 / (8/4))
#define BNR_PLTT_OFFSET      (BNR_IMAGE_OFFSET + BANNER_IMAGE_SIZE)
#define BNR_PLTT_NUM          16
#define BNR_PLTT_SIZE        (BNR_PLTT_NUM * 2)
#define BNR_LANG_OFFSET      (BNR_PLTT_OFFSET  + BANNER_PLTT_SIZE)
#define BNR_LANG_LENGTH       128
#define BNR_LANG_SIZE        (BNR_LANG_LENGTH * 2)

typedef struct
{
	u8  version;
	u8  reserved_A;
	u16 crc16_v1;
	u16 crc16_v2;
	u16 crc16_v3;
	u8  reserved_B[BNR_HEADER_RSV_SIZE];
} BannerHeader;

typedef struct
{
	u8     image[BNR_IMAGE_SIZE];
	GXRgba pltt[BNR_PLTT_NUM];

	u16    gameName[BNR_LANG_NUM_V1][BNR_LANG_LENGTH];
} BannerFileV1;

typedef struct
{
	u16    gameName[BNR_LANG_NUM_V2][BNR_LANG_LENGTH];
} BannerFileV2;

typedef struct
{
	u16    gameName[BNR_LANG_NUM_V3][BNR_LANG_LENGTH];
} BannerFileV3;

typedef struct
{
	BannerHeader  h;
	BannerFileV1  v1;
	BannerFileV2  v2;
	BannerFileV3  v3;
} BannerFile;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* BANNER_H_ */
#endif
