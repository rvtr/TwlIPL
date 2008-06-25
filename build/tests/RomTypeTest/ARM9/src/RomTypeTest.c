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

// extern data------------------------------------------

// function's prototype declaration---------------------
static BOOL ROTestCore( char *path, char *testfile );
static BOOL RWExTestCore( char *path, char *testfile );
static BOOL SRLTest( void );
static BOOL ContentTest( void );
static void FinalizeRWTest( FSFile *file, char* filename );
static BOOL RWTestCore( char *path, char *testfile );
static BOOL RWTest( char *path );
static void TestFSPermission( void );

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------
static BOOL s_quiettest = FALSE;
static char s_testnum = 0;

// const data  -----------------------------------------
static const BOOL s_answer_data[][15] = 
{
	{ FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE }, // 0
	{ FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE }, // 1
	{ FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE }, // 2
	{  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE }, // 3
	{  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE }, // 4
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // 5
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // 6
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // 7
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // 8
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // 9
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // a
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // b
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // c
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // d
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // e
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // f
	{ FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }, // g
	{  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE }, // h
	{  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE }, // i
	{  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE }, // j
	{  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE }  // k
};

//======================================================
// テストプログラム
//======================================================

static BOOL ROTestCore( char *path, char *testfile )
{
	char filename[256];
	char testfilename[256];
	int len;
	char buf[5];
	FSFile file[1];
	
	FS_InitFile( file );
	STD_TSNPrintf( filename, 256, "%s/%s", path, testfile );
	STD_TSNPrintf( testfilename, 256, "%s/test.txt", path );

/*
	if ( FS_CreateFile(testfilename, FS_PERMIT_R | FS_PERMIT_W) )
	{
		// ReadOnlyなので、ファイル作成成功したらだめ
		if( !s_quiettest ) OS_TPrintf("%s:File Create succeed. (ReadOnly) \n",testfilename);
		FS_DeleteFile( testfilename );
		//return FALSE;
	}
*/

	// ファイルオープン
	if ( FS_OpenFileEx( file, filename, FS_FILEMODE_RWL ) )
	{
		// ReadOnlyなので、Writeのファイルオープン成功したらだめ
		if( !s_quiettest ) OS_TPrintf("%s:Write mode open succeed. (ReadOnly) \n",filename);
		FS_CloseFile( file );
		return FALSE;
	}

	// ファイルオープン
	if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_R ) )
	{
		// ファイルオープン失敗
		if( !s_quiettest ) OS_TPrintf("%s:open failed.\n",filename);
		return FALSE;
	}
	// ファイルリード
	len = FS_ReadFile( file, buf, 3 );
	if( len != 3 )
	{
		// リード失敗
		if( !s_quiettest ) OS_TPrintf("%s:read failed.\n",filename);
		FS_CloseFile( file );
		return FALSE;
	}
	// ファイルクローズ
	if( !FS_CloseFile( file ) )
	{
		// クローズ失敗
		if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
		return FALSE;
	}
	
	return TRUE;
}

static BOOL RWExTestCore( char *path, char *testfile )
{
	char filename[256];
	char testfilename[256];
	int len;
	char buf[5];
	FSFile file[1];
	
	FS_InitFile( file );
	STD_TSNPrintf( filename, 256, "%s/%s", path, testfile );
	STD_TSNPrintf( testfilename, 256, "%s/test.txt", path );

	// ファイルオープン
	if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_RWL ) )
	{
		// RWLモードファイルオープン失敗
		if( !s_quiettest ) OS_TPrintf("%s:RWL mode open failed.\n",filename);
		FS_CloseFile( file );
		return FALSE;
	}

	// ファイルリード
	len = FS_ReadFile( file, buf, 3 );
	if( len != 3 )
	{
		// リード失敗
		if( !s_quiettest ) OS_TPrintf("%s:read failed.\n",filename);
		FS_CloseFile( file );
		return FALSE;
	}
	// ファイルクローズ
	if( !FS_CloseFile( file ) )
	{
		// クローズ失敗
		if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
		return FALSE;
	}
	
	return TRUE;
}

