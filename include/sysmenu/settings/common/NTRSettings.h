/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     NTRSettings.c

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


#ifndef	NTR_SETTINGS_H_
#define	NTR_SETTINGS_H_
#if		defined(SDK_CW)							// NTRConfigData�Ƀr�b�g�t�B�[���h���g���Ă���̂ŁA�R���p�C���ˑ��ŕs�����������\��������B
												// ����āACW�ȊO�̃R���p�C���̏ꍇ�́A���̃w�b�_�𖳌��ɂ��ăG���[���o������悤�ɂ��čĊm�F����B

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define NTR_SETTINGS_DATA_VERSION		5							// NTR�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����
#define NTR_SETTINGS_DATA_EX_VERSION	1							// �g��NTR�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����
#define NTR_NICKNAME_LENGTH				10							// �j�b�N�l�[����
#define NTR_NICKNAME_BUFFERSIZE			( NTR_NICKNAME_LENGTH * 2 )	// �j�b�N�l�[���o�b�t�@�T�C�Y
#define NTR_COMMENT_LENGTH				26							// �R�����g��
#define NTR_COMMENT_BUFFERSIZE			( NTR_COMMENT_LENGTH * 2 )
#define NTR_USER_COLOR_MAX_NUM			16							// ���[�U�[�J���[�̍ő吔
#define NSD_TEMP_BUFFER_SIZE			( sizeof(NSDStoreEx) * 2 )	// NSD_ReadSettings�ŕK�v��TempBuffer�T�C�Y

// ����ݒ�R�[�h
typedef enum NTRLangCode{
	NTR_LANG_JAPANESE = 0,						// ���{��
	NTR_LANG_ENGLISH  = 1,						// �p��
	NTR_LANG_FRENCH   = 2,						// �t�����X��
	NTR_LANG_GERMAN   = 3,						// �h�C�c��
	NTR_LANG_ITALIAN  = 4,						// �C�^���A��
	NTR_LANG_SPANISH  = 5,						// �X�y�C����
	NTR_LANG_CHINESE  = 6,						// ������
	NTR_LANG_KOREAN   = 7,						// �؍���
	NTR_LANG_CODE_MAX
}NTRLangCode;

#define NTR_LANG_CODE_MAX_WW		( NTR_LANG_SPANISH + 1 )

#define NTR_LANG_BITMAP_WW			( ( 0x0001 << NTR_LANG_JAPANESE ) | \
									  ( 0x0001 << NTR_LANG_ENGLISH  ) | \
									  ( 0x0001 << NTR_LANG_FRENCH   ) | \
									  ( 0x0001 << NTR_LANG_GERMAN   ) | \
									  ( 0x0001 << NTR_LANG_ITALIAN  ) | \
									  ( 0x0001 << NTR_LANG_SPANISH  ) ) 	// SystemMenu-WW�łł̑Ή�����r�b�g�}�b�v


#define NTR_LANG_BITMAP_CHINA		( ( 0x0001 << NTR_LANG_CHINESE ) | \
									  ( 0x0001 << NTR_LANG_ENGLISH  ) | \
									  ( 0x0001 << NTR_LANG_FRENCH   ) | \
									  ( 0x0001 << NTR_LANG_GERMAN   ) | \
									  ( 0x0001 << NTR_LANG_ITALIAN  ) | \
									  ( 0x0001 << NTR_LANG_SPANISH  ) ) 	// SystemMenu-CN�łł̑Ή�����r�b�g�}�b�v


#define NTR_LANG_BITMAP_KOREA		( ( 0x0001 << NTR_LANG_KOREAN  ) | \
									  ( 0x0001 << NTR_LANG_JAPANESE ) | \
									  ( 0x0001 << NTR_LANG_ENGLISH  ) | \
									  ( 0x0001 << NTR_LANG_FRENCH   ) | \
									  ( 0x0001 << NTR_LANG_GERMAN   ) | \
									  ( 0x0001 << NTR_LANG_SPANISH  ) ) 	// SystemMenu-KR�łł̑Ή�����r�b�g�}�b�v


