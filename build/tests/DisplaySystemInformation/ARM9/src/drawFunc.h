/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     drawFunc.h

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

#ifndef	__DRAW_FUNC__
#define	__DRAW_FUNC__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif




// �e���j���[�T�C�Y

#define DISP_NUM_LINES 11		// ��y�[�W������̍��ڐ�

#define ROOTMENU_SIZE 7
#define OWNERMENU_SIZE 6
#define PARENTALMENU_SIZE 12
#define NORMAL_HW_MENU_SIZE 4
#define SECURE_HW_MENU_SIZE 6
#define SCFG_ARM7_MENU_SIZE 47
#define SCFG_ARM9_MENU_SIZE 24
#define VERSIONMENU_SIZE 3


// ���j���[ID
#define MENU_ROOT 10
#define MENU_OWNER 0
#define MENU_PARENTAL 1
#define MENU_NORMAL_HW 2
#define MENU_SECURE_HW 3
#define MENU_SCFG_ARM7 4
#define MENU_SCFG_ARM9 5
#define MENU_VERSION 6

// �s�ԍ�
#define OWNER_LANGUAGE			0
#define OWNER_COLOR				1
#define OWNER_BIRTHDAY			2
#define OWNER_COUNTRY			3
#define OWNER_NICKNAME			4
#define OWNER_COMMENT			5

#define PARENTAL_FLAG			0
#define PARENTAL_PICTOCHAT		1
#define PARENTAL_DOWNLOAD		2
#define PARENTAL_BROWSER		3
#define PARENTAL_WIIPOINT		4
#define PARENTAL_PHOTO_EXCHANGE	5
#define PARENTAL_UGC			6
#define PARENTAL_ORGANIZATION 	7
#define PARENTAL_AGE			8
#define PARENTAL_PASSWORD		9
#define PARENTAL_QUESTION_ID	10
#define PARENTAL_ANSWER			11

#define NORMAL_HW_WIRELESS		0
#define NORMAL_HW_RTC_OFFSET	1
#define NORMAL_HW_AGREE_EULA	2
#define NORMAL_HW_EULA_VERSION	3

#define SECURE_HW_FORCE_DISABLE	0
#define SECURE_HW_REGION		1
#define SECURE_HW_UNIQUE_ID		2
#define SECURE_HW_SERIAL		3
#define SECURE_HW_LANGUAGE		4
#define SECURE_HW_FUSE			5

#define SCFG_ARM9_ROM_SEC			0
#define SCFG_ARM9_ROM_STATE			1
#define SCFG_ARM9_CLK_CPU			2
#define SCFG_ARM9_CLK_DSP			3
#define SCFG_ARM9_CLK_CAM			4
#define SCFG_ARM9_CLK_WRAM			5
#define SCFG_ARM9_CLK_CAM_CKI		6
#define SCFG_ARM9_RST_DSP			7
#define SCFG_ARM9_EXT_DMA			8
#define SCFG_ARM9_EXT_GEO			9
#define SCFG_ARM9_EXT_REN			10
#define SCFG_ARM9_EXT_2DE			11
#define SCFG_ARM9_EXT_DIV			12
#define SCFG_ARM9_EXT_MC			13
#define SCFG_ARM9_EXT_INTC			14
#define SCFG_ARM9_EXT_LCDC			15
#define SCFG_ARM9_EXT_VRAM			16
#define SCFG_ARM9_EXT_PS			17
#define SCFG_ARM9_EXT_DMAC			18
#define SCFG_ARM9_EXT_CAM			19
#define SCFG_ARM9_EXT_DSP			20
#define SCFG_ARM9_EXT_MCB			21
#define SCFG_ARM9_EXT_WRAM			22
#define SCFG_ARM9_EXT_CFG			23


#define SCFG_ARM7_ROM_ARM9_SEC		0
#define SCFG_ARM7_ROM_ARM9_RSEL		1
#define SCFG_ARM7_ROM_ARM7_SEC		2
#define SCFG_ARM7_ROM_ARM7_RSEL		3
#define SCFG_ARM7_ROM_ARM7_FUSE		4
#define SCFG_ARM7_ROM_WE			5
#define SCFG_ARM7_CLK_SD1			6
#define SCFG_ARM7_CLK_SD2			7
#define SCFG_ARM7_CLK_AES			8
#define SCFG_ARM7_CLK_WRAM			9
#define SCFG_ARM7_CLK_SND			10
#define SCFG_ARM7_JTAG_A7			11
#define SCFG_ARM7_JTAG_CPU			12
#define SCFG_ARM7_JTAG_DSP			13
#define SCFG_ARM7_EXT_DMA			14
#define SCFG_ARM7_EXT_SDMA			15
#define SCFG_ARM7_EXT_SND			16
#define SCFG_ARM7_EXT_MC			17
#define SCFG_ARM7_EXT_INTC			18
#define SCFG_ARM7_EXT_SPI			19
#define SCFG_ARM7_EXT_DSEL			20
#define SCFG_ARM7_EXT_SIO			21
#define SCFG_ARM7_EXT_LCDC			22
#define SCFG_ARM7_EXT_VRAM			23
#define SCFG_ARM7_EXT_PS			24
#define SCFG_ARM7_EXT_DMAC			25
#define SCFG_ARM7_EXT_AES			26
#define SCFG_ARM7_EXT_SD1			27
#define SCFG_ARM7_EXT_SD2			28
#define SCFG_ARM7_EXT_MIC			29
#define SCFG_ARM7_EXT_I2S			30
#define SCFG_ARM7_EXT_I2C			31
#define SCFG_ARM7_EXT_GPIO			32
#define SCFG_ARM7_EXT_MCB			33
#define SCFG_ARM7_EXT_WRAM			34
#define SCFG_ARM7_EXT_PU			35
#define SCFG_ARM7_EXT_CFG			36
#define SCFG_ARM7_MI_SC1_CDET		37
#define SCFG_ARM7_MI_SC1_MODE		38
#define SCFG_ARM7_MI_SC2_CDET		39
#define SCFG_ARM7_MI_SC2_MODE		40
#define SCFG_ARM7_MI_SWP			41
#define SCFG_ARM7_MI_CC				42
#define SCFG_ARM7_MI_CA				43
#define SCFG_ARM7_WL_OFFB			44
#define SCFG_ARM7_OP_FORM			45
#define SCFG_ARM7_OP_APP			46

#define SCFG_ARM7_SHARED_OFFSET		47

#define VERSION_WIRELESS	0
#define VERSION_FONT		1
#define VERSION_OTHER		2




/* global variables ----------------- */

#define MAXITEM 50
#define MAXPAGE 5

// �e���ڂ�\������Ƃ��̍s�I�t�Z�b�g�\
extern int gMenuKindOffset[ROOTMENU_SIZE][MAXITEM];

/* function prototypes ----------------- */

void drawHeader( int menu, int line );
void drawMenu( int menu, int line, int changeLine, BOOL isChangeMode );
void switchViewMode( void );

#ifdef __cplusplus
}
#endif

#endif
