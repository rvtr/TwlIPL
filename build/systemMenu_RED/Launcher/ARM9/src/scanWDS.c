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

#define WDS_THREAD_PRIO			15
#define WDS_DMA_NO				3

char *callbackstring[] = {
	"WDSWRAPPER_CALLBACK_INITIALIZE",
	"WDSWRAPPER_CALLBACK_CLEANUP",
	"WDSWRAPPER_CALLBACK_STARTSCAN",
	"WDSWRAPPER_CALLBACK_STARTSCAN2",
	"WDSWRAPPER_CALLBACK_STOPSCAN",
};

static BOOL s_isStarted = FALSE;
static BOOL s_isClearnup = FALSE;

// WDSWrapper�I���H
BOOL IsClearnupWDSWrapper( void )
{
	if( s_isStarted ) {
		return s_isClearnup;
	}else {
		return TRUE;
	}
}


// WDSWrapper�p�R�[���o�b�N�֐�
void Callback_WDSWrapper( void *ptr )
{
	WDSWrapperCallbackParam *callback = (WDSWrapperCallbackParam *)ptr;
	WDS_PRINTF("Callback_WDSWrapper: %s %d ", callbackstring[callback->callback], callback->errcode );
	
	switch( callback->callback ) {
	case WDSWRAPPER_CALLBACK_INITIALIZE:
		s_isStarted = TRUE;
		s_isClearnup = FALSE;
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN2:
		// n�b�Ԋu�̃r�[�R���Ԍ��X�L������񕪂�����
		// �r�[�R�����󂯎���Ă��Ȃ��Ƃ������ʂ��o���ꍇ�A�����\���������܂�
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			WDS_PRINTF( "�����\���������܂�" );
		else {
			WDS_PRINTF( "�����\�������܂�" );
		}
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN:
		// n�b�Ԋu�̃r�[�R���Ԍ��X�L������񕪂�����
		// �r�[�R�����󂯎���Ă���ꍇ�̂݋����\����t���܂�
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_SUCCESS )
			WDS_PRINTF( "�����\�������܂�" );
		break;
	case WDSWRAPPER_CALLBACK_CLEANUP:
		s_isStarted = FALSE;
		s_isClearnup = TRUE;
		break;
	}
	
	WDS_PRINTF( "\n" );
}

// Sleep�R�[���o�b�N�֐�
void Callback_WDSPreSleep( void *ptr )
{
#pragma unused( ptr )
	WDS_WrapperCleanup();
	while ( WDS_WrapperCheckThreadRunning() == WDSWRAPPER_ERRCODE_SUCCESS )
	{
		OS_Sleep(1);
	}
}

void Callback_WDSPostSleep( void *ptr )
{
#pragma unused( ptr )
	InitializeWDS();
}

// �������֐�
void InitializeWDS( void )
{
	static BOOL	isInitialized = FALSE;

	{
		WDSWrapperInitializeParam param;
		param.threadprio = WDS_THREAD_PRIO;
		param.dmano      = WDS_DMA_NO;
		param.callback   = Callback_WDSWrapper;
		param.alloc      = SYSM_Alloc;
		param.free       = SYSM_Free;
		(void)WDS_WrapperInitialize( param );		// �������Ɠ���J�n�����˂Ă���B�i���s���Ă��~�܂�͂��Ȃ��̂ŁA�C�ɂ��Ȃ��j
	}

	if ( ! isInitialized )
	{
		static PMSleepCallbackInfo preCbInfo;
		static PMSleepCallbackInfo postCbInfo;
		PM_SetSleepCallbackInfo( &preCbInfo, Callback_WDSPreSleep, NULL );
		PM_PrependPreSleepCallback( &preCbInfo );
		PM_SetSleepCallbackInfo( &postCbInfo, Callback_WDSPostSleep, NULL );
		PM_AppendPostSleepCallback( &postCbInfo );
	}
	isInitialized = TRUE;
}

