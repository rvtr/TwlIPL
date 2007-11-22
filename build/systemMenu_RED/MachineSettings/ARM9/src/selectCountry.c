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

#define MENU_DISPLAY_SIZE 7

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------
static TWLCountryCode s_countryCode;										// ���R�[�h
static TWLRegion s_regionCode;											// ���[�W����

static u16 list_start, list_end;
static u16 s_menu_display_start;

static const u16 *s_pStrCountry[MENU_DISPLAY_SIZE];

// const data  -----------------------------------------
static const u16 *const s_pStrCountryName[] = {
	(const u16 *)L"UNDEFINED",
	(const u16 *)L"JAPAN",					// ���{
	
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	
	// USA���[�W����
	(const u16 *)L"Anguilla",        // �A���M��
	(const u16 *)L"ANTIGUA_AND_BARBUDA",    // �A���e�B�O�A�E�o�[�u�[�_
	(const u16 *)L"ARGENTINA",		// �A���[���`��
	(const u16 *)L"ARUBA",                  // �A���o
	(const u16 *)L"BAHAMAS",                // �o�n�}
	(const u16 *)L"BARBADOS",               // �o���o�h�X
	(const u16 *)L"BELIZE",                 // �x���[�Y
	(const u16 *)L"BOLIVIA",                // �{���r�A
	(const u16 *)L"BRAZIL",                 // �u���W��
	(const u16 *)L"BRITISH_VIRGIN_ISLANDS", // �p�̃��@�[�W������
	(const u16 *)L"CANADA",                 // �J�i�_
	(const u16 *)L"CAYMAN_ISLANDS",         // �P�C�}������
	(const u16 *)L"CHILE",       // �`��
	(const u16 *)L"COLOMBIA",               // �R�����r�A
	(const u16 *)L"COSTA_RICA",             // �R�X�^���J
	(const u16 *)L"DOMINICA",               // �h�~�j�J��
	(const u16 *)L"DOMINICAN_REPUBLIC",     // �h�~�j�J���a��
	(const u16 *)L"ECUADOR",                // �G�N�A�h��
	(const u16 *)L"EL_SALVADOR",            // �G���T���o�h��
	(const u16 *)L"FRENCH_GUIANA",          // �t�����X�̃M�A�i
	(const u16 *)L"GRENADA",                // �O���i�_
	(const u16 *)L"GUADELOUPE",             // �O�A�h���[�v
	(const u16 *)L"GUATEMALA",       // �O�A�e�}��
	(const u16 *)L"GUYANA",                 // �K�C�A�i
	(const u16 *)L"HAITI",                  // �n�C�`
	(const u16 *)L"HONDURAS",               // �z���W�����X
	(const u16 *)L"JAMAICA",                // �W���}�C�J
	(const u16 *)L"MARTINIQUE",             // �}���e�B�j�[�N
	(const u16 *)L"MEXICO",                 // ���L�V�R
	(const u16 *)L"MONTSERRAT",             // �����g�Z���g
	(const u16 *)L"NETHERLANDS_ANTILLES",   // �I�����_�̃A���e�B��
	(const u16 *)L"NICARAGUA",              // �j�J���O�A
	(const u16 *)L"PANAMA",       // �p�i�}
	(const u16 *)L"PARAGUAY",               // �p���O�A�C
	(const u16 *)L"PERU",                   // �y���[
	(const u16 *)L"ST_KITTS_AND_NEVIS",     // �Z���g�L�b�c�E�l�C�r�X
	(const u16 *)L"ST_LUCIA",               // �Z���g���V�A
	(const u16 *)L"ST_VINCENT_AND_THE_GRENADINES",  // �Z���g�r���Z���g�E�O���i�f�B�[��
	(const u16 *)L"SURINAME",               // �X���i��
	(const u16 *)L"TRINIDAD_AND_TOBAGO",    // �g���j�_�[�h�E�g�o�S
	(const u16 *)L"TURKS_AND_CAICOS_ISLANDS",   // �^�[�N�X�E�J�C�R�X����
	(const u16 *)L"UNITED_STATES",          // �A�����J
	(const u16 *)L"URUGUAY",       // �E���O�A�C
	(const u16 *)L"US_VIRGIN_ISLANDS",      // �ė̃o�[�W������
	(const u16 *)L"VENEZUELA",              // �x�l�Y�G��
	
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    // EUR", NAL ���[�W����
	(const u16 *)L"ALBANIA",       // �A���o�j�A
	(const u16 *)L"AUSTRALIA",              // �I�[�X�g�����A
	(const u16 *)L"AUSTRIA",                // �I�[�X�g���A
	(const u16 *)L"BELGIUM",                // �x���M�[
	(const u16 *)L"BOSNIA_AND_HERZEGOVINA", // �{�X�j�A�E�w���c�F�S�r�i
	(const u16 *)L"BOTSWANA",               // �{�c���i
	(const u16 *)L"BULGARIA",       // �u���K���A
	(const u16 *)L"CROATIA",                // �N���A�`�A
	(const u16 *)L"CYPRUS",                 // �L�v���X
	(const u16 *)L"CZECH_REPUBLIC",         // �`�F�R
	(const u16 *)L"DENMARK",                // �f���}�[�N
	(const u16 *)L"ESTONIA",                // �G�X�g�j�A
	(const u16 *)L"FINLAND",                // �t�B�������h
	(const u16 *)L"FRANCE",                 // �t�����X
	(const u16 *)L"GERMANY",                // �h�C�c
	(const u16 *)L"GREECE",                 // �M���V��
	(const u16 *)L"HUNGARY",       // �n���K���[
	(const u16 *)L"ICELAND",                // �A�C�X�����h
	(const u16 *)L"IRELAND",                // �A�C�������h
	(const u16 *)L"ITALY",                  // �C�^���A
	(const u16 *)L"LATVIA",                 // ���g�r�A
	(const u16 *)L"LESOTHO",                // ���\�g
	(const u16 *)L"LIECHTENSTEIN",          // ���q�e���V���^�C��
	(const u16 *)L"LITHUANIA",              // ���g�A�j�A
	(const u16 *)L"LUXEMBOURG",             // ���N�Z���u���N
	(const u16 *)L"MACEDONIA",              // �}�P�h�j�A
	(const u16 *)L"MALTA",       // �}���^
	(const u16 *)L"MONTENEGRO",             // �����e�l�O��
	(const u16 *)L"MOZAMBIQUE",             // ���U���r�[�N
	(const u16 *)L"NAMIBIA",                // �i�~�r�A
	(const u16 *)L"NETHERLANDS",            // �I�����_
	(const u16 *)L"NEW_ZEALAND",            // �j���[�W�[�����h
	(const u16 *)L"NORWAY",                 // �m���E�F�[
	(const u16 *)L"POLAND",                 // �|�[�����h
	(const u16 *)L"PORTUGAL",               // �|���g�K��
	(const u16 *)L"ROMANIA",                // ���[�}�j�A
	(const u16 *)L"RUSSIA",      // ���V�A
	(const u16 *)L"SERBIA",                 // �Z���r�A
	(const u16 *)L"SLOVAKIA",               // �X���o�L�A
	(const u16 *)L"SLOVENIA",               // �X���x�j�A
	(const u16 *)L"SOUTH_AFRICA",           // ��A�t���J
	(const u16 *)L"SPAIN",                  // �X�y�C��
	(const u16 *)L"SWAZILAND",              // �X���W�����h
	(const u16 *)L"SWEDEN",                 // �X�E�F�[�f��
	(const u16 *)L"SWITZERLAND",            // �X�C�X
	(const u16 *)L"TURKEY",                 // �g���R
	(const u16 *)L"UNITED_KINGDOM",   // �C�M���X
	(const u16 *)L"ZAMBIA",                 // �U���r�A
	(const u16 *)L"ZIMBABWE",               // �W���o�u�G

	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    // TWN���[�W����
    (const u16 *)L"TAIWAN",      // ��p
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // KOR���[�W����
    (const u16 *)L"SOUTH_KOREA",      // �؍�
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // HKG���[�W�����iWii�̍����X�g�ɑ��݁j
    (const u16 *)L"HONG_KONG",      // �z���R��
    (const u16 *)L"MACAU",                  // �}�J�I
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // ASI���[�W�����iWii�̍����X�g�ɑ��݁j
    (const u16 *)L"INDONESIA",      // �C���h�l�V�A
    
    // USA���[�W����
    (const u16 *)L"SINGAPORE",      // �V���K�|�[��
    
    // ASI���[�W�����i�Ăсj
    (const u16 *)L"THAILAND",      // �^�C
    (const u16 *)L"PHILIPPINES",            // �t�B���s��
    (const u16 *)L"MALAYSIA",               // �}���[�V�A

	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // ����`���[�W�����iIQue���[�W�����H�j
    (const u16 *)L"CHINA",      // ����
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // USA���[�W����
    (const u16 *)L"UAE",      // �A���u�񒷍��A�M
    
    // ����`���[�W����
    (const u16 *)L"INDIA",      // �C���h
    (const u16 *)L"EGYPT",      // �G�W�v�g
    (const u16 *)L"OMAN",                   // �I�}�[��
    (const u16 *)L"QATAR",                  // �J�^�[��
    (const u16 *)L"KUWAIT",                 // �N�E�F�[�g
    (const u16 *)L"SAUDI_ARABIA",           // �T�E�W�A���r�A
    (const u16 *)L"SYRIA",                  // �V���A
    (const u16 *)L"BAHRAIN",                // �o�[���[��
    (const u16 *)L"JORDAN",                 // �����_��
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//180
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//190
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//200
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//210
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//220
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//230
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//240
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//250
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    (const u16 *)L"OTHERS",
    (const u16 *)L"UNKNOWN"
};

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

