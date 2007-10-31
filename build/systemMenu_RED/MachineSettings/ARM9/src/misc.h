/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     misc.h

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

#ifndef	__MISC_H__
#define	__MISC_H__

#include <twl.h>
#include <sysmenu.h>

#define NNS_G2D_UNICODE
#include <nnsys.h>
#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/g2d_TextCanvas.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
#define TP_CSR_TOUCH_COUNT					2						// TP�J�[�\���̃`���^�����O�z���̂��߂̃J�E���g�l
#define TP_CSR_DETACH_COUNT					2						// TP�J�[�\�����u�I���v�Ɣ��肷��TP�f�^�b�`����̃J�E���g�l

// DrawText �ł̍����
#define TXT_DRAWTEXT_FLAG_DEFAULT   (NNS_G2D_VERTICALORIGIN_TOP | NNS_G2D_HORIZONTALORIGIN_LEFT | NNS_G2D_HORIZONTALALIGN_LEFT)

// TXTColorPalette �̐F�� 16�F�p���b�g�ւ̃��[�h��z��
enum
{
    // �p���b�g0 TXT_CPALETTE_MAIN
    TXT_COLOR_NULL=0,
    TXT_COLOR_WHITE,
    TXT_COLOR_BLACK,
    TXT_COLOR_RED,
    TXT_COLOR_GREEN,
    TXT_COLOR_BLUE,
    TXT_COLOR_CYAN,
    TXT_COLOR_MAGENTA,
    TXT_COLOR_YELLOW,

    // �p���b�g1 TXT_CPALETTE_USERCOLOR
    TXT_UCOLOR_NULL=0,
    TXT_UCOLOR_GRAY,
    TXT_UCOLOR_BROWN,
    TXT_UCOLOR_RED,
    TXT_UCOLOR_PINK,
    TXT_UCOLOR_ORANGE,
    TXT_UCOLOR_YELLOW,
    TXT_UCOLOR_LIMEGREEN,
    TXT_UCOLOR_DARKGREEN,
    TXT_UCOLOR_SEAGREEN,
    TXT_UCOLOR_TURQUOISE,
    TXT_UCOLOR_BLUE,
    TXT_UCOLOR_DARKBLUE,
    TXT_UCOLOR_PURPLE,
    TXT_UCOLOR_VIOLET,
    TXT_UCOLOR_MAGENTA,

    // �p���b�g TXT_CPALETTE_4BPP
    TXT_COLOR_4BPP_NULL=0,
    TXT_COLOR_4BPP_BG=1,
    TXT_COLOR_4BPP_TEXT=1
};


// ���v�\���ꏊ
#define RTC_DATE_TOP_X				(  9 * 8 )
#define RTC_DATE_TOP_Y				( 10 * 8 )
#define RTC_TIME_TOP_X				( 12 * 8 )
#define RTC_TIME_TOP_Y				( 12 * 8 )


// �L�[�f�[�^���[�N
typedef struct {
	u16 trg;									// �g���K����
	u16 cont;									// �x�^  ����
}KeyWork;


// �^�b�`�p�l�����[�N
typedef struct {
	int	   detached;							// ����̃f�[�^���͂Ń^�b�`�����ꂽ���Ƃ������B
	TPData disp;								// ����̓��͒l�iLCD���W�j
	TPData raw;									// ����̓��͒l�iTP ���W�j
	TPData last;								// �O��̓��͒l�iLCD���W�j
}TpWork;


// ���j���[�v�f���W
typedef struct MenuPos {
	BOOL		enable;
	int			x;
	int			y;
}MenuPos;


// ���j���[�\���p�����[�^�\����
typedef struct MenuParam {
	int			num;
	int			normal_color;
	int			select_color;
	int			disable_color;
	MenuPos		*pos;
	const u16	**str_elem;
}MenuParam;


// RTC�f�[�^�\���ʒu���[�N
typedef struct RTCDrawProperty {
	BOOL	isTopLCD;
	int		date_x;
	int		date_y;
	int		time_x;
	int		time_y;
	int		vcount;
	RTCDate	date;
	RTCTime	time;
	RTCDate	date_old;
	RTCTime	time_old;
}RTCDrawProperty;

// global variables--------------------------------------------------
extern TpWork	 tpd;						// �^�b�`�p�l���f�[�^
extern KeyWork	 pad;						// �L�[�p�b�h���̓f�[�^
extern const u8 *const g_strWeek[ 7 ];		// �j��������
extern RTCDrawProperty g_rtcDraw;

extern NNSFndAllocator         g_allocator; // �������A���P�[�^
extern NNSG2dFont              gFont;       // �t�H���g
extern NNSG2dCharCanvas        gCanvas;     // CharCanvas
extern NNSG2dTextCanvas        gTextCanvas; // TextCanvas

// function-------------------------------------------------------------
void InitBG( void );
int  GetPrintfWidth( const NNSG2dTextCanvas *pCanvas, const char *fmt, ... );
void PutStringUTF16   ( int x, int y, int color, const u16 *strUTF16 );
void PutStringUTF16Sub( int x, int y, int color, const u16 *strUTF16 );
void PrintfSJIS   ( int x, int y, int color, const char *fmt, ... );
void PrintfSJISSub( int x, int y, int color, const char *fmt, ... );
void ReadKeyPad( void );
void ReadTP( void );
void DrawMenu( u16 nowCsr, const MenuParam *pMenu );
BOOL SelectMenuByTP( u16 *nowCsr, const MenuParam *pMenu );
BOOL WithinRangeTP( int top_x, int top_y, int bottom_x, int bottom_y, TPData *tgt );
void SetBannerIconOBJ( GXOamAttr *pDstOAM, BannerFileV1 *bannerp );
BOOL GetRTCData( RTCDrawProperty *pRTCDraw, BOOL forceGetFlag );
void DrawRTCData( RTCDrawProperty *pRTCDraw );
void GetAndDrawRTCData( RTCDrawProperty *pRTCDraw, BOOL forceGetFlag );

#ifdef __cplusplus
}
#endif

#endif  // __MISC_H__
