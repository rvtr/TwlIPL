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
 
// なぜかバッファサイズが256byteを超えると出力が欠けるようになる
// ダンプを見ると、OS_TPrintf()呼び出し時にもバッファにはちゃんと格納されている模様
// 原因は現在調査中
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
			// エラーログの書き込み
			OS_TPrintf("test %d/10 ...\n", test+1 );
			EL_WriteErrorLog(test);
		}
	}
		
	// イリーガルなエラーコードは怒られる
	EL_WriteErrorLog(99);
	
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

