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
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 6 * 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 * 8 )

	// 日付データLCD領域
#define DATE_TOP_Y							( 10 * 8 )
#define YEAR_TOP_X							( 5 * 8 )
#define MONTH_TOP_X							( YEAR_TOP_X + 5 * 8 )
#define DAY_TOP_X							( MONTH_TOP_X + 3 * 8 )

	// 時刻データLCD領域
#define TIME_TOP_Y							( DATE_TOP_Y )
#define HOUR_TOP_X							( DAY_TOP_X + 5 * 8 )
#define MINUTE_TOP_X						( HOUR_TOP_X + 3 * 8 )
#define SECOND_TOP_X						( MINUTE_TOP_X + 3 * 8 )

	// 文字入力タッチパネル用カウンタ
#define S_UPDOWN_COUNT_MAX					16
	// 数値入力タッチパネル用カウンタ
#define D_DOWN_COUNT_MAX					-50
#define D_UP_COUNT_MAX						50


	// 日付時刻入力シーケンス用ワーク
typedef struct DateTimeParam {
	int			seq;						// シーケンス番号
	int			*pTgt;						// 入力対象の変数へのポインタ
	RTCDate		Date;
	RTCTime		Time;
	RTCDate		Date_old;
	RTCTime		Time_old;
}DateTimeParam;


	// RTC設定シーケンス用ワーク
typedef struct SetRtcWork {
	int				csr;					// カーソル位置
	s64				rtcOffset[ 2 ];			// RTCオフセット値（[0]:設定変更前の値、[1]:変更後の値）
	DateTimeParam	dtp;					// 日付時刻入力シーケンス用ワーク
	InputNumParam	inp;					// 数値入力インターフェース用ワーク
}SetRtcWork;


typedef struct DateTimeWidth {
	int		year;
	int		month;
	int		day;
	int		hour;
	int		minute;
	int		second;
}DateTimeWidth;

// extern data------------------------------------------

// function's prototype declaration---------------------
RTCWeek CalcWeekFromDate( u32 year, u32 month, u32 day );
void InputDecimal(int *pTgt, InputNumParam *inpp);

static void InputRtcDateTimeInit( int start );
static int InputRtcDateTimeMain( void );

static void SelectString( int *pTgt, const u8 **const pStrp, InputNumParam *inpp);
static void BcdToHex(int *bcdp);
static void HexToBcd(int *hexp);
static BOOL CheckLeapYear( u32 year );
static void DrawDateTime( RTCDate *pDate, RTCTime *pTime, int color );

// global variable -------------------------------------

// static variable -------------------------------------
static SetRtcWork *s_pWork;									// RTC設定用ワーク
// const data  -----------------------------------------

//======================================================
// 日付＆時刻設定
//======================================================

// RTC設定シーケンスの初期化
void SetRTCInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"DATE & TIME SET" );
	PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L" RETURN " );
	
	s_pWork = Alloc( sizeof(SetRtcWork) );	// RTC設定用ワークの確保
	if( s_pWork == NULL ) {
		OS_Panic( "ARM9- Fail to allocate memory...\n" );
	}
	
	SVC_CpuClear( 0x0000, s_pWork, sizeof(SetRtcWork), 16 );
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	RTC_GetDateTime( &s_pWork->dtp.Date, &s_pWork->dtp.Time );
	s_pWork->dtp.Date.year += 2000;
	
	PrintfSJIS    ( YEAR_TOP_X,   DATE_TOP_Y, TXT_COLOR_BLACK, "%04d", s_pWork->dtp.Date.year );
	PrintfSJIS    ( MONTH_TOP_X,  DATE_TOP_Y, TXT_COLOR_BLACK, "%02d", s_pWork->dtp.Date.month );
	PrintfSJIS    ( DAY_TOP_X,    DATE_TOP_Y, TXT_COLOR_BLACK, "%02d", s_pWork->dtp.Date.day );
	PrintfSJIS    ( HOUR_TOP_X,   TIME_TOP_Y, TXT_COLOR_BLACK, "%02d", s_pWork->dtp.Time.hour );
	PrintfSJIS    ( MINUTE_TOP_X, TIME_TOP_Y, TXT_COLOR_BLACK, "%02d", s_pWork->dtp.Time.minute );
	
	DrawDateTime( &s_pWork->dtp.Date, &s_pWork->dtp.Time, TXT_COLOR_BLACK );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// 指定した日付・時刻の表示
