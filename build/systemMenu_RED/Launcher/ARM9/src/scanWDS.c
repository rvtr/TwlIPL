/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     scanWDS.c

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

#include "scanWDS.h"

char *callbackstring[] = {
	"WDSWRAPPER_CALLBACK_INITIALIZE",
	"WDSWRAPPER_CALLBACK_CLEANUP",
	"WDSWRAPPER_CALLBACK_STARTSCAN",
	"WDSWRAPPER_CALLBACK_STARTSCAN2",
	"WDSWRAPPER_CALLBACK_STOPSCAN",
};

static BOOL s_isStarted = FALSE;
static BOOL s_isClearnup = FALSE;

// WDSWrapper終了？
BOOL IsClearnupWDSWrapper( void )
{
	if( s_isStarted ) {
		return s_isClearnup;
	}else {
		return TRUE;
	}
}


// WDSWrapper用コールバック関数
void Callback_WDSWrapper( void *ptr )
{
	WDSWrapperCallbackParam *callback = (WDSWrapperCallbackParam *)ptr;
	OS_TPrintf("Callback_WDSWrapper: %s %d ", callbackstring[callback->callback], callback->errcode );
	
	switch( callback->callback ) {
	case WDSWRAPPER_CALLBACK_INITIALIZE:
		s_isStarted = TRUE;
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN2:
		// n秒間隔のビーコン間欠スキャン一回分が完了
		// ビーコンを受け取っていないという結果が出た場合、強調表示を消します
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			OS_TPrintf( "強調表示を消します" );
		else {
			OS_TPrintf( "強調表示をつけます" );
		}
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN:
		// n秒間隔のビーコン間欠スキャン一回分が完了
		// ビーコンを受け取っている場合のみ強調表示を付けます
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_SUCCESS )
			OS_TPrintf( "強調表示をつけます" );
		break;
	case WDSWRAPPER_CALLBACK_CLEANUP:
		s_isClearnup = TRUE;
		break;
	}
	
	OS_TPrintf( "\n" );
}

