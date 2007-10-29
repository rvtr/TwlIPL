/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     rtcSet.c

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

// define data------------------------------------------

//#define __RTC_MINUTE_OFFSET										// この定義が有効な場合はrtcOffsetは分オフセットで算出されます。また、無効な場合は秒オフセットとなります。

	// RETURNボタンLCD領域
#define RETURN_BUTTON_LT_X					2
#define RETURN_BUTTON_LT_Y					21
#define RETURN_BUTTON_RB_X					(RETURN_BUTTON_LT_X + 8)
#define RETURN_BUTTON_RB_Y					(RETURN_BUTTON_LT_Y + 2)
	// 日付データLCD領域
#define DATE_LT_X							5
#define DATE_LT_Y							10
	// 時刻データLCD領域
#define TIME_LT_X							(DATE_LT_X + 14)
#define TIME_LT_Y							DATE_LT_Y

	// RTC設定メニュー要素
#define RTC_MENU_ELEM_NUM					1

	// 文字入力タッチパネル用カウンタ
#define S_UPDOWN_COUNT_MAX					16
	// 数値入力タッチパネル用カウンタ
#define D_DOWN_COUNT_MAX					-50
#define D_UP_COUNT_MAX						50


	// 日付時刻入力シーケンス用ワーク
typedef struct DateTimeParam {
	int			seq;						// シーケンス番号
	int			*tgtp;						// 入力対象の変数へのポインタ
	RTCDate		Date;
	RTCTime		Time;
}DateTimeParam;


	// RTC設定シーケンス用ワーク
typedef struct SetRtcWork {
	int				csr;					// カーソル位置
	s64				rtcOffset[2];			// RTCオフセット値（[0]:設定変更前の値、[1]:変更後の値）
	DateTimeParam	dtp;					// 日付時刻入力シーケンス用ワーク
	InputNumParam	inp;					// 数値入力インターフェース用ワーク
}SetRtcWork;

// extern data------------------------------------------

// function's prototype declaration---------------------
void SEQ_RtcSet_init(void);
int  SEQ_RtcSet(void);
RTCWeek CalcWeekFromDate( u32 year, u32 month, u32 day );
void InputDecimal(int *tgtp, InputNumParam *inpp);

static void SEQ_InputRtcDateTime_init(int start);
static int  SEQ_InputRtcDateTime(void);
static void TransmitRtcData(DateTimeParam *dtpp, RtcDateTime *rtcp);
static void SelectString( int *tgtp, const u8 **const strpp, InputNumParam *inpp);
static void BcdToHex(int *bcdp);
static void HexToBcd(int *hexp);
static BOOL CheckLeapYear( u32 year );

// global variable -------------------------------------

// static variable -------------------------------------
SetRtcWork *pRtcWork;									// RTC設定用ワーク

// const data  -----------------------------------------

//======================================================
// 日付＆時刻設定
//======================================================

// RTC設定シーケンスの初期化
void SEQ_RtcSet_init(void)
{
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	MI_CpuClearFast(bgBakS, sizeof(bgBakS));
	SVC_CpuClearFast(0xc0,  oamBakS, sizeof(oamBakS));
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS( 1, 0, YELLOW, (const u8 *)"DATE & TIME SET");
	(void)DrawStringSJIS( DATE_LT_X + 3, DATE_LT_Y, WHITE, (const u8 *)"/   /   [    ]   :   :");
	(void)DrawStringSJIS( RETURN_BUTTON_LT_X, RETURN_BUTTON_LT_Y, HIGHLIGHT_C, (const u8 *)" RETURN ");
	if( initialSet ) {
		if( GetSYSMWork()->rtcStatus & 0x01) {
			(void)DrawStringSJIS( 8, 18, RED, (const u8 *)"RTC reset is detected!");
		}else {
			(void)DrawStringSJIS( 8, 18, RED, (const u8 *)"Set RTC.");
		}
	}
	
	pRtcWork=OS_Alloc(sizeof(SetRtcWork));								// RTC設定用ワークの確保
#ifdef __SYSM_DEBUG
	if(pRtcWork==NULL) OS_Panic("ARM9- Fail to allocate memory...\n");
#endif /* __SYSM_DEBUG */
	OS_Printf("Alloc :SetRtcWork\n");
	SVC_CpuClear(0x0000, pRtcWork, sizeof(SetRtcWork), 16);
	
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	
	InitGetAndDrawRtcData( DATE_LT_X, DATE_LT_Y, TIME_LT_X, TIME_LT_Y);	// RTCデータ表示位置の指定
	
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
	
/*	if(0){
		s64 offset;
		RTCDate date;
		RTCTime time;
		
		date.year	= 99;
		date.month	= 12;
		date.day	= 31;
		time.hour	= 23;
		time.minute	= 59;
		time.second = 0;
		offset = IPL2i_CalcRtcSecOffset( &date, &time );			// 設定直前のRTC値のオフセットを算出
		OS_Printf( " 99/12/31 23:59:00  offset = %x\n", offset );
		date.year	= 100;
		date.month	= 1;
		date.day	= 1;
		time.hour	= 0;
		time.minute	= 0;
		time.second = 0;
		offset = IPL2i_CalcRtcSecOffset( &date, &time );			// 設定直前のRTC値のオフセットを算出
		OS_Printf( "100/01/01 00:00:00  offset = %x\n", offset );
	}
*/
}


