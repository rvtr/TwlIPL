/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SDLOG - include
  File:     sd_log_wrapper.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef TWL_SD_LOG_WRAPPER_H_
#define TWL_SD_LOG_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl.h>
#include "log.h"
#include "sd_log.h"

BOOL SDLOG_InitEx(const char* path, const char* filename, char* buffer, u16 bufferSize, u16 partitionSize, SDLOGWriteType writeType);

void SDLOG_FinishEx(void);

inline
void SDLOG_PrintServer(void)
{
    OS_PrintServer();
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SD_LOG_WRAPPER_H_ */
#endif
