/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util.h

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

#ifndef __SYSM_UTIL_H__
#define __SYSM_UTIL_H__

#include <twl.h>
#include <twl/os/common/format_rom.h>
#ifdef SYSM_BUILD_FOR_DEBUGGER
#include <sysmenu/sysmenu_lib/common/sysmenu_work.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
#define BACKLIGHT_BRIGHTNESS_MAX    4

typedef enum FatalErrorCode {
    FATAL_ERROR_UNDEFINED = 0,
    FATAL_ERROR_NAND = 1,                                   // NAND�f�o�C�X�̃G���[
    FATAL_ERROR_HWINFO_NORMAL = 2,                          // HW�m�[�}�����̃��[�h�G���[
    FATAL_ERROR_HWINFO_SECURE = 3,                          // HW�Z�L���A���̃��[�h�G���[
    FATAL_ERROR_TWLSETTINGS = 4,                            // �{�̐ݒ�f�[�^�̃��[�h�G���[
    FATAL_ERROR_SHARED_FONT = 5,                            // ���L�t�H���g�̃��[�h�G���[
    FATAL_ERROR_WLANFIRM_AUTH = 6,                          // �����t�@�[���̔F�؃G���[
    FATAL_ERROR_WLANFIRM_LOAD = 7,                          // �����t�@�[���̃��[�h�G���[
    FATAL_ERROR_TITLE_LOAD_FAILED = 8,                      // �A�v���P�[�V�����̃��[�h�G���[
    FATAL_ERROR_TITLE_POINTER_ERROR = 9,                    // �u�[�g�v�����ꂽ���A�A�v�����w�肳��Ă��Ȃ�
    FATAL_ERROR_AUTHENTICATE_FAILED = 10,                   // �A�v���F�؎��s
    FATAL_ERROR_ENTRY_ADDRESS_ERROR = 11,                   // �A�v���̋N���A�h���X���s��
    FATAL_ERROR_TITLE_BOOTTYPE_ERROR = 12,                  // �A�v���u�[�g�^�C�v���s���iNAND�u�[�g�A�J�[�h�u�[�g�AMB�u�[�g�ȊO�̒l�j
    FATAL_ERROR_SIGN_DECRYPTION_FAILED = 13,                // �A�v�������f�N���v�g���s
    FATAL_ERROR_SIGN_COMPARE_FAILED = 14,                   // �A�v���������؎��s
    FATAL_ERROR_HEADER_HASH_CALC_FAILED = 15,               // �A�v���n�b�V���v�Z�p�������m�ێ��s
    FATAL_ERROR_TITLEID_COMPARE_FAILED = 16,                // �u�[�g�v�����ꂽTWL�A�v���Ǝ��ۂɃ��[�h�����A�v����TitleID���s��v
    FATAL_ERROR_VALID_SIGN_FLAG_OFF = 17,                   // �A�v��ROM�w�b�_�̏����L���t���O�������Ă��Ȃ�
    FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED = 18,      // �A�v���N���F�؎��s
    FATAL_ERROR_MODULE_HASH_CHECK_FAILED = 19,              // �A�v���n�b�V���s��v
    FATAL_ERROR_MODULE_HASH_CALC_FAILED = 20,               // �A�v���n�b�V���v�Z�p�������m�ێ��s
    FATAL_ERROR_MEDIA_CHECK_FAILED = 21,                    // �J�[�h�A�v����NAND�N�� or NAND�A�v�����J�[�h�N�����悤�Ƃ����i�f�o�b�K�N���������j
    FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED = 22,             // DS�_�E�����[�h�v���C�A�v�������̃}�W�b�N�R�[�h���s���iTEMP�u�[�g�A�v���u�[�g���j
    FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED = 23,             // DS�_�E�����[�h�v���C�A�v�������̃f�N���v�g���s�iTEMP�u�[�g�A�v���u�[�g���j
    FATAL_ERROR_DL_HASH_CALC_FAILED = 24,                   // DS�_�E�����[�h�v���C�A�v���n�b�V���v�Z�p�������m�ێ��s�iTEMP�u�[�g�A�v���u�[�g���j
    FATAL_ERROR_DL_SIGN_COMPARE_FAILED = 25,                // DS�_�E�����[�h�v���C�A�v���n�b�V���s��v�iTEMP�u�[�g�A�v���u�[�g���j
    FATAL_ERROR_WHITELIST_INITDB_FAILED = 26,               // NTR�z���C�g���X�g���g�̔F�؎��s
    FATAL_ERROR_WHITELIST_NOTFOUND = 27,                    // �N��NTR�A�v���̃C�j�V�����R�[�h��NTR�z���C�g���X�g�Ɍ�����Ȃ�����
    FATAL_ERROR_DHT_PHASE1_FAILED = 28,                     // �A�v����NTR�z���C�g���X�g�F�؎��s�i�t�F�[�Y�P�j
    FATAL_ERROR_DHT_PHASE2_FAILED = 29,                     // �A�v����NTR�z���C�g���X�g�F�؎��s�i�t�F�[�Y�Q�j
    FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF = 30,             // TMP�u�[�g�A�v����ROM�w�b�_��TMP�W�����v���r�b�g�������Ă��Ȃ�
    FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN = 31,                  // TWL�A�v���u�[�g�^�C�v�s��
    FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN = 32,                  // NTR�A�v���u�[�g�^�C�v�s��
    FATAL_ERROR_PLATFORM_UNKNOWN = 33,                      // ROM�w�b�_�̃v���b�g�z�[���R�[�h�s��
    FATAL_ERROR_LOAD_UNFINISHED = 34,                       // �A�v�����[�h���������Ă��Ȃ��̂ɁA�F�؃t�F�[�Y�ɐi��
    FATAL_ERROR_LOAD_OPENFILE_FAILED = 35,                  // NAND�A�v���̃t�@�C���I�[�v�����s
    FATAL_ERROR_LOAD_MEMALLOC_FAILED = 36,                  // �A�v���n�b�V���v�Z�p�������m�ێ��s
    FATAL_ERROR_LOAD_SEEKFILE_FAILED = 37,                  // NAND�A�v���̃t�@�C���V�[�N���s
    FATAL_ERROR_LOAD_READHEADER_FAILED = 38,                // �A�v��ROM�w�b�_���[�h���s
    FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39,                    // �A�v��ROM�w�b�_Nintendo���SCRC�s��
    FATAL_ERROR_LOAD_READDLSIGN_FAILED = 40,                // TMP�u�[�g�A�v����DS�_�E�����[�h�v���C�������[�h���s
    FATAL_ERROR_LOAD_RELOCATEINFO_FAILED = 41,              // �A�v���Ĕz�u��񐶐����s
    FATAL_ERROR_LOAD_READMODULE_FAILED = 42,                // �A�v�����[�h���s
    FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED = 43,            // �A�v��ROM�w�b�_Nintendo���S�f�[�^�s��
    FATAL_ERROR_SYSMENU_VERSION = 44,                       //
    FATAL_ERROR_DHT_PHASE1_CALC_FAILED = 45,                // NTR�A�v���z���C�g���X�g�n�b�V���v�Z�p�������m�ێ��s
    FATAL_ERROR_LOAD_UNKNOWN_BOOTTYPE = 46,                 // �A�v���u�[�g�^�C�v���s��
    FATAL_ERROR_LOAD_AUTH_HEADER_FAILED = 47,               // �A�v��ROM�w�b�_�F�؎��s
    FATAL_ERROR_LOAD_NEVER_STARTED = 48,                    // ���[�h���J�n����Ă��Ȃ��̂ɁA�F�؂��J�n���ꂽ
    FATAL_ERROR_EJECT_CARD_AFTER_LOAD_START = 49,           // �J�[�h��������Ă���̂ɁA�J�[�h�A�v���̃��[�h���J�n���ꂽ
    FATAL_ERROR_TITLEID_COMPARE_FAILED_NTR = 50,            // �u�[�g�v�����ꂽNTR�A�v���Ǝ��ۂɃ��[�h�����A�v����TitleID���s��v
    FATAL_ERROR_DHT_PHASE3_FAILED = 51,                     // �A�v����NTR�z���C�g���X�g�F�؎��s�i�t�F�[�Y�R�j
    FATAL_ERROR_DHT_PHASE4_FAILED = 52,                     // �A�v����NTR�z���C�g���X�g�F�؎��s�i�t�F�[�Y�S�j
    FATAL_ERROR_BACKUP_DATA_CHECK_FAILED = 53,              // �o�b�N�A�b�v�f�[�^�̌��؎��s

    FATAL_ERROR_MAX = 53
}FatalErrorCode;


