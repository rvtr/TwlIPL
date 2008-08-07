/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     viewSystemInfo.c

  Copyright **** Nintendo.  All rights reserved.

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

#include "misc.h"
#include "viewSystemInfo.h"
#include "strResource.h"
#include "control.h"

#include "getInformation.h"

/* function prototype  ----------------------------- */
void getAllInfo( void );
void initInfo( void );
void infoAlloc( DispInfoEntry *p, u8 index, u8 size, BOOL isSjis );
void printAllInfo ( void );
void getParentalInfo( void );
void getOtherInfo( void );


/* global variables ----------------------------- */

DispInfoEntry* gAllInfo[ROOTMENU_SIZE]; // それぞれのメニューごとに項目の中身を"文字列で"持つ配列

// コンテンツタイトル関係
OSTitleId *gContentsTitle;		// コンテンツタイトルの一覧
s32 gNumContents;				// gContentsTitleに含まれるタイトルの数
u16 *gContentsVersion;			// gContentsTitleそれぞれのバージョン情報

// ARM7側のSCFGデータを保持
u8 gArm7SCFGReg[DISPINFO_SHARED_SCFG_REG_SIZE];
u8 gArm7SCFGShared[DISPINFO_SHARED_SCFG_WRAM_SIZE];

// LCFGReadの結果
BOOL gLCFGAccessible;
u8 * gBufLCFG;

/* static data ---------------------------------- */


/* function definition -------------------------- */

void displayInfoMain( void )
{
	static int menu = MENU_ROOT;
	static int line = 0; 		// 現在選択しているページナンバと行
	static int changeLine = 0;	// changeMode用
	static BOOL firstCall = TRUE;	// 最初の呼び出しか否か
	
	// 0: changemode 1: executemode 2: result on/off 3:(require 2=true) success/failed
	static u8 modeFlag = 0;

	static BOOL isChangeMode = FALSE;
	static BOOL executeMode = FALSE;
	
	BOOL repaintFlag = FALSE;
	ChangeCotnrolResult changeResult = CHANGE_NOTHING;
	
	// 初回の呼び出し時は全ての情報を取得しなおす
	if( firstCall )
	{
		initInfo();
		getAllInfo();
	}
	
	if( modeFlag & MODE_CHANGE_MASK )
	{
		changeResult = changeControl( &menu, &line, &changeLine, &modeFlag );
	}
	else if( modeFlag & MODE_EXECUTE_MASK )
	{
		repaintFlag |= executeControl( &line, &changeLine, &modeFlag );
	}
	else
	{
		repaintFlag |= control( &menu, &line, &changeLine, &modeFlag );
	}

	if( changeResult == CHANGE_VALUE_CHANGED )
	{
		// 値が更新されたときは全部取得しなおす
		LCFG_WriteTWLSettings( (u8 (*) [LCFG_WRITE_TEMP] ) gBufLCFG );
		getAllInfo();
	}
	
	// パッド情報で選んでる場所とか情報とか更新
	if(	changeResult == CHANGE_VALUE_CHANGED || changeResult == CHANGE_CONTROL || repaintFlag || firstCall )
	{
		// 何か操作があったときはキャンバスクリアして描画しなおし
	    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
   	    NNS_G2dCharCanvasClear( &gCanvasSub, TXT_COLOR_WHITE );
		// 情報一覧を描画する
		drawMenu( menu, line, changeLine, modeFlag );

		firstCall = FALSE;
	}
	
}

void initInfo( void )
// 固定文字列で対応できない項目に文字列領域を割り当てる
{
	OS_TPrintf("buffer initialize\n");
	
	// sjis (char)
	infoAlloc( gAllInfo[MENU_OWNER], OWNER_BIRTHDAY, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_USER], SECURE_USER_LASTBOOT_ID, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_USER], SECURE_USER_LAUNCHER_ID, DISPINFO_BUFSIZE, TRUE );
	infoAlloc( gAllInfo[MENU_NORMAL_HW], NORMAL_HW_UNIQUE_ID, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_SERIAL , OS_TWL_HWINFO_SERIALNO_LEN_MAX + 1, TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_LANGUAGE, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_FUSE, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SCFG_ARM7], SCFG_ARM7_MI_CC, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SCFG_ARM7], SCFG_ARM7_MI_CA, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_WL], WL_VERSION, DISPINFO_BUFSIZE, TRUE );
	
	// utf(u16)
	infoAlloc( gAllInfo[MENU_SYSMENU], SYSMENU_EULA_URL, TWL_EULA_URL_LEN + 1, FALSE );
	infoAlloc( gAllInfo[MENU_SYSMENU], SYSMENU_NUP_HOST, TWL_NUP_HOSTNAME_LEN + 1, FALSE );
	infoAlloc( gAllInfo[MENU_OWNER], OWNER_NICKNAME, OS_OWNERINFO_NICKNAME_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_OWNER], OWNER_COMMENT, OS_OWNERINFO_COMMENT_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_PARENTAL], PARENTAL_ANSWER, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_SYSMENU], SYSMENU_VERSION_STR, TWL_SYSMENU_VER_STR_LEN + 1, FALSE );

	OS_TPrintf( "information alloc succeeded\n" );
}

	
void infoAlloc( DispInfoEntry *p, u8 index, u8 size, BOOL isSjis )
// initInfoの作業用関数。isSjis=trueだとcharを、falseならu16を割り当てる
{
	if( isSjis )
	{
		p[index].str.sjis = (char*) Alloc (sizeof(char) * size);
		SDK_ASSERT( p->str.sjis );
		p[index].str.sjis[size] = '\0';
	}
	else
	{
		p[index].str.utf = (u16*) Alloc (sizeof(u16) * size);
		SDK_ASSERT( p->str.utf );
		p[index].str.utf[size-1] = '\0';
	}

}


