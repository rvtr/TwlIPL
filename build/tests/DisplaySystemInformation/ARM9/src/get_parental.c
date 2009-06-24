#include <twl.h>
#include <wchar.h>
#include <twl/lcfg.h>
#include <stdlib.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"

void getParentalInfo( void )
// ペアレンタルコントロール情報を取得する
{
	// ペアレンタルコントロールまわり
	const LCFGTWLParentalControl *pPC = (const LCFGTWLParentalControl *) OS_GetParentalControlInfoPtr();

	OS_TPrintf( "...Parental Control Information\n" );

	gAllInfo[MENU_PARENTAL][PARENTAL_FLAG].str.sjis = s_strBool[ pPC->flags.isSetParentalControl ];	
	gAllInfo[MENU_PARENTAL][PARENTAL_FLAG].iValue = (int) pPC->flags.isSetParentalControl;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_PICTOCHAT].str.sjis = s_strBool[ pPC->flags.pictoChat ];
	gAllInfo[MENU_PARENTAL][PARENTAL_PICTOCHAT].iValue = (int) pPC->flags.pictoChat;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_DOWNLOAD].str.sjis = s_strBool[ pPC->flags.dsDownload ];
	gAllInfo[MENU_PARENTAL][PARENTAL_DOWNLOAD].iValue = (int) pPC->flags.dsDownload;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_BROWSER].str.sjis = s_strBool[ pPC->flags.browser ];
	gAllInfo[MENU_PARENTAL][PARENTAL_BROWSER].iValue = (int) pPC->flags.browser;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_WIIPOINT].str.sjis = s_strBool[ pPC->flags.wiiPoint ];
	gAllInfo[MENU_PARENTAL][PARENTAL_WIIPOINT].iValue = (int) pPC->flags.wiiPoint;

	gAllInfo[MENU_PARENTAL][PARENTAL_PHOTO_EXCHANGE].str.sjis = s_strBool[ pPC->flags.photoExchange ];
	gAllInfo[MENU_PARENTAL][PARENTAL_PHOTO_EXCHANGE].iValue = (int) pPC->flags.photoExchange;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_UGC].str.sjis = s_strBool[ pPC->flags.ugc ];
	gAllInfo[MENU_PARENTAL][PARENTAL_UGC].iValue = (int) pPC->flags.ugc;	
	
	gAllInfo[MENU_PARENTAL][PARENTAL_ORGANIZATION].str.sjis = s_strRatingOrg[ pPC->ogn ];
	gAllInfo[MENU_PARENTAL][PARENTAL_ORGANIZATION].iValue = (int) pPC->ogn;

	gAllInfo[MENU_PARENTAL][PARENTAL_AGE].iValue = pPC->ratingAge;
	gAllInfo[MENU_PARENTAL][PARENTAL_AGE].isNumData = TRUE;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_QUESTION_ID].iValue = pPC->secretQuestionID;
	gAllInfo[MENU_PARENTAL][PARENTAL_QUESTION_ID].isNumData = TRUE;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD].iValue = atoi( pPC->password ) ;
	gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD].isNumData = TRUE;
	
	wcsncpy( gAllInfo[MENU_PARENTAL][PARENTAL_ANSWER].str.utf , pPC->secretAnswer, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1);
	gAllInfo[MENU_PARENTAL][PARENTAL_ANSWER].isSjis = FALSE;
}
