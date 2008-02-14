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

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
#define TITLE_ID_LAUNCHER					( 0x000300074c4e4352LLU )
#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2


// NAMTitleID��HiLo�ɕ������ăA�N�Z�X����ꍇ�Ɏg�p
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;


//----------------------------------------------------------------------
//�@ROM�G�~�����[�V�������
//----------------------------------------------------------------------
#define ISD_ROM_EMULATION_INFO_SIZE			0x20		// ROM�G�~�����[�V�����f�[�^�T�C�Y
#define ISD_ROM_EMULATION_INFO_MAGIC_CODE	0x444c5754  // "TWLD"�̕�����

// IS�f�o�b�KROM�G�~�����[�V�������
typedef struct ISD_RomEmuInfo {
	// �}�W�b�N�R�[�h�iISD_ROM_EMULATION_INFO_MAGIC_CODE�̌Œ�l�j
	u32			magic_code;
	// �t���O��
	u32			isEnableSlot1 : 1;
	u32			isEnableSlot2 : 1;
	u32			bootSlotNo : 2;
	u32			isEnableExMainMemory : 1;
	u32			isBootMachineSettings : 1;
	u32			isBootSpecifiedNANDApp : 1;
	u32			isForceNTRMode : 1;
	u32			isForceBannerViewMode : 1;
	u32			rsv_flags : 23;
	// isBootSpecifiedNANDApp�ŋN������A�v����TitleID
	u64			titleID;
	// �\��
	u8			rsv[ 0x10 ];
}ISD_RomEmuInfo;


//----------------------------------------------------------------------
//�@SYSM���[�N
//----------------------------------------------------------------------
// SYSM���L���[�N�\����
typedef struct SYSM_work {
	Relocate_Info		romRelocateInfo[RELOCATE_INFO_NUM];	// ROM�Ĕz�u���iarm9,arm7���ꂼ��ltd��flx�ōő�4�j
	struct {
		struct {
			vu32		isFatalError :1;				// FATAL�G���[
			vu32		isARM9Start :1;					// ARM9�X�^�[�g�t���O
			vu32		isHotStart :1;					// Hot/Cold�X�^�[�g����
			vu32		isValidLauncherParam :1;			// ���Z�b�g�p�����[�^�L��
			vu32		isValidTSD :1;					// NITRO�ݒ�f�[�^�����t���O
			vu32		isLogoSkip :1;					// ���S�f���X�L�b�v
			vu32		isOnDebugger :1;				// �f�o�b�K���삩�H
			vu32		isExistCard :1;					// �L����NTR/TWL�J�[�h�����݂��邩�H
			vu32		isCardStateChanged :1;			// �J�[�h��ԍX�V�t���O
			vu32		isLoadSucceeded :1;				// �A�v�����[�h�����H
			vu32		isCardBoot :1;					// �J�[�h�u�[�g���H
			vu32		isBrokenHWNormalInfo :1;		// HW�m�[�}����񂪔j�����Ă���B
			vu32		isBrokenHWSecureInfo :1;		// HW�Z�L���A��񂪔j�����Ă���B
			vu32		isResetRTC :1;					// RTC���Z�b�g����
			vu16		isEnableHotSW :1;				// �����}���L���H
			vu16		isBusyHotSW :1;					// �����}���������H
            vu16		isCardLoadCompleted :1;			// �J�[�h����f�[�^���[�h�����H
#ifdef DEBUG_USED_CARD_SLOT_B_
			vu32		isValidCardBanner :1;
			vu32		is1stCardChecked :1;
			vu32		rsv :14;
#else
			vu32		rsv :16;
#endif
		}common;
		struct {
			vu16		reqChangeHotSW :1;
			vu16		nextHotSWStatus :1;
			vu16		rsv:15;
		}arm9;
		struct {
			vu16		rsv:16;
		}arm7;
	}flags;
	
	u16					cardHeaderCrc16;				// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM9���ŃR�s�[���Ďg�p���鑤�j
	u16					cardHeaderCrc16_bak;			// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM7�����C�u�����Ń_�C���N�g�ɏ�������鑤�j
	OSLockWord			lockCardRsc;					// �J�[�h���\�[�X�r������p
	OSLockWord			lockHotSW;						// �J�[�h���\�[�X�r������p
	int					cloneBootMode;
	u32					nCardID;						// �J�[�hID
	u32					gameCommondParam;				// NTR�̃Q�[���R�}���h�p�����[�^(NTR��ROM�w�b�_�̃Q�[���R�}���h�p�����[�^�ɏ㏑������)
    
	LauncherParam		launcherParam;
	ISD_RomEmuInfo		romEmuInfo;
	
	// NTR-IPL2�̃��K�V�[�@�ŏI�I�ɂ͏����Ǝv��
	BOOL				enableCardNormalOnly;
	u8					rtcStatus;
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

