/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     types.h

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

#ifndef	__SYSM_TYPES_H__
#define	__SYSM_TYPES_H__

#include <twl.h>
#include <twl/os/common/format_rom.h>
#include <../build/libraries/os/common/include/application_jump_private.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

// NAMTitleID��HiLo�ɕ������ăA�N�Z�X����ꍇ�Ɏg�p
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;


// �^�C�g�����T�u
typedef struct TitleInfoSub {
	RomExpansionFlags	exFlags;
	char				platform_code;
	u8					parental_control_rating_info[ PARENTAL_CONTROL_INFO_SIZE ];
	u32					card_region_bitmap;
	u8					agree_EULA_version;
}TitleInfoSub;


// �^�C�g�����
typedef struct TitleProperty {			// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	NAMTitleId			titleID;		// �^�C�g��ID�iTitleID_Hi�ŋN�����f�B�A�͔���ł���H�j
	LauncherBootFlags	flags;			// �u�[�g���̃����`���[����t���O
	TWLBannerFile		*pBanner;		// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
	TitleInfoSub		sub_info;
}TitleProperty;


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_TYPES_H__
