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
#define CLIST_LT_X							23
#define CLIST_LT_Y							50

#define CLIST_MARGIN						14

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

#define CHAR_USCORE		L'＿'
#define KEY_PER_LINE	11

// 特殊キー配置設定
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
// 太りすぎの場合は、多分優先的にワークになっていくであろう部分
static u16 s_csr = 0;
static const u16 *s_pStrSetting[ USER_INFO_MENU_ELEMENT_NUM ];			// メインメニュー用文字テーブルへのポインタリスト
static int char_mode = 0;
static u16 s_key_csr = 0;
static u8 s_color_csr = 0;
static TWLNickname s_temp_name;
static TWLComment s_temp_comment;

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
									L"英数",
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

static void DrawOwnerInfoMenuScene( void )
{
	u16 tempbuf[TWL_COMMENT_LENGTH+2];
	u8 color;
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER INFORMATION" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_USER, (const u16 *)L"CANCEL" );
    // メニュー項目
	DrawMenu( s_csr, &s_settingParam );
	// ニックネーム
	PutStringUTF16( 128 , 8*8, TXT_COLOR_USER, TSD_GetNickname()->buffer );
	// カラー
	color = TSD_GetUserColor();
	PutStringUTF16( 128 , 12*8, TXT_COLOR_USER, L"■" );
	// コメント
	SVC_CpuCopy( TSD_GetComment()->buffer, tempbuf, 13 * 2, 16 );
	*(tempbuf+13)='\n';
	SVC_CpuCopy( TSD_GetComment()->buffer+13, tempbuf+14, 13 * 2, 16 );
	*(tempbuf+TWL_COMMENT_LENGTH+1)=0;
	PutStringUTF16( 128-78 , 16*8 , TXT_COLOR_USER, tempbuf );
}

// オーナー情報編集の初期化
void SetOwnerInfoInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
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
	
    DrawOwnerInfoMenuScene();

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

// バーチャルキー関係
// キーの表示
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

// 一文字削除
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

// 選択中文字キー・特殊キーで決定した時の挙動
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
		// 特殊キー
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
				if(*length < max_length) buf[(*length)++] = L'　';
				break;
			case OK_BUTTON_:
				if(setflag) setflag(TRUE);// 設定完了フラグを立てておく
				SVC_CpuClear(0, dest, (max_length + 1) * 2, 16);// ゼロクリア
				*destlength = *length;// 長さコピー
				SVC_CpuCopy( buf, dest, (*length) * 2, 16 );// 内容コピー
				(void)SYSM_WriteTWLSettingsFile();// ファイルへ書き込み
				// セーブ後にキャンセル処理と合流
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
		// 普通キー
		if(*length < max_length) buf[(*length)++] = code;
	}
}

// バーチャルキー上でのキーパッド及びタッチパッド処理
// 先にReadTPしておくこと。
static void PadDetectOnKey( NameOrComment noc )
{
	BOOL tp_select = FALSE;
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_RIGHT ){									// カーソルの移動
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
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// Aキーが押された
		PushKeys( char_tbl[char_mode][s_key_csr], noc );
	}else if( pad.trg & PAD_BUTTON_B ) {
		DeleteACharacter(noc);
	}
}

// ニックネーム編集画面の描画処理
static void DrawSetNicknameScene( void )
{
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"NICKNAME" );
	DrawCharKeys();
	PutStringUTF16( 128-60 , 21 , TXT_COLOR_USER, s_temp_name.buffer );
}

