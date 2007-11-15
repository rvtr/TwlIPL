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

#ifdef __cplusplus
extern "C" {
#endif

// compile switch ---------------------------------
//#define SYSM_RESET_PARAM_READY_

// define data ------------------------------------
#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2


// �^�C�g�����t���O
typedef struct TitleFlags {
	u16			platform : 4;
	u16			media    : 4;
	u16			isLogoSkip : 1;
	u16			rsv : 7;
}TitleFlags;


// ���Z�b�g�p�����[�^
typedef struct ResetParam {
	NAMTitleId	bootTitleID;	// �N������^�C�g�������邩�H����Ȃ炻�̃^�C�g��ID
	u32			rsv_A;
	TitleFlags	flags;
	u8			rsv_B[ 2 ];
}ResetParam;


//----------------------------------------------------------------------
//�@�f�[�^�^��`
//----------------------------------------------------------------------

// SYSM���L���[�N�\����
typedef struct SYSM_work {
	volatile BOOL	isARM9Start;					// ARM9�X�^�[�g�t���O
	BOOL			isHotStart;						// Hot/Cold�X�^�[�g����
	BOOL			isValidTSD;						// NITRO�ݒ�f�[�^�����t���O
	BOOL			isOnDebugger;					// �f�o�b�K���삩�H
	BOOL			isExistCard;					// �L����NTR/TWL�J�[�h�����݂��邩�H
	u16				cardHeaderCrc16;				// �V�X�e�����j���[�Ōv�Z����ROM�w�b�_CRC16
	int				cloneBootMode;
	ResetParam		resetParam;
	
	// NTR-IPL2�̃��K�V�[�@�ŏI�I�ɂ͏����Ǝv��
	u32				nCardID;
	BOOL			enableCardNormalOnly;
	u8				rtcStatus;
}SYSM_work;


//----------------------------------------------------------------------
//�@SYSM���L���[�N�̈�̃A�h���X�l��
//----------------------------------------------------------------------
#ifdef SYSM_RESET_PARAM_READY_
// SYSM���Z�b�g�p�����[�^�̎擾�i�����C�u���������BARM9����SYSM_GetResetParam���g�p���ĉ������B�j
#define SYSMi_GetResetParam()		( (ResetParam *)0x02000100 )
// SYSM���L���[�N�̎擾
#define SYSMi_GetWork()				( (SYSM_work *)HW_RED_RESERVED )
#else	// SYSM_RESET_PARAM_READY_
#define SYSMi_GetResetParam()		( (ResetParam *)HW_RED_RESERVED )
#define SYSMi_GetWork()				( (SYSM_work *)( HW_RED_RESERVED + 0x40 ) )
#endif	// SYSM_RESET_PARAM_READY_

// �J�[�hROM�w�b�_���[�N�̎擾
#define SYSM_GetCardRomHeader()		( (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF )

#ifdef __cplusplus
}
#endif

#endif // __SYSMENU_WORK_H__

