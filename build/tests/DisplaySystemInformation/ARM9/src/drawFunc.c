/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     drawFunc.c
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
 

#include <twl.h>
#include <wchar.h>
#include <string.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include <twl/os/common/sharedFont.h>

#include "drawFunc.h"
#include "getInformation.h"
#include "viewSystemInfo.h"
#include "misc.h"
#include "strResource.h"
#include "myIoreg_SCFG.h"



// 描画関連
#define HEADER_UP 0
#define HEADER_LEFT 10
#define PAGE_LEFT 160
#define FOOTER_UP 110
#define FOOTER_LEFT 10
#define ALLOW_LEFT 0

#define AST_LEFT 	( ALLOW_LEFT + LINE_OFFSET )	
#define KIND_UP 	10								// 項目名の上座標
#define KIND_LEFT 	( AST_LEFT + 10 )				// 項目名の左座標
#define VALUE_UP 	KIND_UP							// 項目値の上座標
#define VALUE_LEFT 	140								// 項目値の左座標
#define LINE_OFFSET 15								// 1行ごとのオフセット
#define ROW_OFFSET	15 								// 長い項目が現れたときの段組用

#define REGISTER_NAME_LEFT	10
#define REGISTER_NAME_UP	15
#define REGISTER_DATA_LEFT	50
#define REGISTER_DATA_UP	15

#define FOOTER_REGIST 	0
#define FOOTER_CONTROL1 1
#define FOOTER_CONTROL2 2
#define FOOTER_MENU		3
#define FOOTER_LINES	4

#define CHANGE_ITEMNAME_UP		20
#define CHANGE_ITEMNAME_LEFT	20
#define CHANGE_VALUE_LEFT 		80
#define CHANGE_VALUE_UP 		80
#define CHANGE_ALLOW_LEFT		(CHANGE_VALUE_LEFT - 15)
#define CHANGE_NOW_SETTING_UP	150

#define RESULT_LEFT	CHANGE_ITEMNAME_LEFT
#define RESULT_UP	CHANGE_NOW_SETTING_UP


#define SCROLL_MARGIN 2 							// 画面端何行でスクロールするか

#define UNIQUE_BUF 12
#define BINARY_BUF 18

/* global variables -------------------- */

static int gDrawIdx[ROOTMENU_SIZE+1] = {0};			// 今何項目目から下を描画しているのか
static int gMenuLineSize[ROOTMENU_SIZE] = {};		// 各メニューの全体行数
static const int gRegisterIdx[2][SCFG_ARM7_MENU_SIZE];
static BOOL gSelectedARM7SCFGReg = TRUE;			// ARM7SCFGでレジスタサイドの描画ならtrue,共有領域ならfalse

/* function prototypes -------------------- */

void calibrateDrawIdx( int menu, int idx );
void printData( int x, int y, int color, DispInfoEntry *entry );
void printValue( int menu,int entryLine, int drawOffset, DispInfoEntry *entry );
void printKindName( int menu, int entryLine, int drawOffset, int selected );
void printBinary16( int x, int y, u16 value, int selected, int selectSize );
void printBinary32( int x, int y, u32 value, int selected, int selectSize );
void drawRegister( int menu, int selected );
void drawChangeMode( DispInfoEntry *Entry,  int changeLine );
void drawExecuteMode( const char *command, int changeLine, u8 mode );
void drawFontInfo( int idx, int drawLine, int selected );
void drawVersion( int idx, int drawLine ,int selected );
void printUniqueID( int drawLineOffset, char *uniqueId );
int getPageNum( int valueIdx, const int* pageOffset );
int countMenuLine( int menu );
int countLinesDown( int menu, int idx );

void drawHeader( int menu, int line)
// 画面端に簡単な情報を表示する
{
	u16 buf[256];
	
	PrintfSJISSub( HEADER_LEFT, HEADER_UP, TXT_COLOR_RED, "%s %d" , "DisplaySystemInfo", DISPINFO_BIN_IDX );


	if( menu != MENU_ROOT )
	{
		swprintf(buf, 256, L"Root > %s", s_strRootMenu[menu] );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + FOOTER_MENU * LINE_OFFSET, TXT_COLOR_BLUE, buf );
		
		swprintf(buf, 256, L"%d / %d", line+1 , s_numMenu[menu] );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + FOOTER_LINES * LINE_OFFSET, TXT_COLOR_BLUE, buf );	
	}
	else
	{
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + FOOTER_MENU * LINE_OFFSET, TXT_COLOR_BLUE, (const u16 *)L"Root" );
		swprintf(buf, 256, L"%d / %d", line+1 , ROOTMENU_SIZE );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + FOOTER_LINES * LINE_OFFSET, TXT_COLOR_BLUE, buf );
	}
	
	PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + FOOTER_CONTROL1 * LINE_OFFSET, TXT_COLOR_BLUE, (const u16 *)L" A: Decide   B: Back ");
	
	if( menu == MENU_SCFG_ARM7 )
	{
		PrintfSJISSub( FOOTER_LEFT, FOOTER_UP + FOOTER_REGIST * LINE_OFFSET, TXT_COLOR_RED, s_strSCFGViewMode[ gSelectedARM7SCFGReg ] );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + FOOTER_CONTROL2 * LINE_OFFSET, TXT_COLOR_BLUE, (const u16 *)L" Select: Switch Viewing Area" );
	}
}

