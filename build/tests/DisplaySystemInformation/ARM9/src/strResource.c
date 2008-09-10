/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     strResource.c

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

#include "drawFunc.h"


// VERSION_MENU_SIZE�AFONTMENU_SIZE�����R���e���c���ɉ����ĉςȂ̂�const�ł͂Ȃ�
int s_numMenu[] = {
	OWNERMENU_SIZE,
	PARENTALMENU_SIZE,
	SECURE_USER_MENU_SIZE,
	OTHERMENU_SIZE,
	NORMAL_HW_MENU_SIZE,
	SECURE_HW_MENU_SIZE,
	SCFG_ARM7_MENU_SIZE,
	SCFG_ARM9_MENU_SIZE,
	SYSMENU_MENU_SIZE,
	FONTMENU_SIZE,
	WLMENU_SIZE,
	WHITEMENU_SIZE,
	VERSIONMENU_SIZE,
	0,
	0,
	0,
	ROOTMENU_SIZE
};

const char *s_strARM7RegisterName[] = {
	"ROM",
	"CLK",
	"JTAG",
	"EXP",
	"MC1",
	"MC2",
	"DSWL",
	"OPT",
};

const char *s_strARM9RegisterName[] = {
	"ROM",
	"CLK",
	"RST",
	"EXP",
};

const char *s_strSCFGViewMode[] = {
	"<Shared Area Data>",
	"<Register Data>"
};

const char *s_strRootMenu[] = {
	"��Owner��",
	"��Parental control��",
	"��Secure User Info��",
	"��Other Info��",
	"��Normal Hardware Info��",
	"��Secure Hardware Info��",
	"��SCFG Info (ARM7 side)��",
	"��SCFG Info (ARM9 side)��",
	"��SystemMenu Version��",
	"��Shared Font��",
	"��Wireless Firmware��",
	"��Whitelist��",
	"��Installed Content Version��",
	"Reset HW Setting to Default",
	"break HW Setting",
	"Reset RTC Data"
};

const char *s_strOwnerMenu[] = {
	"Language",
	"Favorite color",
	"Birthday",
	"Country",
	"Nickname",
	"Comment"
};

const char *s_strParentalMenu[] = {
	"Parental control",
	"Picto Chat",
	"ds Download",
	"browser",
	"Wii Point",
	"Photo Exchange",
	"UGC",
	"Organization",
	"Age",
	"Password",
	"Quastion id",
	"Answer"
};

const char *s_strSecureUserMenu[] = {
	"Finished Initial Setting(Setting)",
	"Finished Initial Setting(Launcher)",
	"Finished Broken Setting Sequence",
	"Installed Softbox Count",
	"Free Softbox Count",
	"Last Boot Soft Index",
	"Last Boot Soft Platform",
	"Last Boot Soft ID"
};

const char *s_strOtherMenu[] = {
	"Agree EULA",
	"EULA Version",
	"Wireless",
	"RTC Offset",
	"TP Calib raw x1",
	"TP Calib raw y1",
	"TP Calib dx1",
	"TP Calib dy1",
	"TP Calib raw x2",
	"TP Calib raw y2",
	"TP Calib dx2",
	"TP Calib dy2",
	"TP Calib RSV"
};

const char *s_strNormalHWMenu[] = {
	"RTC Adjustment",
	"Unique ID"
};

const char *s_strSecureHWMenu[] = {
	"Force Disable Wireless",
	"Region",
	"Serial No",
	"Language Bitmap",
	"Fuse Data",
	"Launcher TitleID Lo"
};

const char *s_strSCFGArm9Menu[] = {
	"ARM9 SecureROM",
	"Rom Status",

	"CPU Speed",
	"DSP Clock",	
	"Camera Clock",
	"WRAM Clock",
	"Camera CKI",
	
	"DSP Reset Flag",

	"Fixed DMA",
	"Fixed Geometry",
	"Fixed Renderer",
	"Fixed 2D Engine",
	"Fixed Divider",
	"Fixed Card I/F",
	"Expanded VRAM",
	"Expanded LCDC",
	"Expanded INTC",
	"PSRAM Boundary",
	"New DMA Access",
	"Camera Access",	
	"DSP Access",
	"MemoryCard I/F",
	"WRAM Access",
	"CFG Block Access",
	"All SCFG Check",
	"SCFG Check Err7",
	"SCFG Check Err9"
};

