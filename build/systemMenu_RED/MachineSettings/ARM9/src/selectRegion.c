/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SelectRegion.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-19#$
  $Rev: 215 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "misc.h"
#include "MachineSetting.h"

// define data------------------------------------------
#define CANCEL_BUTTON_TOP_X					( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( (CANCEL_BUTTON_TOP_X + 8 ) * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( (CANCEL_BUTTON_TOP_Y + 2 ) * 8 )

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------
static TWLRegion s_regionCode;											// ���[�W����

// const data  -----------------------------------------
static const u16 *const s_pStrRegion[] = {
	(const u16 *)L"NCL",
	(const u16 *)L"NOA",
	(const u16 *)L"NOE",
	(const u16 *)L"NAL ",
	(const u16 *)L"IQue",
	(const u16 *)L"NOK ",
};

static MenuPos s_regionPos[] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
};

static const MenuParam regionSel = {
	6,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_regionPos[ 0 ],
	(const u16 **)&s_pStrRegion,
};

TWLLangCode default_lang_list[TWL_REGION_MAX] = 
{
	TWL_LANG_JAPANESE,
	TWL_LANG_ENGLISH,
	TWL_LANG_ENGLISH,
	TWL_LANG_ENGLISH,
	TWL_LANG_SIMP_CHINESE,
	TWL_LANG_KOREAN
};

TWLCountryCode default_country_list[TWL_REGION_MAX] = 
{
	TWL_COUNTRY_JAPAN,
	TWL_COUNTRY_Anguilla,
	TWL_COUNTRY_ALBANIA,
	TWL_COUNTRY_ALBANIA,
	TWL_COUNTRY_CHINA,
	TWL_COUNTRY_SOUTH_KOREA
};

//======================================================
// function's description
//======================================================

// ���[�W�����ݒ�̏�����
void SelectRegionInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"REGION SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	if( g_initialSet ) {
		PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Select region." );
	}
	
	if( !SYSM_IsValidTSD() ||
		( TSD_GetRegion() >= TWL_REGION_MAX ) ) {
		s_regionCode = (TWLRegion)TWL_DEFAULT_REGION;
	}else {
		s_regionCode = (TWLRegion)TSD_GetRegion();
	}
	
	DrawMenu( (u16)s_regionCode, &regionSel );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// ���[�W�����I��
int SelectRegionMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();												// TP���͂̎擾
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ) {								// �J�[�\���̈ړ�
		if( ++s_regionCode == TWL_REGION_MAX ) {
			s_regionCode = (TWLRegion)0;
		}
	}
	if( pad.trg & PAD_KEY_UP ) {
		if( --s_regionCode < 0 ) {
			s_regionCode = (TWLRegion)( TWL_REGION_MAX - 1 );
		}
	}
	tp_select = SelectMenuByTP( (u16 *)&s_regionCode, &regionSel );
	DrawMenu( (u16)s_regionCode, &regionSel );
	
	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// ���j���[���ڂւ̕���
		TSD_SetRegion( s_regionCode );
		// TSD_SetFlagRegion( TRUE );							// Region���̓t���O�𗧂Ă�
		TSD_SetLanguage( default_lang_list[s_regionCode] );		// �f�t�H���g����ɋ����ݒ�
		TSD_SetCountry( default_country_list[s_regionCode] );	// �f�t�H���g���ɋ����ݒ�
		TSD_SetFlagLanguage( TRUE );							// Language���̓t���O�𗧂Ă�
		//TSD_SetFlagCountry( TRUE );							// Country���̓t���O�𗧂Ă�
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL�ݒ�f�[�^�t�@�C���ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)SYSM_WriteTWLSettingsFile();
		
		MachineSettingInit();
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		MachineSettingInit();
		return 0;
	}
	
	return 0;
}


