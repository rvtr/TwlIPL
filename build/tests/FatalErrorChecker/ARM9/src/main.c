/*---------------------------------------------------------------------------*
  Project:  TwlIPL - Tests - FatalErrorChecker
  File:     main.c

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
#include <sysmenu/errorLog.h>
#include "kami_font.h"
#include "graphics.h"
#include "keypad.h"

#define BUFSIZE 256

#define FATAL_ERROR_MAX		49
#define NUM_ENTRY_PER_PAGE	5
#define NUM_LINE_PER_ENTRY	4
#define SKIP_SPAN			5
#define FOOTER_Y			22

#define ASK_LINE_OFFSET		9
#define RESULT_LINE_OFFSET	13

#define SCREEN_WIDTH		32

#define DST_LOGFILE_PATH	"sdmc:/sysmenu.log"
#define ERRORLOG_LOGFILE_PATH	"nand:/sys/log/sysmenu.log"

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static char *s_strError[ FATAL_ERROR_MAX ];
static BOOL nowEntryView;
static BOOL nowAskingCopy;
static BOOL nowAskingDelete;
static BOOL resetConsoleFlag;
static int drawIndex = 0;
static int numEntry;
/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static void drawErrorLog( void );
static BOOL deleteLogfile();
static void control();
static void removeLC( char *dst, const char *src );
static void drawMessage( void );
static void kamiFontPrintfWrap( s16 x, s16 y, u8 color, char *fmt, ... );
static s16 kamiFontPrintfWrapSub( s16 x, s16 y, u8 color, char *str );
static BOOL copyLogToSD( void );
int convertLF( char *dst, char *src );
static void drawMenu( void );
/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
TwlMain()
{
	char tst[] = "\n\n\n\n\na";
	
    OS_Init();
    OS_InitArena();
    PXI_Init();
    OS_InitLock();
    OS_InitArenaEx();
    OS_InitIrqTable();
    OS_SetIrqStackChecker();
    MI_Init();
    OSi_InitVramExclusive();
    OS_InitThread();
    OS_InitReset();
    GX_Init();


    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

	// FS_Initの後の方が良い模様
	InitAllocation();

    // 表示関連初期化
	InitGraphics();
	kamiFontInit();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	ERRORLOG_Init( OS_AllocFromMain, OS_FreeToMain );

	numEntry = ERRORLOG_GetNum();
	nowEntryView = FALSE;
	nowAskingCopy = FALSE;
	nowAskingDelete = FALSE;
	resetConsoleFlag = TRUE;

    while (1)
    {
		static i = 0 ;
		kamiPadRead();

		// Vブランク待ち
        OS_WaitVBlankIntr();

		// フォントスクリーンデータロード
		kamiFontLoadScreenData();
		
		if( resetConsoleFlag )
		{
			drawMenu();
			resetConsoleFlag = FALSE;
		}

		// 操作検出と描画
		control();

		if( nowEntryView )
		{
			drawMessage();
		}
		else
		{
			drawErrorLog();
		}

    }
    
}

static void drawMessage( void )
{
	char titlebuf[5];
	const ErrorLogEntry *entry = ERRORLOG_Read( drawIndex );
	kamiFontClear();
	
	if( entry->isBroken )
	{
		kamiFontPrintf( 0, 0 , FONT_COLOR_BLACK, "%02d: Broken Entry", drawIndex);
		
		kamiFontPrintfWrap( 0, 2, FONT_COLOR_BLACK, "%s\n" ,entry->errorStr );

	}
	else if( entry->isLauncherError )
	{
		STD_CopyLStringZeroFill( titlebuf, (char*)&entry->titleId, 5);
		
		kamiFontPrintf( 0, 0, FONT_COLOR_BLACK, "%02d: RED %02d/%02d/%02d %02d:%02d:%02d" ,
						drawIndex , entry->year, entry->month, entry->day,
						entry->hour, entry->minute, entry->second );
						
		kamiFontPrintf( 0, 1, FONT_COLOR_BLACK, "titleID: %s errorCode: %d",
						 titlebuf, entry->errorCode );
		
		kamiFontPrintfWrap( 0, 3, FONT_COLOR_BLACK, s_strError[entry->errorCode] );
		
	}
	else{
		STD_CopyLStringZeroFill( titlebuf, (char*)&entry->titleId, 5);
		
		kamiFontPrintf( 0, 0 , FONT_COLOR_BLACK, "%02d: FFT %02d/%02d/%02d %02d:%02d:%02d" ,
						drawIndex , entry->year, entry->month, entry->day,
						entry->hour, entry->minute, entry->second );
		
		kamiFontPrintf( 0, 1, FONT_COLOR_BLACK, "titleID: %s", titlebuf );
		
		kamiFontPrintfWrap( 0, 3, FONT_COLOR_BLACK, entry->errorStr );
		
	}
	
}

static void drawErrorLog( void )
{
	char titlebuf[5];
	s16 i;
	
	kamiFontClear();

	for( i = 0; i < NUM_ENTRY_PER_PAGE && i+drawIndex < numEntry ; i++ )
	{
		u8 color = (i == 0) ? (u8)FONT_COLOR_BLUE : (u8)FONT_COLOR_BLACK;
		s16 posY = (s16)(i * NUM_LINE_PER_ENTRY);
		const ErrorLogEntry *entry = ERRORLOG_Read( i + drawIndex );
		char drawBuf[256 + 1];
		
		if( entry->isBroken )
		{
//			kamiFontPrintf( 0, posY , color, "%02d: Broken Entry", i + drawIndex);
			kamiFontPrintf( 0, posY , color, "%02d: Broken Entry", i + drawIndex);
			
			// 改行を取り除いてから表示
			removeLC( drawBuf, entry->errorStr );
			kamiFontPrintf( 0, (s16)( posY + 1), color, drawBuf );

		}
		else if( entry->isLauncherError )
		{
			STD_CopyLStringZeroFill( titlebuf, (char*)&entry->titleId, 5);
			
			kamiFontPrintf( 0, posY , color, "%02d: RED %02d/%02d/%02d %02d:%02d:%02d" ,
							i + drawIndex , entry->year, entry->month, entry->day,
							entry->hour, entry->minute, entry->second );
							
			kamiFontPrintf( 0, (s16)( posY + 1), color, "titleID: %s errorCode: %d",
							 titlebuf, entry->errorCode );
			
			kamiFontPrintf( 0, (s16)( posY + 2), color, s_strError[entry->errorCode] );
		}
		else{
			STD_CopyLStringZeroFill( titlebuf, (char*)&entry->titleId, 5);
			
			kamiFontPrintf( 0, posY , color, "%02d: FFT %02d/%02d/%02d %02d:%02d:%02d" ,
							i + drawIndex , entry->year, entry->month, entry->day,
							entry->hour, entry->minute, entry->second );

			kamiFontPrintf( 0, (s16)( posY + 1), color, "titleID: %s", titlebuf );
			
			// 改行を取り除いてから表示
			removeLC( drawBuf, entry->errorStr );
			kamiFontPrintf( 0, (s16)( posY + 2), color, drawBuf );
			
		}
	}
	
	kamiFontPrintf( 0, FOOTER_Y , FONT_COLOR_BLACK, "numEntry : %d", numEntry );
}

static void control()
{
	if( nowAskingCopy )
	{
		// 本当にやるの？って聞いてる最中
		if( kamiPadIsTrigger( PAD_BUTTON_A ))
		{
			BOOL result;
			kamiFontPrintfMain( 0, ASK_LINE_OFFSET+2, CONSOLE_ORANGE, "now copying...");
			result = copyLogToSD();
			nowAskingCopy = FALSE;
		}
		
		if( kamiPadIsTrigger( PAD_BUTTON_B ))
		{
			drawMenu();
			nowAskingCopy = FALSE;
		}
		
		return;
	}
	
	if( nowAskingDelete )
	{
		// 本当にやるの？って聞いてる最中
		if( kamiPadIsTrigger( PAD_BUTTON_A ))
		{
			BOOL result;
			kamiFontPrintfMain( 0, ASK_LINE_OFFSET+2, CONSOLE_ORANGE, "now deleting...");
			result = deleteLogfile();
			nowAskingDelete = FALSE;
		}
		
		if( kamiPadIsTrigger( PAD_BUTTON_B ))
		{
			drawMenu();
			nowAskingDelete = FALSE;
		}
		
		return;
	}
	
	if( kamiPadIsTrigger( PAD_KEY_UP ) )
	{
		drawIndex--;
	}
	else if( kamiPadIsTrigger( PAD_KEY_DOWN ) )
	{
		drawIndex++;
	}
	
	if( kamiPadIsTrigger( PAD_KEY_LEFT ) )
	{
		drawIndex -= SKIP_SPAN;
	}
	else if( kamiPadIsTrigger( PAD_KEY_RIGHT ) )
	{
		drawIndex += SKIP_SPAN;
	}
	
	if( kamiPadIsTrigger( PAD_BUTTON_A ) )
	{
		if( 0 < numEntry )
		{
			nowEntryView = !nowEntryView ;
		}
	}
	else if( kamiPadIsTrigger( PAD_BUTTON_X ) )
	{
		drawMenu();
		kamiFontPrintfMain( 0, ASK_LINE_OFFSET, CONSOLE_ORANGE, "Copy Logfile to SD. OK ?");
		kamiFontPrintfMain( 0, ASK_LINE_OFFSET + 1, CONSOLE_ORANGE, "A: Decide  B: Cancel");

		nowAskingCopy = TRUE;
	}
	else if ( kamiPadIsTrigger( PAD_BUTTON_Y ) )
	{
		drawMenu();
		kamiFontPrintfMain( 0, ASK_LINE_OFFSET, CONSOLE_ORANGE, "Delete Logfile From NAND. OK ?");
		kamiFontPrintfMain( 0, ASK_LINE_OFFSET + 1, CONSOLE_ORANGE, "A: Decide  B: Cancel");

		nowAskingDelete = TRUE;
	}
	
	// 操作の結果、描画インデクスがはみ出しそうだったら修正
	drawIndex = numEntry-1 < drawIndex ? numEntry-1 : drawIndex ;
	drawIndex = drawIndex < 0 ? 0: drawIndex;
}

// srcから改行を取り除いてdstに引き渡す
static void removeLC( char *dst, const char *src )
{
	char *idx;
	STD_StrCpy( dst, src );
	
	while( ( idx = STD_StrChr( dst, '\n' )) != NULL )
	{
		*idx = ' ';
	}
}


static void kamiFontPrintfWrap( s16 x, s16 y, u8 color, char *fmt, ... )
{
    va_list vlist;
    char    temp[BUFSIZE + 1];
    char	*head, *tail;

    va_start(vlist, fmt);
    (void)vsnprintf(temp, BUFSIZE+1, fmt, vlist);
    va_end(vlist);
    
    head = temp;
    
   	// 終端追加
    temp[BUFSIZE] = '\0';
    
    // 自前でctok的な事をして改行を別々に出力する
    tail = STD_StrChr( temp, '\n' );
    while( tail != NULL )
    {
		*tail = '\0';
		y += kamiFontPrintfWrapSub( x, y, color, head );
		head = tail+1;
		tail = STD_StrChr( head, '\n' );
	}
    
    kamiFontPrintfWrapSub( x, y, color, head );
}

// 表示可能文字数で自動折り返し
// 返り値は描画に利用した行数
static s16 kamiFontPrintfWrapSub( s16 x, s16 y, u8 color, char *str )
{
	char *head = str;
	char buf[SCREEN_WIDTH+1];
	s16 linecount = 0;
	int length;
	
	while( SCREEN_WIDTH <= ( length = STD_StrLen( head )) )
	{
		STD_StrLCpy( buf, head, SCREEN_WIDTH+1 );
		kamiFontPrintf( x, (s16)(y + linecount++), color, buf );
		head += SCREEN_WIDTH;
	}
	
	kamiFontPrintf( x, (s16)(y + linecount++), color, head );
	
	return linecount;
	
}


static BOOL deleteLogfile( void )
{
	
	if( !FS_DeleteFile( ERRORLOG_LOGFILE_PATH ) )
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Delete Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_DeleteFile" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE,
			"errorCode : %d", FS_GetArchiveResultCode( ERRORLOG_LOGFILE_PATH ) );
		
		return FALSE;
	}
	
	kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Delete Succeeded!") ;
	return TRUE;
}

static BOOL copyLogToSD( void )
{
	FSFile src, dst;
	// 最悪で読み込んだサイズの倍の文字列になる可能性がある
	char buf[BUFSIZE + 1];
	char winbuf[BUFSIZE*2 +1];
	s32 readSize;
	
	buf[256] = '\0';
	FS_InitFile( &dst );
	
	// まずファイルを削除
	FS_DeleteFile( DST_LOGFILE_PATH );
	
	if( ! FS_CreateFile( DST_LOGFILE_PATH, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_CreateFile" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	// ファイル作成に成功
	if( !( FS_OpenFileEx( &dst , DST_LOGFILE_PATH, FS_FILEMODE_RW ) &&
		 ( FS_SetFileLength( &dst, ERRORLOG_SIZE ) == FS_RESULT_SUCCESS ) ) )
	{
		// 作成したファイルをopenできなかった場合
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_OpenFile / FS_SetFileLength" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	// サイズ変更が終わったら、念のためファイルサイズ変更不可なRWLモードで開きなおしておく
	// →ファイルサイズ可変長に変更
	FS_CloseFile( &dst );

	if( !FS_OpenFileEx( &dst, DST_LOGFILE_PATH, FS_FILEMODE_RW ) )
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_OpenFile dst" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	if( !FS_OpenFileEx( &src, ERRORLOG_LOGFILE_PATH, FS_FILEMODE_R ) )
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_OpenFile src" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
	}
	
	if( !FS_SeekFileToBegin( &src ))
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_SeekFileToBegin" );
		return FALSE;
	}
	
	while( ( readSize = FS_ReadFile( &src, buf, 256 )) > 0 )
	{
		int size;
		size = convertLF( winbuf, buf );
		
		if( FS_WriteFile( &dst, winbuf, size ) < 0 )
		{
			kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
			kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_WriteFile" );
			return FALSE;
		}
	}

	if( !FS_CloseFile( &dst ))
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_CloseFile" );
	}
	kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Succeeded!") ;

	return TRUE;

}

static void drawMenu( void )
{
	int line = 0;
	
	kamiFontClearMain();
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "How to");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "+-----------------------------+");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "l U/D Key  : Scroll Line      l");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "l L/R Key  : Scroll Page      l");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "l A Button : Switch View Mode l");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "l X Button : Copy to SDCard   l");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "l Y Button : Delete Logfile   l");
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "+-----------------------------+");
}

// LFをLFCRに置換して書き戻す
// dstは最悪時でsrcの二倍の領域が必要
int convertLF( char *dst, char *src )
{
	char *head = src, *tail;
	int writesize = 0;
	
    tail = STD_StrChr( src, '\n' );
    
    while( tail != NULL )
    {
		*tail = '\0';
		writesize += STD_StrLCpy( &dst[writesize], head,BUFSIZE );
		dst[writesize] = 0x0d;
		dst[writesize+1] = 0x0a;
		writesize += 2;
		
		head = tail + 1;
	    tail = STD_StrChr( head, '\n' );
	}
	
	writesize += STD_StrLCpy( &dst[writesize], head, BUFSIZE );
	
	return writesize;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank割り込み処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocation

  Description:  ヒープの初期化.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocation(void)
{
    void   *tmp;
    OSHeapHandle hh;

    /* アリーナの初期化 */
    tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tmp);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
        OS_Panic("ARM9: Fail to create heap...\n");
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}



