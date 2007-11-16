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
#define TWL_DEFAULT_REGION				0							// �f�t�H���g�̃��[�W�����@����H�H
#define TWL_SETTINGS_DATA_VERSION		1							// TWL�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����(�J�nNo.:1)
#define TWL_NICKNAME_LENGTH				NTR_NICKNAME_LENGTH			// �j�b�N�l�[����
#define TWL_NICKNAME_BUFFERSIZE			( ( TWL_NICKNAME_LENGTH + 1 ) * 2 )	// �j�b�N�l�[���o�b�t�@�T�C�Y
#define TWL_COMMENT_LENGTH				NTR_COMMENT_LENGTH			// �R�����g��
#define TWL_COMMENT_BUFFERSIZE			( ( TWL_COMMENT_LENGTH + 1 ) * 2 )	// �R�����g�o�b�t�@�T�C�Y
#define TWL_FAVORITE_COLOR_MAX_NUM		NTR_FAVORITE_COLOR_MAX_NUM	// �D���ȐF�̍ő吔
#define TSD_TEMP_BUFFER_SIZE			( sizeof(TSDStore) * 2 )	// TSD_ReadTWLSettings�ŕK�v��TempBuffer�T�C�Y


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

#define TWL_LANG_CODE_MAX_WW		TWL_LANG_SPANISH

// NTR ����Ƃ̑Ή��}�b�v
#if 0
const u8 LangCodeMapFromTWLtoNTR[][ 2 ] = {
	{ TWL_LANG_JAPANESE,			NTR_LANG_JAPANESE	},
	{ TWL_LANG_ENGLISH,				NTR_LANG_ENGLISH	},
	{ TWL_LANG_FRENCH,				NTR_LANG_FRENCH		},
	{ TWL_LANG_GERMAN,				NTR_LANG_GERMAN		},
	{ TWL_LANG_ITALIAN,				NTR_LANG_ITALIAN	},
	{ TWL_LANG_SPANISH,				NTR_LANG_SPANISH	},
	{ TWL_LANG_SIMP_CHINESE,		NTR_LANG_CHINESE	},
	{ TWL_LANG_KOREAN,				NTR_LANG_KOREAN		},
//	{ TWL_LANG_DUTCH,				NTR_LANG_ENGLISH	},
//	{ TWL_LANG_TRAD_CHINESE,		NTR_LANG_CHINESE	}
};
#endif

