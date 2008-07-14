/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInfo
  File:     viewSystemInfo.c

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
 

#include <wchar.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include "misc.h"
#include "viewSystemInfo.h"
#include "strResource.h"

#define ID_BUFSIZE 2
#define AGE_BUFSIZE 4
#define BIRTHDAY_BUFSIZE 6

/* function prototype  ----------------------------- */
void getAllInfo( void );
void initInfo( void );
void infoAlloc( char* p, int size, const char *name);
void control( u8 *menu, u8 *page, u8 *line );
void printAllInfo ( void );


/* global variables ----------------------------- */
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

char** gAllInfo[ROOTMENU_KSIZE]; // それぞれのメニューごとに項目の中身を"文字列で"持つ配列
u16 gUserName[OS_OWNERINFO_NICKNAME_MAX + 1];
u16 gUserComment[OS_OWNERINFO_COMMENT_MAX + 1];
u16 gSecretAnswer[OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1];


static OSOwnerInfoEx 		gOwnerInfo;						// オーナー情報
static BOOL 				gbWirelessAvailable;			// 無線が有効かどうか
static BOOL 				gbBatteryExtensionAvailable;	// バッテリーエクステンションモードが有効か
static BOOL 				gbAgreeEula;					// Eula同意済みか
static u8 					gEulaVersion;					// 同意Eulaのバージョン
static const OSTWLParentalControl	*gpPC;		// ペアレンタルコントロール情報
static u8 					gUniqueID[OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN];	// ユニークId
static BOOL 				gbForceDisableWireless;			// 無線の強制無効化フラグ
static OSTWLRegion			gRegion; 						// リージョン番号
static u8 					gSerialNo[OS_TWL_HWINFO_SERIALNO_LEN_MAX]; // シリアル番号


/* static data ---------------------------------- */

// メニュー項目数
static const u8 s_numMenu[11]; //暫定

// 文字列リソース
/*
static const u16 *s_strRootMenu[ROOTMENU_SIZE];
static const char *s_strMenuName[ROOTMENU_SIZE];
static const u16 *s_strOwnerMenu[OWNERMENU_SIZE];
static const u16 *s_strParentalMenu[PARENTALMENU_SIZE];
static char *s_strCountry[ 256 ];
static char *s_strRegion[ OS_TWL_REGION_MAX ];
static char *s_strUserColor[ OS_FAVORITE_COLOR_MAX ];
static char *s_strLanguage[ OS_LANGUAGE_CODE_MAX ];
static char *s_strBool[2];
static char *s_strRatingOrg[OS_TWL_PCTL_OGN_MAX];
*/

/* function definition -------------------------- */

void displayInfoMain( void )
{
	static u8 menu = MENU_ROOT;
	static u8 page = 0, line = 0; // 現在選択しているページナンバと行

	static BOOL firstCall = TRUE;	// 最初の呼び出しか否か
	
	// 初回の呼び出し時は全ての情報を取得しなおす
	if( firstCall )
	{
		firstCall = FALSE;
		initInfo();
		getAllInfo();
	}
	
	// パッド情報で選んでる場所とか情報とか更新
	control( &menu, &page, &line );
		
	
	// 変更可能な項目でＡボタン押したら変更モードに移るとか
	// Ｂボタンでジャンルに戻るとかそんなんやる
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
    
	drawHeader(menu, page, line);
	drawMenu(menu, page, line); // 情報一覧を描画する
	GetAndDrawRTCData( &g_rtcDraw, FALSE ); // RTC更新

}

void initInfo( void )
{
	OS_TPrintf("buffer initialize\n");
	
	// 固定文字列で対応できない項目に文字列領域を割り当てる
	gAllInfo[MENU_OWNER][OWNER_BIRTHDAY] = (char*) Alloc (sizeof(char) * BIRTHDAY_BUFSIZE);
	SDK_ASSERT( gAllInfo[MENU_OWNER][OWNER_BIRTHDAY] );

	gAllInfo[MENU_PARENTAL][PARENTAL_AGE] = (char*) Alloc (sizeof(char) * AGE_BUFSIZE);
	SDK_ASSERT( gAllInfo[MENU_PARENTAL][PARENTAL_AGE] );
	
	gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD] = (char*) Alloc (sizeof(char) * OS_TWL_PCTL_PASSWORD_LENGTH+1);
	SDK_ASSERT( gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD] );

	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION] = (char*) Alloc (sizeof(char) * AGE_BUFSIZE);
	SDK_ASSERT( gAllInfo[MENU_OTHER][OTHER_EULA_VERSION] );

	gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID] = (char*) Alloc (sizeof(char) * OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3);
	SDK_ASSERT( gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID] );

	gAllInfo[MENU_OTHER][OTHER_SERIAL] = (char*) Alloc (sizeof(char) * OS_TWL_HWINFO_SERIALNO_LEN_MAX);
	SDK_ASSERT( gAllInfo[MENU_OTHER][OTHER_SERIAL] );
	
	OS_TPrintf( "information alloc succeeded\n" );
}
	