const char *s_strSCFGArm7Menu[] = {
	
	// rom����
	"ARM9 SecureROM",
	"ARM9 ROM Type",
	"ARM7 SecureROM",
	"ARM7 ROM Type",
	"ARM7 FuseRom",
	"Write Rom Area",
	
	// �V�K�u���b�N�N���b�N
	"SD1 I/F Clock",
	"SD2 I/F Clock",
	"AES Clock",
	"WRAM Clock",
	"SND Clock",
	
	// JTAG
	"Chain ARM7 to CPU JTAG",
	"CPU JTAG",
	"DSP JTAG",
	
	// �g���@�\
	"Fixed A7-DMAC1",
	"Fixed Sound DMA",
	"Fixed Sound",
	"Fixed Memory Card",
	"Expanded A7-INTC",
	"Expanded SPI",
	"Expanded Sound DMA",
	"Expanded SIO",
	"Expanded LCDC",
	"Expanded VRAM",
	"PSRAM Boundary",
	"A7-DMAC2 Block",
	"AES Block",
	"SD1 I/F Block",
	"SD2 I/F Block",
	"Mic Block",
	"I2S I/F Block",
	"I2C I/F Block",
	"GPIO Block",
	"MemoryCard I/F",
	"Shared WRAM",	
	"PU Resistance",
	"ALL SCFG Block",
	
	// �������J�[�h I/F
	"MC Slot1 DET",
	"MC Slot1 Mode",
	"MC Slot2 DET",
	"MC Slot2 Mode",
	"Swap MC1 MC2",
	"Chatter. Count",
	"MC Counter Data",
	
	// ������
	"Old Wireless",
	
	// �I�v�V�����[�q�ǂݏo��
	"Option Form",
	"Option App for"
};

const char *s_strSystemMenu[] = {
	"Timestamp",
	"Version (numeric)",
	"Version (string)",
	"EULA URL",
	"NUP Hostname",

	".twl-nup-cert.der" ,
	".twl-nup-prvkey.der",
	".twl-shop-cert.der",
	".twl-shop-prvkey.der",
	"NintendoCA-G2.der"
};

const char *s_strFontMenu[] = {
	"Timestamp",
};

const char *s_strWLMenu[] = {
	"Version",
	"Num of FW",
	"FW1 type",
	"FW1 version",
	"FW2 type",
	"FW2 version"
};

const char *s_strWhiteMenu[] = {
	"Num of Entry",
	"Tmd Hash"
};

const char **s_strMetaMenu[] = {
	s_strOwnerMenu,
	s_strParentalMenu,
	s_strSecureUserMenu,
	s_strOtherMenu,
	s_strNormalHWMenu,
	s_strSecureHWMenu,
	s_strSCFGArm7Menu,
	s_strSCFGArm9Menu,
	s_strSystemMenu,
	s_strFontMenu,
	s_strWLMenu,
	s_strWhiteMenu,
	NULL,
	NULL,
	NULL,
	NULL,
	s_strRootMenu
};

char *s_strAccess[] = {
	"Inaccessible",
	"Accessible"
};

char *s_strJoint[] = {
	"Jointed",
	"DisJointed"
};

char *s_strSupply[] = {
	"STOPPED",
	"SUPPLIED"
};

char *s_strPSRAM[] = {
	"4MB",
	"16MB",
	"32MB"
};

char *s_strRomMode[] = {
	"TWL 64KB ROM",
	"NITRO 8KB ROM"

};

char *s_strCpuSpeed[] = {
	"67.03 MHz",
	"134.06 MHz"
};

char *s_strRomForm[] = {
	"Included ROM",
	"Downloaded PROM"
};

char *s_strRomApp[] = {
	"Mass Product",
	"Development 1",
	"Development 2",
	"PROM Download"
};

char *s_strWLFWType[] = {
	"2in1 module 1.1",
	"2in1 module 2.0",
	"One Chip (M&M)"
};

char *s_strMCMode[] = {
	"00",
	"01",
	"10",
	"11"
};

