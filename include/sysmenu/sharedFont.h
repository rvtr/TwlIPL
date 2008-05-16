/*---------------------------------------------------------------------------*
  Project:  TwlIPL
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

#ifndef	TWL_SHARED_FONT_H_
#define	TWL_SHARED_FONT_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// ���L�t�H���g�C���f�b�N�X
typedef enum SFONT_Index {
	SHARED_FONT_WW_S = 0,
	SHARED_FONT_WW_M = 1,
	SHARED_FONT_WW_L = 2,
	SHARED_FONT_MAX  = 3
}SFONT_Index;


// ���L�t�H���g������
BOOL SFONT_Init( void );

// ���L�t�H���g�@�e�[�u���T�C�Y�擾
int SFONT_GetInfoTableSize( void );

// ���L�t�H���g�@�e�[�u�����[�h
BOOL SFONT_LoadInfoTable( void *pBuffer );

// ���L�t�H���g�@�t�H���g�T�C�Y�擾
int SFONT_GetFontSize( SFONT_Index index );

// ���L�t�H���g�@�t�H���g�l�[���擾
const u8 *SFONT_GetFontName( SFONT_Index index );

// ���L�t�H���g�@�^�C���X�^���v�擾
u32 SFONT_GetFontTimestamp( void );

// ���L�t�H���g�@�t�H���g���[�h
BOOL SFONT_LoadFont( SFONT_Index index, void *pBuffer );


#ifdef __cplusplus
}
#endif

#endif  // TWL_SHARED_FONT_H_
