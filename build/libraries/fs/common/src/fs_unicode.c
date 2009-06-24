/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_unicode.c

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
#include <firm.h>

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringSjisToUnicode

  Description:  ShiftJIS文字列をUnicode文字列に変換。
                扱うパス名が明らかにASCIIのみである場合など
                UnicodeとShiftJISの相互変換を簡略化できる場合は
                この関数をオーバーライドすることによって
                STDライブラリの標準処理がリンクされるのを防ぐことができる。

  Arguments:    dst               変換先バッファ.
                                  NULL を指定すると格納処理は無視される.
                dst_len           変換先バッファの最大文字数を格納して渡し,
                                  実際に格納された文字数を受け取るポインタ.
                                  NULL を与えた場合は無視される.
                src               変換元バッファ.
                src_len           変換すべき最大文字数を格納して渡し,
                                  実際に変換された文字数を受け取るポインタ.
                                  この指定よりも文字列終端の位置が優先される.
                                  負の値を格納して渡すか NULL を与えた場合は
                                  終端位置までの文字数を指定したとみなされる.
                callback          変換できない文字が現れた時に呼ばれるコールバック.
                                  NULLを指定した場合, 変換できない文字の位置で
                                  変換処理を終了する.

  Returns:      変換処理の結果.
 *---------------------------------------------------------------------------*/
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
{
#pragma unused(callback)
        STDResult   result = STD_RESULT_SUCCESS;
        int         i;
        int         max = 0x7FFFFFFF;
        if (src_len && (*src_len >= 0))
        {
            max = *src_len;
        }
        if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
        {
            max = *dst_len;
        }
        for (i = 0; i < max; ++i)
        {
            int     c = ((const u8 *)src)[i];
            if (c == 0)
            {
               break;
            }
            else if (c >= 0x80)
            {
               result = STD_RESULT_ERROR;
               break;
            }
            dst[i] = (u16)c;
        }
        if (src_len)
        {
            *src_len = i;
        }
        if (dst_len)
        {
            *dst_len = i;
        }
        return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringUnicodeToSjis

  Description:  Unicode文字列をShiftJIS文字列に変換。
                扱うパス名が明らかにASCIIのみである場合など
                UnicodeとShiftJISの相互変換を簡略化できる場合は
                この関数をオーバーライドすることによって
                STDライブラリの標準処理がリンクされるのを防ぐことができる。

  Arguments:    dst               変換先バッファ.
                                  NULL を指定すると格納処理は無視される.
                dst_len           変換先バッファの最大文字数を格納して渡し,
                                  実際に格納された文字数を受け取るポインタ.
                                  NULL を与えた場合は無視される.
                src               変換元バッファ.
                src_len           変換すべき最大文字数を格納して渡し,
                                  実際に変換された文字数を受け取るポインタ.
                                  この指定よりも文字列終端の位置が優先される.
                                  負の値を格納して渡すか NULL を与えた場合は
                                  終端位置までの文字数を指定したとみなされる.
                callback          変換できない文字が現れた時に呼ばれるコールバック.
                                  NULLを指定した場合, 変換できない文字の位置で
                                  変換処理を終了する.

  Returns:      変換処理の結果.
 *---------------------------------------------------------------------------*/
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
{
#pragma unused(callback)
    STDResult   result = STD_RESULT_SUCCESS;
    int         i;
    int         max = 0x7FFFFFFF;
    if (src_len && (*src_len >= 0))
    {
        max = *src_len;
    }
    if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
    {
        max = *dst_len;
    }
    for (i = 0; i < max; ++i)
    {
        int     c = ((const u16 *)src)[i];
        if (c == 0)
        {
           break;
        }
        else if (c >= 0x80)
        {
           result = STD_RESULT_ERROR;
           break;
        }
        dst[i] = (char)c;
    }
    if (src_len)
    {
        *src_len = i;
    }
    if (dst_len)
    {
        *dst_len = i;
    }
    return result;
}
