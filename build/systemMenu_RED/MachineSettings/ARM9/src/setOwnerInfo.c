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
#define CLIST_LT_X							23
#define CLIST_LT_Y							50

#define CLIST_MARGIN						14

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

#define CHAR_USCORE		L'�Q'
#define KEY_PER_LINE	11

// ����L�[�z�u�ݒ�
typedef struct CsrPos {
	u16 			x;												// x
	u16 			y;												// y
}CsrPos;

typedef enum NameOrComment
{
	NOC_NAME,
	NOC_COMMENT
}NameOrComment;

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
// ���肷���̏ꍇ�́A�����D��I�Ƀ��[�N�ɂȂ��Ă����ł��낤����
static u16 s_csr = 0;
static const u16 *s_pStrSetting[ USER_INFO_MENU_ELEMENT_NUM ];			// ���C�����j���[�p�����e�[�u���ւ̃|�C���^���X�g
static int char_mode = 0;
static u16 s_key_csr = 0;
static u8 s_color_csr = 0;
static TWLNickname s_temp_name;
static TWLComment s_temp_comment;

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
									L"�p��",
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

static void DrawOwnerInfoMenuScene( void )
{
	u16 tempbuf[TWL_COMMENT_LENGTH+2];
	u8 color;
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER INFORMATION" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_USER, (const u16 *)L"CANCEL" );
    // ���j���[����
	DrawMenu( s_csr, &s_settingParam );
	// �j�b�N�l�[��
	PutStringUTF16( 128 , 8*8, TXT_COLOR_USER, TSD_GetNickname()->buffer );
	// �J���[
	color = TSD_GetUserColor();
	PutStringUTF16( 128 , 12*8, TXT_COLOR_USER, L"��" );
	// �R�����g
	SVC_CpuCopy( TSD_GetComment()->buffer, tempbuf, 13 * 2, 16 );
	*(tempbuf+13)='\n';
	SVC_CpuCopy( TSD_GetComment()->buffer+13, tempbuf+14, 13 * 2, 16 );
	*(tempbuf+TWL_COMMENT_LENGTH+1)=0;
	PutStringUTF16( 128-78 , 16*8 , TXT_COLOR_USER, tempbuf );
}

// �I�[�i�[���ҏW�̏�����
void SetOwnerInfoInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
    DrawOwnerInfoMenuScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
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
	
    DrawOwnerInfoMenuScene();

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

// �o�[�`�����L�[�֌W
// �L�[�̕\��
static void DrawCharKeys( void )
{
	int l;
	u16 code;

	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		int color=TXT_COLOR_BLACK;
		code = char_tbl[char_mode][l];
		if (s_key_csr == l) color = TXT_COLOR_GREEN;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + 7*((l%KEY_PER_LINE)/5) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, str_button[x] );
			}
			else
			{
				u16 s[2];
				s[0] = code;
				s[1] = 0;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + 7*((l%KEY_PER_LINE)/5) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, s );
			}
		}
	}
}

// �ꕶ���폜
static void DeleteACharacter( NameOrComment noc )
{
	u16 *buf;
	u8 *length;
	if(noc == NOC_NAME)
	{
		buf = s_temp_name.buffer;
		length = &s_temp_name.length;
	}else if(noc == NOC_COMMENT)
	{
		buf = s_temp_comment.buffer;
		length = &s_temp_comment.length;
	}else
	{
		//unknown
		return;
	}
	
	if(*length > 0) buf[--(*length)] = CHAR_USCORE;
}

