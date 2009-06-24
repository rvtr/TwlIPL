/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compstatic
  File:     compress.c

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
#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "compress.h"

static int LZCompressRV(u8 *src_buffer, int src_size, u8 *dst_buffer, int dst_size);
static int FindMatched(u8 *src_buffer, int src_size, u8 *dic_buffer, int dic_size, int *index);
static int HowManyMatched(u8 *src_buffer, u8 *dic_buffer, int max_len);
static int CheckOverwrite(int orig_size, u8 *cmprs_buffer, int cmprs_buffer_size,
                          int *orig_safe, int *cmprs_safe);


/*---------------------------------------------------------------------------*
  Name:         Compress

  Description:  Buffer の逆順の圧縮を行なう．
                ただし圧縮データと展開データがメモリ空間を共有できるように
                調整する

                buffer       : 被圧縮データ
                buffer_size  : 被圧縮データサイズ

  Return:       >=0: 圧縮後のサイズ
                < 0: 失敗
 *---------------------------------------------------------------------------*/
int Compress(u8 *buffer_original, int buffer_original_size)
{
    u8     *buffer;
    int     buffer_size;
    int     buffer_start;
    u8     *temp_buffer_original;
    u8     *temp_buffer;
    int     temp_buffer_size;
    int     temp_buffer_start;
    int     compressed_size;
    int     aligned_size;
    int     total_size;
    int     reduced;
    int     i;
    CompFooter *footer;

    // 前準備
    if (NULL == (temp_buffer_original = (u8 *)malloc(buffer_original_size)))
    {
        ErrorPrintf("Cannot allocate memory size=%d\n", buffer_original_size);
        return COMPRESS_FATAL_ERROR;
    }

    if ((u32)buffer_original % 4 != 0)
    {
        ErrorPrintf("Top of buffer is not aligned by 4.\n");
        return COMPRESS_FATAL_ERROR;
    }

    buffer = buffer_original;
    buffer_size = buffer_original_size;
    temp_buffer = temp_buffer_original;
    temp_buffer_size = buffer_original_size;

    // 圧縮を行なう
    reduced = LZCompressRV(buffer, buffer_size, temp_buffer, temp_buffer_size);
    if (reduced < 0)
    {
        DebugPrintf("Compressed buffer size exceeds original data size.\n");
        free(temp_buffer_original);
        return COMPRESS_LARGER_ORIGINAL;
    }

    temp_buffer_size -= reduced;
    temp_buffer += reduced;

    DebugPrintf("1: source size = %d  compressed = %d\n", buffer_size, temp_buffer_size);

    // 展開不能な上書きが発生するか確認
    if (!CheckOverwrite
        (buffer_size, temp_buffer, temp_buffer_size, &buffer_start, &temp_buffer_start))
    {
        // 上書きが発生するなら圧縮範囲を変更する
        buffer += buffer_start;
        buffer_size -= buffer_start;
        temp_buffer += temp_buffer_start;
        temp_buffer_size -= temp_buffer_start;

        DebugPrintf("  !! Shrink back Compressed region to avoid overwriting.\n"
                    "  !! Expand non-compressed region = +%d\n"
                    "2: source size = %d  compressed = %d\n",
                    buffer_start, buffer_size, temp_buffer_size);
    }

    //  PADDING とパラメータ領域を加えても超えないかどうか判定
    compressed_size = buffer_start + temp_buffer_size;  // header+body
    aligned_size = ROUNDUP4(compressed_size);   //       +padding
    total_size = aligned_size + sizeof(CompFooter);     //       +footer

    if (buffer_original_size <= total_size)
    {
        DebugPrintf("Compressed buffer size exceeds or equals original data size.\n");
        free(temp_buffer_original);
        return COMPRESS_LARGER_ORIGINAL;
    }

    // データをテンポラリバッファから元データへ上書きする
    CopyBuffer(temp_buffer, buffer, temp_buffer_size);
    free(temp_buffer_original);

    // サイズが 4 の倍数になるように PADDING
    //   LZ の実装上圧縮領域の最初のバイト値は 0xff にならない(最初は圧縮
    //   フラグであり、最初のデータは圧縮なしで格納されるから)ので 0xff で
    //   埋める
    for (i = compressed_size; i < aligned_size; i++)
    {
        buffer_original[i] = 0xff;
    }

    // サイズ設定
    //    compressBottom は sizeof(PAD)+sizeof(footer) なので 1バイトで十分
    footer = (CompFooter *) (buffer_original + aligned_size);
    footer->bufferTop = total_size - buffer_start;      // 正の値
    footer->compressBottom = total_size - compressed_size;      // 正の値
    footer->originalBottom = buffer_original_size - total_size; // 正の値

    return total_size;
}


