/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - yasuda - MenuTest
  File:     menu.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include  "font.h"
#include  "screen.h"
#include  "menu.h"
#include  "dataver.h"
#include  <nitro/std.h>

/*
 * menu.c
 * メニュー表示機能 
 * 
 * テストアプリに適した簡単なメニュー表示を行う。メニュー階層は複数段可能。
 * 
 * なお、この機能を使用するためには、以下のソースが必要
 *   font.c, font.h, screen.c, screen.h
 * 
 * 導入手順：
 *   1, menu.hの以下の定義値を変更する
 *       MENU_DEPTH_NUM   : メニューの階層数
 *       MENU_ITEM_NUM    : メニュー項目の最大数
 *       MENU_ITEM_STRLEN : メニュー表示文字列の最大長
 *   2, menu.cの以下の定義を変更する
 *       MENU_TITLE       : メニューのタイトル
 *   3, 以下の構造体を変更する
 *       MenuItem       : メニュー表示内容および階層間情報
 *   4, 関数 ExecMenuItem() がメニュー項目に応じた処理を定義するハンドラなので、
 *      定義を行う
 *   5, Mainから以下の関数を呼ぶ
 *       InitMenu       : メニュー関連の初期化
 *       DisplayMenuSet : メニュー機能の表示
 *       ChangeMenuItem : メニュー選択項目の変更(この関数はキー操作時に呼び出すようにする)
 *       ExecMenuItem   : メニュー項目に応じた処理の実行(この関数はキー操作時に呼び出すようにする)
 *
 *
 *   不明な点は安田まで。
 */

#define MENU_TITLE  "APP version viewer"
//#define NEW_MENU_POS		// YASUDA
#define DRAW_LINE_AS_CHAR

static char cItemStr[MENU_ITEM_NUM][MENU_ITEM_STRLEN];
static char cItemSt2[MENU_ITEM_NUM][MENU_ITEM_STRLEN];

static menu_sheet_t MenuItem[MENU_DEPTH_NUM] =
{
    {-1, -1, cItemStr[0], -1, cItemStr[1], -1, cItemStr[2], -1, cItemStr[3],
		 -1, cItemStr[4], -1, cItemStr[5], -1, cItemStr[6], -1, cItemStr[7],
	     -1, cItemStr[8], -1, cItemStr[9],  1, cItemStr[10] },
    { 0, -1, cItemSt2[0], -1, cItemSt2[1], -1, cItemSt2[2], -1, cItemSt2[3],
		 -1, cItemSt2[4], -1, cItemSt2[5], -1, cItemSt2[6], -1, cItemSt2[7],
	     -1, cItemSt2[8], -1, cItemSt2[9], -1, cItemSt2[10] },
};

static s16 MenuDepth;
static s16 MenuPos;
static u8  isMenuCancel = 0;

//---- for RTC
static RTCTime myCurrentTime;
static int myResult;

static void DisplayControlInfo(void);
static void DisplayMenu(void);
static void EnterMenuCancel(void);

static void setAllItems(void);
static void setAppVerItem(void);
static void setPublicDataItem(void);
static void setPrivateDataItem(void);
static void setOtherDataItem(void);
static void setAllItems_OtherApp(void);
static void setOtherAppVerItem(void);
static void setOtherNandAppPublicDataItem(void);
static void setOtherNandAppPrivateDataItem(void);

// YASUDA
extern FSResult g_fs_result;

static s32 g_other_app_num = 0;
static s32 g_current_other_app = 0;

#ifdef USER_MODE
    char* APP_TYPE = "USER";
#else
    char* APP_TYPE = "SEQURE";
#endif


//----------------------------------------------------------------
//  InitMenu
//
void InitMenu(void)
{
    MenuDepth = 0;
    MenuPos   = 2;

	GetOutNandAppInfo();

	setAllItems();
#if defined(DRAW_LINE_AS_CHAR)
	STD_CopyString( MenuItem[0].sItem[1].cItemStr, "---------+------------" );
	STD_CopyString( MenuItem[0].sItem[5].cItemStr, "---------+------------" );
	STD_CopyString( MenuItem[0].sItem[9].cItemStr, "---------+------------" );
#endif
}

