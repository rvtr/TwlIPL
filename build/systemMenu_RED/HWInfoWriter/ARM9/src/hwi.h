/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Chat.h

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

#ifndef	__HWI_LIB__
#define	__HWI_LIB__

#ifdef __cplusplus
extern "C" {
#endif


#include <twl.h>

// define data----------------------------------------------------------

typedef enum 
{
	HWI_INIT_FAILURE,
	HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE,
	HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE,
	HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE
} 
HwiInitResult;

HwiInitResult HWI_Init( void *(*pAlloc)( u32 ), void (*pFree)( void * ) );
void HWI_ModifyLanguage( u8 region );
BOOL HWI_WriteHWNormalInfoFile( void );
BOOL HWI_WriteHWSecureInfoFile( u8 region, const u8 *pSerialNo );
BOOL HWI_WriteHWIDSignFile( void );
BOOL HWI_DeleteHWNormalInfoFile( void );
BOOL HWI_DeleteHWSecureInfoFile( void );
BOOL HWI_DeleteHWIDSignFile( void );

#ifdef __cplusplus
}
#endif

#endif  // __HWI_LIB__
