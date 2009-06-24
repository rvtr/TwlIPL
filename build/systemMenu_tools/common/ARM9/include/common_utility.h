/*---------------------------------------------------------------------------*
  Project:  TwlIPL - Tools
  File:     common_utility.h

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
#ifndef COMMON_UTILITY_H_
#define COMMON_UTILITY_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         MakeFullPathForSD

  Description:  SDアーカイブ用のフルパスを作成します。

  Arguments:    

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MakeFullPathForSD(char* file_name, char* full_path)
{
	STD_CopyString( full_path, "sdmc:/" );
	STD_ConcatenateString( full_path, file_name );
}

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* COMMON_UTILITY_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
