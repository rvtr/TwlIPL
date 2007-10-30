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
#include <twl/rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

// compile switch ---------------------------------
#ifndef SDK_FINALROM

//#define __SYSM_DEBUG

#endif // SDK_FINALROM

//#define __DEBUG_SECURITY_CODE										// PassMe�̃Z�L�����e�B�R�[�h�m�F�p�X�C�b�`


// define data ------------------------------------
#define SYSMENU_VER							0x071029					// SystemMenu�o�[�W����

#define PXI_FIFO_TAG_SYSM					PXI_FIFO_TAG_USER_1			// SystemMenu�p��FIFO�^�O

#define PAD_PRODUCTION_NITRO_SHORTCUT		( PAD_BUTTON_A | PAD_BUTTON_B	\
											| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
#define PAD_PRODUCTION_AGB_SHORTCUT			( PAD_BUTTON_A | PAD_BUTTON_B	\
											| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_L )
																		// �ʎY�H���Ŏg�p����NITRO����N���ݒ���L�����Z������V���[�g�J�b�g�L�[


	// bootFlag�̒l
#define BFLG_EXIST_AGB_CARTRIDGE			0x00000001
#define BFLG_EXIST_NITRO_CARD				0x00000002
#define BFLG_ILLEGAL_NITRO_CARD				0x00000004
#define BFLG_ILLEGAL_BMENU					0x00000008
#define BFLG_BOOT_AGB						0x00000010
#define BFLG_BOOT_NITRO						0x00000020
#define BFLG_BOOT_BMENU						0x00000040
#define BFLG_BOOT_PICT_CHAT					0x00000080
#define BFLG_BOOT_WIRELESS_BOOT				0x00000100
#define BFLG_LOAD_CARD_COMPLETED			0x00000200
#define BFLG_LOAD_BMENU_COMPLETED			0x00000400
#define BFLG_LOAD_SYSM_DATA_COMPLETED		0x00000800
#define BFLG_REQ_UNCOMP_BMENU				0x00001000
#define BFLG_REQ_UNCOMP_SYSM_DATA			0x00002000
#define BFLG_ARM7_INIT_COMPLETED			0x00004000
#define BFLG_READ_NCD_COMPLETED				0x00008000
#define BFLG_SHORTCUT_CHECK_COMPLETED		0x00010000
#define BFLG_HOT_START						0x00020000
#define BFLG_BOOT_1SEG						0x00040000
#define BFLG_PERMIT_TO_BOOT					0x08000000
#define BFLG_SYSM_DATA_ENABLE				0x10000000
#define BFLG_CARD_CHECKED					0x20000000
#define BFLG_WM_INITIALIZED					0x40000000
#define BFLG_BOOT_DECIDED					0x80000000

#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2

	// mainp_state�̒l
typedef enum MainpState {
	MAINP_STATE_INIT = 1,
	MAINP_STATE_START,
	MAINP_STATE_WAIT_BOOT_DECISION,
	MAINP_STATE_WAIT_NITRO_GAME_LOAD,
	MAINP_STATE_WAIT_READY_CHANGE_AGB,
	MAINP_STATE_WAIT_BMENU_LOAD,
	MAINP_STATE_WAIT_BOOT_REQ,
	MAINP_STATE_WAIT_START_NITRO_GAME_REQ,
	MAINP_STATE_BOOT_SELECTED_TARGET,
	MAINP_STATE_BOOT_AGB_REQ
}MainpState;


	// subp_state�̒l
typedef enum SubpState {
	SUBP_STATE_INIT = 1,
	SUBP_STATE_STAY,
	SUBP_STATE_CLEAR_MAIN_MEMORY,
	SUBP_STATE_BOOT_NITRO_GAME_INIT,
	SUBP_STATE_LOAD_NITRO_GAME,
	SUBP_STATE_LOAD_BMENU,
	SUBP_STATE_BOOT_NITRO_GAME,
	SUBP_STATE_WAIT_START_BMENU_REQ,
	SUBP_STATE_START_BMENU,
	SUBP_STATE_BOOT_AGB,
	SUBP_STATE_BOOT_AGB_ACK,
	SUBP_STATE_BOOT_FAILED,
	SUBP_STATE_MB_BOOT,
	SUBP_STATE_TERMINATE_WM
}SubpState;


	// SYSMi_SendMessageToARM7(int msg)��ARM9����ARM7�ɒʒm���郁�b�Z�[�W
	// ARM7����̃��b�Z�[�W���܂�
