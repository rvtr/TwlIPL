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

#include <twl.h>
#include <sysmenu/WDS.h>
#include <sysmenu.h>
#include "scanWDS.h"

#define	WDS_THREAD_PRIO		10
#define	WDS_STACK_SIZE		1024
#define WDS_MESG_DEPTH		1
static OSThread s_thread;
u64 s_stack[ WDS_STACK_SIZE / sizeof(u64) ];
OSMessage s_msgBuffer[ WDS_MESG_DEPTH ];
OSMessageQueue s_msgQueue;

// WDS�X�L�����X���b�h
static void ScanWDSThread( void *arg );
// �A�N�Z�X�|�C���g���̃f�o�b�O�\���p�֐�
static void DumpWDSApInfo( WDSApInfo *apinfo );

// WDS�񓯊��֐��̃R�[���o�b�N�֐�����
// �R�[���o�b�N�֐����ł̓A�v���P�[�V�����̃X�e�[�g�ϐ�s_appstate��ύX����
void WDS_Initialize_CB(void *arg);
void WDS_StartScan_CB(void *arg);
void WDS_EndScan_CB(void *arg);
void WDS_End_CB(void *arg);

// �A�v���P�[�V�����𐧌䂷��X�e�[�g�̗񋓌^
typedef enum AppState {
	APP_STATE_WDSINIT,
	APP_STATE_WDSWAITINIT,
	APP_STATE_WDSSCAN,
	APP_STATE_WDSWAITSCAN,
	APP_STATE_WDSCOMPLETESCAN,
	APP_STATE_WDSENDSCAN,
	APP_STATE_WDSWAITENDSCAN,
	APP_STATE_WDSCOMPLETEENDSCAN,
	APP_STATE_WDSWAITEND,
	APP_STATE_WDSCOMPLETEEND
} AppState;


// �󂯎�����r�[�R�������i�[����ϐ�(���̔z��������`���[�o�R�Ńz�b�g�X�|�b�g�`�����l���ɓn��)
static WDSBriefApInfo briefapinfo[WDS_APINFO_MAX];



// WDS�X�L�����X���b�h�̋N��
void StartScanWDS( void )
{
	u8 *wdsSysBuf;
	
	// WDS���C�u�������g�p����o�b�t�@���m��(32�o�C�g�A���C�������g���Ă���K�v������)
	wdsSysBuf = SYSM_Alloc( WDS_GetWorkAreaSize() );
	if( wdsSysBuf == NULL)
	{
		OS_Panic("OS_Alloc Failed");
	}
	// WDS�X���b�h�̋N��
	OS_InitMessageQueue( &s_msgQueue, &s_msgBuffer[0], WDS_MESG_DEPTH );
    OS_CreateThread( &s_thread, ScanWDSThread, (void *)wdsSysBuf, s_stack + WDS_STACK_SIZE / sizeof(u64), WDS_STACK_SIZE, WDS_THREAD_PRIO );
    OS_WakeupThreadDirect( &s_thread );
	
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSINIT, OS_MESSAGE_BLOCK );
}