// RTC設定シーケンス
int SEQ_RtcSet(void)
{
	BOOL tp_set		= FALSE;
	BOOL tp_return	= FALSE;
	
	ReadTpData();													// タッチパネル入力の取得
	GetAndDrawRtcData();
	
	if(tpd.disp.touch) {
		tp_set = InRangeTp( DATE_LT_X*8,       DATE_LT_Y*8-4,		// [RTC設定]領域押下チェック
						   (TIME_LT_X + 8)*8, (TIME_LT_Y+2)*8-4, &tpd.disp);
																	// [RETURN]ボタン押下チェック
		tp_return = InRangeTp(RETURN_BUTTON_LT_X*8, RETURN_BUTTON_LT_Y*8-4,
							  RETURN_BUTTON_RB_X*8, RETURN_BUTTON_RB_Y*8-4, &tpd.disp);
	}
	if( initialSet && !GetNCDWork()->option.input_rtc ) {
		tp_set = TRUE;
	}
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if(pad.trg & PAD_KEY_DOWN){										// カーソルの移動
		if(++pRtcWork->csr == RTC_MENU_ELEM_NUM)	pRtcWork->csr = 0;
	}
	if(pad.trg & PAD_KEY_UP){
		if(--pRtcWork->csr < 0) pRtcWork->csr = RTC_MENU_ELEM_NUM - 1;
	}
	
	if((pad.trg & PAD_BUTTON_A) || (tp_set)) {						// RTC設定開始
		if(pRtcWork->csr == 0) {
			SEQ_InputRtcDateTime_init(1);
			nowProcess = SEQ_InputRtcDateTime;
		}
	}else if((pad.trg & PAD_BUTTON_B) || (tp_return)) {				// メニューに戻る
		OS_Free(pRtcWork);												// RTC設定用ワークの解放
		pRtcWork = NULL;
		OS_Printf("Free :SetRtcWork\n");
		SEQ_MainMenu_init();
	}
	
#ifdef __SYSM_DEBUG
	if(pad.trg & PAD_BUTTON_START) {
		ClearRTC();
		OS_Printf("RTC offset in NVRAM is ZERO clear!\n");
	}
#endif /* __SYSM_DEBUG */
	
	return 0;
}


//======================================================
// 日付＆時刻入力処理
//======================================================

// 日付時刻入力初期化
static void SEQ_InputRtcDateTime_init(int start)
{
	mf_clearRect( RETURN_BUTTON_LT_X, RETURN_BUTTON_LT_Y, 2, 28);
	if(start) {
		DrawOKCancelButton();
		pRtcWork->dtp.seq = 0;
	}else {
		(void)DrawStringSJIS( RETURN_BUTTON_LT_X, RETURN_BUTTON_LT_Y, HIGHLIGHT_C, (const u8 *)" RETURN ");
	}
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
}


