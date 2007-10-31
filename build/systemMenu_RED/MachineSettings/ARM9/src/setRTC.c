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

//#define __RTC_MINUTE_OFFSET										// ���̒�`���L���ȏꍇ��rtcOffset�͕��I�t�Z�b�g�ŎZ�o����܂��B�܂��A�����ȏꍇ�͕b�I�t�Z�b�g�ƂȂ�܂��B

	// RETURN�{�^��LCD�̈�
#define RETURN_BUTTON_TOP_X					(  2 * 8 )
#define RETURN_BUTTON_TOP_Y					( 21 * 8 )
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 6 * 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 * 8 )

	// ���t�f�[�^LCD�̈�
#define DATE_TOP_Y							( 10 * 8 )
#define YEAR_TOP_X							( 5 * 8 )
#define MONTH_TOP_X							( YEAR_TOP_X + 5 * 8 )
#define DAY_TOP_X							( MONTH_TOP_X + 3 * 8 )

	// �����f�[�^LCD�̈�
#define TIME_TOP_Y							( DATE_TOP_Y )
#define HOUR_TOP_X							( DAY_TOP_X + 5 * 8 )
#define MINUTE_TOP_X						( HOUR_TOP_X + 3 * 8 )
#define SECOND_TOP_X						( MINUTE_TOP_X + 3 * 8 )

	// �������̓^�b�`�p�l���p�J�E���^
#define S_UPDOWN_COUNT_MAX					16
	// ���l���̓^�b�`�p�l���p�J�E���^
#define D_DOWN_COUNT_MAX					-50
#define D_UP_COUNT_MAX						50


	// ���t�������̓V�[�P���X�p���[�N
typedef struct DateTimeParam {
	int			seq;						// �V�[�P���X�ԍ�
	int			*pTgt;						// ���͑Ώۂ̕ϐ��ւ̃|�C���^
	RTCDate		Date;
	RTCTime		Time;
	RTCDate		Date_old;
	RTCTime		Time_old;
}DateTimeParam;


	// RTC�ݒ�V�[�P���X�p���[�N
typedef struct SetRtcWork {
	int				csr;					// �J�[�\���ʒu
	s64				rtcOffset[ 2 ];			// RTC�I�t�Z�b�g�l�i[0]:�ݒ�ύX�O�̒l�A[1]:�ύX��̒l�j
	DateTimeParam	dtp;					// ���t�������̓V�[�P���X�p���[�N
	InputNumParam	inp;					// ���l���̓C���^�[�t�F�[�X�p���[�N
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

static void TransmitRtcData(DateTimeParam *dtpp, RtcDateTime *rtcp);
static void SelectString( int *pTgt, const u8 **const pStrp, InputNumParam *inpp);
static void BcdToHex(int *bcdp);
static void HexToBcd(int *hexp);
static BOOL CheckLeapYear( u32 year );
static void DrawDateTime( RTCDate *pDate, RTCTime *pTime, int color );

// global variable -------------------------------------

// static variable -------------------------------------
static SetRtcWork *s_pWork;									// RTC�ݒ�p���[�N
// const data  -----------------------------------------

//======================================================
// ���t�������ݒ�
//======================================================

// RTC�ݒ�V�[�P���X�̏�����
void SetRTCInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"DATE & TIME SET" );
	PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L" RETURN " );
	
	if( g_initialSet ) {
		if( GetSYSMWork()->rtcStatus & 0x01) {
			PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"RTC reset is detected!" );
		}else {
			PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Set RTC." );
		}
	}
	
	s_pWork = NNS_FndAllocFromAllocator( &g_allocator, sizeof(SetRtcWork) );	// RTC�ݒ�p���[�N�̊m��
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
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// �w�肵�����t�E�����̕\��
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


