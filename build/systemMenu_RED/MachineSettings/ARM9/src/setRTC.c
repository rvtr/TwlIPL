/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setRTC.c

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

//#define __RTC_MINUTE_OFFSET										// この定義が有効な場合はrtcOffsetは分オフセットで算出されます。また、無効な場合は秒オフセットとなります。

	// RETURNボタンLCD領域
#define RETURN_BUTTON_TOP_X					(  2 * 8 )
#define RETURN_BUTTON_TOP_Y					( 21 * 8 )
#define RETURN_BUTTON_BOTTOM_X				( (RETURN_BUTTON_TOP_X + 8) * 8 )
#define RETURN_BUTTON_BOTTOM_Y				( (RETURN_BUTTON_TOP_Y + 2) * 8 )

	// 日付データLCD領域
#define DATE_TOP_X							( 5 * 8 )
#define DATE_TOP_Y							( 10 * 8 )
	// 時刻データLCD領域
#define TIME_TOP_X							( (DATE_TOP_X + 14) * 8 )
#define TIME_TOP_Y							( DATE_TOP_Y * 8 )

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
	s64				rtcOffset[ 2 ];			// RTCオフセット値（[0]:設定変更前の値、[1]:変更後の値）
	DateTimeParam	dtp;					// 日付時刻入力シーケンス用ワーク
	InputNumParam	inp;					// 数値入力インターフェース用ワーク
}SetRtcWork;

// extern data------------------------------------------

// function's prototype declaration---------------------
RTCWeek CalcWeekFromDate( u32 year, u32 month, u32 day );
void InputDecimal(int *tgtp, InputNumParam *inpp);

static void InputRtcDateTimeInit( int start );
static int InputRtcDateTimeMain( void );

static void TransmitRtcData(DateTimeParam *dtpp, RtcDateTime *rtcp);
static void SelectString( int *tgtp, const u8 **const strpp, InputNumParam *inpp);
static void BcdToHex(int *bcdp);
static void HexToBcd(int *hexp);
static BOOL CheckLeapYear( u32 year );

// global variable -------------------------------------

// static variable -------------------------------------
SetRtcWork *s_pRTCWork;									// RTC設定用ワーク

// const data  -----------------------------------------

//======================================================
// 日付＆時刻設定
//======================================================

