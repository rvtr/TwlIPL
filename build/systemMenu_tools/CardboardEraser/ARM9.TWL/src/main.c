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
#include <twl/lcfg.h>
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>

#define BUFSIZE 256

#define SCREEN_WIDTH		32


#define NAND_BLOCK_BYTE 			       0x200
#define NAND_START_OFFSET					0x200
#define NANDFIRM_FILE_START_OFFSET			0x200

// 現時点では中国韓国は無視する
typedef enum {
    eRegion_JP = 0,
    eRegion_US,
    eRegion_EU,
    eRegion_AU,
    eRegion_Max
} Region;

char * regionStr[] = {
    "JP",
    "US",
    "EU",
    "AU"
};

static const OSTitleId cTitleIdList[] = {
    0x00030015484e4f4aULL, // JP
    0x00030015484e4f45ULL, // US
    0x00030015484e4f50ULL, // EU
    0x00030015484e4f55ULL  // AU
};

// nandfirm格納先のシンボル
extern void* nandfirm_begin;
extern void* nandfirm_end;

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static s16 lineoffset;

#define printConsole(...) \
    PrintString(0, lineoffset++, CONSOLE_WHITE, __VA_ARGS__)
#define printConsoleErr(...) \
    PrintString(0, lineoffset++, CONSOLE_RED, __VA_ARGS__)

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);

static void doProc();
static BOOL writebackFirm( void );
static void drawMenu( void );

static BOOL isExistCardboard( int region );
static BOOL checkCardboardStateNormal( int region );
static BOOL deleteDir( int region );

static void myInit(void);

void* myAlloc(u32 size);
void myFree(void* ptr);

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

    NAM_Init( myAlloc, myFree );

	drawMenu();
	doProc();
	
	printConsole("process finished.");
	
	NAM_End( NULL, NULL );
	OS_WaitVBlankIntr();
	OS_Terminate();
}

void* myAlloc(u32 size)
{
    return OS_Alloc(size);
}

void myFree(void* ptr)
{
    OS_Free(ptr);
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
    int regionIdx = 0;
    
	// 空行挿入
	lineoffset++;
	
	for(regionIdx = 0; regionIdx < eRegion_Max;  regionIdx++ ){
        // cardboardの存在チェック
        if( isExistCardboard( regionIdx ) ){
            
            if( !checkCardboardStateNormal( regionIdx ) ){
                // 削除するべき状態になっていれば削除する
                deleteDir( regionIdx );
            } 
        }
	}
	
	writebackFirm();
}

// 該当リージョンについてNAMTitleリストを取得して存在するか調べる
static BOOL isExistCardboard( int region )
{
    s32 result;
    s32 numTitle;
    BOOL found = FALSE;
    NAMTitleId* pTitleIdList;
    int idx;
    
    char buf[128];
    printConsole( "Search Cardboard %s ...", regionStr[region] );
    
    // タイトル数を取得して、タイトル情報を確保できるだけのバッファを確保
    numTitle = NAM_GetNumTitles();
    if( numTitle < 0 ){
        printConsoleErr( "NAM_GetNumTitles() failed." );
        printConsoleErr( "errorCode : %d", numTitle );
        return FALSE;
    }
    
    pTitleIdList = (NAMTitleId*) OS_Alloc( sizeof(NAMTitleId) * numTitle );
    SDK_ASSERT( pTitleIdList );
    
    result = NAM_GetTitleList( pTitleIdList, numTitle );
    if( result != NAM_OK ){
        printConsoleErr( "NAM_GetTitleList() failed." );
        printConsoleErr( "errorCode : %d", result );
        OS_Free( pTitleIdList );
        return FALSE;
    }
    
    // リストの取得に成功したら、その中にcardboardが含まれているかどうか調べる
    for(idx = 0; idx < numTitle; idx++ ){
        
        if( pTitleIdList[idx] == cTitleIdList[region] ){
            found = TRUE;
            break;
        }
    }
    
    
    if( found ){
        printConsole( "Cardboard %s is detected!", regionStr[region] );
    } else {
        printConsole( "Cardboard %s is not detected.", regionStr[region] );
    }
    
    OS_Free( pTitleIdList );
    
    return found;
}

