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
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu/errorLog.h>
 
// �Ȃ����o�b�t�@�T�C�Y��256byte�𒴂���Əo�͂�������悤�ɂȂ�
// �_���v������ƁAOS_TPrintf()�Ăяo�����ɂ��o�b�t�@�ɂ͂����Ɗi�[����Ă���͗l
// �����͌��ݒ�����
#define BUFSIZE 128
 
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
		int numEntry = 0;
		int totalSize = 0, nowSize = 0;
		buf[BUFSIZE] = '\0';
		
		FS_InitFile( &file );
		FS_OpenFileEx( &file, "nand:/sys/log/sysmenu.log", FS_FILEMODE_R );
		
		while( ( nowSize = FS_ReadFile( &file, buf, BUFSIZE ) ) == BUFSIZE )
		{
			OS_TPrintf("%s",buf);
			numEntry++;
			totalSize += nowSize;
		}
		
		OS_TPrintf("%s\n",buf);
		totalSize += nowSize;
		
		OS_TPrintf("num entry : %d\n", numEntry );
		OS_TPrintf("total Size : %d\n", totalSize);
	}
		 
	OS_TPrintf( "*** End of demo\n" ); 
	OS_Terminate();
}

void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

