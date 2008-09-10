/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     sharedFont.h

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

#ifndef	TWL_OS_SHARED_FONT_H_
#define	TWL_OS_SHARED_FONT_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

// ���L�t�H���g�C���f�b�N�X
typedef enum OSSharedFontIndex {
	OS_SHARED_FONT_WW_L = 0,
	OS_SHARED_FONT_WW_M = 1,
	OS_SHARED_FONT_WW_S = 2,
	OS_SHARED_FONT_MAX  = 3
}OSSharedFontIndex;


// ���L�t�H���g������
BOOL OS_InitSharedFont( void );

// ���L�t�H���g�@�e�[�u���T�C�Y�擾
int OS_GetSharedFontTableSize( void );

// ���L�t�H���g�@�e�[�u�����[�h
BOOL OS_LoadSharedFontTable( void *pBuffer );

// ���L�t�H���g�@�t�H���g�T�C�Y�擾
int OS_GetSharedFontSize( OSSharedFontIndex index );

// ���L�t�H���g  ���k��T�C�Y�擾
int OS_GetSharedFontCompressedSize( OSSharedFontIndex index );

// ���L�t�H���g�@�t�H���g�l�[���擾
const u8 *OS_GetSharedFontName( OSSharedFontIndex index );

// ���L�t�H���g�@�^�C���X�^���v�擾
u32 OS_GetSharedFontTimestamp( void );

// ���L�t�H���g�@�t�H���g���[�h
BOOL OS_LoadSharedFont( OSSharedFontIndex index, void *pBuffer );

#endif // SDK_ARM9

#ifdef __cplusplus
}
#endif

#endif  // TWL_OS_SHARED_FONT_H_
