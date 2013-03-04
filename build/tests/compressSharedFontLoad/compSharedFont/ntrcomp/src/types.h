/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     types.h

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

#ifndef __NTCOMPRESS_TYPES_H__
#define __NTCOMPRESS_TYPES_H__

//===========================================================================================
// Œ^’è‹`
//===========================================================================================
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
typedef signed   char  s8;
typedef signed   short s16;
typedef signed   long  s32;
typedef float          f32;
typedef unsigned long  BOOL;
#define     FALSE       0
#define     TRUE        1

#define DIFF_CODE_HEADER        (0x80)
#define LZ_CODE_HEADER          (0x10)
#define HUFF_CODE_HEADER        (0x20)
#define RL_CODE_HEADER          (0x30)
#define LH_CODE_HEADER          (0x40)
#define LRC_CODE_HEADER         (0x50)
#define CODE_HEADER_MASK        (0xF0)


#if defined( __GNUC__ )
  typedef unsigned long long int u64;
  typedef signed long long int   s64;
  #define INLINE inline
  #define ASSERT(x) (void)0 
#else
  #include <assert.h>
  #define INLINE __inline
  #define ASSERT assert
  typedef unsigned __int64 u64;
  typedef signed __int64   s64;
#endif

#endif // __NTCOMPRESS_TYPES_H__