// ニックネーム編集の初期化
static void SetNicknameInit( void )
{
	GX_DispOff();
	GXS_DispOff();
	
	SetSoftKeyboardButton(0);
	s_key_csr = 0;
	
	// ニックネーム用テンポラリバッファの初期化
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

// ニックネーム編集メイン
static int SetNicknameMain( void )
{
	ReadTP();
	
	PadDetectOnKey(NOC_NAME);
	
	if(pad.trg || tpd.disp.touch)
	{// 描画処理……ボタン押したorタッチ時ぐらいで十分
		DrawSetNicknameScene();
	}
	
	return 0;
}

// 誕生日編集の初期化
static void SetBirthdayInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BIRTHDAY" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_USER, (const u16 *)L"CANCEL" );
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
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

static void DrawColorSample( void )
{
	int l;
	
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"USER COLOR" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_USER, (const u16 *)L"CANCEL" );
	for(l=0;l<16;l++) //16色
	{
		PutStringUTF16( 88 + 24 * (l%4), 54 + 24 * (l/4), 40 + l + 8 * (l/8), (const u16 *)L"■" );
	}
	PutStringUTF16( 88 + 24 * (s_color_csr%4), 54 + 24 * (s_color_csr/4), TXT_COLOR_WHITE, (const u16 *)L"☆" );
}

// ユーザーカラー編集の初期化
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

// ユーザーカラー編集メイン
static int SetUserColorMain( void )
{
	BOOL tp_cancel = FALSE;
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
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

	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	DrawColorSample();
	
	if( ( pad.trg & PAD_BUTTON_A ) ) {				// 色決定
		TSD_SetUserColor( (u8 )s_color_csr );
		TSD_SetFlagUserColor( TRUE );
		(void)SYSM_WriteTWLSettingsFile();// ファイルへ書き込み
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		ChangeUserColor( TSD_GetUserColor() ); // パレット色を元にもどす
		SetOwnerInfoInit();
		g_pNowProcess = SetOwnerInfoMain;
		return 0;
	}
	
	if(pad.trg || tpd.disp.touch)
	{// 描画処理……ボタン押したorタッチ時ぐらいで十分
		ChangeUserColor( s_color_csr );
		DrawColorSample();
	}
	
	return 0;
}

// コメント編集画面の描画処理
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

