/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     NitroSettings.c

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


#ifndef	NITRO_SETTINGS_H_
#define	NITRO_SETTINGS_H_
#if		defined(SDK_CW)							// NitroConfigData�Ƀr�b�g�t�B�[���h���g���Ă���̂ŁA�R���p�C���ˑ��ŕs�����������\��������B
												// ����āACW�ȊO�̃R���p�C���̏ꍇ�́A���̃w�b�_�𖳌��ɂ��ăG���[���o������悤�ɂ��čĊm�F����B

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define NITRO_CONFIG_DATA_VERSION		5		// NITRO�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����
#define NITRO_CONFIG_DATA_EX_VERSION	1		// �g��NITRO�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����
#define NCD_NICKNAME_LENGTH				10		// �j�b�N�l�[����
#define NCD_COMMENT_LENGTH				26		// �R�����g��
#define NCD_FAVORITE_COLOR_MAX_NUM		16		// �D���ȐF�̍ő吔

// ����ݒ�R�[�h
typedef enum NvLangCode{
	LANG_JAPANESE =0,							// ���{��
	LANG_ENGLISH  =1,							// �p��
	LANG_FRENCH   =2,							// �t�����X��
	LANG_GERMAN   =3,							// �h�C�c��
	LANG_ITALIAN  =4,							// �C�^���A��
	LANG_SPANISH  =5,							// �X�y�C����
  	LANG_CHINESE  =6,							// ������
	LANG_HANGUL   =7,							// �؍���
	LANG_CODE_MAX
}NvLangCode;

#define LANG_CODE_MAX_WW			( LANG_SPANISH + 1 )
#define LANG_BITMAP_WW			( 	( 0x0001 << LANG_JAPANESE )  \
								  | ( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_ITALIAN  )  \
								  | ( 0x0001 << LANG_SPANISH  ) ) 	// �ʏ�łł̑Ή�����r�b�g�}�b�v

#define VALID_LANG_BITMAP			LANG_BITMAP_WW					// �{IPL2�̑Ή�����r�b�g�}�b�v


// ���t�f�[�^
typedef struct NvDate{
	u8				month;						// ��:01�`12
	u8				day;						// ��:01�`31
}NvDate;			// 2byte

// �j�b�N�l�[��
typedef struct NvNickname{
	u16				str[NCD_NICKNAME_LENGTH];	// �j�b�N�l�[���iUnicode(UTF16)�ōő�10�����A�I�[�R�[�h�Ȃ��j
	u8				length;						// ������
	u8				rsv;
}NvNickname;		// 22byte

// �R�����g
typedef struct NvComment{
	u16				str[NCD_COMMENT_LENGTH];	//�R�����g�iUnicode(UTF16)�ōő�26�����A�I�[�R�[�h�Ȃ��j
	u8				length;						// ������
	u8				rsv;
}NvComment;			// 54byte

// �I�[�i�[���
typedef struct NvOwnerInfo{
	u8				favoriteColor : 4;			// �D���ȐF
	u8				rsv : 4;					// �\��B
	NvDate			birthday;					// ���N����
	u8				pad;
	NvNickname		nickname;					// �j�b�N�l�[��
	NvComment		comment;					// �R�����g
}NvOwnerInfo;		// 80byte

// IPL�p�ڊo�܂����v�f�[�^
typedef struct NvAlarm{
	u8				hour;						// �A���[����:00�`23
	u8				minute;						// �A���[����:00�`59
	u8				rsv1;						// �\��
	u8				pad;
	u16				alarmOn : 1;				// �A���[��ON,OFF�i0:OFF, 1:ON�j
	u16				rsv2 : 15;					// �\��
}NvAlarm;			// 6byte

// �^�b�`�p�l���L�����u���[�V�����f�[�^
typedef struct NvTpCalibData{
	u16				raw_x1;						// ��P�L�����u���[�V�����_��TP�擾�lX
	u16				raw_y1;						// �@�@�@�@�V�@�@�@�@�@�@�@�@TP�擾�lY
	u8				dx1;						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W X
	u8				dy1;						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W Y
	u16				raw_x2;						// ��Q�L�����u���[�V�����_��TP�擾�lX
	u16				raw_y2;						// �@�@�@�@�V�@�@�@�@�@�@�@�@TP�擾�lY
	u8				dx2;   						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W X
	u8				dy2;   						// �@�@�@�@�V�@�@�@�@�@�@�@�@LCD���W Y
}NvTpCalibData;		// 12byte