void printData( int x, int y, int color, DispInfoEntry *entry )
{
	if( entry->isNumData )
	{
		// 数値データだったらそのままプリント
		PrintfSJIS( x, y, color, "%d", entry->iValue );
	}
	else if( entry->isSjis )
	{
		if( entry->str.sjis == s_strCorrect[0] )
		{
			color = TXT_COLOR_RED;
		}
		
		// SJIS文字列
		PrintfSJIS( x, y, color, "%s", entry->str.sjis );
	}
	else
	{
		// UTF16文字列
		PutStringUTF16( x, y, color, entry->str.utf );
	}
	
}

int countMenuLine( int menu )
{
	int i;
	int sum = 0;
	
	for( i=0; i<s_numMenu[menu]; i++ )
	{
		sum += gAllInfo[menu][i].numLines;
	}
	
	return sum;
}
void printKindName( int menu, int entryLine, int drawOffset, int selected )
{
	int kindColor = TXT_COLOR_BLACK;
	
	// 項目名描画
	if( entryLine == selected)
	{
		// 選択項目はいろかえる
		kindColor = TXT_COLOR_GREEN;
		PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, (const u16 *)L"→");
	}
	
	if( menu != MENU_ROOT && gAllInfo[menu][entryLine].changable )
	{
		PutStringUTF16( AST_LEFT, KIND_UP + LINE_OFFSET*drawOffset, kindColor, (const u16 *) L"*");
	}
	
	PrintfSJIS( KIND_LEFT, KIND_UP + LINE_OFFSET*drawOffset , kindColor, s_strMetaMenu[menu][entryLine] );

}	

void printValue( int menu,int entryLine, int drawOffset, DispInfoEntry *entry )
{
	int txtColor = TXT_COLOR_BLACK;
	
	if( entry->fromLCFG )
	{
		// LCFG経由で取得したデータは青にする
		txtColor = TXT_COLOR_BLUE;
	}
	
	//////////////////////////////////////////////	
	// 特殊描画を行う必要がある場合の処理はここ //
	//////////////////////////////////////////////
	
	if( menu == MENU_OWNER && entryLine == OWNER_COLOR )
	{
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawOffset, TXT_UCOLOR_GRAY + entry->iValue , "■" );
		printData( VALUE_LEFT + LINE_OFFSET , VALUE_UP + LINE_OFFSET*drawOffset, txtColor, entry );
		return;
	}
	
	if( menu == MENU_NORMAL_HW && entryLine == NORMAL_HW_UNIQUE_ID )
	{
		printUniqueID( drawOffset, entry->str.sjis );
		entry->numLines = 4;
		return;
	}
	
	if( menu == MENU_PARENTAL && entryLine == PARENTAL_PASSWORD )
	{
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset, txtColor, "%04d", entry->iValue );
		return;
	}
	
	if( menu == MENU_SYSMENU && entryLine == SYSMENU_VERSION_NUM )
	{
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset,
			 txtColor, "%d.%d", entry->iValue >> 16,  entry->iValue & 0xFFFF );
		return;
	}
	
	if( ( menu == MENU_SYSMENU && entryLine == SYSMENU_TIMESTAMP )||
		( menu == MENU_FONT && entryLine == FONT_TIMESTAMP ))
	{
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset, txtColor, "%08lx", entry->iValue );
		return;
	}
	
	if( menu == MENU_SECURE_HW && entryLine == SECURE_HW_LAUNCHER_ID )
	{
		char buf[5];
		
		if( entry->iValue == 0 )
		{
			// 情報が受け取れていない状態
			PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET * (1 + drawOffset), txtColor, s_strNA );
		}
		
		MI_CpuCopy( &(entry->iValue), buf, 4 );
		buf[4] = '\0';
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET * (1 + drawOffset), txtColor, "%s", buf );
		return;
	}
	
	if( menu == MENU_SECURE_HW && entryLine == SECURE_HW_FUSE )
	{
		// 16文字を8文字の二段組みに変更
		char buf[9] = {0};
		STD_StrLCpy( buf, entry->str.sjis , 9 );
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET * drawOffset, txtColor, "%s", buf );
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET * (1+drawOffset), txtColor, "%s", &(entry->str.sjis[8]) );
		return;
	}
	
	if( ( menu == MENU_SYSMENU && entryLine == SYSMENU_EULA_URL ) ||
		( menu == MENU_SYSMENU && entryLine == SYSMENU_NUP_HOST ))
	{
		char buf[17] = {0};
		int i, lineOffset = 0;
		
		for( i = 0; i < STD_StrLen( entry->str.sjis ) ; i+=16 )
		{
			STD_StrLCpy( buf, &(entry->str.sjis[i]), 17 );
			PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET * ( drawOffset + lineOffset++), txtColor, "%s", buf );
		}
		
		entry->numLines = lineOffset > 0 ? lineOffset : 1;
		return;
	}

	if( ( menu == MENU_OWNER && entryLine == OWNER_COMMENT ) ||
		( menu == MENU_PARENTAL && entryLine == PARENTAL_ANSWER ))
	{
		u16 buf[11] = {0};
		int i, lineOffset = 0;
		
		for( i = 0; i < wcslen( entry->str.utf ) ; i+=10 )
		{
			MI_CpuCopy( &(entry->str.utf[i]), buf, 20 );
			PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET * ( drawOffset + lineOffset++), txtColor, buf );
		}
		
		// 項目が空文字しかないときでも1行表示だけはされるようにする
		entry->numLines = lineOffset > 0 ? lineOffset : 1;
		
		return;
	}


	//////////////////////////
	// 通常の項目描画はここ //
	//////////////////////////
	
	if( entry->isAligned )
	{
		printData( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset, txtColor, entry );
	}
	else
	{
		// 項目名が長くて字下げが必要な場合
		printData( VALUE_LEFT, VALUE_UP + LINE_OFFSET* ( drawOffset + 1 ), txtColor, entry );
	}
}	

