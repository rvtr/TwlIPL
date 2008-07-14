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
  $Rev:$
  $Author:$
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



// �`��֘A
#define HEADER_UP 0
#define HEADER_LEFT 10
#define PAGE_LEFT 160
#define FOOTER_UP 140
#define FOOTER_LEFT 10
#define ALLOW_LEFT 0

#define AST_LEFT 	( ALLOW_LEFT + LINE_OFFSET )	
#define KIND_UP 	10								// ���ږ��̏���W
#define KIND_LEFT 	( AST_LEFT + 10 )				// ���ږ��̍����W
#define VALUE_UP 	KIND_UP							// ���ڒl�̏���W
#define VALUE_LEFT 	140								// ���ڒl�̍����W
#define LINE_OFFSET 15								// 1�s���Ƃ̃I�t�Z�b�g
#define ROW_OFFSET	15 								// �������ڂ����ꂽ�Ƃ��̒i�g�p

#define REGISTER_NAME_LEFT	10
#define REGISTER_NAME_UP	15
#define REGISTER_DATA_LEFT	100
#define REGISTER_DATA_UP	15

#define SCROLL_MARGIN 2 							// ��ʒ[���s�ŃX�N���[�����邩

#define UNIQUE_BUF 12
#define BINARY_BUF 20

/* global variables -------------------- */

static int gDrawIdx[ROOTMENU_SIZE];			// �������ږڂ��牺��`�悵�Ă���̂�
static int gMenuLineSize[ROOTMENU_SIZE] = {};// �e���j���[�̑S�̍s��

int gMenuKindOffset[ROOTMENU_SIZE][MAXITEM];

/* function prototypes -------------------- */

void printData( int x, int y, int color, DispInfoEntry *entry );
void printValue( int menu,int entryLine, int drawOffset, DispInfoEntry *entry );
void printKindName( int menu, int entryLine, int drawOffset, int selected );
void printBinary16( int x, int y, u16 value );
void printBinary32( int x, int y, u32 value );
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
// ��ʒ[�ɊȒP�ȏ���\������
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
		// ���l�f�[�^�������炻�̂܂܃v�����g
		PrintfSJIS( x, y, color, "%d", entry->iValue );
	}
	else if( entry->isSjis )
	{
		// SJIS������
		PrintfSJIS( x, y, color, "%s", entry->str.sjis );
	}
	else
	{
		// UTF16������
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
	
	// ���ږ��`��
	if( entryLine == selected)
	{
		// �I�����ڂ͂��납����
		kindColor = TXT_COLOR_GREEN;
		PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, (const u16 *)L"��");
	}
	
	if( menu != MENU_ROOT && gAllInfo[menu][entryLine].changable )
	{
		PutStringUTF16( AST_LEFT, KIND_UP + LINE_OFFSET*drawOffset, kindColor, (const u16 *) L"*");
	}
	
	PrintfSJIS( KIND_LEFT, KIND_UP + LINE_OFFSET*drawOffset , kindColor, s_strMetaMenu[menu][entryLine] );

}	

void printValue( int menu,int entryLine, int drawOffset, DispInfoEntry *entry )
{
	// ����`����s���K�v������ꍇ�̏���
	if( menu == MENU_OWNER && entryLine == OWNER_COLOR )
	{
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawOffset, TXT_UCOLOR_G0 + entry->iValue , "��" );
		printData( VALUE_LEFT + LINE_OFFSET , VALUE_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, entry );
		return;
	}
	
	if( menu == MENU_SECURE_HW && entryLine == SECURE_HW_UNIQUE_ID )
	{
		printUniqueID( drawOffset, entry->str.sjis );
		entry->numLines = 4;
		return;
	}
	
	
	// �ʏ�̒l�̕`��
	if( entry->isAligned )
	{
		printData( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawOffset, TXT_COLOR_BLACK, entry );
	}
	else
	{
		// ���ږ��������Ď��������K�v�ȏꍇ
		printData( VALUE_LEFT, VALUE_UP + LINE_OFFSET* ( drawOffset + 1 ), TXT_COLOR_BLACK, entry );
	}
}	

