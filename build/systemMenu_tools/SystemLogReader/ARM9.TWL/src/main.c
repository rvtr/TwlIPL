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
#include <kami_pxi.h>
#include "keypad.h"
#include "font.h"
#include "screen.h"

#define BUFSIZE 256

#define FATAL_ERROR_MAX 51
#define SCREEN_WIDTH		32
#define RESULT_LINE_OFFSET	6

#define DST_LOGFILE_PATH	"sdmc:/sysmenu.log"
#define ERRORLOG_LOGFILE_PATH	"nand:/sys/log/sysmenu.log"

#define NAND_BLOCK_BYTE 			       0x200
#define NAND_START_OFFSET					0x200
#define NANDFIRM_FILE_START_OFFSET			0x200

// nandfirm格納先のシンボル
extern void* nandfirm_begin;
extern void* nandfirm_end;
/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static char * FatalErrorCode[FATAL_ERROR_MAX];
static BOOL resetConsoleFlag;
static s16 lineoffset;
/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static BOOL deleteLogfile( void );
static void control();
static void removeLC( char *dst, const char *src );
static BOOL copyLogToSD( void );
static void doProc();
static BOOL writebackFirm( void );
static int convertLF( char *dst, char *src );
static void drawMenu( void );
static void myInit(void);
static void printConsole(char *text, ...);
static void printConsoleErr(char *text, ...);
static u64 parseRedError(const ErrorLogEntry *entry);
static BOOL putRedError( FSFile *dst, u64 code );

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
	u8 bootmode = 0x3;
	// OS_InitのなかでCard_Initが呼ばれる前に、Boottypeを無理やりNANDにする
	//MI_CpuMove( &bootmode, (void*) 0x02fffc40, 1 );
	
	myInit();
	KamiPxiInit();
	CARD_Enable(TRUE);
	// FS_Initの後の方が良い模様
	InitAllocation();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	ERRORLOG_Init( OS_AllocFromMain, OS_FreeToMain );

	resetConsoleFlag = TRUE;
	OS_TPrintf( "boottype : %d\n", OS_GetBootType() );
	
	drawMenu();
	doProc();
	
	printConsole("process finished.");
	OS_WaitVBlankIntr();
	ERRORLOG_End();
	OS_Terminate();
}

static void control()
{
	if( kamiPadIsTrigger( PAD_BUTTON_A ) )
	{
		drawMenu();
		doProc();
	}
}

