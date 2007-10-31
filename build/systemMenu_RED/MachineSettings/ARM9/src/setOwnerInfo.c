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
#include <sysmenu.h>
#include "misc.h"
#include "DS_Setting.h"
#include "myFontequ.h"

/*

	※ニックネームは、現在SJISで保存していますが、NVRAMには各文字コードのHi/Loを逆転した形で格納しています。

*/

// define data----------------------------------

#define OWNER_INFO_ELEM_NUM					3

	// RETURNボタンLCD領域
#define RETURN_BUTTON_LT_X					2
#define RETURN_BUTTON_LT_Y					21
#define RETURN_BUTTON_RB_X					(RETURN_BUTTON_LT_X + 8)
#define RETURN_BUTTON_RB_Y					(RETURN_BUTTON_LT_Y + 2)

	// オーナー情報カーソルLCD領域
#define OWNER_INFO_CSR_LT_X					2
#define OWNER_INFO_CSR_LT_Y					5
#define OWNER_INFO_CSR_NEXT_Y_NUM			3

	// 生年月日情報カーソルLCD領域
#define DAY_LT_X							(OWNER_INFO_CSR_LT_X + 13)
#define DAY_LT_Y							(OWNER_INFO_CSR_LT_Y + OWNER_INFO_CSR_NEXT_Y_NUM * 1)

	// 好きな色カーソルLCD領域
#define FCOLOR_LT_X							(OWNER_INFO_CSR_LT_X + 13)
#define FCOLOR_LT_Y							(OWNER_INFO_CSR_LT_Y + OWNER_INFO_CSR_NEXT_Y_NUM * 2)

	// ソフトウェアキーボードLCD領域
#define CLIST_LT_X							4
#define CLIST_LT_Y							7
#define CLIST_RB_X							(CLIST_LT_X + 24)
#define CLIST_RB_Y							(CLIST_LT_Y + 12)
	// ボタン先頭LCD領域
#define BUTTON_TOP_LT_X						(CLIST_LT_X + 18)
#define BUTTON_TOP_LT_Y						(CLIST_LT_Y + 2)
#define BUTTON_TOP_RB_X						(BUTTON_TOP_LT_X + 8)
#define BUTTON_TOP_RB_Y						(BUTTON_TOP_LT_Y + 2)
	// 入力ネームLCD領域
#define INPUT_NAME_LT_X						11
#define INPUT_NAME_LT_Y						3
	// キャラクタコード
#define CHAR_MODE_HKANA						0
#define CHAR_MODE_KKANA						1
#define CHAR_MODE_EISUU						2
#define CHAR_MODE_MAX						CHAR_MODE_EISUU

	// ニックネーム未入力キャラ
#define CHAR_USCORE							0x5181					// '＿' (0x8151)を逆転させたもの。

#define CHAR_LIST_CHAR_NUM					120

	// ニックネーム入力構造体
typedef struct Nickname {
	u8	 			input_flag;										// 入力フラグ
	u8	 			change_flag;									// 変更フラグ
	u8				length;											// 文字数
	u8				pad;
	u16				str[NCD_NICKNAME_LENGTH + 1];					// ニックネームコード
}Nickname;

	// カーソルX,Y位置（キャラ単位）
typedef struct CsrPos {
	u16 			x;												// x
	u16 			y;												// y
}CsrPos;

	// 誕生日情報
typedef struct Birthday {
	int				year;											// 年(1900-2099)
	int				month;											// 月(1-12)
	int				day;											// 日(1-31)
}Birthday;

	// プロフィール編集ワーク
typedef struct OwnerWork {
	u16				sel;
	Nickname		nickname;										// ニックネーム
	Birthday		birthday;										// 生年月日
	int				favoriteColor;									// 好きな色
	
	// ユーザーカラー＆生年月日入力ワーク
	int				seq;											// シーケンス番号
	int				*tgtp;											// 入力ターゲットへのポインタ
	InputNumParam	inp;											// 数値入力関数InputDecimal用パラメータ
	
	// ニックネーム入力ワーク
	u16				char_mode;										// 入力キャラクタモード（かな、カナ、英数）
	u16				rsv;											// 構造体CsrPosのアラインメント調整のためのパディング
	CsrPos			tpcsr;											// TPによって算出したカーソル位置
	CsrPos			csr_now;										// 現在のカーソル位置（キーとTPの両方から算出した有効な値）
	CsrPos			csr_old;										// 前フレームのカーソル位置
	u16				detach_count;									// TPが有効なカーソル位置から離されてからのカウント値
	u16				touch_count;									// TPが有効なカーソル位置でタッチされている状態のカウント値
	u16				handleTbl[CHAR_LIST_CHAR_NUM];					// キャラクタリスト用ハンドル格納配列
}OwnerWork;

// extern data----------------------------------


// function's prototype-------------------------
void SEQ_OwnerInfo_init(void);
int  SEQ_OwnerInfo(void);

static void SEQ_InputBirthday_init(void);
static int  SEQ_InputBirthday(void);
static void SEQ_InputFavoriteColor_init(void);
static int  SEQ_InputFavoriteColor(void);
static void DrawBirthday(u16 x, u16 y, u16 color, NvDate *birthp);
static void SEQ_InputNickname_init(void);
static int  SEQ_InputNickname(void);
static void ReturnMenu(int save_flag);
static u16  CalcTblIndex(CsrPos *csrp);
static void DeleteName1Char(void);
static void MoveCharCursor(int force_flag);
static BOOL MoveCharCursorTp(CsrPos *csrp);
static void DrawTargetCsrChar(CsrPos *csrp, u16 color);
static void DrawCharacterList(void);
static void SetSoftKeyboardButton(u16 char_mode);
static void SJISCodeExchangeCopy(u16 *srcp, u16 *dstp, u16 length);