void getAllInfo( void )
// 
{
	BOOL SCFGAccessable = FALSE;
	BOOL fuseRomAccessable = FALSE;
	u8 uniqIDBuf[OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN];
	
	
	OS_TPrintf("reflesh Information\n");
		
	OS_TPrintf( "...Owner Information\n");
	// 本体設定まわり
	OS_GetOwnerInfoEx( &gOwnerInfo );
	
	// 全体情報につめていく
	// 泥臭すぎるやり方なのであとで考える
	gAllInfo[MENU_OWNER][OWNER_LANGUAGE] = s_strLanguage[ gOwnerInfo.language ];
	gAllInfo[MENU_OWNER][OWNER_COLOR] = s_strUserColor[ gOwnerInfo.favoriteColor ];
	snprintf( gAllInfo[MENU_OWNER][OWNER_BIRTHDAY], BIRTHDAY_BUFSIZE, "%02d/%02d",  gOwnerInfo.birthday.month, gOwnerInfo.birthday.day);
			OS_TPrintf( "language:%s\n", gAllInfo[MENU_OWNER][OWNER_LANGUAGE]);
	gAllInfo[MENU_OWNER][OWNER_COUNTRY] = s_strCountry[gOwnerInfo.country];
	wcsncpy( gUserName, gOwnerInfo.nickName, OS_OWNERINFO_NICKNAME_MAX + 1);
	wcsncpy( gUserComment, gOwnerInfo.comment, OS_OWNERINFO_COMMENT_MAX + 1 ); // !!! なぜかコメントが取得できない 要検証
//	wcsncpy( gUserComment, L"うべらべらー", OS_OWNERINFO_COMMENT_MAX + 1 );

	OS_TPrintf( "...Parental Control Information\n" );
	// ペアレンタルコントロールまわり
	gpPC = OS_GetParentalControlInfoPtr();
	
	gAllInfo[MENU_PARENTAL][PARENTAL_FLAG] = s_strBool[ gpPC->flags.isSetParentalControl ];
	gAllInfo[MENU_PARENTAL][PARENTAL_ORGANIZATION] = s_strRatingOrg[ gpPC->ogn ];
	snprintf( gAllInfo[MENU_PARENTAL][PARENTAL_AGE], AGE_BUFSIZE, "%d", gpPC->ratingAge );
	snprintf( gAllInfo[MENU_PARENTAL][PARENTAL_QUESTION_ID], ID_BUFSIZE, "%d", gpPC->secretQuestionID );
	wcsncpy( gUserComment, gpPC->secretAnswer, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1);
	snprintf( gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD], OS_TWL_PCTL_PASSWORD_LENGTH + 1, "%d", gpPC->password );
	
	OS_TPrintf( "...Other Machine Setting Information\n" );
	// そのほかの本体情報まわり	
	//gbBatteryExtensionAvailable = OS_IsAvailableBatteryExtension();
	gAllInfo[MENU_OTHER][OTHER_WIRELESS] = s_strEnable[ OS_IsAvailableWireless() ];
	gAllInfo[MENU_OTHER][OTHER_FORCE_DISABLE] = s_strBool[ OS_IsForceDisableWireless() ];
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA] = s_strBool[ OS_IsAgreeEULA() ];
	snprintf( gAllInfo[MENU_OTHER][OTHER_EULA_VERSION], AGE_BUFSIZE, "%d", OS_GetAgreedEULAVersion()) ;
	gAllInfo[MENU_OTHER][OTHER_REGION] = s_strRegion[ OS_GetRegion() ];
	
	{
		int i;
		char ascii[] = "0123456789abcdef";
		const u8 *unq = OS_GetMovableUniqueIDPtr();
		// 16進で1バイトずつ詰めていく
		// バッファが長さの3倍長なのは、データを"%02x-%02x-%02x..."に置換するため
		for(i=0; i < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3; i += 3, unq++ )
		{	
			gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i] = ascii[(*unq>>4) & 0x0f];
			gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i+1] = ascii[*unq & 0x0f];
			gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i+2] = 
				(i+2) == (OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 - 1) ? '\0' : '-' ;
			OS_TPrintf("uniqid: %d\n", *unq);
		}
	}
	{
		u8 serialBuf[OS_TWL_HWINFO_SERIALNO_LEN_MAX];
		OS_GetSerialNo( serialBuf );
		snprintf(  gAllInfo[MENU_OTHER][OTHER_SERIAL], OS_TWL_HWINFO_SERIALNO_LEN_MAX, "%s", serialBuf);
	}

	// ヘッダ情報をチェック
	
	// SecureアプリならfuseROM見える
	// Secureアプリか明示的に指定したアプリならSCFGレジスタ見える
	
	/*
	if( SCFGAccessable && SCFG_IsSecureRomAccessible())
	{
		
	}
	
	if( fuseRomAccessable )
	{
		
	}*/
	
	printAllInfo();
	
	OS_TPrintf("reflesh information finished\n");
}

