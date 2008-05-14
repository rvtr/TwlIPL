/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_font.h

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

#ifndef PROCESS_FONT_H_
#define PROCESS_FONT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    íËêîíËã`
 *---------------------------------------------------------------------------*/
#define FONT_DATA_FILE_PATH_IN_ROM    "rom:/data/TWLFontTable.dat"
#define FONT_DATA_FILE_PATH_IN_NAND   "nand:sys/TWLFontTable.dat"

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void* fontProcess0(void);
void* fontProcess1(void);
void* fontProcess2(void);
void* fontProcess3(void);
void* fontProcess4(void);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* PROCESS_FONT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