// 
static BOOL checkCardboardStateNormal( int region )
{
    FSFile file[1];
    s32 readLen;
    BOOL bSuccess;
    ROM_Header_Short romHeader;
    char path[FS_ENTRY_LONGNAME_MAX];

    
    // cardboardのファイルパスを取得
    readLen = NAM_GetTitleBootContentPathFast( path, cTitleIdList[region] );
    if( readLen != NAM_OK ){
        printConsoleErr( " GetTitleBootContentPath failed." );
        printConsoleErr( " try to delete Cardboard." );
    
        return FALSE;
    }
    
    // ファイルを開いてみる
    bSuccess = FS_OpenFileEx( file, path, FS_FILEMODE_R );
    if( !bSuccess ){
        printConsoleErr( " OpenFileEX failed." );
        printConsoleErr( " try to delete Cardboard." );

        return FALSE;
    }
    
    // readもしてみる
    readLen = FS_ReadFile( file, &romHeader, sizeof(ROM_Header_Short));
    if( readLen != sizeof(ROM_Header_Short) ){
        printConsoleErr( " readfile failed." );
        printConsoleErr( " try to delete Cardboard." );
        FS_CloseFile(file);
    
        return FALSE;
    }
    
    // 全部正常に実行できれば削除する必要なし
    FS_CloseFile(file);
    printConsole(" %s is normally installed.", regionStr[region] );
    return TRUE;
}

static BOOL deleteDir( int region )
{
	// NAMでパスを取得できない可能性があるのでパスをtitleIDで決めうちにする
    char path[FS_ENTRY_LONGNAME_MAX];
	char* titleRootBase = "nand:/title";
	STD_TSPrintf( path, "%s/%08x/%08x", 
	    titleRootBase, (u32)(cTitleIdList[region] >> 32), (u32)(cTitleIdList[region] & 0xFFFFFFFFULL ));
   	
	if( !FS_DeleteDirectoryAuto( path ) )
	{
		printConsoleErr( "  Delete Failed." );
		printConsoleErr( "  func: FS_DeleteFile"  );
		printConsoleErr( "  path: %s", path  );
		printConsoleErr( "  errorCode : %d", FS_GetArchiveResultCode( path ) );
		
		return FALSE;
	}
	
	printConsole( "  Delete Succeeded." );
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
		printConsoleErr( "too large file size." );
		return FALSE;
	}
		
	// 書き込みサイズは512バイトのブロック単位なのでそれに配慮
	allocSize = MATH_ROUNDUP(fileSize, 512); 
//	printConsole( "fileBegin: %08x", &nandfirm_begin );
//	printConsole( "fileEnd: %08x", &nandfirm_end );
//	printConsole( "fileSize: %08x", fileSize );
//	printConsole( "allocsize: %08x", allocSize );
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
	
	// データの書き出し	
	writeBlock = nandfirmSize/NAND_BLOCK_BYTE + (nandfirmSize % NAND_BLOCK_BYTE != 0);
//	printConsole( "Blocksize : %08x", writeBlock );	
	DC_FlushRange(pBuf, allocSize);
	printConsole( "nandfirm writing..." );	
	kamiNandWrite( NAND_START_OFFSET/NAND_BLOCK_BYTE, pBuf+NANDFIRM_FILE_START_OFFSET, writeBlock);
	printConsole( "write firm succeeded." );	

	OS_Free(pBuf);

	return TRUE;
}
static void drawMenu( void )
{
	lineoffset = 0;
	ClearScreen();
	printConsole( "+--------------------+" );
	printConsole( "l Cardboard Eraser   l" );
	printConsole( "+--------------------+" );
	lineoffset++;
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

