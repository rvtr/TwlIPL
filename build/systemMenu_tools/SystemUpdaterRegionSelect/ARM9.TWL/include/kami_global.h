/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_global.h

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

#ifndef KAMI_GROBAL_H_
#define KAMI_GROBAL_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

typedef struct _SystemUpdaterLog
{
	int magic_code;
	int sdk_version;
	int ipl_version;
	int reserve[5];
} SystemUpdaterLog;

typedef enum
{ 
	IS_TWL_DEBUGGER=0, IS_TWL_CAPTURE, TWL, UNKNOWN 
} RunningConsole;

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define SYSTEM_UPDATER_NAME         L"SystemUpdater ver.1.3"
#define SYSTEM_UPDATER_LOG_PATH     "nand:/sys/log/updater.log"
#define SYSTEM_UPDATER_MAGIC_CODE   44001111

/*---------------------------------------------------------------------------*
    グローバル変数宣言
 *---------------------------------------------------------------------------*/

extern const char *g_strIPLSvnRevision;
extern const char *g_strSDKSvnRevision;
extern s32 gLockId;
extern const u16* sRegionStringArray[OS_TWL_REGION_MAX];
extern OSTWLRegion gRegion;
extern BOOL gIsDeleteNMenuAndNandFiler;

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/
extern void ProcessSelectRegion(void);
extern void ProcessCancel(const char* string);
extern void ProcessCheckLog(void);
extern void ProcessNote(void);
extern void ProcessCheckConsole(void);
extern BOOL ProcessFormat(void);
extern BOOL ProcessHwinfo(void);
extern void ProcessLog(void);
extern void ProcessFinish(BOOL result);
extern BOOL ProcessImport( void *(*alloc)(unsigned long), void (*free)(void *) );
extern BOOL ProcessWriteFont(void);
extern BOOL ProcessWriteCert(void);
extern BOOL ProcessWriteDummy(void);
extern BOOL FadeOutTick(void);
extern BOOL FadeInTick(void);
extern BOOL FadeInMaster(void);
extern RunningConsole GetConsole(void);
extern BOOL ProcessNandfirm(void);
extern BOOL ProcessNamutFormat(void);
extern BOOL ProcessDeleteOtherResionSysmenu(void);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_GROBAL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