// global variable------------------------------------------------------
// function-------------------------------------------------------------

#ifdef SDK_ARM9

// �o�b�N���C�g
extern u32 UTL_SetBacklightBrightness( u8 brightness );                     // �o�b�N���C�g�P�x�Z�b�g
extern u32 UTL_GetBacklightBrightness( u8 *pBrightness );                   // �o�b�N���C�g�P�x�Q�b�g

// �^�b�`�p�l��
extern void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib );            // TP�L�����u���[�V����
extern BOOL UTL_IsValidCalibration( u16 x, u16 y, u16 correct_x, u16 correct_y );   // TP�L�����u���[�V������Ƀ^�b�`�����|�C���g�����m���H

// �X���[�v
extern void UTL_GoSleepMode( void );

// RTC�֌W
extern BOOL UTL_CheckRTCDate( RTCDate *pDate );                             // ���t�����킩�`�F�b�N
extern BOOL UTL_CheckRTCTime( RTCTime *pTime );                             // ���������킩�`�F�b�N
extern s64  UTL_CalcRTCOffset( RTCDate *pNewDate, RTCTime *pNewTime );      // RTC�I�t�Z�b�g�v�Z��RTC�ւ̓��t�����`�F�b�N���s��
extern u32  UTL_GetDayNum( u32 year, u32 month );                           // �w�肳�ꂽ�N�E���̓������擾����
extern BOOL UTL_IsLeapYear100( u32 year );                                  // �w�肳�ꂽ�N�����邤�N�����ׂ�

