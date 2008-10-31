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
#include <sysmenu/types.h>
#include <sysmenu/memorymap.h>
#include <sysmenu/reloc_info/common/reloc_info.h>
#include <firm/gcd/blowfish.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
typedef enum SYSMCloneBootMode {
    SYSM_CLONE_BOOT_MODE = 1,
    SYSM_OTHER_BOOT_MODE = 2
}
SYSMCloneBootMode;

typedef enum CardDataReadState {
	CARD_READ_SUCCESS = 0,
    CARD_READ_TIME_OUT,
    CARD_READ_PULLED_OUT_ERROR,
    CARD_READ_BUFFER_OVERRUN_ERROR,
    CARD_READ_MODE_ERROR,
    CARD_READ_BUSY,
    CARD_READ_ID_CHECK_ERROR,
    CARD_READ_BUS_LOCK_ERROR,
    CARD_READ_UNEXPECTED_ERROR
}
CardDataReadState;

// WRAM�o�R�ŃJ�[�h�f�[�^��ǂݍ��ޏꍇ�Ɏg�p
typedef struct CardReadParam {
	u32					src;
    u32					dest;
	u32					size;
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
			vu8			isHotStart :1;					// Hot/Cold�X�^�[�g����
			vu8			isValidLauncherParam :1;		// �����`���[�p�����[�^�L��
			vu8			isResetRTC :1;					// RTC���Z�b�g����
			vu8			isNANDFatalError :1;			// NANDFATAL�G���[����
			vu8			isARM9Start :1;					// ARM9�X�^�[�g�t���O
			vu8			:0;
		}arm7;

		struct {
			vu8			isValidTSD :1;					// NITRO�ݒ�f�[�^�����t���O
			vu8			isLogoSkip :1;					// ���S�f���X�L�b�v
		    vu8			isHeaderLoadCompleted :1;		// �A�v���w�b�_���[�h�����H
			vu8			isLoadFinished :1;				// �A�v�����[�h�����H
			vu8			isLoadSucceeded :1;				// �A�v�����[�h�����H
			vu8			isCardBoot :1;					// �J�[�h�u�[�g���H
			vu8			:0;
		}arm9;

        struct {
            vu16		isExistCard :1;					// �L����NTR/TWL�J�[�h�����݂��邩�H
			vu16		isInspectCard :1;				// �����J�[�h���H
			vu16		isOnDebugger :1;				// �f�o�b�K���삩�H
			vu16		isEnableHotSW :1;				// �����}���L���H
			vu16		isLoadRomEmuOnly :1;			// ROM�G�~�����[�V�������̂݃��[�h
			vu16		isCardLoadCompleted :1;			// �J�[�h����f�[�^���[�h�����H
   			vu16		isValidCardBanner :1;			// �o�i�[�f�[�^�X�V�H
			vu16		is1stCardChecked :1;			// �J�[�h�f�[�^��1st�`�F�b�N�����H
            vu16		isCardGameMode :1;				// �J�[�h���Q�[�����[�h�ɑJ�ڂ������H
            vu16		isFinalized :1;					// HOTSW�I����������
            vu16		:0;
            vu8			isCardStateChanged;				// �J�[�h��ԍX�V�t���O
            vu8			isBusyHotSW;					// �����}���������H
            vu8			isKeyTableLoadReady;			// Key Table�̃��[�h���������H
            vu16		romHeaderCRC;
            vu16		secure1CRC;
            vu16		secure2CRC;
        }hotsw;
	}flags; // 9B

    OSLockWord			lockCardRsc ATTRIBUTE_ALIGN(8);	// �J�[�h���\�[�X�r������p
	OSLockWord			lockHotSW;						// �J�[�h���\�[�X�r������p
	u32					appCardID;						// �J�[�hID
	OSBootType			appBootType;					// �u�[�g���
	u32					gameCommondParam;				// NTR�̃Q�[���R�}���h�p�����[�^(NTR��ROM�w�b�_�̃Q�[���R�}���h�p�����[�^�ɏ㏑������)
	u8					cloneBootMode;
    
	CardReadParam		cardReadParam;					// �J�[�h���[�h�p�����[�^
	u32					romHeaderNTR[HW_CARD_ROM_HEADER_SIZE/sizeof(u32)];  // NTR-ROM�w�b�_�ꎞ�o�b�t�@
    
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


// ARM9����ARM7��WRAM�o�R�ň����n������񃏁[�N
typedef struct DeliverBROM9Key {
	BLOWFISH_CTX	ds_blowfish;
//	u8              aes_key[ AES_KEY_SIZE ];
}DeliverBROM9Key;


//----------------------------------------------------------------------
//�@SYSM���L���[�N�̈�̃A�h���X�l��
//----------------------------------------------------------------------
// SYSM�����`���[�p�����[�^�A�h���X�̎擾�i�����C�u���������BARM9����SYSM_GetLauncherParam���g�p���ĉ������B�j
#define SYSMi_GetLauncherParamAddr()			( (LauncherParam *)HW_PARAM_LAUNCH_PARAM )

// SYSM���L���[�N�̎擾
#define SYSMi_GetWork()						( (SYSM_work *)HW_TWL_SHARED_RESERVED )
#define SYSMi_GetWork2()					( (SYSM_work2 *)HW_MAIN_MEM_SHARED )

// SDK�u�[�g�`�F�b�N�i�A�v���N�����ɃJ�[�hID���Z�b�g����K�v������B�j
#define SYSMi_GetSDKBootCheckInfo()			( (SDKBootCheckInfo *)HW_BOOT_CHECK_INFO_BUF )
#define SYSMi_GetSDKBootCheckInfoForNTR()	( (SDKBootCheckInfo *)0x027ffc00 )

// NAND�t�@�[�������[�h���Ă���Ă���}�C�R���t���[���W�X�^�l�̎擾
#define SYSMi_GetMCUFreeRegisterValue()		( *(vu8 *)HW_RESET_PARAMETER_BUF )

// ROM�w�b�_���[�N�̎擾
#define SYSM_GetAppRomHeader()				( (ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF )
#define SYSM_GetCardRomHeader()				SYSM_GetAppRomHeader()

// ARM9��������n������񃏁[�N�̎擾
#ifdef SDK_ARM9
#define GetDeliverBROM9KeyAddr()			( (DeliverBROM9Key *)HW_WRAM_0 )
#else
#define GetDeliverBROM9KeyAddr()			( (DeliverBROM9Key *)HW_WRAM_0_LTD )
#endif

// IS�f�o�b�K��œ��삵�Ă��邩�H
static inline BOOL SYSM_IsRunOnDebugger( void )
{
#ifdef SYSM_BUILD_FOR_DEBUGGER
	return SYSMi_GetWork()->flags.hotsw.isOnDebugger;
#else
	return FALSE;
#endif
}



#ifdef __cplusplus
}
#endif

#endif // __SYSMENU_WORK_H__

