/*---------------------------------------------------------------------------*
  Project:  TwlIPL - ErrorLog
  File:     errorLog.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu/errorLog.h>
 
#define BUFSIZE 512
 
void VBlankIntr(void);


void TwlMain( void )
{	
	
	OS_Init();
	RTC_Init();
	
	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	
	FS_Init( FS_DMA_NOT_USE );

	{
		int test = 0;
		
		for( test = 0; test < 10 ; test++ )
		{
			// �G���[���O�̏�������
			OS_TPrintf("test %d/10 ...\n", test+1 );
			EL_WriteErrorLog(test);
		}
	}
		
	// �C���[�K���ȃG���[�R�[�h�͓{����
	EL_WriteErrorLog(99);
	
	OS_TPrintf( "*** log file data\n" );
	
	{
		// ���O�t�@�C���̒��g���o��
		FSFile file;
		char buf[BUFSIZE+1];
		buf[BUFSIZE] = '\0';
		
		FS_InitFile( &file );
		FS_OpenFileEx( &file, "nand:/sys/log/sysmenu.log", FS_FILEMODE_R );
		
		while( FS_ReadFile( &file, buf, BUFSIZE ) == BUFSIZE )
		{
			OS_TPrintf("%s",buf);
		}
		
		OS_TPrintf("%s\n",buf);
	}
		 
	OS_TPrintf( "*** End of demo\n" ); 
	OS_Terminate();
}

void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

