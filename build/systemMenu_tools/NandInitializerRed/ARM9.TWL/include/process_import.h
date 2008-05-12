/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_import.h

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

#ifndef PROCESS_IMPORT_H_
#define PROCESS_IMPORT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    å^íËã`
 *---------------------------------------------------------------------------*/

//typedef void*  (*TpProcess)(void);

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void* ImportProcess0(void);
void* ImportProcess1(void);
void* ImportProcess2(void);
void* ImportProcess3(void);
void* ImportProcess4(void);

void ProgressInit(void);
void ProgressDraw(f32 ratio);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* PROCESS_IMPORT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
