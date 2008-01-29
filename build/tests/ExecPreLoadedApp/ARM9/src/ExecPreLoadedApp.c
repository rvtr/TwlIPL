/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ExecPreLoadedApp.c

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
#include "ExecPreLoadedApp.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define MLEP_MENU_ELEMENT_NUM			7						// ���j���[�̍��ڐ�

#define PARAM_LENGTH	10

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------
static u16 s_csr = 0;
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const u16 *s_pStrMenu[ MLEP_MENU_ELEMENT_NUM ] = 
{
	L"�Ĕz�u�`�F�b�J0�i�Ĕz�u�����j",
	L"�Ĕz�u�`�F�b�J1�i�G���[�j",
	L"�Ĕz�u�`�F�b�J2�i�����R�s�[1�j",
	L"�Ĕz�u�`�F�b�J3�i�����R�s�[2�j",
	L"�Ĕz�u�`�F�b�J4�i�G���[�j",
	L"�Ĕz�u�`�F�b�J5�iWRAM�֔z�u�j",
	L"�����`���[�ɖ߂�",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
	{ TRUE,  4 * 8,  20 * 8 },
};

static const MenuParam s_menuParam = {
	MLEP_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const u16 **)&s_pStrMenu,
};
									
//======================================================
// �e�X�g�v���O����
//======================================================

// ============================================================================
//
// �A�v���N��
//
// ============================================================================

static void DrawMenuScene( void )
{
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecPreLoadedApp");
    // ���j���[����
	DrawMenu( s_csr, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecPreLoadedApp");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}


static BOOL LoadTitle( NAMTitleId bootTitleID )
{
	// ���[�h
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
    NAM_GetTitleBootContentPath(path, bootTitleID);

	if( !OS_SetRelocateInfoAndLoadApplication( path ) )
	{
		return FALSE;
	}
	
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
    return TRUE;
}

static void MenuScene(void)
{
	BOOL tp_select = FALSE;
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, TRUE, FALSE, 0};
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		if( ++s_csr == MLEP_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=MLEP_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_menuParam );
	
   	DrawMenuScene();
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// ���j���[���ڂւ̕���
		if( s_menuPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					//�A�v���N��
					if(LoadTitle(0x0003000452434b30))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b30, &tempflag ); // RCK0
					break;
				case 1:
					//�A�v���N��
					if(LoadTitle(0x0003000452434b31))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b31, &tempflag ); // RCK1
					break;
				case 2:
					//�A�v���N��
					if(LoadTitle(0x0003000452434b32))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b32, &tempflag ); // RCK2
					break;
				case 3:
					//�A�v���N��
					if(LoadTitle(0x0003000452434b33))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b33, &tempflag ); // RCK3
					break;
				case 4:
					//�A�v���N��
					if(LoadTitle(0x0003000452434b34))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b34, &tempflag ); // RCK4
					break;
				case 5:
					//�A�v���N��
					if(LoadTitle(0x0003000452434b35))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b35, &tempflag ); // RCK5
					break;
				case 6:
					OS_SetLauncherParamAndResetHardware( 0, NULL, &tempflag );
					//�ċN��
					break;
			}
		}
	}
}

// ������
void ExecPreLoadedAppInit( void )
{
	ChangeUserColor( LCFG_TSD_GetUserColor() );
	MenuInit();
}

// ���C�����[�v
void ExecPreLoadedAppMain(void)
{
	s_pNowProcess();
}
