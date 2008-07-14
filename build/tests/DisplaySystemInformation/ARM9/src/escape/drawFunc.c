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
 

#include <twl.h>
#include <wchar.h>
#include <string.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include "drawFunc.h"
#include "viewSystemInfo.h"
#include "misc.h"
#include "strResource.h"



// 描画関連
#define HEADER_UP 0
#define HEADER_LEFT 15
#define FOOTER_UP 160
#define FOOTER_LEFT 10
#define ALLOW_LEFT 10

#define KIND_UP 40		// 項目名の上座標
#define KIND_LEFT 20	// 項目名の左座標
#define VALUE_UP 40		// 項目値の上座標
#define VALUE_LEFT 140	// 項目値の左座標
#define LINE_OFFSET 15	// 1行ごとのオフセット
#define ROW_OFFSET 15 // 長い項目が現れたときの段組用

#define UNIQUE_BUF 12

const int s_pageOffset[][MAXPAGE] = {
	{ 0, OWNERMENU_KSIZE },			// owner
	{ 0, PARENTALMENU_KSIZE },			// parental
	{ 0, 5, OTHERMENU_KSIZE },		// other
	{ 0, SCFGMENU_KSIZE },			// scfg
	{ 0, FUSEMENU_KSIZE }			// fuse
};


void drawKindName( u8 menu, u8 page, u8 line );
void drawRootMenu( u8 page, u8 line );
void drawOwnerMenu( u8 page, u8 line );
void drawParentalMenu( u8 page, u8 line );
void drawOtherMenu( u8 page, u8 line );
void drawSCFGMenu( u8 page, u8 line );
void drawFuseMenu( u8 page, u8 line );
void printUniqueID( u8 drawLineOffset );

void drawHeader( u8 menu, u8 page, u8 line )
// 画面端に簡単な情報を表示する
{
	PutStringUTF16( HEADER_LEFT, HEADER_UP, TXT_COLOR_RED, (const u16 *)L"DisplaySystemInfo");
	if( menu != MENU_ROOT )
	{
		u16 buf[256];
		swprintf(buf, 256, L"Root>%s page %d / %d", s_strMenuName[menu], page+1 , (s_numMenuK[menu] / NUM_LINES) + 1 );
		PutStringUTF16( HEADER_LEFT, HEADER_UP + LINE_OFFSET, TXT_COLOR_BLUE, buf );	
	}
	else
	{
		PutStringUTF16( HEADER_LEFT, HEADER_UP + LINE_OFFSET, TXT_COLOR_BLUE, (const u16 *)L"Root" );	
	}
	
	PutStringUTF16( FOOTER_LEFT, FOOTER_UP, TXT_COLOR_BLUE, (const u16 *)L" A: Decide   B: Back ");
}

void drawMenu( u8 menu, u8 page, u8 line )
// 情報一覧を描画する
{
	switch( menu ){
		case MENU_ROOT:
			drawRootMenu( page, line );
			break;
		case MENU_OWNER:
			drawOwnerMenu( page, line );
			break;
		case MENU_PARENTAL:
			drawParentalMenu( page, line );
			break;
		case MENU_OTHER:
			drawOtherMenu( page, line );
			break;
		case MENU_SCFG:
			drawSCFGMenu( page, line );
			break;
		case MENU_FUSE:
			drawFuseMenu( page, line );
			break;
	}	
}

void drawKindName( u8 menu, u8 page, u8 line )
// 項目名描画関数
{
	u8 linenum = 0;
	u8 offset = 0;
	for(linenum = NUM_LINES * page ; linenum < s_numMenuK[menu] && linenum < (page+1) * NUM_LINES; linenum++, offset++)
	{
		if( linenum == NUM_LINES * page + line )
		{
			// 選択中の項目は表示色かえて矢印表示
			PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*offset, TXT_COLOR_BLACK, (const u16 *)L"→" );
			PutStringUTF16( KIND_LEFT, KIND_UP + LINE_OFFSET*offset, TXT_COLOR_GREEN, s_strMetaMenu[menu][linenum]) ;
		}
		else
		{
			PutStringUTF16( KIND_LEFT, KIND_UP + LINE_OFFSET*offset, TXT_COLOR_BLACK, s_strMetaMenu[menu][linenum]) ;
		}
	}
	
}