// static variable------------------------------
static OwnerWork *ow;

// const data-----------------------------------
static const u16 char_tbl[3][CHAR_LIST_CHAR_NUM];

static const u8 *const str_ownerInfoSel[] ATTRIBUTE_ALIGN(2) = {
	(const u8 *)"NICKNAME  ",
	(const u8 *)"BIRTHDAY  ",
	(const u8 *)"USER COLOR",
};

const MenuComponent ownerInfoSel={
	OWNER_INFO_ELEM_NUM,
	OWNER_INFO_CSR_LT_X,
	OWNER_INFO_CSR_LT_Y,
	0,
	OWNER_INFO_CSR_NEXT_Y_NUM,
	23,
	WHITE,
	HIGHLIGHT_Y,
	(const u8 **)&str_ownerInfoSel,
};

static const u8  str_button_hkana[]	 ATTRIBUTE_ALIGN(2) = " かな   ";
static const u8  str_button_kkana[]	 ATTRIBUTE_ALIGN(2) = " カナ   ";
static const u8  str_button_eisuu[]	 ATTRIBUTE_ALIGN(2) = " ABC  ";
static const u8  str_button_del[]	 ATTRIBUTE_ALIGN(2) = " DEL  ";
static const u8  str_button_cancel[] ATTRIBUTE_ALIGN(2) = "CANCEL";
static const u8  str_button_ok[] 	 ATTRIBUTE_ALIGN(2) = "  OK  ";
static const u16 *str_button[] = {	NULL,
									NULL,
									(const u16 *)str_button_del,
									(const u16 *)str_button_cancel,
									(const u16 *)str_button_ok,
									};
//static const u16 str_uscore[] = {	uscore_, uscore_, uscore_, uscore_, uscore_, uscore_, uscore_, uscore_, EOM_};

//======================================================
// オーナー情報編集
//======================================================

// オーナー情報編集の初期化
void SEQ_OwnerInfo_init(void)
{
	u16 x,y;
	u16 temp[NCD_NICKNAME_LENGTH + 1];
	
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	MI_CpuClearFast(bgBakS,sizeof(bgBakS));
	SVC_CpuClearFast(0xc0,  oamBakS, sizeof(oamBakS));
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS( 1, 0, YELLOW, (const u8 *)"USER INFO SET");
	(void)DrawStringSJIS( RETURN_BUTTON_LT_X, RETURN_BUTTON_LT_Y,HIGHLIGHT_C, (const u8 *)" RETURN ");
	
	if(ow == NULL) {
		ow = NNS_FndAllocFromAllocator( &g_allocator, sizeof(OwnerWork) );			// オーナー情報編集用ワークの確保
#ifdef __SYSM_DEBUG
		if(ow == NULL) OS_Panic("ARM9- Fail to allocate memory...\n");
#endif /* __SYSM_DEBUG */
		OS_Printf("Alloc :OwnerWork\n");
		SVC_CpuClear(0x0000, ow, sizeof(OwnerWork), 16);
	}
	
	// オーナー情報のチェック
	{
		u32 dayNum;
		
		if(GetNCDWork()->owner.nickname.length > NCD_NICKNAME_LENGTH) {
			GetNCDWork()->owner.nickname.length = 0;
			SVC_CpuClear(0x0000, GetNCDWork()->owner.nickname.str, NCD_NICKNAME_LENGTH * 2, 16);
		}
		if((GetNCDWork()->owner.birthday.month == 0) || (GetNCDWork()->owner.birthday.month > 12)) {
			GetNCDWork()->owner.birthday.month = 1;
		}
		dayNum = SYSM_GetDayNum( 0, (u32)GetNCDWork()->owner.birthday.month );
		if((GetNCDWork()->owner.birthday.day == 0) || (GetNCDWork()->owner.birthday.day > dayNum)) {
			GetNCDWork()->owner.birthday.day = 1;
		}
		if( GetNCDWork()->owner.favoriteColor >= NCD_FAVORITE_COLOR_MAX_NUM ) {
			GetNCDWork()->owner.favoriteColor = 0;
		}
	}
	
	// オーナー情報の表示
	x = (u16)(ownerInfoSel.pos_x+13);
	y = (u16)ownerInfoSel.pos_y;
	SVC_CpuClear(0x0000, temp, sizeof(temp), 16);
	ExUTF16_LEtoSJIS_BE( (u8 *)temp, GetNCDWork()->owner.nickname.str, GetNCDWork()->owner.nickname.length);
	(void)DrawStringSJIS ( x, y, LIGHTGREEN, temp);
	DrawBirthday         ( x, (u16)(y + OWNER_INFO_CSR_NEXT_Y_NUM * 1), LIGHTGREEN, &GetNCDWork()->owner.birthday);
	ow->favoriteColor = GetNCDWork()->owner.favoriteColor;
	(void)DrawDecimalSJIS( x, (u16)(y + OWNER_INFO_CSR_NEXT_Y_NUM * 2), LIGHTGREEN, &ow->favoriteColor, 2, 1);
	
	DrawMenu(ow->sel, &ownerInfoSel);
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
}


