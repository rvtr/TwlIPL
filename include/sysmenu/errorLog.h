/*---------------------------------------------------------------------------*
  Project:  TwlIPL - ErrorLog
  File:     errorLog.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/
 
 #ifndef __SYSM_ERRORLOG__
 #define __SYSM_ERRORLOG__
 
 
#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

/*-- function prototype -------------------------*/
extern BOOL EL_WriteErrorLog( u64 errorCode );



#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif

 
 
#endif
