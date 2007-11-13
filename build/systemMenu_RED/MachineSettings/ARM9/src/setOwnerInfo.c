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

// ソフトウェアキーボードLCD領域
#define CLIST_LT_X							14
#define CLIST_LT_Y							40

#define CANCEL_BUTTON_TOP_X					( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( (CANCEL_BUTTON_TOP_X + 8 ) * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( (CANCEL_BUTTON_TOP_Y + 2 ) * 8 )

#define USER_INFO_MENU_ELEMENT_NUM			4						// ユーザ情報メニューの項目数

#define CHAR_LIST_CHAR_NUM					120
#define CHAR_LIST_MODE_NUM					3

// 特殊キーコード
#define EOM_			(u16)0xe050
#define CODE_BUTTON_TOP_	(u16)0xe051
#define DEL_BUTTON_		(u16)0xe051
#define SPACE_BUTTON_	(u16)0xe052
#define VAR_BUTTON1_	(u16)0xe053
#define VAR_BUTTON2_	(u16)0xe054
#define OK_BUTTON_		(u16)0xe055
#define CANCEL_BUTTON_	(u16)0xe056
#define CODE_BUTTON_BOTTOM_	(u16)0xe057


	// カーソルX,Y位置（キャラ単位）
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
static const u16 *s_pStrSetting[ USER_INFO_MENU_ELEMENT_NUM ];			// メインメニュー用文字テーブルへのポインタリスト
static int char_mode = 0;
static u16 s_key_csr = 0;

// const data-----------------------------------
static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM];

