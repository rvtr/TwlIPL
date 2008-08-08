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
static BOOL s_quiettest = FALSE;

// const data  -----------------------------------------
static AccessPermission s_answer_data[TEST_NUM];

static const u16 *s_answer_str[PERMISSION_MAX] = 
{
	L"NA",
	L"RO",
	L"RW",
	L"×",
};

static const u16 *s_test_name[TEST_NUM] = 
{
	L"nand:",
	L"nand2:",
	L"content:",
	L"shared1:",
	L"shared2:",
	L"photo:",
	L"dataPub:",
	L"dataPrv:",
	L"sdmc:",
	L"nand:/sys",
	L"nand:/import",
	L"nand:/tmp",
	L"nand:/<srl>",
	L"nand:/<banner>",
	L"nand:/<tmpjump>"
};

//======================================================
// テストプログラム
//======================================================

static AccessPermission RWExistTestCore( char *path, char *testfile )
{
	char filename[256];
	int len;
	char buf[5];
	FSFile file[1];
	AccessPermission result = PERMISSION_RW;
	
	FS_InitFile( file );
	STD_TSNPrintf( filename, 256, "%s/%s", path, testfile );

	// RWLモードファイルオープン
	if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_RWL ) )
	{
		// RWLモードファイルオープン失敗
		if( !s_quiettest ) OS_TPrintf("%s:RWL mode open failed.\n",filename);
		FS_CloseFile( file );
		// Rモードファイルオープン
		if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_R ) )
		{
			// Rモードファイルオープン失敗
			if( !s_quiettest ) OS_TPrintf("%s:R mode open failed.\n",filename);
			return PERMISSION_NA;
		}
		result = PERMISSION_RO;
	}

	// ファイルリード
	len = FS_ReadFile( file, buf, 3 );
	if( len != 3 )
	{
		// リード失敗
		if( !s_quiettest ) OS_TPrintf("%s:read failed.\n",filename);
		FS_CloseFile( file );
		return PERMISSION_ERROR;
	}

	// ファイルクローズ
	if( !FS_CloseFile( file ) )
	{
		// クローズ失敗
		if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
		return PERMISSION_ERROR;
	}

	return result;
}

static AccessPermission BannerTest( void )
{
	return RWExistTestCore( "nand:", "<banner>" );
}

static AccessPermission SRLTest( void )
{
	return RWExistTestCore( "nand:", "<srl>" );
}

static AccessPermission ContentTest( void )
{
	return RWExistTestCore( "content:", "title.tmd" );
}

static AccessPermission Shared1Test( void )
{
	return RWExistTestCore( "shared1:", "TWLCFG0.dat" );
}

static void FinalizeRWTest( FSFile *file, char* filename )
{
	FS_CloseFile( file );
	FS_DeleteFile( filename );
}

// パス名は最後にスラッシュを入れない事
static AccessPermission RWTestCore( char *path, char *testfile )
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
			return PERMISSION_ERROR;
		}
		// ファイルライト
		len = FS_WriteFile( file, "test", 5);
		if( len != 5 )
		{
			// ライト失敗
			if( !s_quiettest ) OS_TPrintf("%s:write failed.\n",filename);
			FinalizeRWTest( file, filename );
			return PERMISSION_ERROR;
		}
		// ファイルクローズ
		if( !FS_CloseFile( file ) )
		{
			// クローズ失敗
			if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
			FinalizeRWTest( file, filename );
			return PERMISSION_ERROR;
		}

		// ファイルオープン
		if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_R ) )
		{
			// ファイルオープン失敗
			if( !s_quiettest ) OS_TPrintf("%s:open failed.\n",filename);
			FS_DeleteFile( filename );
			return PERMISSION_ERROR;
		}
		// ファイルリード
		len = FS_ReadFile( file, buf, len );
		if( len != 5 || STD_CompareString( buf, "test" ) != 0 )
		{
			// リード失敗
			if( !s_quiettest ) OS_TPrintf("%s:read failed.\n",filename);
			FinalizeRWTest( file, filename );
			return PERMISSION_ERROR;
		}
		// ファイルクローズ
		if( !FS_CloseFile( file ) )
		{
			// クローズ失敗
			if( !s_quiettest ) OS_TPrintf("%s:close failed.\n",filename);
			FinalizeRWTest( file, filename );
			return PERMISSION_ERROR;
		}
		// ファイルデリート
		if( !FS_DeleteFile( filename ))
		{
			// デリート失敗
			if( !s_quiettest ) OS_TPrintf("%s:delete failed.\n",filename);
			FinalizeRWTest( file, filename );
			return PERMISSION_ERROR;
		}
	}else
	{
		// ファイル作成失敗
		FSResult fs_result = FS_GetArchiveResultCode( filename );
		if( !s_quiettest ) OS_TPrintf("%s:cleate failed.\n",filename);
		OS_TPrintf( "FS_GetArchiveResultCode(%s) : %d\n", filename, fs_result );
		if( FS_RESULT_NO_ENTRY == fs_result )
		{
			// ファイル作成にNO_ENTRY＝存在しないディレクトリの下にファイルを作ろうとしたかマウントされていない
			// どちらにせよアクセスできないし、ディレクトリの存在を確認する事もできないので、パーミッションNAとして扱う
			return PERMISSION_NA;
		}else if( FS_RESULT_PERMISSION_DENIED == fs_result )
		{
			// PERMISSION_DENIEDを貰った場合、ROでない事を確認する必要がある
			if ( !FS_OpenFileEx( file, filename, FS_FILEMODE_R ) )
			{
				if( FS_RESULT_PERMISSION_DENIED == FS_GetArchiveResultCode( filename ) )
				{
					// 架空のファイルをRモードで読み込もうとした結果もDENIEDならパーミッションNA
					return PERMISSION_NA;
				}
			}
			return PERMISSION_ERROR;
		}
		// あとはERROR扱い
		return PERMISSION_ERROR;
	}
	
	return PERMISSION_RW;
}

