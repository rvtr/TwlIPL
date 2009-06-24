/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Chat.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include <sysmenu.h>
#include "misc.h"
#include "RomTypeTest.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define ENABLE_CONTENT 0
#define ENABLE_SHARED2 0

#if (ENABLE_CONTENT == 1)
#define CTRUE TRUE
#else
#define CTRUE FALSE
#endif

#if (ENABLE_SHARED2 == 1)
#define STRUE TRUE
#else
#define STRUE FALSE
#endif

#define TEST_NUM 15

typedef enum AccessPermission {
	PERMISSION_NA = 0,
    PERMISSION_RO,
    PERMISSION_RW,
    PERMISSION_ERROR,
    PERMISSION_MAX
}
AccessPermission;

// extern data------------------------------------------

// function's prototype declaration---------------------
static AccessPermission RWExTestCore( char *path, char *testfile );
static AccessPermission SRLTest( void );
static AccessPermission ContentTest( void );
static void FinalizeRWTest( FSFile *file, char* filename );
static AccessPermission RWTestCore( char *path, char *testfile );
static AccessPermission RWTest( char *path );
static void TestFSPermission( void );

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------

// const data  -----------------------------------------

static BOOL CreateFile( void )
{
	int len;
	FSFile file[1];
	
	FS_InitFile( file );
	
	// ファイル作成テスト
	if ( FS_CreateFile("nand:/sys/dev.kp", FS_PERMIT_R | FS_PERMIT_W) )
	{
		// ファイルオープン
		if ( !FS_OpenFileEx( file, "nand:/sys/dev.kp", FS_FILEMODE_W ) )
		{
			// ファイルオープン失敗
			OS_TPrintf("%s:open failed.\n","nand:/sys/dev.kp");
			return FALSE;
		}
		// ファイルライト
		len = FS_WriteFile( file, "test", 5);
		if( len != 5 )
		{
			// ライト失敗
			OS_TPrintf("%s:write failed.\n","nand:/sys/dev.kp");
			return FALSE;
		}
		// ファイルクローズ
		if( !FS_CloseFile( file ) )
		{
			// クローズ失敗
			OS_TPrintf("%s:close failed.\n","nand:/sys/dev.kp");
			return FALSE;
		}
	}else
	{
		OS_TPrintf("%s:create failed.\n","nand:/sys/dev.kp");
		return FALSE;
	}
	
	return TRUE;
}

// テストプログラムの初期化
void RomTypeTestInit( void )
{
	BOOL res;
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PrintfSJIS( 1 * 8, 9 * 8, TXT_COLOR_BLACK, "CreateFile");
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	res = CreateFile();
	
	if(res)
	{
		PutStringUTF16( 1 * 8, 11 * 8, TXT_COLOR_BLACK, (const u16 *)L"Succeed." );
	}else
	{
		PutStringUTF16( 1 * 8, 11 * 8, TXT_COLOR_BLACK, (const u16 *)L"NG." );
	}

}