// WDS�X�L�����X���b�h
static void ScanWDSThread( void *arg )
{
#pragma unused(arg)
	OSTick wdsScanBeginTick = 0;
	u8 *wdsSysBuf = arg;
	
	// ���C�����[�v
	while( 1 ) {
		OSTick now;
		int i;
		// �A�v���P�[�V�����𐧌䂷��X�e�[�g�ϐ�
		AppState appstate;
		
        (void)OS_ReceiveMessage( &s_msgQueue, (OSMessage)&appstate, OS_MESSAGE_BLOCK );
		
		switch( appstate )
		{
		case APP_STATE_WDSINIT:
			// �C�j�V�����X�e�[�g
			
			// WDS���C�u�����̏������֐����Ăяo���A���̔񓯊������̊�����҂�
			OS_Printf("*** WDS_Initialize\n");
			if( WDS_Initialize( wdsSysBuf, WDS_Initialize_CB, 0 ) == 0 )
			{
				OS_Printf("WDS_Initialize successed\n");
			}
			else {
				OS_Panic("WDS_Initialize failed");
			}
			break;
			OS_Printf("*** WDS_Initialize waiting asyncronous process\n");
		case APP_STATE_WDSWAITINIT:
		case APP_STATE_WDSWAITSCAN:
		case APP_STATE_WDSWAITENDSCAN:
		case APP_STATE_WDSWAITEND:
			// �񓯊������̊�����҂X�e�[�g�Q
			
			// �R�[���o�b�N�֐����Ăяo����A�X�e�[�g���ύX�����̂�҂Ă΂悢
			break;
		case APP_STATE_WDSSCAN:
			// �������������������ォ�A�r�[�R���X�L�����������Ɉ��������X�L�������s���ꍇ�ɓ����Ă���X�e�[�g
			
			//OS_Printf("*** WDS_StartScan\n");
			// �r�[�R���X�L�����񓯊��������J�n����
			if( WDS_StartScan( WDS_StartScan_CB ) == 0 )
			{
				if( wdsScanBeginTick == 0 )
					wdsScanBeginTick = OS_GetTick();
			}
			else {
				OS_Panic("WDS_StartScan failed");
			}
			break;
		case APP_STATE_WDSCOMPLETESCAN:
			// �X�L����������ɓ����Ă���X�e�[�g
			
			// ���̃X�L�����ł̓r�[�R�������Ȃ����Ƃ������̂ŁA2�b�ԃr�[�R����M���J��Ԃ�
			now = OS_GetTick();
			if( OS_TicksToMilliSeconds(now - wdsScanBeginTick) < 2000 )
			{
				// �ăX�L�����̂��߂ɃX�L�����J�n�X�e�[�g�Ɉڍs
				(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSSCAN, OS_MESSAGE_NOBLOCK );
			}
			else {
				// �X�L�����I���X�e�[�g�Ɉڍs
				(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSENDSCAN, OS_MESSAGE_NOBLOCK );
			}
			break;
		case APP_STATE_WDSENDSCAN:
			// �X�L�������I��������ۂɓ����Ă���X�e�[�g
			OS_Printf("*** WDS_EndScan\n");
			
			// �X�L�������I��������񓯊��������J�n����
			if( WDS_EndScan( WDS_EndScan_CB ) == 0 )
			{
				OS_Printf("WDS_EndScan successed\n");
			}
			else {
				OS_Panic("WDS_EndScan failed");
			}
			break;
		case APP_STATE_WDSCOMPLETEENDSCAN:
			// �X�L�����I���񓯊��������I������ۂɓ����ė���X�e�[�g
			OS_Printf("*** WDS_GetApInfoAll\n");
			if( WDS_GetApInfoAll( briefapinfo ) != 0 )
			{
				OS_Panic("WDS_GetApInfoAll failed\n");
			}
			for( i = 0 ; i < WDS_APINFO_MAX ; i++ )
			{
				if( briefapinfo[i].isvalid == TRUE )
				{
					OS_TPrintf("rssi: %d\n", briefapinfo[i].rssi);
					DumpWDSApInfo( &briefapinfo[i].apinfo );
					*(u16 *)0x0500003c = 0x03ff;
				}
			}
			
			// WDS���C�u�������I�����A�����n�[�h�̓d���𗎂Ƃ��񓯊��������J�n����
			OS_Printf("*** WDS_End\n");
			if( WDS_End( WDS_End_CB ) == 0 )
			{
				OS_Printf("WDS_End successed\n");
			}
			else {
				OS_Panic("WDS_End failed");
			}
			break;
		case APP_STATE_WDSCOMPLETEEND:
			// WDS���C�u�����̉�����������������ۂɓ����ė���X�e�[�g
			SYSM_Free( wdsSysBuf );
			OS_TPrintf("WDS test successfully completed\n");
			return;
		}
	}
}

// �A�N�Z�X�|�C���g���̃f�o�b�O�\���p�֐�
static void DumpWDSApInfo( WDSApInfo *apinfo )
{
	int i;
	char buf[256];
	
	OS_TPrintf( "================================\n" );
	// SSID
	MI_CpuCopy8( apinfo->ssid, buf, WDS_SSID_BUF_SIZE) ;
	buf[WDS_SSID_BUF_SIZE] = 0x00;
	OS_TPrintf( "SSID: %s\n", buf );
	
	// APNUM
	MI_CpuCopy8( apinfo->apnum, buf, WDS_APNUM_BUF_SIZE) ;
	buf[WDS_APNUM_BUF_SIZE] = 0x00;
	OS_TPrintf( "APNUM: %s\n", buf );
	
	// CHANNEL
	OS_TPrintf( "channel: %d\n", apinfo->channel );
	
	// ENCRYPTFLAG
	OS_TPrintf( "encryptmethod: %d\n", apinfo->encryptflag);
	
	// WEPKEY
	OS_TPrintf( "WEPKEY: " );
	for( i = 0 ; i < WDS_WEPKEY_BUF_SIZE ; i++ )
	{
		OS_TPrintf( "%02x", apinfo->wepkey[i] );
	}
	OS_TPrintf( "\n" );
	OS_TPrintf( "================================\n" );
}

static void WDS_Initialize_CB(void *arg)
{
#pragma unused(arg)
	OS_TPrintf("WDS_Initialize_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSSCAN, OS_MESSAGE_NOBLOCK );
}

static void WDS_StartScan_CB(void *arg)
{
#pragma unused(arg)
//	OS_TPrintf("WDS_StartScan_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSCOMPLETESCAN, OS_MESSAGE_NOBLOCK );
}

static void WDS_EndScan_CB(void *arg)
{
#pragma unused(arg)
	OS_TPrintf("WDS_EndScan_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSCOMPLETEENDSCAN, OS_MESSAGE_NOBLOCK );
}

static void WDS_End_CB(void *arg)
{
#pragma unused(arg)
	OS_TPrintf("WDS_End_CB\n");
	(void)OS_SendMessage(&s_msgQueue, (OSMessage)APP_STATE_WDSCOMPLETEEND, OS_MESSAGE_NOBLOCK );
}