// オーナー情報編集
int SEQ_OwnerInfo(void)
{
	BOOL tp_select;
	BOOL tp_return = FALSE;
	
	ReadTP();													// タッチパネル入力の取得
	
	if(tpd.disp.touch) {											// [RETURN]ボタン押下チェック
		tp_return = InRangeTp(RETURN_BUTTON_LT_X*8, RETURN_BUTTON_LT_Y*8-4,
							  RETURN_BUTTON_RB_X*8, RETURN_BUTTON_RB_Y*8-4, &tpd.disp);
	}
	
	// オーナー情報の初回起動シーケンス
	if(g_initialSet) {
		
		pad.trg = 0;
		
		if(GetNCDWork()->option.input_nickname == 0) {
			ow->sel = 0;
			pad.trg |= PAD_BUTTON_A;
		}else if(GetNCDWork()->option.input_favoriteColor == 0) {
			ow->sel = 2;
			pad.trg |= PAD_BUTTON_A;
		}else if(GetNCDWork()->option.input_birthday == 0) {
			ow->sel = 1;
			pad.trg |= PAD_BUTTON_A;
		}else if ( GetNCDWork()->option.input_nickname
			    || GetNCDWork()->option.input_favoriteColor
			    || GetNCDWork()->option.input_birthday   ) {
			pad.trg |= PAD_BUTTON_B;								// メニューに戻らす
		}
	}
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if(pad.trg & PAD_KEY_DOWN){										// カーソルの移動
		if(++ow->sel == OWNER_INFO_ELEM_NUM) ow->sel= 0;
	}
	if(pad.trg & PAD_KEY_UP){
		if(--ow->sel & 0x80) ow->sel = OWNER_INFO_ELEM_NUM - 1;
	}
	tp_select=SelectMenuByTp(&ow->sel, &ownerInfoSel);
	DrawMenu(ow->sel, &ownerInfoSel);
	
	if((pad.trg & PAD_BUTTON_A) || (tp_select)) {					// メニュー項目への分岐
		
		mf_clearRect( RETURN_BUTTON_LT_X, RETURN_BUTTON_LT_Y, 2, 8);
		DrawOKCancelButton();
		
		switch(ow->sel) {
			case 0:
				SEQ_InputNickname_init();
				g_pNowProcess = SEQ_InputNickname;
				break;
			case 1:
				SEQ_InputBirthday_init();
				g_pNowProcess = SEQ_InputBirthday;
				break;
			case 2:
				SEQ_InputFavoriteColor_init();
				g_pNowProcess = SEQ_InputFavoriteColor;
				break;
		}
	}else if((pad.trg & PAD_BUTTON_B) || (tp_return)) {				// メニューに戻る
		NNS_FndFreeToAllocator( &g_allocator, ow );					// ワークの解放
		ow = NULL;
		OS_Printf("Free :OwnerWork\n");
		SEQ_MainMenu_init();
	}
	
	return 0;
}


//======================================================
// 生年月日入力
//======================================================

// 生年月日入力の初期化
static void SEQ_InputBirthday_init(void)
{
	// 生年月日の表示
	DrawBirthday((u16)(ownerInfoSel.pos_x + 13), (u16)(ownerInfoSel.pos_y + OWNER_INFO_CSR_NEXT_Y_NUM * 1), WHITE, &GetNCDWork()->owner.birthday);
	// 生年月日情報のロード
	ow->birthday.month = (int)GetNCDWork()->owner.birthday.month;
	ow->birthday.day   = (int)GetNCDWork()->owner.birthday.day;
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	ow->seq = 0;
}


