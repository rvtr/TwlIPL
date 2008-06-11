/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util.c

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
#include <twl/nam.h>
#include <sysmenu.h>

// define data------------------------------------------
#define TP_CL_CONFIRM_MARGIN		4				// TP�L�����u���[�V�����̍��W�}�[�W���i�L�����u���[�V������̍��W�ł̒l�j

// extern data------------------------------------------
// function's prototype declaration---------------------
static s64 UTLi_CalcRTCSecOffset( RTCDate *datep, RTCTime *timep );

// global variable -------------------------------------
// static variable -------------------------------------
// const data  -----------------------------------------
// function's description-------------------------------


//======================================================================
//  �o�b�N���C�g
//======================================================================

// �o�b�N���C�g�P�x�Z�b�g
u32 UTL_SetBacklightBrightness( u8 brightness )
{
	brightness %= ( BACKLIGHT_BRIGHTNESS_MAX + 1 );
	return PM_SendUtilityCommand( PMi_UTIL_SET_BACKLIGHT_BRIGHTNESS, (u16)brightness, NULL );
}


// �o�b�N���C�g�P�x
u32 UTL_GetBacklightBrightness( u8 *pBrightness )
{
	u16 status;
	u32 result = PM_SendUtilityCommand( PM_UTIL_GET_STATUS, PMi_UTIL_GET_BACKLIGHT_BRIGHTNESS, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (pBrightness)
		{
			*pBrightness = (u8)status;
		}
	}
	return result;
}


//======================================================================
//  �^�b�`�p�l��
//======================================================================

// �^�b�`�p�l���L�����u���[�V����
void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib )
{
	TPCalibrateParam calibParam;
	
	// TP�L�����u���[�V����
	( void )TP_CalcCalibrateParam( &calibParam,							// �^�b�`�p�l��������
			pCalib->data.raw_x1, pCalib->data.raw_y1, (u16)pCalib->data.dx1, (u16)pCalib->data.dy1,
			pCalib->data.raw_x2, pCalib->data.raw_y2, (u16)pCalib->data.dx2, (u16)pCalib->data.dy2 );
	TP_SetCalibrateParam( &calibParam );
	OS_TPrintf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			pCalib->data.raw_x1, pCalib->data.raw_y1, (u16)pCalib->data.dx1, (u16)pCalib->data.dy1,
			pCalib->data.raw_x2, pCalib->data.raw_y2, (u16)pCalib->data.dx2, (u16)pCalib->data.dy2 );
}


// �L�����u���[�V����������ɍs��ꂽ���`�F�b�N
BOOL UTL_IsValidCalibration( u16 x, u16 y, u16 correct_x, u16 correct_y )
{
	return !( x < correct_x - TP_CL_CONFIRM_MARGIN ||
			  x > correct_x + TP_CL_CONFIRM_MARGIN ||
			  y < correct_y - TP_CL_CONFIRM_MARGIN ||
			  y > correct_y + TP_CL_CONFIRM_MARGIN );
}


//======================================================================
//  �X���[�v
//======================================================================

// �X���[�v���[�h�ւ̑J��
void UTL_GoSleepMode( void )
{
    // �W������
    if ( ! PAD_DetectFold() )
    {
        return;
    }

    // �f�o�b�K�ڑ��������̓X���[�v�ɓ���Ȃ��i�W���ł��f�o�b�K���N������悤�Ɂj
    if ( !SYSM_IsRunOnDebugger() || (OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER) )
    {
        // �J�[�h���������o�ݒ�
        //   TWL�ł̓Q�[���J�[�h�̍ă��[�h���\�Ȃ���
        //   �X���[�v���̃J�[�h�������o�𖳌���
        //   �iDS-IPL�ł̓Q�[���J�[�h���N���ł��Ȃ��Ȃ�̂�
        //     ���W���[������ROM-ID�`�F�b�N�ŃG���[�ɂȂ��
        //     �V���b�g�_�E�����Ă����j
        OSIntrMode enable = OS_DisableInterrupts();
        reg_MI_MCCNT0 &= ~REG_MI_MCCNT0_I_MASK;
        OS_ResetRequestIrqMask( OS_IE_CARD_IREQ );
        OS_RestoreInterrupts( enable );

        // �X���[�v�J��
    	PM_GoSleepMode( PM_TRIGGER_COVER_OPEN |
	    				PM_TRIGGER_RTC_ALARM,
		    			0,
			    		0 );
    }
}


//======================================================================
//  RTC�I�t�Z�b�g����
//======================================================================