static char *s_strError[ FATAL_ERROR_MAX ] = {
	"UNDEFINED",
	"NAND",
	"HWINFO_NORMAL",
	"HWINFO_SECURE",
	"TWLSETTINGS",
	"SHARED_FONT",
	"WLANFIRM_AUTH",
	"WLANFIRM_LOAD",
	"TITLE_LOAD_FAILED",
	"TITLE_POINTER_ERROR",
	"AUTHENTICATE_FAILED",
	"ENTRY_ADDRESS_ERROR",
	"TITLE_BOOTTYPE_ERROR",
	"SIGN_DECRYPTION_FAILED",
	"SIGN_COMPARE_FAILED",
	"HEADER_HASH_CALC_FAILED",
	"TITLEID_COMPARE_FAILED",
	"VALID_SIGN_FLAG_OFF",
	"CHECK_TITLE_LAUNCH_RIGHTS_FAILED",
	"MODULE_HASH_CHECK_FAILED",
	"MODULE_HASH_CALC_FAILED",
	"MEDIA_CHECK_FAILED",
	"DL_MAGICCODE_CHECK_FAILED",
	"DL_SIGN_DECRYPTION_FAILED",
	"DL_HASH_CALC_FAILED",
	"DL_SIGN_COMPARE_FAILED",
	"WHITELIST_INITDB_FAILED",
	"WHITELIST_NOTFOUND",
	"DHT_PHASE1_FAILED",
	"DHT_PHASE2_FAILED",
	"LANDING_TMP_JUMP_FLAG_OFF",
	"TWL_BOOTTYPE_UNKNOWN",
	"NTR_BOOTTYPE_UNKNOWN",
	"PLATFORM_UNKNOWN",
	"LOAD_UNFINISHED",
	"LOAD_OPENFILE_FAILED",
	"LOAD_MEMALLOC_FAILED",
	"LOAD_SEEKFILE_FAILED",
	"LOAD_READHEADER_FAILED",
	"LOAD_LOGOCRC_ERROR = 39",
	"LOAD_READDLSIGN_FAILED",
	"LOAD_RELOCATEINFO_FAILED",
	"LOAD_READMODULE_FAILED",
    "NINTENDO_LOGO_CHECK_FAILED",
    "SYSMENU_VERSION",
    "DHT_PHASE1_CALC_FAILED",
    "LOAD_UNKNOWN_BOOTTYPE",
    "LOAD_AUTH_HEADER_FAILED",
    "LOAD_NEVER_STARTED"
};
