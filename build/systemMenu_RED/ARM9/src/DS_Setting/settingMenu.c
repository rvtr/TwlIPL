/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mainMenu.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include "main.h"
#include "DS_Setting.h"
#include "spi.h"

// define data------------------------------------------
	// �L�����Z���{�^��LCD�̈�
#define CANCEL_BUTTON_LT_X					12
#define CANCEL_BUTTON_LT_Y					21
#define CANCEL_BUTTON_RB_X					(CANCEL_BUTTON_LT_X + 8)
#define CANCEL_BUTTON_RB_Y					(CANCEL_BUTTON_LT_Y + 2)
	// OK�{�^��LCD�̈�
#define OK_BUTTON_LT_X						22
#define OK_BUTTON_LT_Y						21
#define OK_BUTTON_RB_X						(OK_BUTTON_LT_X + 8)
#define OK_BUTTON_RB_Y						(OK_BUTTON_LT_Y + 2)


#define MAIN_MENU_ELEM_NUM					6						// ���C�����j���[�̍��ڐ�

// extern data------------------------------------------

// function's prototype declaration---------------------
void SEQ_MainMenu_init(void);
int  SEQ_MainMenu(void);
BOOL SelectMenuByTp(u16 *nowCsr, const MenuComponent *menu);
//BOOL InRangeTp(u16 lt_x,u16 lt_y,u16 rb_x,u16 rb_y, TPData *tgt);
BOOL InRangeTp(int lt_x, int lt_y, int rb_x, int rb_y, TPData *tgt);

static void SEQ_SettingEnd_init( void );
static int  SEQ_SettingEnd( void );

// global variable -------------------------------------
u16  csrMenu	= 0;													// ���j���[�̃J�[�\���ʒu�ibm_main.c�ŎQ�Ƃ��Ă�̂ŁA�O���[�o���j
BOOL initialSet	= FALSE;

// static variable -------------------------------------
static const u8 *str_MainMenu[MAIN_MENU_ELEM_NUM];					// ���C�����j���[�p�����e�[�u���ւ̃|�C���^���X�g

// const data  -----------------------------------------


//===============================================
// mainMenu.c
//===============================================
const u8 *const str_MeinMenuElemTbl[ MAIN_MENU_ELEM_NUM ][ LANG_CODE_MAX ] = {
	{
		(const u8 *)"���[�U�[�@���傤�ق��@�@�@",
		(const u8 *)"USER INFORMATION   ",
		(const u8 *)"USER INFORMATION(F)",
		(const u8 *)"USER INFORMATION(G)",
		(const u8 *)"USER INFORMATION(I)",
		(const u8 *)"USER INFORMATION(S)",
	},
	{
		(const u8 *)"�ЂÂ��@���@�������@�@�@�@",
		(const u8 *)"DATE & TIME         ",
		(const u8 *)"DATE & TIME(F)      ",
		(const u8 *)"DATE & TIME(G)      ",
		(const u8 *)"DATE & TIME(I)      ",
		(const u8 *)"DATE & TIME(S)      ",
	},
	{
		(const u8 *)"���񂲁@�@�@�@�@�@�@�@�@�@",
		(const u8 *)"LANGUAGE            ",
		(const u8 *)"LANGUAGE(F)         ",
		(const u8 *)"LANGUAGE(G)         ",
		(const u8 *)"LANGUAGE(I)         ",
		(const u8 *)"LANGUAGE(S)         ",
	},
	{
		(const u8 *)"AGB�@���[�h�@�@�@�@�@�@�@�@",
		(const u8 *)"AGB MODE            ",
		(const u8 *)"AGB MODE(F)         ",
		(const u8 *)"AGB MODE(G)         ",
		(const u8 *)"AGB MODE(I)         ",
		(const u8 *)"AGB MODE(S)         ",
	},
	{
		(const u8 *)"�^�b�`�p�l���ق����@�@�@�@�@",
		(const u8 *)"TOUCH PANEL         ",
		(const u8 *)"TOUCH PANEL(F)      ",
		(const u8 *)"TOUCH PANEL(G)      ",
		(const u8 *)"TOUCH PANEL(I)      ",
		(const u8 *)"TOUCH PANEL(S)      ",
	},
	{
		(const u8 *)"���ǂ����[�h�@�@�@�@�@�@�@�@",
		(const u8 *)"AUTO BOOT           ",
		(const u8 *)"AUTO BOOT(F)        ",
		(const u8 *)"AUTO BOOT(G)        ",
		(const u8 *)"AUTO BOOT(I)        ",
		(const u8 *)"AUTO BOOT(S)        ",
	},
};