// RTC�ɐV�����ݒ�l���Z�b�g���āA���̒l�����Ƃ�rtcOffset�l���Z�o����B
s64 UTL_CalcRTCOffset( RTCDate *newDatep, RTCTime *newTimep )
{
	RTCDate oldDate;
	RTCTime oldTime;
	s64		offset0;
	s64		offset1;
	s64		offset;
	
	// RTC�ւ̐V�����l�̐ݒ�
	(void)RTC_GetDateTime( &oldDate, &oldTime );					// ���C�g���O�Ɍ��݂�RTC�l���擾����B
	oldTime.second = 0;
	
	// RTC�ݒ莞�́A����̐ݒ�łǂꂾ��RTC�l���ω��������i�b�I�t�Z�b�g�P�ʁj���Z�o�B
	if( ( oldDate.year < LCFG_TSD_GetRTCLastSetYear() ) && ( LCFG_TSD_IsFinishedInitialSetting() ) ) {
		oldDate.year += 100;										// �O��̐ݒ�`����̐ݒ�̊Ԃ�RTC��������Ă��܂�����Ayear��100�����Z����offset���v�Z����B
	}
	LCFG_TSD_SetRTCLastSetYear( (u8)newDatep->year );
	
	offset0	= UTLi_CalcRTCSecOffset( &oldDate, &oldTime );			// �ݒ蒼�O��RTC�l�̃I�t�Z�b�g���Z�o
	offset1	= UTLi_CalcRTCSecOffset(  newDatep, newTimep );		// �V�����Z�b�g���ꂽRTC�l�̃I�t�Z�b�g���Z�o
	offset	= LCFG_TSD_GetRTCOffset() + offset1 - offset0;			// �VRTC_ofs �� ���݂�RTC_ofs �̍����̒l�����Z���ă��^�[���B
	
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
static s64 UTLi_CalcRTCSecOffset( RTCDate *datep, RTCTime *timep )
{
	u32 i;
	int uruu   = 0;
	int dayNum = 0;
	s64 offset;
	
	// ���A���A�b���@�b or ���I�t�Z�b�g��
#ifdef SECOND_OFFSET
	offset = ( timep->hour * 60 + timep->minute ) * 60 + timep->second;	// ���L���X�g�����Ƀo�O����
#else
	offset =   timep->hour * 60 + timep->minute;
#endif
	
	// ���A�����@�����Ɋ��Z���Ă���A�@�b or ���I�t�Z�b�g��
	dayNum = (int)datep->day - 1;
	for( i = 1; i < datep->month; i++ ) {
		dayNum += UTL_GetDayNum( datep->year, i );
	}
	
	// �N���@�����Ɋ��Z
	if( datep->year > 0 ) {
		uruu = ( ( (int)datep->year - 1 ) >> 2 ) + 1;					// �w��N-1�܂ł̂��邤�N�̌����Z�o���āA���̓��������Z�B
	}
	dayNum += uruu + (u32)( datep->year * 365 );
	
	// �N�E���E��������Ɋ��Z�����l���@�b or ���I�t�Z�b�g��
#ifdef SECOND_OFFSET
	offset += (s64)( dayNum * 24 * 3600 );	// ���L���X�g�����Ƀo�O����
#else
	offset += (s64)( dayNum * 24 * 60 );
#endif
	
	return offset;
}


// �w�肳�ꂽ�N�E���̓�����Ԃ��B
u32 UTL_GetDayNum( u32 year, u32 month )
{
	u32 dayNum = 31;
	if( month == 2 ) {
		if( UTL_IsLeapYear100( year ) ) {
			dayNum -= 2;
		}else {
			dayNum -= 3;
		}
	}else if( ( month == 4 ) || ( month == 6 ) || ( month == 9 ) || ( month == 11 ) ) {
		dayNum--;
	}
	return dayNum;
}


// �ȈՂ��邤�N�̔��� (���邤�N�F1�A�ʏ�̔N�F0�j��RTC�̂Ƃ肤���2000�`2100�N�Ɍ��肷��B
BOOL UTL_IsLeapYear100( u32 year )
{
	if( ( year & 0x03 ) || ( year == 100 ) ) {						// ���邤�N�́A�u4�Ŋ���؂�@���@100�Ŋ���؂�Ȃ��N�v�܂��́u400�Ŋ���؂��N�v
		return FALSE;
	}else {
		return TRUE;
	}
}


// RTC�̓��t�����������`�F�b�N
BOOL UTL_CheckRTCDate( RTCDate *datep )
{
	if(	 ( datep->year >= 100 )
	  || ( datep->month < 1 ) || ( datep->month > 12 )
	  || ( datep->day   < 1 ) || ( datep->day   > 31 )
	  || ( datep->week >= RTC_WEEK_MAX ) ) {
		return FALSE;
	}
	return TRUE;
}


// RTC�̎��������������`�F�b�N
BOOL UTL_CheckRTCTime( RTCTime *timep )
{
	if(  ( timep->hour   > 23 )
	  || ( timep->minute > 59 )
	  || ( timep->second > 59 ) ) {
		return FALSE;
	}
	return TRUE;
}