// �I�v�V�������
typedef struct NvOption{
	u16				language : 3;				// ����R�[�h�iLANG_SPANISH�܂ł̕W������R�[�h������j
	u16				agbLcd : 1;					// AGB���[�h�ŋN�����鎞�ɂǂ����LCD�ŋN�����邩�H�i0:TOP,1:BOTTOM�j
	u16				backLightBrightness : 2;	// �o�b�N���C�g�P�x�f�[�^
	u16				autoBootFlag : 1;			// �N���V�[�P���X�ŁA���j���[��~�Ȃ��Ŏ����N�����邩�ǂ����H(0:OFF, 1:ON)
	u16				backLightOffFlag : 1;		// �o�b�N���C�gON,OFF�t���O�i0:ON, 1:OFF�j
	u16				rsv2 : 1;					// �\��
	u16				destroyFlashFlag : 1;		// �t���b�V�����V�[�P���X���t���O
	u16				input_birthday : 1;			// �a���������͂��ꂽ���H
	u16				input_favoriteColor : 1;	// �D���ȐF�����͂��ꂽ���H
	u16				input_tp : 1;				// �^�b�`�p�l�����L�����u���[�V�������ꂽ���H�i  �V  )
	u16				input_language : 1;			// ������͂����ꂽ���H�@		(0:���ݒ�, 1:�ݒ�ς�)
	u16				input_rtc : 1;				// RTC�ݒ肪���ꂽ���H			(		�V�@�@�@�@�@ )
	u16				input_nickname : 1;			// �j�b�N�l�[�������͂��ꂽ���H	(		�V�@�@�@�@�@ )
	u8				rtcLastSetYear;				// RTC�̑O��ݒ�N
	u8				rtcClockAdjust;				// RTC�N���b�N�����l
	s64				rtcOffset;					// RTC�ݒ莞�̃I�t�Z�b�g�l�i���[�U�[��RTC�ݒ��ύX����x�ɂ��̒l�ɉ����đ������܂��B�j
}NvOption;			// 12byte

// NITRO�e��ݒ�f�[�^
typedef struct NitroConfigData{
	u8				version;					// �t���b�V���i�[�f�[�^�t�H�[�}�b�g�̃o�[�W����
	u8				pad;
	NvOwnerInfo		owner;						// �I�[�i�[���
	NvAlarm			alarm;						// IPL�p�ڊo�܂����v�f�[�^
	NvTpCalibData	tp;							// �^�b�`�p�l���L�����u���[�V�����f�[�^
	NvOption		option;						// �I�v�V����
}NitroConfigData;	// 112byte

// NITRO�e��ݒ�f�[�^��NVRAM�ۑ����t�H�[�}�b�g
typedef struct NCDStore{
	NitroConfigData ncd;						// NITRO�e��ݒ�f�[�^
	u16				saveCount;					// 0x00-0x7f�����[�v���ăJ�E���g���A�J�E���g�l���V�����f�[�^���L���B
	u16				crc16;						// NITRO�e��ݒ�f�[�^��16bitCRC
	u8				pad[ 128 - sizeof(NitroConfigData) - 4];
}NCDStore;			// 128byte					// ���{���Ȃ�AsaveCount��crc16��256byte�̍Ō�ɕt�����āA�ԂɃp�f�B���O�𖄂߂���������B


//----------------------------------------------
// IPL2�����E�؍��łł̊g���t�H�[�}�b�g
//----------------------------------------------
// �g��NITRO�ݒ�f�[�^
typedef struct NitroConfigDataEx{
	u8				version;					// �o�[�W����
	u8				language;					// ����R�[�h�iLANG_CHINESE�ȍ~�Ɋg�����ꂽ�l������B�j
	u16				valid_language_bitmap;		// �{IPL2�ŗL���Ȍ���R�[�h���������r�b�g�}�b�v
	u8				pad[ 256 - sizeof(NitroConfigData) - 4 - 4 - 2 ];		// 4:saveCount+crc16, 2:NCDEx.version+NCDEx.language, 2:crc16_ex
}NitroConfigDataEx;	// 138bytes

// NITRO�e��ݒ�f�[�^��NVRAM�ۑ����t�H�[�}�b�g
typedef struct NCDStoreEx{
	NitroConfigData		ncd;					// NITRO�e��ݒ�f�[�^
	u16					saveCount;				// 0x00-0x7f�����[�v���ăJ�E���g���A�J�E���g�l���V�����f�[�^���L���B
	u16					crc16;					// NITRO�e��ݒ�f�[�^��16bitCRC
	NitroConfigDataEx	ncd_ex;
	u16					crc16_ex;
}NCDStoreEx;		// 256byte					// ���{���Ȃ�AsaveCount��crc16��256byte�̍Ō�ɕt�����āA�ԂɃp�f�B���O�𖄂߂���������B


