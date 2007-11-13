/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ownerInfo.c

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

// define data----------------------------------

// �\�t�g�E�F�A�L�[�{�[�hLCD�̈�
#define CLIST_LT_X							14
#define CLIST_LT_Y							40

#define CANCEL_BUTTON_TOP_X					( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( (CANCEL_BUTTON_TOP_X + 8 ) * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( (CANCEL_BUTTON_TOP_Y + 2 ) * 8 )

#define USER_INFO_MENU_ELEMENT_NUM			4						// ���[�U��񃁃j���[�̍��ڐ�

#define CHAR_LIST_CHAR_NUM					120
#define CHAR_LIST_MODE_NUM					3

// ����L�[�R�[�h
#define EOM_			(u16)0xe050
#define CODE_BUTTON_TOP_	(u16)0xe051
#define DEL_BUTTON_		(u16)0xe051
#define SPACE_BUTTON_	(u16)0xe052
#define VAR_BUTTON1_	(u16)0xe053
#define VAR_BUTTON2_	(u16)0xe054
#define OK_BUTTON_		(u16)0xe055
#define CANCEL_BUTTON_	(u16)0xe056
#define CODE_BUTTON_BOTTOM_	(u16)0xe057


	// �J�[�\��X,Y�ʒu�i�L�����P�ʁj
typedef struct CsrPos {
	u16 			x;												// x
	u16 			y;												// y
}CsrPos;

// extern data----------------------------------


// function's prototype-------------------------
static void SetNicknameInit( void );
static int SetNicknameMain( void );
static void SetBirthdayInit( void );
static int SetBirthdayMain( void );
static void SetUserColorInit( void );
static int SetUserColorMain( void );
static void SetCommentInit( void );
static int SetCommentMain( void );

// static variable------------------------------
static u16 s_csr = 0;
static const u16 *s_pStrSetting[ USER_INFO_MENU_ELEMENT_NUM ];			// ���C�����j���[�p�����e�[�u���ւ̃|�C���^���X�g
static int char_mode = 0;
static u16 s_key_csr = 0;

// const data-----------------------------------
static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM];

static const u16 *const s_pStrSettingElemTbl[ USER_INFO_MENU_ELEMENT_NUM ][ TWL_LANG_CODE_MAX ] = {
	{
		(const u16 *)L"���[�U�[�l�[��",
		(const u16 *)L"NICKNAME",
		(const u16 *)L"NICKNAME(F)",
		(const u16 *)L"NICKNAME(G)",
		(const u16 *)L"NICKNAME(I)",
		(const u16 *)L"NICKNAME(S)",
	},
	{
		(const u16 *)L"�a����",
		(const u16 *)L"BIRTHDAY",
		(const u16 *)L"BIRTHDAY(F)",
		(const u16 *)L"BIRTHDAY(G)",
		(const u16 *)L"BIRTHDAY(I)",
		(const u16 *)L"BIRTHDAY(S)",
	},
	{
		(const u16 *)L"���[�U�[�J���[",
		(const u16 *)L"USER COLOR",
		(const u16 *)L"USER COLOR(F)",
		(const u16 *)L"USER COLOR(G)",
		(const u16 *)L"USER COLOR(I)",
		(const u16 *)L"USER COLOR(S)",
	},
	{
		(const u16 *)L"�R�����g",
		(const u16 *)L"COMMENT",
		(const u16 *)L"COMMENT(F)",
		(const u16 *)L"COMMENT(G)",
		(const u16 *)L"COMMENT(I)",
		(const u16 *)L"COMMENT(S)",
	},
};

static MenuPos s_settingPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE, 4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
};


static const MenuParam s_settingParam = {
	USER_INFO_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_settingPos[ 0 ],
	(const u16 **)&s_pStrSetting,
};

static const u16 *str_button_char[CHAR_LIST_MODE_NUM] = {
									L"����",
									L"�J�i",
									L"ABC",
									};

static u16 next_char_mode[CHAR_LIST_MODE_NUM-1];

static const u16  str_button_del[] = L"DEL";
static const u16  str_button_space[] = L"SPACE";
static const u16  str_button_ok[] = L"OK";
static const u16  str_button_cancel[] = L"CANCEL";

static const u16 *str_button[] = {
									(const u16 *)str_button_del,
									(const u16 *)str_button_space,
									NULL,
									NULL,
									(const u16 *)str_button_ok,
									(const u16 *)str_button_cancel,
									};

//======================================================
// �I�[�i�[���ҏW
//======================================================

