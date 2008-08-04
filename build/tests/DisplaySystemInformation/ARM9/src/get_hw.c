#include <twl.h>
//#include <wchar.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include <twl/lcfg.h>


#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"

void getNormalHWInfo( void );
void getSecureHWInfo( void );

void getHWInfo( void )
{
	getNormalHWInfo();
	getSecureHWInfo();
}


void getNormalHWInfo( void )
{
	int value;

	value = (int) OS_GetOwnerRtcOffset();
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_RTC_OFFSET].iValue = (int) value;
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_RTC_OFFSET].isNumData = TRUE;

	{
		int i;
		char ascii[] = "0123456789abcdef";
		const u8 *unq = OS_GetMovableUniqueIDPtr();
		// 16進で1バイトずつ詰めていく
		// バッファが長さの3倍長なのは、データを"%02x-%02x-%02x..."に置換するため
		for(i=0; i < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3; i += 3, unq++ )
		{	
			gAllInfo[MENU_NORMAL_HW][NORMAL_HW_UNIQUE_ID].str.sjis[i] = ascii[(*unq>>4) & 0x0f];
			gAllInfo[MENU_NORMAL_HW][NORMAL_HW_UNIQUE_ID].str.sjis[i+1] = ascii[*unq & 0x0f];
			gAllInfo[MENU_NORMAL_HW][NORMAL_HW_UNIQUE_ID].str.sjis[i+2] = 
				(i+2) == (OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 - 1) ? (char)'\0' : (char)'-' ;
			OS_TPrintf("uniqid: %d\n", *unq);
		}
	}

}

void getSecureHWInfo( void )
{
	int value;
	
	value = OS_IsForceDisableWireless();
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].iValue = value;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].str.sjis = s_strBool[ value ];
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].isAligned = FALSE;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].numLines = 2;
	
	value = OS_GetRegion();
	gAllInfo[MENU_SECURE_HW][SECURE_HW_REGION].iValue = value;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_REGION].str.sjis = s_strRegion[ value ];

	{
		u8 serialBuf[OS_TWL_HWINFO_SERIALNO_LEN_MAX];
		OS_GetSerialNo( serialBuf );
		snprintf(  gAllInfo[MENU_SECURE_HW][SECURE_HW_SERIAL].str.sjis , OS_TWL_HWINFO_SERIALNO_LEN_MAX, "%s", serialBuf);
	}
	
	{
		u64	buf;
		buf = OS_GetValidLanguageBitmap();
		OS_TPrintf("language bitmap : %lx\n", buf );
		snprintf( gAllInfo[MENU_SECURE_HW][SECURE_HW_LANGUAGE].str.sjis ,
				DISPINFO_BUFSIZE-1, "%08lx", OS_GetValidLanguageBitmap() );


		// fuseRomデータの読み出し
		// secureなアプリ以外はハード的に切り離されるのでゼロになる
		buf = SCFG_ReadFuseData();
		OS_TPrintf("fuse data : %016llx\n", buf);
		snprintf( gAllInfo[MENU_SECURE_HW][SECURE_HW_FUSE].str.sjis ,
				DISPINFO_BUFSIZE-1, "%016llx", SCFG_ReadFuseData() );
		gAllInfo[MENU_SECURE_HW][SECURE_HW_FUSE].numLines = 2;

	}
	


}