//=========================================================
// NVRAM�ւ̃��[�h���C�g�֐�
//=========================================================
#ifdef SDK_ARM9
extern int  NVRAMm_ReadNitroConfigData (NitroConfigData *dstp);
extern void NVRAMm_WriteNitroConfigData(NitroConfigData *dstp);
#endif


//=========================================================
// NITRO�ݒ�f�[�^�ւ̃A�N�Z�X�֐�
//=========================================================
extern NitroConfigDataEx ncdEx;

#define GetNCDWork()		( (NitroConfigData *)( HW_NVRAM_USER_INFO ) )
#define GetNCDExWork()		( &ncdEx )
												// NITRO�ݒ�f�[�^�̈�̃A�h���X�l��

extern void NCD_ClearOwnerInfo( void );			// �j�b�N�l�[���E�a�����E�D���ȐF�̃N���A
extern u8   NCD_GetIPL2Type( void );			
extern u8  *NCD_GetIPL2Version( void );
extern u32  NCD_GetNCDRomAddr( void );

//=========================================================
// �f�[�^�擾
//=========================================================

//-----------------------------------
// �I�[�i�[���S�̂̎擾�B
static inline NvOwnerInfo *NCD_GetOwnerInfo(void)
{
	return	&GetNCDWork()->owner;
}

// �D���ȐF�̎擾�B
static inline u8 NCD_GetFavoriteColor(void)
{
	return	(u8)GetNCDWork()->owner.favoriteColor;
}

// �a�����̎擾�B
static inline NvDate *NCD_GetBirthday(void)
{
	return	&GetNCDWork()->owner.birthday;
}

// �j�b�N�l�[���̎擾�B
static inline NvNickname *NCD_GetNickname(void)
{
	return	&GetNCDWork()->owner.nickname;
}

// �R�����g�̎擾�B
static inline NvComment *NCD_GetComment(void)
{
	return	&GetNCDWork()->owner.comment;
}


//-----------------------------------
// �A���[�����̎擾�B
static inline NvAlarm *NCD_GetAlarmData(void)
{
	return	&GetNCDWork()->alarm;
}


//-----------------------------------
// �^�b�`�p�l���L�����u���[�V�����f�[�^�̎擾�B
static inline NvTpCalibData *NCD_GetTPCalibration(void)
{
	return	&GetNCDWork()->tp;
}


//-----------------------------------
// �I�v�V�������̎擾�B

// ����R�[�h�̎擾
static inline NvLangCode NCD_GetLanguage(void)
{
  	return	(NvLangCode)GetNCDExWork()->language;
}

static inline NvLangCode NCD_GetLanguageOrg(void)
{
	return	(NvLangCode)GetNCDWork()->option.language;
}

// RTC�I�t�Z�b�g�l�̎擾
static inline s64 NCD_GetRtcOffset(void)
{
	return	GetNCDWork()->option.rtcOffset;
}

// RTC�N���b�N�����l�̎擾
static inline u8 NCD_GetRtcClockAdjust(void)
{
	return	GetNCDWork()->option.rtcClockAdjust;
}

// RTC�̑O��Z�b�g�����N�̎擾
static inline u8 NCD_GetRtcLastSetYear(void)
{
	return	GetNCDWork()->option.rtcLastSetYear;
}

// �N���V�[�P���X�̎����N��ON���H�i0:OFF, 1:ON�j
static inline int NCD_GetAutoBootFlag(void)
{
	return	(int)GetNCDWork()->option.autoBootFlag;
}

// �o�b�N���C�g�P�x�擾�i0-3�j
static inline int NCD_GetBackLightBrightness(void)
{
	return	(int)GetNCDWork()->option.backLightBrightness;
}

// �t���b�V�����V�[�P���X�����ǂ����H
static inline int NCD_GetDestroyFlash(void)
{
	return	(int)GetNCDWork()->option.destroyFlashFlag;
}

// �a�����f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputBirthday(void)
{
	return	(int)GetNCDWork()->option.input_birthday;
}

// �D���ȐF�f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputFavoriteColor(void)
{
	return	(int)GetNCDWork()->option.input_favoriteColor;
}

// TP�L�����u���[�V�����f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputTP(void)
{
	return	(int)GetNCDWork()->option.input_tp;
}

// ����R�[�h���Z�b�g����Ă��邩�H
static inline int NCD_GetInputLanguage(void)
{
	return	(int)GetNCDWork()->option.input_language;
}

// RTC�f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputRTC(void)
{
	return	(int)GetNCDWork()->option.input_rtc;
}

// �I�[�i�[���̃j�b�N�l�[�����Z�b�g����Ă��邩�H
static inline int NCD_GetInputNickname(void)
{
	return	(int)GetNCDWork()->option.input_nickname;
}