static void SetSoftKeyboardButton(int mode)
{
	int l;
	int count = 0;
	for(l=0; l<CHAR_LIST_MODE_NUM ;l++)
	{
		if(l != mode){
			str_button[2+count]=str_button_char[l];
			next_char_mode[count] = (u16)l;
			count++;
		}
	}
	char_mode = mode;
}

// �I�[�i�[���ҏW�̏�����
void SetOwnerInfoInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER INFORMATION" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	SetSoftKeyboardButton(0);
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �I�[�i�[���ҏW���j���[
int SetOwnerInfoMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		if( ++s_csr == USER_INFO_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=USER_INFO_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_settingParam );
	DrawMenu( s_csr, &s_settingParam );

	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// ���j���[���ڂւ̕���
		if( s_settingPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					SetNicknameInit();
					g_pNowProcess = SetNicknameMain;
					break;
				case 1:
					SetBirthdayInit();
					g_pNowProcess = SetBirthdayMain;
					break;
				case 2:
					SetUserColorInit();
					g_pNowProcess = SetUserColorMain;
					break;
				case 3:
					SetCommentInit();
					g_pNowProcess = SetCommentMain;
					break;
			}
		}
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		MachineSettingInit();
		return 0;
	}
	return 0;
}

// �L�[�̕\��
static void DrawCharKeys( void )
{
	int l;
	u16 code;
	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		int color=TXT_COLOR_BLACK;
		code = char_tbl[char_mode][l];
		if (s_key_csr == l) color = TXT_COLOR_RED;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
				PutStringUTF16( CLIST_LT_X + 64 + 8*8*(x%2) , CLIST_LT_Y + 15*(7+x/2) , color, str_button[x] );
			}
			else
			{
				u16 s[2];
				s[0] = code;
				s[1] = 0;
				PutStringUTF16( CLIST_LT_X + 15*(l%15) + 5*((l/5)%3) , CLIST_LT_Y + 15*(l/15) , color, s );
			}
		}
	}
}

// �L�[�̕\��
static void PushKeys( u16 code )
{
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		// ����L�[
		if(code == VAR_BUTTON1_ || code == VAR_BUTTON2_)
			SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
	}
	else
	{
		// ���ʃL�[
	}
}

// �j�b�N�l�[���ҏW�̏�����
static void SetNicknameInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"NICKNAME" );
	
	DrawCharKeys();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �j�b�N�l�[���ҏW���C��
static int SetNicknameMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_RIGHT ){									// �J�[�\���̈ړ�
		do
		{
			if(s_key_csr%15 != 14) s_key_csr++;
			else s_key_csr -= 14;
			if( s_key_csr == CHAR_LIST_CHAR_NUM ) s_key_csr -= s_key_csr%15;
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_LEFT ){
		do
		{
			if(s_key_csr%15 != 0) s_key_csr--;
			else s_key_csr += 14;
			if( s_key_csr & 0x8000 ) s_key_csr = 14;
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		do
		{
			s_key_csr += 15;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= 15*(s_key_csr/15);
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_UP ){
		do
		{
			if( s_key_csr < 15 ) s_key_csr += (CHAR_LIST_CHAR_NUM/15)*15;
			else s_key_csr -= 15;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= 15;
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	tp_select = SelectMenuByTP( &s_csr, &s_settingParam );

	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// A�L�[�������ꂽ
		PushKeys( char_tbl[char_mode][s_key_csr] );
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	if(pad.trg)
	{
	    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
		PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"NICKNAME" );
		DrawCharKeys();
	}
	
	return 0;
}

// �a�����ҏW�̏�����
static void SetBirthdayInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BIRTHDAY" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �a�����ҏW���C��
static int SetBirthdayMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		if( ++s_csr == USER_INFO_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=USER_INFO_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_settingParam );
	DrawMenu( s_csr, &s_settingParam );

	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// ���j���[���ڂւ̕���
		if( s_settingPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					SetNicknameInit();
					g_pNowProcess = SetNicknameMain;
					break;
				case 1:
					SetBirthdayInit();
					g_pNowProcess = SetBirthdayMain;
					break;
				case 2:
					SetUserColorInit();
					g_pNowProcess = SetUserColorMain;
					break;
				case 3:
					SetCommentInit();
					g_pNowProcess = SetCommentMain;
					break;
			}
		}
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	return 0;
}

// ���[�U�[�J���[�ҏW�̏�����
static void SetUserColorInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER COLOR" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// ���[�U�[�J���[�ҏW���C��
static int SetUserColorMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		if( ++s_csr == USER_INFO_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=USER_INFO_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_settingParam );
	DrawMenu( s_csr, &s_settingParam );

	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// ���j���[���ڂւ̕���
		if( s_settingPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					SetNicknameInit();
					g_pNowProcess = SetNicknameMain;
					break;
				case 1:
					SetBirthdayInit();
					g_pNowProcess = SetBirthdayMain;
					break;
				case 2:
					SetUserColorInit();
					g_pNowProcess = SetUserColorMain;
					break;
				case 3:
					SetCommentInit();
					g_pNowProcess = SetCommentMain;
					break;
			}
		}
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	return 0;
}