// RTC設定シーケンスの初期化
void SetRTCInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"DATE & TIME SET" );
	PrintfSJIS( DATE_TOP_X + 3 * 8, DATE_TOP_Y, TXT_COLOR_BLACK,  "/   /   [    ]   :   :");
	
	PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L" RETURN " );
	
	if( g_initialSet ) {
		if( GetSYSMWork()->rtcStatus & 0x01) {
			PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"RTC reset is detected!" );
		}else {
			PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Set RTC." );
		}
	}
	
	s_pRTCWork = NNS_FndAllocFromAllocator( &g_allocator, sizeof(SetRtcWork) );	// RTC設定用ワークの確保
	if( s_pRTCWork == NULL ) {
		OS_Panic( "ARM9- Fail to allocate memory...\n" );
	}
	
	SVC_CpuClear( 0x0000, s_pRTCWork, sizeof(SetRtcWork), 16 );
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// RTC設定シーケンス
int SetRTCMain( void )
{
	BOOL tp_set		= FALSE;
	BOOL tp_return	= FALSE;
	
	ReadTP();													// TP入力の取得
	
	if(tpd.disp.touch) {
		tp_set = InRangeTp( DATE_TOP_X,       DATE_TOP_Y,		// [RTC設定]領域押下チェック
						   ( TIME_TOP_X + 8 * 8 ), (TIME_TOP_Y + 2 * 8 ), &tpd.disp );
																	// [RETURN]ボタン押下チェック
		tp_return = InRangeTp( RETURN_BUTTON_TOP_X,    RETURN_BUTTON_TOP_Y,
							   RETURN_BUTTON_BOTTOM_X, RETURN_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	if( g_initialSet && !GetNCDWork()->option.input_rtc ) {
		tp_set = TRUE;
	}
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ) {									// カーソルの移動
		if( ++s_pRTCWork->csr == RTC_MENU_ELEM_NUM)	{
			s_pRTCWork->csr = 0;
		}
	}
	if( pad.trg & PAD_KEY_UP ) {
		if( --s_pRTCWork->csr < 0 ) {
			s_pRTCWork->csr = RTC_MENU_ELEM_NUM - 1;
		}
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_set ) {					// RTC設定開始
		if( s_pRTCWork->csr == 0 ) {
			InputRtcDateTimeInit( 1 );
			g_pNowProcess = InputRtcDateTimeMain;
		}
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_return ) {			// メニューに戻る
		NNS_FndFreeToAllocator( &g_allocator, s_pRTCWork );			// RTC設定用ワークの解放
		s_pRTCWork = NULL;
		MachineSettingInit();
	}
	
#ifdef __SYSM_DEBUG
	if( pad.trg & PAD_BUTTON_START ) {
		ClearRTC();
		OS_Printf( "RTC offset in NVRAM is ZERO clear!\n" );
	}
#endif /* __SYSM_DEBUG */
	
	return 0;
}


//======================================================
// 日付＆時刻入力処理
//======================================================

// 日付時刻入力初期化
static void InputRtcDateTimeInit( int start )
{
	NNS_G2dCharCanvasClearArea(	&gCanvas, TXT_COLOR_WHITE,
								RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, 28 * 8, 2 * 8 );
	if( start ) {
		DrawOKCancelButton();
		s_pRTCWork->dtp.seq = 0;
	}else {
		PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"RETURN" );
	}
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
}