// コメント編集の初期化
static void SetCommentInit( void )
{
	GX_DispOff();
	GXS_DispOff();
	
	SetSoftKeyboardButton(0);
	s_key_csr = 0;
	
	// コメント用テンポラリバッファの初期化
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

// コメント編集メイン
static int SetCommentMain( void )
{
	ReadTP();
	
	PadDetectOnKey(NOC_COMMENT);
	
	if(pad.trg || tpd.disp.touch)
	{// 描画処理……ボタン押したorタッチ時ぐらいで十分
		DrawSetCommentScene();
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
		L'か',	L'き',	L'く',	L'け',	L'こ',	DEL_BUTTON_,
		L'さ',	L'し',	L'す',	L'せ',	L'そ',
		L'た',	L'ち',	L'つ',	L'て',	L'と',	SPACE_BUTTON_,
		L'な',	L'に',	L'ぬ',	L'ね',	L'の',
		L'は',	L'ひ',	L'ふ',	L'へ',	L'ほ',	EOM_,
		
		L'ま',	L'み',	L'む',	L'め',	L'も',
		L'や',	EOM_,	L'ゆ',	EOM_,	L'よ',	VAR_BUTTON1_,
		L'ら',	L'り',	L'る',	L'れ',	L'ろ',
		L'わ',	L'ゐ',	L'ゑ',	L'を',	L'ん',	VAR_BUTTON2_,
		L'ぁ',	L'ぃ',	L'ぅ',	L'ぇ',	L'ぉ',
		L'ゃ',	EOM_,	L'ゅ',	EOM_,	L'ょ',	EOM_,
		
		L'が',	L'ぎ',	L'ぐ',	L'げ',	L'ご',
		L'ざ',	L'じ',	L'ず',	L'ぜ',	L'ぞ',	EOM_,
		L'だ',	L'ぢ',	L'づ',	L'で',	L'ど',
		L'ば',	L'び',	L'ぶ',	L'べ',	L'ぼ',	OK_BUTTON_,
		L'ぱ',	L'ぴ',	L'ぷ',	L'ぺ',	L'ぽ',
		L'っ',	L'、',	L'。',	L'！',	L'？',	EOM_,
		
		L'「',	L'」',	L'～',	L'・',	L'ー',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// カタカナ
		L'ア',	L'イ',	L'ウ',	L'エ',	L'オ',
		L'カ',	L'キ',	L'ク',	L'ケ',	L'コ',	DEL_BUTTON_,
		L'サ',	L'シ',	L'ス',	L'セ',	L'ソ',
		L'タ',	L'チ',	L'ツ',	L'テ',	L'ト',	SPACE_BUTTON_,
		L'ナ',	L'ニ',	L'ヌ',	L'ネ',	L'ノ',
		L'ハ',	L'ヒ',	L'フ',	L'ヘ',	L'ホ',	EOM_,
		
		L'マ',	L'ミ',	L'ム',	L'メ',	L'モ',
		L'ヤ',	EOM_,	L'ユ',	EOM_,	L'ヨ',	VAR_BUTTON1_,
		L'ラ',	L'リ',	L'ル',	L'レ',	L'ロ',
		L'ワ',	EOM_,	L'ヲ',	EOM_,	L'ン',	VAR_BUTTON2_,
		L'ァ',	L'ィ',	L'ゥ',	L'ェ',	L'ォ',
		L'ャ',	EOM_,	L'ュ',	EOM_,	L'ョ',	EOM_,
		
		L'ガ',	L'ギ',	L'グ',	L'ゲ',	L'ゴ',
		L'ザ',	L'ジ',	L'ズ',	L'ゼ',	L'ゾ',	EOM_,
		L'ダ',	L'ヂ',	L'ヅ',	L'デ',	L'ド',
		L'バ',	L'ビ',	L'ブ',	L'ベ',	L'ボ',	OK_BUTTON_,
		L'パ',	L'ピ',	L'プ',	L'ペ',	L'ポ',
		L'ッ',	L'、',	L'。',	L'！',	L'ー',	EOM_,
		
		L'「',	L'」',	L'～',	L'・',	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// 英数
		L'Ａ',	L'Ｂ',	L'Ｃ',	L'Ｄ',	L'Ｅ',
		L'Ｆ',	L'Ｇ',	L'Ｈ',	L'Ｉ',	L'Ｊ',	DEL_BUTTON_,
		L'Ｋ',	L'Ｌ',	L'Ｍ',	L'Ｎ',	L'Ｏ',
		L'Ｐ',	L'Ｑ',	L'Ｒ',	L'Ｓ',	L'Ｔ',	SPACE_BUTTON_,
		L'Ｕ',	L'Ｖ',	L'Ｗ',	L'Ｘ',	L'Ｙ',
		L'Ｚ',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'ａ',	L'ｂ',	L'ｃ',	L'ｄ',	L'ｅ',
		L'ｆ',	L'ｇ',	L'ｈ',	L'ｉ',	L'ｊ',	VAR_BUTTON1_,
		L'ｋ',	L'ｌ',	L'ｍ',	L'ｎ',	L'ｏ',
		L'ｐ',	L'ｑ',	L'ｒ',	L'ｓ',	L'ｔ',	VAR_BUTTON2_,
		L'ｕ',	L'ｖ',	L'ｗ',	L'ｘ',	L'ｙ',
		L'ｚ',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'０',	L'１',	L'２',	L'３',	L'４',
		L'５',	L'６',	L'７',	L'８',	L'９',	EOM_,
		L'！',	EOM_,	L'＆',	EOM_,	L'／',
		L'，',	EOM_,	L'．',	EOM_,	L'－',	OK_BUTTON_,
		L'’',	EOM_,	L'”',	EOM_,	EOM_,
		L'＠',	EOM_,	L'（',	EOM_,	L'）',	EOM_,
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
};