//----------------------------------------------------------------
//  DisplayMenuSet
//
void DisplayMenuSet(void)
{
    if(isMenuCancel > 0)
    {
        return;
    }

    //PrintString(3, 3, 15, "parameter is %d", param);
    DisplayMenu();

    //---- display time
    myResult = RTC_GetTime(&myCurrentTime);
    if (myResult == 0 /*no error */ )
    {
#if !defined(NEW_MENU_POS)
        PrintString(5, 21, 8, "%02d:%02d:%02d",
                    myCurrentTime.hour, myCurrentTime.minute, myCurrentTime.second);
#else
        PrintString(5, 23, 8, "%02d:%02d:%02d",
                    myCurrentTime.hour, myCurrentTime.minute, myCurrentTime.second);
#endif
    }

    //---- display counter
    //PrintString(18, 20, 4, "%08X", OS_GetVBlankCount());

    //---- display control infomation
	DisplayControlInfo();
}

//----------------------------------------------------------------
//  DisplayControlInfo
//
static void DisplayControlInfo(void)
{
	if(MenuDepth == 0)
	{
	    //---- display key description
	    PrintString(3, 2, 15, MENU_TITLE);

		if( MenuPos != 10 )
		{
#if !defined(NEW_MENU_POS)
		    PrintString(5, 17, 15, "[A] set current data");
    		PrintString(5, 18, 15, "[X] set both data");
#else
	    	PrintString(5, 19, 15, "[A] set current data");
    		PrintString(5, 20, 15, "[X] set both data");
#endif
		}
		else
		{
#if !defined(NEW_MENU_POS)
		    PrintString(5, 17, 15, "[A] show other app data");
    		PrintString(5, 18, 15, "[X] set both data");
#else
	    	PrintString(5, 19, 15, "[A] show other app data");
    		PrintString(5, 20, 15, "[X] set both data");
#endif
		}
	}
	else
	{
	    //---- display key description
	    PrintString(3, 2, 15, "OTHER APP info");

#if !defined(NEW_MENU_POS)
	    PrintString(5, 17, 15, "[A/X] set data");
   		PrintString(5, 18, 15, "[B] return main");
   		PrintString(5, 19, 15, "[<-/->] switch apps");
#else
    	PrintString(5, 19, 15, "[A/X] set data");
   		PrintString(5, 20, 15, "[B] set both data");
   		PrintString(5, 21, 15, "[<-/->] switch apps");
#endif
	}
}

//----------------------------------------------------------------
//  DisplayMenu
//
static void DisplayMenu(void)
{
    u16 pos;
    char *str;

    PrintString(0, 0, 14, APP_TYPE);
    

    for(pos = 0; pos < MENU_ITEM_NUM; pos++)
    {
        str = MenuItem[MenuDepth].sItem[pos].cItemStr;
        //---- display key description
        if( str[0] != '\0' )
        {
#if !defined(DRAW_LINE_AS_CHAR)
            PrintString(3, (s16)(5 + pos), 15, str);
#else
			if( MenuDepth == 0 )
			{
				if( (pos != 1) && (pos != 5) && (pos != 9) )
				{
	    	        PrintString(3, (s16)(5 + pos), 15, str);
				}
				else
				{
	    	        PrintString(3, (s16)(5 + pos),  6, str);
				}
			}
			else
			{
				if( (pos != 2) && (pos != 6) )
				{
	    	        PrintString(3, (s16)(5 + pos), 15, str);
				}
				else
				{
	    	        PrintString(3, (s16)(5 + pos),  5, str);
				}
			}
#endif
        }

        PrintString(1, (s16)(5 + MenuPos), 8, "#");
    }
}


//----------------------------------------------------------------
//  ChangeMenuItem
//
void ChangeMenuItem(u16 action)
{
    switch(action)
    {
        case MENU_ITEM_UP:
            ColorString((s16)(1 + MenuPos), 5, 1, 0);
            MenuPos--;
            if(MenuPos < 0)
            {
                MenuPos = 0;
            }
			//動作のチューニング
			if( MenuDepth == 0 )
			{
				if( (MenuPos>=0) && (MenuPos<6) ){
					MenuPos = 2;
				}else if( (MenuPos>=6) && (MenuPos<10) ){
					MenuPos = 6;
				}else if( (MenuPos>=10) && (MenuPos<11) ){
					MenuPos = 10;
				}
			}
			else
			{
				if( (MenuPos>=0) && (MenuPos<7) ){
					MenuPos = 3;
				}else if( (MenuPos>=7) && (MenuPos<11) ){
					MenuPos = 7;
				}
			}
            break;
        case MENU_ITEM_DOWN:
            ColorString((s16)(1 + MenuPos), 5, 1, 0);
            MenuPos++;
            if(MenuPos >= MENU_ITEM_NUM)
            {
                MenuPos--;
            }
            if(MenuItem[MenuDepth].sItem[MenuPos].cItemStr[0] == '\0')
            {
                MenuPos--;
            }
			//動作のチューニング
			if( MenuDepth == 0 )
			{
				if( (MenuPos>0) && (MenuPos<=2) ){
					MenuPos = 2;
				}else if( (MenuPos>2) && (MenuPos<=6) ){
					MenuPos = 6;
				}else if( (MenuPos>6) && (MenuPos<=10) ){
					MenuPos = 10;
				}
			}
			else
			{
				if( (MenuPos>0) && (MenuPos<=3) ){
					MenuPos = 3;
				}else if( (MenuPos>3) && (MenuPos<=10) ){
					MenuPos = 7;
				}
			}
            break;
        case MENU_DEPTH_UP:
            MenuDepth = MenuItem[MenuDepth].nPrev;
            MenuPos = 0;
            if(MenuDepth < 0)
            {
                 MenuDepth = 0;
            }
            break;
        case MENU_DEPTH_DOWN:
            {
                s16 pos;
                pos = MenuItem[MenuDepth].sItem[MenuPos].nNext;
                if(pos >= 0)
                {
                    MenuDepth = pos;
                    MenuPos = 0;
                }
            }
            break;
    }
}