// 生年月日入力
static int SEQ_InputBirthday(void)
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	int x_base, y_base, abs_y_offset, new_seq;
	
	enum {															// 日付入力シーケンス番号
		SEQ_INIT = 0,
		SEQ_MONTH_INIT = 2, SEQ_MONTH_SET,
		SEQ_DAY_INIT,      	SEQ_DAY_SET,
		SEQ_END,
		SEQ_RETURN=64
	};
	
	
	ReadTP();													// タッチパネル入力の取得
	
	ow->inp.y_offset	= 0;
	
	CheckOKCancelButton(&tp_ok, &tp_cancel);
	
	if(tpd.disp.touch) {											// [CANCEL]ボタン押下チェック
		if((ow->seq & 0x01) && (ow->seq < SEQ_END)) {				// SEQ_**_SETの時のみ有効
			new_seq = ow->seq;
			x_base  = (ownerInfoSel.pos_x + 13) * 8;
			y_base  = (ownerInfoSel.pos_y + OWNER_INFO_CSR_NEXT_Y_NUM * 1) * 8 + 6;
			// 入力項目移動のチェック
			if( InRangeTp( x_base, (y_base - 6), (x_base + 80), (y_base + 6), &tpd.disp) ) {
				if(tpd.disp.x < x_base + 2 * 8) {
					new_seq = SEQ_MONTH_SET;
				}else if((tpd.disp.x >= x_base + 3 * 8) && (tpd.disp.x < x_base + 5 * 8)) {
					new_seq = SEQ_DAY_SET;
				}else if(tpd.disp.x >= x_base + 7 * 8) {
				}
			}
			if(ow->seq != new_seq) {
				ow->seq = new_seq - 1;
			}else {
				// 入力値の増減
				if(InRangeTp( ow->inp.pos_x * 8, (y_base - 30), (ow->inp.pos_x + ow->inp.keta_max) * 8, (y_base + 30), &tpd.disp)) {
					ow->inp.y_offset = tpd.disp.y - y_base;
					abs_y_offset     = (ow->inp.y_offset >= 0) ? ow->inp.y_offset : -ow->inp.y_offset;
					if(abs_y_offset <= 6) {
						ow->inp.y_offset   = 0;
					}else if(abs_y_offset <= 14){
						ow->inp.y_offset >>= 2;
					}else if(abs_y_offset <= 22){
						ow->inp.y_offset >>= 1;
					}
				}
			}
		}
	}
	
	// タッチパネル or キー入力によって、カーソル位置が動いた時に、元の位置のカーソルを消す。
	if((ow->seq > 0) && ((ow->seq & 0x01) == 0)) {					// SEQ_INITの時は実行しない
		(void)DrawDecimalSJIS( ow->inp.pos_x, ow->inp.pos_y, WHITE, ow->tgtp, (u8)ow->inp.keta_max, 4);
	}
	
	// 各シーケンスにおける処理
	switch(ow->seq){
	  case SEQ_INIT:
		ow->seq = SEQ_MONTH_INIT;
		// そのままSEQ_MONTH_INITへ
		
	  case SEQ_MONTH_INIT:
		ow->inp.pos_x		= DAY_LT_X;
		ow->inp.pos_y		= DAY_LT_Y;
		ow->inp.keta_max	= 2;
		ow->inp.value_max	= 12;
		ow->inp.value_min	= 1;
		ow->inp.y_offset	= 0;
		ow->tgtp			= (int *)&ow->birthday.month;
		break;
		
	  case SEQ_DAY_INIT:
		ow->inp.pos_x		= DAY_LT_X + 3;
		ow->inp.keta_max	= 2;
		ow->inp.value_min	= 1;
		ow->inp.value_max	= (int)SYSM_GetDayNum( 0, (u32)ow->birthday.month );
																	// 年・月をもとにその月の日数を算出する。
		if(ow->birthday.day > ow->inp.value_max) {
			ow->birthday.day = ow->inp.value_max;
		}
		ow->inp.y_offset	= 0;
		ow->tgtp			= (int *)&ow->birthday.day;
		break;
		
	  case SEQ_MONTH_SET:
	  case SEQ_DAY_SET:
		InputDecimal(ow->tgtp, &ow->inp);
		
		// 月入力ならば、日数を算出して、現在の入力日が日数を超えていたら修正する。
		if(ow->seq == SEQ_MONTH_SET) {
			u32 dayNum = SYSM_GetDayNum( 0, (u32)ow->birthday.month );
			if( dayNum < ow->birthday.day) {
				ow->birthday.day = (u8)dayNum;
				(void)DrawDecimalSJIS( DAY_LT_X + 3, DAY_LT_Y, WHITE, &ow->birthday.day, 2, 4);
			}
		}
		break;
		
	  case SEQ_END:
		GetNCDWork()->owner.birthday.month	= (u8 )ow->birthday.month;
		GetNCDWork()->owner.birthday.day	= (u8 )ow->birthday.day;
		GetNCDWork()->option.input_birthday	= 1;
		GetSYSMWork()->ncd_invalid			= 0;
		
		if ( GetNCDWork()->option.destroyFlashFlag ) {
			GetNCDWork()->option.destroyFlashFlag = 0;
		}
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
		
		// SEQ_ENDの時はこのままリターンする。
		
	  case SEQ_RETURN:
		SEQ_OwnerInfo_init();
		g_pNowProcess = SEQ_OwnerInfo;
		return 0;
	}
	
	if(ow->seq & 0x01) {											// SEQ_**_SETの時のみ有効
		if((pad.trg & PAD_BUTTON_A) || (tp_ok)) {
			ow->seq = SEQ_END;										// Aボタンで決定
		}else if((pad.trg & PAD_BUTTON_B) || (tp_cancel)) {			// Bボタンでキャンセル
			ow->seq = SEQ_RETURN;
		}else if(pad.trg & PAD_KEY_LEFT) {
			if(ow->seq == SEQ_MONTH_SET)	ow->seq = SEQ_DAY_INIT;
			else							ow->seq -= 3;
		}else if(pad.trg & PAD_KEY_RIGHT) {
			if(ow->seq == SEQ_DAY_SET)	ow->seq = SEQ_MONTH_INIT;
			else						ow->seq++;
		}
	}else {															// SEQ_**_INITの時のみ有効
		ow->seq++;
	}
	return 0;
}


// 生年月日の描画
static void DrawBirthday(u16 x, u16 y, u16 color, NvDate *birthp)
{
	(void)DrawStringSJIS ( (u16)(x + 2), y, color, (const u8 *)"/");
	(void)DrawDecimalSJIS( x, y, color, &birthp->month, 2, 1);
	(void)DrawDecimalSJIS( (u16)(x + 3), y, color, &birthp->day,   2, 1);
}


//======================================================
// 好きな色入力
//======================================================

// 好きな色入力の初期化
static void SEQ_InputFavoriteColor_init(void)
{
	// 好きな色のロード
	ow->favoriteColor = (int)GetNCDWork()->owner.favoriteColor;
	// 好きな色の表示
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	ow->seq = 0;
	
	if( g_initialSet ) {
		(void)DrawStringSJIS( 8, 18, RED, (const u8 *)"Select user color.");
	}
}


