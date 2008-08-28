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

#define FATAL_ERROR_MAX		45
#define NUM_ENTRY_PER_PAGE	5
#define NUM_LINE_PER_ENTRY	4
#define SKIP_SPAN			5
#define FOOTER_Y			22

#define ASK_LINE_OFFSET		8
#define RESULT_LINE_OFFSET	10

#define SCREEN_WIDTH		32

#define DST_LOGFILE_PATH	"sdmc:/sysmenu.log"

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/
static char *s_strError[ FATAL_ERROR_MAX ];
static BOOL nowEntryView;
static BOOL nowAsking;
static BOOL resetConsoleFlag;
static int drawIndex = 0;
static int numEntry;
/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static void drawErrorLog( void );
static void control();
static void removeLC( char *dst, const char *src );
static void drawMessage( void );
static void kamiFontPrintfWrap( s16 x, s16 y, u8 color, char *fmt, ... );
static s16 kamiFontPrintfWrapSub( s16 x, s16 y, u8 color, char *str );
static BOOL copyLogToSD( void );
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


    // V�u�����N���荞�ݐݒ�
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

	// FS_Init�̌�̕����ǂ��͗l
	InitAllocation();

    // �\���֘A������
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
	nowAsking = FALSE;
	resetConsoleFlag = TRUE;

    while (1)
    {
		static i = 0 ;
		kamiPadRead();

		// V�u�����N�҂�
        OS_WaitVBlankIntr();

		// �t�H���g�X�N���[���f�[�^���[�h
		kamiFontLoadScreenData();
		
		if( resetConsoleFlag )
		{
			drawMenu();
			resetConsoleFlag = FALSE;
		}

		// ���쌟�o�ƕ`��
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
			
			// ���s����菜���Ă���\��
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
			
			// ���s����菜���Ă���\��
			removeLC( drawBuf, entry->errorStr );
			kamiFontPrintf( 0, (s16)( posY + 2), color, drawBuf );
			
		}
	}
	
	kamiFontPrintf( 0, FOOTER_Y , FONT_COLOR_BLACK, "numEntry : %d", numEntry );
}

static void control()
{
	if( nowAsking )
	{
		// �{���ɂ��́H���ĕ����Ă�Œ�
		if( kamiPadIsTrigger( PAD_BUTTON_A ))
		{
			BOOL result = copyLogToSD();
			nowAsking = FALSE;
		}
		
		if( kamiPadIsTrigger( PAD_BUTTON_B ))
		{
			drawMenu();
			nowAsking = FALSE;
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
		nowEntryView = !nowEntryView ;
	}
	else if( kamiPadIsTrigger( PAD_BUTTON_X ) )
	{
		drawMenu();
		kamiFontPrintfMain( 0, ASK_LINE_OFFSET, CONSOLE_ORANGE, "Copying Logfile to SD. OK ?");
		kamiFontPrintfMain( 0, ASK_LINE_OFFSET + 1, CONSOLE_ORANGE, "A: Decide  B: Cancel");

		nowAsking = TRUE;
	}	
	
	// ����̌��ʁA�`��C���f�N�X���͂ݏo��������������C��
	drawIndex = numEntry-1 < drawIndex ? numEntry-1 : drawIndex ;
	drawIndex = drawIndex < 0 ? 0: drawIndex;
}

// src������s����菜����dst�Ɉ����n��
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
    char    temp[256 + 2];
    char	*head, *tail;

    va_start(vlist, fmt);
    (void)vsnprintf(temp, 256, fmt, vlist);
    va_end(vlist);
    
    head = temp;
    
   	// �I�[�ǉ�
    temp[256] = '\0';
    
    // ���O��ctok�I�Ȏ������ĉ��s��ʁX�ɏo�͂���
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

// �\���\�������Ŏ����܂�Ԃ�
// �Ԃ�l�͕`��ɗ��p�����s��
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


static BOOL copyLogToSD( void )
{
	FSFile src, dst;
	char buf[256 + 1];
	s32 readSize;
	
	buf[256] = '\0';
	FS_InitFile( &dst );
	
	// �܂��t�@�C�����폜
	FS_DeleteFile( DST_LOGFILE_PATH );
	
	if( ! FS_CreateFile( DST_LOGFILE_PATH, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_CreateFile" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	// �t�@�C���쐬�ɐ���
	if( !( FS_OpenFileEx( &dst , DST_LOGFILE_PATH, FS_FILEMODE_RW ) &&
		 ( FS_SetFileLength( &dst, ERRORLOG_SIZE ) == FS_RESULT_SUCCESS ) ) )
	{
		// �쐬�����t�@�C����open�ł��Ȃ������ꍇ
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_OpenFile / FS_SetFileLength" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	// �T�C�Y�ύX���I�������A�O�̂��߃t�@�C���T�C�Y�ύX�s��RWL���[�h�ŊJ���Ȃ����Ă���
	FS_CloseFile( &dst );

	if( !FS_OpenFileEx( &dst, DST_LOGFILE_PATH, FS_FILEMODE_RWL ) )
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_OpenFile" );
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+2, CONSOLE_ORANGE, "errorCode : %d", FS_GetArchiveResultCode( DST_LOGFILE_PATH ) );
		return FALSE;
	}
	
	src = ERRORLOGi_getLogFilePt();
	
	if( !FS_SeekFileToBegin( &src ))
	{
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET, CONSOLE_ORANGE, "Copy Failed!") ;
		kamiFontPrintfMain( 0, RESULT_LINE_OFFSET+1, CONSOLE_ORANGE, "func: FS_SeekFileToBegin" );
		return FALSE;
	}
	
	while( ( readSize = FS_ReadFile( &src, buf, 256 )) > 0 )
	{
		if( FS_WriteFile( &dst, buf, readSize ) < 0 )
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
	kamiFontPrintfMain( 0, (s16)(line++), CONSOLE_ORANGE, "+-----------------------------+");
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank���荞�ݏ���

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

  Description:  �q�[�v�̏�����.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocation(void)
{
    void   *tmp;
    OSHeapHandle hh;

    /* �A���[�i�̏����� */
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