const MenuComponent mainMenu = {
	MAIN_MENU_ELEM_NUM,
	2,
	6,
	0,
	2,
	17,
	WHITE,
	HIGHLIGHT_Y,
	(const u8 **)&str_MainMenu,
};

//======================================================
// ���C�����j���[
//======================================================

// ���C�����j���[�̏�����
void SEQ_MainMenu_init(void)
{
#ifdef __DIRECT_BOOT_BMENU_ENABLE
	// �e��ݒ肪���ݒ莞�̃_�C���N�g�N���B
	{
		if(GetNCDWork()->option.input_language == 0) {			// ����ݒ肪�܂��B
			initialSet	= TRUE;
			csrMenu		= 3;
			SEQ_LangSelect_init();
			nowProcess	= SEQ_LangSelect;
			return;
		}else if(GetNCDWork()->option.input_tp == 0) {			// TP�L�����u���[�V�������܂��B
			initialSet	= TRUE;
			csrMenu		= 5;
			SEQ_TP_Calibration_init();
			nowProcess	= SEQ_TP_Calibration;
			return ;
		}else if(GetNCDWork()->option.input_rtc == 0) {			// RTC�ݒ肪�܂��B
			ClearRTC();
			initialSet	= TRUE;
			csrMenu		= 2;
			SEQ_RtcSet_init();
			nowProcess	= SEQ_RtcSet;
			return;
		}else if( (GetNCDWork()->option.input_nickname == 0)	// �j�b�N�l�[���܂��͍D���ȐF���͂��܂��B
			  ||  (GetNCDWork()->option.input_favoriteColor == 0) ) {
			initialSet	= TRUE;
			csrMenu		= 1;
			SEQ_OwnerInfo_init();
			nowProcess	= SEQ_OwnerInfo;
			return;
		}
		
		if( initialSet ) {
			SEQ_SettingEnd_init();
			nowProcess = SEQ_SettingEnd;
			return;
		}
	}
#endif /* __DIRECT_BOOT_BMENU_ENABLE */
	
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	SVC_CpuClearFast(0x0000, bgBakS, sizeof(bgBakS));
	SVC_CpuClearFast(0xc0,  oamBakS, sizeof(oamBakS));
	
	ClearAllStringSJIS();
	
#ifdef __NCD_CLEAR_ENABLE
	(void)DrawStringSJIS( 18, 21, LIGHTGREEN, (const u8 *)"[START]:NCD clear.");
#endif	/* __NCD_CLEAR_ENABLE */
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	{
		int i;
		NvLangCode langCode = LANG_ENGLISH;
		if(GetSYSMWork()->ncd_invalid == 0) {
			langCode = (NvLangCode)GetNCDWork()->option.language;
		}
		for(i = 0; i < MAIN_MENU_ELEM_NUM; i++) {
			str_MainMenu[i] = str_MeinMenuElemTbl[i][langCode];
		}
	}
	DrawMenu(csrMenu, &mainMenu);
	
	SVC_CpuClear(0x0000,&tpd,sizeof(TpWork),16);
	
	GXS_SetVisiblePlane(GX_PLANEMASK_BG1);
	
	nowProcess		  = SEQ_MainMenu;
}


