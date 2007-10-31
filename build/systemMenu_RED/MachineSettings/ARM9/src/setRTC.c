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
#define RETURN_BUTTON_BOTTOM_X				( (RETURN_BUTTON_TOP_X + 8) * 8 )
#define RETURN_BUTTON_BOTTOM_Y				( (RETURN_BUTTON_TOP_Y + 2) * 8 )

	// ���t�f�[�^LCD�̈�
#define DATE_TOP_X							( 5 * 8 )
#define DATE_TOP_Y							( 10 * 8 )
	// �����f�[�^LCD�̈�
#define TIME_TOP_X							( (DATE_TOP_X + 14) * 8 )
#define TIME_TOP_Y							( DATE_TOP_Y * 8 )

	// RTC�ݒ胁�j���[�v�f
#define RTC_MENU_ELEM_NUM					1

	// �������̓^�b�`�p�l���p�J�E���^
#define S_UPDOWN_COUNT_MAX					16
	// ���l���̓^�b�`�p�l���p�J�E���^
#define D_DOWN_COUNT_MAX					-50
#define D_UP_COUNT_MAX						50


	// ���t�������̓V�[�P���X�p���[�N
typedef struct DateTimeParam {
	int			seq;						// �V�[�P���X�ԍ�
	int			*tgtp;						// ���͑Ώۂ̕ϐ��ւ̃|�C���^
	RTCDate		Date;
	RTCTime		Time;
}DateTimeParam;


	// RTC�ݒ�V�[�P���X�p���[�N
typedef struct SetRtcWork {
	int				csr;					// �J�[�\���ʒu
	s64				rtcOffset[ 2 ];			// RTC�I�t�Z�b�g�l�i[0]:�ݒ�ύX�O�̒l�A[1]:�ύX��̒l�j
	DateTimeParam	dtp;					// ���t�������̓V�[�P���X�p���[�N
	InputNumParam	inp;					// ���l���̓C���^�[�t�F�[�X�p���[�N
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
SetRtcWork *s_pRTCWork;									// RTC�ݒ�p���[�N

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
	PrintfSJIS( DATE_TOP_X + 3 * 8, DATE_TOP_Y, TXT_COLOR_BLACK,  "/   /   [    ]   :   :");
	
	PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L" RETURN " );
	
	if( g_initialSet ) {
		if( GetSYSMWork()->rtcStatus & 0x01) {
			PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"RTC reset is detected!" );
		}else {
			PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Set RTC." );
		}
	}
	
	s_pRTCWork = NNS_FndAllocFromAllocator( &g_allocator, sizeof(SetRtcWork) );	// RTC�ݒ�p���[�N�̊m��
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


