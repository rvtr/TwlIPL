/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util_recoveryFile.h

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
 
#include <twl.h>
#include <sysmenu/util_recoveryFile.h>

static char *s_strResult[4];

void VBlankIntr(void);

void TwlMain( void )
{	
	char path[] = "nand:/tmp/a/b/c/d/hogehoge.dat";
	UTL_RecoveryStatus result;

	OS_Init();
	
	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	

	// とりあえずファイルサイズに16byte指定
	result = UTL_RecoveryFile( path, 0x0010 );
	
	OS_TPrintf("recovery result: %d\n", result);
	OS_TPrintf("%s\n", s_strResult[result] );
	OS_TPrintf( "*** End of demo\n" ); 
	OS_Terminate();
	

}

void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static char *s_strResult[] = {
	"Target file exists and file size matched.",
	"File size didn't match. Changing size succeeded.",
	"Target file didn't exist. Creating file and setting size succeeded.",
	"ERROR: File Recovery Failed."
};