//----------------------------------------------------------------
//  ExecMenuItem
//
void ExecMenuItem(void)
{
	switch(MenuDepth)
	{
		case 0:
			switch(MenuPos)
			{
				case 0:
					break;
				case 2:
					// Public data タイムスタンプ変更
					SetPublicDataTimeStamp();
					setPublicDataItem();
					break;
				case 6:
					// Private data タイムスタンプ変更
					SetPrivateDataTimeStamp();
					setPrivateDataItem();
					setOtherDataItem();
					break;
				case 10:
					// 他アプリデータ表示へ
					ChangeMenuItem(MENU_DEPTH_DOWN);
					MenuPos = 3;
					setAllItems_OtherApp();
					break;
				default:
					break;
			}
			break;
		case 1:
			switch(MenuPos)
			{
				case 3:
					// 該当アプリのPublic data タイムスタンプ変更
					SetOurNandAppPublicDataTimeStamp(g_current_other_app);
					setOtherNandAppPublicDataItem();
					break;
				case 7:
					// 該当アプリのPrivate data タイムスタンプ変更
					SetOurNandAppPrivateDataTimeStamp(g_current_other_app);
					setOtherNandAppPrivateDataItem();
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------
//  ExecMenuItemB
//
void ExecMenuItemB(void)
{
	switch(MenuDepth)
	{
		case 1:
			// メインメニューに戻る
			ChangeMenuItem(MENU_DEPTH_UP);
			MenuPos = 2;
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------
//  ExecMenuItemX
//
void ExecMenuItemX(void)
{
	switch(MenuDepth)
	{
		case 0:
			switch(MenuPos)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
					// 両 data タイムスタンプ変更
					SetPublicDataTimeStamp();
					SetPrivateDataTimeStamp();
					setAllItems();
					break;
				default:
					break;
			}
			break;
		case 1:
			// 両 data タイムスタンプ変更
			SetOurNandAppPublicDataTimeStamp(g_current_other_app);
			SetOurNandAppPrivateDataTimeStamp(g_current_other_app);
			setAllItems_OtherApp();
			break;
		default:
			break;
	}
}


//----------------------------------------------------------------
//  ExecMenuItemRight
//
void ExecMenuItemRight(void)
{
	switch(MenuDepth)
	{
		case 1:
			// アプリ番号を変更
			if( g_other_app_num > 0 )
			{
				if(++g_current_other_app >= g_other_app_num )
				{
					g_current_other_app = 0;
				}
				// 表示書き換え
				setAllItems_OtherApp();
			}
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------
//  ExecMenuItemLeft
//
void ExecMenuItemLeft(void)
{
	switch(MenuDepth)
	{
		case 1:
			// アプリ番号を変更
			if( g_other_app_num > 0 )
			{
				if(--g_current_other_app < 0)
				{
					g_current_other_app = g_other_app_num - 1;
				}
				// 表示書き換え
				setAllItems_OtherApp();
			}
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------
//  EnterMenuCancel
//
static void EnterMenuCancel(void)
{
    isMenuCancel = 1;
    ClearScreen();
}

//----------------------------------------------------------------
//  ExitMenuCancel
//
void ExitMenuCancel(void)
{
    if(isMenuCancel > 0){
        isMenuCancel = 0;
    }
}

void SetMenuString(u32 depth, u32 ipos, char *str)
{
	STD_CopyString( MenuItem[depth].sItem[ipos].cItemStr, str );
}

void SetCurrentMenuString(char *str)
{
	SetMenuString( MenuDepth, MenuPos, str );
}

static void setAllItems(void)
{
	setAppVerItem();
	setPublicDataItem();
	setPrivateDataItem();
	setOtherDataItem();
}

static void setAppVerItem(void)
{
	char ver[8];

	GetAppVersionString(ver);
	STD_TSPrintf( cItemStr[0], "APP ver  : %s", ver );
}

static void setPublicDataItem(void)
{
	char ver[8], time1[16], time2[16];

	if( GetPublicDataInfoStrings( ver, time1, time2 ) == TRUE )
	{
		STD_TSPrintf( cItemStr[2], "PUB ver  : %s", ver );
		STD_TSPrintf( cItemStr[3], "    date :  %s", time1 );
		STD_TSPrintf( cItemStr[4], "         :  %s", time2 );
	}
	else
	{
		STD_TSPrintf( cItemStr[2], "PUB      : NONE", ver );
		STD_TSPrintf( cItemStr[3], "          " );
		STD_TSPrintf( cItemStr[4], "          " );
	}
}

static void setPrivateDataItem(void)
{
	char ver[8], time1[16], time2[16];

	if( GetPrivateDataInfoStrings( ver, time1, time2 ) == TRUE )
	{
		STD_TSPrintf( cItemStr[6], "PRV ver  : %s", ver );
		STD_TSPrintf( cItemStr[7], "    date :  %s", time1 );
		STD_TSPrintf( cItemStr[8], "         :  %s", time2 );
	}
	else
	{
		STD_TSPrintf( cItemStr[6], "PRV      : NONE" );
		STD_TSPrintf( cItemStr[7], "          " );
		STD_TSPrintf( cItemStr[8], "          " );
	}
}

static void setOtherDataItem(void)
{
	g_other_app_num = GetOurNandAppNum();
	STD_TSPrintf( cItemStr[10], "OTHER    : %d apps", g_other_app_num );
}

static void setAllItems_OtherApp(void)
{
	setOtherAppVerItem();
	setOtherNandAppPublicDataItem();
	setOtherNandAppPrivateDataItem();

#if defined(DRAW_LINE_AS_CHAR)
	STD_CopyString( MenuItem[1].sItem[2].cItemStr, "---------+-------------" );
	STD_CopyString( MenuItem[1].sItem[6].cItemStr, "---------+-------------" );
#endif
}

static void setOtherAppVerItem(void)
{
	char ver[8], ini[8];

	if( g_other_app_num == 0 )
	{
		STD_TSPrintf( cItemSt2[0], "OTHER APP : none" );
	}
	else
	{
		GetOurNandAppVersionString( g_current_other_app, ver, ini );

		STD_TSPrintf( cItemSt2[0], "No.      : %02d/%02d (%s)",
						g_current_other_app + 1, g_other_app_num, ini );
		STD_TSPrintf( cItemSt2[1], "APP ver  : %s", ver );
	}
}


static void setOtherNandAppPublicDataItem(void)
{
	char ver[8], time1[16], time2[16];

	if( GetOurNandAppPublicDataInfoStrings( g_current_other_app, ver, time1, time2 ) == TRUE )
	{
		STD_TSPrintf( cItemSt2[3], "PUB ver  : %s", ver );
		STD_TSPrintf( cItemSt2[4], "    date :  %s", time1 );
		STD_TSPrintf( cItemSt2[5], "         :  %s", time2 );
	}
	else
	{
		STD_TSPrintf( cItemSt2[3], "PUB      : NONE", ver );
		STD_TSPrintf( cItemSt2[4], "          " );
		STD_TSPrintf( cItemSt2[5], "          " );
	}
}

static void setOtherNandAppPrivateDataItem(void)
{
	char ver[8], time1[16], time2[16];

	if( GetOurNandAppPrivateDataInfoStrings( g_current_other_app, ver, time1, time2 ) == TRUE )
	{
		STD_TSPrintf( cItemSt2[7], "PRV ver  : %s", ver );
		STD_TSPrintf( cItemSt2[8], "    date :  %s", time1 );
		STD_TSPrintf( cItemSt2[9], "         :  %s", time2 );
	}
	else
	{
		STD_TSPrintf( cItemSt2[7], "PRV      : NONE" );
		STD_TSPrintf( cItemSt2[8], "          " );
		STD_TSPrintf( cItemSt2[9], "          " );
	}
}