void drawRegister( int menu, int selected )
{
	// SCFGレジスタのバイナリをサブ画面に描画する
	int selectRegSize = 1;
	int selectBitNum;
	
	if( menu == MENU_SCFG_ARM7 && selected < SCFG_ARM7_MI_SC1_CDET)
	{
		// ARM7の前半4つ
		u32 regRom = 0;
		u16 regClk = 0;
		u16 regJtag = 0;
		u32 regExt = 0;
		selectBitNum = gRegisterIdx[0][selected];
		
		regRom = MI_LoadLE32( &gArm7SCFGReg[DISP_REG_ROM_OFFSET - 0x4000] );
		regClk = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_CLK_OFFSET - 0x4000] );
		regJtag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_JTAG_OFFSET - 0x4000] );
		regExt = MI_LoadLE32( &gArm7SCFGReg[DISP_REG_EXT_OFFSET - 0x4000] );
		
		if( selected == SCFG_ARM7_EXT_PS )
		{
			selectRegSize = 2;
		}
		
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP, TXT_COLOR_BLACK, s_strARM7RegisterName[0] );
		printBinary32( REGISTER_DATA_LEFT, REGISTER_DATA_UP, regRom, selectBitNum, selectRegSize);
		
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 2*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[1] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 2*LINE_OFFSET, regClk, selectBitNum - 32, selectRegSize );
		
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 3*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[2] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 3*LINE_OFFSET, regJtag, selectBitNum - 48, selectRegSize );
		
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 4*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[3] );
		printBinary32( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 4*LINE_OFFSET, regExt, selectBitNum - 64, selectRegSize );
	}
	else if( menu == MENU_SCFG_ARM7 && selected >= SCFG_ARM7_MI_SC1_CDET )
	{
		// SCFGの後半4つ
		u32 regMc1 = MI_LoadLE32( &gArm7SCFGReg[DISP_REG_MC_OFFSET - 0x4000] );
		u16 regMc2 = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC2_OFFSET - 0x4000] );
		u16 regOldWL = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_WL_OFFSET - 0x4000] );
		u16 regOption = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_OP_OFFSET - 0x4000] );
		selectBitNum = gRegisterIdx[0][selected];
		
		if( selected == SCFG_ARM7_MI_SC1_MODE ||
			selected == SCFG_ARM7_MI_SC2_MODE ||
			selected == SCFG_ARM7_OP_FORM ||
			selected == SCFG_ARM7_OP_APP 	)
		{
				selectRegSize = 2;
		}
		else if( selected == SCFG_ARM7_MI_CC || selected == SCFG_ARM7_MI_CA )
		{
			selectRegSize = 16;
		}
		
		// 描画				
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP, TXT_COLOR_BLACK, s_strARM7RegisterName[4] );
		printBinary32( REGISTER_DATA_LEFT, REGISTER_DATA_UP, regMc1, selectBitNum - 96, selectRegSize );

		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 2*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[5]);
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 2*LINE_OFFSET, regMc2, selectBitNum - 128, selectRegSize );
		
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 3*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[6] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 3*LINE_OFFSET, regOldWL, selectBitNum - 144, selectRegSize );
		
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 4*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[7] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 4*LINE_OFFSET, regOption, selectBitNum - 160, selectRegSize );
	
	}
	else if( menu == MENU_SCFG_ARM9 )
	{
		// ARM9側のSCFGレジスタセット
		u16 regRom = MI_LoadLE16( (void*) (HW_IOREG + REG_A9ROM_OFFSET) );
		u16 regClk = MI_LoadLE16( (void*) (HW_IOREG + REG_CLK_OFFSET) );
		u16 regRst = MI_LoadLE16( (void*) (HW_IOREG + REG_RST_OFFSET) );
		u32 regExt = MI_LoadLE32( (void*) (HW_IOREG + REG_EXT_OFFSET) );
		u16 regMc  = MI_LoadLE16( (void*) (HW_IOREG + REG_MC_OFFSET ) );
		selectBitNum = gRegisterIdx[1][selected];

		if( selected == SCFG_ARM9_EXT_PS)
		{
			selectRegSize = 2;
		}
		
		// 描画				
		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP, TXT_COLOR_BLACK, s_strARM9RegisterName[0] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP, regRom, selectBitNum, selectRegSize );

		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + LINE_OFFSET, TXT_COLOR_BLACK, s_strARM9RegisterName[1] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + LINE_OFFSET, regClk, selectBitNum - 16, selectRegSize );

		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 2*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM9RegisterName[2] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 2*LINE_OFFSET, regRst, selectBitNum - 32, selectRegSize );

		PrintfSJISSub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 3*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM9RegisterName[3] );
		printBinary32( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 3*LINE_OFFSET, regExt, selectBitNum - 48, selectRegSize );
		
	}
}

