/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     font.h

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

#ifndef	__FONT_H_
#define	__FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl.h>

// define data----------------------------------
#define	STR_ENTRY_MAX_NUM				256					// �o�^�\�ȕ�����f�[�^�̍ő��

#define SJIS_CHAR_VRAM_OFFSET			0x4000				// SJIS������L�����N�^�pVRAM�̃I�t�Z�b�g�l
#define SJIS_CHAR_VRAM_SIZE				(0x8000 + 0x20)		// �@�@�V�@�@�@�@�@�@�@�@�@�@�̃T�C�Y�i0x20�̓q�[�v�̃w�b�_�j

#define VRAM_M_ARENA_LO					(HW_BG_VRAM    + SJIS_CHAR_VRAM_OFFSET - 0x20)
#define VRAM_M_ARENA_HI					(VRAM_M_ARENA_LO + SJIS_CHAR_VRAM_SIZE)
#define VRAM_S_ARENA_LO					(HW_DB_BG_VRAM + SJIS_CHAR_VRAM_OFFSET - 0x20)
#define VRAM_S_ARENA_HI					(VRAM_S_ARENA_LO + SJIS_CHAR_VRAM_SIZE)
															// VRAM�A���[�i��Lo & Hi
	// SJIS�R�[�h����p�̒l
#define SJIS_HIGHER_CODE1_MIN			0x81
#define SJIS_HIGHER_CODE1_MAX			0x9f
#define SJIS_HIGHER_CODE2_MIN			0xe0
#define SJIS_HIGHER_CODE2_MAX			0xea

// �֐��̃G���[���^�[���l
#define DSJIS_ERR_ENTRY_GET_FAILED		0x8000
#define DSJIS_ERR_ENTRY_ALLOC_FAILED	0x8001
#define DSJIS_ERR_CHAR_ALLOC_FAILED		0x8002
#define DSJIS_ERR_STR_MEMORY_OVER		0x8003
#define DSJIS_ERR_STR_LENGTH_TOO_LONG	0x8004

// SetTargetScreenSJIS�̈���target
typedef enum TargetScreen {
	TOP_SCREEN =0,
	BOTTOM_SCREEN
}TargetScreen;

// �t�H���g��ރf�[�^�iSelectFont�Ŏw��j
typedef enum FontType{										// �S�p�@�@���p
	FONT12,													// 12x12 & 12x 7dot
	FONT_TYPE_MAX
}FontType;


// function's prototype declaration-------------

void InitFont( TargetScreen target );
void SetFont( FontType font );
void SetTargetScreenSJIS( TargetScreen target );
u16  ChangeColorSJIS( u16 handle, u16 new_color );

// �ȉ��̕\���֐��́A�f�[�^�A�h���X����f�[�^�n���h�����Z�o����̂ŁA�n���h���������ŗ^���Ȃ��ėǂ����A����A�h���X�̃f�[�^�𕡐��ꏊ�ɔz�u���邱�Ƃ��ł��Ȃ��B
u16  DrawStringSJIS ( int x, int y, u16 color, const void *str );
u16  DrawHexSJIS    ( int x, int y, u16 color, const void *hexp, u16 figure );
u16  DrawDecimalSJIS( int x, int y, u16 color, const void *hexp, u16 figure, u16 size );

// Ex�n�́A������index��݂��邱�ƂŁA��L�֐��Ő�������Ă��铯��A�h���X�f�[�^�̕����ꏊ�z�u�ɑΉ����Ă���B
u16	 DrawStringSJISEx ( int x, int y, u16 color, const void *strp, u32 index );
u16  DrawHexSJISEx    ( int x, int y, u16 color, const void *hexp, u16 figure, u32 index );
u16  DrawDecimalSJISEx( int x, int y, u16 color, const void *hexp, u16 figure, u16 size, u32 index );

// �\��������N���A�֐�
void ClearStringSJIS( void *datap );
void ClearStringSJISEx( void *datap, u32 handleIndex );
void ClearStringSJIS_handle( u16 handle );
void ClearAllStringSJIS( void );


#ifdef __cplusplus
}
#endif

#endif		// __FONT_H_

