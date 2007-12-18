/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     CooperationB.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-31#$
  $Rev: 91 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "misc.h"
#include "CooperationB.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define COPB_MENU_ELEMENT_NUM			2						// ���j���[�̍��ڐ�

#define PARAM_LENGTH	10

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------
static u16 s_csr = 0;
static u16 s_parameter[ PARAM_LENGTH + 1 ];
static void(*s_pNowProcess)(void);

static u32 invGameCode;

// const data  -----------------------------------------
static const u16 *s_pStrMenu[ COPB_MENU_ELEMENT_NUM ] = 
{
	L"�Ăяo�����A�v�����N��",
	L"�����`���[�ɖ߂�",
};

static MenuPos s_menuPos[] = {
	{ FALSE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
};

static const MenuParam s_menuParam = {
	COPB_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const u16 **)&s_pStrMenu,
};
									
//======================================================
// �A�v���A�g�e�X�g�v���O����B
//======================================================

static void DrawMenuScene( void )
{
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"CooperationB");
	PutStringUTF16( 1*8, 18*8, TXT_COLOR_BLACK,  (const u16 *)L"�󂯎�����p�����[�^�F");
	PutStringUTF16( 3 * 8 , 20*8, TXT_UCOLOR_G0, s_parameter );
	PutStringUTF16( 1*8, 14*8, TXT_COLOR_BLACK,  (const u16 *)L"�Ăяo�����A�v���F");
	PrintfSJIS(3*8, 16*8, TXT_COLOR_BLACK, "0x%llx",(u64)0x0001000100000000 + invGameCode);
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
    // ���j���[����
	DrawMenu( s_csr, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"CooperationB");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;
	
	MI_CpuClear8(s_parameter, 2*(PARAM_LENGTH+1));
	
	{
		OSDeliverArgInfo *arginfo = (OSDeliverArgInfo *)HW_PARAM_DELIVER_ARG;
		u8 *gc = (u8 *)&arginfo->gameCode;
		invGameCode = (u32)(gc[0]<<24) + (u32)(gc[1]<<16) + (u32)(gc[2]<<8) + (u32)gc[3];
		if(invGameCode != NULL)
		{
			s_menuPos[ 0 ].enable = TRUE;
			MI_CpuCopy8(arginfo->buf, s_parameter, 2*(PARAM_LENGTH+1));
		}
	}
	
	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}

static void MenuScene(void)
{
	BOOL tp_select = FALSE;
	BootFlags tempflag = {TRUE, 0, TRUE, FALSE, FALSE, FALSE, 0};
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		if( ++s_csr == COPB_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=COPB_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_menuParam );
	
   	DrawMenuScene();
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// ���j���[���ڂւ̕���
		if( s_menuPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					if(invGameCode != NULL)
					{
						// �A�v���ԃp�����[�^���Z�b�g
						OSDeliverArgInfo *arginfo = (OSDeliverArgInfo *)HW_PARAM_DELIVER_ARG;
						// ���[�J�[�R�[�h�ƃQ�[���R�[�h�̃Z�b�g(Launcher���ł��ׂ��H)
						u16 *maker_code_src_addr = (u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
						u32 *game_code_src_addr = (u32 *)(HW_TWL_ROM_HEADER_BUF + 0xc);
						arginfo->makerCode = *maker_code_src_addr;
						arginfo->gameCode = *game_code_src_addr;
						// �A�v����p�����̃Z�b�g
						MI_CpuCopy8("-r", arginfo->buf, 3);
						//�Ăяo�����A�v���N��
						OS_SetResetParamAndResetHardware( 0, (u64)0x0001000100000000 + invGameCode, &tempflag );
						//OS_SetResetParamAndResetHardware( 0, (u64)0x00010001434f5041, &tempflag );
					}
					break;
				case 1:
					OS_SetResetParamAndResetHardware( 0, NULL, &tempflag );
					//�ċN��
					break;
			}
		}
	}
}

// ������
void CooperationBInit( void )
{
	ChangeUserColor( TSD_GetUserColor() );
	s_parameter[0] = 0;
	MenuInit();
}

// ���C�����[�v
void CooperationBMain(void)
{
	s_pNowProcess();
}