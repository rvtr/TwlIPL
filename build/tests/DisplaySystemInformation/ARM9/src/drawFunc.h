/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     drawFunc.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	__DRAW_FUNC__
#define	__DRAW_FUNC__

#include <twl.h>
#include "strResource.h"

#ifdef __cplusplus
extern "C" {
#endif




// �e���j���[�T�C�Y

#define DISP_NUM_LINES 11		// ��y�[�W������̍��ڐ�

/* global variables ----------------- */

#define MAXITEM 50
#define MAXPAGE 5

// �e���ڂ�\������Ƃ��̍s�I�t�Z�b�g�\
extern int gMenuKindOffset[ROOTMENU_SIZE][MAXITEM];

/* function prototypes ----------------- */

void drawHeader( int menu, int line );
void drawMenu( int menu, int line, int changeLine, u8 mode );
void printResult( BOOL res );
void switchViewMode( void );

#ifdef __cplusplus
}
#endif

#endif
