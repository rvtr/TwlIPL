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
 
// なぜかバッファサイズが256byteを超えると出力が欠けるようになる
// ダンプを見ると、OS_TPrintf()呼び出し時にもバッファにはちゃんと格納されている模様
// 原因は現在調査中
#define BUFSIZE 128
 
void VBlankIntr(void);


void TwlMain( void )
{	
	int numEntry;
	
	OS_Init();
	RTC_Init();
	
	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	
	FS_Init( FS_DMA_NOT_USE );
	InitAllocator();
//	FS_DeleteFile("nand:/sys/log/sysmenu.log");
	
	ERRORLOG_Init( Alloc, Free );

	numEntry = ERRORLOG_GetNum();
	OS_TPrintf("API: before numEntry : %d\n", numEntry );
	
	ERRORLOG_Write( (u64)0x0200040051LL );
	ERRORLOG_Printf( "%d hyoooo nyoronyoro\naboooon dogaaaaaaaan", 2+5 );
	ERRORLOG_Printf( "this is overflow message!\nthis is overflow message!\nthis is overflow message!\n"
						"this is overflow message!\nthis is overflow message!\nthis is overflow message!\n"
						"this is overflow message!\nthis is overflow message!\nthis is overflow message!\n"
						"this is overflow message!\nthis is overflow message!\nthis is overflow message!\n"
						"this is overflow message!\nthis is overflow message!\nthis is overflow message!\n"
						"this is overflow message!\nthis is overflow message!\nthis is overflow message!\n" );

/*	
	if( numEntry > 2 )
	{
		pEntry = ERRORLOG_Read(1);
		OS_TPrintf("entry[1] : %02d-%02d-%02d %02d:%02d:%02d  errorCode: %d\n",
					pEntry->year,
					pEntry->month,
					pEntry->day,
					pEntry->hour,
					pEntry->minute,
					pEntry->second,
					pEntry->errorCode);
	}
*/
	OS_TPrintf( "*** log file data\n" );
	
	{
		// ログファイルの中身を出力
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
				// '#'が出てきた回数だけエントリ数を増やす
				numEntry++;
				p++;
			}
			
		}
		
		//OS_TPrintf("%s\n",buf);
		OS_TPrintf("\n");
		FS_CloseFile( &file );
		
		totalSize += nowSize;
		
		OS_TPrintf("count: \'#\' count : %d\n", numEntry );
		OS_TPrintf("total Size : %d\n", totalSize);
	}

	numEntry = ERRORLOG_GetNum();
	OS_TPrintf("API: end numEntry : %d\n", numEntry );

		 
	ERRORLOG_End();
	OS_TPrintf( "*** End of demo\n" ); 
	OS_Terminate();
}

void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

