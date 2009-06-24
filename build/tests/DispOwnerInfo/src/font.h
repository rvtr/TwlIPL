/********************************************************************/
/*      font.c                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	�t�H���g�����w�b�_


	$Log: font.h,v $
	Revision 1.2  2005/03/09 04:44:39  yosiokat
	�@�\�ǉ��B
	
	Revision 1.1.1.1  2004/08/31 06:20:24  Yosiokat
	no message
	



	// **** old logs ****

	Revision 1.7  2004/08/18 07:17:26  Yosiokat
	�㉺LCD���^�[�Q�b�g�ɂ��āA�ʌɏ������ł���悤�ύX�B
	
	Revision 1.6  2004/08/17 07:52:03  Yosiokat
	�ESetTargetScreenSJIS��ǉ����āA�㉺LCD�̂ǂ���ɂ������\�����\�ɂȂ�悤�ύX�B
	
	Revision 1.5  2004/08/07 05:44:43  Yosiokat
	�ESJIS������\���֐��������Ńn���h�����w�肵�Ȃ��d�l�ɕύX����B
	�E��L�ύX�ɑΉ����āA�N���A�֐����d�l�ύX�B
	
	Revision 1.4  2004/07/13 00:31:48  Yosiokat
	�E�T�uLCD����VRAM��Ώۂɂ���悤�ύX�B
	
	Revision 1.3  2004/06/06 02:39:31  Yosiokat
	SJIS�R�[�h����p�̒萔��`��font.h�Ɉړ��B
	
	Revision 1.2  2004/05/26 01:16:57  Yosiokat
	���������SJIS�x�[�X�ɕύX���B
	
	Revision 1.1  2004/05/25 08:59:22  Yosiokat
	�������SJIS�Ő��䂷��悤�ɕύX�B
	

*/

#ifndef	__FONT_H_
#define	__FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

// define data----------------------------------
#define	STR_ENTRY_MAX_NUM				256					// �o�^�\�ȕ�����f�[�^�̍ő��

#define SJIS_CHAR_VRAM_OFFSET			0x100				// SJIS������L�����N�^�pVRAM�̃I�t�Z�b�g�l
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
u16  DrawStringSJIS ( u16 x, u16 y, u16 color, const void *str );
u16  DrawHexSJIS    ( u16 x, u16 y, u16 color, const void *hexp, u16 figure );
u16  DrawDecimalSJIS( u16 x, u16 y, u16 color, const void *hexp, u16 figure, u16 size );

// Ex�n�́A������index��݂��邱�ƂŁA��L�֐��Ő�������Ă��铯��A�h���X�f�[�^�̕����ꏊ�z�u�ɑΉ����Ă���B
u16	 DrawStringSJISEx ( u16 x, u16 y, u16 color, const void *strp, int index );
u16  DrawHexSJISEx    ( u16 x, u16 y, u16 color, const void *hexp, u16 figure, int index );
u16  DrawDecimalSJISEx( u16 x, u16 y, u16 color, const void *hexp, u16 figure, u16 size, int index );

// �\��������N���A�֐�
void ClearStringSJIS( void *datap );
void ClearStringSJISEx( void *datap, int handleIndex );
void ClearStringSJIS_handle( u16 handle );
void ClearAllStringSJIS( void );


#ifdef __cplusplus
}
#endif

#endif		// __FONT_H_