void drawRootMenu( u8 page, u8 line )
{
	drawKindName( MENU_ROOT, page, line);
}

void drawOwnerMenu( u8 page, u8 line )
{
	u8 linenum;
	
	drawKindName( MENU_OWNER, page, line);
	
	for(linenum = page * NUM_LINES; linenum < s_numMenuV[MENU_OWNER] && linenum < (page+1) * NUM_LINES; linenum++)
	{
		// utf16で描画するものだけ分ける
		if(linenum == OWNER_NICKNAME)
		{
			PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, gUserName) ;
		}
		else if(linenum == OWNER_COMMENT)
		{
			PutStringUTF16( VALUE_LEFT + ROW_OFFSET, VALUE_UP + LINE_OFFSET*( linenum+1 ), TXT_COLOR_BLACK, gUserComment) ;
		}
		else
		{
			// sjisで描画するもの
			PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, gAllInfo[MENU_OWNER][linenum] );
		}
	}
}

void drawParentalMenu( u8 page, u8 line )
{
	u8 linenum;
	
	drawKindName( MENU_PARENTAL, page, line);
	
	for(linenum = page * NUM_LINES; linenum < s_numMenuV[MENU_PARENTAL] && linenum < (page+1) * NUM_LINES; linenum++)
	{
		// utf16で描画するものだけ分ける
		if(linenum == PARENTAL_ANSWER)
		{			
			PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, gSecretAnswer) ;
		}
		else
		{
			// sjisで描画するもの
			PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, gAllInfo[MENU_PARENTAL][linenum] );
		}
	}
}	

void drawOtherMenu( u8 page, u8 line )
{
	u8 linenum, drawLineOffset=0;
	u8 nowPageMaxLine = s_pageOffset[MENU_OTHER][page+1] - s_pageOffset[MENU_OTHER][page];
//	u8 maxline = s_numMenuV[MENU_OTHER] - page*5 > 5 ? 5 : s_numMenuV[MENU_OTHER] - page*5;
	drawKindName( MENU_OTHER, page, line);
	
	for(linenum = 0; linenum < nowPageMaxLine; linenum++)
	{
		u8 valueIdx = s_pageOffset[MENU_OTHER][page] + linenum;
		drawLineOffset = linenum;
		
		// 複数行必要な項目があると描画位置がずれるのでそれの対応
		if( valueIdx >= OTHER_FORCE_DISABLE && page == 0)
		{
			drawLineOffset++;
		}
		
		if( valueIdx > OTHER_UNIQUE_ID && page == 1 )
		{
			drawLineOffset += OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 / UNIQUE_BUF;
		}
		
		// 描画する
		if( valueIdx == OTHER_UNIQUE_ID )
		{
			printUniqueID(drawLineOffset);
		}
		else
		{
			PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, gAllInfo[MENU_OTHER][valueIdx] );
		}
	}
}

void printUniqueID( u8 drawLineOffset)
// ユニークIDを整形して出力
{
	char buf[UNIQUE_BUF+1];
	u8 i;
	
	for( i=0; i * UNIQUE_BUF < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 ; i++)
	{
		strncpy(buf, &gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i * UNIQUE_BUF], UNIQUE_BUF);
		buf[UNIQUE_BUF] = '\0';
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET * (drawLineOffset + i), TXT_COLOR_BLACK, buf );
	}
	
}

void drawSCFGMenu( u8 page, u8 line )
{
	drawKindName( MENU_SCFG, page, line);
}

void drawFuseMenu( u8 page, u8 line )
{
	drawKindName( MENU_FUSE, page, line);
}
