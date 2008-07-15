/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     MachineSetting.h

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

#ifndef	__GET_SYSMENU_VERSION_H__
#define	__GET_SYSMENU_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <twl.h>


// define data----------------------------------------------------------
#define TWL_SYSMENU_VER_STR_LEN			28				// �V�X�e�����j���[�o�[�W����������MAX bytes
#define TWL_EULA_URL_LEN				128
#define TWL_NUP_HOSTNAME_LEN			64

// �V�X�e�����j���[�o�[�W�������������̃��[�h
extern BOOL ReadSystemMenuVersionInfo( void *pWork, u32 workSize );


// �o�[�W����������̎擾
extern const u16 *GetSystemMenuVersionString( void );


// ���W���[�o�[�W�����̎擾
extern u16 GetSystemMenuMajorVersion( void );


// �}�C�i�[�o�[�W�����̎擾
extern u16 GetSystemMenuMinorVersion( void );

// EULA URL�̎擾
extern const u8 *GetEULA_URL( void );

// NUP HostName�̎擾
extern const u8 *GetNUP_HostName( void );

// SystemMenuVersion���̃^�C���X�^���v�̎擾
extern u32 GetSystemMenuVersionTimeStamp( void );


#ifdef __cplusplus
}
#endif

#endif  // __GET_SYSMENU_VERSION_H__