static u32 region_country_mapping[TWL_REGION_MAX] = 
{
	TWL_COUNTRY_MAPPING_JAPAN,
	TWL_COUNTRY_MAPPING_AMERICA,
	TWL_COUNTRY_MAPPING_EUROPE,
	TWL_COUNTRY_MAPPING_AUSTRALIA,
	TWL_COUNTRY_MAPPING_CHINA,
	TWL_COUNTRY_MAPPING_KOREA
};

//======================================================
// function's description
//======================================================

// �����ݒ�̏�����
void SelectCountryInit( void )
{
    int l;
    BOOL in_list_flag = FALSE;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COUNTRY SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	if( g_initialSet ) {
		PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Select country." );
	}
	
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

	// ���j���[�ɕ\�����鍑�����X�g�S�̂̍ŏ��ƍŌ���}�b�s���O�f�[�^����擾
	list_start = (u16)(region_country_mapping[s_regionCode] >> 16);
	list_end = (u16)(region_country_mapping[s_regionCode]);
	if(list_start > list_end) OS_Panic("selectCountry.c:list_start>list_end!");
	
	// ��ʂɕ\������ő區�ڐ������A�������X�g�����������H
	countrySel.num = (MENU_DISPLAY_SIZE < list_end - list_start + 1) ? MENU_DISPLAY_SIZE : list_end - list_start + 1 ;
	
	// �ݒ肳��Ă��������R�[�h�����X�g�͈͂ɓ����Ă��Ȃ���΃f�t�H���g�l�ɂ���
	if(s_countryCode < list_start || list_end < s_countryCode)
	{
		s_countryCode = (TWLCountryCode)list_start;
	}
	
	// ���ۂɕ\������͈͂̒���
	s_menu_display_start = s_countryCode;
	if(s_countryCode + countrySel.num - 1 > list_end)
	{
		s_menu_display_start = (u16)(list_end + 1 - countrySel.num);
	}
	
	// ���ۂɕ\�����鍑���̂݃��X�g��
	for(l=0; l<countrySel.num;l++)
	{
		s_pStrCountry[l] = s_pStrCountryName[s_menu_display_start + l];
	}
	
	DrawMenu( (u16)(s_countryCode - s_menu_display_start), &countrySel );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// �����I��
int SelectCountryMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	static u16 selecteditem;
	int l;
	static int padcount = 0;
	BOOL padcont = FALSE;
	
	ReadTP();												// TP���͂̎擾
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.cont & PAD_KEY_DOWN ) {								// �J�[�\���̈ړ�
		if(padcount == 0 || (padcount>29 && padcount%5==0))
			if( s_countryCode < list_end ) s_countryCode++;
		padcont = TRUE;
	}
	if( pad.cont & PAD_KEY_UP ) {
		if(padcount == 0 || (padcount>29 && padcount%5==0))
			if( s_countryCode > list_start ) s_countryCode--;
		padcont = TRUE;
	}
	if( padcont ) padcount++;
	else padcount = 0;
	padcont = FALSE;
	
	// �L�[���͌�A�\������鍀�ڂ̒���
	if( s_countryCode < s_menu_display_start ) s_menu_display_start = s_countryCode;
	else if( s_menu_display_start + countrySel.num - 1 < s_countryCode ) s_menu_display_start = (u16)(s_countryCode - countrySel.num + 1);
	
	// �ȈՃX�N���[���o�[�ɂ��X�N���[��
	
	
	// �^�b�`�p�b�h�ɂ��X�N���[����A�\������鍀�ڂ̒���
	if( s_countryCode < s_menu_display_start ) s_countryCode = (TWLCountryCode)s_menu_display_start;
	else if( s_menu_display_start + countrySel.num - 1 < s_countryCode ) s_countryCode = (TWLCountryCode)(s_menu_display_start + countrySel.num - 1);
	
	// ���ۂɕ\�����鍑���̂݃��X�g��
	for(l=0; l<countrySel.num;l++)
	{
		s_pStrCountry[l] = s_pStrCountryName[s_menu_display_start + l];
	}
	
	// �^�b�`�p�b�h�ɂ�郁�j���[���ڂ̑I��
	selecteditem = (u16)(s_countryCode - s_menu_display_start);
	tp_select = SelectMenuByTP( (u16 *)&selecteditem, &countrySel );
	s_countryCode = (TWLCountryCode)(s_menu_display_start + selecteditem);
	
	// �`��
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	// �ȈՃX�N���[���o�[�\��
	{
		int list_size = list_end - list_start + 1;
		int bar_height = 107 - (list_size - countrySel.num);
		double dots_per_item = 1;
		if(bar_height<11){
			bar_height = 11;
			dots_per_item = (double)(107-11)/(list_size - countrySel.num);
		}
		PutStringUTF16( 10, 48-11, TXT_UCOLOR_G0, (const u16 *)L"��" );
		for(l=0; l<bar_height-11;l+=11)
		{
			PutStringUTF16( 10, (int)(l+48+dots_per_item * (s_menu_display_start - list_start)), TXT_UCOLOR_G2, (const u16 *)L"��" );
		}
		PutStringUTF16( 10, (int)(bar_height-11+48+dots_per_item * (s_menu_display_start - list_start)), TXT_UCOLOR_G2, (const u16 *)L"��" );
		PutStringUTF16( 10, 48+107, TXT_UCOLOR_G0, (const u16 *)L"��" );
	}
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COUNTRY SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	DrawMenu( (u16)(s_countryCode - s_menu_display_start), &countrySel );
	
	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// ���j���[���ڂւ̕���
		TSD_SetCountry( s_countryCode );
		//TSD_SetFlagCountry( TRUE );							// �������̓t���O�𗧂Ă�
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


