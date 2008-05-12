/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_fade.h

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

#ifndef PROCESS_FADE_H_
#define PROCESS_FADE_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef void*  (*Process)(void);

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/

#define FADE_IN_RETURN(P)  SetNextProcess(P);return fadeInProcess;
#define FADE_OUT_RETURN(P) SetNextProcess(P);return fadeOutProcess;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

void* fadeInProcess(void);
void* fadeOutProcess(void);
void SetNextProcess(Process process);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* PROCESS_FADE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
