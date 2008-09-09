#include <twl.h>
#include <twl/lcfg.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"

void getOtherInfo( void ) 
{
	// Ç±ÇÃï”Ç©ÇÁÉ}ÉNÉçÇ≈ê∂ê¨ÇµÇΩï™
	s64 rtcoffset;
	int value;
	LCFGTWLTPCalibData tpdata;
	
	value = OS_IsAgreeEULA();
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].str.sjis = s_strBool[value];
		
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].iValue = OS_GetAgreedEULAVersion();
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].isNumData = TRUE;
	
	value = OS_IsAvailableWireless();
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].str.sjis = s_strEnable[value];

	rtcoffset = OS_GetOwnerRtcOffset();
	snprintf( gAllInfo[MENU_OTHER][OTHER_RTC_OFFSET].str.sjis,
				DISPINFO_BUFSIZE-1, "%016llx", rtcoffset );
	gAllInfo[MENU_OTHER][OTHER_RTC_OFFSET].numLines = 2;
		
	LCFG_TSD_GetTPCalibration( &tpdata );
	gAllInfo[MENU_OTHER][OTHER_TP_RAWX1].iValue = tpdata.data.raw_x1;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWX1].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWX1].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_RAWX2].iValue = tpdata.data.raw_x2;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWX2].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWX2].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_DX1].iValue = tpdata.data.dx1;
	gAllInfo[MENU_OTHER][OTHER_TP_DX1].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_DX1].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_DY1].iValue = tpdata.data.dy1;
	gAllInfo[MENU_OTHER][OTHER_TP_DY1].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_DY1].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_RAWY1].iValue = tpdata.data.raw_y1;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWY1].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWY1].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_RAWY2].iValue = tpdata.data.raw_y2;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWY2].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_RAWY2].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_DX2].iValue = tpdata.data.dx2;
	gAllInfo[MENU_OTHER][OTHER_TP_DX2].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_DX2].fromLCFG = TRUE;
	
	gAllInfo[MENU_OTHER][OTHER_TP_DY2].iValue = tpdata.data.dy2;
	gAllInfo[MENU_OTHER][OTHER_TP_DY2].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_TP_DY2].fromLCFG = TRUE;

	snprintf( gAllInfo[MENU_OTHER][OTHER_TP_RSV].str.sjis,
			DISPINFO_BUFSIZE, "%016llx", MI_LoadLE64( tpdata.rsv ) );
	gAllInfo[MENU_OTHER][OTHER_TP_RSV].fromLCFG = TRUE;	
	

}
