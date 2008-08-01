/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Nmenu
  File:     WPrintf.h

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
#ifndef TLIB_SYSTEM_WPRINTF_H_
#define TLIB_SYSTEM_WPRINTF_H_



namespace util
{

int VSNWPrintf(wchar_t *dst, size_t len, const wchar_t *fmt, va_list vlist);


/*---------------------------------------------------------------------------*
  Name:         OS_SNPrintf

  Description:  equal to 'OS_VSNPrintf' except argument style.

  Arguments:    dst   : destination buffer.
                len   : destination buffer size.
                fmt   : format string.
 
  Returns:      length of the generated string. (except '\0')
                if(result < len),
                  put NUL in dst[result].
                else if(len > 0),
                  put NUL in dst[len - 1].
                else,
                  do nothing.
 *---------------------------------------------------------------------------*/
inline int 
SNWPrintf(wchar_t *dst, size_t len, const wchar_t *fmt, ...)
{
    int ret;
    va_list va;
    va_start(va, fmt);
    ret = VSNWPrintf(dst, len, fmt, va);
    va_end(va);
    return  ret;
}

/*---------------------------------------------------------------------------*
  Name:         OS_VSPrintf

  Description:  equal to 'OS_VSNPrintf' except buffer size argument.

  Arguments:    dst   : destination buffer.
                fmt   : format string.
                vlist : parameters.

  Returns:      length of the generated string.
 *---------------------------------------------------------------------------*/
inline int
VSWPrintf(wchar_t *dst, const wchar_t *fmt, va_list vlist)
{
    return  VSNWPrintf(dst, 0x7FFFFFFF, fmt, vlist);
}

/*---------------------------------------------------------------------------*
  Name:         OS_SPrintf

  Description:  equal to 'OS_VSPrintf' except argument style.

  Arguments:    dst   : destination buffer.
                fmt   : format string.

  Returns:      length of the generated string.
 *---------------------------------------------------------------------------*/
inline int
SWPrintf(wchar_t *dst, const wchar_t *fmt, ...)
{
    int ret;
    va_list va;
    va_start(va, fmt);
    ret = VSWPrintf(dst, fmt, va);
    va_end(va);
    return  ret;
}


}
// end of namespace tlib



#endif // TLIB_SYSTEM_WPRINTF_H_