static AccessPermission RWTest( char *path )
{
	return RWTestCore( path, "test.txt" );
}

static AccessPermission TMPJumpTest( void )
{
	return RWTestCore( "nand:", "<tmpjump>" );
}

static void TestFSPermission( void )
{
	AccessPermission result[TEST_NUM];
	BOOL test_ok = TRUE;
	int l;
	
	result[0] = RWTest( "nand:" );                // nand:
	result[1] = RWTest( "nand2:" );               // nand2:
	result[2] = ContentTest();                    // content:
	result[3] = Shared1Test();                    // shared1:
	result[4] = RWTest( "shared2:" );             // shared2:
	result[5] = RWTest( "photo:" );               // photo:
	result[6] = RWTest( "dataPub:" );             // dataPub:
	result[7] = RWTest( "dataPrv:" );             // dataPrv:
	result[8] = RWTest( "sdmc:" );                // sdmc:
	result[9] = RWTest( "nand:/sys" );            // nand:/sys
	result[10] = RWTest( "nand:/import" );        // nand:/import
	result[11] = RWTest( "nand:/tmp" );           // nand:/tmp
	result[12] = SRLTest();                       // nand:/<srl>
	result[13] = BannerTest();                    // nand:/<banner>
	result[14] = TMPJumpTest();                   // nand:/<tmpjump>
	
	for( l=0; l<TEST_NUM; l++ )
	{
		test_ok = result[l]==s_answer_data[l] ? test_ok : FALSE;
	}
	
    NNS_G2dCharCanvasClear( &gCanvas, test_ok ? TXT_COLOR_BLUE : TXT_COLOR_RED );
    NNS_G2dCharCanvasClear( &gCanvasSub, test_ok ? TXT_COLOR_BLUE : TXT_COLOR_RED );
	PrintfSJIS( 1 * 8, 9 * 8, TXT_COLOR_WHITE, "FATFSPermissionCheck %c", (char)((ROM_Header_Short *)(HW_TWL_ROM_HEADER_BUF))->titleID_Lo[1]);
	PutStringUTF16( 1 * 8, 11 * 8, TXT_COLOR_WHITE,  test_ok ? (const u16 *)L"Test Succeeded." : (const u16 *)L"Test Failed..." );

	for( l=0; l<15; l++ )
	{
		PutStringUTF16Sub( 8*1, l * 12, TXT_COLOR_WHITE, s_test_name[l]);
		PutStringUTF16Sub( 8*18 + 8, l * 12, TXT_COLOR_WHITE, s_answer_str[ s_answer_data[l] ]);
		PutStringUTF16Sub( 8*18 + 8*7, l * 12, ( result[l]==s_answer_data[l] ? TXT_COLOR_CYAN : TXT_COLOR_YELLOW ), s_answer_str[ result[l] ]);
	}
	
}