// ���t�f�[�^
typedef struct NTRDate{
	u8				month;						// ��:01�`12
	u8				day;						// ��:01�`31
}NTRDate;			// 2byte

// �j�b�N�l�[��
typedef struct NTRNickname{
	u16				buffer[ NTR_NICKNAME_LENGTH ];	// �j�b�N�l�[���iUnicode(UTF16)�ōő�10�����A�I�[�R�[�h�Ȃ��j
	u8				length;						// ������
	u8				rsv;
}NTRNickname;		// 22byte

// �R�����g
typedef struct NTRComment{
	u16				buffer[ NTR_COMMENT_LENGTH ];	//�R�����g�iUnicode(UTF16)�ōő�26�����A�I�[�R�[�h�Ȃ��j
	u8				length;						// ������
	u8				rsv;
}NTRComment;		// 54byte

// �I�[�i�[���
typedef struct NTROwnerInfo{
	u8				userColor : 4;			// ���[�U�[�J���[
	u8				rsv : 4;					// �\��B
	NTRDate			birthday;					// ���N����
	u8				pad;
	NTRNickname		nickname;					// �j�b�N�l�[��
	NTRComment		comment;					// �R�����g
}NTROwnerInfo;		// 80byte

// IPL�p�ڊo�܂����v�f�[�^
typedef struct NTRAlarm{
	u8				hour;						// �A���[����:00�`23
	u8				minute;						// �A���[����:00�`59
	u8				rsv1;						// �\��
	u8				pad;
	u16				alarmOn : 1;				// �A���[��ON,OFF�i0:OFF, 1:ON�j
	u16				rsv2 : 15;					// �\��
}NTRAlarm;			// 6byte

// �^�b�`�p�l���L�����u���[�V�����f�[�^
typedef struct NTRTPCalibData{
	u16				raw_x1;						// ��P�L�����u���[�V�����_��TP�擾�lX
	u16				raw_y1;						// �@�@�@�@�V�@�@�@�@�@�@�@�@TP�擾�lY
	u8				dx1;						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W X
	u8				dy1;						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W Y
	u16				raw_x2;						// ��Q�L�����u���[�V�����_��TP�擾�lX
	u16				raw_y2;						// �@�@�@�@�V�@�@�@�@�@�@�@�@TP�擾�lY
	u8				dx2;   						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W X
	u8				dy2;   						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W Y
}NTRTPCalibData;	// 12byte

// �I�v�V�������
typedef struct NTROption{
	u16				language : 3;				// ����R�[�h�iNTR_LANG_SPANISH�܂ł̕W������R�[�h������j
	u16				isGBUseTopLCD : 1;			// AGB���[�h�ŋN�����鎞�ɂǂ����LCD�ŋN�����邩�H�i0:TOP,1:BOTTOM�j
	u16				backlightBrightness : 2;	// �o�b�N���C�g�P�x�f�[�^
	u16				isAutoBoot : 1;			// �N���V�[�P���X�ŁA���j���[��~�Ȃ��Ŏ����N�����邩�ǂ����H(0:OFF, 1:ON)
	u16				isBacklightOff : 1;			// �o�b�N���C�gON,OFF�t���O�i0:ON, 1:OFF�j
	u16				rsv2 : 1;					// �\��
	u16				initialSequence : 1;		// ����N���V�[�P���X���t���O
	u16				isSetBirthday : 1;			// �a���������͂��ꂽ���H
	u16				isSetUserColor : 1;		// ���[�U�[�J���[�����͂��ꂽ���H
	u16				isSetTP : 1;				// �^�b�`�p�l�����L�����u���[�V�������ꂽ���H�i  �V  )
	u16				isSetLanguage : 1;			// ������͂����ꂽ���H�@		(0:���ݒ�, 1:�ݒ�ς�)
	u16				isSetDateTime : 1;			// ���t�E�����ݒ肪���ꂽ���H	(		�V�@�@�@�@�@ )
	u16				isSetNickname : 1;			// �j�b�N�l�[�������͂��ꂽ���H	(		�V�@�@�@�@�@ )
	u8				rtcLastSetYear;				// RTC�̑O��ݒ�N
	u8				rtcClockAdjust;				// RTC�N���b�N�����l
	s64				rtcOffset;					// RTC�ݒ莞�̃I�t�Z�b�g�l�i���[�U�[��RTC�ݒ��ύX����x�ɂ��̒l�ɉ����đ������܂��B�j
}NTROption;			// 12byte