// 好きな色入力
static int SEQ_InputFavoriteColor(void)
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	int x_base, y_base, abs_y_offset, new_seq;
	
	enum {															// 日付入力シーケンス番号
		SEQ_INIT = 0, SEQ_SET,
		SEQ_END,
		SEQ_RETURN=64
	};
	
	
	ReadTP();													// タッチパネル入力の取得
	
	ow->inp.y_offset	= 0;
	
	CheckOKCancelButton(&tp_ok, &tp_cancel);
	
	if(tpd.disp.touch) {											// [CANCEL]ボタン押下チェック
		if((ow->seq & 0x01) && (ow->seq < SEQ_END)) {				// SEQ_**_SETの時のみ有効
			new_seq = ow->seq;
			x_base  = FCOLOR_LT_X * 8;
			y_base  = FCOLOR_LT_Y * 8 + 6;
			// 入力値の増減
			if(InRangeTp( ow->inp.pos_x * 8, (y_base - 30), (ow->inp.pos_x + ow->inp.keta_max) * 8, (y_base + 30), &tpd.disp)) {
				ow->inp.y_offset = tpd.disp.y - y_base;
				abs_y_offset     = (ow->inp.y_offset >= 0) ? ow->inp.y_offset : -ow->inp.y_offset;
				if(abs_y_offset <= 6) {
					ow->inp.y_offset   = 0;
				}else if(abs_y_offset <= 14){
					ow->inp.y_offset >>= 2;
				}else if(abs_y_offset <= 22){
					ow->inp.y_offset >>= 1;
				}
			}
		}
	}
	
	// 各シーケンスにおける処理
	switch(ow->seq){
	  case SEQ_INIT:
		ow->inp.pos_x		= FCOLOR_LT_X;
		ow->inp.pos_y		= FCOLOR_LT_Y;
		ow->inp.keta_max	= 2;
		ow->inp.value_max	= NCD_FAVORITE_COLOR_MAX_NUM - 1;
		ow->inp.value_min	= 0;
		ow->inp.y_offset	= 0;
		ow->tgtp			= (int *)&ow->favoriteColor;
		break;
		
	  case SEQ_SET:
		InputDecimal(ow->tgtp, &ow->inp);
		break;
		
	  case SEQ_END:
		GetNCDWork()->option.input_favoriteColor	=  1;
		GetNCDWork()->owner.favoriteColor			= (u8 )ow->favoriteColor;
		GetSYSMWork()->ncd_invalid					= 0;
		
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
		
		// SEQ_ENDの時はこのままリターンする。
		
	  case SEQ_RETURN:
		SEQ_OwnerInfo_init();
		g_pNowProcess = SEQ_OwnerInfo;
		return 0;
	}
	
	if(ow->seq & 0x01) {											// SEQ_**_SETの時のみ有効
		if((pad.trg & PAD_BUTTON_A) || (tp_ok)) {
			ow->seq = SEQ_END;										// Aボタンで決定
		}else if((pad.trg & PAD_BUTTON_B) || (tp_cancel)) {			// Bボタンでキャンセル
			ow->seq = SEQ_RETURN;
		}
	}else {															// SEQ_**_INITの時のみ有効
		ow->seq++;
	}
	return 0;
}


//======================================================
// ニックネームの入力
//======================================================

// ニックネーム入力の初期化
static void SEQ_InputNickname_init(void)
{
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	MI_CpuClearFast(bgBakS, sizeof(bgBakS));
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS( 1, 0, YELLOW, (const u8 *)"INPUT NICKNAME");
//	(void)DrawStringSJIS( INPUT_NAME_LT_Y, WHITE, (const u16 *)str_uscore);
	if( g_initialSet ) {
		(void)DrawStringSJIS( 8, 20, RED, (const u8 *)"Input nickname.");
	}
	
	// ニックネームをUTF16からSJISに変換してコピー
	SVC_CpuClear(CHAR_USCORE, ow->nickname.str, NCD_NICKNAME_LENGTH * 2, 16);
	if(GetSYSMWork()->ncd_invalid == 0) {
		ExUTF16_LEtoSJIS_BE( (u8 *)ow->nickname.str, GetNCDWork()->owner.nickname.str, GetNCDWork()->owner.nickname.length);
		ow->nickname.length = GetNCDWork()->owner.nickname.length;
		ow->nickname.input_flag = 1;
	}
	ow->nickname.change_flag = 0;
	(void)DrawStringSJIS( INPUT_NAME_LT_X, INPUT_NAME_LT_Y, WHITE, (const u16 *)ow->nickname.str);
	
	ow->char_mode = CHAR_MODE_HKANA;								// 「ひらがな」入力をセット
	SetSoftKeyboardButton(ow->char_mode);							// 初期ボタンのセット
	ow->detach_count = 0;
	ow->touch_count  = 0;
	ow->tpcsr.x      = 0;
	ow->tpcsr.y      = 0;
	ow->csr_now.x    = 0;
	ow->csr_now.y    = 0;
	ow->csr_now.x    = 15;
	ow->csr_now.y    = 5;
	DrawCharacterList();											// 入力キャラ一覧の描画
	MoveCharCursor(1);												// カーソル表示
	
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	
	GXS_SetVisiblePlane(GX_PLANEMASK_BG1);
}


