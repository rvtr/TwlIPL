/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS - include
  File:     init.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef FIRM_OS_INIT_H_
#define FIRM_OS_INIT_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_InitNOR

  Description:  initialize sdk os for norfirm

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_InitNOR(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_OS_INIT_H_ */
#endif
