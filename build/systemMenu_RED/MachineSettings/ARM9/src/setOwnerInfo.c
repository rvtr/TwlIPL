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
#define CLIST_KEY_PER_SEGMENT				5
#define CLIST_SEGMENT_INTERVAL				7

// �L�����Z���{�^���̈�
#define CANCEL_BUTTON_TOP_X					( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( CANCEL_BUTTON_TOP_X + (8 * 8) )
#define CANCEL_BUTTON_BOTTOM_Y				( CANCEL_BUTTON_TOP_Y + (2 * 8) )

// OK�{�^���̈�
#define OK_BUTTON_TOP_X					( 26 * 8 )
#define OK_BUTTON_TOP_Y					( 21 * 8 )
#define OK_BUTTON_BOTTOM_X				( OK_BUTTON_TOP_X + (4 * 8) )
#define OK_BUTTON_BOTTOM_Y				( OK_BUTTON_TOP_Y + (2 * 8) )

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

#define KEY_START		109	//�\�t�g�E�F�A�L�[�̃J�[�\���f�t�H���g�ʒu�̓L�����Z���L�[

#define KEY_OK			0xffff
#define KEY_CANCEL		0xfffe

typedef enum NameOrComment
{
	NOC_NAME,
	NOC_COMMENT
}NameOrComment;

// extern data----------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_birth_scr_data[32 * 32];

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
// �ꎞ�I�ɂ����g��Ȃ�����static�ɂ��Ă���̂�
// �����ł��_�C�G�b�g����������Work�����ɂ���Alloc��Free���܂��傤
static u16 s_csr = 0;
static const u16 *s_pStrSetting[ USER_INFO_MENU_ELEMENT_NUM ];			// ���C�����j���[�p�����e�[�u���ւ̃|�C���^���X�g
static int s_char_mode = 0;
static u16 s_key_csr = 0;
static u8 s_color_csr = 0;
static BOOL s_birth_csr = FALSE;
static TWLDate s_temp_birthday;
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

static const u16  str_button_del[] = L"�ADEL";
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
	s_char_mode = mode;
}

// �L�����Z���{�^����pSelectSomethingFunc�̎���
static BOOL SelectCancelFunc( u16 *csr, TPData *tgt )
{
	BOOL ret;
	ret = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, tgt );
	if(ret) *csr = KEY_CANCEL;
	return ret;
}

// OK�{�^����pSelectSomethingFunc�̎���
static BOOL SelectOKFunc( u16 *csr, TPData *tgt )
{
	BOOL ret;
	ret = WithinRangeTP( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y,
							   OK_BUTTON_BOTTOM_X, OK_BUTTON_BOTTOM_Y, tgt );
	if(ret) *csr = KEY_OK;
	return ret;
}

static void DrawOwnerInfoMenuScene( void )
{
	u16 tempbuf[TWL_COMMENT_LENGTH+2];
	u8 color;
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER INFORMATION" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ARETURN" );
    // ���j���[����
	DrawMenu( s_csr, &s_settingParam );
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾���ĕ\��
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// �j�b�N�l�[��
	PutStringUTF16( 128 , 8*8, TXT_UCOLOR_G0, TSD_GetNickname()->buffer );
	// �a����
	PrintfSJIS( 128, 10*8, TXT_UCOLOR_G0, "%d�^%d", TSD_GetBirthday()->month, TSD_GetBirthday()->day);
	// �J���[
	color = TSD_GetUserColor();
	PutStringUTF16( 128 , 12*8, TXT_UCOLOR_G0, L"��" );
	// �R�����g
	SVC_CpuCopy( TSD_GetComment()->buffer, tempbuf, 13 * 2, 16 );
	*(tempbuf+13)='\n';
	SVC_CpuCopy( TSD_GetComment()->buffer+13, tempbuf+14, 13 * 2, 16 );
	*(tempbuf+TWL_COMMENT_LENGTH+1)=0;
	PutStringUTF16( 128-78 , 16*8 , TXT_UCOLOR_G0, tempbuf );
}

