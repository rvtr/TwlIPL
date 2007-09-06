/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenorfirm
  File:     compress.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef COMPRESS_H_
#define COMPRESS_H_

#include    "misc.h"


/*---------------------------------------------------------------------------*
  Name:         MI_CompressLZ

  Description:  LZ77圧縮を行なう関数

  Arguments:    srcp            圧縮元データへのポインタ
                size            圧縮元データサイズ
                dstp            圧縮先データへのポインタ
                                圧縮元データよりも大きいサイズのバッファが必要です。

  Returns:      圧縮後のデータサイズ。
 *---------------------------------------------------------------------------*/
u32 LZCompWrite(u8 *srcp, u32 size, u8 *dstp, int boundary);


#endif //COMPRESS_H_
