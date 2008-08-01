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
 * ���j���[�\���@�\ 
 * 
 * �e�X�g�A�v���ɓK�����ȒP�ȃ��j���[�\�����s���B���j���[�K�w�͕����i�\�B
 * 
 * �Ȃ��A���̋@�\���g�p���邽�߂ɂ́A�ȉ��̃\�[�X���K�v
 *   font.c, font.h, screen.c, screen.h
 * 
 * �����菇�F
 *   1, menu.h�̈ȉ��̒�`�l��ύX����
 *       MENU_DEPTH_NUM   : ���j���[�̊K�w��
 *       MENU_ITEM_NUM    : ���j���[���ڂ̍ő吔
 *       MENU_ITEM_STRLEN : ���j���[�\��������̍ő咷
 *   2, menu.c�̈ȉ��̒�`��ύX����
 *       MENU_TITLE       : ���j���[�̃^�C�g��
 *   3, �ȉ��̍\���̂�ύX����
 *       MenuItem       : ���j���[�\�����e����ъK�w�ԏ��
 *   4, �֐� ExecMenuItem() �����j���[���ڂɉ������������`����n���h���Ȃ̂ŁA
 *      ��`���s��
 *   5, Main����ȉ��̊֐����Ă�
 *       InitMenu       : ���j���[�֘A�̏�����
 *       DisplayMenuSet : ���j���[�@�\�̕\��
 *       ChangeMenuItem : ���j���[�I�����ڂ̕ύX(���̊֐��̓L�[���쎞�ɌĂяo���悤�ɂ���)
 *       ExecMenuItem   : ���j���[���ڂɉ����������̎��s(���̊֐��̓L�[���쎞�ɌĂяo���悤�ɂ���)
 *
 *
 *   �s���ȓ_�͈��c�܂ŁB
 */

#define MENU_TITLE  "APP version viewer"
//#define NEW_MENU_POS		// YASUDA
#define DRAW_LINE_AS_CHAR

static char cItemStr[9][MENU_ITEM_STRLEN];

static menu_sheet_t MenuItem[MENU_DEPTH_NUM] =
{
    {-1,  1, cItemStr[0],  2, cItemStr[1], 3, cItemStr[2], 4, cItemStr[3],
		  5, cItemStr[4], 6, cItemStr[5],  7, cItemStr[6], 8, cItemStr[7],
	     -1, cItemStr[8] }
};

static s16 MenuDepth;
static s16 MenuPos;
static u8  isMenuCancel;

//---- for RTC
static RTCTime myCurrentTime;
static int myResult;

static void DisplayMenu(void);
static void EnterMenuCancel(void);

static void setAllItems(void);
static void setAppVerItem(void);
static void setPublicDataItem(void);
static void setPrivateDataItem(void);

// YASUDA
extern FSResult g_fs_result;

//----------------------------------------------------------------
//  InitMenu
//
void InitMenu(void)
{
    MenuDepth = 0;
    MenuPos   = 2;

	setAllItems();
#if defined(DRAW_LINE_AS_CHAR)
	STD_CopyString( MenuItem[0].sItem[1].cItemStr, "---------+------------" );
	STD_CopyString( MenuItem[0].sItem[5].cItemStr, "---------+------------" );
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

    //---- display key description
    PrintString(3, 2, 15, MENU_TITLE);
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
#if !defined(NEW_MENU_POS)
    PrintString(5, 17, 15, "[A] set current data");
    PrintString(5, 18, 15, "[X] set both data");
    PrintString(5, 19, 15, "[up/down] switch item");
#else
    PrintString(5, 19, 15, "[A] set current data");
    PrintString(5, 20, 15, "[X] set both data");
    PrintString(5, 21, 15, "[up/down] switch item");
#endif
}

//----------------------------------------------------------------
//  DisplayMenu
//
static void DisplayMenu(void)
{
    u16 pos;
    char *str;

    for(pos = 0; pos < MENU_ITEM_NUM; pos++)
    {
        str = MenuItem[MenuDepth].sItem[pos].cItemStr;
        //---- display key description
        if( str[0] != '\0' )
        {
#if !defined(DRAW_LINE_AS_CHAR)
            PrintString(3, (s16)(5 + pos), 15, str);
#else
			if( (pos != 1) && (pos != 5) )
			{
	            PrintString(3, (s16)(5 + pos), 15, str);
			}
			else
			{
	            PrintString(3, (s16)(5 + pos),  6, str);
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
			//����̃`���[�j���O
			if( (MenuPos>=0) && (MenuPos<6) )
			{
				MenuPos = 2;
			}
			if( (MenuPos>=6) && (MenuPos<9) )
			{
				MenuPos = 6;
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
			//����̃`���[�j���O
			if( (MenuPos>0) && (MenuPos<=2) )
			{
				MenuPos = 2;
			}
			if( (MenuPos>2) && (MenuPos<=9) )
			{
				MenuPos = 6;
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
				case 1:
				case 2:
				case 3:
					// Public data �^�C���X�^���v�ύX
					SetPublicDataTimeStamp();
#if 0  // YASUDA
					STD_TSPrintf( cItemStr[1], "PUB err  : %d", g_fs_result );
#else
					setPublicDataItem();
#endif
					break;
				case 4:
				case 5:
				case 6:
					// Private data �^�C���X�^���v�ύX
					SetPrivateDataTimeStamp();
					setPrivateDataItem();
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
					// �� data �^�C���X�^���v�ύX
					SetPublicDataTimeStamp();
					SetPrivateDataTimeStamp();
					setAllItems();
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