// �R�����g�ҏW�̏�����
static void SetCommentInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COMMENT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �R�����g�ҏW���C��
static int SetCommentMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		if( ++s_csr == USER_INFO_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=USER_INFO_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_settingParam );
	DrawMenu( s_csr, &s_settingParam );

	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// ���j���[���ڂւ̕���
		if( s_settingPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					SetNicknameInit();
					g_pNowProcess = SetNicknameMain;
					break;
				case 1:
					SetBirthdayInit();
					g_pNowProcess = SetBirthdayMain;
					break;
				case 2:
					SetUserColorInit();
					g_pNowProcess = SetUserColorMain;
					break;
				case 3:
					SetCommentInit();
					g_pNowProcess = SetCommentMain;
					break;
			}
		}
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	return 0;
}

//======================================================
// �j�b�N�l�[�����͗p�L�����e�[�u��
//======================================================

/*
	��SJIS�����𕶎��萔�Ƃ��ċL�q����ꍇ�A�ȉ��̂Q�ʂ�ŏ�ʁE���ʃR�[�h�̊i�[������
�@�@�@�t�ɂȂ��Ă��܂��̂ŁA���ӂ��邱�ƁB
	
	u8 str[] = "����������";	0x82,0xa0,0x82,0xa2...�Ə�ʃR�[�h�����ʃo�C�g�Ɋi�[�����B
	u16 code = '��';			0xa0,0x82 �Ə�ʁE���ʃR�[�h�����̂܂܊i�[�����B

*/

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM] = {
	{	// �Ђ炪��
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'�A',	L'�B',	L'�I',	L'�H',
		
		L'�u',	L'�v',	L'�`',	L'�E',	L'�[',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	SPACE_BUTTON_,	VAR_BUTTON1_,	VAR_BUTTON2_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// �J�^�J�i
		L'�A',	L'�C',	L'�E',	L'�G',	L'�I',
		L'�J',	L'�L',	L'�N',	L'�P',	L'�R',
		L'�T',	L'�V',	L'�X',	L'�Z',	L'�\',
		L'�^',	L'�`',	L'�c',	L'�e',	L'�g',
		L'�i',	L'�j',	L'�k',	L'�l',	L'�m',
		L'�n',	L'�q',	L'�t',	L'�w',	L'�z',
		
		L'�}',	L'�~',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',
		L'�@',	L'�B',	L'�D',	L'�F',	L'�H',
		L'��',	EOM_,	L'��',	EOM_,	L'��',
		
		L'�K',	L'�M',	L'�O',	L'�Q',	L'�S',
		L'�U',	L'�W',	L'�Y',	L'�[',	L'�]',
		L'�_',	L'�a',	L'�d',	L'�f',	L'�h',
		L'�o',	L'�r',	L'�u',	L'�x',	L'�{',
		L'�p',	L'�s',	L'�v',	L'�y',	L'�|',
		L'�b',	L'�A',	L'�B',	L'�I',	L'�[',
		
		L'�u',	L'�v',	L'�`',	L'�E',	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	SPACE_BUTTON_,	VAR_BUTTON1_,	VAR_BUTTON2_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// �p��
		L'�`',	L'�a',	L'�b',	L'�c',	L'�d',
		L'�e',	L'�f',	L'�g',	L'�h',	L'�i',
		L'�j',	L'�k',	L'�l',	L'�m',	L'�n',
		L'�o',	L'�p',	L'�q',	L'�r',	L'�s',
		L'�t',	L'�u',	L'�v',	L'�w',	L'�x',
		L'�y',	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'�O',	L'�P',	L'�Q',	L'�R',	L'�S',
		L'�T',	L'�U',	L'�V',	L'�W',	L'�X',
		L'�I',	EOM_,	L'��',	EOM_,	L'�^',
		L'�C',	EOM_,	L'�D',	EOM_,	L'�|',
		L'�f',	EOM_,	L'�h',	EOM_,	EOM_,
		L'��',	EOM_,	L'�i',	EOM_,	L'�j',
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	SPACE_BUTTON_,	VAR_BUTTON1_,	VAR_BUTTON2_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,
	},
};

