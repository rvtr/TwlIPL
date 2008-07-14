/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInfo
  File:     drawFunc.h

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

#ifndef	__DRAW_FUNC__
#define	__DRAW_FUNC__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif




#define STRING_LINES 10

// 各メニューサイズ

#define NUM_LINES 8		// 一ページあたりの項目数

#define ROOTMENU_SIZE 5
#define OWNERMENU_SIZE 6
#define PARENTALMENU_SIZE 6
#define OTHERMENU_SIZE 7
#define SCFGMENU_SIZE 1
#define FUSEMENU_SIZE 1


// メニューID
#define MENU_ROOT 10
#define MENU_OWNER 0
#define MENU_PARENTAL 1
#define MENU_OTHER 2
#define MENU_SCFG 3
#define MENU_FUSE 4

// 行番号
#define OWNER_LANGUAGE 0
#define OWNER_COLOR 1
#define OWNER_BIRTHDAY 2
#define OWNER_COUNTRY 3
#define OWNER_NICKNAME 4
#define OWNER_COMMENT 5

#define PARENTAL_FLAG 0
#define PARENTAL_ORGANIZATION 1
#define PARENTAL_AGE 2
#define PARENTAL_PASSWORD 3
#define PARENTAL_QUESTION_ID 4
#define PARENTAL_ANSWER 5


#define OTHER_WIRELESS 0
#define OTHER_FORCE_DISABLE 1
#define OTHER_AGREE_EULA 2
#define OTHER_EULA_VERSION 3
#define OTHER_REGION 4
#define OTHER_UNIQUE_ID 5
#define OTHER_SERIAL 6


/* global variables ----------------- */

#define MAXPAGE 10

// 各項目のページごとのオフセット値
extern const int s_pageOffset[][MAXPAGE];
	

/* function prototypes ----------------- */

void drawHeader( u8 menu, u8 page, u8 line );
void drawMenu( u8 menu, u8 page, u8 line );

#ifdef __cplusplus
}
#endif

#endif
