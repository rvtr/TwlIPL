/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_format.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <stdlib.h>	// atoi
#include <twl.h>
#include <twl/nam.h>
#include "font.h"
#include "graphics.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "kami_global.h"
#include "kami_pxi.h"
#include "kami_font.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �O���[�o���ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/
#define CORRECT_NAND_ARCHIVE_SIZE  0xcde0000

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/
static vu8 sIsFormatFinish;
static u8 sFormatResult;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void FormatCallback(KAMIResult result, void* arg);

/*---------------------------------------------------------------------------*
  Name:         ProcessFormat

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessFormat(void)
{
	FSArchiveResource resource;

	// L & R & Start & Select �{�^�������ŋ����t�H�[�}�b�g
	if (!gIsForceFormatNand)
	{	
		// ���ɍŐV�t�H�[�}�b�g�ł���΃t�H�[�}�b�g���Ȃ�	
		if (FS_GetArchiveResource("nand:/", &resource))
		{
			if (resource.totalSize == CORRECT_NAND_ARCHIVE_SIZE)
			{
				return TRUE;
			}
		}
	}

	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();

	NNS_G2dTextCanvasDrawText(&gTextCanvas, 74, 72,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
		L"Now Format Nand.."
	);

	// �t�H�[�}�b�g���s
	sIsFormatFinish = FALSE;
    ExeFormatAsync(FORMAT_MODE_QUICK, FormatCallback);

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	    OS_WaitVBlankIntr();
		FadeInTick();

		if (sIsFormatFinish) break;
	}

	if (sFormatResult)
	{
		kamiFontPrintfConsole(FONT_COLOR_GREEN, "NAND Format Success.\n");
	}
	else
	{
		kamiFontPrintfConsole(FONT_COLOR_RED, "NAND Format Failure!\n");		
	}

	// �t�H�[�}�b�g���ES�ɕK�v�ȃt�@�C�����Ȃ��Ȃ��Ă��邽��
	// ES_InitLib���Ăяo�����Ƃō쐬���Ă���
	NAM_End( NULL, NULL );
	NAM_Init( OS_AllocFromMain, OS_FreeToMain );

	while (!FadeOutTick())
	{
		OS_WaitVBlankIntr();
	}

	return sFormatResult;
}


/*---------------------------------------------------------------------------*
  Name:         FormatCallback

  Description:  �t�H�[�}�b�g�R�[���o�b�N

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FormatCallback(KAMIResult result, void* /*arg*/)
{
	if ( result == KAMI_RESULT_SUCCESS_TRUE )
	{
		sFormatResult = TRUE;
	}
	else
	{
		sFormatResult = FALSE;
	}

	sIsFormatFinish = TRUE;
}