void printBinary32( int x, int y, u32 value, int selected, int selectSize )
{
	printBinary16( x, y, (u16)(value >> 16), selected - 16, selectSize );
	printBinary16( x, y + LINE_OFFSET, (u16)(value & 0xFFFF), selected, selectSize );
}

void printBinary16( int x, int y, u16 value, int selected, int selectSize )
{
	// 16ビットのバイナリをディスプレイに描画
	
	int i;
	int bitNum;
	u16 buf[BINARY_BUF] = {0};	// 16bit + 1 space + 1 EOS
	u16 mask = 0x8000;

	for( i = 0; i < BINARY_BUF; i++)
	{
		if( i%9 == 8 )
		{
			buf[i] = L'　';
			continue;
		}
		
		if( value & mask )
		{
			buf[i] = L'１';
		}
		else
		{
			buf[i] = L'０';
		}
		
		mask = (u16)( mask >> 1 );
	}
	
	buf[BINARY_BUF] = L'\0';

	PutStringUTF16Sub( x, y, TXT_COLOR_BLACK, buf );
	
	if( selected >= 16 || selected < 0 )
	{
		// 描画レジスタ内に選択項目に対応するビットがなければこれで終わり
		return;
	}
	
	// マスクをリセット
	mask = 0x8000;
	
	// 今回の描画レジスタ内に選択項目に対応するビットがあるとき
	// そのビットだけ赤字で表示する
	for( i = 0, bitNum = 0; bitNum < 16; i++ )
	{
		if( i == 8 )
		{
			// 8ビット、8ビットの間の空白
			buf[i] = L'　';
			continue;
		}
		
		if( selected <= 15 - bitNum && 15 - bitNum < selected + selectSize )
		{
			buf[i] = ( value & ( mask >> bitNum )) ? L'１': L'０' ;
		}
		else
		{
			buf[i] = L'　';
		}
		
		bitNum++;
	}
	
	PutStringUTF16Sub( x, y, TXT_COLOR_RED, buf );
}

void drawChangeMode( DispInfoEntry *entry,  int changeLine )
// 値変更画面の描画
{
	int i;
	
	// 項目名
	PrintfSJIS( CHANGE_ITEMNAME_LEFT, CHANGE_ITEMNAME_UP, TXT_COLOR_RED, entry->kind );
	
	if( entry->kindNameList != NULL )
	{
		// BOOL値で変更設定する項目の場合は値一覧を選択肢として表示
		PrintfSJIS( CHANGE_ITEMNAME_LEFT, CHANGE_NOW_SETTING_UP, TXT_COLOR_BLACK, "Now Setting: %s", entry->kindNameList[entry->iValue] );
		
		for( i = 0; i < entry->numKindName; i++ )
		{
			int kindColor = TXT_COLOR_BLACK;
			
			if( i == changeLine )
			{
				kindColor = TXT_COLOR_GREEN;
				PutStringUTF16( CHANGE_ALLOW_LEFT, CHANGE_VALUE_UP + i*LINE_OFFSET, TXT_COLOR_BLACK, (const u16 *)L"→" );
			}
			
			PrintfSJIS( CHANGE_VALUE_LEFT, CHANGE_VALUE_UP + i*LINE_OFFSET, kindColor, entry->kindNameList[i] );
		}
	}
	
}