// NTR�e��ݒ�f�[�^
typedef struct NTRSettingsData{
	u8				version;					// �t���b�V���i�[�f�[�^�t�H�[�}�b�g�̃o�[�W����
	u8				pad;
	NTROwnerInfo	owner;						// �I�[�i�[���
	NTRAlarm		alarm;						// IPL�p�ڊo�܂����v�f�[�^
	NTRTPCalibData	tp;							// �^�b�`�p�l���L�����u���[�V�����f�[�^
	NTROption		option;						// �I�v�V����
}NTRSettingsData;	// 112byte


// �g��NTR�ݒ�f�[�^
typedef struct NTRSettingsDataEx{
	u8				version;					// �o�[�W����
	u8				language;					// ����R�[�h�iNTR_LANG_CHINESE�ȍ~�Ɋg�����ꂽ�l������B�j
	u16				valid_language_bitmap;		// �{IPL2�ŗL���Ȍ���R�[�h���������r�b�g�}�b�v
	u8				pad[ 256 - sizeof(NTRSettingsData) - 4 - 4 - 2 ];		// 4:saveCount+crc16, 2:NSDEx.version+NSDEx.language, 2:crc16_ex
}NTRSettingsDataEx;	// 138bytes


// NTR�e��ݒ�f�[�^��NVRAM�ۑ����t�H�[�}�b�g
typedef struct NSDStore{
	NTRSettingsData nsd;				// NTR�e��ݒ�f�[�^
	u16				saveCount;			// 0x00-0x7f�����[�v���ăJ�E���g���A�J�E���g�l���V�����f�[�^���L���B
	u16				crc16;				// NTR�e��ݒ�f�[�^��16bitCRC
	u8				pad[ 128 - sizeof(NTRSettingsData) - 4];
}NSDStore;			// 128byte			// ���{���Ȃ�AsaveCount��crc16��256byte�̍Ō�ɕt�����āA�ԂɃp�f�B���O�𖄂߂���������B


// NTR�e��ݒ�f�[�^EX��NVRAM�ۑ����t�H�[�}�b�g�i��LNCDStore�ƌ݊����Ƃ邽�߂̖������g���j
typedef struct NSDStoreEx{
	NTRSettingsData	nsd;				// NTR�e��ݒ�f�[�^
	u16					saveCount;		// 0x00-0x7f�����[�v���ăJ�E���g���A�J�E���g�l���V�����f�[�^���L���B
	u16					crc16;			// NTR�e��ݒ�f�[�^��16bitCRC
	NTRSettingsDataEx	nsd_ex;
	u16					crc16_ex;
}NSDStoreEx;		// 256byte			// ���{���Ȃ�AsaveCount��crc16��256byte�̍Ō�ɕt�����āA�ԂɃp�f�B���O�𖄂߂���������B


#ifdef SDK_ARM9

//=========================================================
// �O���[�o���ϐ�
//=========================================================
extern NTRSettingsData   *g_pNSD;
extern NTRSettingsDataEx *g_pNSDEx;
#define GetNSD()		( g_pNSD )
#define GetNSDEx()		( g_pNSDEx )

//=========================================================
// NVRAM�ւ̃��[�h���C�g�֐�
//=========================================================
extern void NSD_ClearSettings( void );
extern BOOL NSD_IsReadSettings( void );
extern BOOL NSD_ReadSettings( u8 region, NSDStoreEx (*pTempBuffer)[2] );	// NSD_TEMP_BUFFER_SIZE��pTempBuffer���K�v�B
extern BOOL NSD_WriteSettings( u8 region );									// ���NSD_ReadSettings�����s���Ă����K�v������B

