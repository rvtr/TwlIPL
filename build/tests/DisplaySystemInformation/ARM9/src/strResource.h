/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     strResource.h

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

#ifndef __DISPLAY_INFO_RESOURCE__
#define __DISPLAY_INFO_RESOURCE__

// メニューID
#define MENU_ROOT 			15
#define MENU_OWNER 			0
#define MENU_PARENTAL 		1
#define MENU_SECURE_USER	2
#define MENU_OTHER			3
#define MENU_NORMAL_HW		4
#define MENU_SECURE_HW		5
#define MENU_SCFG_ARM7		6
#define MENU_SCFG_ARM9		7
#define MENU_SYSMENU		8
#define MENU_FONT			9
#define MENU_WL				10
#define MENU_WHITE			11
#define MENU_VERSION		12
#define MENU_RESET_INFO		13
#define MENU_BREAK_DATA		14


// 行番号
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

#define SECURE_USER_INITIAL_SETTINGS	0
#define SECURE_USER_INITIAL_LAUNCHER	1
#define SECURE_USER_BROKEN_SETTINGS		2
#define SECURE_USER_INSTALLED_SOFTBOX	3
#define SECURE_USER_FREE_SOFTBOX		4
#define SECURE_USER_LASTBOOT_IDX		5
#define SECURE_USER_LASTBOOT_PLATFORM	6
#define SECURE_USER_LASTBOOT_ID			7

#define OTHER_AGREE_EULA				0
#define OTHER_EULA_VERSION				1
#define OTHER_WIRELESS					2

#define NORMAL_HW_RTC_OFFSET	0
#define NORMAL_HW_UNIQUE_ID		1

#define SECURE_HW_FORCE_DISABLE	0
#define SECURE_HW_REGION		1
#define SECURE_HW_SERIAL		2
#define SECURE_HW_LANGUAGE		3
#define SECURE_HW_FUSE			4
#define SECURE_HW_LAUNCHER_ID	5

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

#define SYSMENU_TIMESTAMP		0
#define SYSMENU_VERSION_NUM		1
#define SYSMENU_VERSION_STR		2
#define SYSMENU_EULA_URL		3
#define SYSMENU_NUP_HOST		4

#define SYSMENU_NUP_CERT		5
#define SYSMENU_NUP_PRV			6
#define SYSMENU_SHOP_CERT		7
#define SYSMENU_SHOP_PRV		8
#define SYSMENU_NINTENDO_CAG2	9

#define SYSMENU_HASH_IDX	5

#define FONT_TIMESTAMP		0
#define FONT_INFO			1

#define WL_VERSION			0
#define WL_NUM_FW			1
#define WL_FW_TYPE			2

#define WHITE_NUM			0
#define WHITE_HASH			1

#define VERSION_OTHER		0

// 各メニューサイズ
#define ROOTMENU_SIZE 			15
#define OWNERMENU_SIZE 			6
#define PARENTALMENU_SIZE 		12
#define SECURE_USER_MENU_SIZE	8
#define OTHERMENU_SIZE			3
#define NORMAL_HW_MENU_SIZE 	2
#define SECURE_HW_MENU_SIZE 	6
#define SCFG_ARM7_MENU_SIZE 	47
#define SCFG_ARM9_MENU_SIZE 	24
#define SYSMENU_MENU_SIZE		10
#define FONTMENU_SIZE			1
#define WLMENU_SIZE				3
#define WHITEMENU_SIZE			2
#define VERSIONMENU_SIZE 		0


extern int s_numMenu[];
extern const char *s_strRootMenu[];
extern const char *s_strOwnerMenu[];
extern const char *s_strParentalMenu[];
extern const char *s_strSecureUserMenu[];
extern const char *s_strOtherMenu[];
extern const char *s_strNormalHWMenu[];
extern const char *s_strSecureHWMenu[];
extern const char *s_strSCFGARM7Menu[];
extern const char *s_strSCFGARM9Menu[];
extern const char *s_strOtherMenu[];
extern const char *s_strSystemMenu[];
extern const char *s_strFontMenu[] ;
extern const char *s_strWLMenu[];
extern const char *s_strWhiteMenu[];
extern const char **s_strMetaMenu[];
extern const char *s_strARM7RegisterName[];
extern const char *s_strARM9RegisterName[];
extern const char *s_strSCFGViewMode[];

extern char *s_strEnable[];
extern char *s_strJoint[];
extern char *s_strCorrect[];
extern char *s_strSysMenuKey[];
extern char *s_strSupply[];
extern char *s_strRomMode[];
extern char *s_strPSRAM[];
extern char *s_strCpuSpeed[];
extern char *s_strOK[];
extern char *s_strWLFWType[];
extern char *s_strResult[];
extern char *s_strBool[];
extern char *s_strAccess[];
extern char *s_strRomApp[];
extern char *s_strRomForm[];
extern char *s_strMCMode[];
extern char *s_strRatingOrg[];
extern char *s_strRegion[];
extern char *s_strUserColor[];
extern char *s_strLanguage[];
extern char *s_strCountry[];
extern char s_strNA[];	

#endif