void drawExecuteMode( const char *command, int changeLine, u8 mode )
{
	int i;
	
	// 項目名
	PrintfSJIS( CHANGE_ITEMNAME_LEFT, CHANGE_ITEMNAME_UP, TXT_COLOR_RED, command );
	
	for( i = 0; i < 2 ; i++ )
	{
		int kindColor = TXT_COLOR_BLACK;
		
		if( i == changeLine )
		{
			kindColor = TXT_COLOR_GREEN;
			PutStringUTF16( CHANGE_ALLOW_LEFT, CHANGE_VALUE_UP + i*LINE_OFFSET, TXT_COLOR_BLACK, (const u16 *)L"→" );
		}
		
		PrintfSJIS( CHANGE_VALUE_LEFT, CHANGE_VALUE_UP + i*LINE_OFFSET, kindColor, s_strOK[i] );
	}

	if( mode & MODE_RESULT_DISPLAY_MASK )
	{
		PrintfSJIS( RESULT_LEFT, RESULT_UP, TXT_COLOR_BLACK, s_strResult[ (mode & MODE_RESULT_MASK) >> MODE_RESULT_SHIFT ] );
	}
	
}	

void drawVersion( int idx, int drawLine ,int selected )
{
	int kindColor = TXT_COLOR_BLACK;
	u16 value = gContentsVersion[idx];
				
	if( idx == selected)
	{
		// 選択項目はいろかえる
		kindColor = TXT_COLOR_GREEN;
		PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*drawLine, TXT_COLOR_BLACK, (const u16 *)L"→");
	}
	
	
	// 項目名
	// gContentsTitle[i]は頭4バイトがイニシャルコードのビッグエンディアン記述、
	// お尻4バイトがアプリのフラグ
	{
		u64 id = gContentsTitle[idx];
		char buf[5] = {0};
		int i;

		for(i = 3; 0<=i; i-- )
		{
			buf[i] = (char)(id & 0xFF) ;
			id >>= 8;
		}
		
		PrintfSJIS( KIND_LEFT, KIND_UP + LINE_OFFSET*drawLine , kindColor, "%s", buf );
	}
	
	// 値	
	PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLine , TXT_COLOR_BLACK, "%x.%x", value >> 8, value & 0xFF );
}

void drawFontInfo( int idx, int drawLine, int selected )
{
	int color = TXT_COLOR_BLACK;
	int fontIdx = idx / NUM_FONT_INFO;
	int dataType = idx % NUM_FONT_INFO;
	
	static const char* s_strFontDataType[] = {
		"name",
		"size",
		"hash"
	};
	
	// 種類名
	if( idx == selected)
	{
		color = TXT_COLOR_GREEN;
		PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*drawLine, TXT_COLOR_BLACK, (const u16 *)L"→");
	}
	
	PrintfSJIS( KIND_LEFT, KIND_UP + LINE_OFFSET*drawLine , color, "font%d %s", fontIdx, s_strFontDataType[dataType] );
	
	// 値	
	switch( dataType )
	{
		case 0:
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLine , TXT_COLOR_BLACK, (char*)gFontInfo[fontIdx].name );
		break;
		
		case 1:
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLine , TXT_COLOR_BLACK, "%d byte", gFontInfo[fontIdx].size );
		break;
		
		case 2:
		if( gFontInfo[fontIdx].isHashOK )
		{
			color = TXT_COLOR_BLACK;
		}
		else
		{
			color = TXT_COLOR_RED;
		}
		
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLine , color, s_strCorrect[ gFontInfo[fontIdx].isHashOK ] );
		break;
	}
}

void drawMenu( int menu, int line, int changeLine, u8 mode )
// 情報一覧を描画する
{
	int lineNum = 0;
	int i=0;
	
	drawHeader( menu, line );	
	drawRegister( menu, line );	
	
	if( mode & MODE_CHANGE_MASK )
	{
		// 変更モード画面の描画
		drawChangeMode( &gAllInfo[menu][line], changeLine );
		return;
	}
	
	if( mode & MODE_EXECUTE_MASK )
	{
		drawExecuteMode( s_strMetaMenu[MENU_ROOT][line] , changeLine, mode );
		return;
	}
	
	// 描画位置の調整
	if( line - gDrawIdx[menu] < SCROLL_MARGIN )
	{
		gDrawIdx[menu] = line - SCROLL_MARGIN >= 0 ? line - SCROLL_MARGIN : 0;
	}
	else if( countLinesDown(menu, line) < SCROLL_MARGIN )
	{
		calibrateDrawIdx( menu, line );
	}
	
	for( i = gDrawIdx[menu] ; i < s_numMenu[menu] && lineNum < DISP_NUM_LINES ; i++ )
	{		
		// 可変長項目なバージョン情報だけ特例処理
		if( menu == MENU_VERSION && VERSION_OTHER <= i )
		{
			drawVersion( i - VERSION_OTHER, lineNum++, line - VERSION_OTHER );
			continue;
		}
		
		if( menu == MENU_FONT && FONT_INFO <= i )
		{
			drawFontInfo( i - FONT_INFO, lineNum++, line - FONT_INFO );
			continue;
		}

		// 項目名の描画
		printKindName( menu, i, lineNum, line );
	
		// 値の描画
		if( menu == MENU_SCFG_ARM7 && !gSelectedARM7SCFGReg )
		{
			// ARM7のSCFGで共有領域側の値を表示してるときはオフセットを加える
			printValue( menu, i, lineNum, &gAllInfo[menu][i + SCFG_ARM7_SHARED_OFFSET] );
		}
		else if( menu != MENU_ROOT )
		{
			printValue( menu, i, lineNum, &gAllInfo[menu][i] );
		}
		
		// 描画オフセットの更新
		lineNum += menu != MENU_ROOT ? gAllInfo[menu][i].numLines : 1;
	}

	/*
	// スクロールバーとか出す？	
	// 全体の行数を把握
	if( gMenuLineSize[menu] == 0 )
	{
		gMenuLineSize[menu] = countMenuLine(menu);
	}
	*/
}

