/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_eticket.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef PROCESS_ETICKET_H_
#define PROCESS_ETICKET_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    íËêîíËã`
 *---------------------------------------------------------------------------*/
#define E_TICKET_FILE_PATH_IN_ROM    "data/cert.sys"
#define E_TICKET_FILE_PATH_IN_NAND   "F:sys/cert.sys"

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void* eTicketProcess0(void);
void* eTicketProcess1(void);
void* eTicketProcess2(void);
void* eTicketProcess3(void);
void* eTicketProcess4(void);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* PROCESS_ETICKET_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
