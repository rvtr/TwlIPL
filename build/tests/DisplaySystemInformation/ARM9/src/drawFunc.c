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
#include "drawFunc.h"
#include "viewSystemInfo.h"
#include "misc.h"
#include "strResource.h"
#include "myIoreg_SCFG.h"



// 描画関連
#define HEADER_UP 0
#define HEADER_LEFT 10
#define PAGE_LEFT 160
#define FOOTER_UP 140
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

#define SCROLL_MARGIN 2 							// 画面端何行でスクロールするか

#define UNIQUE_BUF 12
#define BINARY_BUF 18

/* global variables -------------------- */

static int gDrawIdx[ROOTMENU_SIZE];			// 今何項目目から下を描画しているのか
static int gMenuLineSize[ROOTMENU_SIZE] = {};// 各メニューの全体行数
static const int gRegisterIdx[2][SCFG_ARM7_MENU_SIZE];
int gMenuKindOffset[ROOTMENU_SIZE][MAXITEM];

/* function prototypes -------------------- */

void printData( int x, int y, int color, DispInfoEntry *entry );
void printValue( int menu,int entryLine, int drawOffset, DispInfoEntry *entry );
void printKindName( int menu, int entryLine, int drawOffset, int selected );
void printBinary16( int x, int y, u16 value, int selected, int selectSize );
void printBinary32( int x, int y, u32 value, int selected, int selectSize );
void drawRegister( int menu, int selected );
void drawOwnerMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void drawParentalMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void drawNormalHWMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void drawSecureHWMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void drawSCFGARM7Menu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void drawSCFGARM9Menu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void drawVersionMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset);
void printUniqueID( int drawLineOffset, char *uniqueId );
int getPageNum( int valueIdx, const int* pageOffset );
int countMenuLine( int menu );