int countLinesDown( int menu, int idx )
// 選択項目より下側に何項目表示されているかカウント
{
	int i;
	int lines = 0;
	
	for( i = gDrawIdx[menu]; lines < DISP_NUM_LINES && i < s_numMenu[menu] ; i++ )
	{
		lines +=	( menu == MENU_VERSION && VERSION_OTHER <= i )  ||
					( menu == MENU_FONT && FONT_INFO <= i )  ||
					( menu == MENU_ROOT ) ? 
					1 : gAllInfo[menu][i].numLines;
	}
	
	return (i-1) - idx;
}

void calibrateDrawIdx( int menu, int idx )
// 折り返し項目とかがあると描画位置がずれるので
// 下側に常に2項目表示されるように補正を行う
{
	int i;
	int lines = 0;
	
	// まずは自分と下2項目の行数を探索
	for( i = 0; i <= 2 && i + idx < s_numMenu[menu] ; i++ )
	{
		lines +=	( menu == MENU_VERSION && VERSION_OTHER <= i+idx ) ||
					( menu == MENU_FONT && FONT_INFO <= i+idx ) || 
					( menu == MENU_ROOT ) ?
					1 : gAllInfo[menu][i + idx].numLines;
	}
	
	// 自分より上方向へ探索
	for( i = 1; 0 <= idx - i && lines < DISP_NUM_LINES ; i++)
	{
		lines +=	( menu == MENU_VERSION && VERSION_OTHER <= i+idx ) ||
					( menu == MENU_FONT && FONT_INFO <= i+idx ) ||
					( menu == MENU_ROOT ) ?
					1 : gAllInfo[menu][idx - i].numLines;
	}
	
	// ループが一回余計に回る
	i--;
	
	if( lines < DISP_NUM_LINES )
	{
		// idx - iがゼロより小さくなった場合
		gDrawIdx[menu] = 0;
	}
	else if( lines == DISP_NUM_LINES )
	{
		// 表示行数ちょうどはそのまま
		gDrawIdx[menu] = idx - i;
	}
	else if( lines == DISP_NUM_LINES + 1  && gAllInfo[menu][ idx - i ].numLines != 1)
	{
		// 1行溢れてて、かつ一番上の項目が複数行描画だった場合はそのまま
		gDrawIdx[menu] = idx - i;
	}
	else
	{
		// それ以外の場合は一番上の行を外す
		gDrawIdx[menu] = idx - i + 1;
	}
	
}

void switchViewMode( void )
// ARM7SCFGのレジスタデータと共有領域データの表示内容を切り替える
{
	gSelectedARM7SCFGReg = !gSelectedARM7SCFGReg;
}