#define TWL_LANG_BITMAP_JAPAN		( ( 0x0001 << TWL_LANG_JAPANESE ) ) 	// JPN�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_AMERICA		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// AME�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_EUROPE		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_GERMAN  ) | \
									  ( 0x0001 << TWL_LANG_ITALIAN  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// EUR�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_AUSTRALIA	( ( 0x0001 << TWL_LANG_ENGLISH  ) ) 	// AUS�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_CHINA		( ( 0x0001 << TWL_LANG_SIMP_CHINESE ) ) // CHI�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_KOREA		( ( 0x0001 << TWL_LANG_KOREAN ) ) 		// KOR�łł̑Ή�����r�b�g�}�b�v


// ���[�W�����R�[�h�i�̎ЕʂɂȂ錩���݁j
typedef enum TWLRegionCode {
	TWL_REGION_JAPAN     = 0,   // NCL
	TWL_REGION_AMERICA   = 1,   // NOA
	TWL_REGION_EUROPE    = 2,   // NOE
	TWL_REGION_AUSTRALIA = 3,   // NAL
	TWL_REGION_CHINA     = 4,   // IQue
	TWL_REGION_KOREA     = 5,   // NOK
	TWL_REGION_MAX       = 5
}TWLRegion;


// ���t
#define TWLDate						NTRDate

// �A���[��
#define TWLAlarm					NTRAlarm

// TP�L�����u���[�V�����iNTR�Ƃ̈Ⴂ�́A�\��̈悠��j
typedef struct TWLTPCalibData {
	NTRTPCalibData	data;
	u8				rsv[ 8 ];
}TWLTPCalibData;

// �j�b�N�l�[���iNTR�Ƃ̈Ⴂ�́A������ɏI�[����j
typedef struct TWLNickname{
	u16				buffer[ TWL_NICKNAME_LENGTH + 1 ];	// �j�b�N�l�[���iUnicode(UTF16)�ōő�10�����A�I�[�R�[�h����j
	u8				length;							// ������
	u8				rsv;
}TWLNickname;		// 24byte

// �R�����g�iNTR�Ƃ̈Ⴂ�́A������ɏI�[����j
typedef struct TWLComment{
	u16				buffer[ TWL_COMMENT_LENGTH + 1 ];	//�R�����g�iUnicode(UTF16)�ōő�26�����A�I�[�R�[�h����j
	u8				length;							// ������
	u8				rsv;
}TWLComment;		// 54byte

// �I�[�i�[���
typedef struct TWLOwnerInfo{
	u8				userColor : 4;				// �D���ȐF
	u8				rsv : 4;					// �\��B
	TWLDate			birthday;					// ���N����
	u8				pad;
	TWLNickname		nickname;					// �j�b�N�l�[��
	TWLComment		comment;					// �R�����g
}TWLOwnerInfo;		// 80byte


// �y�A�����^���R���g���[��
#define TWL_PARENTAL_CONTROL_PASSWORD_LENGTH     4                  // �Ïؔԍ��̌���
#define TWL_PARENTAL_CONTROL_PASSWORD_DEFAULT    "0000"             // �f�t�H���g�Ïؔԍ�
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MIN 6             // �閧�̎���̉񓚁AUTF-16�ōŏ� MIN ����
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX 32            // �閧�̎���̉񓚁AUTF-16�ōő� MAX ����

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

typedef struct TWLParentalControl {
	TWLRatingOgn	ogn;				// �R���c��
//	u8				flags;				// Wii�ł́APARENTAL_CONTROL_USE�t���O�̂� --> isSetParentalControl ������̂Ō���K�v�Ȃ�
	u8				rating;				// ���[�e�B���O�i�N��j�l
	char			password[ TWL_PARENTAL_CONTROL_PASSWORD_LENGTH + 1 ];   // �Ïؔԍ��A�I�[�R�[�h����
	u8				secretQuestion;     // �閧�̎��╶ ID
	u8				rsv_A;
	u16				secretAnswer[ TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX + 1 ];  // UTF16,�閧�̎���ւ̉񓚁A�I�[�R�[�h����
	u16				secretAnswerLength; // �閧�̎���ւ̉񓚕������iWii��u16,LENGTH_MAX ���ێ��ł��邩��u8�ł������̂ł́H�j
//	u8				rsv_B[ 16 ]; 		// �폜�\��i16�o�C�g�̃��[�e�B���O�������̂̓A�v������ROM�w�b�_�j
}TWLParentalControl;


// �C���X�g�[���E�\�t�g��
#define TWL_FREE_SOFT_BOX_COUNT_MAX  35     // NAND�A�v���̍ő��Box��, ���� - freeSoftBoxCount�ŃC���X�g�[��Soft��


// TWL�ݒ�f�[�^�w�b�_
typedef struct TWLSettingsHeader{
	u8					version;
	u8					saveCount;
	u16					dataLength;
	u8					digest[ SVC_SHA1_DIGEST_SIZE ];				// SHA1�_�C�W�F�X�g�@�@CRC16�ŏ\�������Ȃ��B�B�B
}TWLSettingsHeader;


// TWL�ݒ�f�[�^�i��{�A�ߋ�ver�݊����l�����āA�ǉ��������Ȃ����j�ŁB�j
typedef struct TWLSettingsData{
	struct flags {
		u32		initialSequence : 1;
		u32		isSetCountry : 1;
		u32		isSetLanguage : 1;
		u32		isSetDateTime : 1;
		u32		isSetNickname : 1;
		u32		isSetUserColor : 1;
		u32		isSetBirthday : 1;
		u32		isSetTP : 1;
		u32		isSetParentalControl : 1;
//		u32		isSetBrowserRestriction : 1;    // Wii�ő��݁B�t���u���E�U�𐧌����邩�ǂ����BTWL�ł͌������B
		u32		isAgreeEURA : 1;
		// WiFi�ݒ�͕ʃf�[�^�Ȃ̂ŁA�����ɐݒ�ς݃t���O�͗p�ӂ��Ȃ��B
		u32		isGBUseTopLCD : 1;
		u32		isAvailableWireless : 1;            // �������W���[����RF���j�b�g�̗L�����^������
		u32		isAvailableBatteryExtension : 1;    // �o�b�e���G�N�X�e���V�������[�h�̗L�����^������
		u32		rsv : 19;
	}flags;
	u16					valid_language_bitmap;		// �Ή�����r�b�g�}�b�v�i����������Ȃ��A"/sys/HWINFO.dat"���̕����ǂ������j
	TWLCountryCode		country;					// ���R�[�h
	u8					region;						// ���[�W�����i����������Ȃ��A"/sys/HWINFO.dat"���̕����ǂ������j
	u8					language;					// ����(NTR�Ƃ̈Ⴂ�́A�f�[�^�T�C�Y8bit)
	u8					backLightBrightness;		// �o�b�N���C�g�P�x(NTR�Ƃ̈Ⴂ�́A�f�[�^�T�C�Y8bit)
	u8					rtcLastSetYear;				// RTC�̑O��ݒ�N
	s64					rtcOffset;					// RTC�ݒ莞�̃I�t�Z�b�g�l�i���[�U�[��RTC�ݒ��ύX����x�ɂ��̒l�ɉ����đ������܂��B�j
	TWLOwnerInfo		owner;						// �I�[�i�[���
	TWLAlarm			alarm;						// �A���[��
	TWLTPCalibData		tp;							// �^�b�`�p�l���L�����u���[�V�����f�[�^
	TWLParentalControl	parental;
	u8					freeSoftBoxCount;			// �C���X�g�[���\��NAND�A�v����
}TWLSettingsData;	// xxbyte


// TWL�ݒ�f�[�^�ۑ��t�H�[�}�b�g
typedef struct TSDStore {
	TWLSettingsHeader	header;
	TWLSettingsData		tsd;
}TSDStore;


#ifdef SDK_ARM9

//=========================================================
// �O���[�o���ϐ�
//=========================================================
extern TWLSettingsData   *g_pTSD;
#define GetTSD()		( g_pTSD )

//=========================================================
// NAND�t�@�C���ւ̃��[�h���C�g�֐�
//=========================================================
extern BOOL TSD_IsReadSettings( void );
extern BOOL TSD_ReadSettings( TSDStore (*pTempBuffer)[2] );	// TSD_TEMP_BUFFER_SIZE��pTempBuffer���K�v�B
extern BOOL TSD_WriteSettings( void );						// ���NSD_ReadSettings�����s���Ă����K�v������B

//=========================================================
// �f�[�^�擾�iTSD_ReadSettings�œ������[�N�ɓǂݏo�������̎擾�j
//=========================================================

// ���[�W�����̎擾�B
static inline u8 TSD_GetRegion( void )
{
	return	(u8)GetTSD()->region;
}

// �I�[�i�[���S�̂̎擾�B
static inline TWLOwnerInfo *TSD_GetOwnerInfo( void )
{
	return	&GetTSD()->owner;
}

// �D���ȐF�̎擾�B
static inline u8 TSD_GetUserColor( void )
{
	return	(u8)GetTSD()->owner.userColor;
}

// �a�����̎擾�B
static inline TWLDate *TSD_GetBirthday( void )
{
	return	&GetTSD()->owner.birthday;
}

// �j�b�N�l�[���̎擾�B
static inline TWLNickname *TSD_GetNickname( void )
{
	return	&GetTSD()->owner.nickname;
}

// �R�����g�̎擾�B
static inline TWLComment *TSD_GetComment( void )
{
	return	&GetTSD()->owner.comment;
}

// �A���[�����̎擾�B
static inline TWLAlarm *TSD_GetAlarmData( void )
{
	return	&GetTSD()->alarm;
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�̎擾�B
static inline TWLTPCalibData *TSD_GetTPCalibration( void )
{
	return	&GetTSD()->tp;
}

// ����R�[�h�̎擾
static inline TWLLangCode TSD_GetLanguage( void )
{
  	return	(TWLLangCode)GetTSD()->language;
}

// �Ή�����r�b�g�}�b�v�̎擾
static inline u16 TSD_GetLanguageBitmap( void )
{
  	return	GetTSD()->valid_language_bitmap;
}

// RTC�I�t�Z�b�g�l�̎擾
static inline s64 TSD_GetRTCOffset( void )
{
	return	GetTSD()->rtcOffset;
}

// RTC�̑O��Z�b�g�����N�̎擾
static inline u8 TSD_GetRTCLastSetYear( void )
{
	return	GetTSD()->rtcLastSetYear;
}

// �o�b�N���C�g�P�x�擾
static inline int TSD_GetBacklightBrightness( void )
{
	return	(int)GetTSD()->backLightBrightness;
}

// ����N���V�[�P���X���H
static inline BOOL TSD_IsInitialSequence( void )
{
	return	(BOOL)GetTSD()->flags.initialSequence;
}

// �a�������͍ς݁H
static inline BOOL TSD_IsSetBirthday( void )
{
	return	(BOOL)GetTSD()->flags.isSetBirthday;
}

// ���[�U�[�J���[���͍ς݁H
static inline BOOL TSD_IsSetUserColor( void )
{
	return	(BOOL)GetTSD()->flags.isSetUserColor;
}

// TP�L�����u���[�V�����f�[�^���͍ς݁H
static inline BOOL TSD_IsSetTP( void )
{
	return	(BOOL)GetTSD()->flags.isSetTP;
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


//=========================================================
// �f�[�^�Z�b�g�iTSD_ReadSettings�œ������[�N�ɓǂݏo�������ւ̒l�Z�b�g�j
//=========================================================

// ���[�W�����̃Z�b�g�B
static inline void TSD_SetRegion( u8 region )
{
	GetTSD()->region = region;
}

// �I�[�i�[���S�̂̃Z�b�g�B
static inline void TSD_SetOwnerInfo( TWLOwnerInfo *pOwner )
{
	MI_CpuCopy16( pOwner, &GetTSD()->owner, sizeof(TWLOwnerInfo) );
}

// ���[�U�[�J���[�̃Z�b�g�B
static inline void TSD_SetUserColor( u8 userColor )
{
	GetTSD()->owner.userColor = userColor;
}

// �a�����̃Z�b�g�B
static inline void TSD_SetBirthday( TWLDate *pBirth )
{
	GetTSD()->owner.birthday.month	= pBirth->month;
	GetTSD()->owner.birthday.day	= pBirth->day;
}

// �j�b�N�l�[���̃Z�b�g�B
static inline void TSD_SetNickname( TWLNickname *pNickname )
{
	MI_CpuCopy16( pNickname, &GetTSD()->owner.nickname, sizeof(TWLNickname) );
}

// �R�����g�̃Z�b�g�B
static inline void TSD_SetComment( TWLComment *pComment )
{
	MI_CpuCopy16( pComment, &GetTSD()->owner.comment, sizeof(TWLComment) );
}

// �A���[�����̃Z�b�g�B
static inline void TSD_SetAlarmData( TWLAlarm *pAlarm )
{
	MI_CpuCopy16( pAlarm, &GetTSD()->alarm, sizeof(TWLAlarm) );
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�̃Z�b�g�B
static inline void TSD_SetTPCalibration( TWLTPCalibData *pTPCalib )
{
	MI_CpuCopy16( pTPCalib, &GetTSD()->tp, sizeof(TWLTPCalibData) );
}

// ����R�[�h�̃Z�b�g
static inline void TSD_SetLanguage( TWLLangCode language )
{
	GetTSD()->language = language;
}

// �Ή�����r�b�g�}�b�v�̃Z�b�g
static inline void TSD_SetLanguageBitmap( u16 valid_language_bitmap )
{
	GetTSD()->valid_language_bitmap = valid_language_bitmap;
}

// RTC�I�t�Z�b�g�l�̃Z�b�g
static inline void TSD_SetRTCOffset( s64 rtcOffset )
{
	GetTSD()->rtcOffset = rtcOffset;
}

// RTC��LastSetYear�ւ̃Z�b�g
static inline void TSD_SetRTCLastSetYear( u8 rtcLastSetYear )
{
	GetTSD()->rtcLastSetYear = rtcLastSetYear;
}

// �o�b�N���C�g�P�x�����Z�b�g�B
static inline void TSD_SetBacklightBrightness( u8 backLightBrightness )
{
	GetTSD()->backLightBrightness = backLightBrightness;
}

// �t���b�V�����V�[�P���X�����ǂ����̃t���O�Z�b�g�B
static inline void TSD_SetFlagInitialSequence( BOOL initialSequence )
{
	GetTSD()->flags.initialSequence = (u32)initialSequence;
}

// �a�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagBirthday( BOOL set )
{
	GetTSD()->flags.isSetBirthday = (u32)set;
}

// ���[�U�[�J���[�̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagUserColor( BOOL set )
{
	GetTSD()->flags.isSetUserColor = (u32)set;
}

// TP�L�����u���[�V�����̓��͍ς݃t���O�Z�b�g�B
static inline void TSD_SetFlagTP( BOOL set )
{
	GetTSD()->flags.isSetTP = (u32)set;
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


#endif // SDK_ARM9



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// TWL_SETTINGS_H_
