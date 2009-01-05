/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_write_dummy.c

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

#include <twl.h>
#include "font.h"
#include "graphics.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "kami_global.h"
#include "kami_copy_file.h"
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
#ifdef FOR_LOTCHECK
#define DUMMY_FILE_NUM   1
#else
#define DUMMY_FILE_NUM   2
#endif
static const char* sDummyFileList[DUMMY_FILE_NUM] =
{ 
	"nand:/shared2/launcher/wrap.bin",	// �Ȃ��Ɩ{�̏������ł�����
#ifndef FOR_LOTCHECK
	"nand:/sys/dev.kp"					// �Ȃ��ƃA�v���Ǘ���ʂ��\������Ȃ�
#endif
};

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessWriteDummy

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessWriteDummy(void)
{
	BOOL ret = TRUE;
	s32 i;

	for (i=0;i<DUMMY_FILE_NUM;i++)
	{
		if (!kamiCreateDummyData(sDummyFileList[i]))
		{
			kamiFontPrintfConsole(FONT_COLOR_RED, "Write Dummy %d Fail!\n", i);
			ret = FALSE;
		}
	}
#ifdef FOR_LOTCHECK
	FS_DeleteFile("nand:/sys/dev.kp");
#endif
	
	OS_WaitVBlankIntr();
	kamiFontLoadScreenData();
	return ret;
}