void control( u8 *menu, u8 *page, u8 *line )
{
	u8 pagemax = ( s_numMenuK[*menu] / NUM_LINES ) + 1;	// 選択中メニューのページ数
	u8 linemax = ( s_numMenuK[*menu] - (*page * NUM_LINES)) > NUM_LINES ?
					NUM_LINES : s_numMenuK[*menu] - (*page * NUM_LINES); // 選択中ページの項目数
								
	if( pad.trg & PAD_KEY_UP )
	{
		// 項目名のないところはカーソル飛ばす
		do
		{
			*line = (*line + linemax - 1) % linemax;
		}
		while( !wcscmp( s_strMetaMenu[*menu][*line], L"") );

	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		do
		{
			*line = (*line+1) % linemax;
		}
		while( !wcscmp( s_strMetaMenu[*menu][*line], L"") );
	}

	if( pad.trg & PAD_KEY_LEFT )
	{
		// ページ送り
		*page = (*page + pagemax - 1) % pagemax;
		*line = 0;
	}
	else if( pad.trg & PAD_KEY_RIGHT )
	{
		*page = (*page+1) % pagemax;
		*line = 0;
	}
	
	if( pad.trg & PAD_BUTTON_A )
	{
		if(*menu == MENU_ROOT)
		{
			// 次のメニュー画面を開く
			*menu = *line;
			*line = 0;
			*page = 0;
		}
		else
		{
			// !!! 設定可能な項目だったら設定変更画面
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		// !!! とりあえず今はルートに戻る
		// 値設定画面の時はキャンセルするだけにする
		*menu = MENU_ROOT;
		*page = 0;
		*line = 0;
	}
}

void displayInfoInit( void )
{
	int loop1,loop2;
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"RomTypeTest");
	PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push A To Start Test.");
	PutStringUTF16( 4 * 8, 10 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push X To Start Test Quietly.");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );

	// 全体情報を持つ配列をセット
	// 全項目に文字列バッファを持たせるとメモリ書き込みが多くなるので
	// 可能なものはstatic文字列へのポインタで対応
	// 必要なものだけあとでmallocする
	for(loop1=0; loop1<ROOTMENU_KSIZE; loop1++)
	{
		int loop2;
		gAllInfo[loop1] = (char**)Alloc(sizeof(char*) * s_numMenuV[loop1]);
				
		if(gAllInfo[loop1] == NULL ){
			OS_TPrintf( "AllInfo malloc failed\n");
			return;
		}
		
		// とりあえず全部N/Aで埋めておく
		for(loop2=0; loop2<s_numMenuV[loop1]; loop2++)
		{
			gAllInfo[loop1][loop2] = s_strNA;
		}
	}

	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

void printAllInfo ( void )
{
	int loop1, loop2;
	
	for(loop1=0; loop1 < ROOTMENU_KSIZE; loop1++ )
	{
		for(loop2=0; loop2 < s_numMenuV[loop1]; loop2++ )
		{
			// utf16で描画するものだけ分ける
			if(loop1 == MENU_OWNER && loop2 == OWNER_NICKNAME)
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gUserName); 
			}
			else if(loop1 == MENU_OWNER && loop2 == OWNER_COMMENT)
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gUserComment); 
			}
			else if(loop1 == MENU_PARENTAL && loop2 == PARENTAL_ANSWER)
			{			
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gSecretAnswer); 
			}
			else
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gAllInfo[loop1][loop2]); 
			}
		}
	}
}

