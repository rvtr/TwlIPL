/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
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

#define FATAL_ERROR_MAX		45
#define NUM_ENTRY_PER_PAGE	7
#define NUM_LINE_PER_ENTRY	3
#define SKIP_SPAN			7
#define FOOTER_Y			22

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static char *s_strError[ FATAL_ERROR_MAX ];
static int drawIndex = 0;
static int numEntry;
/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static void drawErrorLog( void );
static void control();
/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
TwlMain()
{
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

	kamiFontPrintfConsole( CONSOLE_ORANGE, "How to \n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "+-----------------------------+\n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "l Up/Down Key   : Scroll line l\n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "l Left/Right Key: Scroll page l\n");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "+-----------------------------+\n");

	numEntry = ERRORLOG_GetNum();

    while (1)
    {
		static i = 0 ;
		kamiPadRead();

		// Vブランク待ち
        OS_WaitVBlankIntr();

		// フォントスクリーンデータロード
		kamiFontLoadScreenData();

		// 操作検出と描画
		control();
		drawErrorLog();

    }
    
}

static void drawErrorLog( void )
{
	u16 i;
	
	kamiFontClear();

	for( i = 0; i < NUM_ENTRY_PER_PAGE && i+drawIndex < numEntry ; i++ )
	{
		const ErrorLogEntry *entry = ERRORLOG_Read( i + drawIndex );
		kamiFontPrintf( 0, i * NUM_LINE_PER_ENTRY , 0, "%02d: %02d/%02d/%02d %02d:%02d:%02d Err: %d" ,
						i + drawIndex , entry->year, entry->month, entry->day,
						entry->hour, entry->minute, entry->second, entry->errorCode );
						
		kamiFontPrintf( 0, ( i * NUM_LINE_PER_ENTRY ) + 1, 0, s_strError[entry->errorCode] );
		
	}
	
	kamiFontPrintf( 0, FOOTER_Y , 0, "numEntry : %d", numEntry );
}

static void control()
{
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
	
	// 操作の結果、描画インデクスがはみ出しそうだったら修正
	drawIndex = numEntry - NUM_ENTRY_PER_PAGE < drawIndex ? numEntry - NUM_ENTRY_PER_PAGE: drawIndex ;
	drawIndex = drawIndex < 0 ? 0: drawIndex;
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
	"FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39",
	"FATAL_ERROR_LOAD_READDLSIGN_FAILED",
	"FATAL_ERROR_LOAD_RELOCATEINFO_FAILED",
	"FATAL_ERROR_LOAD_READMODULE_FAILED",
    "FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED",
    "FATAL_ERROR_SYSMENU_VERSION",
};