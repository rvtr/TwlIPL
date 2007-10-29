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

//#define __RTC_MINUTE_OFFSET										// ���̒�`���L���ȏꍇ��rtcOffset�͕��I�t�Z�b�g�ŎZ�o����܂��B�܂��A�����ȏꍇ�͕b�I�t�Z�b�g�ƂȂ�܂��B

	// RETURN�{�^��LCD�̈�
#define RETURN_BUTTON_LT_X					2
#define RETURN_BUTTON_LT_Y					21
#define RETURN_BUTTON_RB_X					(RETURN_BUTTON_LT_X + 8)
#define RETURN_BUTTON_RB_Y					(RETURN_BUTTON_LT_Y + 2)
	// ���t�f�[�^LCD�̈�
#define DATE_LT_X							5
#define DATE_LT_Y							10
	// �����f�[�^LCD�̈�
#define TIME_LT_X							(DATE_LT_X + 14)
#define TIME_LT_Y							DATE_LT_Y

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
	s64				rtcOffset[2];			// RTC�I�t�Z�b�g�l�i[0]:�ݒ�ύX�O�̒l�A[1]:�ύX��̒l�j
	DateTimeParam	dtp;					// ���t�������̓V�[�P���X�p���[�N
	InputNumParam	inp;					// ���l���̓C���^�[�t�F�[�X�p���[�N
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
SetRtcWork *pRtcWork;									// RTC�ݒ�p���[�N

// const data  -----------------------------------------

//======================================================
// ���t�������ݒ�
//======================================================

// RTC�ݒ�V�[�P���X�̏�����
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
	
	pRtcWork=OS_Alloc(sizeof(SetRtcWork));								// RTC�ݒ�p���[�N�̊m��
#ifdef __SYSM_DEBUG
	if(pRtcWork==NULL) OS_Panic("ARM9- Fail to allocate memory...\n");
#endif /* __SYSM_DEBUG */
	OS_Printf("Alloc :SetRtcWork\n");
	SVC_CpuClear(0x0000, pRtcWork, sizeof(SetRtcWork), 16);
	
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	
	InitGetAndDrawRtcData( DATE_LT_X, DATE_LT_Y, TIME_LT_X, TIME_LT_Y);	// RTC�f�[�^�\���ʒu�̎w��
	
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
		offset = IPL2i_CalcRtcSecOffset( &date, &time );			// �ݒ蒼�O��RTC�l�̃I�t�Z�b�g���Z�o
		OS_Printf( " 99/12/31 23:59:00  offset = %x\n", offset );
		date.year	= 100;
		date.month	= 1;
		date.day	= 1;
		time.hour	= 0;
		time.minute	= 0;
		time.second = 0;
		offset = IPL2i_CalcRtcSecOffset( &date, &time );			// �ݒ蒼�O��RTC�l�̃I�t�Z�b�g���Z�o
		OS_Printf( "100/01/01 00:00:00  offset = %x\n", offset );
	}
*/
}


