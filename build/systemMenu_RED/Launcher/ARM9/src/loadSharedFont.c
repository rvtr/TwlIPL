/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     loadSharedFont.c

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
#include "launcher.h"
#include "misc.h"
#include "loadSharedFont.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define FONT_LOAD_THREAD_PRIO	13
#define THREAD_STACK_SIZE		1024

// function's prototype-------------------------------------------------------

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static u64 s_fontLoadThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread s_fontLoadThread;
static OSTick s_fontLoadStartTick;
static u8 *s_pFontBuffer[ SHARED_FONT_MAX ];	// 読み込みはテストなのでロード先はstatic変数にしている。
static BOOL s_isStarted = FALSE;

// const data------------------------------------------------------------------


// ============================================================================
// 共有フォントロード
// ============================================================================
BOOL LoadSharedFontInit( void )
{
	u8 *pBuffer;
	int size;

	s_fontLoadStartTick = OS_GetTick();
	
	// ロードスレッド生成
    OS_CreateThread(&s_fontLoadThread,
                    LoadSharedFontThread,
                    NULL,
                    s_fontLoadThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, FONT_LOAD_THREAD_PRIO);
	
	// フォントロード準備
	if( !SFONT_Init() ) {
		OS_TPrintf( "SFONT_LoadInfoTable failed.\n" );
		return FALSE;
	}
	size = SFONT_GetInfoTableSize();
	if( size < 0 ) {
		OS_TPrintf( "SFONT_GetInfoTableSize failed.\n" );
		return FALSE;
	}
	
	pBuffer = Alloc( (u32)size );
	if( pBuffer == NULL ) {
		OS_TPrintf( "malloc failed.\n" );
		return FALSE;
	}
	if( !SFONT_LoadInfoTable( pBuffer ) ) {
		OS_TPrintf( "SFONT_LoadInfoTable failed.\n" );
		return FALSE;
	}
	
	// ロードスレッド起動
    OS_WakeupThreadDirect(&s_fontLoadThread);
	s_isStarted = TRUE;
	return TRUE;
}


void LoadSharedFontThread( void *arg )
{
#pragma unused(arg)
	BOOL retval = TRUE;
	SFONT_Index i;
	
	for( i = SHARED_FONT_WW_S; i < SHARED_FONT_MAX; i++ ) {
		int size;
		
		OS_TPrintf( "%s read.\n", SFONT_GetFontName( i ) );
		
		size = SFONT_GetFontSize( i );
		if( size < 0 ) {
			OS_TPrintf( "SFONT_GetFontSize failed.\n" );
			retval = FALSE;
			break;
		}
		
		// FSのキャッシュが怪しそうなので、とりあえずアラインメントをとっておく。
		size = MATH_ROUNDUP( size, SYSM_ALIGNMENT_LOAD_MODULE );
		
		s_pFontBuffer[ i ] = Alloc( (u32)size );
		if( s_pFontBuffer[ i ] == NULL ) {
			OS_TPrintf( "malloc failed.\n" );
			retval = FALSE;
			break;
		}
		
		if( !SFONT_LoadFont( i, s_pFontBuffer[ i ] ) ) {
			OS_TPrintf( "SFONT_LoadFont failed.\n" );
			retval = FALSE;
			break;
		}
	}
	OS_TPrintf( "Shared Font load time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - s_fontLoadStartTick ) );
	
	if( retval ) {
		OS_TPrintf( "Shared Font load succeeded.\n" );
	}else {
		OS_TPrintf( "Shared Font load failed.\n" );
		SYSM_SetFatalError( TRUE );
	}
}


BOOL IsFinishedLoadSharedFont( void )
{
	if( s_isStarted ) {
		return OS_IsThreadTerminated( &s_fontLoadThread );
	}else {
		return TRUE;
	}
}

