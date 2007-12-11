/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLSettings.h

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


#ifndef	TWL_SETTINGS_H_
#define	TWL_SETTINGS_H_
#if		defined(SDK_CW)							// TWLSettingsData�Ƀr�b�g�t�B�[���h���g���Ă���̂ŁA�R���p�C���ˑ��ŕs�����������\��������B
												// ����āACW�ȊO�̃R���p�C���̏ꍇ�́A���̃w�b�_�𖳌��ɂ��ăG���[���o������悤�ɂ��čĊm�F����B

#include <twl.h>
#include <sysmenu/settings/common/countryCode.h>
#include <sysmenu/settings/common/NTRSettings.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define TWL_SETTINGS_FILE_LENGTH				( 16 * 1024 )
#define TWL_SETTINGS_DATA_VERSION				1							// TWL�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����(�J�nNo.:1)
// �I�[�i�[���
#define TWL_NICKNAME_LENGTH						NTR_NICKNAME_LENGTH			// �j�b�N�l�[����
#define TWL_NICKNAME_BUFFERSIZE					( ( TWL_NICKNAME_LENGTH + 1 ) * 2 )	// �j�b�N�l�[���o�b�t�@�T�C�Y
#define TWL_COMMENT_LENGTH						NTR_COMMENT_LENGTH			// �R�����g��
#define TWL_COMMENT_BUFFERSIZE					( ( TWL_COMMENT_LENGTH + 1 ) * 2 )	// �R�����g�o�b�t�@�T�C�Y
#define TWL_FAVORITE_COLOR_MAX_NUM				NTR_FAVORITE_COLOR_MAX_NUM	// �D���ȐF�̍ő吔
// �o�b�N���C�g�P�x
#define TWL_BACKLIGHT_LEVEL_MAX					22							// TWL�o�b�N���C�g�ő�P�x���x��
// �y�A�����^���R���g���[��
#define TWL_PARENTAL_CONTROL_RATING_AGE_MAX		31
#define TWL_PARENTAL_CONTROL_PASSWORD_LENGTH	4               			// �Ïؔԍ��̌���
#define TWL_PARENTAL_CONTROL_PASSWORD_DEFAULT	"0000"          			// �f�t�H���g�Ïؔԍ�
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MIN		6   			// �閧�̎���̉񓚁AUTF-16�ōŏ� MIN ����
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX		32     			// �閧�̎���̉񓚁AUTF-16�ōő� MAX ����
// �C���X�g�[���E�\�t�g��
#define TWL_FREE_SOFT_BOX_COUNT_MAX				35   						// NAND�A�v���̍ő��Box��, ���� - freeSoftBoxCount�ŃC���X�g�[��Soft��


// ����R�[�h
// ���B�Ɩk�Ă̕\���e�L�X�g�̈Ⴂ�́A���[�W�����ƌ���R�[�h�𕹂��Ĕ��f
typedef enum TWLLangCode{
	TWL_LANG_JAPANESE = 0,						// ���{��
	TWL_LANG_ENGLISH  = 1,						// �p��
	TWL_LANG_FRENCH   = 2,						// �t�����X��
	TWL_LANG_GERMAN   = 3,						// �h�C�c��
	TWL_LANG_ITALIAN  = 4,						// �C�^���A��
	TWL_LANG_SPANISH  = 5,						// �X�y�C����
	TWL_LANG_SIMP_CHINESE = 6,					// ������i�ȑ̎��j
	TWL_LANG_KOREAN   = 7,						// �؍���
//	TWL_LANG_DUTCH    = 8,						// �I�����_��iWii�ł͑��݁j
//	TWL_LANG_TRAD_CHINESE = 9,					// ��p��i�ɑ̎��j�iWii�ł͑��݁j
	
	TWL_LANG_CODE_MAX
}TWLLangCode;

// ���t
#define TWLDate						NTRDate

// �A���[��
#define TWLAlarm					NTRAlarm


// TP�L�����u���[�V�����iNTR�Ƃ̈Ⴂ�́A�\��̈悠��j
typedef struct TWLTPCalibData {
	NTRTPCalibData	data;						// TP�L�����u���[�V�����f�[�^
	u8				rsv[ 8 ];
}TWLTPCalibData;


