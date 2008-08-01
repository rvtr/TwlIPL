/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - yasuda - MenuTest
  File:     menu.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	MENU_H_
#define	MENU_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<nitro/types.h>

//メニューに関する定義
#define  MENU_DEPTH_NUM    2
#define  MENU_ITEM_NUM     11
#define  MENU_ITEM_STRLEN  24

//ChangeMenuItemのパラメータ
#define  MENU_ITEM_UP     0
#define  MENU_ITEM_DOWN   1
#define  MENU_DEPTH_UP    2
#define  MENU_DEPTH_DOWN  3


typedef struct {
    s16         nNext;    // -1: これ以上枝はない
    //char        cItemStr[MENU_ITEM_STRLEN];
	char       *cItemStr;
}menu_item_t;

typedef struct {
    s16         nPrev;    // -1: 自分が親
    menu_item_t sItem[MENU_ITEM_NUM];
}menu_sheet_t;


void DisplayMenuSet(void);
void InitMenu(void);
void ChangeMenuItem(u16 action);
void ExecMenuItem(void);
void ExecMenuItemB(void);
void ExecMenuItemX(void);
void ExecMenuItemRight(void);
void ExecMenuItemLeft(void);
void ExitMenuCancel(void);
void SetMenuString(u32 depth, u32 ipos, char *str);
void SetCurrentMenuString(char *str);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* MENU_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