//=========================================================
// �f�[�^�擾�iNSD_ReadSettings�œ������[�N�ɓǂݏo�������̎擾�j
//=========================================================

// �o�[�W�����̎擾�B
static inline u8 NSD_GetVersion( void )
{
	return	(u8)GetNSD()->version;
}

// EX�o�[�W�����̎擾�B
static inline u8 NSD_GetExVersion( void )
{
	return	(u8)GetNSDEx()->version;
}

// �I�[�i�[���S�̂ւ̃|�C���^�̎擾�B
static inline NTROwnerInfo *NSD_GetOwnerInfoPtr( void )
{
	return	&GetNSD()->owner;
}

// �I�[�i�[���S�̂̎擾�B
static inline void NSD_GetOwnerInfo( NTROwnerInfo *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner, pDst, sizeof(NTROwnerInfo) );
}

// ���[�U�[�J���[�̎擾�B
static inline u8 NSD_GetUserColor( void )
{
	return	(u8)GetNSD()->owner.userColor;
}

// �a�����ւ̃|�C���^�̎擾�B
static inline NTRDate *NSD_GetBirthdayPtr( void )
{
	return	&GetNSD()->owner.birthday;
}

// �a�����̎擾�B
static inline void NSD_GetBirthday( NTRDate *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner.birthday, pDst, sizeof(NTRDate) );
}

// �j�b�N�l�[���ւ̃|�C���^�̎擾�B
static inline NTRNickname *NSD_GetNicknamePtr( void )
{
	return	&GetNSD()->owner.nickname;
}

// �j�b�N�l�[���̎擾�B
static inline void NSD_GetNickname( NTRNickname *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner.nickname, pDst, sizeof(NTRNickname) );
}

// �R�����g�ւ̃|�C���^�̎擾�B
static inline NTRComment *NSD_GetCommentPtr( void )
{
	return	&GetNSD()->owner.comment;
}

// �R�����g�̎擾�B
static inline void NSD_GetComment( NTRComment *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner.comment, pDst, sizeof(NTRComment) );
}

// �A���[�����ւ̃|�C���^�̎擾�B
static inline NTRAlarm *NSD_GetAlarmDataPtr( void )
{
	return	&GetNSD()->alarm;
}