void drawHeader( int menu, int line)
// 画面端に簡単な情報を表示する
{
	u16 buf[256];
	
	PutStringUTF16Sub( HEADER_LEFT, HEADER_UP, TXT_COLOR_RED, (const u16 *)L"DisplaySystemInfo");
	if( menu != MENU_ROOT )
	{

		swprintf(buf, 256, L"Root > %s", s_strMenuName[menu] );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + LINE_OFFSET, TXT_COLOR_BLUE, buf );	
		
		swprintf(buf, 256, L"%d / %d", line+1 , s_numMenu[menu] );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + 2*LINE_OFFSET, TXT_COLOR_BLUE, buf );	
	}
	else
	{
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + LINE_OFFSET, TXT_COLOR_BLUE, (const u16 *)L"Root" );
		swprintf(buf, 256, L"%d / %d", line+1 , ROOTMENU_SIZE );
		PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP + 2*LINE_OFFSET, TXT_COLOR_BLUE, buf );			
	}
	
	PutStringUTF16Sub( FOOTER_LEFT, FOOTER_UP, TXT_COLOR_BLUE, (const u16 *)L" A: Decide   B: Back ");
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
	// 特殊描画を行う必要がある場合の処理
	if( menu == MENU_OWNER && entryLine == OWNER_COLOR )
	{
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawOffset, TXT_UCOLOR_G0 + entry->iValue , "■" );
		printData( VALUE_LEFT + LINE_OFFSET , VALUE_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, entry );
		return;
	}
	
	if( menu == MENU_SECURE_HW && entryLine == SECURE_HW_UNIQUE_ID )
	{
		printUniqueID( drawOffset, entry->str.sjis );
		entry->numLines = 4;
		return;
	}
	
	if( menu == MENU_PARENTAL && entryLine == PARENTAL_PASSWORD )
	{
		PrintfSJIS( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, "%04d", entry->iValue );
	}
	
	
	
	
	// 通常の値の描画
	if( entry->isAligned )
	{
		printData( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, entry );
	}
	else
	{
		// 項目名が長くて字下げが必要な場合
		printData( VALUE_LEFT, VALUE_UP + LINE_OFFSET* ( drawOffset + 1 ), TXT_COLOR_BLACK, entry );
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
		
		/*
		regRom =(u32)(
						p[SCFG_ARM7_ROM_ARM9_SEC].iValue << DISP_REG_SCFG_ROM_ARM9SEL_SHIFT 	|
						p[SCFG_ARM7_ROM_ARM9_RSEL].iValue << DISP_REG_SCFG_ROM_ARM9RSEL_SHIFT	|
						p[SCFG_ARM7_ROM_ARM9_SEC].iValue << DISP_REG_SCFG_ROM_ARM7SEL_SHIFT		|
						p[SCFG_ARM7_ROM_ARM7_RSEL].iValue << DISP_REG_SCFG_ROM_ARM7RSEL_SHIFT	|
						p[SCFG_ARM7_ROM_ARM7_FUSE].iValue << DISP_REG_SCFG_ROM_ARM7FUSE_SHIFT	|
						p[SCFG_ARM7_ROM_WE].iValue << DISP_REG_SCFG_ROM_ROMWE_SHIFT
					);

		regClk =(u16)(
					 	p[SCFG_ARM7_CLK_SD1].iValue << DISP_REG_SCFG_CLK_SD1HCLK_SHIFT	|
						p[SCFG_ARM7_CLK_SD2].iValue << DISP_REG_SCFG_CLK_SD2HCLK_SHIFT	|
						p[SCFG_ARM7_CLK_AES].iValue << DISP_REG_SCFG_CLK_AESHCLK_SHIFT	|
						p[SCFG_ARM7_CLK_WRAM].iValue << DISP_REG_SCFG_CLK_WRAMHCLK_SHIFT|
						p[SCFG_ARM7_CLK_SND].iValue << DISP_REG_SCFG_CLK_SNDMCLK_SHIFT
					);
					
		regJtag =(u16)(
						p[SCFG_ARM7_JTAG_A7].iValue << DISP_REG_SCFG_JTAG_ARM7SEL_SHIFT	|
						p[SCFG_ARM7_JTAG_CPU].iValue << DISP_REG_SCFG_JTAG_CPUJE_SHIFT	|
						p[SCFG_ARM7_JTAG_DSP].iValue << DISP_REG_SCFG_JTAG_DSPJE_SHIFT
					);
					
		regExt =(u32)(
						p[SCFG_ARM7_EXT_DMA].iValue << DISP_REG_SCFG_EXT_DMA_SHIFT	|
						p[SCFG_ARM7_EXT_SDMA].iValue << DISP_REG_SCFG_EXT_SDMA_SHIFT	|
						p[SCFG_ARM7_EXT_SND].iValue << DISP_REG_SCFG_EXT_SND_SHIFT	|
						p[SCFG_ARM7_EXT_MC].iValue << DISP_REG_SCFG_EXT_MC_SHIFT	|
						p[SCFG_ARM7_EXT_INTC].iValue << DISP_REG_SCFG_EXT_INTC_SHIFT	|
						p[SCFG_ARM7_EXT_SPI].iValue << DISP_REG_SCFG_EXT_SPI_SHIFT	|
						p[SCFG_ARM7_EXT_DSEL].iValue << DISP_REG_SCFG_EXT_DSEL_SHIFT	|
						p[SCFG_ARM7_EXT_SIO].iValue << DISP_REG_SCFG_EXT_SIO_SHIFT	|
						p[SCFG_ARM7_EXT_LCDC].iValue << DISP_REG_SCFG_EXT_LCDC_SHIFT	|
						p[SCFG_ARM7_EXT_VRAM].iValue << DISP_REG_SCFG_EXT_VRAM_SHIFT	|
						p[SCFG_ARM7_EXT_PS].iValue << DISP_REG_SCFG_EXT_PSRAM_SHIFT	|
						p[SCFG_ARM7_EXT_DMAC].iValue << DISP_REG_SCFG_EXT_DMAC_SHIFT	|
						p[SCFG_ARM7_EXT_AES].iValue << DISP_REG_SCFG_EXT_AES_SHIFT	|
						p[SCFG_ARM7_EXT_SD1].iValue << DISP_REG_SCFG_EXT_SD1_SHIFT	|
						p[SCFG_ARM7_EXT_SD2].iValue << DISP_REG_SCFG_EXT_SD2_SHIFT	|
						p[SCFG_ARM7_EXT_MIC].iValue << DISP_REG_SCFG_EXT_MIC_SHIFT	|
						p[SCFG_ARM7_EXT_I2S].iValue << DISP_REG_SCFG_EXT_I2S_SHIFT	|
						p[SCFG_ARM7_EXT_I2C].iValue << DISP_REG_SCFG_EXT_I2C_SHIFT	|
						p[SCFG_ARM7_EXT_GPIO].iValue << DISP_REG_SCFG_EXT_GPIO_SHIFT	|
						p[SCFG_ARM7_EXT_MCB].iValue << DISP_REG_SCFG_EXT_MC_B_SHIFT	|
						p[SCFG_ARM7_EXT_WRAM].iValue << DISP_REG_SCFG_EXT_WRAM_SHIFT	|
						p[SCFG_ARM7_EXT_PU].iValue << DISP_REG_SCFG_EXT_PUENABLE_SHIFT	|
						p[SCFG_ARM7_EXT_CFG].iValue << DISP_REG_SCFG_EXT_CFG_SHIFT
					);
		*/

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
		// ARM9側のSCFGレジスタ
		u16 regRom = MI_LoadLE16( (void*) (HW_IOREG + REG_A9ROM_OFFSET) );
		u16 regClk = MI_LoadLE16( (void*) (HW_IOREG + REG_CLK_OFFSET) );
		u16 regRst = MI_LoadLE16( (void*) (HW_IOREG + REG_RST_OFFSET) );
		u32 regExt = MI_LoadLE32( (void*) (HW_IOREG + REG_EXT_OFFSET) );
		u16 regMc  = MI_LoadLE16( (void*) (HW_IOREG + REG_MC_OFFSET ) );
		selectBitNum = gRegisterIdx[1][selected];

		if( selected == SCFG_ARM9_PSRAM_BOUNDARY )
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

void drawMenu( int menu, int line )
// 情報一覧を描画する
{
	int lineNum = 0;
	int i=0;
	
	if( menu == MENU_ROOT )
	{
		// ルートメニューは値がないので項目名だけ描画して終わり
		for( i=0; i<ROOTMENU_SIZE; i++ )
		{
			printKindName( menu, i, i,line );
		}
		
		return;
	}
	
	if( line - gDrawIdx[menu] < SCROLL_MARGIN )
	{
		gDrawIdx[menu] = line - SCROLL_MARGIN >= 0 ? line - SCROLL_MARGIN : 0;
	}
	else if( gDrawIdx[menu] + DISP_NUM_LINES - line - 1 <= SCROLL_MARGIN )	
	{
		// 矢印が下に近すぎる場合のスクロール補正
		gDrawIdx[menu] = line + SCROLL_MARGIN < s_numMenu[menu] ?
			 line + SCROLL_MARGIN - DISP_NUM_LINES + 1 : s_numMenu[menu] - DISP_NUM_LINES ;
	}
	

	for( i = gDrawIdx[menu] ; i < s_numMenu[menu] && lineNum < DISP_NUM_LINES ; i++ )
	{
		printKindName( menu, i, lineNum, line );
		printValue( menu, i, lineNum, &gAllInfo[menu][i] );
		
		// 描画オフセットの更新
		lineNum += gAllInfo[menu][i].numLines;
	}

	drawRegister( menu, line );
	
	// 全体の行数を把握
	if( gMenuLineSize[menu] == 0 )
	{
		gMenuLineSize[menu] = countMenuLine(menu);
	}

	// スクロールバーとか出す
}



/*	
	for( linenum = 0; linenum < nowPageMaxLine; linenum++ )
	{
		drawMenuSub( menu, line, &gAllInfo[menu][0] );

		switch( menu ){
			case MENU_ROOT:
				break;
			case MENU_OWNER:
				drawOwnerMenu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
			case MENU_PARENTAL:
				drawParentalMenu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
			case MENU_NORMAL_HW:
				drawNormalHWMenu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
			case MENU_SECURE_HW:
				drawSecureHWMenu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
			case MENU_SCFG_ARM7:
				drawSCFGARM7Menu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
			case MENU_SCFG_ARM9:
				drawSCFGARM9Menu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
			case MENU_VERSION:
				drawVersionMenu( page, linenum, valueIdx, &gAllInfo[menu][0], &gMenuKindOffset[menu][0], &s_pageOffset[menu][0] );
				break;
		}	
*/



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