//=========================================================
// �f�[�^�Z�b�g
//=========================================================
//-----------------------------------
// �I�[�i�[���S�̂̃Z�b�g�B
static inline void NCD_SetOwnerInfo(NvOwnerInfo *owinfop)
{
	SVC_CpuCopy( owinfop, &GetNCDWork()->owner, sizeof(NvOwnerInfo), 16);
}

// �D���ȐF�̃Z�b�g�B
static inline void NCD_SetFavoriteColor(u8 favoriteColor)
{
	GetNCDWork()->owner.favoriteColor = favoriteColor;
}

// �a�����̃Z�b�g�B
static inline void NCD_SetBirthday(NvDate *birthp)
{
	GetNCDWork()->owner.birthday.month	= birthp->month;
	GetNCDWork()->owner.birthday.day	= birthp->day;
}

// �j�b�N�l�[���̃Z�b�g�B
static inline void NCD_SetNickname(NvNickname *namep)
{
	SVC_CpuCopy( namep, &GetNCDWork()->owner.nickname, sizeof(NvNickname), 16);
}

// �R�����g�̃Z�b�g�B
static inline void NCD_SetComment(NvComment *commentp)
{
	SVC_CpuCopy( commentp, &GetNCDWork()->owner.comment, sizeof(NvComment), 16);
}


//-----------------------------------
// �A���[�����̃Z�b�g�B
static inline void NCD_SetAlarmData(NvAlarm *alarmp)
{
	SVC_CpuCopy( alarmp, &GetNCDWork()->alarm, sizeof(NvAlarm), 16);
}


//-----------------------------------
// �^�b�`�p�l���L�����u���[�V�����f�[�^�̃Z�b�g�B
static inline void NCD_SetTPCalibration(NvTpCalibData *tp_calibp)
{
	SVC_CpuCopy( tp_calibp, &GetNCDWork()->tp, sizeof(NvTpCalibData), 16);
}


//-----------------------------------
// �I�v�V�������̃Z�b�g�B

// ����R�[�h�̃Z�b�g
static inline void NCD_SetLanguage(NvLangCode language)
{
	GetNCDExWork()->language              = language;
	
	if( language >= LANG_CODE_MAX_WW ) {
		GetNCDWork()->option.language = LANG_ENGLISH;
	}else {
		GetNCDWork()->option.language = language;
	}
}

// RTC�I�t�Z�b�g�l�̃Z�b�g
static inline void NCD_SetRtcOffset(s64 rtcOffset)
{
	GetNCDWork()->option.rtcOffset = rtcOffset;
}

// RTC�N���b�N�����l�̃Z�b�g
static inline void NCD_SetRtcClockAdjust(u8 rtcClockAdjust)
{
	GetNCDWork()->option.rtcClockAdjust = rtcClockAdjust;
}


// RTC��LastSetYear�ւ̃Z�b�g
static inline void NCD_SetRtcLastSetYear(u8 rtcLastSetYear)
{
	GetNCDWork()->option.rtcLastSetYear = rtcLastSetYear;
}


// �N���V�[�P���X�̎����N��ON,OFF�t���O���Z�b�g�B
static inline void NCD_SetAutoBootFlag(BOOL autoBootFlag)
{
	GetNCDWork()->option.autoBootFlag = (u16)autoBootFlag;
}

// �o�b�N���C�g�P�x�����Z�b�g�B
static inline void NCD_SetBackLightBrightness(BOOL backLightBrightness )
{
	GetNCDWork()->option.backLightBrightness = (u16)backLightBrightness;
}

// �t���b�V�����V�[�P���X�����ǂ����̃t���O�Z�b�g�B
static inline void NCD_SetDestroyFlash(BOOL destroy)
{
	GetNCDWork()->option.destroyFlashFlag = (u16)destroy;
}

// �a�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputBirthday(BOOL input)
{
	GetNCDWork()->option.input_birthday = (u16)input;
}

// �D���ȐF�f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputFavoriteColor(BOOL input)
{
	GetNCDWork()->option.input_favoriteColor = (u16)input;
}

// TP�L�����u���[�V�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputTP(BOOL input)
{
	GetNCDWork()->option.input_tp = (u16)input;
}

// ����R�[�h�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputLanguage(BOOL input)
{
	GetNCDWork()->option.input_language = (u16)input;
}

// RTC�f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputRTC(BOOL input)
{
	GetNCDWork()->option.input_rtc = (u16)input;
}

// �I�[�i�[���̃j�b�N�l�[���̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputNickname(BOOL input)
{
	GetNCDWork()->option.input_nickname = (u16)input;
}



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// NITRO_SETTINGS_H_