static void makeAnswerData( void )
{
	ROM_Header_Short *rhs = (ROM_Header_Short *)(HW_TWL_ROM_HEADER_BUF);

	BOOL isNandApp = (rhs->titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ? TRUE : FALSE;
	BOOL isSecureApp = (rhs->titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK) ? TRUE : FALSE;
	BOOL isNandAccessOn = (rhs->access_control.nand_access) ? TRUE : FALSE;
	BOOL isSDCardAccessOn = (rhs->access_control.sd_card_access) ? TRUE : FALSE;
	BOOL isEnablePublicSaveData = ( rhs->public_save_data_size != 0 ) ? TRUE : FALSE;
	BOOL isEnablePrivateSaveData = ( rhs->private_save_data_size != 0 ) ? TRUE : FALSE;

	// 正しいアクセス設定情報を生成、特筆しない限りは、指定条件下でフルアクセス
	s_answer_data[0] = ( ( isNandApp || isNandAccessOn ) && isSecureApp ) ? PERMISSION_RW : PERMISSION_NA;	// nand:			（NANDアプリ or NANDアクセスON）and SecureApp
	s_answer_data[1] = ( ( isNandApp || isNandAccessOn ) && isSecureApp ) ? PERMISSION_RW : PERMISSION_NA;	// nand2:			（NANDアプリ or NANDアクセスON）and SecureApp
	s_answer_data[2] = ( isNandApp && CTRUE ) ? PERMISSION_RO : PERMISSION_NA;								// content:			現在無効。有効の場合はNANDアプリのみReadOnly
	s_answer_data[3] = ( isNandApp || isNandAccessOn ) ? PERMISSION_RO : PERMISSION_NA;						// shared1:			（NANDアプリ or NANDアクセスON）でReadOnly
	s_answer_data[4] = ( ( isNandApp || isNandAccessOn ) && STRUE ) ? PERMISSION_RW : PERMISSION_NA;		// shared2:			現在無効。有効の場合は（NANDアプリ or NANDアクセスON）
	s_answer_data[5] = ( isNandApp || isNandAccessOn ) ? PERMISSION_RW : PERMISSION_NA;						// photo:			NANDアプリ or NANDアクセスON
	s_answer_data[6] = ( isNandApp && isEnablePublicSaveData ) ? PERMISSION_RW : PERMISSION_NA;				// dataPub:			NANDアプリ and dataPub サイズ設定あり
	s_answer_data[7] = ( isNandApp && isEnablePrivateSaveData ) ? PERMISSION_RW : PERMISSION_NA;			// dataPrv:			NANDアプリ and dataPrv サイズ設定あり
	s_answer_data[8] = isSDCardAccessOn ? PERMISSION_RW : PERMISSION_NA;									// sdmc:			SDアクセス可（SDカードが刺さっていないと失敗）
	s_answer_data[9] = ( ( isNandApp || isNandAccessOn ) && isSecureApp ) ? PERMISSION_RW : PERMISSION_NA;	// nand:/sys		（NANDアプリ or NANDアクセスON）and SecureApp
	s_answer_data[10] = ( ( isNandApp || isNandAccessOn ) && isSecureApp ) ? PERMISSION_RW : PERMISSION_NA;	// nand:/import		（NANDアプリ or NANDアクセスON）and SecureApp
	s_answer_data[11] = ( ( isNandApp || isNandAccessOn ) && isSecureApp ) ? PERMISSION_RW : PERMISSION_NA;	// nand:/tmp		（NANDアプリ or NANDアクセスON）and SecureApp
	s_answer_data[12] = isNandApp ? ( isSecureApp ? PERMISSION_RW : PERMISSION_RO ) : PERMISSION_NA;		// nand:/<srl>		NANDアプリでReadOnly（Secureの場合はフルアクセス）
	s_answer_data[13] = isNandApp ? PERMISSION_RW : PERMISSION_NA;											// nand:/<banner>	NANDアプリ
	s_answer_data[14] = ( isNandApp || isNandAccessOn ) ? PERMISSION_RW : PERMISSION_NA;					// nand:/<tmpjump>	NANDアプリ or NANDアクセスON
}

// テストプログラムの初期化
void RomTypeTestInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PrintfSJIS( 1 * 8, 9 * 8, TXT_COLOR_BLACK, "FATFSPermissionCheck %c", (char)((ROM_Header_Short *)(HW_TWL_ROM_HEADER_BUF))->titleID_Lo[1]);
	PutStringUTF16( 1 * 8, 11 * 8, TXT_COLOR_BLACK, (const u16 *)L"Start." );
	//GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	s_quiettest = FALSE;
	
	makeAnswerData();
	
	TestFSPermission();

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
	}
	
	if( ( pad.trg & PAD_BUTTON_X ) ) {
	}
	
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
//		SYSM_RebootLauncher();
	}
	
	//GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