static const u16 *const s_pStrSettingElemTbl[ USER_INFO_MENU_ELEMENT_NUM ][ TWL_LANG_CODE_MAX ] = {
	{
		(const u16 *)L"ユーザーネーム",
		(const u16 *)L"NICKNAME",
		(const u16 *)L"NICKNAME(F)",
		(const u16 *)L"NICKNAME(G)",
		(const u16 *)L"NICKNAME(I)",
		(const u16 *)L"NICKNAME(S)",
	},
	{
		(const u16 *)L"誕生日",
		(const u16 *)L"BIRTHDAY",
		(const u16 *)L"BIRTHDAY(F)",
		(const u16 *)L"BIRTHDAY(G)",
		(const u16 *)L"BIRTHDAY(I)",
		(const u16 *)L"BIRTHDAY(S)",
	},
	{
		(const u16 *)L"ユーザーカラー",
		(const u16 *)L"USER COLOR",
		(const u16 *)L"USER COLOR(F)",
		(const u16 *)L"USER COLOR(G)",
		(const u16 *)L"USER COLOR(I)",
		(const u16 *)L"USER COLOR(S)",
	},
	{
		(const u16 *)L"コメント",
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
									L"かな",
									L"カナ",
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
// オーナー情報編集
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

// オーナー情報編集の初期化
void SetOwnerInfoInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER INFORMATION" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	SetSoftKeyboardButton(0);
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
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

// オーナー情報編集メニュー
int SetOwnerInfoMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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

	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
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

// キーの表示
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

// キーの表示
static void PushKeys( u16 code )
{
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		// 特殊キー
		if(code == VAR_BUTTON1_ || code == VAR_BUTTON2_)
			SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
	}
	else
	{
		// 普通キー
	}
}

// ニックネーム編集の初期化
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

// ニックネーム編集メイン
static int SetNicknameMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_RIGHT ){									// カーソルの移動
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
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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

	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// Aキーが押された
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

// 誕生日編集の初期化
static void SetBirthdayInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BIRTHDAY" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
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

// 誕生日編集メイン
static int SetBirthdayMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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

	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
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

// ユーザーカラー編集の初期化
static void SetUserColorInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER COLOR" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
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

// ユーザーカラー編集メイン
static int SetUserColorMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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

	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
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

// コメント編集の初期化
static void SetCommentInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COMMENT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
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

// コメント編集メイン
static int SetCommentMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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

	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
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
// ニックネーム入力用キャラテーブル
//======================================================

/*
	※SJIS文字を文字定数として記述する場合、以下の２通りで上位・下位コードの格納順序が
　　　逆になってしまうので、注意すること。
	
	u8 str[] = "あいうえお";	0x82,0xa0,0x82,0xa2...と上位コードが下位バイトに格納される。
	u16 code = 'あ';			0xa0,0x82 と上位・下位コードがそのまま格納される。

*/

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM] = {
	{	// ひらがな
		L'あ',	L'い',	L'う',	L'え',	L'お',
		L'か',	L'き',	L'く',	L'け',	L'こ',
		L'さ',	L'し',	L'す',	L'せ',	L'そ',
		L'た',	L'ち',	L'つ',	L'て',	L'と',
		L'な',	L'に',	L'ぬ',	L'ね',	L'の',
		L'は',	L'ひ',	L'ふ',	L'へ',	L'ほ',
		
		L'ま',	L'み',	L'む',	L'め',	L'も',
		L'や',	EOM_,	L'ゆ',	EOM_,	L'よ',
		L'ら',	L'り',	L'る',	L'れ',	L'ろ',
		L'わ',	L'ゐ',	L'ゑ',	L'を',	L'ん',
		L'ぁ',	L'ぃ',	L'ぅ',	L'ぇ',	L'ぉ',
		L'ゃ',	EOM_,	L'ゅ',	EOM_,	L'ょ',
		
		L'が',	L'ぎ',	L'ぐ',	L'げ',	L'ご',
		L'ざ',	L'じ',	L'ず',	L'ぜ',	L'ぞ',
		L'だ',	L'ぢ',	L'づ',	L'で',	L'ど',
		L'ば',	L'び',	L'ぶ',	L'べ',	L'ぼ',
		L'ぱ',	L'ぴ',	L'ぷ',	L'ぺ',	L'ぽ',
		L'っ',	L'、',	L'。',	L'！',	L'？',
		
		L'「',	L'」',	L'～',	L'・',	L'ー',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	SPACE_BUTTON_,	VAR_BUTTON1_,	VAR_BUTTON2_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// カタカナ
		L'ア',	L'イ',	L'ウ',	L'エ',	L'オ',
		L'カ',	L'キ',	L'ク',	L'ケ',	L'コ',
		L'サ',	L'シ',	L'ス',	L'セ',	L'ソ',
		L'タ',	L'チ',	L'ツ',	L'テ',	L'ト',
		L'ナ',	L'ニ',	L'ヌ',	L'ネ',	L'ノ',
		L'ハ',	L'ヒ',	L'フ',	L'ヘ',	L'ホ',
		
		L'マ',	L'ミ',	L'ム',	L'メ',	L'モ',
		L'ヤ',	EOM_,	L'ユ',	EOM_,	L'ヨ',
		L'ラ',	L'リ',	L'ル',	L'レ',	L'ロ',
		L'ワ',	EOM_,	L'ヲ',	EOM_,	L'ン',
		L'ァ',	L'ィ',	L'ゥ',	L'ェ',	L'ォ',
		L'ャ',	EOM_,	L'ュ',	EOM_,	L'ョ',
		
		L'ガ',	L'ギ',	L'グ',	L'ゲ',	L'ゴ',
		L'ザ',	L'ジ',	L'ズ',	L'ゼ',	L'ゾ',
		L'ダ',	L'ヂ',	L'ヅ',	L'デ',	L'ド',
		L'バ',	L'ビ',	L'ブ',	L'ベ',	L'ボ',
		L'パ',	L'ピ',	L'プ',	L'ペ',	L'ポ',
		L'ッ',	L'、',	L'。',	L'！',	L'ー',
		
		L'「',	L'」',	L'～',	L'・',	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	SPACE_BUTTON_,	VAR_BUTTON1_,	VAR_BUTTON2_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// 英数
		L'Ａ',	L'Ｂ',	L'Ｃ',	L'Ｄ',	L'Ｅ',
		L'Ｆ',	L'Ｇ',	L'Ｈ',	L'Ｉ',	L'Ｊ',
		L'Ｋ',	L'Ｌ',	L'Ｍ',	L'Ｎ',	L'Ｏ',
		L'Ｐ',	L'Ｑ',	L'Ｒ',	L'Ｓ',	L'Ｔ',
		L'Ｕ',	L'Ｖ',	L'Ｗ',	L'Ｘ',	L'Ｙ',
		L'Ｚ',	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'ａ',	L'ｂ',	L'ｃ',	L'ｄ',	L'ｅ',
		L'ｆ',	L'ｇ',	L'ｈ',	L'ｉ',	L'ｊ',
		L'ｋ',	L'ｌ',	L'ｍ',	L'ｎ',	L'ｏ',
		L'ｐ',	L'ｑ',	L'ｒ',	L'ｓ',	L'ｔ',
		L'ｕ',	L'ｖ',	L'ｗ',	L'ｘ',	L'ｙ',
		L'ｚ',	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'０',	L'１',	L'２',	L'３',	L'４',
		L'５',	L'６',	L'７',	L'８',	L'９',
		L'！',	EOM_,	L'＆',	EOM_,	L'／',
		L'，',	EOM_,	L'．',	EOM_,	L'－',
		L'’',	EOM_,	L'”',	EOM_,	EOM_,
		L'＠',	EOM_,	L'（',	EOM_,	L'）',
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	SPACE_BUTTON_,	VAR_BUTTON1_,	VAR_BUTTON2_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,
	},
};

