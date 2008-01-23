/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     internal_api.h

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

#ifndef SYSM_INTERNAL_API_H_
#define SYSM_INTERNAL_API_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define SYSM_LIB_NO_MESSAGE					// Printf�}���X�C�b�`

#ifdef	SYSM_LIB_NO_MESSAGE
#define OS_Printf( ... )					((void)0)
#define OS_TPrintf( ... )					((void)0)
#define OS_PutString( ... )					((void)0)
#endif


//-------------------------------------------------------
// �}�E���g���Z�b�g
//-------------------------------------------------------

// �����`���[�p
void SYSMi_SetLauncherMountInfo( void );

// �N���A�v���p
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle );


//-------------------------------------------------------
// �f�o�C�X
//-------------------------------------------------------

// RTC�␳
void SYSMi_WriteAdjustRTC( void );

// RTC�`�F�b�N
void SYSMi_CheckRTC( void );


//-------------------------------------------------------
// �o�i�[
//-------------------------------------------------------

// �J�[�h�o�i�[���[�h�i��NTR-IPL2�d�l�j
BOOL SYSMi_ReadCardBannerFile( u32 bannerOffset, TWLBannerFile *pBanner );


#ifdef __cplusplus
}
#endif

#endif  // SYSM_INTERNAL_API_H_