void getAllInfo( void )
// 
{
	BOOL SCFGAccessable = FALSE;
	BOOL fuseRomAccessable = FALSE;

	OS_TPrintf("reflesh Information\n");
		
	getOwnerInfo();
	getParentalInfo();
	getSecureUserInfo();
	getOtherInfo();
	getHWInfo();
	getSCFGInfo();
	getSysmenuInfo();
	getFontInfo();
	getWhiteListInfo();
	getWLInfo();
	
#if NAM_ENABLE
	getContentsVersion();
#endif
	
	printAllInfo();
	OS_TPrintf("reflesh information finished\n");
}


void displayInfoInit( void )
{
	int loop1, loop2;
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	OS_TPrintf(" size version: %d\n", s_numMenu[MENU_VERSION]);	
	// 全体情報を持つ配列をセット
	// 可能なものは用意したstatic文字列へのポインタで対応
	// 必要なものだけあとでmallocする
	for(loop1=0; loop1<ROOTMENU_SIZE; loop1++)
	{
		// ARM7メニューのとこだけバッファを二倍分取る
		int numBuf = loop1 == MENU_SCFG_ARM7 ? SCFG_ARM7_MENU_SIZE*2 : s_numMenu[loop1];
		
		gAllInfo[loop1] = (DispInfoEntry*)Alloc(sizeof(DispInfoEntry) * numBuf);
				
		SDK_ASSERT( gAllInfo[loop1] );
				
		for(loop2 = 0; loop2 < numBuf; loop2++)
		{
			// デフォルト値の設定
			gAllInfo[loop1][loop2].kind = s_strMetaMenu[loop1][loop2];
			gAllInfo[loop1][loop2].iValue = 0;
			gAllInfo[loop1][loop2].isNumData = FALSE;
			gAllInfo[loop1][loop2].numLines	= 1;
			gAllInfo[loop1][loop2].isSjis = TRUE;
			gAllInfo[loop1][loop2].isAligned = TRUE;
			gAllInfo[loop1][loop2].str.sjis = s_strNA;
			gAllInfo[loop1][loop2].changable = FALSE;
			gAllInfo[loop1][loop2].fromLCFG = FALSE;
		}
	}
	
	// LCFGデータの読み出し
	gBufLCFG = (u8*) Alloc ( LCFG_READ_TEMP );
	SDK_ASSERT( gBufLCFG );
	gLCFGAccessible = LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ]) gBufLCFG );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	OS_TPrintf(" size version: %d\n", s_numMenu[MENU_VERSION]);
}

void printAllInfo ( void )
{
	int loop1, loop2;
	OS_TPrintf(" size version: %d\n", s_numMenu[MENU_VERSION]);
	
	for(loop1=0; loop1 < ROOTMENU_SIZE; loop1++ )
	{
		for(loop2=0; loop2 < s_numMenu[loop1]; loop2++ )
		{
			DispInfoEntry *entry;
			
			if( loop1 == MENU_VERSION && VERSION_OTHER <= loop2 )
			{
				int idx = loop2 - VERSION_OTHER;
				OS_TPrintf("%d %d : %x %x\n", loop1, loop2, gContentsTitle[idx], gContentsVersion[idx] );
				continue;
			}
			
			if( loop1 == MENU_FONT && FONT_INFO <= loop2 )
			{
				int infoType = (loop2 - FONT_INFO) % NUM_FONT_INFO;
				int fontidx = (loop2 - FONT_INFO) / NUM_FONT_INFO;
				
				switch( infoType )
				{
					case 0:
					OS_TPrintf("%d %d : %s\n", loop1, loop2, gFontInfo[fontidx].name );
					break;
					
					case 1:
					OS_TPrintf("%d %d : %d\n", loop1, loop2, gFontInfo[fontidx].size );
					break;
					
					case 2:
					OS_TPrintf("%d %d : %d\n", loop1, loop2, gFontInfo[fontidx].isHashOK );
					break;
				}

				continue;
			}
			
			entry = &gAllInfo[loop1][loop2];

			if( entry->isNumData )
			{
				OS_TPrintf("%d %d : %d\n", loop1, loop2, entry->iValue);
			}
			else if( entry->isSjis )
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, entry->str.sjis);
			}
			else
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, entry->str.utf); 
			}
			
		}
	}
}