// RTC�ݒ�V�[�P���X
int SetRTCMain( void )
{
	BOOL tp_set		= FALSE;
	BOOL tp_return	= FALSE;
	
	ReadTP();													// TP���͂̎擾
	
	if(tpd.disp.touch) {
		tp_set = InRangeTp( DATE_TOP_X,       DATE_TOP_Y,		// [RTC�ݒ�]�̈扟���`�F�b�N
						   ( TIME_TOP_X + 8 * 8 ), (TIME_TOP_Y + 2 * 8 ), &tpd.disp );
																	// [RETURN]�{�^�������`�F�b�N
		tp_return = InRangeTp( RETURN_BUTTON_TOP_X,    RETURN_BUTTON_TOP_Y,
							   RETURN_BUTTON_BOTTOM_X, RETURN_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	if( g_initialSet && !GetNCDWork()->option.input_rtc ) {
		tp_set = TRUE;
	}
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ) {									// �J�[�\���̈ړ�
		if( ++s_pRTCWork->csr == RTC_MENU_ELEM_NUM)	{
			s_pRTCWork->csr = 0;
		}
	}
	if( pad.trg & PAD_KEY_UP ) {
		if( --s_pRTCWork->csr < 0 ) {
			s_pRTCWork->csr = RTC_MENU_ELEM_NUM - 1;
		}
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_set ) {					// RTC�ݒ�J�n
		if( s_pRTCWork->csr == 0 ) {
			InputRtcDateTimeInit( 1 );
			g_pNowProcess = InputRtcDateTimeMain;
		}
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_return ) {			// ���j���[�ɖ߂�
		NNS_FndFreeToAllocator( &g_allocator, s_pRTCWork );			// RTC�ݒ�p���[�N�̉��
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
// ���t���������͏���
//======================================================

// ���t�������͏�����
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


// ���t��������
static int InputRtcDateTimeMain( void )
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	int  new_seq, x_base, y_base, abs_y_offset;
	
	enum {															// ���t�������̓V�[�P���X�ԍ�
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
	
	ReadTP();													// �^�b�`�p�l�����͂̎擾
	CheckOKCancelButton( &tp_ok, &tp_cancel );						// [OK],[CANCEL]�{�^�������`�F�b�N
	
	s_pRTCWork->inp.y_offset = 0;
	
	if( tpd.disp.touch ) {											// [CANCEL]�{�^�������`�F�b�N
		if( ( s_pRTCWork->dtp.seq & 0x01 ) && ( s_pRTCWork->dtp.seq < SEQ_END ) ) {		// SEQ_**_SET�̎��̂ݗL��
			new_seq = s_pRTCWork->dtp.seq;
			x_base  = DATE_TOP_X;
			y_base  = DATE_TOP_Y + 6;
			// ���͍��ڈړ��̃`�F�b�N
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
				// ���͒l�̑���
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
	
	// �^�b�`�p�l�� or �L�[���͂ɂ���āA�J�[�\���ʒu�����������ɁA���̈ʒu�̃J�[�\���������B
	if( ( s_pRTCWork->dtp.seq > 0 ) && ( ( s_pRTCWork->dtp.seq & 0x01 ) == 0 ) ) {		// SEQ_INIT�̎��͎��s���Ȃ�
		PrintfSJIS( s_pRTCWork->inp.pos_x, s_pRTCWork->inp.pos_y, TXT_COLOR_BLACK,
					"%02d", *s_pRTCWork->dtp.tgtp );
	}
	
	// �e�V�[�P���X�̏���
	switch(s_pRTCWork->dtp.seq){
		
	  case SEQ_INIT:
		s_pRTCWork->dtp.Date		= GetSYSMWork()->rtc[0].Date;
		s_pRTCWork->dtp.Time		= GetSYSMWork()->rtc[0].Time;
		s_pRTCWork->dtp.Date.year	+= 2000;									// year���{�Q�O�O�O����B
		s_pRTCWork->dtp.seq			= SEQ_YEAR_INIT;
		// ��SEQ_INIT�͒��ʂ�SEQ_YEAR_INIT��
		
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
																	// �N�E�������Ƃɂ��̌��̓������Z�o����B
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
		
		// �N�������͂Ȃ�΁A�j�����Z�o���ĕ\���B
		if( ( s_pRTCWork->dtp.seq == SEQ_YEAR_SET ) ||
			( s_pRTCWork->dtp.seq == SEQ_MONTH_SET ) ||
			( s_pRTCWork->dtp.seq == SEQ_DAY_SET ) ) {
			s_pRTCWork->dtp.Date.week = CalcWeekFromDate( s_pRTCWork->dtp.Date.year, s_pRTCWork->dtp.Date.month, s_pRTCWork->dtp.Date.day );
			PrintfSJIS( DATE_TOP_X + 10 * 8, DATE_TOP_Y, TXT_COLOR_BLACK, "%s", g_strWeek[ s_pRTCWork->dtp.Date.week ] );
		}
		
		// �N�E�����͂Ȃ�΁A�������Z�o���āA���݂̓��͓��������𒴂��Ă�����C������B
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
		s_pRTCWork->dtp.Date.year -= 2000;									// year���|�Q�O�O�O����B
		s_pRTCWork->dtp.Time.second = 0;
		
		NCD_SetRtcOffset( SYSM_CalcRtcOffsetAndSetDateTime( &s_pRTCWork->dtp.Date, &s_pRTCWork->dtp.Time ) );
		
		GetSYSMWork()->rtc[0].Date = s_pRTCWork->dtp.Date;
		GetSYSMWork()->rtc[0].Time = s_pRTCWork->dtp.Time;
		GetSYSMWork()->ncd_invalid = 0;
		GetNCDWork()->option.input_rtc = 1;						// RTC���̓t���O�𗧂Ă�B
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAM�ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData( GetNCDWork() );
		
		// SEQ_END�̎��͂��̂܂܃��^�[������B
		
	  case SEQ_RETURN:
		g_pNowProcess = SetRTCMain;
		InputRtcDateTimeInit( 0 );								// ���t���͉�ʂ̃N���A
		return 0;
	}
	
	if( s_pRTCWork->dtp.seq & 0x01 ) {										// SEQ_**_SET�̎��̂ݗL��
		if( ( pad.trg & PAD_BUTTON_A ) || tp_ok ) {
			s_pRTCWork->dtp.seq = SEQ_END;									// A�{�^���Ō���
		}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {			// B�{�^���ŃL�����Z��
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
	}else {															// SEQ_**_INIT�̎��̂ݗL��
		s_pRTCWork->dtp.seq++;
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
	
	if((pad.trg & PAD_KEY_DOWN) || (value_down)) {					// �\��������؂�ւ�
		if(++*tgtp>inpp->value_max)	*tgtp = 0;
	}else if((pad.trg & PAD_KEY_UP) || (value_up)) {
		if(--*tgtp & 0x8000)		*tgtp = inpp->value_max;
	}
	
	(void)DrawStringSJIS( inpp->pos_x, inpp->pos_y, HIGHLIGHT_Y, srtpp[*tgtp]);	// ���ݑI�����Ă��镶�����\��
}
*/

// �P�O�i�����l����
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
	
	// �L�[���͂ɉ����đΏےl�𑝌�
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

