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

#ifdef __cplusplus
extern "C" {
#endif

// compile switch ---------------------------------
#ifndef SDK_FINALROM

//#define __SYSM_DEBUG

#endif // SDK_FINALROM


// define data ------------------------------------
#define SYSMENU_VER							0x071113				// SystemMenu�o�[�W����

#define PAD_PRODUCTION_SKIP_INITIAL_SHORTCUT	( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// �ʎY�H���Ŏg�p���鏉��N���ݒ���L�����Z������V���[�g�J�b�g�L�[

#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2


//----------------------------------------------------------------------
//�@�f�[�^�^��`
//----------------------------------------------------------------------

// SYSM���L���[�N�\����
typedef struct SYSM_work {
	BOOL				isValidTSD;						// NITRO�ݒ�f�[�^�����t���O
	BOOL				isOnDebugger;					// �f�o�b�K���삩�H
	BOOL				isExistCard;					// �L����NTR/TWL�J�[�h�����݂��邩�H
	BOOL				isHotStart;						// Hot/Cold�X�^�[�g����
	BOOL				isARM9Start;					// ARM9�X�^�[�g�t���O
	u16					cardHeaderCrc16;				// �V�X�e�����j���[�Ōv�Z����ROM�w�b�_CRC16
	int					cloneBootMode;
	
	// NTR-IPL2�̃��K�V�[�@�ŏI�I�ɂ͏����Ǝv��
	u32					nCardID;
	BOOL				enableCardNormalOnly;
	u8					rtcStatus;
}SYSM_work;


//----------------------------------------------------------------------
//�@SYSM���L���[�N�̈�̃A�h���X�l��
//----------------------------------------------------------------------
#if 1
#define SYSM_GetResetParam()		( (ResetParam *)HW_RED_RESERVED )

#define SYSM_GetWork()				( (SYSM_work *)( HW_RED_RESERVED + 0x40 ) )
#else
// SYSM���Z�b�g�p�����[�^�̎擾
#define SYSM_GetResetParam()		( (ResetParam *)0x02000100 )

// SYSM���L���[�N�̎擾
#define SYSM_GetWork()				( (SYSM_work *)HW_RED_RESERVED )
#endif

// �J�[�hROM�w�b�_���[�N�̎擾
#define SYSM_GetCardRomHeader()		( (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF )

#ifdef __cplusplus
}
#endif

#endif		// __SYSMENU_WORK_H__

