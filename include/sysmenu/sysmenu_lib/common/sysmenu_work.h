/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     sysmenu_work.c

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

#ifndef	__SYSMENU_WORK_H__
#define	__SYSMENU_WORK_H__

#include <twl.h>
#include <twl/nam.h>

#include <sysmenu/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
#define SYSM_RESET_PARAM_MAGIC_CODE			"TRST"
#define SYSM_RESET_PARAM_MAGIC_CODE_LEN		4

#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2


// NAMTitleID��HiLo�ɕ������ăA�N�Z�X����ꍇ�Ɏg�p
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;


// BootFlags�Ŏg�p����media���
typedef enum TitleMedia {
	TITLE_MEDIA_NAND = 0,
	TITLE_MEDIA_CARD = 1,
	TITLE_MEDIA_MAX  = 2
}TitleMedia;


// �^�C�g�������Z�b�g�p�����[�^�@�t���O
typedef struct BootFlags {
	u16			isValid : 1;				// TRUE:valid, FALSE:invalid
	u16			media : 3;					// 0:nand, 1:card, 2-7:rsv;
	u16			isLogoSkip : 1;				// ���S�f���X�L�b�v�v��
	u16			isInitialShortcutSkip : 1;	// ����N���V�[�P���X�X�L�b�v�v��
	u16			isAppLoadCompleted : 1;		// �A�v�����[�h�ς݂�����
	u16			isAppRelocate : 1;			// �A�v���Ĕz�u�v��
	u16			rsv : 9;
}BootFlags;


// ���Z�b�g�p�����[�^�@�w�b�_
typedef struct ResetParameterHeader {
	u32			magicCode;				// SYSM_RESET_PARAM_MAGIC_CODE������
	u8			type;					// �^�C�v�ɂ����Body�𔻕ʂ���B
	u8			bodyLength;				// body�̒���
	u16			crc16;					// body��CRC16
}ResetParamHeader;


// ���Z�b�g�p�����[�^�@�{�f�B
typedef union ResetParamBody {
	struct {							// ���Ƃ肠�����ŏ���TitleProperty�ƃt�H�[�}�b�g�����킹�Ă���
		NAMTitleId	bootTitleID;		// ���Z�b�g��Ƀ_�C���N�g�N������^�C�g��ID
		BootFlags	flags;				// ���Z�b�g���̃����`���[����t���O
		u8			rsv[ 4 ];			// �\��
	}v1;
}ResetParamBody;


// ���Z�b�g�p�����[�^
typedef struct ResetParam {
	ResetParamHeader	header;
	ResetParamBody		body;
}ResetParam;


//----------------------------------------------------------------------
//�@�f�[�^�^��`
//----------------------------------------------------------------------

// SYSM���L���[�N�\����
typedef struct SYSM_work {
	vu16			isARM9Start :1;					// ARM9�X�^�[�g�t���O
	vu16			isHotStart :1;					// Hot/Cold�X�^�[�g����
	vu16			isValidResetParam :1;			// ���Z�b�g�p�����[�^�L��
	vu16			isValidTSD :1;					// NITRO�ݒ�f�[�^�����t���O
	vu16			isLogoSkip :1;					// ���S�f���X�L�b�v
	vu16			isOnDebugger :1;				// �f�o�b�K���삩�H
	vu16			isExistCard :1;					// �L����NTR/TWL�J�[�h�����݂��邩�H
	vu16			isCardStateChanged :1;			// �J�[�h��ԍX�V�t���O
	vu16			isLoadSucceeded :1;
#ifdef DEBUG_USED_CARD_SLOT_B_
	vu16			isValidCardBanner :1;
	vu16			is1stCardChecked :1;
	vu16			rsv :5;
#else
	vu16			rsv :7;
#endif
	
	u16				cardHeaderCrc16;				// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM9���ŃR�s�[���Ďg�p���鑤�j
	u16				cardHeaderCrc16_bak;			// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM7�����C�u�����Ń_�C���N�g�ɏ�������鑤�j
	OSLockWord		lockCardRsc;					// �J�[�h���\�[�X�r������p
	int				cloneBootMode;
	u32				nCardID;						// �J�[�hID
	
	ResetParam		resetParam;
	
	// NTR-IPL2�̃��K�V�[�@�ŏI�I�ɂ͏����Ǝv��
	BOOL			enableCardNormalOnly;
	u8				rtcStatus;
}SYSM_work;

// NTR�ɂ�����d�l���p������K�v�̂��郏�[�N
typedef struct SDKBootCheckInfo{
	u32 nCardID;					// NORMAL�J�[�hID				// SDK�ł͂����������Ă�����ۂ��@���ŏI�I�ɂ̓����`���[�ł����ɃJ�[�hID���Z�b�g����
	u32 sCardID;					// SECURE�J�[�hID
	u16 cardHeaderCrc16;			// �J�[�h�w�b�_CRC16
	u16 cardSecureCrc16;			// �J�[�hSECURE�̈�CRC16
	s16 cardHeaderError;			// �J�[�h�w�b�_�G���[
	s16 disableEncryptedCardData;	// �J�[�hSECURE�̈�Í����f�[�^����
	
	u16 sysromCrc16;				// �V�X�e��ROM��CRC16
	s16 enableCardNormalOnly;		// �J�[�hNORMAL���[�h�̂ݗL��
	s16 isOnDebugger;				// �f�o�b�K��œ��쒆��
	s8  rtcError;					// RTC�G���[
	u8  rtcStatus1;					// RTC�X�e�[�^�X1
	
}SDKBootCheckInfo;

//----------------------------------------------------------------------
//�@SYSM���L���[�N�̈�̃A�h���X�l��
//----------------------------------------------------------------------
// SYSM���Z�b�g�p�����[�^�A�h���X�̎擾�i�����C�u���������BARM9����SYSM_GetResetParam���g�p���ĉ������B�j
#define SYSMi_GetResetParamAddr()			( (ResetParam *)0x02000100 )

#if 0
// SYSM���L���[�N�̎擾
#define SYSMi_GetWork()						( (SYSM_work *)HW_RED_RESERVED )
#else
#define SYSMi_GetWork()						( (SYSM_work *)( HW_RED_RESERVED + 0x10 ) )
#endif

// SDK�u�[�g�`�F�b�N�i�A�v���N�����ɃJ�[�hID���Z�b�g����K�v������B�j
#define SYSMi_GetSDKBootCheckInfo()			( (SDKBootCheckInfo *)HW_BOOT_CHECK_INFO_BUF )
#define SYSMi_GetSDKBootCheckInfoForNTR()	( (SDKBootCheckInfo *)0x027ffc00 )

// NAND�t�@�[�������[�h���Ă���Ă���}�C�R���t���[���W�X�^�l�̎擾
#define SYSMi_GetMCUFreeRegisterValue()		( *(vu8 *)HW_RESET_PARAMETER_BUF )

// �J�[�hROM�w�b�_���[�N�̎擾
#define SYSM_GetCardRomHeader()				( (ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BUF )


#ifdef __cplusplus
}
#endif

#endif // __SYSMENU_WORK_H__

