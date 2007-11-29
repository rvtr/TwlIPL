/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     reset_param.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-29#$
  $Rev: 72 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#ifndef _RESET_PARAM_H_
#define _RESET_PARAM_H_

#include <twl.h>
#include <twl/nam.h>
#include <spi.h>


#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------

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

// function's prototype------------------------------------

void RP_Reset( u8 type, NAMTitleId id, BootFlags *flag );

#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // _RESET_PARAM_H_