// 日付時刻入力
static int SEQ_InputRtcDateTime(void)
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	int  new_seq, x_base, y_base, abs_y_offset;
	
	enum {															// 日付時刻入力シーケンス番号
		SEQ_INIT=0,
		SEQ_YEAR_INIT=2, SEQ_YEAR_SET,
		SEQ_MONTH_INIT,  SEQ_MONTH_SET,
		SEQ_DAY_INIT,    SEQ_DAY_SET,
		SEQ_HOUR_INIT,   SEQ_HOUR_SET,
		SEQ_MINUTE_INIT, SEQ_MINUTE_SET,
		SEQ_SECOND_INIT, SEQ_SECOND_SET,
		SEQ_END,
		SEQ_RETURN=64
	};
	
	ReadTpData();													// タッチパネル入力の取得
	CheckOKCancelButton(&tp_ok, &tp_cancel);						// [OK],[CANCEL]ボタン押下チェック
	
	pRtcWork->inp.y_offset = 0;
	if(tpd.disp.touch) {											// [CANCEL]ボタン押下チェック
		if((pRtcWork->dtp.seq & 0x01) && (pRtcWork->dtp.seq < SEQ_END)) {		// SEQ_**_SETの時のみ有効
			new_seq = pRtcWork->dtp.seq;
			x_base  = DATE_LT_X * 8;
			y_base  = DATE_LT_Y * 8 + 6;
			// 入力項目移動のチェック
			if( InRangeTp( x_base, (y_base - 6), (x_base + 22 * 8), (y_base + 6), &tpd.disp) ) {
				if(tpd.disp.x < x_base + 28) {
					new_seq = SEQ_YEAR_SET;
				}else if((tpd.disp.x >= x_base +  4*8) && (tpd.disp.x < x_base +  6*8)) {
					new_seq = SEQ_MONTH_SET;
				}else if((tpd.disp.x >= x_base +  7*8) && (tpd.disp.x < x_base +  9*8)) {
					new_seq = SEQ_DAY_SET;
				}else if((tpd.disp.x >= x_base + 14*8) && (tpd.disp.x < x_base + 16*8)) {
					new_seq = SEQ_HOUR_SET;
				}else if((tpd.disp.x >= x_base + 17*8) && (tpd.disp.x < x_base + 19*8)) {
					new_seq = SEQ_MINUTE_SET;
				}else if(tpd.disp.x >= x_base + 20*8) {
					new_seq = SEQ_SECOND_SET;
				}
			}
			if(pRtcWork->dtp.seq != new_seq) {
				pRtcWork->dtp.seq = new_seq - 1;
			}else {
				// 入力値の増減
				if(InRangeTp( pRtcWork->inp.pos_x * 8, (y_base - 30), (pRtcWork->inp.pos_x + pRtcWork->inp.keta_max)*8, (y_base + 30), &tpd.disp)) {
					pRtcWork->inp.y_offset = tpd.disp.y - y_base;
					abs_y_offset     = (pRtcWork->inp.y_offset >= 0) ? pRtcWork->inp.y_offset : -pRtcWork->inp.y_offset;
					if(abs_y_offset <= 6) {
						pRtcWork->inp.y_offset   = 0;
					}else if(abs_y_offset <= 14){
						pRtcWork->inp.y_offset >>= 2;
					}else if(abs_y_offset <= 22){
						pRtcWork->inp.y_offset >>= 1;
					}
				}
			}
		}
	}
	
	// タッチパネル or キー入力によって、カーソル位置が動いた時に、元の位置のカーソルを消す。
	if((pRtcWork->dtp.seq > 0) && ((pRtcWork->dtp.seq & 0x01) == 0)) {		// SEQ_INITの時は実行しない
		(void)DrawDecimalSJIS( pRtcWork->inp.pos_x, pRtcWork->inp.pos_y, WHITE, pRtcWork->dtp.tgtp, (u8)pRtcWork->inp.keta_max, 4);
	}
	
	// 各シーケンスの処理
	switch(pRtcWork->dtp.seq){
		
	  case SEQ_INIT:
		pRtcWork->dtp.Date		= GetSYSMWork()->rtc[0].Date;
		pRtcWork->dtp.Time		= GetSYSMWork()->rtc[0].Time;
		pRtcWork->dtp.Date.year += 2000;									// yearを＋２０００する。
		pRtcWork->dtp.seq		= SEQ_YEAR_INIT;
		// ※SEQ_INITは直通でSEQ_YEAR_INITへ
		
	  case SEQ_YEAR_INIT:
		pRtcWork->inp.pos_x		= DATE_LT_X;
		pRtcWork->inp.pos_y		= DATE_LT_Y;
		pRtcWork->inp.keta_max	= 4;
		pRtcWork->inp.value_max	= 2099;
		pRtcWork->inp.value_min	= 2000;
//		pRtcWork->inp.value_min	= 2004;
//		if(pRtcWork->dtp.Date.year < 2004) {
//			pRtcWork->dtp.Date.year = 2004;
//		}
		pRtcWork->dtp.tgtp		= (int *)&pRtcWork->dtp.Date.year;
		break;
		
	  case SEQ_MONTH_INIT:
		pRtcWork->inp.pos_x		= DATE_LT_X + 4;
		pRtcWork->inp.keta_max	= 2;
		pRtcWork->inp.value_max	= 12;
		pRtcWork->inp.value_min	= 1;
		pRtcWork->dtp.tgtp		= (int *)&pRtcWork->dtp.Date.month;
		break;
		
	  case SEQ_DAY_INIT:
		pRtcWork->inp.pos_x		= DATE_LT_X + 7;
		pRtcWork->inp.keta_max	= 2;
		pRtcWork->inp.value_max	= (int)SYSM_GetDayNum( pRtcWork->dtp.Date.year, pRtcWork->dtp.Date.month );
																	// 年・月をもとにその月の日数を算出する。
		pRtcWork->inp.value_min	= 1;
		if(pRtcWork->dtp.Date.day > pRtcWork->inp.value_max) {
			pRtcWork->dtp.Date.day = (u32)pRtcWork->inp.value_max;
		}
		pRtcWork->dtp.tgtp		= (int *)&pRtcWork->dtp.Date.day;
		break;
		
	  case SEQ_HOUR_INIT:
		pRtcWork->inp.pos_x		= TIME_LT_X;
		pRtcWork->inp.keta_max	= 2;
		pRtcWork->inp.value_max	= 23;
		pRtcWork->inp.value_min	= 0;
		pRtcWork->dtp.tgtp		= (int *)&pRtcWork->dtp.Time.hour;
		break;
		
	  case SEQ_MINUTE_INIT:
		pRtcWork->inp.pos_x		= TIME_LT_X + 3;
		pRtcWork->inp.keta_max	= 2;
		pRtcWork->inp.value_max	= 59;
		pRtcWork->inp.value_min	= 0;
		pRtcWork->dtp.tgtp		= (int *)&pRtcWork->dtp.Time.minute;
		break;
		
	  case SEQ_SECOND_INIT:
		pRtcWork->inp.pos_x		= TIME_LT_X + 6;
		pRtcWork->inp.keta_max	= 2;
		pRtcWork->inp.value_max	= 59;
		pRtcWork->inp.value_min	= 0;
		pRtcWork->dtp.tgtp		= (int *)&pRtcWork->dtp.Time.second;
		break;
		
	  case SEQ_YEAR_SET:
	  case SEQ_MONTH_SET:
	  case SEQ_DAY_SET:
	  case SEQ_HOUR_SET:
	  case SEQ_MINUTE_SET:
	  case SEQ_SECOND_SET:
		InputDecimal( pRtcWork->dtp.tgtp, &pRtcWork->inp);
		
		// 年月日入力ならば、曜日を算出して表示。
		if( (pRtcWork->dtp.seq == SEQ_YEAR_SET) || (pRtcWork->dtp.seq == SEQ_MONTH_SET) || (pRtcWork->dtp.seq == SEQ_DAY_SET) ) {
			pRtcWork->dtp.Date.week = CalcWeekFromDate(pRtcWork->dtp.Date.year, pRtcWork->dtp.Date.month, pRtcWork->dtp.Date.day);
			(void)DrawStringSJIS( DATE_LT_X + 10, DATE_LT_Y, WHITE, g_strWeek[pRtcWork->dtp.Date.week]);
		}
		
		// 年・月入力ならば、日数を算出して、現在の入力日が日数を超えていたら修正する。
		if( (pRtcWork->dtp.seq == SEQ_YEAR_SET) || (pRtcWork->dtp.seq == SEQ_MONTH_SET) ) {
			u32 dayNum = SYSM_GetDayNum( pRtcWork->dtp.Date.year, pRtcWork->dtp.Date.month );
			if( dayNum < pRtcWork->dtp.Date.day) {
				pRtcWork->dtp.Date.day = dayNum;
				(void)DrawDecimalSJIS( DATE_LT_X + 7, DATE_LT_Y, WHITE, &pRtcWork->dtp.Date.day, 2, 4);
			}
		}
		break;
		
	  case SEQ_END:
		pRtcWork->dtp.Date.year -= 2000;									// yearを－２０００する。
		
/*		// RTCへの新しい値の設定
		(void)RTC_GetDateTime(&now_dtp.Date, &now_dtp.Time);		// ライト直前に現在のRTC値を取得する。
		(void)RTC_SetDateTime(&pRtcWork->dtp.Date, &pRtcWork->dtp.Time);		// 新RTC設定値のセット。
		
		if((GetSYSMWork()->rtc[0].Date.year == 99) && (now_dtp.Date.year == 0)) {
			now_dtp.Date.year = 100;								// 設定前～設定完了の間にRTCが一周してしまったら、yearは100としてoffsetを計算する。
		}
		// RTC設定時は、今回の設定でどれだけRTC値が変化したか（秒オフセット単位）を算出してNVRAMに保存する。（とりあえず実装）
		pRtcWork->rtcOffset[0] = IPL2i_CalcRtcSecOffset( &now_dtp.Date,  &now_dtp.Time );	// 現在のRTC値のオフセットを算出
		pRtcWork->rtcOffset[1] = IPL2i_CalcRtcSecOffset( &pRtcWork->dtp.Date, &pRtcWork->dtp.Time );	// 新しくセットされたRTC値のオフセットを算出
		GetNCDWork()->option.rtcOffset += pRtcWork->rtcOffset[1] - pRtcWork->rtcOffset[0];
																	// 新RTC_ofs と 現在のRTC_ofs の差分の値を加算。
*/
		
		pRtcWork->dtp.Time.second = 0;
		NCD_SetRtcOffset( SYSM_CalcRtcOffsetAndSetDateTime( &pRtcWork->dtp.Date, &pRtcWork->dtp.Time ) );
		
		GetSYSMWork()->rtc[0].Date = pRtcWork->dtp.Date;
		GetSYSMWork()->rtc[0].Time = pRtcWork->dtp.Time;
		GetSYSMWork()->ncd_invalid = 0;
		GetNCDWork()->option.input_rtc = 1;						// RTC入力フラグを立てる。
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
		
		// SEQ_ENDの時はこのままリターンする。
		
	  case SEQ_RETURN:
		nowProcess = SEQ_RtcSet;
		SEQ_InputRtcDateTime_init(0);								// 日付入力画面のクリア
		return 0;
	}
	
	if(pRtcWork->dtp.seq & 0x01) {										// SEQ_**_SETの時のみ有効
		if((pad.trg & PAD_BUTTON_A) || (tp_ok)) {
			pRtcWork->dtp.seq = SEQ_END;									// Aボタンで決定
		}else if((pad.trg & PAD_BUTTON_B) || (tp_cancel)) {			// Bボタンでキャンセル
			pRtcWork->dtp.seq = SEQ_RETURN;
		}else if(pad.trg & PAD_KEY_LEFT) {
			if(pRtcWork->dtp.seq == SEQ_YEAR_SET)	pRtcWork->dtp.seq = SEQ_SECOND_INIT;
			else								pRtcWork->dtp.seq -= 3;
		}else if(pad.trg & PAD_KEY_RIGHT) {
			if(pRtcWork->dtp.seq == SEQ_SECOND_SET)	pRtcWork->dtp.seq = SEQ_YEAR_INIT;
			else								pRtcWork->dtp.seq++;
		}
	}else {															// SEQ_**_INITの時のみ有効
		pRtcWork->dtp.seq++;
	}
	return 0;
}


