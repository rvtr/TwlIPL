/********************************************************************/
/*      data.h                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	�f�[�^��`�@�w�b�_
*/

#ifndef	__DATA_H__
#define	__DATA_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro.h>
#include <fnt.h>

// define data---------------------------------------------

	// �p���b�g�J���[
#define WHITE			(  1 << 12 )
#define RED				(  2 << 12 )
#define	GREEN			(  3 << 12 )
#define	BLUE			(  4 << 12 )
#define	YELLOW			(  5 << 12 )
#define	CYAN			(  6 << 12 )
#define	PURPLE			(  7 << 12 )
#define	LIGHTGREEN		(  8 << 12 )
#define	HIGHLIGHT_Y		(  9 << 12 )
#define	HIGHLIGHT_C		( 10 << 12 )
#define	HIGHLIGHT_W		( 11 << 12 )
#define	HIGHLIGHT_B		( 12 << 12 )
#define	HIGHLIGHT_R		( 13 << 12 )


// fnt���C�u�����̃J���[�w��
#define FNT_BLACK				0
#define FNT_RED					1
#define FNT_LIGHT_GREEN			2
#define FNT_YELLOW				3
#define FNT_BLUE				4
#define FNT_PURPLE				5
#define FNT_VERMILION			6
#define FNT_WHITE				7
#define FNT_SYUIRO				8
#define FNT_GREEN				9
#define FNT_USER_COLOR			10


#define CANVAS_WIDTH 			256			// �����\���L�����p�X���h�b�g��
#define CANVAS_HEIGHT			192			// �@�@�@�V�@�@�@�@�@�c�h�b�g��
#define LINE_DOT_NUM			10


	// �L�[�f�[�^���[�N
typedef struct {
	u16 trg;									// �g���K����
	u16 cont;									// �x�^  ����
}KeyWork;


	// �^�b�`�p�l�����[�N
typedef struct {
	int		detached;							// ����̃f�[�^���͂Ń^�b�`�����ꂽ���Ƃ������B
	BOOL	initial;							// ����������́ATP���f�^�b�`�����܂ŁA�f�[�^�擾���Ȃ��悤�ɂ���B
	TPData	disp;								// ����̓��͒l�iLCD���W�j
	TPData	raw;								// ����̓��͒l�iTP ���W�j
	TPData	last;								// �O��̓��͒l�iLCD���W�j
}TpWork;


// global variables----------------------------------------
extern int				(*nowProcess)( void );
extern GXOamAttr		oamBakM[ 128 ];				// OAM �o�b�N�A�b�v
extern GXOamAttr		oamBakS[ 128 ];				// OAM �o�b�N�A�b�v
extern u16				bgBakM[ 32*24 ];			// BG  �o�b�N�A�b�v
extern u16				bgBakS[ 32*24 ];			// BG  �o�b�N�A�b�v
extern TpWork			tpd;						// �^�b�`�p�l���f�[�^
extern KeyWork			pad;						// �L�[�p�b�h���̓f�[�^

extern tFntEntry		font_m;
extern tFntDrawContext	context_m;
extern u16				canvas_m[ CANVAS_WIDTH * CANVAS_HEIGHT / 4 ] ATTRIBUTE_ALIGN(32);
extern u16				screen_m[ (CANVAS_WIDTH>>3) * (CANVAS_HEIGHT>>3) ] ATTRIBUTE_ALIGN(32);

extern tFntEntry		font_s;
extern tFntDrawContext	context_s;
extern u16				canvas_s[ CANVAS_WIDTH * CANVAS_HEIGHT / 4 ] ATTRIBUTE_ALIGN(32);
extern u16				screen_s[ (CANVAS_WIDTH>>3) * (CANVAS_HEIGHT>>3) ] ATTRIBUTE_ALIGN(32);

extern u16				s_Palette[ 0x10 ];

// global const data---------------------------------------
extern const u16 myPlttData[13][16];


// function------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif  // __DATA_H__