/*
void drawKindName( int menu, int page, int line )
// 項目名描画関数
{
	int linenum = 0;
	int offset = 0;

	if( menu == MENU_ROOT )
	{
		// ルートメニューにはインデクスデータがないので、間隔をあけないことを前提に羅列するだけ
		for(linenum = 0; linenum < ROOTMENU_SIZE ; linenum++)
		{
			if( linenum == line )
			{
				// 選択中の項目は表示色かえて矢印表示
				PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, (const u16 *)L"→" );
				PutStringUTF16( KIND_LEFT, KIND_UP + LINE_OFFSET*linenum, TXT_COLOR_GREEN, s_strMetaMenu[MENU_ROOT][linenum]) ;
			}
			else
			{
				PutStringUTF16( KIND_LEFT, KIND_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, s_strMetaMenu[MENU_ROOT][linenum]) ;
			}
		}
		
	}
	else
	{
		for(linenum = s_pageOffset[menu][page] ; linenum < s_pageOffset[menu][page+1] ; linenum++)
		{
			if( linenum == s_pageOffset[menu][page] + line )
			{
				// 選択中の項目は表示色かえて矢印表示
				PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET * gMenuKindOffset[menu][linenum], 
					TXT_COLOR_BLACK, (const u16 *)L"→" );
				PutStringUTF16( KIND_LEFT, KIND_UP + LINE_OFFSET * gMenuKindOffset[menu][linenum],
					TXT_COLOR_GREEN, s_strMetaMenu[menu][linenum]) ;
			}
			else
			{
				PutStringUTF16( KIND_LEFT, KIND_UP + LINE_OFFSET * gMenuKindOffset[menu][linenum], 
					TXT_COLOR_BLACK, s_strMetaMenu[menu][linenum]) ;
			}
		}
	}
}

void drawOwnerMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;	
		
	// utf16で描画するものだけ分ける
	if( valueIdx == OWNER_NICKNAME )
	{			
		kindOffset[valueIdx] = drawKindOffset;
		PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, gUserName) ;
	}
	else if( valueIdx == OWNER_COMMENT )
	{
		kindOffset[valueIdx] = drawKindOffset;
		PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, gUserComment) ;
	}
	else if( valueIdx == OWNER_COLOR )
	{
		kindOffset[valueIdx] = drawKindOffset;
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_UCOLOR_G5, "■" );
		PrintfSJIS( VALUE_LEFT + LINE_OFFSET , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
		
	}
	else
	{
		// sjisで描画するもの
		kindOffset[valueIdx] = drawKindOffset;
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	}
	
}

void drawParentalMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;	
	
	// utf16で描画するものだけ分ける
	if(valueIdx == PARENTAL_ANSWER)
	{			
		kindOffset[valueIdx] = drawKindOffset;
		PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, gSecretAnswer) ;
	}
	else
	{
		// sjisで描画するもの
		kindOffset[valueIdx] = drawKindOffset;
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	}
}	


void drawNormalHWMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;

	// 描画してついでにインデクス差分データをいれる
	kindOffset[valueIdx] = drawKindOffset;
	PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );

}


void drawSecureHWMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;
	
	// 複数行必要な項目があると描画位置がずれるため
	// 描画オフセットを調整する処理が必要
	if( valueIdx >= SECURE_HW_FORCE_DISABLE && page == getPageNum( SECURE_HW_FORCE_DISABLE, pageOffset ) )
	{
		drawLineOffset++;
	}		
	
	if( valueIdx > SECURE_HW_UNIQUE_ID && page == getPageNum( SECURE_HW_UNIQUE_ID, pageOffset ) )
	{
		drawLineOffset += OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 / UNIQUE_BUF;
	}
	
	// 同じように項目名のオフセットも調整
	if( valueIdx > SECURE_HW_FORCE_DISABLE && page == getPageNum( SECURE_HW_FORCE_DISABLE, pageOffset ) )
	{
		drawKindOffset++;
	}
	
	if( valueIdx > SECURE_HW_UNIQUE_ID && page == getPageNum( SECURE_HW_UNIQUE_ID,  pageOffset ) )
	{
		drawKindOffset += OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 / UNIQUE_BUF;
	}
	
	
	// 描画する、ついでにインデクスデータも入れる
	if( valueIdx == SECURE_HW_UNIQUE_ID )
	{
		kindOffset[valueIdx] = drawKindOffset;
		printUniqueID(drawLineOffset);
	}
	else
	{
		kindOffset[valueIdx] = drawKindOffset;
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	}
}	

void drawSCFGARM7Menu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	kindOffset[valueIdx] = linenum;
}

void drawSCFGARM9Menu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum, drawKindOffset = linenum;
	
	PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	kindOffset[valueIdx] = drawKindOffset;	
	
}

void drawVersionMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum, drawKindOffset = linenum;

	PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	kindOffset[valueIdx] = drawKindOffset;

}
*/

void printUniqueID( int drawLineOffset, char *uniqueId)
// ユニークIDを整形して出力
{
	char buf[UNIQUE_BUF+1];
	int i;
	
	for( i=0; i * UNIQUE_BUF < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 ; i++)
	{
		strncpy(buf, &uniqueId[i * UNIQUE_BUF], UNIQUE_BUF);
		buf[UNIQUE_BUF] = '\0';
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET * (drawLineOffset + i), TXT_COLOR_BLACK, buf );
	}
	
}

int getPageNum( int valueIdx, const int* pageOffset )
// valueIdxが何ページ目にあるかをpageOffsetから調べる
{
	int i;
	
	for( i=0; i < MAXITEM - 1; i++ )
	{
		if( valueIdx > pageOffset[i] && valueIdx < pageOffset[i+1] )
		{
			return i;
		}
	}
	
	return -1;
}