// ニックネームの入力
static int SEQ_InputNickname(void)
{
	BOOL		tp_input = FALSE;
	u16			tbl_index, charCode;
	const u16	*char_listp;
	
	ow->csr_old = ow->csr_now;										// 前フレームのカーソル位置を保存
	
	ReadTP();													// タッチパネル入力の取得
	
	tp_input = MoveCharCursorTp(&ow->csr_now);						// TP入力によるカーソル移動
	
	// カーソル移動
	char_listp = char_tbl[ow->char_mode];
	if(pad.trg & PAD_KEY_UP) {
		while(1) {
			if(--ow->csr_now.y & 0x8000)	ow->csr_now.y = 5;
			tbl_index = CalcTblIndex(&ow->csr_now);
			if(char_listp[tbl_index]) {
				break;
			}else if(ow->csr_now.x > 15) {
				ow->csr_now.x = 15;
				ow->csr_now.y = 6;									// 次のループで-1されて5になる。
			}
		}
	}else if(pad.trg & PAD_KEY_DOWN) {
		while(1) {
			if(++ow->csr_now.y > 5)			ow->csr_now.y = 0;
			tbl_index = CalcTblIndex(&ow->csr_now);
			if(char_listp[tbl_index]) {
				break;
			}else if(ow->csr_now.x > 15) {
				ow->csr_now.x--;
				ow->csr_now.y--;
			}
		}
	}
	
	if(pad.trg & PAD_KEY_LEFT) {
		while(1) {
			if(--ow->csr_now.x & 0x8000) ow->csr_now.x = 18;
			tbl_index = CalcTblIndex(&ow->csr_now);
			if(char_listp[tbl_index]) {
				break;
			}
		}
	}else if(pad.trg & PAD_KEY_RIGHT) {
		while(1) {
			if(++ow->csr_now.x > 18)	ow->csr_now.x = 0;
			tbl_index = CalcTblIndex(&ow->csr_now);
			if(char_listp[tbl_index]) {
				break;
			}
		}
	}
	
	if(pad.trg & PAD_BUTTON_START) {
		ow->csr_now.x = 15;
		ow->csr_now.y = 5;
	}
	
	// 入力文字切り替え
	if(pad.trg & (PAD_BUTTON_R | PAD_BUTTON_L)) {
		if(pad.trg & PAD_BUTTON_R) {
			if(++ow->char_mode > CHAR_MODE_MAX)	ow->char_mode = 0;
		}else {
			if(--ow->char_mode & 0x8000) 		ow->char_mode = CHAR_MODE_MAX;
		}
		SetSoftKeyboardButton(ow->char_mode);
		while(1) {
			tbl_index = CalcTblIndex(&ow->csr_now);
			if(char_tbl[ow->char_mode][tbl_index]) {
				break;
			}
			ow->csr_now.x--;
		}
		DrawCharacterList();
	}
	
	charCode = (u16)char_listp[CalcTblIndex(&ow->csr_now)];
	MoveCharCursor(0);
	
	if((pad.trg & PAD_BUTTON_A)||(tp_input)) {
		// 右端コマンド
		if((charCode == VAR_BUTTON1_)||(charCode == VAR_BUTTON2_)) {// 入力文字切り替え
			ow->char_mode = (u16)(ow->char_mode + 1 + charCode - CODE_BUTTON_TOP_);
			if(ow->char_mode > CHAR_MODE_MAX) ow->char_mode -= CHAR_MODE_MAX+1;
			SetSoftKeyboardButton(ow->char_mode);
			DrawCharacterList();
		}else if(charCode == OK_BUTTON_) {							// 決定
			ReturnMenu(1);
		}else if(charCode == CANCEL_BUTTON_){						// キャンセル
			ReturnMenu(0);
		}else if(charCode == DEL_BUTTON_) { 						// １文字削除
			DeleteName1Char();
		}else {
			if(ow->nickname.length < NCD_NICKNAME_LENGTH) {			// 一文字入力
				ow->nickname.str[ow->nickname.length] = (u16)((charCode >> 8) | (charCode << 8));
																	// SJIS･ASCII混載文字列の際にこれらを判別できるよう、SJISをHi,Loの順で格納。
				ow->nickname.length++;
				ow->nickname.change_flag = 1;
				(void)DrawStringSJIS( INPUT_NAME_LT_X, INPUT_NAME_LT_Y, WHITE, ow->nickname.str);
			}
		}
	}else if(pad.trg & PAD_BUTTON_B) {
		if(!( (charCode >= CODE_BUTTON_TOP_)&&(charCode < CODE_BUTTON_BOTTOM_) )) {
			DeleteName1Char();										// １文字削除
		}else if(charCode == OK_BUTTON_) {
			ReturnMenu(0);											// [OK]ボタン上でBボタンならリターン
		}
	}
	
	return 0;
}


// メインメニューに戻る
static void ReturnMenu(int save_flag)
{
	int i;
	
	if((save_flag)&&(ow->nickname.change_flag)) {
		GetSYSMWork()->ncd_invalid = 0;
		GetNCDWork()->option.input_nickname = 1;				// ニックネーム入力フラグを立てる。
		
		ExSJIS_BEtoUTF16_LE( (u8 *)ow->nickname.str, GetNCDWork()->owner.nickname.str, ow->nickname.length);
																	// 入力されたネームをSJISからUTF16へ変換する。
		GetNCDWork()->owner.nickname.length = ow->nickname.length;
		for(i = ow->nickname.length; i < NCD_NICKNAME_LENGTH; i++) {// 入力された名前以降を0x0000で埋める。
			GetNCDWork()->owner.nickname.str[i] = 0x0000;
		}
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData(GetNCDWork());
	}
	SEQ_OwnerInfo_init();
	g_pNowProcess = SEQ_OwnerInfo;										// オーナー情報編集に戻る
}

// １文字削除
static void DeleteName1Char(void)
{
	if(ow->nickname.length == 0) return;
	
	ow->nickname.change_flag = 1;
	ow->nickname.length--;
	ow->nickname.str[ow->nickname.length] = CHAR_USCORE;
	(void)DrawStringSJIS( INPUT_NAME_LT_X, INPUT_NAME_LT_Y, WHITE, ow->nickname.str);
}


