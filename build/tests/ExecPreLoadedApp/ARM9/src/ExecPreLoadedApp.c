/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - nandApp - ExecPreLoadedApp
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
#include <twl/nam.h>
#include "misc_simple.h"
#include "ExecPreLoadedApp.h"

// define data------------------------------------------

#define MLEP_MENU_ELEMENT_NUM			2						// ���j���[�̍��ڐ�

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------

// static variable -------------------------------------
static u16 s_csr = 0;
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const char *s_pStrMenu[ MLEP_MENU_ELEMENT_NUM ] = 
{
	"load app and restart",
	"return to launcher",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  3,   6 },
	{ TRUE,  3,   8 },
};

static const MenuParam s_menuParam = {
	MLEP_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const char **)&s_pStrMenu,
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
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "ExecPreLoadedApp");
    // ���j���[����
	myDp_DrawMenu( s_csr, MAIN_SCREEN, &s_menuParam );
}

static void MenuInit( void )
{
	FS_Init(3);
	GX_DispOff();
 	GXS_DispOff();
	
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "ExecPreLoadedApp");
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}


static BOOL LoadTitle( void )
{
	// ���[�h
	if( !OS_SetRelocateInfoAndLoadApplication( "rom:/data/simpleapp.srl" ) )
	{
		return FALSE;
	}
	
    return TRUE;
}

static void MenuScene(void)
{
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_MEMORY, TRUE, FALSE, TRUE, FALSE, 0};
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( MYPAD_IS_TRIG(PAD_KEY_DOWN) ){									// �J�[�\���̈ړ�
		if( ++s_csr == MLEP_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( MYPAD_IS_TRIG(PAD_KEY_UP) ){
		if( --s_csr & 0x80 ) {
			s_csr=MLEP_MENU_ELEMENT_NUM - 1;
		}
	}
	
   	DrawMenuScene();
	
	if( MYPAD_IS_TRIG(PAD_BUTTON_A) ) {				// ���j���[���ڂւ̕���
		if( s_menuPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					//�A�v���N��
					if(LoadTitle())
						OS_SetLauncherParamAndResetHardware( 0, 0x00030004534d504c, &tempflag ); // SMPL
					break;
				case 1:
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
	MenuInit();
}

// ���C�����[�v
void ExecPreLoadedAppMain(void)
{
	s_pNowProcess();
}
