/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::             $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include	<twl/os.h>

extern void		Dummy_EXTWRAM(void);

void
Dummy_EXTWRAM(void)
{
	OS_Printf("Printed from extra wram area. [%p]\n", Dummy_EXTWRAM);
}

