/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SelectCountry.c

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

#define MENU_DISPLAY_SIZE					7

// �X�N���[���o�[�֌W
#define BAR_ZERO_X							230
#define BAR_ZERO_Y							48
#define BAR_HEIGHT_MAX						107
#define BAR_HEIGHT_MIN						SQUARE_SIZE
#define BAR_WIDTH							SQUARE_SIZE
#define BAR_BUTTON_HEIGHT					SQUARE_SIZE
#define BAR_BUTTON_WIDTH					SQUARE_SIZE
#define SQUARE_SIZE							11
#define BAR_LOOSENESS						2
#define BAR_OFFSET							2				// �\����"��"�e�L�X�g���g���Ă���̂ŁA�^�b�`���W��␳����ړI��OFFSET

// �{�^���������ł̃J�[�\�������ړ��֌W
#define START_AUTOMOVE_COUNT				30
#define AUTOMOVE_INTERVAL					5

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------
static TWLCountryCode s_countryCode;										// ���R�[�h
static TWLRegion s_regionCode;											// ���[�W����

static u16 s_list_start, s_list_end;
static u16 s_menu_display_start;

static const u16 *s_pStrCountry[MENU_DISPLAY_SIZE];

static int s_list_size;
static int s_bar_height;
static double s_dots_per_item;

// const data  -----------------------------------------
extern const u16 *const s_pStrCountryName[];
extern const u32 region_country_mapping[TWL_REGION_MAX];

static MenuPos s_countryPos[MENU_DISPLAY_SIZE] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
};

static MenuParam countrySel = {
	MENU_DISPLAY_SIZE,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_countryPos[ 0 ],
	(const u16 **)&s_pStrCountry,
};

//======================================================
// function's description
//======================================================

// �X�N���[�����j���[�̏�����
static void InitScrollMenuList( void )
{
	int l;
	// ���j���[�ɕ\�����鍑�����X�g�S�̂̍ŏ��ƍŌ���}�b�s���O�f�[�^����擾
	s_list_start = (u16)(region_country_mapping[s_regionCode] >> 16);
	s_list_end = (u16)(region_country_mapping[s_regionCode]);
	if(s_list_start > s_list_end) OS_Panic("selectCountry.c:s_list_start>s_list_end!");
	
	s_list_size = s_list_end - s_list_start + 1;
	
	// ��ʂɕ\������ő區�ڐ������A�������X�g�����������H
	countrySel.num = (MENU_DISPLAY_SIZE < s_list_size) ? MENU_DISPLAY_SIZE : s_list_size ;
	
	// �ݒ肳��Ă��������R�[�h�����X�g�͈͂ɓ����Ă��Ȃ���΃f�t�H���g�l�ɂ���
	if(s_countryCode < s_list_start || s_list_end < s_countryCode)
	{
		s_countryCode = (TWLCountryCode)s_list_start;
	}
	
	// ���ۂɕ\������͈͂̒���
	s_menu_display_start = s_countryCode;
	if(s_countryCode + countrySel.num - 1 > s_list_end)
	{
		s_menu_display_start = (u16)(s_list_end + 1 - countrySel.num);
	}
	
	// ���ۂɕ\�����鍑���̂݃��X�g��
	for(l=0; l<countrySel.num;l++)
	{
		s_pStrCountry[l] = s_pStrCountryName[s_menu_display_start + l];
	}
	
	// �X�N���[���o�[
	s_bar_height = BAR_HEIGHT_MAX - (s_list_size - countrySel.num);
	s_dots_per_item = 1;
	if(s_bar_height < BAR_HEIGHT_MIN){
		s_bar_height = BAR_HEIGHT_MIN;
		s_dots_per_item = (double)(BAR_HEIGHT_MAX-BAR_HEIGHT_MIN)/(s_list_size - countrySel.num);
	}
}

