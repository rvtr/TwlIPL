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

#ifndef HW_INFO_H_
#define HW_INFO_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         HW���S�̂̃��C�g

  Description:  

  Arguments:    region : 
				wirelessForceOff : ��������OFF�Ȃ�TRUE���w��

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL WriteHWInfoFile( u8 region, BOOL wirelessForceOff );


/*---------------------------------------------------------------------------*
  Name:         HWInfo�t�@�C���̍폜

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DeleteHWInfoFile( void );

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* HW_INFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