static void DrawDateTime( RTCDate *pDate, RTCTime *pTime, int color )
{
	PrintfSJIS( YEAR_TOP_X,   DATE_TOP_Y, color, "%04d", pDate->year );
	PrintfSJIS( MONTH_TOP_X,  DATE_TOP_Y, color, "%02d", pDate->month );
	PrintfSJIS( DAY_TOP_X,    DATE_TOP_Y, color, "%02d", pDate->day );
	PrintfSJIS( HOUR_TOP_X,   TIME_TOP_Y, color, "%02d", pTime->hour );
	PrintfSJIS( MINUTE_TOP_X, TIME_TOP_Y, color, "%02d", pTime->minute );
	PutStringUTF16( YEAR_TOP_X  + 4 * 8, DATE_TOP_Y, color, L"/" );
	PutStringUTF16( MONTH_TOP_X + 2 * 8, DATE_TOP_Y, color, L"/" );
	PutStringUTF16( HOUR_TOP_X  + 2 * 8, TIME_TOP_Y, color, L":" );
}


// RTC設定シーケンス
int SetRTCMain( void )
{
	BOOL tp_set		= FALSE;
	BOOL tp_return	= FALSE;
	
	ReadTP();													// TP入力の取得
	
	if(tpd.disp.touch) {
		tp_set    = WithinRangeTP(	YEAR_TOP_X,   DATE_TOP_Y,	// [RTC設定]領域押下チェック
									SECOND_TOP_X, DATE_TOP_Y + 16, &tpd.disp );
																// [RETURN]ボタン押下チェック
		tp_return = WithinRangeTP(	RETURN_BUTTON_TOP_X,    RETURN_BUTTON_TOP_Y,
									RETURN_BUTTON_BOTTOM_X, RETURN_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( ( pad.trg & PAD_BUTTON_A ) || tp_set ) {					// RTC設定開始
		InputRtcDateTimeInit( 1 );
		g_pNowProcess = InputRtcDateTimeMain;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_return ) {			// メニューに戻る
		Free( s_pWork );			// RTC設定用ワークの解放
		s_pWork = NULL;
		MachineSettingInit();
	}
#ifdef SYSM_DEBUG_
	else if( pad.trg & PAD_BUTTON_START ) {
		ClearRTC();
		OS_Printf( "RTC offset in NVRAM is ZERO clear!\n" );
	}
#endif /* SYSM_DEBUG_ */
	
	return 0;
}


//======================================================
// 日付＆時刻入力処理
//======================================================

// 日付時刻入力初期化
static void InputRtcDateTimeInit( int start )
{
	NNS_G2dCharCanvasClearArea(	&gCanvas, TXT_COLOR_WHITE,
								RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, 256 - RETURN_BUTTON_TOP_X, 2 * 8 );
	if( start ) {
		DrawOKCancelButton();
		s_pWork->dtp.seq = 0;
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
	int  new_seq, y_base, abs_y_offset;
	
	enum {															// 日付時刻入力シーケンス番号
		SEQ_INIT=0,
		SEQ_YEAR_INIT=2, SEQ_YEAR_SET,
		SEQ_MONTH_INIT,  SEQ_MONTH_SET,
		SEQ_DAY_INIT,    SEQ_DAY_SET,
		SEQ_HOUR_INIT,   SEQ_HOUR_SET,
		SEQ_MINUTE_INIT, SEQ_MINUTE_SET,
		SEQ_END,
		SEQ_CANCEL=64
	};
	
	ReadTP();													// タッチパネル入力の取得
	CheckOKCancelButton( &tp_ok, &tp_cancel );						// [OK],[CANCEL]ボタン押下チェック
	
	s_pWork->inp.y_offset = 0;
	
	if( tpd.disp.touch ) {											// [CANCEL]ボタン押下チェック
		if( ( s_pWork->dtp.seq & 0x01 ) && ( s_pWork->dtp.seq < SEQ_END ) ) {		// SEQ_**_SETの時のみ有効
			new_seq = s_pWork->dtp.seq;
			y_base  = DATE_TOP_Y + 6;
			
			// 入力項目移動のチェック
			if( WithinRangeTP( YEAR_TOP_X, DATE_TOP_Y, SECOND_TOP_X, DATE_TOP_Y + 12, &tpd.disp ) ) {
				if( tpd.disp.x < YEAR_TOP_X + 32 ) {
					new_seq = SEQ_YEAR_SET;
				}else if( ( tpd.disp.x >= MONTH_TOP_X )  && ( tpd.disp.x < MONTH_TOP_X + 16 ) ) {
					new_seq = SEQ_MONTH_SET;
				}else if( ( tpd.disp.x >= DAY_TOP_X )    && ( tpd.disp.x < DAY_TOP_X + 16 ) ) {
					new_seq = SEQ_DAY_SET;
				}else if( ( tpd.disp.x >= HOUR_TOP_X )   && ( tpd.disp.x < HOUR_TOP_X + 16 ) ) {
					new_seq = SEQ_HOUR_SET;
				}else if( ( tpd.disp.x >= MINUTE_TOP_X ) && ( tpd.disp.x < MINUTE_TOP_X + 16 ) ) {
					new_seq = SEQ_MINUTE_SET;
				}
			}
			if( s_pWork->dtp.seq != new_seq ) {
				s_pWork->dtp.seq = new_seq - 1;
			}else if( WithinRangeTP( s_pWork->inp.pos_x, y_base - 40,
									 s_pWork->inp.pos_x + s_pWork->inp.keta_max * 8, y_base + 60, &tpd.disp ) ) {
				// 入力値の増減
				s_pWork->inp.y_offset = tpd.disp.y - y_base;
				abs_y_offset = ( s_pWork->inp.y_offset >= 0 ) ? s_pWork->inp.y_offset : -s_pWork->inp.y_offset;
				if( abs_y_offset <= 6 ) {
					s_pWork->inp.y_offset = 0;
				}else if( abs_y_offset <= 20 ){
					s_pWork->inp.y_offset >>= 2;
				}else if( abs_y_offset <= 40 ){
					s_pWork->inp.y_offset >>= 1;
				}
			}
		}
	}
	
	// タッチパネル or キー入力によって、カーソル位置が動いた時に、元の位置のカーソルを消す。
	if( ( s_pWork->dtp.seq > 0 ) && ( ( s_pWork->dtp.seq & 0x01 ) == 0 ) ) {		// SEQ_INITの時は実行しない
		PrintfSJIS( s_pWork->inp.pos_x, s_pWork->inp.pos_y, TXT_COLOR_BLACK, "%02d", *s_pWork->dtp.pTgt );
	}
	
	// 各シーケンスの処理
	switch(s_pWork->dtp.seq){
		
	  case SEQ_INIT:
		MI_CpuCopy32( &s_pWork->dtp.Date, &s_pWork->dtp.Date_old, sizeof(RTCDate) );
		MI_CpuCopy32( &s_pWork->dtp.Time, &s_pWork->dtp.Time_old, sizeof(RTCTime) );
		
		s_pWork->dtp.seq		= SEQ_YEAR_INIT;
		// ※SEQ_INITは直通でSEQ_YEAR_INITへ
		
	  case SEQ_YEAR_INIT:
		s_pWork->inp.pos_x		= YEAR_TOP_X;
		s_pWork->inp.pos_y		= DATE_TOP_Y;
		s_pWork->inp.keta_max	= 4;
		s_pWork->inp.value_max	= 2099;
		s_pWork->inp.value_min	= 2000;
//		s_pWork->inp.value_min	= 2004;
//		if(s_pWork->dtp.Date.year < 2004) {
//			s_pWork->dtp.Date.year = 2004;
//		}
		s_pWork->dtp.pTgt		= (int *)&s_pWork->dtp.Date.year;
		break;
		
	  case SEQ_MONTH_INIT:
		s_pWork->inp.pos_x		= MONTH_TOP_X;
		s_pWork->inp.keta_max	= 2;
		s_pWork->inp.value_max	= 12;
		s_pWork->inp.value_min	= 1;
		s_pWork->dtp.pTgt		= (int *)&s_pWork->dtp.Date.month;
		break;
		
	  case SEQ_DAY_INIT:
		s_pWork->inp.pos_x		= DAY_TOP_X;
		s_pWork->inp.keta_max	= 2;
		s_pWork->inp.value_max	= (int)UTL_GetDayNum( s_pWork->dtp.Date.year, s_pWork->dtp.Date.month );
																	// 年・月をもとにその月の日数を算出する。
		s_pWork->inp.value_min	= 1;
		if(s_pWork->dtp.Date.day > s_pWork->inp.value_max) {
			s_pWork->dtp.Date.day = (u32)s_pWork->inp.value_max;
		}
		s_pWork->dtp.pTgt		= (int *)&s_pWork->dtp.Date.day;
		break;
		
	  case SEQ_HOUR_INIT:
		s_pWork->inp.pos_x		= HOUR_TOP_X;
		s_pWork->inp.keta_max	= 2;
		s_pWork->inp.value_max	= 23;
		s_pWork->inp.value_min	= 0;
		s_pWork->dtp.pTgt		= (int *)&s_pWork->dtp.Time.hour;
		break;
		
	  case SEQ_MINUTE_INIT:
		s_pWork->inp.pos_x		= MINUTE_TOP_X;
		s_pWork->inp.keta_max	= 2;
		s_pWork->inp.value_max	= 59;
		s_pWork->inp.value_min	= 0;
		s_pWork->dtp.pTgt		= (int *)&s_pWork->dtp.Time.minute;
		break;
		
	  case SEQ_YEAR_SET:
	  case SEQ_MONTH_SET:
	  case SEQ_DAY_SET:
	  case SEQ_HOUR_SET:
	  case SEQ_MINUTE_SET:
		InputDecimal( s_pWork->dtp.pTgt, &s_pWork->inp );
		
		// 年月日入力ならば、曜日を再計算
		if( ( s_pWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pWork->dtp.seq == SEQ_MONTH_SET ) ||
			( s_pWork->dtp.seq == SEQ_DAY_SET ) ) {
			s_pWork->dtp.Date.week = CalcWeekFromDate( s_pWork->dtp.Date.year, s_pWork->dtp.Date.month, s_pWork->dtp.Date.day );
		}
		
		// 年・月入力ならば、日数を算出して、現在の入力日が日数を超えていたら修正する。
		if( ( s_pWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pWork->dtp.seq == SEQ_MONTH_SET ) ) {
			u32 dayNum = UTL_GetDayNum( s_pWork->dtp.Date.year, s_pWork->dtp.Date.month );
			if( dayNum < s_pWork->dtp.Date.day ) {
				PrintfSJIS( DAY_TOP_X, DATE_TOP_Y, TXT_COLOR_WHITE, "%02d", s_pWork->dtp.Date.day );
				PrintfSJIS( DAY_TOP_X, DATE_TOP_Y, TXT_COLOR_BLACK, "%02d", dayNum );
				s_pWork->dtp.Date.day = dayNum;
			}
		}
		break;
		
	  case SEQ_END:
		s_pWork->dtp.Time.second = 0;
		{
			RTCDate date;			// +-2000を処理するのが面倒なので、別バッファにコピーして処理
			MI_CpuCopy32( &s_pWork->dtp.Date, &date, sizeof(RTCDate) );
			date.year -= 2000;
			// オフセット計算→日付のセットの順で実行する。
			LCFG_TSD_SetRTCOffset( UTL_CalcRTCOffset( &date, &s_pWork->dtp.Time ) );
			(void)RTC_SetDateTime( &date, &s_pWork->dtp.Time );
		}
		
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL設定データファイルへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		if( !MY_WriteTWLSettings() ) {
			OS_TPrintf( "TWL settings write failed.\n" );
		}
		
		// 上画面の表示更新
		GetAndDrawRTCData( &g_rtcDraw, TRUE );
		
		g_pNowProcess = SetRTCMain;
		InputRtcDateTimeInit( 0 );								// 日付入力画面のクリア
		return 0;
		
	  case SEQ_CANCEL:
		// 日付・時刻表示を元の値に戻して表示更新
		DrawDateTime( &s_pWork->dtp.Date,     &s_pWork->dtp.Time,     TXT_COLOR_WHITE );
		DrawDateTime( &s_pWork->dtp.Date_old, &s_pWork->dtp.Time_old, TXT_COLOR_BLACK );
		MI_CpuCopy32( &s_pWork->dtp.Date_old, &s_pWork->dtp.Date, sizeof(RTCDate) );
		MI_CpuCopy32( &s_pWork->dtp.Time_old, &s_pWork->dtp.Time, sizeof(RTCTime) );
		g_pNowProcess = SetRTCMain;
		InputRtcDateTimeInit( 0 );								// 日付入力画面のクリア
		return 0;
	}
	
	if( s_pWork->dtp.seq & 0x01 ) {										// SEQ_**_SETの時のみ有効
		if( ( pad.trg & PAD_BUTTON_A ) || tp_ok ) {
			s_pWork->dtp.seq = SEQ_END;									// Aボタンで決定
		}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {			// Bボタンでキャンセル
			s_pWork->dtp.seq = SEQ_CANCEL;
		}else if( pad.trg & PAD_KEY_LEFT ) {
			if( s_pWork->dtp.seq == SEQ_YEAR_SET ) {
				s_pWork->dtp.seq = SEQ_MINUTE_INIT;
			}else {
				s_pWork->dtp.seq -= 3;
			}
		}else if( pad.trg & PAD_KEY_RIGHT ) {
			if( s_pWork->dtp.seq == SEQ_MINUTE_SET ) {
				s_pWork->dtp.seq = SEQ_YEAR_INIT;
			}else {
				s_pWork->dtp.seq++;
			}
		}
	}else {															// SEQ_**_INITの時のみ有効
		s_pWork->dtp.seq++;
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
static void SelectString(int *pTgt, const u8 **const ppStr, InputNumParam *pInp )
{
	BOOL value_up   = FALSE;
	BOOL value_down = FALSE;
	const u8 *pStr_old = *pTgt;
	
	if( pInp->y_offset == 0 ) {
		pInp->up_count = S_UPDOWN_COUNT_MAX;
	}else {
		pInp->up_count ++;
		if( pInp->up_count > S_UPDOWN_COUNT_MAX ) {
			pInp->up_count = 0;
			if( pInp->y_offset < 0 ) value_up   = TRUE;
			else                     value_down = TRUE;
		}
	}
	
	if( ( pad.trg & PAD_KEY_DOWN ) || value_down ) {					// 表示文字列切り替え
		if( ++ *pTgt > pInp->value_max ) {
			*pTgt = 0;
		}
	}else if( ( pad.trg & PAD_KEY_UP ) || value_up ) {
		if( -- *pTgt & 0x8000 )	{
 			*pTgt = pInp->value_max;
		}
	}
	
	PutStringUTF16( pInp->pos_x, pInp->pos_y, TXT_COLOR_WHITE, pStr_old );			// 旧文字列を消去
	PutStringUTF16( pInp->pos_x, pInp->pos_y, TXT_COLOR_BLACK, ppStr[ *pTgt ] );	// 現在選択している文字列を表示
}
*/

// １０進数数値入力
void InputDecimal( int *pTgt, InputNumParam *pInp )
{
	BOOL value_up   = FALSE;
	BOOL value_down = FALSE;
	int old = *pTgt;
	
	if( pInp->y_offset == 0 ) {
		pInp->up_count   = D_UP_COUNT_MAX;
		pInp->down_count = D_DOWN_COUNT_MAX;
	}else if( pInp->y_offset < 0 ) {
		pInp->down_count += pInp->y_offset;
		if( pInp->down_count < D_DOWN_COUNT_MAX ) {
			pInp->down_count = 0;
			value_down       = TRUE;
		}
	}else {	// y_offset > 0
		pInp->up_count += pInp->y_offset;
		if( pInp->up_count > D_UP_COUNT_MAX ) {
			pInp->up_count = 0;
			value_up       = TRUE;
		}
	}
	
	// キー入力に応じて対象値を増減
	if(	value_down ||
		( pad.trg & PAD_KEY_UP ) ||
		( ( pad.cont & PAD_KEY_UP ) && ( pad.cont & PAD_BUTTON_R ) ) ) {
		if( --*pTgt < pInp->value_min ) {
			*pTgt = pInp->value_max;
		}
	}else if( value_up ||
			  ( pad.trg & PAD_KEY_DOWN ) ||
			  ( ( pad.cont & PAD_KEY_DOWN ) && ( pad.cont & PAD_BUTTON_R ) ) ) {
		if( ++*pTgt > pInp->value_max ) {
			*pTgt = pInp->value_min;
		}
	}
	
	PrintfSJIS( pInp->pos_x, pInp->pos_y, TXT_COLOR_WHITE, "%02d", old );
	PrintfSJIS( pInp->pos_x, pInp->pos_y, TXT_COLOR_GREEN, "%02d", *pTgt );
																	// 対象値をハイライト表示
}


// RTC設定のクリア
void ClearRTC( void )
{
	RTCDate date = { 0, 1, 1, RTC_WEEK_SUNDAY };
	RTCTime time = { 0, 0, 0 };
	(void)RTC_SetDateTime( &date, &time );
	LCFG_TSD_SetRTCOffset( 0 );
	LCFG_TSD_SetRTCLastSetYear( 0 );
	// ::::::::::::::::::::::::::::::::::::::::::::::
	// TWL設定データファイルへの書き込み
	// ::::::::::::::::::::::::::::::::::::::::::::::
	if( !MY_WriteTWLSettings() ) {
		OS_TPrintf( "TWL settings write failed.\n" );
	}
}