// �I�[�i�[���ҏW�̏�����
void SetOwnerInfoInit( void )
{
	int i;
	
	// NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
	for( i = 0; i < USER_INFO_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ TSD_GetLanguage() ];
	}
	
    // BG�f�[�^�̃��[�h����
	GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));
	
    DrawOwnerInfoMenuScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �I�[�i�[���ҏW���j���[
int SetOwnerInfoMain( void )
{
	static u16 dummy = 0;
	SelectSomethingFunc func[1]={SelectCancelFunc};
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

	// [CANCEL]�{�^���`�F�b�N
	tp_cancel = SelectSomethingByTP(&dummy, func, 1 );
	
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

// �\�t�g�E�F�A�L�[�֌W
// �L�[�̕\��
static void DrawCharKeys( void )
{
	int l;
	u16 code;

	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		int color=TXT_COLOR_BLACK;
		code = char_tbl[s_char_mode][l];
		if (s_key_csr == l) color = TXT_COLOR_GREEN;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, str_button[x] );
			}
			else
			{
				u16 s[2];
				s[0] = code;
				s[1] = 0;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
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
				// ::::::::::::::::::::::::::::::::::::::::::::::
				// TWL�ݒ�f�[�^�t�@�C���ւ̏�������
				// ::::::::::::::::::::::::::::::::::::::::::::::
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

// PadDetectOnKey��SelectSomethingByTP�Ŏg��SelectSomethingFunc�̎���
static BOOL SelectSoftwareKeyFunc( u16 *csr, TPData *tgt )
{
	// �܂��͌��ƂȂ���W�i�J�[�\���P�ʁj���擾
	int csrx;
	int csry;
	int csrxy;
	int a;
	int b;
	NNSG2dTextRect rect;
	u16 code;
	BOOL ret;
	
	csrx = tgt->x - CLIST_LT_X;
	csrx = csrx - (CLIST_SEGMENT_INTERVAL*(csrx/(CLIST_MARGIN*CLIST_KEY_PER_SEGMENT+CLIST_SEGMENT_INTERVAL)));
	csrx = csrx / CLIST_MARGIN;
	csry = (tgt->y - CLIST_LT_Y) / CLIST_MARGIN;
	if(csrx < 0 ) return FALSE;

	if ( csrx >= KEY_PER_LINE ) csrx = KEY_PER_LINE - 1;
	csrxy = csrx + csry * KEY_PER_LINE;

	if ( csrxy < 0 || csrxy >= CHAR_LIST_CHAR_NUM) return FALSE;// ���炩�ɂ͂ݏo����

	// �����W�̃L�[�R�[�h�擾
	code = char_tbl[s_char_mode][csrxy];
	if(code == EOM_) return FALSE;
	
	// �����W�̗̈�擾
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		int x = code - CODE_BUTTON_TOP_;
		rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str_button[x] );
	}
	else
	{
		u16 s[2];
		s[0] = code;
		s[1] = 0;
		// rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, s );
		// ���������Ⴉ�Ȃ蔻�肪�������c�c�M���M���܂łƂ��Ă݂�
		rect.width = CLIST_MARGIN;
		rect.height = CLIST_MARGIN;
	}
	a = CLIST_LT_X + CLIST_MARGIN*(csrxy%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((csrxy%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT);
	b = CLIST_LT_Y + CLIST_MARGIN*(csrxy/KEY_PER_LINE);
	
	// �����W�̗̈�Ƀ^�b�`���W���܂܂�Ă��邩�`�F�b�N
	ret = WithinRangeTP( a, b, a+rect.width, b+rect.height, tgt );
	
	if(ret)
	{
		*csr = (u16)csrxy;
	}
	return ret;
}

// �\�t�g�E�F�A�L�[��ł̃L�[�p�b�h�y�у^�b�`�p�b�h����
// ���ReadTP���Ă������ƁB
static void PadDetectOnKey( NameOrComment noc )
{
	SelectSomethingFunc func[1];
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
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_LEFT ){
		do
		{
			if(s_key_csr%KEY_PER_LINE != 0) s_key_csr--;
			else s_key_csr += KEY_PER_LINE-1;
			if( s_key_csr & 0x8000 ) s_key_csr = KEY_PER_LINE-1;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_DOWN ){									// �J�[�\���̈ړ�
		do
		{
			s_key_csr += KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE*(s_key_csr/KEY_PER_LINE);
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_UP ){
		do
		{
			if( s_key_csr < KEY_PER_LINE ) s_key_csr += (CHAR_LIST_CHAR_NUM/KEY_PER_LINE)*KEY_PER_LINE;
			else s_key_csr -= KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	
	func[0] = (SelectSomethingFunc)SelectSoftwareKeyFunc;
	tp_select = SelectSomethingByTP(&s_key_csr, func, 1 );
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// �L�[�������ꂽ
		PushKeys( char_tbl[s_char_mode][s_key_csr], noc );
	}else if( pad.trg & PAD_BUTTON_B ) {
		DeleteACharacter(noc);
	}
}

// �j�b�N�l�[���ҏW��ʂ̕`�揈��
static void DrawSetNicknameScene( void )
{
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"NICKNAME" );
	PutStringUTF16( 128-60 , 21 , TXT_UCOLOR_G0, s_temp_name.buffer );
	DrawCharKeys();
}

// �j�b�N�l�[���ҏW�̏�����
static void SetNicknameInit( void )
{
	SetSoftKeyboardButton(0);
	s_key_csr = KEY_START;
	
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// �j�b�N�l�[���p�e���|�����o�b�t�@�̏�����
	s_temp_name.length = TSD_GetNickname()->length;
	SVC_CpuClear(CHAR_USCORE, s_temp_name.buffer, TWL_NICKNAME_LENGTH * 2, 16);
	SVC_CpuCopy( TSD_GetNickname()->buffer, s_temp_name.buffer, s_temp_name.length * 2, 16 );
	s_temp_name.buffer[TWL_NICKNAME_LENGTH] = 0;
	
	DrawSetNicknameScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �j�b�N�l�[���ҏW���C��
static int SetNicknameMain( void )
{
	ReadTP();
	
	PadDetectOnKey(NOC_NAME);
	
	// �`�揈��
	DrawSetNicknameScene();
	
	return 0;
}

// �a�����ҏW��ʂ̕\��
static void DrawSetBirthdayScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BIRTHDAY" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
	PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );
	PutStringUTF16( 128-36+16, 11*8, TXT_COLOR_BLACK, (const u16 *)L"���@�@�@��" );
	PrintfSJIS( 128-36, 11*8, (s_birth_csr ? TXT_COLOR_GREEN : TXT_COLOR_BLACK), "%d", s_temp_birthday.month / 10);
	PrintfSJIS( 128-28, 11*8, (s_birth_csr ? TXT_COLOR_GREEN : TXT_COLOR_BLACK), "%d", s_temp_birthday.month % 10);
	PrintfSJIS( 128+12, 11*8, (!s_birth_csr ? TXT_COLOR_GREEN : TXT_COLOR_BLACK), "%d", s_temp_birthday.day / 10);
	PrintfSJIS( 128+20, 11*8, (!s_birth_csr ? TXT_COLOR_GREEN : TXT_COLOR_BLACK), "%d", s_temp_birthday.day % 10);
}

// SetBirthdayMain��SelectSomethingByTP�Ŏg��SelectSomethingFunc�̎���
static BOOL SelectBirthdayFunc( u16 *csr, TPData *tgt )
{
	int l;
	
	// �P���Ȏ�����
	// �L���͈͑S���ɂ��ĉ����ꂽ���ǂ����̊m�F
	// �L���͈͂̋敪���������́A�^�b�`�p�b�h�̍��W����m�F�͈͂��i��̂��]�܂���
	for(l=0; l<4; l++)
	{
		int x = 12*8 + (l%2)*6*8;
		int y = 8*8 + (l/2)*6*8;
		if(WithinRangeTP( x, y, x+16, y+16, tgt ))
		{
			*csr = (u16)l;
			return TRUE;
		}
	}
	
	return FALSE;
}

// �a�����ҏW�̏�����
static void SetBirthdayInit( void )
{
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// �a����
	s_temp_birthday.month = TSD_GetBirthday()->month;
	s_temp_birthday.day = TSD_GetBirthday()->day;
	
    // BG�f�[�^�̃��[�h����
	GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG1Scr(bg_birth_scr_data, 0, sizeof(bg_birth_scr_data));
	
	DrawSetBirthdayScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

static void CheckDate( void )
{
	u8 maxday;
	if( s_temp_birthday.month == 0 ) s_temp_birthday.month = 12;
	if( s_temp_birthday.month == 13 ) s_temp_birthday.month = 1;
	maxday = (u8)SYSM_GetDayNum( 2000, s_temp_birthday.month );
	if( s_temp_birthday.day == 0 ) s_temp_birthday.day = maxday;
	if( s_temp_birthday.day > maxday ) s_temp_birthday.day = 1;
}

// �����Ă���Ԑ��������X�s�[�h�ŕω�����悤�ȏ���
static void Birthday_AutoNumScrollByTP( void )
{
	static u16 first_csr = 0xffff;
	u16 temp_csr;
	static int same_count = 0;
	
	if( tpd.disp.touch )
	{
		BOOL t = SelectBirthdayFunc( &temp_csr, &tpd.disp );
		if( t )
		{
			if(same_count == 0) // count start
			{
				first_csr = temp_csr;
				same_count = 1;
			}else if(first_csr == temp_csr)
			{
				if( same_count == 1 || (same_count > 29 && same_count%10==0))
				{
					switch(temp_csr)
					{
						case 0:
							s_birth_csr = TRUE;
							s_temp_birthday.month++;
							break;
						case 1:
							s_birth_csr = FALSE;
							s_temp_birthday.day++;
							break;
						case 2:
							s_birth_csr = TRUE;
							s_temp_birthday.month--;
							break;
						case 3:
							s_birth_csr = FALSE;
							s_temp_birthday.day--;
							break;
						default:
							break;
					}
				}
				same_count++;
			}
		}
	}else // touch==0
	{
		same_count = 0;
		first_csr = 0xffff;
	}
}

// �a�����ҏW���C��
static int SetBirthdayMain( void )
{
	SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
	BOOL tp_touch = FALSE;
	u16 temp_ok_cancel;
	
	ReadTP();
	
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){
		(*(s_birth_csr ? &s_temp_birthday.month : &s_temp_birthday.day))--;
	}
	if( pad.trg & PAD_KEY_UP ){
		(*(s_birth_csr ? &s_temp_birthday.month : &s_temp_birthday.day))++;
	}
	if( pad.trg & (PAD_KEY_RIGHT | PAD_KEY_LEFT)){									// �J�[�\���̈ړ�
		s_birth_csr = !s_birth_csr;
	}

	// ���t�`�F�b�N
	CheckDate();
	
	// TP�`�F�b�N
	tp_touch = SelectSomethingByTP(&temp_ok_cancel, func, 2 );
	// TP�Ń{�^���������Ă���Ԑ��������X�s�[�h�ŕω�����悤�ȏ���
	Birthday_AutoNumScrollByTP();
	
	// ���t�`�F�b�N
	CheckDate();
	
	DrawSetBirthdayScene();
	
	if( pad.trg & PAD_BUTTON_A || (tp_touch && temp_ok_cancel == KEY_OK) ) {
		TSD_SetBirthday(&s_temp_birthday);
		TSD_SetFlagBirthday( TRUE );
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL�ݒ�f�[�^�t�@�C���ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)SYSM_WriteTWLSettingsFile();// �t�@�C���֏�������
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || (tp_touch && temp_ok_cancel == KEY_CANCEL) ) {
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
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
	PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );
	for(l=0;l<16;l++) //16�F
	{
		PutStringUTF16( 88 + 24 * (l%4), 54 + 24 * (l/4), TXT_UCOLOR_GRAY + l, (const u16 *)L"��" );
	}
	for(l=0;l<4;l++)
	{
		PutStringUTF16( 83 + 24 * (s_color_csr%4) + 10*(l%2), 49 + 24 * (s_color_csr/4) + 10*(l/2), TXT_UCOLOR_G0, (const u16 *)L"��" );
	}
}

// SetUserColorMain��SelectSomethingByTP�Ŏg��SelectSomethingFunc�̎���
static BOOL SelectColorFunc( u16 *csr, TPData *tgt )
{
	// �܂��͌��ƂȂ���W�i�J�[�\���P�ʁj���擾
	int csrx;
	int csry;
	int csrxy;
	int a;
	int b;
	BOOL ret;
	
	csrx = (tgt->x - 88) / 24;
	csry = (tgt->y - 54) / 24;
	if(csrx < 0 ) return FALSE;

	if ( csrx >= 4 ) csrx = 3;
	csrxy = csrx + csry * 4;

	if ( csrxy < 0 || csrxy >= 16) return FALSE;// �͂ݏo����

	a = 88 + csrx * 24;
	b = 54 + csry * 24;
	
	// �����W�̗̈�Ƀ^�b�`���W���܂܂�Ă��邩�`�F�b�N
	ret = WithinRangeTP( a, b, a+11, b+11, tgt );
	
	if(ret)
	{
		*csr = (u16)csrxy;
	}
	return ret;
}

// ���[�U�[�J���[�ҏW�̏�����
static void SetUserColorInit( void )
{
	DrawColorSample();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���[�U�[�J���[
	s_color_csr = TSD_GetUserColor();
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// ���[�U�[�J���[�ҏW���C��
static int SetUserColorMain( void )
{
	SelectSomethingFunc func[3]={SelectColorFunc, SelectCancelFunc, SelectOKFunc};
	BOOL tp_touch = FALSE;
	u16 temp_csr;
	
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
	temp_csr = s_color_csr;
	
	// TP�`�F�b�N
	tp_touch = SelectSomethingByTP(&temp_csr, func, 3 );
	if ((temp_csr != KEY_OK && temp_csr != KEY_CANCEL)){
		s_color_csr = (u8)temp_csr;
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || (tp_touch && temp_csr == KEY_OK) ) {				// �F����
		TSD_SetUserColor( (u8 )s_color_csr );
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL�ݒ�f�[�^�t�@�C���ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)SYSM_WriteTWLSettingsFile();// �t�@�C���֏�������
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || (tp_touch && temp_csr == KEY_CANCEL) ) {
		ChangeUserColor( TSD_GetUserColor() ); // �p���b�g�F�����ɂ��ǂ�
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	
	ChangeUserColor( s_color_csr );
	DrawColorSample();
	
	return 0;
}

// �R�����g�ҏW��ʂ̕`�揈��
static void DrawSetCommentScene( void )
{
	u16 tempbuf[TWL_COMMENT_LENGTH+2];
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COMMENT" );
	SVC_CpuCopy( s_temp_comment.buffer, tempbuf, 13 * 2, 16 );
	*(tempbuf+13)='\n';
	SVC_CpuCopy( s_temp_comment.buffer+13, tempbuf+14, 13 * 2, 16 );
	*(tempbuf+TWL_COMMENT_LENGTH+1)=0;
	PutStringUTF16( 128-78 , 15 , TXT_UCOLOR_G0, tempbuf );
	DrawCharKeys();
}

// �R�����g�ҏW�̏�����
static void SetCommentInit( void )
{
	SetSoftKeyboardButton(0);
	s_key_csr = KEY_START;
	
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// �R�����g�p�e���|�����o�b�t�@�̏�����
	s_temp_comment.length = TSD_GetComment()->length;
	SVC_CpuClear(CHAR_USCORE, s_temp_comment.buffer, TWL_COMMENT_LENGTH * 2, 16);
	SVC_CpuCopy( TSD_GetComment()->buffer, s_temp_comment.buffer, s_temp_comment.length * 2, 16 );
	s_temp_comment.buffer[TWL_COMMENT_LENGTH] = 0;
	
	DrawSetCommentScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �R�����g�ҏW���C��
static int SetCommentMain( void )
{
	ReadTP();
	
	PadDetectOnKey(NOC_COMMENT);
	
	// �`�揈��
	DrawSetCommentScene();
	
	return 0;
}

//======================================================
// �\�t�g�E�F�A�L�[�{�[�h�p�L�����e�[�u��
//======================================================

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