/*---------------------------------------------------------------------------*
  Name:         LZCompressRV

  Description:  LZ 圧縮を行なう．ただしデータの後方から圧縮開始する
                圧縮結果も後ろ詰めになる

  Returns:      圧縮データの先頭 index
                圧縮データは dst_buffer+index から dst_buffer+dst_size-1 まで
                -1: 圧縮失敗(圧縮した結果の方が大きい場合)
 *---------------------------------------------------------------------------*/
static int LZCompressRV(u8 *src_buffer, int src_size, u8 *dst_buffer, int dst_size)
{
    int     src_index = src_size;
    int     dst_index = dst_size;
    int     compflag;
    int     compflag_index;
    int     i;

    while (src_index > 0)
    {
        if (dst_index < 1)
            return -1;                 // Buffer Overflow

        // 8bit の圧縮フラグの挿入位置を予約
        compflag = 0x00;
        compflag_index = --dst_index;

        // フラグ系列が8ビットデータとして格納されるため、8回ループ
        for (i = 0; i < 8; i++)
        {
            compflag <<= 1;

            if (src_index > 0)         // src が残っているか判定
            {
                u8     *dic_buffer;
                int     dic_size;
                u8     *ref_buffer;
                int     ref_size;
                int     index;
                int     len;

                dic_buffer = src_buffer + src_index;
                dic_size = src_size - src_index;
                ref_size = MIN(src_index, LZ_MAX_COPY);
                ref_buffer = dic_buffer - ref_size;

                len = FindMatched(ref_buffer, ref_size,
                                  dic_buffer, MIN(dic_size, LZ_MAX_DIC_LENGTH), &index);

                if (len >= LZ_MIN_COPY)
                {
                    u16     half;

                    // Offset/Len の記録が可能かどうか確認
                    if (dst_index < 2)
                        return -1;     // Buffer Overflow

                    // src index 進める
                    src_index -= len;

                    // len >= LZ_MIN_COPY なのでその分減算し値域を節約する
                    index -= (LZ_MIN_COPY - 1);
                    len -= (LZ_MIN_COPY - 0);

                    // 16bit データとしてたたむ
                    half = (u16)((index & (LZ_MAX_INDEX - 1)) | (len << LZ_BIT_INDEX));
                    dst_buffer[--dst_index] = (half >> 8) & 0xff;
                    dst_buffer[--dst_index] = (half >> 0) & 0xff;

                    // flag セット
                    compflag |= 0x01;
                }
                else
                {
                    // 値そのままを記録する & src index 進める
                    if (dst_index < 1)
                        return -1;     // Buffer Overflow
                    dst_buffer[--dst_index] = src_buffer[--src_index];
                }
            }
        }
        // 圧縮フラグの保存
        dst_buffer[compflag_index] = compflag;
    }
    return dst_index;
}


/*---------------------------------------------------------------------------*
  Name:         FindMatched

  Description:  一致するパターンの検索を行なう．ただしデータの後方から前方への
                検索

                src_buffer[0...src_size-1] のパターンを src_buffer の後方から
                dic_buffer[0...dic_size-1] のパターンと最大一致する部分を
                検索する．

  Returns:      一致したサイズ
 *index 一致した位置
 *---------------------------------------------------------------------------*/