static BOOL SRLTest( void )
{
	if( s_testnum < 17 )
	{
		return ROTestCore( "nand:", "<srl>" );
	}else if( 16 < s_testnum && s_testnum < 21)
	{
		return RWExTestCore( "nand:", "<srl>" );
	}else
	{
		return FALSE;
	}
}

static BOOL ContentTest( void )
{
	return ROTestCore( "content:", "title.tmd" );
}

static BOOL Shared1Test( void )
{
	return ROTestCore( "shared1:", "TWLCFG0.dat" );
}

static void FinalizeRWTest( FSFile *file, char* filename )
{
	FS_CloseFile( file );
	FS_DeleteFile( filename );
}

// パス名は最後にスラッシュを入れない事
static BOOL RWTestCore( char *path, char *testfile )
{
	char filename[256];
	int len;
	char buf[5];
	FSFile file[1];
	
	FS_InitFile( file );
	STD_TSNPrintf( filename, 256, "%s/%s", path, testfile );

	// ファイル残ってると嫌なので削除を先に走らせておく（テスト結果には影響せず）
	FS_DeleteFile(filename);
	
	// ファイル作成テスト
	if ( FS_CreateFile(filename, FS_PERMIT_R | FS_PERMIT_W) )
	{
		// ファイルオープン
		if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_W ) )
		{
			// ファイルオープン失敗
			if( !s_quiettest ) OS_TPrintf("%s:open failed.\n",filename);
			FS_DeleteFile( filename );
			return FALSE;
		}
		// ファイルライト
		len = FS_WriteFile( file, "test", 5);
		if( len != 5 )
		{
			// ライト失敗
			if( !s_quiettest ) OS_TPrintf("%s:write failed.\n",filename);
			FinalizeRWTest( file, filename );
			return FALSE;
		}
		// ファイルクローズ
		if( !FS_CloseFile( file ) )
		{
			// クローズ失敗
			if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
			FinalizeRWTest( file, filename );
			return FALSE;
		}
		// ファイルオープン
		if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_R ) )
		{
			// ファイルオープン失敗
			if( !s_quiettest ) OS_TPrintf("%s:open failed.\n",filename);
			FS_DeleteFile( filename );
			return FALSE;
		}
		// ファイルリード
		len = FS_ReadFile( file, buf, len );
		if( len != 5 || STD_CompareString( buf, "test" ) != 0 )
		{
			// リード失敗
			if( !s_quiettest ) OS_TPrintf("%s:read failed.\n",filename);
			FinalizeRWTest( file, filename );
			return FALSE;
		}
		// ファイルクローズ
		if( !FS_CloseFile( file ) )
		{
			// クローズ失敗
			if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
			FinalizeRWTest( file, filename );
			return FALSE;
		}
		// ファイルデリート
		if( !FS_DeleteFile( filename ))
		{
			// デリート失敗
			if( !s_quiettest ) OS_TPrintf("%s:delete failed.\n",filename);
			FinalizeRWTest( file, filename );
			return FALSE;
		}
	}else
	{
		// ファイル作成失敗
		if( !s_quiettest ) OS_TPrintf("%s:cleate failed.\n",filename);
		return FALSE;
	}
	
	return TRUE;
}

static BOOL RWTest( char *path )
{
	return RWTestCore( path, "test.txt" );
}

static BOOL TMPJumpTest( void )
{
	return RWTestCore( "nand:", "<tmpjump>" );
}

u8 tempbuf[ LCFG_TEMP_BUFFER_SIZE * 2 ];
static TWLSubBannerFile sbf;

