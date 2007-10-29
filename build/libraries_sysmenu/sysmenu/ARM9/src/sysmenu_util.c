/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_util.c

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

// define data------------------------------------------

// extern data------------------------------------------

// function's prototype declaration---------------------
static s64 SYSMi_CalcRtcSecOffset( RTCDate *datep, RTCTime *timep );

// global variable -------------------------------------

// static variable -------------------------------------

// const data  -----------------------------------------

// function's description-------------------------------

//======================================================================
//  NITRO�ݒ�f�[�^�@���[�N����
//======================================================================

// NITRO�ݒ�f�[�^�̃j�b�N�l�[���E�F�E�a�����̏������B
void NCD_ClearOwnerInfo( void )
{
	NitroConfigData *ncdp = GetNCDWork();
	
	MI_CpuClear16( &ncdp->owner, sizeof(NvOwnerInfo) );
	ncdp->owner.birthday.month			= 1;
	ncdp->owner.birthday.day			= 1;
	ncdp->option.input_birthday			= 0;
	ncdp->option.input_favoriteColor	= 0;
	ncdp->option.input_nickname			= 0;
}


//======================================================================
//  RTC�I�t�Z�b�g����
//======================================================================

// RTC�ɐV�����ݒ�l���Z�b�g���āA���̒l�����Ƃ�rtcOffset�l���Z�o����B
s64 SYSM_CalcRtcOffsetAndSetDateTime( RTCDate *newDatep, RTCTime *newTimep )
{
	RTCDate oldDate;
	RTCTime oldTime;
	s64		offset0;
	s64		offset1;
	s64		offset;
	
	// RTC�ւ̐V�����l�̐ݒ�
	(void)RTC_GetDateTime( &oldDate, &oldTime );					// ���C�g���O�Ɍ��݂�RTC�l���擾����B
	(void)RTC_SetDateTime(  newDatep, newTimep );					// �VRTC�ݒ�l�̃Z�b�g�B
	oldTime.second = 0;
	
	// RTC�ݒ莞�́A����̐ݒ�łǂꂾ��RTC�l���ω��������i�b�I�t�Z�b�g�P�ʁj���Z�o�B
	if( ( oldDate.year < NCD_GetRtcLastSetYear() ) && ( NCD_GetInputRTC() ) ) {
		oldDate.year += 100;										// �O��̐ݒ�`����̐ݒ�̊Ԃ�RTC��������Ă��܂�����Ayear��100�����Z����offset���v�Z����B
	}
	NCD_SetRtcLastSetYear( (u8)newDatep->year );
	
	offset0	= SYSMi_CalcRtcSecOffset( &oldDate, &oldTime );			// �ݒ蒼�O��RTC�l�̃I�t�Z�b�g���Z�o
	offset1	= SYSMi_CalcRtcSecOffset(  newDatep, newTimep );		// �V�����Z�b�g���ꂽRTC�l�̃I�t�Z�b�g���Z�o
	offset	= NCD_GetRtcOffset() + offset1 - offset0;				// �VRTC_ofs �� ���݂�RTC_ofs �̍����̒l�����Z���ă��^�[���B
	
	OS_Printf ("Now    Date = year:%3d month:%3d date:%3d  hour:%3d minute:%3d second:%3d\n",
			   oldDate.year, oldDate.month,  oldDate.day,
			   oldTime.hour, oldTime.minute, oldTime.second);
	OS_Printf ("Set    Date = year:%3d month:%3d date:%3d  hour:%3d minute:%3d second:%3d\n",
				newDatep->year, newDatep->month,  newDatep->day,
				newTimep->hour, newTimep->minute, newTimep->second);
	OS_Printf ("offset[0] = %x\n", offset0 );
	OS_Printf ("offset[1] = %x\n", offset1 );
	OS_Printf ("rtcOffset = %x\n", offset );
	
	return offset;
}


// RTC�I�t�Z�b�g�l�̎Z�o
#define SECOND_OFFSET
static s64 SYSMi_CalcRtcSecOffset( RTCDate *datep, RTCTime *timep )
{
	u32 i;
	int uruu   = 0;
	int dayNum = 0;
	s64 offset;
	
	// ���A���A�b���@�b or ���I�t�Z�b�g��
#ifdef SECOND_OFFSET
	offset = ( timep->hour * 60 + timep->minute ) * 60 + timep->second;
#else
	offset =   timep->hour * 60 + timep->minute;
#endif
	
	// ���A�����@�����Ɋ��Z���Ă���A�@�b or ���I�t�Z�b�g��
	dayNum = (int)datep->day - 1;
	for( i = 1; i < datep->month; i++ ) {
		dayNum += SYSM_GetDayNum( datep->year, i );
	}
	
	// �N���@�����Ɋ��Z
	if( datep->year > 0 ) {
		uruu = ( ( (int)datep->year - 1 ) >> 2 ) + 1;					// �w��N-1�܂ł̂��邤�N�̌����Z�o���āA���̓��������Z�B
	}
	dayNum += uruu + (u32)( datep->year * 365 );
	
	// �N�E���E��������Ɋ��Z�����l���@�b or ���I�t�Z�b�g��
#ifdef SECOND_OFFSET
	offset += (s64)( dayNum * 24 * 3600 );
#else
	offset += (s64)( dayNum * 24 * 60 );
#endif
	
	return offset;
}


// �w�肳�ꂽ�N�E���̓�����Ԃ��B
u32 SYSM_GetDayNum( u32 year, u32 month )
{
	u32 dayNum = 31;
	if( month == 2 ) {
		if( SYSM_IsLeapYear100( year ) ) {
			dayNum -= 2;
		}else {
			dayNum -= 3;
		}
	}else if( ( month == 4 ) || ( month == 6 ) || ( month == 9 ) || ( month == 11 ) ) {
		dayNum--;
	}
	return dayNum;
}

/*
u32 SYSM_GetDayNum( u32 year, u32 month )
{
	u8 date_tbl[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if( ( month == 2 ) && SYSM_IsLeapYear100( year ) ) {
		return 29;
	}
	return date_tbl[ month - 1 ];									// �P������P�Q��������P����
}
*/

// �ȈՂ��邤�N�̔��� (���邤�N�F1�A�ʏ�̔N�F0�j��RTC�̂Ƃ肤���2000�`2100�N�Ɍ��肷��B
BOOL SYSM_IsLeapYear100( u32 year )
{
	if( ( year & 0x03 ) || ( year == 100 ) ) {						// ���邤�N�́A�u4�Ŋ���؂�@���@100�Ŋ���؂�Ȃ��N�v�܂��́u400�Ŋ���؂��N�v
		return FALSE;
	}else {
		return TRUE;
	}
}

