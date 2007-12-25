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
#include <sysmenu/reloc_info/common/reloc_info.h>
//#include <sysmenu/reset_param/ARM9/reset_param.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
#define SYSM_RESET_PARAM_MAGIC_CODE			"TRST"
#define SYSM_RESET_PARAM_MAGIC_CODE_LEN		4

#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2

#define RELOCATE_INFO_NUM					4 // ROM�Ĕz�u���̐��i���̂Ƃ���arm9,arm7���ꂼ��ltd��flx�ōő�4�j


// NAMTitleID��HiLo�ɕ������ăA�N�Z�X����ꍇ�Ɏg�p
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;

//----------------------------------------------------------------------
//�@�f�[�^�^��`
//----------------------------------------------------------------------

// SYSM���L���[�N�\����
typedef struct SYSM_work {
	Relocate_Info	romRelocateInfo[RELOCATE_INFO_NUM];	// ROM�Ĕz�u���iarm9,arm7���ꂼ��ltd��flx�ōő�4�j
	vu32			isARM9Start :1;					// ARM9�X�^�[�g�t���O
	vu32			isHotStart :1;					// Hot/Cold�X�^�[�g����
	vu32			isValidLauncherParam :1;			// ���Z�b�g�p�����[�^�L��
	vu32			isValidTSD :1;					// NITRO�ݒ�f�[�^�����t���O
	vu32			isLogoSkip :1;					// ���S�f���X�L�b�v
	vu32			isOnDebugger :1;				// �f�o�b�K���삩�H
	vu32			isExistCard :1;					// �L����NTR/TWL�J�[�h�����݂��邩�H
	vu32			isCardStateChanged :1;			// �J�[�h��ԍX�V�t���O
	vu32			isLoadSucceeded :1;				// �A�v�����[�h�����H
	vu32			isCardBoot :1;					// �J�[�h�u�[�g���H
	vu32			isBrokenHWNormalInfo :1;		// HW�m�[�}����񂪔j�����Ă���B
	vu32			isBrokenHWSecureInfo :1;		// HW�Z�L���A��񂪔j�����Ă���B
	vu32			isResetRTC :1;					// RTC���Z�b�g����
#ifdef DEBUG_USED_CARD_SLOT_B_
	vu32			isValidCardBanner :1;
	vu32			is1stCardChecked :1;
	vu32			rsv :18;
#else
	vu32			rsv :20;
#endif
	
	u16				cardHeaderCrc16;				// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM9���ŃR�s�[���Ďg�p���鑤�j
	u16				cardHeaderCrc16_bak;			// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM7�����C�u�����Ń_�C���N�g�ɏ�������鑤�j
	OSLockWord		lockCardRsc;					// �J�[�h���\�[�X�r������p
	int				cloneBootMode;
	u32				nCardID;						// �J�[�hID
	
	LauncherParam	launcherParam;
	
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
// SYSM���Z�b�g�p�����[�^�A�h���X�̎擾�i�����C�u���������BARM9����SYSM_GetLauncherParam���g�p���ĉ������B�j
#define SYSMi_GetLauncherParamAddr()			( (LauncherParam *)HW_PARAM_LAUNCH_PARAM )

// SYSM���L���[�N�̎擾
#define SYSMi_GetWork()						( (SYSM_work *)HW_RED_RESERVED )

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

