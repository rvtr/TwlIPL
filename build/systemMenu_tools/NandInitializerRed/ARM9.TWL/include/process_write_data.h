/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_write_data.h

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

#ifndef PROCESS_WRITE_DATA_H_
#define PROCESS_WRITE_DATA_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define FONT_DATA_FILE_PATH_IN_NAND   "nand:sys/TWLFontTable.dat"
#define WRAP_DATA_FILE_PATH_IN_NAND   "nand:/shared2/launcher/wrap.bin"
#define DEVKP_DATA_FILE_PATH_IN_NAND  "nand:/sys/dev.kp"
#define CERT_DATA_FILE_PATH_IN_ROM    "rom:/data/cert.sys"
#define CERT_DATA_FILE_PATH_IN_NAND   "nand:/sys/cert.sys"

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

void* WriteDataProcessPre0(void);
void* WriteDataProcessAfter0(void);
void* WriteDataProcess0(void);
void* WriteDataProcess1(void);
void* WriteDataProcess2(void);
void* WriteDataProcess3(void);
void* WriteDataProcess4(void);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* PROCESS_WRITE_DATA_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