typedef enum SYSMMsg {
	MSG_INVALID = 0,												// �����f�[�^�B
	
	MSG_UNCOMP_SYSM_DATA,											// ARM9��SYSM_data�����k�W�J����悤�v���B
	MSG_UNCOMP_BMENU,												// ARM9��bmenu�����k�W�J����悤�v���B
	
	MSG_BOOT_TYPE_NITRO,											// ARM7�ɁuNITRO�Q�[���N���v��ʒm�B
	MSG_BOOT_TYPE_AGB,												// ARM7�ɁuAGB�N���v��ʒm�B
#ifndef __DS_CHAT_OFF
	MSG_BOOT_TYPE_PICT_CHAT,										// ARM7�Ɂu�G�`���b�g�N���v��ʒm�B
#endif
	MSG_BOOT_TYPE_WIRELESS_BOOT,									// ARM7�Ɂu�����}���`�u�[�g�N���v��ʒm�B
	MSG_BOOT_TYPE_BMENU,											// ARM7�Ɂu�u�[�g���j���[�N���v��ʒm�B
	MSG_START_BMENU,												// ARM7�Ɂu�u�[�g���j���[�J�n�v��ʒm�B
	MSG_TERMINATE_WM												// ARM7�ɁuWM�I���v��ʒm�B
}SYSMMsg;


//----------------------------------------------------------------------
//�@�f�[�^�^��`
//----------------------------------------------------------------------

// ���b�N���
typedef struct LockVariable{
	OSLockWord			lock;
	vu32				value;
}LockVariable;

// RTC���t�����\����
typedef struct RtcDateTime {
	RTCDate				Date;
	RTCTime				Time;
}RtcDateTime;

// SYSM���L���[�N�\����
typedef struct SYSM_work{
	u32					card_arm7_ram_adr;							// NITRO�J�[�hARM7�����u�[�g�R�[�h��RAM���[�h�A�h���X
	int					ncd_invalid;								// NITRO�ݒ�f�[�^�����t���O
	u32					ncd_rom_adr;								// NITRO�ݒ�f�[�^��ROM�A�h���X
	u32					bm_arm7_ram_adr;							// �u�[�g���j���[ARM9RAM�A�h���X
	u32					bm_arm7_comp_adr;							// �u�[�g���j���[ARM7�̈��k�o�C�i��RAM�A�h���X
	u16					sysm_data_crc16;
	u16					bm_crc16;
	u8					sysm_type;
	u8					pmic_type;									// �f�o�b�K�݂̂Ŏg�p�B
	u8					clone_boot_mode;
	
	
	u8					rtcStatus;
	u16					cardHeaderCrc16;
	u16					rsv;
	BOOL				isOnDebugger;
	BOOL				enableCardNormalOnly;
	u32					nCardID;									// NORMAL�J�[�hID�iLoadCardHeader() �Ŏ擾�j
	
	volatile MainpState	mainp_state;								// ARM9�v���O�����X�e�[�g
	volatile SubpState	subp_state;									// ARM7�v���O�����X�e�[�g
	LockVariable		boot_flag;									// �u�[�g��ԃt���O�iSYSM_GetBootFlag(),SetBootFlag()�ŃA�N�Z�X���s���܂��B�j
	RtcDateTime			rtc[2];										// RTC���ԃf�[�^([0]:�N�����̒l�A[1]:�Q�[���u�[�g���O�̒l�j
//	u32					mb_flag;
//	u32					mb_ggid;
}SYSM_work;


//----------------------------------------------------------------------
//�@SYSM���L���[�N�̈�̃A�h���X�l��
//----------------------------------------------------------------------

#define SYSM_GetResetParam()		( (ResetParam *)HW_RED_RESERVED )

#define GetSYSMWork()				( (SYSM_work *)( HW_RED_RESERVED + sizeof(ResetParam) ) )

//�ESYSM���L���[�N�̈�̃A�h���X���l�����܂��B

//----------------------------------------------------------------------
//�@bootFlag�̃��[�h
//----------------------------------------------------------------------
#define SYSM_GetBootFlag()			( *(vu32 *)&GetSYSMWork()->boot_flag.value )

//�EbootFlag�l���l�����܂��B


#ifdef __cplusplus
}
#endif

#endif		// __SYSMENU_WORK_H__

