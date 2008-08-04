#include <twl.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"

void getOtherInfo( void ) 
{
	// Ç±ÇÃï”Ç©ÇÁÉ}ÉNÉçÇ≈ê∂ê¨ÇµÇΩï™
	
	int value;
	
	value = OS_IsAgreeEULA();
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].str.sjis = s_strBool[value];
		
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].iValue = OS_GetAgreedEULAVersion();
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].isNumData = TRUE;
	
	value = OS_IsAvailableWireless();
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].str.sjis = s_strEnable[value];



	
	
}
