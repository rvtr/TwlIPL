/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fs
  File:     fs_firm.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef FIRM_FS_FS_FIRM_H_
#define FIRM_FS_FS_FIRM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_OpenSrl

  Description:  open srl file named in HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    None

  Returns:      file discriptor
 *---------------------------------------------------------------------------*/
int FS_OpenSrl( void );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FS_FS_FIRM_H_ */
#endif