void drawRegister( int menu, int selected )
{
	// SCFG���W�X�^�̃o�C�i�����T�u��ʂɕ`�悷��
	u32 regState = 0;
	DispInfoEntry *p;
	int drawOffset = 0;
	
	if( menu == MENU_SCFG_ARM7 && selected < SCFG_ARM7_MI_SC1_CDET)
	{
		// ARM7�̑O��4��
		u32 regRom = 0;
		u16 regClk = 0;
		u16 regJtag = 0;
		u32 regExt = 0;
		p = gAllInfo[MENU_SCFG_ARM7];
		
		
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
		
		PutStringUTF16Sub( REGISTER_NAME_LEFT, REGISTER_NAME_UP, TXT_COLOR_BLACK, s_strARM7RegisterName[0] );
		printBinary32( REGISTER_DATA_LEFT, REGISTER_DATA_UP, regRom );
		PutStringUTF16Sub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 2*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[1] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 2*LINE_OFFSET, regClk );
		PutStringUTF16Sub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 3*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[2] );
		printBinary16( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 3*LINE_OFFSET, regJtag );
		PutStringUTF16Sub( REGISTER_NAME_LEFT, REGISTER_NAME_UP + 4*LINE_OFFSET, TXT_COLOR_BLACK, s_strARM7RegisterName[3] );
		printBinary32( REGISTER_DATA_LEFT, REGISTER_DATA_UP + 4*LINE_OFFSET, regExt );
	}
	else if( menu == MENU_SCFG_ARM7 && selected >= SCFG_ARM7_MI_SC1_CDET )
	{
		// SCFG�̌㔼4��
		u32 regMcard1;
		u16 regMcard2;
		u16 regOldWL;
		u16 regOption;

	}
	else if( menu == MENU_SCFG_ARM9 )
	{
		
	}
}

void printBinary32( int x, int y, u32 value )
{
	printBinary16( x, y, (u16)(value >> 16) );
	printBinary16( x, y + LINE_OFFSET, (u16)(value & 0xFF) );
}

void printBinary16( int x, int y, u16 value )
{
	int i;
	u16 buf[BINARY_BUF] = {0};	// 16bit + 3 space + EOS
	u16 mask = 0x8000;

	for( i=0; i<BINARY_BUF; i++)
	{
		if( i%5 == 4 )
		{
			buf[i] = L'�@';
			continue;
		}
		
		if( value & mask )
		{
			buf[i] = L'1';
		}
		else
		{
			buf[i] = L'0';
		}
		
		mask = mask >> 1;
	}
	
	buf[20] = L'\0';

	PutStringUTF16Sub( x, y, TXT_COLOR_BLACK, buf );
}

