/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SystemUpdater
  File:     kami_copy_file.h

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

#ifndef KAMI_COPY_FILE_H_
#define KAMI_COPY_FILE_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>

BOOL kamiCopyFile(char* srcPath, char* dstPath);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_COPY_FILE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
