/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compstatic
  File:     compress.h

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef COMPRESS_H__
#define COMPRESS_H__
#include "common.h"

//---------------------------------------------------------
typedef struct
{
    u32     bufferTop:24;              // 圧縮領域終端 - 先頭
    u32     compressBottom:8;          // 圧縮領域終端 - データ終端
    u32     originalBottom;            // 展開領域終端 - 圧縮領域終端
}
CompFooter;

//---------------------------------------------------------
int     Compress(u8 *buffer, int buffer_size);

#define	COMPRESS_LARGER_ORIGINAL	(-1)
#define	COMPRESS_FATAL_ERROR		(-2)


// loader area
#define LOADER_SIZE_ARM9        (16*1024)
#define LOADER_SIZE_ARM7        ( 1*1024)

// LZ compress parameters
#define LZ_BIT_INDEX            12     // 12bit offset
#define LZ_BIT_LENGTH            4     //  4bit length
#define LZ_MAX_INDEX            (1 << LZ_BIT_INDEX)
#define LZ_MAX_LENGTH           (1 << LZ_BIT_LENGTH)

#define LZ_MIN_COPY             3
#define LZ_MAX_COPY             (LZ_MIN_COPY+LZ_MAX_LENGTH-1)
#define LZ_MAX_DIC_LENGTH       (LZ_MIN_COPY+LZ_MAX_INDEX-1)

// macro
#define MIN(a,b)                ((a)<(b)?(a):(b))
#define ROUNDUP4(x)             (((x)+3)&~3)

#endif
