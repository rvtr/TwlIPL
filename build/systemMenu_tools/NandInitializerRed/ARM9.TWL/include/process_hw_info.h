/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_hw_info.h

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

#ifndef PROCESS_HW_INFO_H_
#define PROCESS_HW_INFO_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

//typedef void*  (*TpProcess)(void);

/*---------------------------------------------------------------------------*
    �֐��錾
 *---------------------------------------------------------------------------*/

extern BOOL GetNandInitializerSetting(u8* region, u8* wireless, u8* logodemoskip);
void* HWInfoProcess0(void);
void* HWInfoProcess1(void);
void* HWInfoProcess2(void);
void* HWInfoProcess3(void);
void* HWInfoProcess4(void);

/*---------------------------------------------------------------------------*
  Name:         HW���S�̂̃��C�g

  Description:  

  Arguments:    region : 
				wirelessForceOff : ��������OFF�Ȃ�TRUE���w��

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL WriteHWInfoFile( u8 region, BOOL wirelessForceOff, BOOL logoDemoSkipForce );

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* PROCESS_HW_INFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
