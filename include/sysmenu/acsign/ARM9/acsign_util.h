/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     acsign_util.h

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

#ifndef _ACSIGN_UTIL_H_
#define _ACSIGN_UTIL_H_

#include <mb_fileinfo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_BUFFER_LEN     20

extern int ACSignDecrpto(void *output_buffer, MBDownloadFileInfo *download_file_info_buf);
extern int ACSignDigest(void *input_buffer, MBDownloadFileInfo *download_file_info_buf);


#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_UTIL_H_
