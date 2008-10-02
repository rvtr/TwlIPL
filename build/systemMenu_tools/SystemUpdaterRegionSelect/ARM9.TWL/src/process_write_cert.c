/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_write_cert.c

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
    型定義
 *---------------------------------------------------------------------------*/

typedef struct _CopyFileList
{
	char* srcPath;
	char* dstPath;
} CopyFileList;

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/

static const CopyFileList sCertList =
{ 
	"rom:/local/cert.sys",  "nand:/sys/cert.sys"
};

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessWriteCert

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
BOOL ProcessWriteCert(void)
{
	BOOL ret;

	ret = kamiCopyFile(sCertList.srcPath, sCertList.dstPath);

	if (ret)
	{
		kamiFontPrintfConsole(FONT_COLOR_GREEN, "Write Data2 Success.\n");
	}
	else
	{
		kamiFontPrintfConsole(FONT_COLOR_RED, "Write Data2 Failure!\n");
	}

	OS_WaitVBlankIntr();
	kamiFontLoadScreenData();
	return ret;
}