// ���C�����j���[
int SEQ_MainMenu(void)
{
	BOOL tp_select;
	
	ReadTpData();													// �^�b�`�p�l�����͂̎擾
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if(pad.trg & PAD_KEY_DOWN){										// �J�[�\���̈ړ�
		if(++csrMenu == MAIN_MENU_ELEM_NUM) csrMenu=0;
	}
	if(pad.trg & PAD_KEY_UP){
		if(--csrMenu & 0x80) csrMenu=MAIN_MENU_ELEM_NUM-1;
	}
	tp_select=SelectMenuByTp(&csrMenu, &mainMenu);
	DrawMenu(csrMenu, &mainMenu);
	
	if((pad.trg & PAD_BUTTON_A)||(tp_select)) {						// ���j���[���ڂւ̕���
		switch(csrMenu) {
			case 0:
				SEQ_OwnerInfo_init();
				nowProcess=SEQ_OwnerInfo;
				break;
			case 1:
				SEQ_RtcSet_init();
				nowProcess=SEQ_RtcSet;
				break;
			case 2:
				SEQ_LangSelect_init();
				nowProcess=SEQ_LangSelect;
				break;
			case 3:
				SEQ_AgbLcdSelect_init();
				nowProcess=SEQ_AgbLcdSelect;
				break;
			case 4:
				SEQ_TP_Calibration_init();
				nowProcess=SEQ_TP_Calibration;
				break;
			case 5:
				SEQ_AutoBootSelect_init();
				nowProcess=SEQ_AutoBootSelect;
				break;
		}
	}
	
#ifdef __NCD_CLEAR_ENABLE
	if(pad.trg & PAD_BUTTON_START) {
		SVC_CpuClearFast(0x0000, GetNCDWork(), sizeof(NitroConfigData));
		(void)SPI_NvramWriteEnable();
		SVC_WaitVBlankIntr();
		(void)SPI_NvramPageErase(0x3fe00);
		SVC_WaitVBlankIntr();
		(void)SPI_NvramWriteEnable();
		SVC_WaitVBlankIntr();
		(void)SPI_NvramPageErase(0x3ff00);
		SVC_WaitVBlankIntr();
		(void)SPI_NvramWriteDisable();
		OS_Printf("NitroConfigData zero clear!!\n");
	}
#endif	/* __NCD_CLEAR_ENABLE */
	
	return 0;
}


// OK / CANCEL�{�^���̕`��
void DrawOKCancelButton(void)
{
	(void)DrawStringSJIS( CANCEL_BUTTON_LT_X, CANCEL_BUTTON_LT_Y,HIGHLIGHT_C, (const u8 *)" CANCEL ");
	(void)DrawStringSJIS( OK_BUTTON_LT_X,     OK_BUTTON_LT_Y,    HIGHLIGHT_C, (const u8 *)"   OK   ");
}


// OK or CANCEL�{�^�������`�F�b�N
void CheckOKCancelButton(BOOL *tp_ok, BOOL *tp_cancel)
{
	*tp_cancel = InRangeTp(CANCEL_BUTTON_LT_X*8, CANCEL_BUTTON_LT_Y*8-4,
						   CANCEL_BUTTON_RB_X*8, CANCEL_BUTTON_RB_Y*8-4, &tpd.disp);
	*tp_ok     = InRangeTp(OK_BUTTON_LT_X*8,     OK_BUTTON_LT_Y*8-4,
						   OK_BUTTON_RB_X*8,     OK_BUTTON_RB_Y*8-4, &tpd.disp);
}


//---------------------------------------------------------
//
// �ݒ�I��
//
//---------------------------------------------------------

static void SEQ_SettingEnd_init( void )
{
	ClearAllStringSJIS();
	(void)DrawStringSJIS( 6, 10, WHITE, (const u8 *)" Initial setting completed.");
	(void)DrawStringSJIS( 6, 12, WHITE, (const u8 *)"      Please reboot.");
}


static int SEQ_SettingEnd( void )
{
	return 0;
}

