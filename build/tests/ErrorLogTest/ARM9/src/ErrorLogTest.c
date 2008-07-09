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
#include "misc.h"
 
// �Ȃ����o�b�t�@�T�C�Y��256byte�𒴂���Əo�͂�������悤�ɂȂ�
// �_���v������ƁAOS_TPrintf()�Ăяo�����ɂ��o�b�t�@�ɂ͂����Ɗi�[����Ă���͗l
// �����͌��ݒ�����
#define BUFSIZE 128
 
void VBlankIntr(void);


void TwlMain( void )
{	
	int numEntry;
	const ErrorLogEntry *pEntry;
	
	OS_Init();
	RTC_Init();
	
	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	
	FS_Init( FS_DMA_NOT_USE );
	InitAllocator();
	
	EL_Init( Alloc, Free );
	
	EL_WriteErrorLog( (u64)0x077777777777LL );

	numEntry = EL_getErrorLogNum();
	OS_TPrintf("numEntry : %d\n", numEntry );
	
	if( numEntry > 2 )
	{
		pEntry = EL_getErrorLog(1);
		OS_TPrintf("entry[1] : %02d-%02d-%02d %02d:%02d:%02d  errorCode: %d\n",
					pEntry->year,
					pEntry->month,
					pEntry->day,
					pEntry->hour,
					pEntry->minute,
					pEntry->second,
					pEntry->errorCode);
	}

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
			char *p = buf;
			
			OS_TPrintf("%s",buf);
			totalSize += nowSize;
			
			while( (p = STD_SearchChar(p, '#')) != NULL)
			{
				// '#'���o�Ă����񐔂����G���g�����𑝂₷
				numEntry++;
				p++;
			}
			
		}
		
		//OS_TPrintf("%s\n",buf);
		OS_TPrintf("\n");
		FS_CloseFile( &file );
		
		totalSize += nowSize;
		
		OS_TPrintf("total Size : %d\n", totalSize);
	}
		 
	EL_End();
	OS_TPrintf( "*** End of demo\n" ); 
	OS_Terminate();
}

void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