// RTC�ݒ�V�[�P���X
int SEQ_RtcSet(void)
{
	BOOL tp_set		= FALSE;
	BOOL tp_return	= FALSE;
	
	ReadTpData();													// �^�b�`�p�l�����͂̎擾
	GetAndDrawRtcData();
	
	if(tpd.disp.touch) {
		tp_set = InRangeTp( DATE_LT_X*8,       DATE_LT_Y*8-4,		// [RTC�ݒ�]�̈扟���`�F�b�N
						   (TIME_LT_X + 8)*8, (TIME_LT_Y+2)*8-4, &tpd.disp);
																	// [RETURN]�{�^�������`�F�b�N
		tp_return = InRangeTp(RETURN_BUTTON_LT_X*8, RETURN_BUTTON_LT_Y*8-4,
							  RETURN_BUTTON_RB_X*8, RETURN_BUTTON_RB_Y*8-4, &tpd.disp);
	}
	if( initialSet && !GetNCDWork()->option.input_rtc ) {
		tp_set = TRUE;
	}
	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if(pad.trg & PAD_KEY_DOWN){										// �J�[�\���̈ړ�
		if(++pRtcWork->csr == RTC_MENU_ELEM_NUM)	pRtcWork->csr = 0;
	}
	if(pad.trg & PAD_KEY_UP){
		if(--pRtcWork->csr < 0) pRtcWork->csr = RTC_MENU_ELEM_NUM - 1;
	}
	
	if((pad.trg & PAD_BUTTON_A) || (tp_set)) {						// RTC�ݒ�J�n
		if(pRtcWork->csr == 0) {
			SEQ_InputRtcDateTime_init(1);
			nowProcess = SEQ_InputRtcDateTime;
		}
	}else if((pad.trg & PAD_BUTTON_B) || (tp_return)) {				// ���j���[�ɖ߂�
		OS_Free(pRtcWork);												// RTC�ݒ�p���[�N�̉��
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
// ���t���������͏���
//======================================================

// ���t�������͏�����
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


// ���t��������
static int SEQ_InputRtcDateTime(void)
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
	
	ReadTpData();													// �^�b�`�p�l�����͂̎擾
	CheckOKCancelButton(&tp_ok, &tp_cancel);						// [OK],[CANCEL]�{�^�������`�F�b�N
	
	pRtcWork->inp.y_offset = 0;
	if(tpd.disp.touch) {											// [CANCEL]�{�^�������`�F�b�N
		if((pRtcWork->dtp.seq & 0x01) && (pRtcWork->dtp.seq < SEQ_END)) {		// SEQ_**_SET�̎��̂ݗL��
			new_seq = pRtcWork->dtp.seq;
			x_base  = DATE_LT_X * 8;
			y_base  = DATE_LT_Y * 8 + 6;
			// ���͍��ڈړ��̃`�F�b�N
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
				// ���͒l�̑���
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
	
	// �^�b�`�p�l�� or �L�[���͂ɂ���āA�J�[�\���ʒu�����������ɁA���̈ʒu�̃J�[�\���������B
	if((pRtcWork->dtp.seq > 0) && ((pRtcWork->dtp.seq & 0x01) == 0)) {		// SEQ_INIT�̎��͎��s���Ȃ�
		(void)DrawDecimalSJIS( pRtcWork->inp.pos_x, pRtcWork->inp.pos_y, WHITE, pRtcWork->dtp.tgtp, (u8)pRtcWork->inp.keta_max, 4);
	}
	
	// �e�V�[�P���X�̏���
	switch(pRtcWork->dtp.seq){
		
	  case SEQ_INIT:
		pRtcWork->dtp.Date		= GetSYSMWork()->rtc[0].Date;
		pRtcWork->dtp.Time		= GetSYSMWork()->rtc[0].Time;
		pRtcWork->dtp.Date.year += 2000;									// year���{�Q�O�O�O����B
		pRtcWork->dtp.seq		= SEQ_YEAR_INIT;
		// ��SEQ_INIT�͒��ʂ�SEQ_YEAR_INIT��
		
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
																	// �N�E�������Ƃɂ��̌��̓������Z�o����B
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
		
		// �N�������͂Ȃ�΁A�j�����Z�o���ĕ\���B
		if( (pRtcWork->dtp.seq == SEQ_YEAR_SET) || (pRtcWork->dtp.seq == SEQ_MONTH_SET) || (pRtcWork->dtp.seq == SEQ_DAY_SET) ) {
			pRtcWork->dtp.Date.week = CalcWeekFromDate(pRtcWork->dtp.Date.year, pRtcWork->dtp.Date.month, pRtcWork->dtp.Date.day);
			(void)DrawStringSJIS( DATE_LT_X + 10, DATE_LT_Y, WHITE, g_strWeek[pRtcWork->dtp.Date.week]);
		}
		
		// �N�E�����͂Ȃ�΁A�������Z�o���āA���݂̓��͓��������𒴂��Ă�����C������B
		if( (pRtcWork->dtp.seq == SEQ_YEAR_SET) || (pRtcWork->dtp.seq == SEQ_MONTH_SET) ) {
			u32 dayNum = SYSM_GetDayNum( pRtcWork->dtp.Date.year, pRtcWork->dtp.Date.month );
			if( dayNum < pRtcWork->dtp.Date.day) {
				pRtcWork->dtp.Date.day = dayNum;
				(void)DrawDecimalSJIS( DATE_LT_X + 7, DATE_LT_Y, WHITE, &pRtcWork->dtp.Date.day, 2, 4);
			}
		}
		break;
		
	  case SEQ_END:
		pRtcWork->dtp.Date.year -= 2000;									// year���|�Q�O�O�O����B
		
/*		// RTC�ւ̐V�����l�̐ݒ�
		(void)RTC_GetDateTime(&now_dtp.Date, &now_dtp.Time);		// ���C�g���O�Ɍ��݂�RTC�l���擾����B
		(void)RTC_SetDateTime(&pRtcWork->dtp.Date, &pRtcWork->dtp.Time);		// �VRTC�ݒ�l�̃Z�b�g�B
		
		if((GetSYSMWork()->rtc[0].Date.year == 99) && (now_dtp.Date.year == 0)) {
			now_dtp.Date.year = 100;								// �ݒ�O�`�ݒ芮���̊Ԃ�RTC��������Ă��܂�����Ayear��100�Ƃ���offset���v�Z����B
		}
		// RTC�ݒ莞�́A����̐ݒ�łǂꂾ��RTC�l���ω��������i�b�I�t�Z�b�g�P�ʁj���Z�o����NVRAM�ɕۑ�����B�i�Ƃ肠���������j
		pRtcWork->rtcOffset[0] = IPL2i_CalcRtcSecOffset( &now_dtp.Date,  &now_dtp.Time );	// ���݂�RTC�l�̃I�t�Z�b�g���Z�o
		pRtcWork->rtcOffset[1] = IPL2i_CalcRtcSecOffset( &pRtcWork->dtp.Date, &pRtcWork->dtp.Time );	// �V�����Z�b�g���ꂽRTC�l�̃I�t�Z�b�g���Z�o
		GetNCDWork()->option.rtcOffset += pRtcWork->rtcOffset[1] - pRtcWork->rtcOffset[0];
																	// �VRTC_ofs �� ���݂�RTC_ofs �̍����̒l�����Z�B
*/
		
		pRtcWork->dtp.Time.second = 0;
		NCD_SetRtcOffset( SYSM_CalcRtcOffsetAndSetDateTime( &pRtcWork->dtp.Date, &pRtcWork->dtp.Time ) );
		
		GetSYSMWork()->rtc[0].Date = pRtcWork->dtp.Date;
		GetSYSMWork()->rtc[0].Time = pRtcWork->dtp.Time;
		GetSYSMWork()->ncd_invalid = 0;
		GetNCDWork()->option.input_rtc = 1;						// RTC���̓t���O�𗧂Ă�B
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAM�ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
		
		// SEQ_END�̎��͂��̂܂܃��^�[������B
		
	  case SEQ_RETURN:
		nowProcess = SEQ_RtcSet;
		SEQ_InputRtcDateTime_init(0);								// ���t���͉�ʂ̃N���A
		return 0;
	}
	
	if(pRtcWork->dtp.seq & 0x01) {										// SEQ_**_SET�̎��̂ݗL��
		if((pad.trg & PAD_BUTTON_A) || (tp_ok)) {
			pRtcWork->dtp.seq = SEQ_END;									// A�{�^���Ō���
		}else if((pad.trg & PAD_BUTTON_B) || (tp_cancel)) {			// B�{�^���ŃL�����Z��
			pRtcWork->dtp.seq = SEQ_RETURN;
		}else if(pad.trg & PAD_KEY_LEFT) {
			if(pRtcWork->dtp.seq == SEQ_YEAR_SET)	pRtcWork->dtp.seq = SEQ_SECOND_INIT;
			else								pRtcWork->dtp.seq -= 3;
		}else if(pad.trg & PAD_KEY_RIGHT) {
			if(pRtcWork->dtp.seq == SEQ_SECOND_SET)	pRtcWork->dtp.seq = SEQ_YEAR_INIT;
			else								pRtcWork->dtp.seq++;
		}
	}else {															// SEQ_**_INIT�̎��̂ݗL��
		pRtcWork->dtp.seq++;
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
	if(month == 1 || month == 2 ){
		year--;
		month += 12;
	}
	return (RTCWeek)( (year + year/4 - year/100 + year/400 + (13*month + 8)/5 + day) % 7 );
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
	
	// �L�[���͂ɉ����đΏےl�𑝌�
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
																	// �Ώےl���n�C���C�g�\��
}


// RTC�ݒ�̃N���A
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
		// NVRAM�ւ̏�������
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
}