static int FindMatched(u8 *src_buffer, int src_size, u8 *dic_buffer, int dic_size, int *index)
{
    u8     *src_bottom = src_buffer + src_size - 1;
    u8      char_src_bottom = *src_bottom;
    int     n, len, max_len;

    // 返値初期化
    max_len = 0;

    for (n = 0; n < dic_size; n++)
    {
        // 高速化のためのキャッシュ
        if (char_src_bottom == dic_buffer[n])
        {
            len = HowManyMatched(src_bottom, dic_buffer + n, MIN(n + 1, src_size));
            if (max_len < len)
            {
                max_len = len;
                *index = n;
            }
        }
    }

    // 最小サイズ以上なら成功
    return max_len;
}


/*---------------------------------------------------------------------------*
  Name:         HowManyMatched

  Description:  2つのパターンが逆順にどこまで一致しているかを調査する

                src_buffer, dic_buffer 比較パターンのアドレス
                                       (このアドレスから逆方向へ検索する)
                max_len                最大調査する長さ

  Returns:      一致した長さ
 *---------------------------------------------------------------------------*/
static int HowManyMatched(u8 *src_buffer, u8 *dic_buffer, int max_len)
{
    int     i;

    // パターン一致検索(逆順)
    for (i = 0; i < max_len; i++)
    {
        if (*src_buffer != *dic_buffer)
        {
            break;
        }
        src_buffer--;
        dic_buffer--;
    }
    return i;
}


/*---------------------------------------------------------------------------*
  Name:         CheckOverwrite

  Description:  LZ 展開で展開先と展開元を同じアドレスに置いた場合に、どこまで
                正常に展開が可能かをチェックする

  Returns:      最後まで展開可能なら TRUE 途中までなら FALSE
 *---------------------------------------------------------------------------*/
static int CheckOverwrite(int orig_size, u8 *cmprs_buffer, int cmprs_buffer_size,
                          int *orig_safe, int *cmprs_safe)
{
    int     src = cmprs_buffer_size;
    int     dst = orig_size;
    int     flag;
    int     i;

//#define DETAIL
    while (dst > 0)
    {
        flag = cmprs_buffer[--src];    // 圧縮非圧縮フラグ 8 ループ分

#ifdef	DETAIL
        DebugPrintf("%08x %08x FLG=0x%02x\n", src, dst, flag);
#endif
        for (i = 0; i < 8; i++)
        {
            if (dst > 0)
            {
                if (flag & 0x80)       // 圧縮データか？
                {
                    u16     half;
                    int     len;

                    // 展開長を計算
                    src -= 2;
                    half = (u16)(cmprs_buffer[src] | (cmprs_buffer[src + 1] << 8));
                    len = ((half >> LZ_BIT_INDEX) & (LZ_MAX_LENGTH - 1)) + LZ_MIN_COPY;
#ifdef	DETAIL
                    DebugPrintf("%08x %08x-%08x LEN=%d\n", src, dst - 1, dst - len, len);
#endif
                    // ソースデータを上書きしてしまうかチェック
                    dst -= len;

                    if (dst < 0)
                    {
                        ErrorPrintf("System error in CheckOverwrite???\n");
                        exit(-1);      // Panic!!
                    }

                    if (dst < src)
                    {
                        // 上書きしてしまうなら圧縮は現在のところまでで止める
                        *orig_safe = dst;
                        *cmprs_safe = src;
                        return FALSE;
                    }
                }
                else
                {
                    // 非圧縮データならそのままコピーなので
                    // 破壊を伴なう上書きは起こらない
                    src--;
                    dst--;
#ifdef	DETAIL
                    DebugPrintf("%08x %08x CHR=0x%02x\n", src, dst, cmprs_buffer[src]);
#endif
                }
                flag <<= 1;
            }
        }
    }
    *orig_safe = 0;
    *cmprs_safe = 0;
    return TRUE;
}