char *s_strEnable[] = {
	"DISABLED",
	"ENABLED"
};

char *s_strResult[] = {
	"Failed.",
	"Succeeded.",	
	""
};


char *s_strCorrect[] = {
	"Incorrect",
	"Correct"
};

char *s_strSysMenuKey[] = {
	"Incorrect",
	"correct: dev",
	"correct: prod"
};

char *s_strBool[] = {
	"FALSE",
	"TRUE"
};

char *s_strOK[] = {
	"CANCEL",
	"OK"
};

char *s_strRatingOrg[] = {
	"CERO",
	"ESRB",
	"BBFC",
	"USK",
	"PEGI general",
	"PEGI Finland",
	"PEGI Portugal",
	"PEGI and BBFC Great Briten",
	"OFLC",
	"GRB"
};

 char *s_strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPA",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

 char *s_strUserColor[] = {
	"GRAY      ",
	"BROWN     ",
	"RED       ",
	"PINK      ",
	"ORANGE    ",
	"YELLOW    ",
	"LIME GREEN",
	"GREEN     ",
	"DARK GREEN",
	"SEA GREEN ",
	"TURQUOISE ",
	"BLUE      ",
	"DARK BLUE ",
	"PURPLE    ",
	"VIOLET    ",
	"MAGENTA   ",
};

char *s_strLanguage[] = {
	"JAPANESE",
	"ENGLISH",
	"FRENCH",
	"GERMAN",
	"ITALIAN",
	"SPANISH",
	"CHINESE",
	"KOREAN",
};