// �I�[�i�[���
typedef struct TWLOwnerInfo{
	u8				userColor : 4;				// �D���ȐF
	u8				rsv : 4;					// �\��B
	u8				pad;						// �p�f�B���O
	TWLDate			birthday;					// ���N����
	u16				nickname[ TWL_NICKNAME_LENGTH + 1 ];	// �j�b�N�l�[���i�I�[����j
	u16				comment[ TWL_COMMENT_LENGTH + 1 ];		// �R�����g�i�I�[����j
}TWLOwnerInfo;		// 80byte


// �y�A�����^���R���g���[��
// �R���c��
typedef enum TWLRatingOgn {
	TWL_RATING_OGN_CERO          = 0,   // ���{
	TWL_RATING_OGN_ESRB          = 1,   // �A�����J
	TWL_RATING_OGN_USK           = 2,   // �h�C�c
	TWL_RATING_OGN_PEGI_GENERAL  = 3,   // ���B
	TWL_RATING_OGN_PEGI_PORTUGAL = 4,   // �|���g�K��
	TWL_RATING_OGN_PEGI_BBFC     = 5,   // �C�M���X
	TWL_RATING_OGN_AGCB          = 6,   // �I�[�X�g�����A
	TWL_RATING_OGN_OFLC          = 7,   // �j���[�W�[�����h
	TWL_RATING_OGN_GRB           = 8,   // �؍�
	TWL_RATING_OGN_MAX           = 8
}TWLRatingOgn;

// �f�[�^
typedef struct TWLParentalControl {
	TWLRatingOgn	ogn;				// �R���c��
//	u8				flags;				// Wii�ł́APARENTAL_CONTROL_USE�t���O�̂� --> isSetParentalControl ������̂Ō���K�v�Ȃ�
	u8				ratingAge;			// ���[�e�B���O�i�N��j�l
	char			password[ TWL_PARENTAL_CONTROL_PASSWORD_LENGTH + 1 ];   // �Ïؔԍ��A�I�[�R�[�h����
	u8				secretQuestion;     // �閧�̎��╶ ID
	u8				rsv_A;
	u16				secretAnswer[ TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX + 1 ];  // UTF16,�閧�̎���ւ̉񓚁A�I�[�R�[�h����
	u16				secretAnswerLength; // �閧�̎���ւ̉񓚕������iWii��u16,LENGTH_MAX ���ێ��ł��邩��u8�ł������̂ł́H�j
//	u8				rsv_B[ 16 ]; 		// �폜�\��i16�o�C�g�̃��[�e�B���O�������̂̓A�v������ROM�w�b�_�j
}TWLParentalControl;


// TWL�ݒ�f�[�^�i��{�A�ߋ�ver�݊����l�����āA�ǉ��������Ȃ����j�ŁB�j
typedef struct TWLSettingsData{
	struct flags {
		u32		initialSequence : 1;				// ����N���V�[�P���X���H
		u32		isSetCountry : 1;					// ���R�[�h�ݒ�ς݁H
		u32		isSetLanguage : 1;					// ����ݒ�ς݁H
		u32		isSetDateTime : 1;					// ���t�E�����ݒ�ς݁H
		u32		isSetNickname : 1;					// �j�b�N�l�[���ݒ�ς݁H
		u32		isSetUserColor : 1;					// ���[�U�[�J���[�ݒ�ς݁H
		u32		isSetBirthday : 1;					// �a�����ݒ�ς݁H
		u32		isSetTP : 1;						// TP�ݒ�ς݁H
		u32		isSetParentalControl : 1;			// �p�����^���R���g���[���ݒ�ς݁H
//		u32		isSetBrowserRestriction : 1;		// Wii�ő��݁B�t���u���E�U�𐧌����邩�ǂ����BTWL�ł͌������B
		u32		isAgreeEURA : 1;					// EURA���Ӎς݁H
		// WiFi�ݒ�͕ʃf�[�^�Ȃ̂ŁA�����ɐݒ�ς݃t���O�͗p�ӂ��Ȃ��B
		u32		isGBUseTopLCD : 1;					// �P��ʂ�GB�Q�[�����ɏ��ʂ��g���H
		u32		isAvailableWireless : 1;            // �������W���[����RF���j�b�g�̗L�����^������
		u32		isAvailableBatteryExtension : 1;    // �o�b�e���G�N�X�e���V�������[�h�̗L�����^������
		u32		rsv : 19;
	}flags;
	u8					rsv2[ 3 ];					// �\��
	u8					country;					// ���R�[�h
	u8					language;					// ����(NTR�Ƃ̈Ⴂ�́A�f�[�^�T�C�Y8bit)
	u8					backLightBrightness;		// �o�b�N���C�g�P�x(NTR�Ƃ̈Ⴂ�́A�f�[�^�T�C�Y8bit)
	u8					freeSoftBoxCount;			// �C���X�g�[���\��NAND�A�v����
	u8					rtcLastSetYear;				// RTC�̑O��ݒ�N
	s64					rtcOffset;					// RTC�ݒ莞�̃I�t�Z�b�g�l�i���[�U�[��RTC�ݒ��ύX����x�ɂ��̒l�ɉ����đ������܂��B�j
	TWLOwnerInfo		owner;						// �I�[�i�[���
	TWLAlarm			alarm;						// �A���[��
	TWLTPCalibData		tp;							// �^�b�`�p�l���L�����u���[�V�����f�[�^
	TWLParentalControl	parental;
}TWLSettingsData;	// xxbyte