void drawMenu( int menu, int line )
// ���ꗗ��`�悷��
{
	int lineNum = 0;
	int i=0;
	
	if( menu == MENU_ROOT )
	{
		// ���[�g���j���[�͒l���Ȃ��̂ō��ږ������`�悵�ďI���
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
		// ��󂪉��ɋ߂�����ꍇ�̃X�N���[���␳
		gDrawIdx[menu] = line + SCROLL_MARGIN < s_numMenu[menu] ?
			 line + SCROLL_MARGIN - DISP_NUM_LINES + 1 : s_numMenu[menu] - DISP_NUM_LINES ;
	}
	

	for( i = gDrawIdx[menu] ; i < s_numMenu[menu] && lineNum < DISP_NUM_LINES ; i++ )
	{
		printKindName( menu, i, lineNum, line );
		printValue( menu, i, lineNum, &gAllInfo[menu][i] );
		
		// �`��I�t�Z�b�g�̍X�V
		lineNum += gAllInfo[menu][i].numLines;
	}

	drawRegister( menu, line );
	
	// �S�̂̍s����c��
	if( gMenuLineSize[menu] == 0 )
	{
		gMenuLineSize[menu] = countMenuLine(menu);
	}

	// �X�N���[���o�[�Ƃ��o��
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
// ���ږ��`��֐�
{
	int linenum = 0;
	int offset = 0;

	if( menu == MENU_ROOT )
	{
		// ���[�g���j���[�ɂ̓C���f�N�X�f�[�^���Ȃ��̂ŁA�Ԋu�������Ȃ����Ƃ�O��ɗ��񂷂邾��
		for(linenum = 0; linenum < ROOTMENU_SIZE ; linenum++)
		{
			if( linenum == line )
			{
				// �I�𒆂̍��ڂ͕\���F�����Ė��\��
				PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET*linenum, TXT_COLOR_BLACK, (const u16 *)L"��" );
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
				// �I�𒆂̍��ڂ͕\���F�����Ė��\��
				PutStringUTF16( ALLOW_LEFT, KIND_UP + LINE_OFFSET * gMenuKindOffset[menu][linenum], 
					TXT_COLOR_BLACK, (const u16 *)L"��" );
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
		
	// utf16�ŕ`�悷����̂���������
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
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_UCOLOR_G5, "��" );
		PrintfSJIS( VALUE_LEFT + LINE_OFFSET , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
		
	}
	else
	{
		// sjis�ŕ`�悷�����
		kindOffset[valueIdx] = drawKindOffset;
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	}
	
}

void drawParentalMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;	
	
	// utf16�ŕ`�悷����̂���������
	if(valueIdx == PARENTAL_ANSWER)
	{			
		kindOffset[valueIdx] = drawKindOffset;
		PutStringUTF16( VALUE_LEFT, VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, gSecretAnswer) ;
	}
	else
	{
		// sjis�ŕ`�悷�����
		kindOffset[valueIdx] = drawKindOffset;
		PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );
	}
}	


void drawNormalHWMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;

	// �`�悵�Ă��łɃC���f�N�X�����f�[�^�������
	kindOffset[valueIdx] = drawKindOffset;
	PrintfSJIS( VALUE_LEFT , VALUE_UP + LINE_OFFSET*drawLineOffset, TXT_COLOR_BLACK, info[valueIdx] );

}


void drawSecureHWMenu( int page, int linenum, int valueIdx, char** info, int* kindOffset, const int* pageOffset )
{
	int drawLineOffset = linenum,  drawKindOffset = linenum;
	
	// �����s�K�v�ȍ��ڂ�����ƕ`��ʒu������邽��
	// �`��I�t�Z�b�g�𒲐����鏈�����K�v
	if( valueIdx >= SECURE_HW_FORCE_DISABLE && page == getPageNum( SECURE_HW_FORCE_DISABLE, pageOffset ) )
	{
		drawLineOffset++;
	}		
	
	if( valueIdx > SECURE_HW_UNIQUE_ID && page == getPageNum( SECURE_HW_UNIQUE_ID, pageOffset ) )
	{
		drawLineOffset += OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 / UNIQUE_BUF;
	}
	
	// �����悤�ɍ��ږ��̃I�t�Z�b�g������
	if( valueIdx > SECURE_HW_FORCE_DISABLE && page == getPageNum( SECURE_HW_FORCE_DISABLE, pageOffset ) )
	{
		drawKindOffset++;
	}
	
	if( valueIdx > SECURE_HW_UNIQUE_ID && page == getPageNum( SECURE_HW_UNIQUE_ID,  pageOffset ) )
	{
		drawKindOffset += OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 / UNIQUE_BUF;
	}
	
	
	// �`�悷��A���łɃC���f�N�X�f�[�^�������
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
// ���j�[�NID�𐮌`���ďo��
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
// valueIdx�����y�[�W�ڂɂ��邩��pageOffset���璲�ׂ�
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