char *s_strCountry[] = {
    	"UNDEFINED  ",        // ���ݒ�
    	"JAPAN      ",        // ���{
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"Anguilla   ",        // �A���M��
    	"ANTIGUA_AND_BARBUDA",   // �A���e�B�O�A�E�o�[�u�[�_
    	"ARGENTINA   ",      // �A���[���`��
    	"ARUBA",                 // �A���o
    	"BAHAMAS",               // �o�n�}
    	"BARBADOS",              // �o���o�h�X
    	"BELIZE",                // �x���[�Y
    	"BOLIVIA",               // �{���r�A
    	"BRAZIL",                // �u���W��
    	"BRITISH_VIRGIN_ISLANDS",    // �p�̃��@�[�W������
    	"CANADA",                // �J�i�_
    	"CAYMAN_ISLANDS",        // �P�C�}������
    	"CHILE       ",      // �`��
    	"COLOMBIA",              // �R�����r�A
    	"COSTA_RICA",            // �R�X�^���J
    	"DOMINICA",              // �h�~�j�J��
    	"DOMINICAN_REPUBLIC",    // �h�~�j�J���a��
    	"ECUADOR",               // �G�N�A�h��
    	"EL_SALVADOR",           // �G���T���o�h��
    	"FRENCH_GUIANA",         // �t�����X�̃M�A�i
    	"GRENADA",               // �O���i�_
    	"GUADELOUPE",            // �O�A�h���[�v
    	"GUATEMALA   ",      // �O�A�e�}��
    	"GUYANA",                // �K�C�A�i
    	"HAITI",                 // �n�C�`
    	"HONDURAS",              // �z���W�����X
    	"JAMAICA",               // �W���}�C�J
    	"MARTINIQUE",            // �}���e�B�j�[�N
    	"MEXICO",                // ���L�V�R
    	"MONTSERRAT",            // �����g�Z���g
    	"NETHERLANDS_ANTILLES",  // �I�����_�̃A���e�B��
    	"NICARAGUA",             // �j�J���O�A
    	"PANAMA      ",      // �p�i�}
    	"PARAGUAY",              // �p���O�A�C
    	"PERU",                  // �y���[
    	"ST_KITTS_AND_NEVIS",    // �Z���g�L�b�c�E�l�C�r�X
    	"ST_LUCIA",              // �Z���g���V�A
    	"ST_VINCENT_AND_THE_GRENADINES", // �Z���g�r���Z���g�E�O���i�f�B�[��
    	"SURINAME",              // �X���i��
    	"TRINIDAD_AND_TOBAGO",   // �g���j�_�[�h�E�g�o�S
    	"TURKS_AND_CAICOS_ISLANDS",  // �^�[�N�X�E�J�C�R�X����
    	"UNITED_STATES",         // �A�����J
    	"URUGUAY     ",      // �E���O�A�C
    	"US_VIRGIN_ISLANDS",     // �ė̃o�[�W������
    	"VENEZUELA",             // �x�l�Y�G��
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"ALBANIA     ",      // �A���o�j�A
    	"AUSTRALIA",             // �I�[�X�g�����A
    	"AUSTRIA",               // �I�[�X�g���A
    	"BELGIUM",               // �x���M�[
    	"BOSNIA_AND_HERZEGOVINA",    // �{�X�j�A�E�w���c�F�S�r�i
    	"BOTSWANA",              // �{�c���i
    	"BULGARIA    ",      // �u���K���A
    	"CROATIA",               // �N���A�`�A
    	"CYPRUS",                // �L�v���X
    	"CZECH_REPUBLIC",        // �`�F�R
    	"DENMARK",               // �f���}�[�N
    	"ESTONIA",               // �G�X�g�j�A
    	"FINLAND",               // �t�B�������h
    	"FRANCE",                // �t�����X
    	"GERMANY",               // �h�C�c
    	"GREECE",                // �M���V��
    	"HUNGARY     ",      // �n���K���[
    	"ICELAND",               // �A�C�X�����h
    	"IRELAND",               // �A�C�������h
    	"ITALY",                 // �C�^���A
    	"LATVIA",                // ���g�r�A
    	"LESOTHO",               // ���\�g
    	"LIECHTENSTEIN",         // ���q�e���V���^�C��
    	"LITHUANIA",             // ���g�A�j�A
    	"LUXEMBOURG",            // ���N�Z���u���N
    	"MACEDONIA",             // �}�P�h�j�A
    	"MALTA       ",      // �}���^
    	"MONTENEGRO",            // �����e�l�O��
    	"MOZAMBIQUE",            // ���U���r�[�N
    	"NAMIBIA",               // �i�~�r�A
    	"NETHERLANDS",           // �I�����_
    	"NEW_ZEALAND",           // �j���[�W�[�����h
    	"NORWAY",                // �m���E�F�[
    	"POLAND",                // �|�[�����h
    	"PORTUGAL",              // �|���g�K��
    	"ROMANIA",               // ���[�}�j�A
    	"RUSSIA      ",     // ���V�A
    	"SERBIA",                // �Z���r�A
    	"SLOVAKIA",              // �X���o�L�A
    	"SLOVENIA",              // �X���x�j�A
    	"SOUTH_AFRICA",          // ��A�t���J
    	"SPAIN",                 // �X�y�C��
    	"SWAZILAND",             // �X���W�����h
    	"SWEDEN",                // �X�E�F�[�f��
    	"SWITZERLAND",           // �X�C�X
    	"TURKEY",                // �g���R
    	"UNITED_KINGDOM ",  // �C�M���X
    	"ZAMBIA",                // �U���r�A
    	"ZIMBABWE",              // �W���o�u�G
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"TAIWAN      ",     // ��p
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"SOUTH_KOREA ",     // �؍�
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"HONG_KONG   ",     // �z���R��
    	"MACAU",                 // �}�J�I
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"INDONESIA   ",     // �C���h�l�V�A
    	"SINGAPORE   ",     // �V���K�|�[��
    	"THAILAND    ",     // �^�C
    	"PHILIPPINES",           // �t�B���s��
    	"MALAYSIA",              // �}���[�V�A
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"CHINA       ",     // ����
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"UAE         ",     // �A���u�񒷍��A�M
    	"INDIA       ",     // �C���h
    	"EGYPT       ",     // �G�W�v�g
    	"OMAN",                  // �I�}�[��
    	"QATAR",                 // �J�^�[��
    	"KUWAIT",                // �N�E�F�[�g
    	"SAUDI_ARABIA",          // �T�E�W�A���r�A
    	"SYRIA",                 // �V���A
    	"BAHRAIN",               // �o�[���[��
    	"JORDAN",                // �����_��
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"OTHERS      ",
    	"UNKNOWN     ",
};

char s_strNA[] = {
	"N/A"
};	

