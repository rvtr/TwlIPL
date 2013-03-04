/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util_recoveryFile.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
 
#ifndef __UTIL_RECOVERY_FILE__
#define __UTIL_RECOVERY_FILE__

#include <twl.h>

typedef enum UTL_RecoveryStatus{
	UTL_RCV_OK = 0,
	UTL_RCV_SIZE_CHANGED,
	UTL_RCV_FILE_CREATED,
	UTL_RCV_FAILED
} UTL_RecoveryStatus;

/*-- function prototype -------------------------*/
extern UTL_RecoveryStatus UTL_RecoveryFile( const char* path, const u32 filesize );

#endif