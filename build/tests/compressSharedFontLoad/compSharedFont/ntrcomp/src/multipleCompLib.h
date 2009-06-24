/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     multipleCompLib.h

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

// nitroCompLib.h : nitroCompLib.DLL のメイン ヘッダー ファイル
//

#ifndef __MULTIPLE_COMPLIB_H__
#define __MULTIPLE_COMPLIB_H__

//===========================================================================================
// インクルード
//===========================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"

//===========================================================================================
// プロトタイプ宣言
//===========================================================================================
// C++用
#ifdef __cplusplus
extern "C"
{
#endif

s32 LHCompRead( const u8* srcp, u32 srcSize, u8* dstp );

u32 LHCompWrite( const u8* srcp, s32 size, u8* dstp );

s32 LRCCompRead( const u8* srcp, u32 srcSize, u8* dstp );

u32 LRCCompWrite( const u8* srcp, u32 size, u8* dstp );


#ifdef __cplusplus
}
#endif

#endif // __MULTIPLEX_COMPLIB_H__