// 日付時刻入力
static int InputRtcDateTimeMain( void )
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
	
	ReadTP();													// タッチパネル入力の取得
	CheckOKCancelButton( &tp_ok, &tp_cancel );						// [OK],[CANCEL]ボタン押下チェック
	
	s_pRTCWork->inp.y_offset = 0;
	
	if( tpd.disp.touch ) {											// [CANCEL]ボタン押下チェック
		if( ( s_pRTCWork->dtp.seq & 0x01 ) && ( s_pRTCWork->dtp.seq < SEQ_END ) ) {		// SEQ_**_SETの時のみ有効
			new_seq = s_pRTCWork->dtp.seq;
			x_base  = DATE_TOP_X;
			y_base  = DATE_TOP_Y + 6;
			// 入力項目移動のチェック
			if( InRangeTp( x_base, (y_base - 6), (x_base + 22 * 8), (y_base + 6), &tpd.disp ) ) {
				if( tpd.disp.x < x_base + 28 ) {
					new_seq = SEQ_YEAR_SET;
				}else if( ( tpd.disp.x >= x_base +  4 * 8 ) && ( tpd.disp.x < x_base +  6 * 8 ) ) {
					new_seq = SEQ_MONTH_SET;
				}else if( ( tpd.disp.x >= x_base +  7 * 8 ) && ( tpd.disp.x < x_base +  9 * 8 ) ) {
					new_seq = SEQ_DAY_SET;
				}else if( ( tpd.disp.x >= x_base + 14 * 8 ) && ( tpd.disp.x < x_base + 16 * 8 ) ) {
					new_seq = SEQ_HOUR_SET;
				}else if( ( tpd.disp.x >= x_base + 17 * 8 ) && ( tpd.disp.x < x_base + 19 * 8 ) ) {
					new_seq = SEQ_MINUTE_SET;
				}else if( tpd.disp.x >= x_base + 20 * 8 ) {
					new_seq = SEQ_SECOND_SET;
				}
			}
			if( s_pRTCWork->dtp.seq != new_seq ) {
				s_pRTCWork->dtp.seq = new_seq - 1;
			}else {
				// 入力値の増減
				if( InRangeTp( s_pRTCWork->inp.pos_x, (y_base - 30),
							   s_pRTCWork->inp.pos_x + s_pRTCWork->inp.keta_max * 8, y_base + 30, &tpd.disp ) ) {
					s_pRTCWork->inp.y_offset = tpd.disp.y - y_base;
					abs_y_offset = ( s_pRTCWork->inp.y_offset >= 0 ) ? s_pRTCWork->inp.y_offset : -s_pRTCWork->inp.y_offset;
					if( abs_y_offset <= 6 ) {
						s_pRTCWork->inp.y_offset   = 0;
					}else if( abs_y_offset <= 14 ){
						s_pRTCWork->inp.y_offset >>= 2;
					}else if( abs_y_offset <= 22 ){
						s_pRTCWork->inp.y_offset >>= 1;
					}
				}
			}
		}
	}
	
	// タッチパネル or キー入力によって、カーソル位置が動いた時に、元の位置のカーソルを消す。
	if( ( s_pRTCWork->dtp.seq > 0 ) && ( ( s_pRTCWork->dtp.seq & 0x01 ) == 0 ) ) {		// SEQ_INITの時は実行しない
		PrintfSJIS( s_pRTCWork->inp.pos_x, s_pRTCWork->inp.pos_y, TXT_COLOR_BLACK,
					"%02d", *s_pRTCWork->dtp.tgtp );
	}
	
	// 各シーケンスの処理
	switch(s_pRTCWork->dtp.seq){
		
	  case SEQ_INIT:
		s_pRTCWork->dtp.Date		= GetSYSMWork()->rtc[0].Date;
		s_pRTCWork->dtp.Time		= GetSYSMWork()->rtc[0].Time;
		s_pRTCWork->dtp.Date.year	+= 2000;									// yearを＋２０００する。
		s_pRTCWork->dtp.seq			= SEQ_YEAR_INIT;
		// ※SEQ_INITは直通でSEQ_YEAR_INITへ
		
	  case SEQ_YEAR_INIT:
		s_pRTCWork->inp.pos_x		= DATE_TOP_X;
		s_pRTCWork->inp.pos_y		= DATE_TOP_Y;
		s_pRTCWork->inp.keta_max	= 4;
		s_pRTCWork->inp.value_max	= 2099;
		s_pRTCWork->inp.value_min	= 2000;
//		s_pRTCWork->inp.value_min	= 2004;
//		if(s_pRTCWork->dtp.Date.year < 2004) {
//			s_pRTCWork->dtp.Date.year = 2004;
//		}
		s_pRTCWork->dtp.tgtp		= (int *)&s_pRTCWork->dtp.Date.year;
		break;
		
	  case SEQ_MONTH_INIT:
		s_pRTCWork->inp.pos_x		= DATE_TOP_X + 4 * 8;
		s_pRTCWork->inp.keta_max	= 2;
		s_pRTCWork->inp.value_max	= 12;
		s_pRTCWork->inp.value_min	= 1;
		s_pRTCWork->dtp.tgtp		= (int *)&s_pRTCWork->dtp.Date.month;
		break;
		
	  case SEQ_DAY_INIT:
		s_pRTCWork->inp.pos_x		= DATE_TOP_X + 7 * 8;
		s_pRTCWork->inp.keta_max	= 2;
		s_pRTCWork->inp.value_max	= (int)SYSM_GetDayNum( s_pRTCWork->dtp.Date.year, s_pRTCWork->dtp.Date.month );
																	// 年・月をもとにその月の日数を算出する。
		s_pRTCWork->inp.value_min	= 1;
		if(s_pRTCWork->dtp.Date.day > s_pRTCWork->inp.value_max) {
			s_pRTCWork->dtp.Date.day = (u32)s_pRTCWork->inp.value_max;
		}
		s_pRTCWork->dtp.tgtp		= (int *)&s_pRTCWork->dtp.Date.day;
		break;
		
	  case SEQ_HOUR_INIT:
		s_pRTCWork->inp.pos_x		= TIME_TOP_X;
		s_pRTCWork->inp.keta_max	= 2;
		s_pRTCWork->inp.value_max	= 23;
		s_pRTCWork->inp.value_min	= 0;
		s_pRTCWork->dtp.tgtp		= (int *)&s_pRTCWork->dtp.Time.hour;
		break;
		
	  case SEQ_MINUTE_INIT:
		s_pRTCWork->inp.pos_x		= TIME_TOP_X + 3 * 8;
		s_pRTCWork->inp.keta_max	= 2;
		s_pRTCWork->inp.value_max	= 59;
		s_pRTCWork->inp.value_min	= 0;
		s_pRTCWork->dtp.tgtp		= (int *)&s_pRTCWork->dtp.Time.minute;
		break;
		
	  case SEQ_SECOND_INIT:
		s_pRTCWork->inp.pos_x		= TIME_TOP_X + 6 * 8;
		s_pRTCWork->inp.keta_max	= 2;
		s_pRTCWork->inp.value_max	= 59;
		s_pRTCWork->inp.value_min	= 0;
		s_pRTCWork->dtp.tgtp		= (int *)&s_pRTCWork->dtp.Time.second;
		break;
		
	  case SEQ_YEAR_SET:
	  case SEQ_MONTH_SET:
	  case SEQ_DAY_SET:
	  case SEQ_HOUR_SET:
	  case SEQ_MINUTE_SET:
	  case SEQ_SECOND_SET:
		InputDecimal( s_pRTCWork->dtp.tgtp, &s_pRTCWork->inp );
		
		// 年月日入力ならば、曜日を算出して表示。
		if( ( s_pRTCWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pRTCWork->dtp.seq == SEQ_MONTH_SET ) ||
			( s_pRTCWork->dtp.seq == SEQ_DAY_SET ) ) {
			s_pRTCWork->dtp.Date.week = CalcWeekFromDate( s_pRTCWork->dtp.Date.year, s_pRTCWork->dtp.Date.month, s_pRTCWork->dtp.Date.day );
			PrintfSJIS( DATE_TOP_X + 10 * 8, DATE_TOP_Y, TXT_COLOR_BLACK, "%s", g_strWeek[ s_pRTCWork->dtp.Date.week ] );
		}
		
		// 年・月入力ならば、日数を算出して、現在の入力日が日数を超えていたら修正する。
		if( ( s_pRTCWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pRTCWork->dtp.seq == SEQ_MONTH_SET ) ) {
			u32 dayNum = SYSM_GetDayNum( s_pRTCWork->dtp.Date.year, s_pRTCWork->dtp.Date.month );
			if( dayNum < s_pRTCWork->dtp.Date.day ) {
				s_pRTCWork->dtp.Date.day = dayNum;
				PrintfSJIS( DATE_TOP_X + 7 * 8, DATE_TOP_Y, TXT_COLOR_BLACK, "%02d", s_pRTCWork->dtp.Date.day );
			}
		}
		break;
		
	  case SEQ_END:
		s_pRTCWork->dtp.Date.year -= 2000;									// yearを－２０００する。
		s_pRTCWork->dtp.Time.second = 0;
		
		NCD_SetRtcOffset( SYSM_CalcRtcOffsetAndSetDateTime( &s_pRTCWork->dtp.Date, &s_pRTCWork->dtp.Time ) );
		
		GetSYSMWork()->rtc[0].Date = s_pRTCWork->dtp.Date;
		GetSYSMWork()->rtc[0].Time = s_pRTCWork->dtp.Time;
		GetSYSMWork()->ncd_invalid = 0;
		GetNCDWork()->option.input_rtc = 1;						// RTC入力フラグを立てる。
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData( GetNCDWork() );
		
		// SEQ_ENDの時はこのままリターンする。
		
	  case SEQ_RETURN:
		g_pNowProcess = SetRTCMain;
		InputRtcDateTimeInit( 0 );								// 日付入力画面のクリア
		return 0;
	}
	
	if( s_pRTCWork->dtp.seq & 0x01 ) {										// SEQ_**_SETの時のみ有効
		if( ( pad.trg & PAD_BUTTON_A ) || tp_ok ) {
			s_pRTCWork->dtp.seq = SEQ_END;									// Aボタンで決定
		}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {			// Bボタンでキャンセル
			s_pRTCWork->dtp.seq = SEQ_RETURN;
		}else if( pad.trg & PAD_KEY_LEFT ) {
			if( s_pRTCWork->dtp.seq == SEQ_YEAR_SET ) {
				s_pRTCWork->dtp.seq = SEQ_SECOND_INIT;
			}else {
				s_pRTCWork->dtp.seq -= 3;
			}
		}else if( pad.trg & PAD_KEY_RIGHT ) {
			if( s_pRTCWork->dtp.seq == SEQ_SECOND_SET ) {
				s_pRTCWork->dtp.seq = SEQ_YEAR_INIT;
			}else {
				s_pRTCWork->dtp.seq++;
			}
		}
	}else {															// SEQ_**_INITの時のみ有効
		s_pRTCWork->dtp.seq++;
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
	if( month == 1 || month == 2 ){
		year--;
		month += 12;
	}
	return (RTCWeek)( ( year + year/4 - year/100 + year/400 + (13*month + 8)/5 + day) % 7 );
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
void InputDecimal( int *tgtp, InputNumParam *inpp )
{
	BOOL value_up   = FALSE;
	BOOL value_down = FALSE;
	
	if( inpp->y_offset == 0 ) {
		inpp->up_count   = D_UP_COUNT_MAX;
		inpp->down_count = D_DOWN_COUNT_MAX;
	}else if( inpp->y_offset < 0 ) {
		inpp->down_count += inpp->y_offset;
		if( inpp->down_count < D_DOWN_COUNT_MAX ) {
			inpp->down_count = 0;
			value_down       = TRUE;
		}
	}else {	// y_offset > 0
		inpp->up_count += inpp->y_offset;
		if( inpp->up_count > D_UP_COUNT_MAX ) {
			inpp->up_count = 0;
			value_up       = TRUE;
		}
	}
	
	// キー入力に応じて対象値を増減
	if(	value_down ||
		( pad.trg & PAD_KEY_UP ) ||
		( ( pad.cont & PAD_KEY_UP ) && ( pad.cont & PAD_BUTTON_R ) ) ) {
		if( --*tgtp < inpp->value_min ) {
			*tgtp = inpp->value_max;
		}
	}else if( value_up ||
			  ( pad.trg & PAD_KEY_DOWN ) ||
			  ( ( pad.cont & PAD_KEY_DOWN ) && ( pad.cont & PAD_BUTTON_R ) ) ) {
		if( ++*tgtp > inpp->value_max ) {
			*tgtp = inpp->value_min;
		}
	}
	
	PrintfSJIS( inpp->pos_x, inpp->pos_y, TXT_COLOR_GREEN, "%02d", *tgtp );
																	// 対象値をハイライト表示
}


// RTC設定のクリア
void ClearRTC( void )
{
		SVC_CpuClear( 0x0000, &GetSYSMWork()->rtc[0].Time, sizeof(RTCTime), 16 );
		GetSYSMWork()->rtc[0].Date.year  = 0;
		GetSYSMWork()->rtc[0].Date.month = 1;
		GetSYSMWork()->rtc[0].Date.day   = 1;
		(void)RTC_SetDateTime( &GetSYSMWork()->rtc[0].Date, &GetSYSMWork()->rtc[0].Time );
		GetNCDWork()->option.input_rtc = 0;
		GetNCDWork()->option.rtcOffset = 0;
		NCD_SetRtcLastSetYear( 0 );
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData( GetNCDWork() );
}

