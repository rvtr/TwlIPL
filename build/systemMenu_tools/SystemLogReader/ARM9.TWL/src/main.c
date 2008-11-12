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
	
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "process finished.");
	OS_WaitVBlankIntr();
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
	
	PrintString(0, lineoffset++, CONSOLE_WHITE, "Copying logfile ...") ;
	
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
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Delete Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_DeleteFile" );
		PrintString( 0, lineoffset++, CONSOLE_WHITE,
			"errorCode : %d", FS_GetArchiveResultCode( ERRORLOG_LOGFILE_PATH ) );
		
		return FALSE;
	}
	
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "Delete Succeeded.") ;
	return TRUE;
}

static BOOL copyLogToSD( void )
{
	FSFile src, dst;
	// 最悪で読み込んだサイズの倍の文字列になる可能性がある
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
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_CreateFile" );
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	// ファイル作成に成功
	if( !FS_OpenFileEx( &dst , DST_LOGFILE_PATH, FS_FILEMODE_RW ))
	{
		// 作成したファイルをopenできなかった場合
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_OpenFile / FS_SetFileLength" );
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	// サイズ変更が終わったら、念のためファイルサイズ変更不可なRWLモードで開きなおしておく
	// →ファイルサイズ可変長に変更
	FS_CloseFile( &dst );

	if( !FS_OpenFileEx( &dst, DST_LOGFILE_PATH, FS_FILEMODE_RW ) )
	{
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_OpenFile dst" );
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	if( !FS_OpenFileEx( &src, ERRORLOG_LOGFILE_PATH, FS_FILEMODE_R ) )
	{
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_OpenFile src" );
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
	}
	
	if( !FS_SeekFileToBegin( &src ))
	{
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_SeekFileToBegin" );
		return FALSE;
	}
	
	while( ( readSize = FS_ReadFile( &src, buf, 256 )) > 0 )
	{
		int size;
		// 改行文字を置換しておく
		size = convertLF( winbuf, buf );
		
		if( FS_WriteFile( &dst, winbuf, size ) < 0 )
		{
			PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
			PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_WriteFile" );
			return FALSE;
		}
		
	}


	if( !FS_CloseFile( &src ))
	{
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed!") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_CloseFile(src)" );
	}

	
	if( !FS_CloseFile( &dst ))
	{
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Failed.") ;
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "func: FS_CloseFile(dst)" );
	}
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "Copy Succeeded.") ;

	return TRUE;

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
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "too large file size.") ;
		return FALSE;
	}
		
	// 書き込みサイズは512バイトのブロック単位なのでそれに配慮
	allocSize = MATH_ROUNDUP(fileSize, 512); 
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "fileBegin: %08x", &nandfirm_begin) ;
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "fileEnd: %08x", &nandfirm_end) ;
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "fileSize: %08x", fileSize) ;
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "allocsize: %08x", allocSize) ;
	pBuf = OS_Alloc(allocSize);
	if(pBuf == NULL)
	{
		PrintString( 0, lineoffset++, CONSOLE_WHITE, "Alloc failed.") ;
		return FALSE;
	}
	
	// データの読み出し
	MI_CpuClear8(pBuf, allocSize);
	DC_FlushRange(pBuf, allocSize);
	MI_CpuCopy8( &nandfirm_begin, pBuf, fileSize);
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "read firm succeeded.") ;
	//CARD_ReadRom( MI_DMA_NOT_USE, nandfirm_begin, pBuf, fileSize);
	
	// データの書き出し	
	writeBlock = nandfirmSize/NAND_BLOCK_BYTE + (nandfirmSize % NAND_BLOCK_BYTE != 0);
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "Blocksize : %08x", writeBlock) ;	
	DC_FlushRange(pBuf, allocSize);
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "nandfirm writing...") ;	
	kamiNandWrite( NAND_START_OFFSET/NAND_BLOCK_BYTE, pBuf+NANDFIRM_FILE_START_OFFSET, writeBlock);
	// まだ書き込まないで出力だけする
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "write firm succeeded.") ;	
	OS_Free(pBuf);

	return TRUE;
}
static void drawMenu( void )
{
	lineoffset = 0;
	ClearScreen();
/*	PrintString( 0, lineoffset++, CONSOLE_WHITE, "SystemLog Reader");
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "+-----------------------------+");
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "l A Button : Copy log to SD   l");
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "+-----------------------------+");
	*/
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "+--------------------+");
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "l System Log Reader  l");
	PrintString( 0, lineoffset++, CONSOLE_WHITE, "+--------------------+");
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
