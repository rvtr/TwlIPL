/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.h

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

#ifndef	__SYSM_UTIL_H__
#define	__SYSM_UTIL_H__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
#define BACKLIGHT_BRIGHTNESS_MAX   	4

// global variable------------------------------------------------------
// function-------------------------------------------------------------

#ifdef SDK_ARM9

// �o�b�N���C�g
extern u32 UTL_SetBacklightBrightness( u8 brightness );						// �o�b�N���C�g�P�x�Z�b�g
extern u32 UTL_GetBacklightBrightness( u8 *pBrightness );					// �o�b�N���C�g�P�x�Q�b�g

// �^�b�`�p�l��
extern void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib );

// �X���[�v
extern void UTL_GoSleepMode( void );

// RTC�֌W
extern BOOL UTL_CheckRTCDate( RTCDate *pDate );								// ���t�����킩�`�F�b�N
extern BOOL UTL_CheckRTCTime( RTCTime *pTime );								// ���������킩�`�F�b�N
extern s64  UTL_CalcRTCOffset( RTCDate *pNewDate, RTCTime *pNewTime );		// RTC�I�t�Z�b�g�v�Z��RTC�ւ̓��t�����`�F�b�N���s��
extern u32  UTL_GetDayNum( u32 year, u32 month );							// �w�肳�ꂽ�N�E���̓������擾����
extern BOOL UTL_IsLeapYear100( u32 year );									// �w�肳�ꂽ�N�����邤�N�����ׂ�

// �y�A�����^���R���g���[���₢���킹
extern u32  UTL_CalcPCTLInquiryCode( void );								// �₢���킹�R�[�h�i�P�O�i�W���j�Z�o
extern u32  UTL_CalcPCTLMasterKey( void );									// �}�X�^�[�L�[�@�@�i�P�O�i�T���j�Z�o�i��������RTC_GetDate���g�p���܂��B�j

// �^�C�g�����擾�i������NAM���g�p����̂ŁANAM_Init�����O�ɌĂ΂�Ă���K�v����j
extern int  UTL_GetInstalledSoftBoxCount( void );

#endif

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_UTIL_H__