// カーソル位置から対応するキャラ番号を取得
static u16 CalcTblIndex(CsrPos *csrp)
{
	u16 tbl_index = 0;
	u16 x_bak = csrp->x;
	
	while(x_bak >= 5) {
		x_bak -= 5;
		tbl_index += 30;
	}
	tbl_index += x_bak + (csrp->y * 5);
	
	return tbl_index;
}


// カーソル移動
static void MoveCharCursor(int force_flag)
{
	if((*(u32 *)&ow->csr_now != *(u32 *)&ow->csr_old) || (force_flag)) {
		DrawTargetCsrChar(&ow->csr_old, WHITE);
		DrawTargetCsrChar(&ow->csr_now, HIGHLIGHT_Y);
	}
}


// カーソル位置のキャラクタを描画
static void DrawTargetCsrChar(CsrPos *csrp, u16 color)
{
	u16 index	 = CalcTblIndex(csrp);
	u16 charCode = (u16)char_tbl[ ow->char_mode ][ index ];
	
	if( (charCode >= CODE_BUTTON_TOP_) && (charCode < CODE_BUTTON_BOTTOM_) ) {
		if(color == WHITE) {
			color = HIGHLIGHT_C;
		}
	}
	
	if(ChangeColorSJIS(ow->handleTbl[ index ], color) == 0) {
		OS_Printf("this handle is not found. %x\n", ow->handleTbl[ index ]);
	}
}


// TPによるカーソル移動
static BOOL MoveCharCursorTp(CsrPos *csrp)
{
	int		x_bak;
	BOOL	active = FALSE;
	CsrPos	temp;
	
	if(ow->detach_count) {
		if(tpd.disp.touch == 0) {
			if(++ow->detach_count == TP_CSR_DETACH_COUNT) {
				ow->detach_count = 0;
				return TRUE;
			}else {
				return FALSE;
			}
		}
	}
	ow->detach_count = 0;
	
	if(tpd.disp.touch) {
		if(InRangeTp(CLIST_LT_X*8, CLIST_LT_Y*8-4, CLIST_RB_X*8-1, CLIST_RB_Y*8-4, &tpd.disp)) {
																		// 少しマージンあり。
			temp.x = (u16)((tpd.disp.x - CLIST_LT_X * 8) / 8);
			temp.y = (u16)((tpd.disp.y - (CLIST_LT_Y * 8)) / 16);
			x_bak = temp.x;												// ５文字ごとにある空白列の補正
			while(x_bak >= 5) {
				x_bak -= 6;
				temp.x--;
			}
			
			if(temp.y == 0) {											// 右端余白の補正
				if(temp.x > 18) temp.x = 18;
			}else if(temp.x > 15) {
				temp.x = 15;
			}
			if(*(u32 *)&temp == *(u32 *)&ow->tpcsr) {					// 今回のTPカーソル位置が前回と同じなら、カウントを進めて、
				if(ow->touch_count < TP_CSR_TOUCH_COUNT) {				// 規定値に達したら有効な位置とする。
					ow->touch_count++;
				}else {
					csrp->x = temp.x;
					csrp->y = temp.y;
				}
				return FALSE;
			}
		}
	}else {	// touch == 0
		if(ow->touch_count == TP_CSR_TOUCH_COUNT) {
			ow->detach_count = 1;
		}
	}
	ow->tpcsr.x = temp.x;
	ow->tpcsr.y = temp.y;
	ow->touch_count = 0;
	return FALSE;
}


// 現在のcharmodeのキャラクタ一覧を描画
static void DrawCharacterList(void)
{
	u16 i, j, k, x, y, index, button;
	const u16 *code;
	u16 str[2];
	
	// キャラリストの削除
	for( i = 0; i < CHAR_LIST_CHAR_NUM; i++ ) {
		if( ow->handleTbl[ i ] ) {
			ClearStringSJIS_handle( ow->handleTbl[ i ] );
		}
	}
	
	// キャラクタリストの描画
	code	= char_tbl[ow->char_mode];
	str[1]	= 0x0000;
	index	= 0;
	button	= 0;
	for(i = 0; i < 4; i++) {
//		buffp = bgBakS + CLIST_LT_X + (CLIST_LT_Y << 5) + (6 * i);
		for(j = 0; j < 6; j++) {
			x = (u16)(CLIST_LT_X + (i * 6));
			y = (u16)(CLIST_LT_Y + (j * 2));
			for(k = 0; k < 5; k++) {
				if(*code != EOM_) {
					if( (*code >= CODE_BUTTON_TOP_) && (*code < CODE_BUTTON_BOTTOM_) ) {	// ボタン
						ow->handleTbl[ index ] = DrawStringSJIS( x, y, HIGHLIGHT_C, str_button[ button++ ]);
					}else {																	// キャラクタ
						str[0] = (u16)( (*code >> 8) | ( *code << 8) );
						ow->handleTbl[ index ] = DrawStringSJISEx( x, y, WHITE, str, index );
					}
				}
				index++;
				code++;
				x++;
			}
		}
	}
	MoveCharCursor(1);
}


// ソフトキーボードのボタンを設定
static void SetSoftKeyboardButton(u16 char_mode)
{
	if(char_mode == 0) {
		str_button[0] = (const u16 *)str_button_kkana;		// 1
		str_button[1] = (const u16 *)str_button_eisuu;		// 2
	}else if(char_mode == 1) {
		str_button[0] = (const u16 *)str_button_eisuu;		// 2
		str_button[1] = (const u16 *)str_button_hkana;		// 0
	}else {
		str_button[0] = (const u16 *)str_button_hkana;		// 0
		str_button[1] = (const u16 *)str_button_kkana;		// 1
	}
}


