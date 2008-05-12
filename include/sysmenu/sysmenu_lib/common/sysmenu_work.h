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
typedef enum SYSMCloneBootMode {
    SYSM_CLONE_BOOT_MODE = 1,
    SYSM_OTHER_BOOT_MODE = 2
}
SYSMCloneBootMode;

// NAMTitleID��HiLo�ɕ������ăA�N�Z�X����ꍇ�Ɏg�p
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;

// WRAM�o�R�ŃJ�[�h�f�[�^��ǂݍ��ޏꍇ�Ɏg�p
typedef struct CardReadParam {
	u32			src;
    u32			dest;
	u32			size;
}CardReadParam;

//----------------------------------------------------------------------
//�@ROM�G�~�����[�V�������
//----------------------------------------------------------------------
#define SYSM_ROMEMU_INFO_SIZE			0x20		// ROM�G�~�����[�V�����f�[�^�T�C�Y
#define SYSM_ROMEMU_INFO_MAGIC_CODE		0x444c5754  // "TWLD"�̕�����

// IS�f�o�b�KROM�G�~�����[�V�������
typedef struct SYSMRomEmuInfo {
	// �}�W�b�N�R�[�h�iSYSM_ROMEMU_INFO_MAGIC_CODE�̌Œ�l�j
	u32			magic_code;
	// �t���O��
	u32			isEnableSlot1 : 1;
	u32			isEnableSlot2 : 1;
	u32			bootSlotNo : 2;
	u32			isEnableExMainMemory : 1;
	u32			isBootMachineSettings : 1;
	u32			isBootSpecifiedNANDApp : 1;
	u32			isTlfRom : 1;

	u32			isForceNTRMode : 1;
	u32			isForceBannerViewMode : 1;
	u32			:0;
	// isBootSpecifiedNANDApp�ŋN������A�v����TitleID
	u64			titleID;
	// �\��
	u8			rsv[ 0x10 ];
}
SYSMRomEmuInfo;


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
			vu32		isValidLauncherParam :1;		// �����`���[�p�����[�^�L��
			vu32		isValidTSD :1;					// NITRO�ݒ�f�[�^�����t���O
			vu32		isLogoSkip :1;					// ���S�f���X�L�b�v
            vu32		isHeaderLoadCompleted :1;		// �A�v���w�b�_���[�h�����H
			vu32		isLoadFinished :1;				// �A�v�����[�h�����H
			vu32		isLoadSucceeded :1;				// �A�v�����[�h�����H
			vu32		isCardBoot :1;					// �J�[�h�u�[�g���H
			vu32		isInitialSettings :1;			// ����N���V�[�P���X���H
			vu32		isBrokenHWNormalInfo :1;		// HW�m�[�}����񂪔j�����Ă���B
			vu32		isBrokenHWSecureInfo :1;		// HW�Z�L���A��񂪔j�����Ă���B
			vu32		isResetRTC :1;					// RTC���Z�b�g����
			vu32		:0;
		}common;
        struct {
            vu16		isExistCard :1;					// �L����NTR/TWL�J�[�h�����݂��邩�H
			vu16		isInspectCard :1;				// �����J�[�h���H
			vu16		isOnDebugger :1;				// �f�o�b�K���삩�H
			vu16		isEnableHotSW :1;				// �����}���L���H
			vu16		isBusyHotSW :1;					// �����}���������H
			vu16		isLoadRomEmuOnly :1;			// ROM�G�~�����[�V�������̂݃��[�h
			vu16		isCardLoadCompleted :1;			// �J�[�h����f�[�^���[�h�����H
   			vu16		isValidCardBanner :1;			// �o�i�[�f�[�^�X�V�H
			vu16		is1stCardChecked :1;			// �J�[�h�f�[�^��1st�`�F�b�N�����H
            vu16		isCardGameMode :1;				// �J�[�h���Q�[�����[�h�ɑJ�ڂ������H
            vu16		:0;
            vu8			isCardStateChanged;				// �J�[�h��ԍX�V�t���O
        }hotsw;
	}flags; // 7B

	u16					cardHeaderCrc16;				// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM9���ŃR�s�[���Ďg�p���鑤�j
	u16					cardHeaderCrc16_bak;			// �J�[�h���o���ɎZ�o����ROM�w�b�_CRC16�iARM7�����C�u�����Ń_�C���N�g�ɏ�������鑤�j
	
    OSLockWord			lockCardRsc ATTRIBUTE_ALIGN(8);	// �J�[�h���\�[�X�r������p
	OSLockWord			lockHotSW;						// �J�[�h���\�[�X�r������p
	u32					nCardID;						// �J�[�hID
	u32					gameCommondParam;				// NTR�̃Q�[���R�}���h�p�����[�^(NTR��ROM�w�b�_�̃Q�[���R�}���h�p�����[�^�ɏ㏑������)
	u8					cloneBootMode;
    
	CardReadParam		cardReadParam;					// �J�[�h���[�h�p�����[�^
    
	LauncherParam		launcherParam;
	SYSMRomEmuInfo		romEmuInfo;
	RTCRawData			Rtc1stData;						// RTC���񃍁[�h�l 8byte
	
	BOOL				isDeveloperAESMode;				// �J���p�Z�L�����e�B���H�i���i�ł�FALSE�j
	void				*addr_AESregion[2];				// AES�Í����̈�̊i�[�A�h���X
	u32					size_AESregion[2];				// AES�Í����̈�̃T�C�Y
	u8					keyAES[AES_KEY_SIZE];			// �J����AES�Í����̈�̕����Ɏg�p����KEY�i�Ɏg���^�C�g���l�[���j
	u8					idAES[GAME_CODE_MAX];			// ���i��AES�Í����̈�̕����Ɏg�p����ID�i�Ɏg���Q�[���R�[�h�j
	u8					seedAES[AES_KEY_SIZE];			// ���i��AES�Í����̈�̕����Ɏg�p����SEED
	u8					counterAES[2][AES_BLOCK_SIZE];	// AES�Í����̈�̕����Ɏg�p����J�E���^�����l
	
	// NTR-IPL2�̃��K�V�[�@�ŏI�I�ɂ͏����Ǝv��
	BOOL				enableCardNormalOnly;
	u8					rtcStatus;
}SYSM_work;

typedef struct SYSM_work2 {
	SVCHMACSHA1Context hmac_sha1_context;
	TitleProperty		bootTitleProperty;
	char 				bootContentPath[ FS_ENTRY_LONGNAME_MAX ];
}SYSM_work2;

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
// SYSM�����`���[�p�����[�^�A�h���X�̎擾�i�����C�u���������BARM9����SYSM_GetLauncherParam���g�p���ĉ������B�j
#define SYSMi_GetLauncherParamAddr()			( (LauncherParam *)HW_PARAM_LAUNCH_PARAM )

// SYSM���L���[�N�̎擾
#define SYSMi_GetWork()						( (SYSM_work *)HW_TWL_MAIN_MEM_SHARED )
#define SYSMi_GetWork2()					( (SYSM_work2 *)HW_MAIN_MEM_SHARED )

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

