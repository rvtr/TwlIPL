/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compstatic
  File:     common.h

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
#ifndef COMMON_H__
#define COMMON_H__

typedef enum
{
    TRUE = 1,
    FALSE = 0
}
BOOL;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef signed char s8;
typedef signed short s16;
typedef signed long s32;

// macro
#define MIN(a,b)                ((a)<(b)?(a):(b))
#define ROUNDUP4(x)             (((x)+3)&~3)
#define LE(a)   ((((a)<<24)&0xff000000)|(((a)<<8)&0x00ff0000)|\
                 (((a)>>8)&0x0000ff00)|(((a)>>24)&0x000000ff))
#define FREE(x)  do { if (x){ free(x); x = NULL; } } while(0)


#endif //COMMON_H__