// �����ݒ�̏�����
void SelectCountryInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COUNTRY SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	if( g_initialSet ) {
		PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Select country." );
	}
	
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// �ݒ�ς݃��[�W�����ƍ����R�[�h�̎擾
	if( !SYSM_IsValidTSD() ||
		( TSD_GetRegion() >= TWL_REGION_MAX ) ) {
		s_regionCode = (TWLRegion)TWL_DEFAULT_REGION;
	}else {
		s_regionCode = (TWLRegion)TSD_GetRegion();
	}
	
	if( !SYSM_IsValidTSD() ||
		( TSD_GetCountry() >= TWL_COUNTRY_MAX ) ) {
		s_countryCode = (TWLCountryCode)0;
	}else {
		s_countryCode = TSD_GetCountry();
	}
	
	// �X�N���[�����j���[�̏�����
	InitScrollMenuList();
	
	DrawMenu( (u16)(s_countryCode - s_menu_display_start), &countrySel );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �L�[���͂ɂ��J�[�\���ړ��̏���
static void MoveCursorByKey( void )
{
	BOOL pad_cont = FALSE;
	static int pad_count = 0;
	
	if( pad.cont & PAD_KEY_DOWN ) {								// �J�[�\���̈ړ�
		if(pad_count == 0 || (pad_count>=START_AUTOMOVE_COUNT && pad_count%AUTOMOVE_INTERVAL==0))
			if( s_countryCode < s_list_end ) s_countryCode++;
		pad_cont = TRUE;
	}
	if( pad.cont & PAD_KEY_UP ) {
		if(pad_count == 0 || (pad_count>=START_AUTOMOVE_COUNT && pad_count%AUTOMOVE_INTERVAL==0))
			if( s_countryCode > s_list_start ) s_countryCode--;
		pad_cont = TRUE;
	}
	if( pad_cont ) pad_count++;
	else pad_count = 0;
	pad_cont = FALSE;
	
	// �L�[���͌�A�\������鍀�ڂ̒���
	if( s_countryCode < s_menu_display_start ) s_menu_display_start = s_countryCode;
	else if( s_menu_display_start + countrySel.num - 1 < s_countryCode ) s_menu_display_start = (u16)(s_countryCode - countrySel.num + 1);
}

// �ȈՃX�N���[���o�[�̃{�^���ɂ��X�N���[��
static void MoveCursorByScrollBarButton( void )
{
	static int tpd_count = 0;
	BOOL tpd_cont = FALSE;
	
	if(tpd.disp.touch)
	{
		if( WithinRangeTP(BAR_ZERO_X,BAR_ZERO_Y+BAR_HEIGHT_MAX+BAR_OFFSET,BAR_ZERO_X+BAR_BUTTON_WIDTH,BAR_ZERO_Y+BAR_HEIGHT_MAX+BAR_OFFSET+BAR_BUTTON_HEIGHT,&tpd.disp) ) {//down
			if(tpd_count == 0 || (tpd_count>=START_AUTOMOVE_COUNT && tpd_count%AUTOMOVE_INTERVAL==0))
				if( s_countryCode < s_list_end ) s_menu_display_start++;
			tpd_cont = TRUE;
		}
		if( WithinRangeTP(BAR_ZERO_X,BAR_ZERO_Y-BAR_BUTTON_HEIGHT+BAR_OFFSET,BAR_ZERO_X+BAR_BUTTON_WIDTH,BAR_ZERO_Y+BAR_OFFSET,&tpd.disp) ) {//up
			if(tpd_count == 0 || (tpd_count>=START_AUTOMOVE_COUNT && tpd_count%AUTOMOVE_INTERVAL==0))
				if( s_countryCode > s_list_start ) s_menu_display_start--;
			tpd_cont = TRUE;
		}
	}
	if( tpd_cont ) tpd_count++;
	else tpd_count = 0;
	tpd_cont = FALSE;
}

// �ȈՃX�N���[���o�[�ɂ��X�N���[��
static void MoveCursorByScrollBar( void )
{
	// �ȈՃX�N���[���o�[�̃{�^���ɂ��X�N���[��
	MoveCursorByScrollBarButton();
	
	// �ȈՃX�N���[���o�[�ɂ��X�N���[��
	{
		static BOOL holding = FALSE;
		static int dy;
		int bar_top = (int)(BAR_ZERO_Y+s_dots_per_item * (s_menu_display_start - s_list_start));
		if(tpd.disp.touch)
		{
			if(holding)
			{
				if ( tpd.disp.y - dy < bar_top - BAR_LOOSENESS)
				{
					bar_top = tpd.disp.y - dy + BAR_LOOSENESS;
				}
				else if ( tpd.disp.y - dy > bar_top + BAR_LOOSENESS)
				{
					bar_top = tpd.disp.y - dy - BAR_LOOSENESS;
				}
				s_menu_display_start = (u16)(((bar_top - BAR_ZERO_Y)/s_dots_per_item) + s_list_start);
			}
			else if(WithinRangeTP(BAR_ZERO_X, bar_top+BAR_OFFSET,BAR_ZERO_X + BAR_WIDTH,bar_top+BAR_OFFSET+s_bar_height,&tpd.disp))
			{
				holding = TRUE;
				dy = tpd.disp.y - bar_top;
			}
		}
		else
		{
			holding = FALSE;
		}
	}
	
	// �^�b�`�p�b�h�ɂ��X�N���[����A�\������鍀�ڂ̒���
	if( s_menu_display_start + countrySel.num - 1 > s_list_end ) s_menu_display_start = (u16)(s_list_end - countrySel.num + 1);
	if( s_menu_display_start < s_list_start ) s_menu_display_start = s_list_start;
	if( s_countryCode < s_menu_display_start ) s_countryCode = (TWLCountryCode)s_menu_display_start;
	else if( s_menu_display_start + countrySel.num - 1 < s_countryCode ) s_countryCode = (TWLCountryCode)(s_menu_display_start + countrySel.num - 1);
}

// ���I����ʂ̕`�揈��
static void DrawCountryMain( void )
{
	int l;
	// ���ۂɕ\�����鍑���̂݃��X�g��
	for(l=0; l<countrySel.num;l++)
	{
		s_pStrCountry[l] = s_pStrCountryName[s_menu_display_start + l];
	}
	
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	// �ȈՃX�N���[���o�[�\��
	{
		PutStringUTF16( BAR_ZERO_X, BAR_ZERO_Y-BAR_BUTTON_HEIGHT, TXT_UCOLOR_G0, (const u16 *)L"��" );
		for(l=0; l<s_bar_height-SQUARE_SIZE; l+=SQUARE_SIZE)
		{
			PutStringUTF16( BAR_ZERO_X, (int)(l+BAR_ZERO_Y+s_dots_per_item * (s_menu_display_start - s_list_start)), TXT_UCOLOR_G2, (const u16 *)L"��" );
		}
		PutStringUTF16( BAR_ZERO_X, (int)(BAR_ZERO_Y+s_bar_height-SQUARE_SIZE+s_dots_per_item * (s_menu_display_start - s_list_start)), TXT_UCOLOR_G2, (const u16 *)L"��" );
		PutStringUTF16( BAR_ZERO_X, BAR_ZERO_Y+BAR_HEIGHT_MAX, TXT_UCOLOR_G0, (const u16 *)L"��" );
	}
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COUNTRY SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	DrawMenu( (u16)(s_countryCode - s_menu_display_start), &countrySel );
}

// �����I��
int SelectCountryMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	static u16 selecteditem;
	
	ReadTP();												// TP���͂̎擾

	// �L�[���͂ɂ��J�[�\���ړ��̏���
	MoveCursorByKey();
	
	// �ȈՃX�N���[���o�[�ɂ��X�N���[��
	MoveCursorByScrollBar();
	
	// �^�b�`�p�b�h�ɂ�郁�j���[���ڂ̑I��
	selecteditem = (u16)(s_countryCode - s_menu_display_start);
	tp_select = SelectMenuByTP( (u16 *)&selecteditem, &countrySel );
	s_countryCode = (TWLCountryCode)(s_menu_display_start + selecteditem);
	
	// �`��
	DrawCountryMain();
	
	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// ���j���[���ڂւ̕���
		TSD_SetCountry( s_countryCode );						// ���R�[�h�ݒ�
		//TSD_SetFlagCountry( TRUE );							// ���R�[�h���̓t���O�𗧂Ă�
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


