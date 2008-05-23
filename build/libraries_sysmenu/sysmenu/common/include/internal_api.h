/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     internal_api.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

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
#include <sysmenu/sysmenu_lib/common/pxi.h>

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

#ifdef SDK_ARM9

// BootSRLPath�󂯓n���p
void SYSMi_SetBootSRLPathToWork2( TitleProperty *pBootTitle );

#else // !SDK_ARM9

// �����`���[�p
void SYSMi_SetLauncherMountInfo( void );

// �N���A�v���p
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle );

#endif // SDK_ARM9


#ifdef SDK_ARM9
//-------------------------------------------------------
// �f�o�C�X
//-------------------------------------------------------

// RTC�␳
void SYSMi_WriteAdjustRTC( void );

// RTC�`�F�b�N
void SYSMi_CheckRTC( void );


#endif // SDK_ARM9


#ifdef SDK_ARM7
//-------------------------------------------------------
// AES���ݒ�
//-------------------------------------------------------

// JPEG�����p�i�����`���[�A�A�v���u�[�g���p�j
void SYSMi_SetAESKeysForSignJPEG( ROM_Header *pROMH, BOOL *pIsClearSlotB, BOOL *pIsClearSlotC );

// �A�v���u�[�g�p
void SYSMi_SetAESKeysForAccessControl( BOOL isNtrMode, ROM_Header *pROMH );

#endif // SDK_ARM7


//=======================================================
//
// ARM9/ARM7����API
//
//=======================================================
BOOL SYSMi_IsDebuggerBannerViewMode( void );
BOOL SYSMi_CheckEntryAddress( void );
BOOL SYSMi_CopyCardRomHeader( void );
BOOL SYSMi_CopyCardBanner( void );


#ifdef __cplusplus
}
#endif

#endif  // SYSM_INTERNAL_API_H_
