/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - yasuda - MenuTest
  File:     dataver.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef DATAVER_H_
#define DATAVER_H_

#ifdef	__cplusplus
extern "C" {
#endif

void GetAppVersionString(char *str);
BOOL GetPublicDataInfoStrings(char *verstr, char *timestr, char *datestr);
BOOL GetPrivateDataInfoStrings(char *verstr, char *timestr, char *datestr);
BOOL SetPublicDataTimeStamp(void);
BOOL SetPrivateDataTimeStamp(void);

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif  /* DATAVER_H_ */
