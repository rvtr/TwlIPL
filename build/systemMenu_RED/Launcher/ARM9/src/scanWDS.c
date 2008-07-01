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
	OS_TPrintf("Callback_WDSWrapper: %s %d ", callbackstring[callback->callback], callback->errcode );
	
	switch( callback->callback ) {
	case WDSWRAPPER_CALLBACK_INITIALIZE:
		s_isStarted = TRUE;
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN2:
		// n�b�Ԋu�̃r�[�R���Ԍ��X�L������񕪂�����
		// �r�[�R�����󂯎���Ă��Ȃ��Ƃ������ʂ��o���ꍇ�A�����\���������܂�
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			OS_TPrintf( "�����\���������܂�" );
		else {
			OS_TPrintf( "�����\�������܂�" );
		}
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN:
		// n�b�Ԋu�̃r�[�R���Ԍ��X�L������񕪂�����
		// �r�[�R�����󂯎���Ă���ꍇ�̂݋����\����t���܂�
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_SUCCESS )
			OS_TPrintf( "�����\�������܂�" );
		break;
	case WDSWRAPPER_CALLBACK_CLEANUP:
		s_isClearnup = TRUE;
		break;
	}
	
	OS_TPrintf( "\n" );
}