// 一連の自動化作業を実行する
static void doProc()
{
	// 空行挿入
	lineoffset++;
	
	printConsole( "Copying logfile ..." );
	
	if( copyLogToSD() )
	{
		// コピー成功時にだけログファイルの削除を行う
		deleteLogfile();
	}

	writebackFirm();
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


static BOOL deleteLogfile( void )
{
	
	if( !FS_DeleteFile( ERRORLOG_LOGFILE_PATH ) )
	{
		printConsoleErr( "Delete Failed." );
		printConsoleErr( "func: FS_DeleteFile"  );
		printConsoleErr( "errorCode : %d", FS_GetArchiveResultCode( ERRORLOG_LOGFILE_PATH ) );
		
		return FALSE;
	}
	
	printConsole( "Delete Succeeded." );
	return TRUE;
}

static BOOL copyLogToSD( void )
{
	FSFile src, dst;
	// 最悪で読み込んだサイズの倍の文字列になる可能性がある
	BOOL result = TRUE;
	int idxlog;
	int sizelog;
	char buf[BUFSIZE + 1];
	char winbuf[BUFSIZE*2 +1];
	s32 readSize;
	s32 writeSize = 0;
	
	buf[256] = '\0';
	FS_InitFile( &dst );
	
	// まずファイルを削除
	FS_DeleteFile( DST_LOGFILE_PATH );
	
	if( ! FS_CreateFile( DST_LOGFILE_PATH, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		printConsoleErr( "Copy Failed." );
		printConsoleErr( "func: FS_CreateFile"  );
		printConsoleErr( "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH )  );
		return FALSE;
	}
	
	// ファイル作成に成功
	if( !FS_OpenFileEx( &dst , DST_LOGFILE_PATH, FS_FILEMODE_RW ))
	{
		// 作成したファイルをopenできなかった場合
		printConsoleErr( "Copy Failed." );
		printConsoleErr( "func: FS_OpenFile / FS_SetFileLength"  );
		printConsoleErr( "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH )  );
		return FALSE;
	}
	
	// サイズ変更が終わったら、念のためファイルサイズ変更不可なRWLモードで開きなおしておく
	// →改行文字の置換やログの置換に伴いファイルサイズ可変長に変更
	/*
	FS_CloseFile( &dst );

	if( !FS_OpenFileEx( &dst, DST_LOGFILE_PATH, FS_FILEMODE_RW ) )
	{
		printConsoleErr( "Copy Failed." );
		printConsoleErr( "func: FS_OpenFile dst"  );
		printConsoleErr( "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH )  );
		return FALSE;
	}
	*/
	if( !FS_OpenFileEx( &src, ERRORLOG_LOGFILE_PATH, FS_FILEMODE_R ) )
	{
		printConsoleErr( "Copy Failed." );
		printConsoleErr( "func: FS_OpenFile src"  );
		printConsoleErr( "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH )  );
		result = FALSE;
		goto ERRORPATH;
	}
	
	if( !FS_SeekFileToBegin( &src ))
	{
		printConsoleErr( "Copy Failed." );
		printConsoleErr( "func: FS_SeekFileToBegin"  );
		result = FALSE;
		goto ERRORPATH;
	}
	
	sizelog = ERRORLOG_GetNum();
	idxlog = 0;
	while( ( readSize = FS_ReadFile( &src, buf, 256 )) > 0 && 
			idxlog < sizelog )
	{
		int size;
		const ErrorLogEntry *entry = ERRORLOG_Read( idxlog++ );
		u64 errorcode;
		
		// 改行文字を置換しておく
		size = convertLF( winbuf, buf );
		
		if( FS_WriteFile( &dst, winbuf, size ) < 0 )
		{
			printConsoleErr( "Copy Failed." );
			printConsoleErr( "func: FS_WriteFile"  );
			result = FALSE;
			goto ERRORPATH;
		}
		
		// 最上位ビットが立っていたらマッチングしてない
		if(!((errorcode = parseRedError(entry)) & (0x1ULL << 63)))
		{
			if(!putRedError(&dst, errorcode))
			{
				result = FALSE;
				goto ERRORPATH;
			}
		}	
		
	}

ERRORPATH:

	if( !FS_CloseFile( &src ))
	{
		printConsoleErr( "Copy Failed!" );
		printConsoleErr( "func: FS_CloseFile(src)"  );
	}

	
	if( !FS_CloseFile( &dst ))
	{
		printConsoleErr( "Copy Failed." );
		printConsoleErr( "func: FS_CloseFile(dst)"  );
	}
	printConsole( "Copy Succeeded." );

	return result;

}

// ログファイルの書き出しが終わった後に、ROMの指定された位置に格納されたnandfirmを本体に書き戻す
static BOOL writebackFirm( void )
{
	u8 *pBuf;
	u32 allocSize;
	u32 fileSize = (u32)(&nandfirm_end) - (u32)(&nandfirm_begin);
	u32 nandfirmSize = fileSize - NANDFIRM_FILE_START_OFFSET;
	u32 writeBlock;
	
	if( 800*1024 < fileSize ) 
	{
		printConsoleErr( "too large file size." );
		return FALSE;
	}
		
	// 書き込みサイズは512バイトのブロック単位なのでそれに配慮
	allocSize = MATH_ROUNDUP(fileSize, 512); 
	printConsole( "fileBegin: %08x", &nandfirm_begin );
	printConsole( "fileEnd: %08x", &nandfirm_end );
	printConsole( "fileSize: %08x", fileSize );
	printConsole( "allocsize: %08x", allocSize );
	pBuf = OS_Alloc(allocSize);
	if(pBuf == NULL)
	{
		printConsoleErr( "Alloc failed." );
		return FALSE;
	}
	
	// データの読み出し
	MI_CpuClear8(pBuf, allocSize);
	DC_FlushRange(pBuf, allocSize);
	MI_CpuCopy8( &nandfirm_begin, pBuf, fileSize);
	printConsole( "read firm succeeded." );
	//CARD_ReadRom( MI_DMA_NOT_USE, nandfirm_begin, pBuf, fileSize);
	
	// データの書き出し	
	writeBlock = nandfirmSize/NAND_BLOCK_BYTE + (nandfirmSize % NAND_BLOCK_BYTE != 0);
	printConsole( "Blocksize : %08x", writeBlock );	
	DC_FlushRange(pBuf, allocSize);
	printConsole( "nandfirm writing..." );	
	kamiNandWrite( NAND_START_OFFSET/NAND_BLOCK_BYTE, pBuf+NANDFIRM_FILE_START_OFFSET, writeBlock);
	// まだ書き込まないで出力だけする
	printConsole( "write firm succeeded." );	
	OS_Free(pBuf);

	return TRUE;
}
static void drawMenu( void )
{
	lineoffset = 0;
	ClearScreen();
/*	printConsole( "SystemLog Reader" );
	printConsole( "+-----------------------------+" );
	printConsole( "l A Button : Copy log to SD   l" );
	printConsole( "+-----------------------------+" );
	*/
	printConsole( "+--------------------+" );
	printConsole( "l System Log Reader  l" );
	printConsole( "+--------------------+" );
	lineoffset++;
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

// RedErrorの類ならエラーコードを、そうでなければ最上位ビットを立てて返す
static u64 parseRedError(const ErrorLogEntry *entry)
{
	u64 errorcode;
	
	if(entry->isLauncherError)
	{
		// REDFormatのログだったらそのまま出力すればいいので拒否
		return 0x1ULL << 63;
	}
	
	if (STD_TSScanf(entry->errorStr,"%*s [l.%*d] RED FATAL %16llx (%*16llx)", &errorcode) == 1|| 
		STD_TSScanf(entry->errorStr,"%*s [l.%*d] BOOT %16llx (%*16llx)", &errorcode) == 1 )
	{
		printConsole("matched");
		return errorcode;
	}
	
	// マッチしなかった場合
	return 0x1ULL << 63;
}


static BOOL putRedError( FSFile *dst, u64 code )
{
	char buf[BUFSIZE];
	u8 counter = 0;
	
	// codeのどこかにビットが立っている限りシフトし続ける
	while(code)
	{
		// 末尾のビットが立っていたらcounter番目のエラーが発生している
		if( code & 0x1 )
		{
			STD_TSNPrintf(buf, BUFSIZE, "err: %s\r\n", FatalErrorCode[counter]);
			
			if( FS_WriteFile( dst, buf, STD_StrLen(buf) ) < 0 )
			{
				printConsoleErr( "Copy Failed." );
				printConsoleErr( "func: FS_WriteFile"  );
				return FALSE;
			}
		}
		
		counter++;
		code >>= 1;
	}
	
	return TRUE;
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
	    //---- upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));


    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

static void printConsole(char *text, ...)
{
	va_list arglist;
	
	va_start( arglist, text);
	PrintString( 0, lineoffset++, CONSOLE_WHITE, text, arglist) ;
}

static void printConsoleErr(char *text, ...)
{
	va_list arglist;
	
	va_start( arglist, text);
	PrintString( 0, lineoffset++, CONSOLE_RED, text, arglist) ;
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

void myInit(void)
{
    //---- init
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    //---- init displaying
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- setting 2D for top screen
    GX_SetBankForBG(GX_VRAM_BG_128_A);

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));



    //---- setting 2D for bottom screen
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                      GX_BG_COLORMODE_16,
                      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);

    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));


    //---- screen
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

   //---- init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

	//---- FileSytem init
	FS_Init(FS_DMA_NOT_USE);

    //---- start displaying
    GX_DispOn();
    GXS_DispOn();
}

static char * FatalErrorCode[] = {
	"FATAL_ERROR_UNDEFINED",
	"FATAL_ERROR_NAND",
	"FATAL_ERROR_HWINFO_NORMAL",
	"FATAL_ERROR_HWINFO_SECURE",
	"FATAL_ERROR_TWLSETTINGS",
	"FATAL_ERROR_SHARED_FONT",
	"FATAL_ERROR_WLANFIRM_AUTH",
	"FATAL_ERROR_WLANFIRM_LOAD",
	"FATAL_ERROR_TITLE_LOAD_FAILED",
	"FATAL_ERROR_TITLE_POINTER_ERROR",
	"FATAL_ERROR_AUTHENTICATE_FAILED",
	"FATAL_ERROR_ENTRY_ADDRESS_ERROR",
	"FATAL_ERROR_TITLE_BOOTTYPE_ERROR",
	"FATAL_ERROR_SIGN_DECRYPTION_FAILED",
	"FATAL_ERROR_SIGN_COMPARE_FAILED",
	"FATAL_ERROR_HEADER_HASH_CALC_FAILED",
	"FATAL_ERROR_TITLEID_COMPARE_FAILED",
	"FATAL_ERROR_VALID_SIGN_FLAG_OFF",
	"FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED",
	"FATAL_ERROR_MODULE_HASH_CHECK_FAILED",
	"FATAL_ERROR_MODULE_HASH_CALC_FAILED",
	"FATAL_ERROR_MEDIA_CHECK_FAILED",
	"FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED",
	"FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED",
	"FATAL_ERROR_DL_HASH_CALC_FAILED",
	"FATAL_ERROR_DL_SIGN_COMPARE_FAILED",
	"FATAL_ERROR_WHITELIST_INITDB_FAILED",
	"FATAL_ERROR_WHITELIST_NOTFOUND",
	"FATAL_ERROR_DHT_PHASE1_FAILED",
	"FATAL_ERROR_DHT_PHASE2_FAILED",
	"FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF",
	"FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN",
	"FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN",
	"FATAL_ERROR_PLATFORM_UNKNOWN",
	"FATAL_ERROR_LOAD_UNFINISHED",
	"FATAL_ERROR_LOAD_OPENFILE_FAILED",
	"FATAL_ERROR_LOAD_MEMALLOC_FAILED",
	"FATAL_ERROR_LOAD_SEEKFILE_FAILED",
	"FATAL_ERROR_LOAD_READHEADER_FAILED",
	"FATAL_ERROR_LOAD_LOGOCRC_ERROR",
	"FATAL_ERROR_LOAD_READDLSIGN_FAILED",
	"FATAL_ERROR_LOAD_RELOCATEINFO_FAILED",
	"FATAL_ERROR_LOAD_READMODULE_FAILED",
	"FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED",
	"FATAL_ERROR_SYSMENU_VERSION",
	"FATAL_ERROR_DHT_PHASE1_CALC_FAILED",
	"FATAL_ERROR_LOAD_UNKNOWN_BOOTTYPE",
	"FATAL_ERROR_LOAD_AUTH_HEADER_FAILED",
	"FATAL_ERROR_LOAD_NEVER_STARTED",
	"FATAL_ERROR_EJECT_CARD_AFTER_LOAD_START",
	"FATAL_ERROR_TITLEID_COMPARE_FAILED_NTR",
};

