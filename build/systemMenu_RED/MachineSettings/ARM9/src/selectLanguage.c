/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SelectLanguage.c

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
static int s_lang;											// ����I�����̉��Ԗڂ�I�����Ă��邩
static LCFGTWLRegion s_regionCode;											// ���[�W�����R�[�h

static const u16* s_pStrLanguage[LCFG_TWL_LANG_CODE_MAX];
static LCFGTWLLangCode s_langCodeList[LCFG_TWL_LANG_CODE_MAX];

// const data  -----------------------------------------
static const u16 region_lang_Mapping[LCFG_TWL_REGION_MAX] =
{
	LCFG_TWL_LANG_BITMAP_JAPAN,
	LCFG_TWL_LANG_BITMAP_AMERICA,
	LCFG_TWL_LANG_BITMAP_EUROPE,
	LCFG_TWL_LANG_BITMAP_AUSTRALIA,
	LCFG_TWL_LANG_BITMAP_CHINA,
	LCFG_TWL_LANG_BITMAP_KOREA
};

static const u16 *const s_pStrLanguageData[LCFG_TWL_LANG_CODE_MAX] = {
	(const u16 *)L"���{��",
	(const u16 *)L"English ",
	(const u16 *)L"Francais",
	(const u16 *)L"Deutsch ",
	(const u16 *)L"Italiano",
	(const u16 *)L"Espanol ",
	(const u16 *)L"������i���j",
	(const u16 *)L"�؍���i���j"
};

static MenuPos s_languagePos[LCFG_TWL_LANG_CODE_MAX] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
	{ TRUE,  4 * 8,  20 * 8 }
};

static MenuParam langSel = {
	LCFG_TWL_LANG_CODE_MAX,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_languagePos[ 0 ],
	(const u16 **)&s_pStrLanguage,
};


//======================================================
// function's description
//======================================================

// ����ݒ�̏�����
void SelectLanguageInit( void )
{
    int l;
    u16 temp_langCode = 0;
    BOOL in_list_flag = FALSE;
	int lang_count = 0;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"LANGUAGE SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���[�W�����̎擾
	s_regionCode = (LCFGTWLRegion)LCFG_THW_GetRegion();
	
	// ����̎擾
	if( !g_isValidTSD ||
		( LCFG_TSD_GetLanguage() >= LCFG_TWL_LANG_CODE_MAX ) ) {
		temp_langCode = LCFG_TWL_LANG_ENGLISH;
	}else {
		temp_langCode = LCFG_TSD_GetLanguage();
	}
	
	// ���[�W����-����}�b�s���O��񂩂�A���݂̃��[�W�����őI���ł��錾������X�g�A�b�v
	s_lang = 0;
	for(l=0; l<LCFG_TWL_LANG_CODE_MAX; l++)
	{
		if( ( 0x0001 << l ) & region_lang_Mapping[s_regionCode] )
		{
			s_pStrLanguage[lang_count] = s_pStrLanguageData[l];
			s_langCodeList[lang_count] = (LCFGTWLLangCode)l;
			if(temp_langCode == l)
			{
				s_lang = lang_count;
			}
			lang_count++;
		}
	}
	
	langSel.num = lang_count;
	
	DrawMenu( (u16)s_lang, &langSel );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// ����I��
int SelectLanguageMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();												// TP���͂̎擾
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ) {								// �J�[�\���̈ړ�
		if( ++s_lang == langSel.num ) {
			s_lang = (LCFGTWLLangCode)0;
		}
	}
	if( pad.trg & PAD_KEY_UP ) {
		if( --s_lang < 0 ) {
			s_lang = (LCFGTWLLangCode)( langSel.num - 1 );
		}
	}
	tp_select = SelectMenuByTP( (u16 *)&s_lang, &langSel );
	DrawMenu( (u16)s_lang, &langSel );
	
	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// ���j���[���ڂւ̕���
		LCFG_TSD_SetLanguage( s_langCodeList[s_lang] );
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL�ݒ�f�[�^�t�@�C���ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		if( !MY_WriteTWLSettings() ) {
			OS_TPrintf( "TWL settings write failed.\n" );
		}
		
		MachineSettingInit();
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		MachineSettingInit();
		return 0;
	}
	
	return 0;
}