static void TestFSPermission( void )
{
	BOOL result[15];
	int l;
	
	result[0] = RWTest( "nand:" );                // nand:
	result[1] = RWTest( "nand2:" );               // nand2:
#if (ENABLE_CONTENT_TEST == 1)
	result[2] = ContentTest();                    // content:
#endif
	result[3] = Shared1Test();                    // shared1:
	result[4] = RWTest( "shared2:" );             // shared2:
	result[5] = RWTest( "photo:" );               // photo:
	result[6] = RWTest( "dataPub:" );             // dataPub:
	result[7] = RWTest( "dataPrv:" );             // dataPrv:
	result[8] = RWTest( "sdmc:" );                // sdmc
	result[9] = RWTest( "nand:/sys" );            // nand:/sys
	result[10] = RWTest( "nand:/import" );        // nand:/import
	result[11] = RWTest( "nand:/tmp" );           // nand:/tmp
	result[12] = SRLTest();                       // nand:/<srl>
	result[13] = OS_DeleteSubBannerFile(&sbf);    // nand:/<banner>
	result[14] = TMPJumpTest();                   // nand:/<tmpjump>
	
	
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"RomTypeTest");
	PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push A To Start Test.");
	PutStringUTF16( 4 * 8, 10 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push X To Start Test Quietly.");
	
	OS_TPrintf( "Correct Answer:\n" );
	for( l=0; l<15; l++ )
	{
#if (ENABLE_CONTENT_TEST == 0)
		if(l==2) continue; // contentチェック飛ばし
#endif
		OS_TPrintf( "%s ", ( s_answer_data[s_testnum][l] ? "○" : "×" ) );
	}
	OS_TPrintf( "\n" );
	OS_TPrintf( "Result:\n" );
	for( l=0; l<15; l++ )
	{
#if (ENABLE_CONTENT_TEST == 0)
		if(l==2) continue; // contentチェック飛ばし
#endif
		OS_TPrintf( "%s ", ( result[l] ? "○" : "×" ) );
		PutStringUTF16( 3 * 8 + l*14, 14 * 8, TXT_COLOR_BLACK, (const u16 *)( s_answer_data[s_testnum][l] ? L"○" : L"×" ));
		PutStringUTF16( 3 * 8 + l*14, 16 * 8, ( result[l]==s_answer_data[s_testnum][l] ? TXT_COLOR_BLUE : TXT_COLOR_RED ),
		                (const u16 *)( result[l] ? L"○" : L"×" ));
	}
	OS_TPrintf( "\n" );
}

// テストプログラムの初期化
void RomTypeTestInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"RomTypeTest");
	PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push A To Start Test.");
	PutStringUTF16( 4 * 8, 10 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push X To Start Test Quietly.");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );

	s_testnum = (char)((ROM_Header_Short *)(HW_TWL_ROM_HEADER_BUF))->titleID_Lo[1];
	if( '0' <= s_testnum && s_testnum <= '9' )
	{
		s_testnum -= '0';
	}else if( 'a' <= s_testnum && s_testnum <= 'z' )
	{
		s_testnum = (char)( s_testnum - 'a' + 10 );
	}else
	{
		s_testnum = 0;
	}

	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// テストプログラムのメインループ
void RomTypeTestMain(void)
{
	BOOL tp_cancel = FALSE;
	
	ReadTP();													// タッチパネル入力の取得
	
	// [RETURN]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = WithinRangeTP(  RETURN_BUTTON_TOP_X * 8,    RETURN_BUTTON_TOP_Y * 8 - 4,
									RETURN_BUTTON_BOTTOM_X * 8, RETURN_BUTTON_BOTTOM_Y * 8 - 4, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) ) {
		s_quiettest = FALSE;
		TestFSPermission();
	}
	
	if( ( pad.trg & PAD_BUTTON_X ) ) {
		s_quiettest = TRUE;
		TestFSPermission();
	}
	
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
//		SYSM_RebootLauncher();
	}
	
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