// RTC�ݒ�V�[�P���X
int SetRTCMain( void )
{
	BOOL tp_set		= FALSE;
	BOOL tp_return	= FALSE;
	
	ReadTP();													// TP���͂̎擾
	
	if(tpd.disp.touch) {
		tp_set    = WithinRangeTP(	YEAR_TOP_X,   DATE_TOP_Y,	// [RTC�ݒ�]�̈扟���`�F�b�N
									SECOND_TOP_X, DATE_TOP_Y + 16, &tpd.disp );
																// [RETURN]�{�^�������`�F�b�N
		tp_return = WithinRangeTP(	RETURN_BUTTON_TOP_X,    RETURN_BUTTON_TOP_Y,
									RETURN_BUTTON_BOTTOM_X, RETURN_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	if( g_initialSet && !GetNCDWork()->option.input_rtc ) {
		tp_set = TRUE;
	}
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( ( pad.trg & PAD_BUTTON_A ) || tp_set ) {					// RTC�ݒ�J�n
		InputRtcDateTimeInit( 1 );
		g_pNowProcess = InputRtcDateTimeMain;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_return ) {			// ���j���[�ɖ߂�
		NNS_FndFreeToAllocator( &g_allocator, s_pWork );			// RTC�ݒ�p���[�N�̉��
		s_pWork = NULL;
		MachineSettingInit();
	}
#ifdef __SYSM_DEBUG
	else if( pad.trg & PAD_BUTTON_START ) {
		ClearRTC();
		OS_Printf( "RTC offset in NVRAM is ZERO clear!\n" );
	}
#endif /* __SYSM_DEBUG */
	
	return 0;
}


//======================================================
// ���t���������͏���
//======================================================

// ���t�������͏�����
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


// ���t��������
static int InputRtcDateTimeMain( void )
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	int  new_seq, y_base, abs_y_offset;
	
	enum {															// ���t�������̓V�[�P���X�ԍ�
		SEQ_INIT=0,
		SEQ_YEAR_INIT=2, SEQ_YEAR_SET,
		SEQ_MONTH_INIT,  SEQ_MONTH_SET,
		SEQ_DAY_INIT,    SEQ_DAY_SET,
		SEQ_HOUR_INIT,   SEQ_HOUR_SET,
		SEQ_MINUTE_INIT, SEQ_MINUTE_SET,
		SEQ_END,
		SEQ_CANCEL=64
	};
	
	ReadTP();													// �^�b�`�p�l�����͂̎擾
	CheckOKCancelButton( &tp_ok, &tp_cancel );						// [OK],[CANCEL]�{�^�������`�F�b�N
	
	s_pWork->inp.y_offset = 0;
	
	if( tpd.disp.touch ) {											// [CANCEL]�{�^�������`�F�b�N
		if( ( s_pWork->dtp.seq & 0x01 ) && ( s_pWork->dtp.seq < SEQ_END ) ) {		// SEQ_**_SET�̎��̂ݗL��
			new_seq = s_pWork->dtp.seq;
			y_base  = DATE_TOP_Y + 6;
			
			// ���͍��ڈړ��̃`�F�b�N
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
				// ���͒l�̑���
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
	
	// �^�b�`�p�l�� or �L�[���͂ɂ���āA�J�[�\���ʒu�����������ɁA���̈ʒu�̃J�[�\���������B
	if( ( s_pWork->dtp.seq > 0 ) && ( ( s_pWork->dtp.seq & 0x01 ) == 0 ) ) {		// SEQ_INIT�̎��͎��s���Ȃ�
		PrintfSJIS( s_pWork->inp.pos_x, s_pWork->inp.pos_y, TXT_COLOR_BLACK, "%02d", *s_pWork->dtp.pTgt );
	}
	
	// �e�V�[�P���X�̏���
	switch(s_pWork->dtp.seq){
		
	  case SEQ_INIT:
		MI_CpuCopy32( &s_pWork->dtp.Date, &s_pWork->dtp.Date_old, sizeof(RTCDate) );
		MI_CpuCopy32( &s_pWork->dtp.Time, &s_pWork->dtp.Time_old, sizeof(RTCTime) );
		
		s_pWork->dtp.seq		= SEQ_YEAR_INIT;
		// ��SEQ_INIT�͒��ʂ�SEQ_YEAR_INIT��
		
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
		s_pWork->inp.value_max	= (int)SYSM_GetDayNum( s_pWork->dtp.Date.year, s_pWork->dtp.Date.month );
																	// �N�E�������Ƃɂ��̌��̓������Z�o����B
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
		
		// �N�������͂Ȃ�΁A�j�����Čv�Z
		if( ( s_pWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pWork->dtp.seq == SEQ_MONTH_SET ) ||
			( s_pWork->dtp.seq == SEQ_DAY_SET ) ) {
			s_pWork->dtp.Date.week = CalcWeekFromDate( s_pWork->dtp.Date.year, s_pWork->dtp.Date.month, s_pWork->dtp.Date.day );
		}
		
		// �N�E�����͂Ȃ�΁A�������Z�o���āA���݂̓��͓��������𒴂��Ă�����C������B
		if( ( s_pWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pWork->dtp.seq == SEQ_MONTH_SET ) ) {
			u32 dayNum = SYSM_GetDayNum( s_pWork->dtp.Date.year, s_pWork->dtp.Date.month );
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
			RTCDate date;			// +-2000����������̂��ʓ|�Ȃ̂ŁA�ʃo�b�t�@�ɃR�s�[���ď���
			MI_CpuCopy32( &s_pWork->dtp.Date, &date, sizeof(RTCDate) );
			date.year -= 2000;
			(void)RTC_SetDateTime( &date, &s_pWork->dtp.Time );
			NCD_SetRtcOffset( SYSM_CalcRtcOffsetAndSetDateTime( &date, &s_pWork->dtp.Time ) );
		}
		
		GetSYSMWork()->ncd_invalid = 0;
		GetNCDWork()->option.input_rtc = 1;						// RTC���̓t���O�𗧂Ă�B
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAM�ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData( GetNCDWork() );
		
		// ���ʂ̕\���X�V
		GetAndDrawRTCData( &g_rtcDraw, TRUE );
		
		g_pNowProcess = SetRTCMain;
		InputRtcDateTimeInit( 0 );								// ���t���͉�ʂ̃N���A
		return 0;
		
	  case SEQ_CANCEL:
		// ���t�E�����\�������̒l�ɖ߂��ĕ\���X�V
		DrawDateTime( &s_pWork->dtp.Date,     &s_pWork->dtp.Time,     TXT_COLOR_WHITE );
		DrawDateTime( &s_pWork->dtp.Date_old, &s_pWork->dtp.Time_old, TXT_COLOR_BLACK );
		MI_CpuCopy32( &s_pWork->dtp.Date_old, &s_pWork->dtp.Date, sizeof(RTCDate) );
		MI_CpuCopy32( &s_pWork->dtp.Time_old, &s_pWork->dtp.Time, sizeof(RTCTime) );
		g_pNowProcess = SetRTCMain;
		InputRtcDateTimeInit( 0 );								// ���t���͉�ʂ̃N���A
		return 0;
	}
	
	if( s_pWork->dtp.seq & 0x01 ) {										// SEQ_**_SET�̎��̂ݗL��
		if( ( pad.trg & PAD_BUTTON_A ) || tp_ok ) {
			s_pWork->dtp.seq = SEQ_END;									// A�{�^���Ō���
		}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {			// B�{�^���ŃL�����Z��
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
	}else {															// SEQ_**_INIT�̎��̂ݗL��
		s_pWork->dtp.seq++;
	}
	return 0;
}


/*
// ���邤�N�̔��� (���邤�N�F�P�A�ʏ�̔N�F�O���^�[���j
BOOL CheckLeapYear( u32 year)
{
	if((year & 0x03) == 0) {										// ���邤�N�́A�u4�Ŋ���؂�@���@100�Ŋ���؂�Ȃ��N�v
		CP_SetDiv32_32(year, 100);									//             �u400�Ŋ���؂��N�v
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

// ���t����j�������߂�B
RTCWeek CalcWeekFromDate( u32 year, u32 month, u32 day )
{
	if( month == 1 || month == 2 ){
		year--;
		month += 12;
	}
	return (RTCWeek)( ( year + year/4 - year/100 + year/400 + (13*month + 8)/5 + day) % 7 );
}


/*
// ������ɂ��p�����[�^�I��
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
	
	if( ( pad.trg & PAD_KEY_DOWN ) || value_down ) {					// �\��������؂�ւ�
		if( ++ *pTgt > pInp->value_max ) {
			*pTgt = 0;
		}
	}else if( ( pad.trg & PAD_KEY_UP ) || value_up ) {
		if( -- *pTgt & 0x8000 )	{
 			*pTgt = pInp->value_max;
		}
	}
	
	PutStringUTF16( pInp->pos_x, pInp->pos_y, TXT_COLOR_WHITE, pStr_old );			// �������������
	PutStringUTF16( pInp->pos_x, pInp->pos_y, TXT_COLOR_BLACK, ppStr[ *pTgt ] );	// ���ݑI�����Ă��镶�����\��
}
*/

// �P�O�i�����l����
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
	
	// �L�[���͂ɉ����đΏےl�𑝌�
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
																	// �Ώےl���n�C���C�g�\��
}


// RTC�ݒ�̃N���A
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
		// NVRAM�ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData( GetNCDWork() );
}