// �I�𒆕����L�[�E����L�[�Ō��肵�����̋���
static void PushKeys( u16 code, NameOrComment noc )
{
	u16 *buf;
	u8 *length;
	u16 *dest;
	u8 *destlength;
	u16 max_length;
	void (*setflag)(BOOL);
	if(noc == NOC_NAME)
	{
		buf = s_temp_name.buffer;
		length = &s_temp_name.length;
		dest = TSD_GetNickname()->buffer;
		destlength = &TSD_GetNickname()->length;
		max_length = TWL_NICKNAME_LENGTH;
		setflag = TSD_SetFlagNickname;
	}else if(noc == NOC_COMMENT)
	{
		buf = s_temp_comment.buffer;
		length = &s_temp_comment.length;
		dest = TSD_GetComment()->buffer;
		destlength = &TSD_GetComment()->length;
		max_length = TWL_COMMENT_LENGTH;
		// setflag = TSD_SetFlagComment;
		setflag = NULL;
	}else
	{
		//unknown
		return;
	}
	
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		// ����L�[
		switch(code)
		{
			case VAR_BUTTON1_:
			case VAR_BUTTON2_:
				SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
				break;
			case DEL_BUTTON_:
				DeleteACharacter(noc);
				break;
			case SPACE_BUTTON_:
				if(*length < max_length) buf[(*length)++] = L'�@';
				break;
			case OK_BUTTON_:
				if(setflag) setflag(TRUE);// �ݒ芮���t���O�𗧂ĂĂ���
				SVC_CpuClear(0, dest, (max_length + 1) * 2, 16);// �[���N���A
				*destlength = *length;// �����R�s�[
				SVC_CpuCopy( buf, dest, (*length) * 2, 16 );// ���e�R�s�[
				(void)SYSM_WriteTWLSettingsFile();// �t�@�C���֏�������
				// �Z�[�u��ɃL�����Z�������ƍ���
			case CANCEL_BUTTON_:
				SetOwnerInfoInit();
				g_pNowProcess = SetOwnerInfoMain;
				break;
			default:// unknown code
				break;
		}
	}
	else
	{
		// ���ʃL�[
		if(*length < max_length) buf[(*length)++] = code;
	}
}