/*
// うるう年の判定 (うるう年：１、通常の年：０リターン）
BOOL CheckLeapYear( u32 year)
{
	if((year & 0x03) == 0) {										// うるう年は、「4で割り切れ　かつ　100で割り切れない年」
		CP_SetDiv32_32(year, 100);									//             「400で割り切れる年」
		if(CP_GetDivRemainder32() != 0) {
			return TRUE;
		}else {
			CP_SetDiv32_32(year, 400);
			if(CP_GetDivRemainder32() == 0) {
				return TRUE;
			}
		}
	}
	return FALSE;
}
*/

// 日付から曜日を求める。
RTCWeek CalcWeekFromDate( u32 year, u32 month, u32 day )
{
	if(month == 1 || month == 2 ){
		year--;
		month += 12;
	}
	return (RTCWeek)( (year + year/4 - year/100 + year/400 + (13*month + 8)/5 + day) % 7 );
}


/*
// 文字列によるパラメータ選択
static void SelectString(int *tgtp, const u8 **const srtpp, InputNumParam *inpp)
{
	BOOL value_up   = FALSE;
	BOOL value_down = FALSE;
	
	if(inpp->y_offset == 0) {
		inpp->up_count = S_UPDOWN_COUNT_MAX;
	}else {
		inpp->up_count ++;
		if(inpp->up_count > S_UPDOWN_COUNT_MAX) {
			inpp->up_count = 0;
			if(inpp->y_offset < 0)  value_up   = TRUE;
			else					value_down = TRUE;
		}
	}
	
	if((pad.trg & PAD_KEY_DOWN) || (value_down)) {					// 表示文字列切り替え
		if(++*tgtp>inpp->value_max)	*tgtp = 0;
	}else if((pad.trg & PAD_KEY_UP) || (value_up)) {
		if(--*tgtp & 0x8000)		*tgtp = inpp->value_max;
	}
	
	(void)DrawStringSJIS( inpp->pos_x, inpp->pos_y, HIGHLIGHT_Y, srtpp[*tgtp]);	// 現在選択している文字列を表示
}
*/

