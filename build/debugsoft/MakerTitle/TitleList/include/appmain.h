/*---------------------------------------------------------------------------*
  Project:  TwlSDK_IPL - debugsoft - MakerTitle - TitleList
  File:     appmain.h

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
 
#ifndef	APPMAIN_H_
#define	APPMAIN_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/
#include	<nitro/types.h>

void    DrawScreen(void);
void    DoWritingTest(void);
void    DoReadingTest(void);
void    DeleteSaveDatas(void);
/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* SCREEN_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
