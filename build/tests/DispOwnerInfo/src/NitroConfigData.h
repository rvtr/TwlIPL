/********************************************************************/
/*      NitroConfigData.h                                           */
/*          NITRO-IPL                                               */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	NITR�ݒ�f�[�^��`�@�w�b�_


	$Log: NitroConfigData.h,v $
	Revision 1.4.2.2.4.1.4.1  2007/01/22 07:36:16  yosiokat
	NAT-IPL2�ւ̑Ή��B
	
	Revision 1.4.2.5  2006/06/26 02:57:54  yosiokat
	�E����R�[�h���؍���ɑΉ�������B
	�E�ꕔ�̒�`�f�[�^�̖��̕ύX�B
	
	Revision 1.4.2.4  2006/02/06 11:34:56  yosiokat
	NCD_GetIPL2BM7RomAddr�֐��̒ǉ��B
	
	Revision 1.4.2.3  2006/02/06 08:27:19  yosiokat
	IPL2�o�[�W�����̒�`��IPLType.h�̒�`�l���g���悤�ύX�B
	
	Revision 1.4.2.2  2005/10/31 08:49:57  yosiokat
	USG_BACKLIGHT_DEFAULT��"2"�ɕύX�B
	
	Revision 1.4.2.1  2005/10/25 08:14:41  yosiokat
	USG�Ή��̂��߁A�ȉ��̕ύX���s���B
	�ENitroConfigData�\���̂�backLightBrightness��ǉ��B
	�EIPL2_TYPE����p�̒萔��`��USG�̂��̂�ǉ��B
	
	Revision 1.4  2005/04/01 05:45:04  yosiokat
	�ŐV�łɍX�V�B
	
	Revision 1.3  2005/03/31 06:06:54  yosiokat
	NitroConfigData�A�N�Z�X�֐��̑S�ʉ���B
	
	Revision 1.31.2.2  2005/03/16 00:36:01  yosiokat
	����R�[�h�̒�����ǉ��ɔ���NitroConfigDataEx�̐V�݂�NitroConfigData�̎戵���̕ύX�B
	
	Revision 1.31.2.1  2005/03/11 11:08:56  yosiokat
	������̒ǉ��B
	
	Revision 1.31  2005/02/15 02:38:27  yosiokat
	NCD_CorrectData�̍폜�B
	
	Revision 1.30  2005/02/07 11:12:03  yosiokat
	NCD_CorrectData�̒ǉ��B
	
	Revision 1.29  2004/09/25 10:42:54  Yosiokat
	NvOption����rsv�̈��rtcOffset�Z�o�p�p�����[�^rtcLastSetYear��ۑ�����悤�ύX�B
	
	Revision 1.28  2004/09/16 07:02:51  Yosiokat
	�I�[�i�[���̃j�b�N�l�[���A�D���ȐF�A�a�������N���A����NCD_ClearOwnerInfo�̒ǉ��B
	
	Revision 1.27  2004/09/01 09:18:35  Yosiokat
	�ENvOption��destroyFlashFlag��ǉ��B
	�E��L�A�N�Z�X�֐�NCD_GetDestroyFlash�ANCD_SetDestroyFlash��ǉ��B
	
	Revision 1.26  2004/08/31 09:52:38  Yosiokat
	small fix.
	
	Revision 1.25  2004/08/27 12:36:08  Yosiokat
	IPL2_BUILD_��SDK_SMALL_BUILD�ɕύX�B
	
	Revision 1.24  2004/08/25 09:27:41  Yosiokat
	�E�K�v�Ȃ��Ȃ������L�v�f��rsv�ɕύX�B�i�t�H�[�}�b�g�̌݊���ۂ��߁A�f�[�^�͋l�߂Ȃ��j
	  NvAlarm.second
	  NvAlarm.enableWeek
	  NvOption.detectPullOutCardFlag
	  NvOption.detectPullOutCtrdgFlag
	  NvOption.timezone
	
	Revision 1.23  2004/08/25 05:12:20  Yosiokat
	�u�����`�^�Ored_ipl2_2004_08_24_pp2_isdbg_fix�Ƃ̃}�[�W�B
	
	Revision 1.22  2004/08/25 01:28:23  Yosiokat
	�ENvOption.backLightOffFlag���r�b�g�t�B�[���h�ɂȂ��Ă��Ȃ������̂��C���B
	�ENvOption.input_birthday�t���O��ǉ��B�A�N�Z�X�֐�NCD_GetInputBirthday�ANCD_SetInputBirthday
	�ENCDStore��32byte�P�ʂ̃T�C�Y�ɂȂ�悤�����B�i�L���b�V�����C���ɂ��킹��j
	
	Revision 1.21  2004/08/23 08:24:23  Yosiokat
	�o�b�N���C�gON,OFF�t���O��IPl2_work����ēxNitroConfigData�ɖ߂��B
	
	Revision 1.20  2004/08/19 06:06:02  yosiokat
	NitroConfigData�Ƀr�b�g�t�B�[���h���g���Ă��邽�߁A���R���p�C���Ŗ�肪�o��\��������B
	�Ƃ肠�����ASDK_CW����`����Ă��Ȃ���΁A�w�b�_��؂��ăG���[���o�����邱�ƂŁA���R���p�C���Ŗ�肪�Ȃ������m�F�ł���悤�ɂ��Ă����B
	
	Revision 1.19  2004/08/18 07:33:00  Yosiokat
	�ENCD_FAVORITE_COLOR_MAX_NUM�ǉ��B
	�ENitroConfigData.option.input_favoriteColor�ǉ��B
	
	Revision 1.18  2004/08/17 05:44:19  Nakasima
	�EtopLcdBackLightOff��NitroConfigData����IPL2_work�ֈړ��B
	�EdetectPullOutFlag���폜���AdetectPullOutCardFla��detectPullOutCtrdgFlag��ǉ��B
	
	Revision 1.17  2004/08/16 10:13:09  Yosiokat
	pullCardFlag��detectPullOutFlag�ɕύX�B
	
	Revision 1.16  2004/08/16 10:09:34  Yosiokat
	NvOption��pullCardFlag��ǉ��B
	
	Revision 1.15  2004/08/13 07:41:30  Yosiokat
	�ENvOwnerInfo����sex���폜�B
	�ENvOption����bottomLcdBackLightOff���폜�B
	�ENvOption��autoBootFlag��ǉ��B
	�ENCD_SetSex, NCD_GetSex���폜�B
	�ENCD_GetRtcClockAdjust�ANCD_GetAutoBootFlag�ANCD_SetRtcClockAdjust�ANCD_SetAutoBootFlag�̒ǉ��B
	
	Revision 1.14  2004/07/29 04:53:18  Yosiokat
	�\���̂̃p�f�B���O�𖾎��I�ɒǉ��B
	
	Revision 1.13  2004/07/18 10:55:13  Yosiokat
	NITRO�ݒ�f�[�^�̃Z�b�g�֐���p�ӁB
	
	Revision 1.12  2004/07/17 09:00:14  Yosiokat
	�E�\���̃T�C�Y�̃R�����g�C���B
	
	Revision 1.11  2004/07/15 12:47:33  Yosiokat
	�E�I�[�i�[���̒a�����́u�N�v���폜�B
	
	Revision 1.10  2004/07/15 12:26:43  Yosiokat
	�ENITRO�ݒ�f�[�^�̃t�H�[�}�b�g��ύX�B
	�E�I�[�i�[�����ȉ��̂悤�ɕύX�B
	�@a)�j�b�N�l�[�������P�O�ɕύX�B
	�@b)�R�����g�f�[�^��ǉ�
	�@c)���t�^�f�[�^���폜�B
	�@d)�D���ȐF�f�[�^��ǉ��B�i���g�͖���j
	  e)���ʃf�[�^���r�b�g�t�B�[���h�ɁB
	�E�A���[������ǉ��B
	�E�^�b�`�p�l���L�����u���[�V�������̊e�v�f���ڍׂɒ�`�B
	�E���̑��̏����I�v�V�������ɂ܂Ƃ߂�B�i����R�[�h���r�b�g�t�B�[���h�Ƃ��Ă����ɓ����B�j
	
	Revision 1.9  2004/07/13 00:20:15  Yosiokat
	�Esmall fix.
	
	Revision 1.8  2004/07/05 02:29:23  Yosiokat
	����R�[�h��FRENCH�ɏC���B
	
	Revision 1.7  2004/06/28 01:56:17  Yosiokat
	�E�o�b�N���C�gON,OFF�ݒ�̒l��ǉ��B
	�ETP�A����ݒ�ARTC�A�I�[�i�[���̓��͍ς݃t���O��ǉ��B
	�E�o�[�W������"3"�ɁB
	
	Revision 1.6  2004/06/14 04:55:10  yosiokat
	NitroConfigData��rtcOffset��int����s64�ɕύX�B
	����ɔ���NCD�t�H�[�}�b�g�̃o�[�W������"2"�ɕύX�B
	
	Revision 1.5  2004/06/07 10:51:58  Yosiokat
	�I�[�i�[���̃j�b�N�l�[����SJIS����Unicode�iUTF16�j�ɕύX�������߁A�o�[�W�������P�ɏグ��B
	
	Revision 1.4  2004/05/21 06:12:17  Yosiokat
	�w�b�_��C++�Ή������Łu};�v�̃Z�~�R�������G���[�ɂȂ��Ă����̂ŁA�폜�B
	
	Revision 1.3  2004/05/21 05:37:46  Yosiokat
	TP�L�����u���[�V������NVRAM�ɕۑ�����f�[�^��LCD�̍���|�C���g�{LCD�̉E���|�C���g���܂߂�悤�ύX�B�i�T�C�Y��8->12byte�ցj
	�܂��ASDK��config.h�̖��O��NitroConfigData.h�Ɠ���t�@�C���������Ă����̂ŁA������C���N���[�h���Ȃ��悤�AMakefile�ŃR���p�C������IPL2_BUILD_�V���{����^����悤�ύX�B
	
	Revision 1.2  2004/05/19 08:25:28  yosiokat
	�X�V���O�̒ǉ��B
	

*/


#ifndef	NITRO_CONFIG_DATA_H_
#define	NITRO_CONFIG_DATA_H_
#if		defined(SDK_CW)							// NitroConfigData�Ƀr�b�g�t�B�[���h���g���Ă���̂ŁA�R���p�C���ˑ��ŕs�����������\��������B
												// ����āACW�ȊO�̃R���p�C���̏ꍇ�́A���̃w�b�_�𖳌��ɂ��ăG���[���o������悤�ɂ��čĊm�F����B
#ifdef __cplusplus
extern "C" {
#endif


#include <nitro.h>

#define USING_COMPONENT							// IPL2��ł͂Ȃ��̂ŁA���̃X�C�b�`��L���ɁB


// define data ------------------------------------
#define NITRO_CONFIG_DATA_VERSION		5		// NITRO�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����
#define NITRO_CONFIG_DATA_EX_VERSION	1		// �g��NITRO�ݒ�f�[�^�t�H�[�}�b�g�o�[�W����
#define NCD_NICKNAME_LENGTH				10		// �j�b�N�l�[����
#define NCD_COMMENT_LENGTH				26		// �R�����g��
#define NCD_FAVORITE_COLOR_MAX_NUM		16		// �D���ȐF�̍ő吔

#define NCD_ROM_ADDR_SHIFT				3		// IPL2�w�b�_��NitroConfigData�i�[�A�h���X�̃V�t�g�l
#define FONT_ROM_ADDR_SHIFT				2		// IPL2�w�b�_�̃t�H���g�f�[�^�i�[�A�h���X�̃V�t�g�l
#define NCD_SYS_RSV_SIZE				1024	// �V�X�e���\��̈�T�C�Y
#define NCD_APP_RSV_SIZE				512		// �A�v���@�\��̈�T�C�Y
												// NCD_SYS_RSV_ROM_ADDR = ncd_rom_addr - NCD_SYS_RSV_SIZE
												// NCD_APP_RSV_ROM_ADDR = ncd_rom_addr - NCD_SYS_RSV_SIZE - NCD_APP_RSV_SIZE
#define USG_BACKLIGHT_DEFAULT			2		// USG�̃o�b�N���C�g�f�t�H���g�l


// ����ݒ�R�[�h
typedef enum NvLangCode{
	LANG_JAPANESE =0,							// ���{��
	LANG_ENGLISH,								// �p��
	LANG_FRENCH,								// �t�����X��
	LANG_GERMAN,								// �h�C�c��
	LANG_ITALIAN,								// �C�^���A��
	LANG_SPANISH,								// �X�y�C����
	LANG_CHINESE,								// ������
	LANG_HANGUL,								// �؍���
	LANG_CODE_MAX
}NvLangCode;

#define LANG_CODE_MAX_WW			LANG_CHINESE					// �����ňȑO�̒ʏ��IPL2�ł�LANG_CODE_MAX

#define LANG_BITMAP_WW			( 	( 0x0001 << LANG_JAPANESE )  \
								  | ( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_ITALIAN  )  \
								  | ( 0x0001 << LANG_SPANISH  ) ) 	// �ʏ�łł̑Ή�����r�b�g�}�b�v

#define LANG_BITMAP_CN			( 	( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_ITALIAN  )  \
								  | ( 0x0001 << LANG_SPANISH  )  \
								  | ( 0x0001 << LANG_CHINESE  ) ) 	// �����łł̑Ή�����r�b�g�}�b�v

#define LANG_BITMAP_KR			( 	( 0x0001 << LANG_JAPANESE )  \
								  | ( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_SPANISH  )  \
								  | ( 0x0001 << LANG_HANGUL  ) ) 	// �؍��łł̑Ή�����r�b�g�}�b�v

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
	u16				rsv2 : 1;					// �\��B
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
// IPL2�����łł̊g���t�H�[�}�b�g
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


typedef struct NCDTimeStamp {
	u8 minute;
	u8 hour;
	u8 day;
	u8 month;
	u8 year;
}NCDTimeStamp;


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
extern u16				 valid_lang_bitmap;

#define GetNcdWorkAddr()		( (NitroConfigData   *)( HW_NVRAM_USER_INFO ) )
#define GetNcdExWorkAddr()		( &ncdEx )
												// NITRO�ݒ�f�[�^�̈�̃A�h���X�l��

extern void NCD_ClearOwnerInfo( void );			// �j�b�N�l�[���E�a�����E�D���ȐF�̃N���A

#ifdef USING_COMPONENT
extern void NCD_ReadIPL2Header  ( void );		// IPL2�w�b�_�̓ǂݏo���B
extern u8   NCD_GetIPL2Type     ( void );		// IPL2�^�C�v�̎擾�B
extern u16  NCD_GetIPL2TypeEx   ( void );		// �g��IPL2�^�C�v�̎擾�B
extern u8  *NCD_GetIPL2Version  ( void );		// IPL2�o�[�W�����̎擾�B
extern u32  NCD_GetNCDRomAddr   ( void );		// NCD�i�[ROM�A�h���X�̎擾�B
extern u32  NCD_GetSysRsvRomAddr( void );		// �V�X�e���\��̈�ROM�A�h���X�̎擾�B
extern u32  NCD_GetAppRsvRomAddr( void );		// �A�v���@�\��̈�ROM�A�h���X�̎擾�B
extern u32  NCD_GetIPL2BM7RomAddr( void );		// �u�[�g���j���[ARM7�R�[�hROM�A�h���X�̎擾�B
extern u32  NCD_GetIPL2DataRomAddr( void );		// IPL2�f�[�^ROM�A�h���X�̎擾�B
extern u32  NCD_GetFontBncmpRomAddr( void );	// �t�H���g�f�[�^bncmp��ROM�A�h���X�擾�B�i�����ŁA���{�������V��łł̂ݗL���B�j
extern u32  NCD_GetFontBnfrRomAddr( void );		// �t�H���g�f�[�^bnfr ��ROM�A�h���X�擾�B�i�����ŁA���{�������V��łł̂ݗL���B�j
#endif

//=========================================================
// �f�[�^�擾
//=========================================================

//-----------------------------------
// �I�[�i�[���S�̂̎擾�B
static inline NvOwnerInfo *NCD_GetOwnerInfo(void)
{
	return	&GetNcdWorkAddr()->owner;
}

// �D���ȐF�̎擾�B
static inline u8 NCD_GetFavoriteColor(void)
{
	return	(u8)GetNcdWorkAddr()->owner.favoriteColor;
}

// �a�����̎擾�B
static inline NvDate *NCD_GetBirthday(void)
{
	return	&GetNcdWorkAddr()->owner.birthday;
}

// �j�b�N�l�[���̎擾�B
static inline NvNickname *NCD_GetNickname(void)
{
	return	&GetNcdWorkAddr()->owner.nickname;
}

// �R�����g�̎擾�B
static inline NvComment *NCD_GetComment(void)
{
	return	&GetNcdWorkAddr()->owner.comment;
}


//-----------------------------------
// �A���[�����̎擾�B
static inline NvAlarm *NCD_GetAlarmData(void)
{
	return	&GetNcdWorkAddr()->alarm;
}


//-----------------------------------
// �^�b�`�p�l���L�����u���[�V�����f�[�^�̎擾�B
static inline NvTpCalibData *NCD_GetTPCalibration(void)
{
	return	&GetNcdWorkAddr()->tp;
}


//-----------------------------------
// �I�v�V�������̎擾�B

// ����R�[�h�̎擾
static inline NvLangCode NCD_GetLanguageOrg(void)
{
	return	(NvLangCode)GetNcdWorkAddr()->option.language;
}

static inline NvLangCode NCD_GetLanguage(void)
{
	return	(NvLangCode)GetNcdExWorkAddr()->language;
}


// RTC�I�t�Z�b�g�l�̎擾
static inline s64 NCD_GetRtcOffset(void)
{
	return	GetNcdWorkAddr()->option.rtcOffset;
}

// RTC�N���b�N�����l�̎擾
static inline u8 NCD_GetRtcClockAdjust(void)
{
	return	GetNcdWorkAddr()->option.rtcClockAdjust;
}

// RTC�̑O��Z�b�g�����N�̎擾
static inline u8 NCD_GetRtcLastSetYear(void)
{
	return	GetNcdWorkAddr()->option.rtcLastSetYear;
}

// �N���V�[�P���X�̎����N��ON���H�i0:OFF, 1:ON�j
static inline int NCD_GetAutoBootFlag(void)
{
	return	(int)GetNcdWorkAddr()->option.autoBootFlag;
}

// �o�b�N���C�gOFF���H�i0:ON, 1:OFF�j
static inline int NCD_GetBackLightOffFlag(void)
{
	return	(int)GetNcdWorkAddr()->option.backLightOffFlag;
}

// �t���b�V�����V�[�P���X�����ǂ����H
static inline int NCD_GetDestroyFlash(void)
{
	return	(int)GetNcdWorkAddr()->option.destroyFlashFlag;
}

// �a�����f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputBirthday(void)
{
	return	(int)GetNcdWorkAddr()->option.input_birthday;
}

// �D���ȐF�f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputFavoriteColor(void)
{
	return	(int)GetNcdWorkAddr()->option.input_favoriteColor;
}

// TP�L�����u���[�V�����f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputTP(void)
{
	return	(int)GetNcdWorkAddr()->option.input_tp;
}

// ����R�[�h���Z�b�g����Ă��邩�H
static inline int NCD_GetInputLanguage(void)
{
	return	(int)GetNcdWorkAddr()->option.input_language;
}

// RTC�f�[�^���Z�b�g����Ă��邩�H
static inline int NCD_GetInputRTC(void)
{
	return	(int)GetNcdWorkAddr()->option.input_rtc;
}

// �I�[�i�[���̃j�b�N�l�[�����Z�b�g����Ă��邩�H
static inline int NCD_GetInputNickname(void)
{
	return	(int)GetNcdWorkAddr()->option.input_nickname;
}

//=========================================================
// �f�[�^�Z�b�g
//=========================================================
//-----------------------------------
// �I�[�i�[���S�̂̃Z�b�g�B
static inline void NCD_SetOwnerInfo(NvOwnerInfo *owinfop)
{
	SVC_CpuCopy( owinfop, &GetNcdWorkAddr()->owner, sizeof(NvOwnerInfo), 16);
}

// �D���ȐF�̃Z�b�g�B
static inline void NCD_SetFavoriteColor(u8 favoriteColor)
{
	GetNcdWorkAddr()->owner.favoriteColor = favoriteColor;
}

// �a�����̃Z�b�g�B
static inline void NCD_SetBirthday(NvDate *birthp)
{
	GetNcdWorkAddr()->owner.birthday.month	= birthp->month;
	GetNcdWorkAddr()->owner.birthday.day	= birthp->day;
}

// �j�b�N�l�[���̃Z�b�g�B
static inline void NCD_SetNickname(NvNickname *namep)
{
	SVC_CpuCopy( namep, &GetNcdWorkAddr()->owner.nickname, sizeof(NvNickname), 16);
}

// �R�����g�̃Z�b�g�B
static inline void NCD_SetComment(NvComment *commentp)
{
	SVC_CpuCopy( commentp, &GetNcdWorkAddr()->owner.comment, sizeof(NvComment), 16);
}


//-----------------------------------
// �A���[�����̃Z�b�g�B
static inline void NCD_SetAlarmData(NvAlarm *alarmp)
{
	SVC_CpuCopy( alarmp, &GetNcdWorkAddr()->alarm, sizeof(NvAlarm), 16);
}


//-----------------------------------
// �^�b�`�p�l���L�����u���[�V�����f�[�^�̃Z�b�g�B
static inline void NCD_SetTPCalibration(NvTpCalibData *tp_calibp)
{
	SVC_CpuCopy( tp_calibp, &GetNcdWorkAddr()->tp, sizeof(NvTpCalibData), 16);
}


//-----------------------------------
// �I�v�V�������̃Z�b�g�B

// ����R�[�h�̃Z�b�g
#if 0
static inline void NCD_SetLanguage(NvLangCode language)
{
	GetNcdExWorkAddr()->language				= language;
	GetNcdExWorkAddr()->valid_language_bitmap	= LANG_BITMAP_CHINESE;
	
	if( language == LANG_CHINESE ) {
		GetNcdWorkAddr()->option.language = LANG_ENGLISH;
	}else {
		GetNcdWorkAddr()->option.language = language;
	}
}
#endif

// RTC�I�t�Z�b�g�l�̃Z�b�g
static inline void NCD_SetRtcOffset(s64 rtcOffset)
{
	GetNcdWorkAddr()->option.rtcOffset = rtcOffset;
}

// RTC�N���b�N�����l�̃Z�b�g
static inline void NCD_SetRtcClockAdjust(u8 rtcClockAdjust)
{
	GetNcdWorkAddr()->option.rtcClockAdjust = rtcClockAdjust;
}


// RTC��LastSetYear�ւ̃Z�b�g
static inline void NCD_SetRtcLastSetYear(u8 rtcLastSetYear)
{
	GetNcdWorkAddr()->option.rtcLastSetYear = rtcLastSetYear;
}


// �N���V�[�P���X�̎����N��ON,OFF�t���O���Z�b�g�B
static inline void NCD_SetAutoBootFlag(BOOL autoBootFlag)
{
	GetNcdWorkAddr()->option.autoBootFlag = (u16)autoBootFlag;
}

// �o�b�N���C�gON,OFF�t���O���Z�b�g�B
static inline void NCD_SetBackLightOffFlag(BOOL backLightOffFlag)
{
	GetNcdWorkAddr()->option.backLightOffFlag = (u16)backLightOffFlag;
}

// �t���b�V�����V�[�P���X�����ǂ����̃t���O�Z�b�g�B
static inline void NCD_SetDestroyFlash(BOOL destroy)
{
	GetNcdWorkAddr()->option.destroyFlashFlag = (u16)destroy;
}

// �a�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputBirthday(BOOL input)
{
	GetNcdWorkAddr()->option.input_birthday = (u16)input;
}

// �D���ȐF�f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputFavoriteColor(BOOL input)
{
	GetNcdWorkAddr()->option.input_favoriteColor = (u16)input;
}

// TP�L�����u���[�V�����f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputTP(BOOL input)
{
	GetNcdWorkAddr()->option.input_tp = (u16)input;
}

// ����R�[�h�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputLanguage(BOOL input)
{
	GetNcdWorkAddr()->option.input_language = (u16)input;
}

// RTC�f�[�^�̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputRTC(BOOL input)
{
	GetNcdWorkAddr()->option.input_rtc = (u16)input;
}

// �I�[�i�[���̃j�b�N�l�[���̓��͍ς݃t���O�Z�b�g�B
static inline void NCD_SetInputNickname(BOOL input)
{
	GetNcdWorkAddr()->option.input_nickname = (u16)input;
}



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// NITRO_CONFIG_DATA_H_