// １０進数数値入力
void InputDecimal(int *tgtp, InputNumParam *inpp)
{
	BOOL value_up   = FALSE;
	BOOL value_down = FALSE;
	
	if(inpp->y_offset == 0) {
		inpp->up_count   = D_UP_COUNT_MAX;
		inpp->down_count = D_DOWN_COUNT_MAX;
	}else if(inpp->y_offset < 0) {
		inpp->down_count += inpp->y_offset;
		if(inpp->down_count < D_DOWN_COUNT_MAX) {
			inpp->down_count = 0;
			value_down       = TRUE;
		}
	}else {	// y_offset > 0
		inpp->up_count += inpp->y_offset;
		if(inpp->up_count > D_UP_COUNT_MAX) {
			inpp->up_count = 0;
			value_up       = TRUE;
		}
	}
	
	// キー入力に応じて対象値を増減
	if(	(value_down) || (pad.trg & PAD_KEY_UP)
		|| ((pad.cont & PAD_KEY_UP) && (pad.cont & PAD_BUTTON_R)) ) {
		if(--*tgtp < inpp->value_min) {
			*tgtp = inpp->value_max;
		}
	}else if( (value_up) || (pad.trg & PAD_KEY_DOWN)
			 || ((pad.cont & PAD_KEY_DOWN) && (pad.cont & PAD_BUTTON_R)) ) {
		if(++*tgtp > inpp->value_max) {
			*tgtp = inpp->value_min;
		}
	}
	
	(void)DrawDecimalSJIS( inpp->pos_x, inpp->pos_y, HIGHLIGHT_Y, tgtp, (u8)inpp->keta_max, 4);
																	// 対象値をハイライト表示
}


// RTC設定のクリア
void ClearRTC( void )
{
		SVC_CpuClear(0x0000, &GetSYSMWork()->rtc[0].Time, sizeof(RTCTime), 16);
		GetSYSMWork()->rtc[0].Date.year  = 0;
		GetSYSMWork()->rtc[0].Date.month = 1;
		GetSYSMWork()->rtc[0].Date.day   = 1;
		(void)RTC_SetDateTime( &GetSYSMWork()->rtc[0].Date, &GetSYSMWork()->rtc[0].Time);
		GetNCDWork()->option.input_rtc = 0;
		GetNCDWork()->option.rtcOffset = 0;
		NCD_SetRtcLastSetYear( 0 );
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
}