// �y�A�����^���R���g���[���₢���킹
extern u32  UTL_CalcPCTLInquiryCode( void );                                // �₢���킹�R�[�h�i�P�O�i�W���j�Z�o
extern u32  UTL_CalcPCTLMasterKey( void );                                  // �}�X�^�[�L�[�@�@�i�P�O�i�T���j�Z�o�i��������RTC_GetDate���g�p���܂��B�j

// �A�v��ROM�w�b�_�̗vEULA�t���O�擾
extern BOOL UTL_IsROMHeaderEULARequired( void );

// �A�v��ROM�w�b�_��nintendo���S�̐������`�F�b�N
extern BOOL UTL_CheckNintendoLogoData( ROM_Header_Short *rh );

#endif

// FATAL�G���[
extern BOOL UTL_IsFatalError( void );                                       // FATAL�G���[���H
extern void UTL_SetFatalError( FatalErrorCode error );                      // FATAL�G���[�̃Z�b�g
extern u64  UTL_GetFatalError( void );                                      // FATAL�G���[��Ԃ̎擾�iFatalErrorCode���r�b�g�Ɋ��蓖�ĂĊi�[���Ă��܂��B�j


// ���[�W�����`�F�b�N
static inline BOOL UTL_CheckAppRegion( u32 card_region_bitmap )
{
#ifdef SYSM_BUILD_FOR_DEBUGGER
#pragma unused(card_region_bitmap)
	// �f�o�b�K���쎞�̂݁A���[�W�����`�F�b�N�𖳌��ɂ���B
	if( SYSM_IsRunOnDebugger() ) {
		return TRUE;
	}
#endif
    return ( card_region_bitmap & ( 0x00000001 << OS_GetRegion() ) ) ? TRUE : FALSE;
}

// CRC�`�F�b�N
static inline BOOL UTL_CheckAppCRC16( ROM_Header_Short *pROMH )
{
    u16 calc_crc = SVC_GetCRC16( 65535, pROMH, 0x015e );
    if( ( calc_crc != pROMH->header_crc16 ) ||
        ( 0xcf56   != pROMH->nintendo_logo_crc16 ) ){
        return FALSE;
    }
    return TRUE;
}


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_UTIL_H__
