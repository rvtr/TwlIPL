/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     unicode.h

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

#ifndef	__UNICODE_H_
#define	__UNICODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl.h>

// define data----------------------------------

// function's prototype declaration-------------
void ExSJIS_BEtoUTF16_LE(u8 *sjisp, u16 *unip, u16 length);
void ExUTF16_LEtoSJIS_BE(u8 *sjisp, u16 *unip, u16 length);
void CheckSJIS_BEtoUTF16_LE(void);


#ifdef __cplusplus
}
#endif

#endif		// __UNICODE_H_