#ifdef SDK_ARM9

//=========================================================
// �O���[�o���ϐ�
//=========================================================
extern TWLSettingsData s_settings;
#define GetTSD()		( &s_settings )

//=========================================================
// NAND�t�@�C���ւ̃��[�h���C�g�֐�
//=========================================================
	// �����ϐ��ւ̃��[�h
extern BOOL TSD_ReadSettings( void );
	// �����ϐ��̒l�̃��C�g�i��Ƀ��[�h���Ă����K�v������j
extern BOOL TSD_WriteSettings( void );
	// ���ڒl���w�肵�Ẵ��C�g�i�J���p�j
extern BOOL TSD_WriteSettingsDirect( const TWLSettingsData *pSrc );
	// �����ϐ��̒l�̃N���A
extern void TSD_ClearSettings( void );


//=========================================================
// �f�[�^�擾�iTSD_ReadSettings�œ������[�N�ɓǂݏo�������̎擾�j
//=========================================================

// ���R�[�h�̎擾�B
static inline TWLCountryCode TSD_GetCountry( void )
{
	return	(TWLCountryCode)GetTSD()->country;
}

// ����R�[�h�̎擾
static inline TWLLangCode TSD_GetLanguage( void )
{
  	return	(TWLLangCode)GetTSD()->language;
}

// �o�b�N���C�g�P�x�擾
static inline int TSD_GetBacklightBrightness( void )
{
	return	(int)GetTSD()->backLightBrightness;
}

// �t���[�\�t�gBOX���̎擾
static inline u8 TSD_GetFreeSoftBoxCount( void )
{
  	return	GetTSD()->freeSoftBoxCount;
}

// RTC�̑O��Z�b�g�����N�̎擾
static inline u8 TSD_GetRTCLastSetYear( void )
{
	return	GetTSD()->rtcLastSetYear;
}

// RTC�I�t�Z�b�g�l�̎擾
static inline s64 TSD_GetRTCOffset( void )
{
	return	GetTSD()->rtcOffset;
}

// �I�[�i�[���S�̂̎擾�B
static inline void TSD_GetOwnerInfo( TWLOwnerInfo *pDst )
{
	MI_CpuCopy8( &GetTSD()->owner, pDst, sizeof(TWLOwnerInfo) );
}

// �I�[�i�[���S�̂ւ̃|�C���^�̎擾�B
static inline const TWLOwnerInfo *TSD_GetOwnerInfoPtr( void )
{
	return	(const TWLOwnerInfo *)&GetTSD()->owner;
}

// �D���ȐF�̎擾�B
static inline u8 TSD_GetUserColor( void )
{
	return	(u8)GetTSD()->owner.userColor;
}

// �a�����̎擾�B
static inline void TSD_GetBirthday( TWLDate *pDst )
{
	MI_CpuCopy8( &GetTSD()->owner.birthday, pDst, sizeof(TWLDate) );
}

// �a�����ւ̃|�C���^�̎擾�B
static inline const TWLDate *TSD_GetBirthdayPtr( void )
{
	return	(const TWLDate *)&GetTSD()->owner.birthday;
}

// �j�b�N�l�[���̎擾�B
static inline void TSD_GetNickname( u16 *pDst )
{
	MI_CpuCopy16( GetTSD()->owner.nickname, pDst, TWL_NICKNAME_BUFFERSIZE );
}

// �j�b�N�l�[���ւ̃|�C���^�̎擾�B
static inline const u16 *TSD_GetNicknamePtr( void )
{
	return	(const u16 *)&GetTSD()->owner.nickname;
}

// �R�����g�̎擾�B
static inline void TSD_GetComment( u16 *pDst )
{
	MI_CpuCopy16( GetTSD()->owner.comment, pDst, TWL_COMMENT_BUFFERSIZE );
}

