/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - WDSWrapperTest
  File:     main.c

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
#include <sysmenu/WDSWrapper.h>

// V�u�����N�֐�
static void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

// �q�[�v�������֐�
static void InitializeAllocateSystem(void)
{
    void *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

// WDSWrapper�p�A���P�[�^
static void *Alloc_WDSWrapper( u32 size )
{
	return OS_Alloc( size );
}

static void Free_WDSWrapper( void *ptr )
{
	OS_Free( ptr );
}

char *callbackstring[] = {
	"WDSWRAPPER_CALLBACK_INITIALIZE",
	"WDSWRAPPER_CALLBACK_CLEANUP",
	"WDSWRAPPER_CALLBACK_STARTSCAN",
	"WDSWRAPPER_CALLBACK_STARTSCAN2",
	"WDSWRAPPER_CALLBACK_STOPSCAN",
};

// WDSWrapper�p�R�[���o�b�N�֐�
static void Callback_WDSWrapper( void *ptr )
{
	WDSWrapperCallbackParam *callback = (WDSWrapperCallbackParam *)ptr;
	OS_TPrintf("Callback_WDSWrapper: %s %d ", callbackstring[callback->callback], callback->errcode );
	
	switch( callback->callback ) {
	case WDSWRAPPER_CALLBACK_INITIALIZE:
		OS_TPrintf( "����������" );
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN2:
		// n�b�Ԋu�̃r�[�R���Ԍ��X�L������񕪂�����
		// �r�[�R�����󂯎���Ă��Ȃ��Ƃ������ʂ��o���ꍇ�A�����\���������܂�
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			OS_TPrintf( "�����\���������܂�" );
		else {
			OS_TPrintf( "�����\�������܂�" );
		}
		// ��M�����r�[�R�����f�[�^��Argument�̈�ɏ�������
#ifdef SDK_TWL
		OS_TPrintf( "\n" );
		WDS_WrapperSetArgumentParam();
#endif
		break;
	case WDSWRAPPER_CALLBACK_STARTSCAN:
		// n�b�Ԋu�̃r�[�R���Ԍ��X�L������񕪂�����
		// �r�[�R�����󂯎���Ă���ꍇ�̂݋����\����t���܂�
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_SUCCESS )
			OS_TPrintf( "�����\�������܂�" );
		break;
	case WDSWRAPPER_CALLBACK_STOPSCAN:
		if( WDS_WrapperCheckValidBeacon() == WDSWRAPPER_ERRCODE_FAILURE )
			OS_TPrintf( "�����\���������܂�" );
		else {
			OS_TPrintf( "�����\�������܂�" );
		}
		break;
	case WDSWRAPPER_CALLBACK_CLEANUP:
		OS_TPrintf( "�������" );
		break;
	}
	OS_TPrintf( "\n" );
}

// �X���[�v���[�h�ɓ���O�ɌĂяo�����R�[���o�b�N�֐�
static void Callback_WDSPreSleep( void *ptr )
{
#pragma unused( ptr )
	WDS_WrapperCleanup();
	while( WDS_WrapperCheckThreadRunning() == WDSWRAPPER_ERRCODE_SUCCESS )
		OS_Sleep( 100 );
}

// �X���[�v���[�h���畜�A����ۂɌĂяo�����R�[���o�b�N�֐�
static void Callback_WDSPostSleep( void *ptr )
{
#pragma unused( ptr )
	WDSWrapperInitializeParam param;
	
	// WDSWrapper�������Ɠ���J�n
	param.threadprio = 20;
	param.dmano = 1;
	
	param.callback = Callback_WDSWrapper;
	param.alloc = Alloc_WDSWrapper;
	param.free = Free_WDSWrapper;
	WDS_WrapperInitialize( param );
}

// ���C���֐�
void NitroMain(void)
{
	WDSWrapperInitializeParam param;
	u16 lastpad = 0x0000;
	u16 nowpad = 0x0000;
	PMSleepCallbackInfo presleepcallbackinfo, postsleepcallbackinfo;
	
	// �e�평��������
	OS_Init();
	OS_InitTick();
	OS_InitAlarm();

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	
	//---- power manager callback
	PM_SetSleepCallbackInfo( &presleepcallbackinfo, Callback_WDSPreSleep, NULL );
	PM_AppendPreSleepCallback( &presleepcallbackinfo );
	PM_SetSleepCallbackInfo( &postsleepcallbackinfo, Callback_WDSPostSleep, NULL );
	PM_AppendPostSleepCallback( &postsleepcallbackinfo );
	
	InitializeAllocateSystem();
	
	OS_TPrintf( "WDSWrapper Sample\n\n" );
	OS_TPrintf( "X�{�^��: WDS���b�p�[��WDS�����������A�Ԍ��X�L�������J�n\n" );
	OS_TPrintf( "B�{�^��: �X�L�����𒆒f\n" );
	OS_TPrintf( "A�{�^��: �X�L�������ĊJ\n" );
	OS_TPrintf( "Y�{�^��: �X�L�����𒆒f���AWDS���b�p�[��WDS�����\n" );
	OS_TPrintf( "�X�^�[�g�{�^��: �q�[�v���_���v�\��\n" );
	OS_TPrintf( "�Z���N�g�{�^��: �X���[�v���[�hin/out\n" );

	// �L�[���͂Œ��f�E�ĊJ�E��������
	while( 1 ) {
		lastpad = nowpad;
		nowpad = PAD_Read();
		
		// A�{�^��: �X�L�������ĊJ
		// B�{�^��: �X�L�����𒆒f
		// X�{�^��: ���b�p�[��������
		// Y�{�^��: ���b�p�[�����
		// �X�^�[�g�{�^��: �q�[�v���_���v�\��
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_A && !( nowpad & PAD_BUTTON_A ) )
			WDS_WrapperStartScan();
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_B && !( nowpad & PAD_BUTTON_B ) )
			WDS_WrapperStopScan();
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_X && !( nowpad & PAD_BUTTON_X ) ) {
			// WDSWrapper�������Ɠ���J�n
			param.threadprio = 20;
			param.dmano = 1;
	
			param.callback = Callback_WDSWrapper;
			param.alloc = Alloc_WDSWrapper;
			param.free = Free_WDSWrapper;
			WDS_WrapperInitialize( param );
		}
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_Y && !( nowpad & PAD_BUTTON_Y ) )
			WDS_WrapperCleanup();
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_START && !( nowpad & PAD_BUTTON_START ) )
			OS_DumpHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE );
		if( ( lastpad ^ nowpad ) & PAD_BUTTON_SELECT && !( nowpad & PAD_BUTTON_SELECT ) ) {
			PM_GoSleepMode( PM_TRIGGER_KEY, PM_PAD_LOGIC_OR, PAD_BUTTON_SELECT );
		}
		OS_Sleep( 100 );
	}
}
