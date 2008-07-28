#include <twl.h>
#include <twl/lcfg.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"

void getOtherInfo( void ) 
{
	// この辺からマクロで生成した分
	
	int value;
	
	value = OS_IsAgreeEULA();
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].str.sjis = s_strBool[value];
		
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].iValue = OS_GetAgreedEULAVersion();
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].isNumData = TRUE;
	
	value = OS_IsAvailableWireless();
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].str.sjis = s_strEnable[value];



	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].fromLCFG = TRUE;

	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].fromLCFG = TRUE;
	STD_StrCpy( gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].str.sjis, "0000" );

	if( gLCFGAccessible ) 
	{
			
		value = LCFG_TSD_IsFinishedInitialSetting();
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].str.sjis = s_strBool[ value ];
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].changable = TRUE;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].changeFunc.cBool = LCFG_TSD_SetFlagFinishedInitialSetting;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].argType = ARG_BOOL;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].kindNameList = s_strBool;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].numKindName = 2;
		
		value = LCFG_TSD_IsFinishedInitialSetting_Launcher();
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].str.sjis = s_strBool[ value ];
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].changable = TRUE;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].changeFunc.cBool = LCFG_TSD_SetFlagFinishedInitialSetting_Launcher;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].argType = ARG_BOOL;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].kindNameList = s_strBool;
		gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].numKindName = 2;
		
		value = LCFG_TSD_IsFinishedBrokenTWLSettings();
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].str.sjis = s_strBool[ value ];
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].changable = TRUE;
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].changeFunc.cBool = LCFG_TSD_SetFlagFinishedBrokenTWLSettings;
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].argType = ARG_BOOL;
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].kindNameList = s_strBool;
		gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].numKindName = 2;

		value = LCFG_TSD_GetInstalledSoftBoxCount();
		gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].isNumData = TRUE;
		
		
		value = LCFG_TSD_GetFreeSoftBoxCount();
		gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].isNumData = TRUE;
		
		value = LCFG_TSD_GetLastTimeBootSoftIndex();
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].isNumData = TRUE;
		
		value = LCFG_TSD_GetLastTimeBootSoftPlatform();
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].iValue = value;
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].isNumData = TRUE;
		

		{
			u64 buf = LCFG_TSD_GetLastTimeBootSoftTitleID();
			
				
			if( buf )
			// bufが0だったら前回起動したアプリが設定されていない
			{
				u32 titleID = MI_LoadBE32( &buf );
				
				OS_TPrintf("Lastboot ID: %x Title ID: %x\n", buf, titleID );

				MI_CpuCopy( &titleID, gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].str.sjis, 4 );
				gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].str.sjis[8] = '\0';
			}
		}
	}
	
	
}