// �R�����g�ւ̃|�C���^�̎擾�B
static inline const u16 *TSD_GetCommentPtr( void )
{
	return	(const u16 *)&GetTSD()->owner.comment;
}

// �A���[�����̎擾�B
static inline void TSD_GetAlarmData( TWLAlarm *pAlarm )
{
	MI_CpuCopy8( &GetTSD()->alarm, pAlarm, sizeof(TWLAlarm) );
}

// �A���[�����ւ̃|�C���^�̎擾�B
static inline const TWLAlarm *TSD_GetAlarmDataPtr( void )
{
	return	(const TWLAlarm *)&GetTSD()->alarm;
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�̎擾�B
static inline void TSD_GetTPCalibration( TWLTPCalibData *pDst )
{
	MI_CpuCopy8( &GetTSD()->tp, pDst, sizeof(TWLTPCalibData) );
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�ւ̃|�C���^�̎擾�B
static inline const TWLTPCalibData *TSD_GetTPCalibrationPtr( void )
{
	return	(const TWLTPCalibData *)&GetTSD()->tp;
}

// ����N���V�[�P���X���H
static inline BOOL TSD_IsInitialSequence( void )
{
	return	(BOOL)GetTSD()->flags.initialSequence;
}

// ���R�[�h���͍ς݁H
static inline BOOL TSD_IsSetCountry( void )
{
	return	(BOOL)GetTSD()->flags.isSetCountry;
}

// ����R�[�h���͍ς݁H
static inline BOOL TSD_IsSetLanguage( void )
{
	return	(BOOL)GetTSD()->flags.isSetLanguage;
}

// ���t�E�����f�[�^���͍ς݁H
static inline BOOL TSD_IsSetDateTime( void )
{
	return	(BOOL)GetTSD()->flags.isSetDateTime;
}

// �j�b�N�l�[�����͍ς݁H
static inline BOOL TSD_IsSetNickname( void )
{
	return	(BOOL)GetTSD()->flags.isSetNickname;
}

// ���[�U�[�J���[���͍ς݁H
static inline BOOL TSD_IsSetUserColor( void )
{
	return	(BOOL)GetTSD()->flags.isSetUserColor;
}

// �a�������͍ς݁H
static inline BOOL TSD_IsSetBirthday( void )
{
	return	(BOOL)GetTSD()->flags.isSetBirthday;
}

// TP�L�����u���[�V�����f�[�^���͍ς݁H
static inline BOOL TSD_IsSetTP( void )
{
	return	(BOOL)GetTSD()->flags.isSetTP;
}

// �p�����^���R���g���[�����͍ς݁H
static inline BOOL TSD_IsSetParentalControl( void )
{
	return	(BOOL)GetTSD()->flags.isSetParentalControl;
}

// EURA�r���[�A���Ӎς݁H
static inline BOOL TSD_IsAgreeEURA( void )
{
	return	(BOOL)GetTSD()->flags.isAgreeEURA;
}

// GB�A�v���ȂǂP��ʃ\�t�g���g���̂͏��ʁH
static inline BOOL TSD_IsGBUseTopLCD( void )
{
	return	(BOOL)GetTSD()->flags.isGBUseTopLCD;
}

// �����g�p�\�H
static inline BOOL TSD_IsAvailableWireless( void )
{
	return	(BOOL)GetTSD()->flags.isAvailableWireless;
}

// �o�b�e���G�N�X�e���V�������[�h�L���H
static inline BOOL TSD_IsAvailableBatteryExtension( void )
{
	return	(BOOL)GetTSD()->flags.isAvailableBatteryExtension;
}


//=========================================================
// �f�[�^�Z�b�g�iTSD_ReadSettings�œ������[�N�ɓǂݏo�������ւ̒l�Z�b�g�j
//=========================================================

// ���R�[�h�̃Z�b�g�B
static inline void TSD_SetCountry( TWLCountryCode country )
{
	GetTSD()->country = (u8)country;
}

// ����R�[�h�̃Z�b�g
static inline void TSD_SetLanguage( TWLLangCode language )
{
	GetTSD()->language = language;
}

// �o�b�N���C�g�P�x�����Z�b�g�B
static inline void TSD_SetBacklightBrightness( u8 backLightBrightness )
{
	GetTSD()->backLightBrightness = backLightBrightness;
}

// �t���[�\�t�gBOX���̃Z�b�g
static inline void TSD_SetFreeSoftBoxCount( u8 count )
{
	GetTSD()->freeSoftBoxCount = count;
}

// RTC��LastSetYear�ւ̃Z�b�g
static inline void TSD_SetRTCLastSetYear( u8 rtcLastSetYear )
{
	GetTSD()->rtcLastSetYear = rtcLastSetYear;
}

// RTC�I�t�Z�b�g�l�̃Z�b�g
static inline void TSD_SetRTCOffset( s64 rtcOffset )
{
	GetTSD()->rtcOffset = rtcOffset;
}

// �I�[�i�[���S�̂̃Z�b�g�B
static inline void TSD_SetOwnerInfo( const TWLOwnerInfo *pOwner )
{
	MI_CpuCopy16( pOwner, &GetTSD()->owner, sizeof(TWLOwnerInfo) );
}

// ���[�U�[�J���[�̃Z�b�g�B
static inline void TSD_SetUserColor( u8 userColor )
{
	GetTSD()->owner.userColor = userColor;
}

// �a�����̃Z�b�g�B
static inline void TSD_SetBirthday( const TWLDate *pSrc )
{
	MI_CpuCopy8( pSrc, &GetTSD()->owner.birthday, sizeof(TWLDate) );
}

// �j�b�N�l�[���̃Z�b�g�B
static inline void TSD_SetNickname( const u16 *pSrc )
{
	MI_CpuCopy16( pSrc, GetTSD()->owner.nickname, TWL_NICKNAME_BUFFERSIZE );
}

// �R�����g�̃Z�b�g�B
static inline void TSD_SetComment( const u16 *pSrc )
{
	MI_CpuCopy16( pSrc, GetTSD()->owner.comment, TWL_COMMENT_BUFFERSIZE );
}

// �A���[�����̃Z�b�g�B
static inline void TSD_SetAlarmData( const TWLAlarm *pAlarm )
{
	MI_CpuCopy16( pAlarm, &GetTSD()->alarm, sizeof(TWLAlarm) );
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�̃Z�b�g�B
static inline void TSD_SetTPCalibration( const TWLTPCalibData *pTPCalib )
{
	MI_CpuCopy16( pTPCalib, &GetTSD()->tp, sizeof(TWLTPCalibData) );
}

// ����N���V�[�P���X�����ǂ����̃t���O�Z�b�g�B
static inline void TSD_SetFlagInitialSequence( BOOL initialSequence )
{
	GetTSD()->flags.initialSequence = (u32)initialSequence;
}

// ���R�[�h�̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagCountry( BOOL set )
{
	GetTSD()->flags.isSetCountry = (u32)set;
}

// ����R�[�h�̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagLanguage( BOOL set )
{
	GetTSD()->flags.isSetLanguage = (u32)set;
}

// ���t�E�����̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagDateTime( BOOL set )
{
	GetTSD()->flags.isSetDateTime = (u32)set;
}

// �j�b�N�l�[���̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagNickname( BOOL set )
{
	GetTSD()->flags.isSetNickname = (u32)set;
}

// ���[�U�[�J���[�̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagUserColor( BOOL set )
{
	GetTSD()->flags.isSetUserColor = (u32)set;
}

// �a�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagBirthday( BOOL set )
{
	GetTSD()->flags.isSetBirthday = (u32)set;
}

// TP�L�����u���[�V�����̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagTP( BOOL set )
{
	GetTSD()->flags.isSetTP = (u32)set;
}

// �p�����^���R���g���[���̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagParentalControl( BOOL set )
{
	GetTSD()->flags.isSetNickname = (u32)set;
}

// EURA�r���[�A���Ӎς݃t���O�Z�b�g�B
static inline void TSD_SetFlagAgreeEURA( BOOL set )
{
	GetTSD()->flags.isSetNickname = (u32)set;
}

// GB�A�v���ȂǂP��ʃ\�t�g���g����ʃt���O���Z�b�g
static inline void TSD_SetFlagGBUseTopLCD( BOOL set )
{
	GetTSD()->flags.isGBUseTopLCD = (u32)set;
}

// �����g�p�ۃt���O���Z�b�g
static inline void TSD_SetFlagAvailableWireless( BOOL set )
{
	GetTSD()->flags.isAvailableWireless = set;
}

// �o�b�e���G�N�X�e���V�������[�h�L���^�����t���O���Z�b�g
static inline void TSD_SetFlagAvailableBatteryExtension( BOOL set )
{
	GetTSD()->flags.isAvailableBatteryExtension = set;
}


#endif // SDK_ARM9



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// TWL_SETTINGS_H_