const static int gRegisterIdx[2][SCFG_ARM7_MENU_SIZE] = {
	// ARM7, ARM9のSCFGの各項目がレジスタにおいて対応しているのかをリストで記述

	// ARM7 side
	{
		DISP_REG_SCFG_ROM_ARM9SEL_SHIFT,
		DISP_REG_SCFG_ROM_ARM9RSEL_SHIFT,
		DISP_REG_SCFG_ROM_ARM7SEL_SHIFT,
		DISP_REG_SCFG_ROM_ARM7RSEL_SHIFT,
		DISP_REG_SCFG_ROM_ARM7FUSE_SHIFT,
		DISP_REG_SCFG_ROM_ROMWE_SHIFT,
		
		DISP_REG_SCFG_CLK_SD1HCLK_SHIFT + 32,
		DISP_REG_SCFG_CLK_SD2HCLK_SHIFT + 32,
		DISP_REG_SCFG_CLK_AESHCLK_SHIFT + 32,
		DISP_REG_SCFG_CLK_WRAMHCLK_SHIFT + 32,
		DISP_REG_SCFG_CLK_SNDMCLK_SHIFT + 32,
		
		DISP_REG_SCFG_JTAG_ARM7SEL_SHIFT + 48,
		DISP_REG_SCFG_JTAG_CPUJE_SHIFT + 48,
		DISP_REG_SCFG_JTAG_DSPJE_SHIFT + 48,
		
		DISP_REG_SCFG_EXT_DMA_SHIFT + 64,
		DISP_REG_SCFG_EXT_SDMA_SHIFT + 64,
		DISP_REG_SCFG_EXT_SND_SHIFT + 64,
		DISP_REG_SCFG_EXT_MC_SHIFT + 64,
		DISP_REG_SCFG_EXT_INTC_SHIFT + 64,
		DISP_REG_SCFG_EXT_SPI_SHIFT + 64,
		DISP_REG_SCFG_EXT_DSEL_SHIFT + 64,
		DISP_REG_SCFG_EXT_SIO_SHIFT + 64,
		DISP_REG_SCFG_EXT_LCDC_SHIFT + 64,
		DISP_REG_SCFG_EXT_VRAM_SHIFT + 64,
		DISP_REG_SCFG_EXT_PSRAM_SHIFT + 64,
		DISP_REG_SCFG_EXT_DMAC_SHIFT + 64,
		DISP_REG_SCFG_EXT_AES_SHIFT + 64,
		DISP_REG_SCFG_EXT_SD1_SHIFT + 64,
		DISP_REG_SCFG_EXT_SD2_SHIFT + 64,
		DISP_REG_SCFG_EXT_MIC_SHIFT + 64,
		DISP_REG_SCFG_EXT_I2S_SHIFT + 64,
		DISP_REG_SCFG_EXT_I2C_SHIFT + 64,
		DISP_REG_SCFG_EXT_GPIO_SHIFT + 64,
		DISP_REG_SCFG_EXT_MC_B_SHIFT + 64,
		DISP_REG_SCFG_EXT_WRAM_SHIFT + 64,
		DISP_REG_SCFG_EXT_PUENABLE_SHIFT + 64,
		DISP_REG_SCFG_EXT_CFG_SHIFT + 64,
		
		DISP_REG_MI_MC1_SL1_CDET_SHIFT + 96,
		DISP_REG_MI_MC1_SL1_M0_SHIFT + 96,
		DISP_REG_MI_MC1_SL2_CDET_SHIFT + 96,
		DISP_REG_MI_MC1_SL2_M0_SHIFT + 96,
		DISP_REG_MI_MC1_SWP_SHIFT + 96,
		DISP_REG_MI_MC1_CC_SHIFT + 96,
		
		DISP_REG_MI_MC2_CA_SHIFT + 128,
		
		DISP_REG_SCFG_WL_OFFB_SHIFT + 144,
		
		DISP_REG_SCFG_OP_OP0_SHIFT + 160,
		DISP_REG_SCFG_OP_OP0_SHIFT + 160	
	},
	
	// ARM9 side
	{
		REG_SCFG_A9ROM_SEC_SHIFT,
		REG_SCFG_A9ROM_RSEL_SHIFT,
		
		REG_SCFG_CLK_CPUSPD_SHIFT + 16,
		REG_SCFG_CLK_DSPHCLK_SHIFT + 16,
		REG_SCFG_CLK_CAMHCLK_SHIFT + 16,
		REG_SCFG_CLK_WRAMHCLK_SHIFT + 16,
		REG_SCFG_CLK_CAMCKI_SHIFT  + 16,
		
		REG_SCFG_RST_DSPRSTB_SHIFT + 32,
		
		REG_SCFG_EXT_DMA_SHIFT + 48,
		REG_SCFG_EXT_GEO_SHIFT + 48,
		REG_SCFG_EXT_REN_SHIFT + 48,
		REG_SCFG_EXT_G2DE_SHIFT + 48,
		REG_SCFG_EXT_DIV_SHIFT + 48,
		REG_SCFG_EXT_MC_SHIFT + 48,
		REG_SCFG_EXT_INTC_SHIFT + 48,
		REG_SCFG_EXT_LCDC_SHIFT + 48,
		REG_SCFG_EXT_VRAM_SHIFT + 48,
		REG_SCFG_EXT_PSRAM_SHIFT + 48,
		REG_SCFG_EXT_DMAC_SHIFT + 48, 
		REG_SCFG_EXT_CAM_SHIFT + 48,
		REG_SCFG_EXT_DSP_SHIFT + 48,
		REG_SCFG_EXT_MC_B_SHIFT + 48,
		REG_SCFG_EXT_WRAM_SHIFT + 48,
		REG_SCFG_EXT_CFG_SHIFT + 48,
		
	}
};