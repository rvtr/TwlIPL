/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenandfirm
  File:     format_nlist.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/
#ifndef     FORMAT_NLIST_H_
#define     FORMAT_NLIST_H_

#include    <tchar.h>
#include    "misc.h"
#include    "path.h"

#define     SPECFILE_MAGIC_ID   "#NANDSF"

#define CRC16_INIT_VALUE        0xffff

//---------------------------------------------------------------------------
//  Banner Spec File
//---------------------------------------------------------------------------

// Command List
typedef struct
{
    char   *string;
    BOOL    (*funcp) (char *, int num);

}
tCommandDesc;


// F Command
typedef struct
{
    u32     offsetStart;
    u32     offsetEnd;
    u32     padding;
    char    fullPathSrc[FILENAME_MAX];

}
tFileDesc;


#endif //       FORMAT_NLIST_H_