// �A���[�����̎擾�B
static inline void NSD_GetAlarmData( NTRAlarm *pDst )
{
	MI_CpuCopy8( &GetNSD()->alarm, pDst, sizeof(NTRAlarm) );
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�ւ̃|�C���^�̎擾�B
static inline NTRTPCalibData *NSD_GetTPCalibrationPtr( void )
{
	return	&GetNSD()->tp;
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�̎擾�B
static inline void NSD_GetTPCalibration( NTRTPCalibData *pDst )
{
	MI_CpuCopy8( &GetNSD()->tp, pDst, sizeof(NTRTPCalibData) );
}

// ����R�[�h�̎擾
static inline NTRLangCode NSD_GetLanguage( void )
{
  	return	(NTRLangCode)GetNSD()->option.language;
}

static inline NTRLangCode NSD_GetLanguageEx( void )
{
	return	(NTRLangCode)GetNSDEx()->language;
}

// �Ή�����r�b�g�}�b�v�̎擾
static inline u16 NSD_GetLanguageBitmap( void )
{
  	return	GetNSDEx()->valid_language_bitmap;
}

// RTC�I�t�Z�b�g�l�̎擾
static inline s64 NSD_GetRTCOffset( void )
{
	return	GetNSD()->option.rtcOffset;
}

// RTC�N���b�N�����l�̎擾
static inline u8 NSD_GetRTCClockAdjust( void )
{
	return	GetNSD()->option.rtcClockAdjust;
}

// RTC�̑O��Z�b�g�����N�̎擾
static inline u8 NSD_GetRTCLastSetYear( void )
{
	return	GetNSD()->option.rtcLastSetYear;
}

// �N���V�[�P���X�̎����N��ON���H�i0:OFF, 1:ON�j
static inline int NSD_IsAutoBoot( void )
{
	return	(int)GetNSD()->option.isAutoBoot;
}

// �o�b�N���C�g�P�x�擾�i0-3�j
static inline BOOL NSD_IsBacklightOff( void )
{
	return	(BOOL)GetNSD()->option.isBacklightOff;
}

// �o�b�N���C�g�P�x�擾�i0-3�j
static inline int NSD_GetBacklightBrightness( void )
{
	return	(int)GetNSD()->option.backlightBrightness;
}

// �t���b�V�����V�[�P���X�����ǂ����H
static inline BOOL NSD_IsInitialSequence( void )
{
	return	(int)GetNSD()->option.initialSequence;
}

// �a�����f�[�^���Z�b�g����Ă��邩�H
static inline BOOL NSD_IsSetBirthday( void )
{
	return	(int)GetNSD()->option.isSetBirthday;
}

// ���[�U�[�J���[�f�[�^���Z�b�g����Ă��邩�H
static inline int NSD_IsSetUserColor( void )
{
	return	(int)GetNSD()->option.isSetUserColor;
}

// TP�L�����u���[�V�����f�[�^���Z�b�g����Ă��邩�H
static inline int NSD_IsSetTP( void )
{
	return	(int)GetNSD()->option.isSetTP;
}

// ����R�[�h���Z�b�g����Ă��邩�H
static inline int NSD_IsSetLanguage( void )
{
	return	(int)GetNSD()->option.isSetLanguage;
}

// ���t�E�������Z�b�g����Ă��邩�H
static inline int NSD_IsSetDateTime( void )
{
	return	(int)GetNSD()->option.isSetDateTime;
}

// �I�[�i�[���̃j�b�N�l�[�����Z�b�g����Ă��邩�H
static inline int NSD_IsSetNickname( void )
{
	return	(int)GetNSD()->option.isSetNickname;
}

// GB�A�v���ȂǂP��ʃ\�t�g���g���̂͏��ʁH
static inline BOOL NSD_IsGBUseTopLCD( void )
{
	return	(BOOL)GetNSD()->option.isGBUseTopLCD;
}

//=========================================================
// �f�[�^�Z�b�g
//=========================================================

// �o�[�W�����̃Z�b�g�B
static inline void NSD_SetVersion( u8 version )
{
	GetNSD()->version = version;
}

// �o�[�W����EX�̃Z�b�g�B
static inline void NSD_SetExVersion( u8 version )
{
	GetNSDEx()->version = version;
}

// �I�[�i�[���̃N���A
extern void NSD_ClearOwnerInfo( void );

// �I�[�i�[���S�̂̃Z�b�g�B
static inline void NSD_SetOwnerInfo( const NTROwnerInfo *pOwner )
{
	MI_CpuCopy16( pOwner, &GetNSD()->owner, sizeof(NTROwnerInfo) );
}

// ���[�U�[�J���[�̃Z�b�g�B
static inline void NSD_SetUserColor( u8 userColor )
{
	GetNSD()->owner.userColor = userColor;
}

// �a�����̃Z�b�g�B
static inline void NSD_SetBirthday( const NTRDate *pBirthday )
{
	GetNSD()->owner.birthday.month	= pBirthday->month;
	GetNSD()->owner.birthday.day	= pBirthday->day;
}

// �j�b�N�l�[���̃Z�b�g�B
static inline void NSD_SetNickname( const NTRNickname *pName )
{
	MI_CpuCopy16( pName, &GetNSD()->owner.nickname, sizeof(NTRNickname) );
}

// �R�����g�̃Z�b�g�B
static inline void NSD_SetComment( const NTRComment *pComment )
{
	MI_CpuCopy16( pComment, &GetNSD()->owner.comment, sizeof(NTRComment) );
}

// �A���[�����̃Z�b�g�B
static inline void NSD_SetAlarmData( const NTRAlarm *pAlarm )
{
	MI_CpuCopy16( pAlarm, &GetNSD()->alarm, sizeof(NTRAlarm) );
}

// �^�b�`�p�l���L�����u���[�V�����f�[�^�̃Z�b�g�B
static inline void NSD_SetTPCalibration( const NTRTPCalibData *pTPCalib )
{
	MI_CpuCopy16( pTPCalib, &GetNSD()->tp, sizeof(NTRTPCalibData) );
}

// ����R�[�h�̃Z�b�g
#if 0
static inline void NSD_SetLanguage( NTRLangCode language )
{
	GetNSDEx()->language = language;
	
	if( language >= NTR_LANG_CODE_MAX_WW ) {
		GetNSD()->option.language = NTR_LANG_ENGLISH;
	}else {
		GetNSD()->option.language = language;
	}
}
#else
static inline void NSD_SetLanguage( NTRLangCode language )
{
	GetNSD()->option.language = language;
}
static inline void NSD_SetLanguageEx( NTRLangCode language )
{
	GetNSDEx()->language = language;
}
#endif

// �Ή�����r�b�g�}�b�v�̃Z�b�g
static inline void NSD_SetLanguageBitmap( u16 valid_language_bitmap )
{
	GetNSDEx()->valid_language_bitmap = valid_language_bitmap;
}

// RTC�I�t�Z�b�g�l�̃Z�b�g
static inline void NSD_SetRTCOffset( s64 rtcOffset )
{
	GetNSD()->option.rtcOffset = rtcOffset;
}

// RTC�N���b�N�����l�̃Z�b�g
static inline void NSD_SetRTCClockAdjust( u8 rtcClockAdjust )
{
	GetNSD()->option.rtcClockAdjust = rtcClockAdjust;
}


// RTC��LastSetYear�ւ̃Z�b�g
static inline void NSD_SetRTCLastSetYear( u8 rtcLastSetYear )
{
	GetNSD()->option.rtcLastSetYear = rtcLastSetYear;
}


// �N���V�[�P���X�̎����N��ON,OFF�t���O���Z�b�g�B
static inline void NSD_SetFlagAutoBoot( BOOL set )
{
	GetNSD()->option.isAutoBoot = (u16)set;
}

// �o�b�N���C�gON/OFF���Z�b�g�B
static inline void NSD_SetFlagBacklightOff( BOOL set )
{
	GetNSD()->option.isBacklightOff = (u16)set;
}

// �o�b�N���C�g�P�x�����Z�b�g�B
static inline void NSD_SetBacklightBrightness( BOOL backlightBrightness )
{
	GetNSD()->option.backlightBrightness = (u16)backlightBrightness;
}

// ����N���V�[�P���X�����ǂ����̃t���O�Z�b�g�B
static inline void NSD_SetFlagInitialSequence( BOOL set )
{
	GetNSD()->option.initialSequence = (u16)set;
}

// �a�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NSD_SetFlagBirthday( BOOL set )
{
	GetNSD()->option.isSetBirthday = (u16)set;
}

// ���[�U�[�J���[�̓��͍ς݃t���O�Z�b�g�B
static inline void NSD_SetFlagUserColor( BOOL set )
{
	GetNSD()->option.isSetUserColor = (u16)set;
}

// TP�L�����u���[�V�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NSD_SetFlagTP( BOOL set )
{
	GetNSD()->option.isSetTP = (u16)set;
}

// ����R�[�h�̓��͍ς݃t���O�Z�b�g�B
static inline void NSD_SetFlagLanguage( BOOL set )
{
	GetNSD()->option.isSetLanguage = (u16)set;
}

// ���t�E�������͍ς݃t���O�Z�b�g�B
static inline void NSD_SetFlagDateTime( BOOL set )
{
	GetNSD()->option.isSetDateTime = (u16)set;
}

// �I�[�i�[���̃j�b�N�l�[���̓��͍ς݃t���O�Z�b�g�B
static inline void NSD_SetFlagNickname( BOOL set )
{
	GetNSD()->option.isSetNickname = (u16)set;
}

// GB�A�v���ȂǂP��ʃ\�t�g���g����ʃt���O���Z�b�g
static inline void NSD_SetFlagGBUseTopLCD( BOOL set )
{
	GetNSD()->option.isGBUseTopLCD = (u32)set;
}

#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// NTR_SETTINGS_H_
