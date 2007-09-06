/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenandfirm
  File:     compress.c

  Copyright 2007 Nintendo.   All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>                 // atoi()
#include    <string.h>                 // strcmp()
#include    <ctype.h>                  // isprint()
#include    <unistd.h>                 // chdir()
#include    <tchar.h>
#include    <limits.h>                 // UCHAR_MAX
#include    <time.h>
#include    <sys/stat.h>               // stat()
#include    "elf.h"
#include    "misc.h"
#include    "defval.h"
#include    "format_rom.h"
#include    "format_nlist.h"
#include    "makenandfirm.h"

//#define ADD_HEADER

#define DIFF_CODE_HEADER        (0x80)
#define RL_CODE_HEADER          (0x30)
#define LZ_CODE_HEADER          (0x10)
#define HUFF_CODE_HEADER        (0x20)
#define CODE_HEADER_MASK        (0xF0)

//===========================================================================
//  LZ77圧縮
//===========================================================================
static u8 SearchLZ(const u8 *nextp, u32 remainSize, u16 *offset);

static u16 windowPos;
static u16 windowLen;

static s16 LZOffsetTable[4096];
static s16 LZByteTable[256];
static s16 LZEndTable[256];


static void LZInitTable(void)
{
    u16     i;

    for (i = 0; i < 256; i++)
    {
        LZByteTable[i] = -1;
        LZEndTable[i] = -1;
    }
    windowPos = 0;
    windowLen = 0;
}

static void SlideByte(const u8 *srcp)
{
    s16     offset;
    u8      in_data = *srcp;
    u16     insert_offset;

    if (windowLen == 4096)
    {
        u8      out_data = *(srcp - 4096);
        if ((LZByteTable[out_data] = LZOffsetTable[LZByteTable[out_data]]) == -1)
        {
            LZEndTable[out_data] = -1;
        }
        insert_offset = windowPos;
    }
    else
    {
        insert_offset = windowLen;
    }

    offset = LZEndTable[in_data];
    if (offset == -1)
    {
        LZByteTable[in_data] = insert_offset;
    }
    else
    {
        LZOffsetTable[offset] = insert_offset;
    }
    LZEndTable[in_data] = insert_offset;
    LZOffsetTable[insert_offset] = -1;

    if (windowLen == 4096)
    {
        windowPos = (u16)((windowPos + 1) % 0x1000);
    }
    else
    {
        windowLen++;
    }
}

static void LZSlide(const u8 *srcp, u32 n)
{
    u32     i;

    for (i = 0; i < n; i++)
    {
        SlideByte(srcp++);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_CompressLZ

  Description:  LZ77圧縮を行なう関数

  Arguments:    srcp            圧縮元データへのポインタ
                size            圧縮元データサイズ
                dstp            圧縮先データへのポインタ
                                圧縮元データよりも大きいサイズのバッファが必要です。

  Returns:      圧縮後のデータサイズ。
                圧縮後のデータが圧縮前よりも大きくなる場合には圧縮を中断し0を返します。
 *---------------------------------------------------------------------------*/
u32 LZCompWrite(u8 *srcp, u32 size, u8 *dstp, int boundary)
{
    u32     LZDstCount;                // 圧縮データのバイト数
    u8      LZCompFlags;               // 圧縮の有無を示すフラグ系列
    u8     *LZCompFlagsp;              // LZCompFlags を格納するメモリ領域をポイント
    u16     lastOffset;                // 一致データまでのオフセット (その時点での最長一致データ)
    u8      lastLength;                // 一致データ長 (その時点での最長一致データ)
    u8      i;
    u32     dstMax;

#ifdef ADD_HEADER
    *(u32 *)dstp = size << 8 | LZ_CODE_HEADER;  // データ・ヘッダ
    dstp += 4;
#endif
    LZDstCount = 4;
    dstMax = size;
    LZInitTable();

    while (size > 0)
    {
        LZCompFlags = 0;
        LZCompFlagsp = dstp++;         // フラグ系列の格納先
        LZDstCount++;

        // フラグ系列が8ビットデータとして格納されるため、8回ループ
        for (i = 0; i < 8; i++)
        {
            LZCompFlags <<= 1;         // 初回 (i=0) は特に意味はない
            if (size <= 0)
            {
                // 終端に来た場合はフラグを最後までシフトさせてから終了
                continue;
            }

            if ((lastLength = SearchLZ(srcp, size, &lastOffset)))
            {
                // 圧縮可能な場合はフラグを立てる
                LZCompFlags |= 0x1;

                // オフセットは上位4ビットと下位8ビットに分けて格納
                *dstp++ = (u8)((lastLength - 3) << 4 | (lastOffset - 1) >> 8);
                *dstp++ = (u8)((lastOffset - 1) & 0xff);
                LZDstCount += 2;
                LZSlide(srcp, lastLength);
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // 圧縮なし
                LZSlide(srcp, 1);
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // 8回ループ終了
        *LZCompFlagsp = LZCompFlags;   // フラグ系列を格納
    }

    // 16バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含める
    i = 0;
    while (LZDstCount & (boundary - 1))
//    while ((LZDstCount + i) & 0x3)
    {
        *dstp++ = 0;
        LZDstCount++;
        i++;
    }

    return LZDstCount;
}

//--------------------------------------------------------
// LZ77圧縮でスライド窓の中から最長一致列を検索します。
//  Arguments:    startp                 データの開始位置を示すポインタ
//                nextp                  検索を開始するデータのポインタ
//                remainSize             残りデータサイズ
//                offset                 一致したオフセットを格納する領域へのポインタ
//  Return   :    一致列が見つかった場合は   TRUE
//                見つからなかった場合は     FALSE
//--------------------------------------------------------
static u8 SearchLZ(const u8 *nextp, u32 remainSize, u16 *offset)
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     maxOffset;
    u8      maxLength = 2;
    u8      tmpLength;
    s32     w_offset;

    if (remainSize < 3)
    {
        return 0;
    }

    w_offset = LZByteTable[*nextp];

    while (w_offset != -1)
    {
        if (w_offset < windowPos)
        {
            searchp = nextp - windowPos + w_offset;
        }
        else
        {
            searchp = nextp - windowLen - windowPos + w_offset;
        }

        /* 無くても良いが、僅かに高速化する */
        if (*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset = LZOffsetTable[w_offset];
            continue;
        }

        if (nextp - searchp < 2)
        {
            // VRAMは2バイトアクセスなので (VRAMからデータを読み出す場合があるため)、
            // 検索対象データは2バイト前からのデータにしなければならない。
            //
            // オフセットは12ビットで格納されるため、4096以下
            break;
        }
        tmpLength = 3;
        searchHeadp = searchp + 3;
        headp = nextp + 3;

        while (((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;

            // データ長は4ビットで格納されるため、18以下 (3の下駄をはかせる)
            if (tmpLength == (0xF + 3))
            {
                break;
            }
        }
        if (tmpLength > maxLength)
        {
            // 最大長オフセットを更新
            maxLength = tmpLength;
            maxOffset = (u16)(nextp - searchp);
            if (maxLength == (0xF + 3))
            {
                // 一致長が最大なので、検索を終了する。
                break;
            }
        }
        w_offset = LZOffsetTable[w_offset];
    }

    if (maxLength < 3)
    {
        return 0;
    }
    *offset = maxOffset;
    return maxLength;
}

