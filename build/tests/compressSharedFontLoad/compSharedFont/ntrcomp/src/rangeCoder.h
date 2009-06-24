/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     rangeCoder.h

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

#include "types.h"

#ifndef __NTCOMPRESS_RANGE_CODER_H__
#define __NTCOMPRESS_RANGE_CODER_H__

/*---------------------------------------------------------------------------*
  Name:         RCCompWrite

  Description:  レンジコーダの圧縮

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
s32
RCCompWrite( const u8* srcp, u32 size, u8* dstp );

s32
RCCompRead( const u8* srcp, u32 srcSize, u8* dstp );

s32
RCACompWrite( const u8* srcp, u32 size, u8* dstp );

s32
RCACompRead( const u8* srcp, u32 srcSize, u8* dstp );

#endif // __NTCOMPRESS_RANGE_CODER_H__