// �o�[�`�����L�[��ł̃L�[�p�b�h�y�у^�b�`�p�b�h����
// ���ReadTP���Ă������ƁB
static void PadDetectOnKey( NameOrComment noc )
{
	BOOL tp_select = FALSE;
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_RIGHT ){									// �J�[�\���̈ړ�
		do
		{
			if(s_key_csr%KEY_PER_LINE != KEY_PER_LINE-1) s_key_csr++;
			else s_key_csr -= KEY_PER_LINE-1;
			if( s_key_csr == CHAR_LIST_CHAR_NUM ) s_key_csr -= s_key_csr%KEY_PER_LINE;
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_LEFT ){
		do
		{
			if(s_key_csr%KEY_PER_LINE != 0) s_key_csr--;
			else s_key_csr += KEY_PER_LINE-1;
			if( s_key_csr & 0x8000 ) s_key_csr = KEY_PER_LINE-1;
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		do
		{
			s_key_csr += KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE*(s_key_csr/KEY_PER_LINE);
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_UP ){
		do
		{
			if( s_key_csr < KEY_PER_LINE ) s_key_csr += (CHAR_LIST_CHAR_NUM/KEY_PER_LINE)*KEY_PER_LINE;
			else s_key_csr -= KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE;
		}
		while(char_tbl[char_mode][s_key_csr]==EOM_);
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// A�L�[�������ꂽ
		PushKeys( char_tbl[char_mode][s_key_csr], noc );
	}else if( pad.trg & PAD_BUTTON_B ) {
		DeleteACharacter(noc);
	}
}

// �j�b�N�l�[���ҏW��ʂ̕`�揈��
static void DrawSetNicknameScene( void )
{
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"NICKNAME" );
	DrawCharKeys();
	PutStringUTF16( 128-60 , 21 , TXT_COLOR_USER, s_temp_name.buffer );
}

// �j�b�N�l�[���ҏW�̏�����
static void SetNicknameInit( void )
{
	GX_DispOff();
	GXS_DispOff();
	
	SetSoftKeyboardButton(0);
	s_key_csr = 0;
	
	// �j�b�N�l�[���p�e���|�����o�b�t�@�̏�����
	s_temp_name.length = TSD_GetNickname()->length;
	SVC_CpuClear(CHAR_USCORE, s_temp_name.buffer, TWL_NICKNAME_LENGTH * 2, 16);
	SVC_CpuCopy( TSD_GetNickname()->buffer, s_temp_name.buffer, s_temp_name.length * 2, 16 );
	s_temp_name.buffer[TWL_NICKNAME_LENGTH] = 0;
	
	DrawSetNicknameScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �j�b�N�l�[���ҏW���C��
static int SetNicknameMain( void )
{
	ReadTP();
	
	PadDetectOnKey(NOC_NAME);
	
	if(pad.trg || tpd.disp.touch)
	{// �`�揈���c�c�{�^��������or�^�b�`�����炢�ŏ\��
		DrawSetNicknameScene();
	}
	
	return 0;
}

// �a�����ҏW�̏�����
static void SetBirthdayInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BIRTHDAY" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_USER, (const u16 *)L"CANCEL" );
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
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

static void DrawColorSample( void )
{
	int l;
	
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER COLOR" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_USER, (const u16 *)L"CANCEL" );
	for(l=0;l<16;l++) //16�F
	{
		PutStringUTF16( 88 + 24 * (l%4), 54 + 24 * (l/4), 40 + l + 8 * (l/8), (const u16 *)L"��" );
	}
	PutStringUTF16( 88 + 24 * (s_color_csr%4), 54 + 24 * (s_color_csr/4), TXT_COLOR_WHITE, (const u16 *)L"��" );
}

// ���[�U�[�J���[�ҏW�̏�����
static void SetUserColorInit( void )
{
	GX_DispOff();
	GXS_DispOff();
	
	DrawColorSample();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	s_color_csr = TSD_GetUserColor();
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// ���[�U�[�J���[�ҏW���C��
static int SetUserColorMain( void )
{
	BOOL tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		s_color_csr += 4;
		if(s_color_csr >= 16) s_color_csr -= 16;
	}
	if( pad.trg & PAD_KEY_UP ){
		if(s_color_csr < 4) s_color_csr += 16;
		s_color_csr -= 4;
	}
	if( pad.trg & PAD_KEY_RIGHT ){
		s_color_csr += 1;
		if(s_color_csr%4 == 0) s_color_csr -= 4;
	}
	if( pad.trg & PAD_KEY_LEFT ){
		if(s_color_csr%4 == 0) s_color_csr += 4;
		s_color_csr -= 1;
	}

	// [CANCEL]�{�^�������`�F�b�N
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	DrawColorSample();
	
	if( ( pad.trg & PAD_BUTTON_A ) ) {				// �F����
		TSD_SetUserColor( (u8 )s_color_csr );
		TSD_SetFlagUserColor( TRUE );
		(void)SYSM_WriteTWLSettingsFile();// �t�@�C���֏�������
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		ChangeUserColor( TSD_GetUserColor() ); // �p���b�g�F�����ɂ��ǂ�
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	
	if(pad.trg || tpd.disp.touch)
	{// �`�揈���c�c�{�^��������or�^�b�`�����炢�ŏ\��
		ChangeUserColor( s_color_csr );
		DrawColorSample();
	}
	
	return 0;
}

// �R�����g�ҏW��ʂ̕`�揈��
static void DrawSetCommentScene( void )
{
	u16 tempbuf[TWL_COMMENT_LENGTH+2];
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COMMENT" );
	DrawCharKeys();
	SVC_CpuCopy( s_temp_comment.buffer, tempbuf, 13 * 2, 16 );
	*(tempbuf+13)='\n';
	SVC_CpuCopy( s_temp_comment.buffer+13, tempbuf+14, 13 * 2, 16 );
	*(tempbuf+TWL_COMMENT_LENGTH+1)=0;
	PutStringUTF16( 128-78 , 15 , TXT_COLOR_USER, tempbuf );
}

// �R�����g�ҏW�̏�����
static void SetCommentInit( void )
{
	GX_DispOff();
	GXS_DispOff();
	
	SetSoftKeyboardButton(0);
	s_key_csr = 0;
	
	// �R�����g�p�e���|�����o�b�t�@�̏�����
	s_temp_comment.length = TSD_GetComment()->length;
	SVC_CpuClear(CHAR_USCORE, s_temp_comment.buffer, TWL_COMMENT_LENGTH * 2, 16);
	SVC_CpuCopy( TSD_GetComment()->buffer, s_temp_comment.buffer, s_temp_comment.length * 2, 16 );
	s_temp_comment.buffer[TWL_COMMENT_LENGTH] = 0;
	
	DrawSetCommentScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

// �R�����g�ҏW���C��
static int SetCommentMain( void )
{
	ReadTP();
	
	PadDetectOnKey(NOC_COMMENT);
	
	if(pad.trg || tpd.disp.touch)
	{// �`�揈���c�c�{�^��������or�^�b�`�����炢�ŏ\��
		DrawSetCommentScene();
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
		L'��',	L'��',	L'��',	L'��',	L'��',	DEL_BUTTON_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	SPACE_BUTTON_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	EOM_,
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	VAR_BUTTON1_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	VAR_BUTTON2_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	EOM_,
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	EOM_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	OK_BUTTON_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'�A',	L'�B',	L'�I',	L'�H',	EOM_,
		
		L'�u',	L'�v',	L'�`',	L'�E',	L'�[',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// �J�^�J�i
		L'�A',	L'�C',	L'�E',	L'�G',	L'�I',
		L'�J',	L'�L',	L'�N',	L'�P',	L'�R',	DEL_BUTTON_,
		L'�T',	L'�V',	L'�X',	L'�Z',	L'�\',
		L'�^',	L'�`',	L'�c',	L'�e',	L'�g',	SPACE_BUTTON_,
		L'�i',	L'�j',	L'�k',	L'�l',	L'�m',
		L'�n',	L'�q',	L'�t',	L'�w',	L'�z',	EOM_,
		
		L'�}',	L'�~',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	VAR_BUTTON1_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	VAR_BUTTON2_,
		L'�@',	L'�B',	L'�D',	L'�F',	L'�H',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	EOM_,
		
		L'�K',	L'�M',	L'�O',	L'�Q',	L'�S',
		L'�U',	L'�W',	L'�Y',	L'�[',	L'�]',	EOM_,
		L'�_',	L'�a',	L'�d',	L'�f',	L'�h',
		L'�o',	L'�r',	L'�u',	L'�x',	L'�{',	OK_BUTTON_,
		L'�p',	L'�s',	L'�v',	L'�y',	L'�|',
		L'�b',	L'�A',	L'�B',	L'�I',	L'�[',	EOM_,
		
		L'�u',	L'�v',	L'�`',	L'�E',	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// �p��
		L'�`',	L'�a',	L'�b',	L'�c',	L'�d',
		L'�e',	L'�f',	L'�g',	L'�h',	L'�i',	DEL_BUTTON_,
		L'�j',	L'�k',	L'�l',	L'�m',	L'�n',
		L'�o',	L'�p',	L'�q',	L'�r',	L'�s',	SPACE_BUTTON_,
		L'�t',	L'�u',	L'�v',	L'�w',	L'�x',
		L'�y',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	VAR_BUTTON1_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	VAR_BUTTON2_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'�O',	L'�P',	L'�Q',	L'�R',	L'�S',
		L'�T',	L'�U',	L'�V',	L'�W',	L'�X',	EOM_,
		L'�I',	EOM_,	L'��',	EOM_,	L'�^',
		L'�C',	EOM_,	L'�D',	EOM_,	L'�|',	OK_BUTTON_,
		L'�f',	EOM_,	L'�h',	EOM_,	EOM_,
		L'��',	EOM_,	L'�i',	EOM_,	L'�j',	EOM_,
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
};

