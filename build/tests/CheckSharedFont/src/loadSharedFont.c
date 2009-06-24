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
#include "loadSharedFont.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define FONT_LOAD_THREAD_PRIO	13
#define THREAD_STACK_SIZE		1024

// function's prototype-------------------------------------------------------

// global variable-------------------------------------------------------------
BOOL g_isSucceededLoad[ OS_SHARED_FONT_CN_KR_MAX + 1 ];

const char *str_fontname[ OS_SHARED_FONT_CN_KR_MAX + 1 ] = {
    "SHARE_FONT_WW_L",
    "SHARE_FONT_WW_M",
    "SHARE_FONT_WW_S",
	"SHARE_FONT_CN_L",
    "SHARE_FONT_CN_M",
    "SHARE_FONT_CN_S",
    "SHARE_FONT_KR_L",
    "SHARE_FONT_KR_M",
    "SHARE_FONT_KR_S",
    "SHARE_FONT_CN_KR_MAX",
};

// static variable-------------------------------------------------------------
static u64 s_fontLoadThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread s_fontLoadThread;
static OSTick s_fontLoadStartTick;
static u8 *s_pFontBuffer[ OS_SHARED_FONT_CN_KR_MAX + 1 ];	// 読み込みはテストなのでロード先はstatic変数にしている。
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
	if( !OS_InitSharedFont() ) {
		OS_TPrintf( "OS_InitSharedFont failed.\n" );
		return FALSE;
	}
	size = OS_GetSharedFontTableSize();
	if( size < 0 ) {
		OS_TPrintf( "OS_GetSharedTableSize failed.\n" );
		return FALSE;
	}
	
	pBuffer = OS_Alloc( (u32)size );
	if( pBuffer == NULL ) {
		OS_TPrintf( "malloc failed.\n" );
		return FALSE;
	}
	if( !OS_LoadSharedFontTable( pBuffer ) ) {
		OS_TPrintf( "OS_LoadSharedTable failed.\n" );
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
	OSSharedFontIndex i;
	
	for( i = OS_SHARED_FONT_WW_L; i < OS_SHARED_FONT_CN_KR_MAX + 1; i++ ) {
		int size;
		
		OS_TPrintf( "%s read.\n", str_fontname[ i ] );
		
		size = OS_GetSharedFontSize( i );
		if( size < 0 ) {
			OS_TPrintf( "    get font size failed.\n" );
			retval = FALSE;
		}
		
		// FSのキャッシュが怪しそうなので、とりあえずアラインメントをとっておく。
		size = MATH_ROUNDUP( size, 32 );
		
		s_pFontBuffer[ i ] = OS_Alloc( (u32)size );
		if( s_pFontBuffer[ i ] == NULL ) {
			OS_TPrintf( "    malloc failed.\n" );
			retval = FALSE;
		}
		
		if( OS_LoadSharedFont( i, s_pFontBuffer[ i ] ) ) {
			OS_TPrintf( "    load succeeded.\n" );
			g_isSucceededLoad[ i ] = TRUE;
		}else {
			OS_TPrintf( "    load failed.\n" );
			g_isSucceededLoad[ i ] = FALSE;
			retval = FALSE;
		}
	}
	OS_TPrintf( "Shared Font load time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - s_fontLoadStartTick ) );
}


BOOL IsFinishedLoadSharedFont( void )
{
	if( s_isStarted ) {
		return OS_IsThreadTerminated( &s_fontLoadThread );
	}else {
		return TRUE;
	}
}