// SJISコードをコードHi/Loを逆転しながらコピー
static void SJISCodeExchangeCopy(u16 *srcp, u16 *dstp, u16 length)
{
	while(length--) {
		*dstp++ = (u16)( (*srcp >> 8) | (*srcp << 8) );
		srcp++;
	}
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

static const u16 char_tbl[3][CHAR_LIST_CHAR_NUM] = {
	{	// ひらがな
		'あ',	'い',	'う',	'え',	'お',
		'か',	'き',	'く',	'け',	'こ',
		'さ',	'し',	'す',	'せ',	'そ',
		'た',	'ち',	'つ',	'て',	'と',
		'な',	'に',	'ぬ',	'ね',	'の',
		'は',	'ひ',	'ふ',	'へ',	'ほ',
		
		'ま',	'み',	'む',	'め',	'も',
		'や',	'　',	'ゆ',	'　',	'よ',
		'ら',	'り',	'る',	'れ',	'ろ',
		'わ',	'　',	'を',	'　',	'ん',
		'ぁ',	'ぃ',	'ぅ',	'ぇ',	'ぉ',
		'ゃ',	'　',	'ゅ',	'　',	'ょ',
		
		'が',	'ぎ',	'ぐ',	'げ',	'ご',
		'ざ',	'じ',	'ず',	'ぜ',	'ぞ',
		'だ',	'ぢ',	'づ',	'で',	'ど',
		'ば',	'び',	'ぶ',	'べ',	'ぼ',
		'ぱ',	'ぴ',	'ぷ',	'ぺ',	'ぽ',
		'っ',	'、',	'。',	'！',	'ー',
		
		'「',	'」',	'～',	'・',	'　',
		VAR_BUTTON1_,	EOM_,	EOM_,	EOM_,	EOM_,
		VAR_BUTTON2_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	EOM_,	EOM_,	EOM_,	EOM_,
		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// カタカナ
		'ア',	'イ',	'ウ',	'エ',	'オ',
		'カ',	'キ',	'ク',	'ケ',	'コ',
		'サ',	'シ',	'ス',	'セ',	'ソ',
		'タ',	'チ',	'ツ',	'テ',	'ト',
		'ナ',	'ニ',	'ヌ',	'ネ',	'ノ',
		'ハ',	'ヒ',	'フ',	'ヘ',	'ホ',
		
		'マ',	'ミ',	'ム',	'メ',	'モ',
		'ヤ',	'　',	'ユ',	'　',	'ヨ',
		'ラ',	'リ',	'ル',	'レ',	'ロ',
		'ワ',	'　',	'ヲ',	'　',	'ン',
		'ァ',	'ィ',	'ゥ',	'ェ',	'ォ',
		'ャ',	'　',	'ュ',	'　',	'ョ',
		
		'ガ',	'ギ',	'グ',	'ゲ',	'ゴ',
		'ザ',	'ジ',	'ズ',	'ゼ',	'ゾ',
		'ダ',	'ヂ',	'ヅ',	'デ',	'ド',
		'バ',	'ビ',	'ブ',	'ベ',	'ボ',
		'パ',	'ピ',	'プ',	'ペ',	'ポ',
		'ッ',	'、',	'。',	'！',	'ー',
		
		'「',	'」',	'～',	'・',	'　',
		VAR_BUTTON1_,	EOM_,	EOM_,	EOM_,	EOM_,
		VAR_BUTTON2_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	EOM_,	EOM_,	EOM_,	EOM_,
		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// 英数
		'Ａ',	'Ｂ',	'Ｃ',	'Ｄ',	'Ｅ',
		'Ｆ',	'Ｇ',	'Ｈ',	'Ｉ',	'Ｊ',
		'Ｋ',	'Ｌ',	'Ｍ',	'Ｎ',	'Ｏ',
		'Ｐ',	'Ｑ',	'Ｒ',	'Ｓ',	'Ｔ',
		'Ｕ',	'Ｖ',	'Ｗ',	'Ｘ',	'Ｙ',
		'Ｚ',	'　',	'　',	'　',	'　',
		
		'ａ',	'ｂ',	'ｃ',	'ｄ',	'ｅ',
		'ｆ',	'ｇ',	'ｈ',	'ｉ',	'ｊ',
		'ｋ',	'ｌ',	'ｍ',	'ｎ',	'ｏ',
		'ｐ',	'ｑ',	'ｒ',	'ｓ',	'ｔ',
		'ｕ',	'ｖ',	'ｗ',	'ｘ',	'ｙ',
		'ｚ',	'　',	'　',	'　',	'　',
		
		'０',	'１',	'２',	'３',	'４',
		'５',	'６',	'７',	'８',	'９',
		'！',	'　',	'＆',	'　',	'／',
		'，',	'　',	'．',	'　',	'－',
		'’',	'　',	'”',	'　',	'　',
		'＠',	'　',	'（',	'　',	'）',
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		VAR_BUTTON1_,	EOM_,	EOM_,	EOM_,	EOM_,
		VAR_BUTTON2_,	EOM_,	EOM_,	EOM_,	EOM_,
		DEL_BUTTON_,	EOM_,	EOM_,	EOM_,	EOM_,
		CANCEL_BUTTON_,	EOM_,	EOM_,	EOM_,	EOM_,
		OK_BUTTON_,		EOM_,	EOM_,	EOM_,	EOM_,
	},
};

