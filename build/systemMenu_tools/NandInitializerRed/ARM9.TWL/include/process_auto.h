/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_auto.h

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

#ifndef AUTO_TOPMENU_H_
#define AUTO_TOPMENU_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	AUTO_PROCESS_MENU_FORMAT = 0,
#ifdef    USE_WRITE_VARIOUS_DATA
	AUTO_PROCESS_MENU_VARIOUS_DATA_1,
#endif // USE_WRITE_VARIOUS_DATA

#ifndef TWL_CAPTURE_VERSION
	AUTO_PROCESS_MENU_HARDWARE_INFO,
#endif // TWL_CAPTURE_VERSION
#ifdef    USE_WRITE_VARIOUS_DATA
	AUTO_PROCESS_MENU_VARIOUS_DATA_2,
#endif // USE_WRITE_VARIOUS_DATA
	AUTO_PROCESS_MENU_IMPORT_TAD,
	AUTO_PROCESS_MENU_IMPORT_NANDFIRM,
	AUTO_PROCESS_MENU_MCU,
#ifdef    MARIOCLUB_VERSION
	AUTO_PROCESS_MENU_MACHINE_INITIALIZE,
#endif // MARIOCLUB_VERSION
	AUTO_PROCESS_MENU_NUM
};

typedef enum _AutoProcessResult
{
	AUTO_PROCESS_RESULT_SUCCESS,
	AUTO_PROCESS_RESULT_FAILURE,
	AUTO_PROCESS_RESULT_SKIP
} 
AutoProcessResult;

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

extern BOOL gAutoFlag;
extern AutoProcessResult gAutoProcessResult[AUTO_PROCESS_MENU_NUM];

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

void* AutoProcess0(void);
void* AutoProcess1(void);
void* AutoProcess2(void);
void* AutoProcess3(void);
void* AutoProcess4(void);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* AUTO_TOPMENU_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
