#include <twl.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include <wchar.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"


void getOwnerInfo( void )
// オーナー情報周りを取得する
{
	OSOwnerInfoEx ownerInfo;
	
	OS_TPrintf( "...Owner Information\n");
	// 本体設定まわり
	OS_GetOwnerInfoEx( &ownerInfo );
	
	// 全体情報につめていく
	// 泥臭すぎるやり方なのであとで考える
	gAllInfo[MENU_OWNER][OWNER_LANGUAGE].str.sjis = s_strLanguage[ ownerInfo.language ];
	gAllInfo[MENU_OWNER][OWNER_LANGUAGE].iValue = ownerInfo.language;
	
	gAllInfo[MENU_OWNER][OWNER_COLOR].str.sjis = s_strUserColor[ ownerInfo.favoriteColor ];
	gAllInfo[MENU_OWNER][OWNER_COLOR].iValue = ownerInfo.favoriteColor;

	snprintf( gAllInfo[MENU_OWNER][OWNER_BIRTHDAY].str.sjis, DISPINFO_BUFSIZE-1, "%02d/%02d",  ownerInfo.birthday.month, ownerInfo.birthday.day);
	gAllInfo[MENU_OWNER][OWNER_BIRTHDAY].iValue = ownerInfo.birthday.month * 100 + ownerInfo.birthday.day;
	
	gAllInfo[MENU_OWNER][OWNER_COUNTRY].str.sjis = s_strCountry[ownerInfo.country];
	gAllInfo[MENU_OWNER][OWNER_COUNTRY].iValue = ownerInfo.country;
	
	wcsncpy( gAllInfo[MENU_OWNER][OWNER_NICKNAME].str.utf , ownerInfo.nickName, OS_OWNERINFO_NICKNAME_MAX + 1);
	gAllInfo[MENU_OWNER][OWNER_NICKNAME].isSjis = FALSE;	
	wcsncpy( gAllInfo[MENU_OWNER][OWNER_COMMENT].str.utf , ownerInfo.comment, OS_OWNERINFO_COMMENT_MAX + 1 );
	gAllInfo[MENU_OWNER][OWNER_COMMENT].isSjis = FALSE;
}
