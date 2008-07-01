/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     WDS.c

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

//**********************************************************************
/**
//	@file		WDSWrapper.c
//	@brief		WDS���C�u�����̃��b�p�[
//
//	@author		S.Nakata
//	@date		2008/06/24
//	@version	01.00
//
***********************************************************************/
#include <sysmenu/WDSWrapper.h>
#ifdef WDS_WITHDWC
#include <dwc.h>
#endif

//-----------------------------------------------------
//	Structs
//-----------------------------------------------------

/**
	@brief	WDS���b�p�[�̃��[�N�̈�
*/
typedef struct WDSWrapperWork
{
	u8							*stack;							//!< WDS���b�p�[���g�p����X�^�b�N
	OSThread					thread;							//!< WDS���b�p�[���g�p����X���b�h�\����
	OSMutex						mutex;							//!< WDS���b�p�[���g�p����mutex
	
	u8							*wdswork;						//!< WDS���g�p���郏�[�N�G���A
	
	WDSBriefApInfo				briefapinfo[WDS_APINFO_MAX];	//!< WDS���b�p�[��WDS���g�p�������ʂ��i�[����̈�
	int							briefapinfonum;					//!< WDS���b�p�[��WDS���g�p�������ʂ��i�[����̈�
	
	WDSWrapperInitializeParam	initparam;						//!< ���������p�����[�^�̃R�s�[
	WDSWrapperStateThreadState	state;							//!< WDS���b�p�[�̃X�e�[�g
	OSTick						tickstart;						//!< �e�펞�ԑ���p
	
	BOOL						terminate;						//!< ����J�n�t���O
	BOOL						idle;							//!< �Ԍ��X�L�������f�t���O
	BOOL						restart;						//!< �Ԍ��X�L�����ĊJ�t���O
	
	OSDeliverArgInfo			deliverinfo;					//!< TWL�p�A�v���Ԉ������[�N�G���A
} WDSWrapperWork;

//-----------------------------------------------------
//	Variables
//-----------------------------------------------------
static WDSWrapperWork	*g_wdswrapperwork	= NULL;

//-----------------------------------------------------
//	Internal Functions
//-----------------------------------------------------
static void WDS_WrapperInitialize_CB( void *arg );
static void WDS_WrapperStartScan_CB( void *arg );
static void WDS_WrapperEndScan_CB(void *arg);
static void WDS_WrapperEnd_CB( void *arg );

//--------------------------------------------------------------------------------
/**	�A�N�Z�X�|�C���g���̃f�o�b�O�p�\���֐�
		@param apinfo �f�o�b�O�\�����s��WDSApInfo�ւ̃|�C���^
		@return �Ȃ�
*///------------------------------------------------------------------------------
static void DumpWDSApInfo( WDSApInfo *apinfo )
{
	int i;
	char buf[256];
	
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
}

//--------------------------------------------------------------------------------
/**	�X�L�����J�n�O�E�F�C�g�X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperBeforeInitState( void )
{
	if( g_wdswrapperwork->restart == TRUE ) {
		// �҂��Ă���r�����������Ɏ�M���J�n
		g_wdswrapperwork->restart = FALSE;

		// �A���X�L�����J�n���Ԃ̋L�^
		g_wdswrapperwork->tickstart = OS_GetTick();
		
		// �X�L�����J�n�X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
		return;
	}
	else if( g_wdswrapperwork->terminate == TRUE ) {
		// ���ł�WDS���������Ă���̂Œ��ڃX�e�[�g��ω�������
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
		return;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		// ���ł�WDS���������Ă���̂Œ��ڃX�e�[�g��ω�������
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		return;
	}
	
	// �\���������ԑ҂��������m�F
	if( g_wdswrapperwork->tickstart + OS_MilliSecondsToTicks( WDSWRAPPER_WAITPERIOD ) < OS_GetTick() ) {
		// �A���X�L�����J�n���Ԃ̋L�^
		g_wdswrapperwork->tickstart = OS_GetTick();
		
		// �X�L�����J�n�X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	}
}

//--------------------------------------------------------------------------------
/**	�������J�n�X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperInitState( void )
{
	WMErrCode err;
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperInitState\n");
#endif
	
	// �����������҂��X�e�[�g�����炩���ߐݒ肵�Ă���
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITINIT;
	
	err = (WMErrCode)WDS_Initialize( g_wdswrapperwork->wdswork, WDS_WrapperInitialize_CB, 0 );
	if( err == WM_ERRCODE_SUCCESS || err == WM_ERRCODE_OPERATING ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_Initialize successed\n");
#endif
	}
	else {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_Initialize failed\n");
#endif
		// �������J�n�Ɏ��s�����̂�WDS���b�p�[����X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
}

//--------------------------------------------------------------------------------
/**	�������R�[���o�b�N�֐�
		@param	arg �񓯊������̌��ʂ��i�[����WMCallback�^�ϐ��ւ̃|�C���^
		@return	�Ȃ�
		@note
			@LI �����������͎����I�ɃX�L�����J�n�X�e�[�g��
			@LI �������s���͎����I��WDS���b�p�[����X�e�[�g��
*///------------------------------------------------------------------------------
static void WDS_WrapperInitialize_CB( void *arg )
{
	WMCallback *callback = (WMCallback *)arg;
	WDSWrapperCallbackParam param;
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_Initialize_CB\n");
#endif
	
	// �Ԃ�l�Ɋ�Â��ăR�[���o�b�N�p�����[�^��ݒ�
	param.callback	= WDSWRAPPER_CALLBACK_INITIALIZE;
	if( callback->errcode != WM_ERRCODE_SUCCESS )
		param.errcode	= WDSWRAPPER_ERRCODE_FAILURE;
	else
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
	
	// �R�[���o�b�N�֐��̌Ăяo��
	g_wdswrapperwork->initparam.callback( &param );
	
	// �Ԃ�l�Ɋ�Â��ăX�e�[�g��ύX
	if( callback->errcode != WM_ERRCODE_SUCCESS )
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	else {
		// �X�L�����J�n�X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_SCAN;
	
		// �A���X�L�����J�n���Ԃ̋L�^
		g_wdswrapperwork->tickstart = OS_GetTick();
	}
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
}

//--------------------------------------------------------------------------------
/**	�X�L�����J�n�X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperScanState( void )
{
	WMErrCode err;
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf( "*** WDS_WrapperScanState\n" );
#endif
	
	// �X�L���������҂��X�e�[�g�ւ��炩���߈ڍs���Ă���
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITSCAN;
	
	err = (WMErrCode)WDS_StartScan( WDS_WrapperStartScan_CB );
	if( err == WM_ERRCODE_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_StartScan successed\n");
#endif
		g_wdswrapperwork->briefapinfonum = 0;
	}
	else {
		// �X�L�����J�n�Ɏ��s�����̂�WDS����J�n�X�e�[�g��
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf( "WDS_StartScan failed\n" );
#endif
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
}

//--------------------------------------------------------------------------------
/**	�X�L���������҂��X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperWaitScanState( void )
{
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
}

//--------------------------------------------------------------------------------
/**	�X�L�����J�n�R�[���o�b�N�֐�
		@param	arg �񓯊������̌��ʂ��i�[����WMCallback�^�ϐ��ւ̃|�C���^
		@return �Ȃ�
		@note
			@LI �����������̓X�L�������ԂɊ�Â���IDLE���X�L�����O�E�F�C�g�X�e�[�g��
			@LI �������s���̓X�L������~��WDS���b�p�[����X�e�[�g��
*///------------------------------------------------------------------------------
static void WDS_WrapperStartScan_CB( void *arg )
{
#pragma unused( arg )
#ifdef WDSWRAPPER_DEBUGPRINT
	int i;
#endif
	WDSWrapperCallbackParam param;

	// �X�e�[�g�����Əd���������Ȃ�����
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperStartScan_CB\n");
#endif

	// �O������̒�~���̗��R�ŃR�[���o�b�N�҂��X�e�[�g�łȂ��ꍇ�ɂ͉������Ȃ�
	if( g_wdswrapperwork->state != WDSWRAPPER_STATE_WAITSCAN ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("state != WDSWRAPPER_STATE_WAITSCAN\n");
#endif
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return;
	}
	
	// �X�L�������ʂ��擾����
	if( WDS_GetApInfoAll( g_wdswrapperwork->briefapinfo ) != 0 ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf( "WDS_GetApInfoAll failed\n" );
#endif
		// �R�[���o�b�N�p�����[�^�̐ݒ�
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_FAILURE;
		
		// �R�[���o�b�N�֐��̌Ăяo��
		g_wdswrapperwork->initparam.callback( &param );
		
		// �������ɃX�L������~�X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
		
		// �ŏI�I��TERMINATE�X�e�[�g�ɓ���悤�ݒ�
		g_wdswrapperwork->terminate = TRUE;
		
		// �X�e�[�g�����Əd���������Ȃ�����
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		
		return;
	}
	else {
		// �r�[�R�������i�[
		g_wdswrapperwork->briefapinfonum = WDS_GetApInfoNum();
	}

	
	if( g_wdswrapperwork->tickstart + OS_MilliSecondsToTicks( WDSWRAPPER_SCANPERIOD ) < OS_GetTick() ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("*** WDS_WrapperCompleteScanState: Scanned more than %u [ms]\n", WDSWRAPPER_SCANPERIOD );
#endif

#ifdef WDSWRAPPER_DEBUGPRINT
		// �X�L�������ʂ��擾�����̂Ńf�o�b�O�\��
		for( i = 0 ; i < WDS_APINFO_MAX ; i++ ) {
			if( g_wdswrapperwork->briefapinfo[i].isvalid == TRUE ) {
				OS_TPrintf( "================================\n" );
				OS_TPrintf( "rssi: %d\n", g_wdswrapperwork->briefapinfo[i].rssi );
				DumpWDSApInfo( &( g_wdswrapperwork->briefapinfo[i].apinfo ) );
				OS_TPrintf( "================================\n" );
			}
		}
#endif
		// �R�[���o�b�N�p�����[�^�̐ݒ�
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN2;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
			
		// �R�[���o�b�N�֐��̌Ăяo��
		g_wdswrapperwork->initparam.callback( &param );
		
		// �\���������ԃX�L���������̂ŃX�L�������f�X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else {
		// �R�[���o�b�N�p�����[�^�̐ݒ�
		param.callback	= WDSWRAPPER_CALLBACK_STARTSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
			
		// �R�[���o�b�N�֐��̌Ăяo��
		g_wdswrapperwork->initparam.callback( &param );

		// �������ɃX�L�����J�n�X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_SCAN;
	}
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
}

//--------------------------------------------------------------------------------
/**	�X�L������~�J�n�X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperEndScanState( void )
{
	WMErrCode err;
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf( "*** WDS_WrapperEndScanState\n" );
#endif
	
	// �X�L������~�҂��X�e�[�g�ւ��炩���߈ڍs���Ă���
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITENDSCAN;
	
	err = (WMErrCode)WDS_EndScan( WDS_WrapperEndScan_CB );
	if( err == WM_ERRCODE_SUCCESS || err == WM_ERRCODE_ILLEGAL_STATE ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan successed\n");
#endif
		// WDS����X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
	else if( err == WM_ERRCODE_OPERATING ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan asynchronously successed\n");
#endif
	}
	else
	{
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan failed\n");
#endif
		// �ēx�X�L������~�����݂�
		// (����͏����I�ɖ������[�v�ɓ���\��������̂ł܂������A���������X�L�������~�܂��Ă��Ȃ���WM_End���ł��Ȃ��̂ŕs��)
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
}

//--------------------------------------------------------------------------------
/**	�X�L������~�R�[���o�b�N�֐�
		@param	arg �񓯊������̌��ʂ��i�[����WMCallback�^�ϐ��ւ̃|�C���^
		@return	�Ȃ�
		@note
			@LI �����������͎����I��WDS����X�e�[�g��
			@LI �������s���͎����I�ɍēx�X�L������~�X�e�[�g��
*///------------------------------------------------------------------------------
static void WDS_WrapperEndScan_CB(void *arg)
{
	WMCallback *callback = (WMCallback *)arg;
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_WrapperEndScan_CB\n");
#endif
	
	if( callback->errcode != WM_ERRCODE_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan success\n");
#endif
		// �ēx�X�L������~�����݂�
		// (����͏����I�ɖ������[�v�ɓ���\��������̂ł܂������A���������X�L�������~�܂��Ă��Ȃ���WM_End���ł��Ȃ��̂ŕs��)
		g_wdswrapperwork->state = WDSWRAPPER_STATE_ENDSCAN;
	}
	else {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_EndScan success\n");
#endif
		// �X�L������~���������玩���I��WDS����X�e�[�g��
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
}

//--------------------------------------------------------------------------------
/**	����J�n�X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperEndState( void )
{
	WMErrCode err;
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_Printf("*** WDS_WrapperEndState\n");
#endif
	
	
	// ����҂��X�e�[�g�ւ��炩���߈ڍs���Ă���
	g_wdswrapperwork->state = WDSWRAPPER_STATE_WAITEND;
	err = (WMErrCode)WDS_End( WDS_WrapperEnd_CB );
	
	if( err == WM_ERRCODE_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_End successed\n");
#endif
	}
	else if( err == WM_ERRCODE_OPERATING ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_End asynchronously successed\n");
#endif
	}
	else {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_Printf("WDS_End failed\n");
#endif
		// ����������������݂�
		g_wdswrapperwork->state = WDSWRAPPER_STATE_END;
	}
}

//--------------------------------------------------------------------------------
/**	����J�n�R�[���o�b�N�֐�
		@param	arg �񓯊������̌��ʂ��i�[����WMCallback�^�ϐ��ւ̃|�C���^
		@return	�Ȃ�
		@note
			@LI WDS_End�͏�ɐ������邽�߁A�t���O�Ɋ�Â���IDLE�܂��͏������O�E�F�C�g�܂��̓��C�u��������X�e�[�g��
*///------------------------------------------------------------------------------
static void WDS_WrapperEnd_CB( void *arg )
{
#pragma unused(arg)
	WDSWrapperCallbackParam	param;
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf("WDS_WrapperEnd_CB\n");
#endif
	
	// WDS���I�������^�C�~���O���L�^���Ă���
	g_wdswrapperwork->tickstart = OS_GetTick();
	
	// WDS_End��̃X�e�[�g���e��t���O�Ɋ�Â��ĕύX
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
	else if( g_wdswrapperwork->idle == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_IDLE;
		
		// �R�[���o�b�N�p�����[�^�̐ݒ�
		param.callback	= WDSWRAPPER_CALLBACK_STOPSCAN;
		param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
		
		// �R�[���o�b�N�֐��̌Ăяo��
		g_wdswrapperwork->initparam.callback( &param );
	}
	else {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_BEFOREINIT;
	}
	
	
	// �X�e�[�g�����Əd���������Ȃ�����
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
}

//--------------------------------------------------------------------------------
/**	�A�C�h���X�e�[�g�֐�
		@param	�Ȃ�
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperIdleState( void )
{
	if( g_wdswrapperwork->terminate == TRUE ) {
		g_wdswrapperwork->state = WDSWRAPPER_STATE_TERMINATE;
	}
	else if( g_wdswrapperwork->restart == TRUE )
	{
		g_wdswrapperwork->idle = FALSE;
		g_wdswrapperwork->restart = FALSE;
		g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	}
}

//--------------------------------------------------------------------------------
/**	WDS���b�p�[���C�u�����������Ŏ��s����X���b�h�֐�
		@param	arg ���NULL
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperThreadFunc( void *arg )
{
#pragma unused( arg )
	WDSWrapperCallbackParam param;
	
	// �X�e�[�g�̏�����
	g_wdswrapperwork->state = WDSWRAPPER_STATE_INIT;
	
	while( 1 ) {
		OS_Sleep(20);
		
		// �X�e�[�g�ɂ�菈���𕪊�(�X�e�[�g��������mutex�ɂ��lock���s����)
		OS_LockMutex( &g_wdswrapperwork->mutex );
		if( g_wdswrapperwork->state == WDSWRAPPER_STATE_TERMINATE )
			break;
		
		switch( g_wdswrapperwork->state ) {
		case WDSWRAPPER_STATE_BEFOREINIT:		WDS_WrapperBeforeInitState();	break;
		case WDSWRAPPER_STATE_INIT:				WDS_WrapperInitState();			break;
		case WDSWRAPPER_STATE_WAITINIT:			break;
		case WDSWRAPPER_STATE_SCAN:				WDS_WrapperScanState();			break;
		case WDSWRAPPER_STATE_WAITSCAN:			WDS_WrapperWaitScanState();		break;
		case WDSWRAPPER_STATE_ENDSCAN:			WDS_WrapperEndScanState();		break;
		case WDSWRAPPER_STATE_WAITENDSCAN:		break;
		case WDSWRAPPER_STATE_END:				WDS_WrapperEndState();			break;
		case WDSWRAPPER_STATE_WAITEND:			break;
		case WDSWRAPPER_STATE_IDLE:				WDS_WrapperIdleState();			break;
		case WDSWRAPPER_STATE_TERMINATE:		break;
		}
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
	}
	
	// �R�[���o�b�N�p�����[�^�̐ݒ�
	param.callback	= WDSWRAPPER_CALLBACK_CLEANUP;
	param.errcode	= WDSWRAPPER_ERRCODE_SUCCESS;
	
	// �R�[���o�b�N�֐��̌Ăяo��
	g_wdswrapperwork->initparam.callback( &param );
}

//--------------------------------------------------------------------------------
/**	WDS�R���g���[���X���b�h�f�X�g���N�^
		@param	arg ���NULL
		@return	�Ȃ�
*///------------------------------------------------------------------------------
static void WDS_WrapperThreadDestructor( void *arg )
{
#pragma unused( arg )
	if( g_wdswrapperwork != NULL ) {
		if( g_wdswrapperwork->stack != NULL ) {
			g_wdswrapperwork->initparam.free( g_wdswrapperwork->stack );
			g_wdswrapperwork->stack = NULL;
		}
		if( g_wdswrapperwork->wdswork != NULL ) {
			g_wdswrapperwork->initparam.free( g_wdswrapperwork->wdswork );
			g_wdswrapperwork->wdswork = NULL;
		}
		g_wdswrapperwork->initparam.free( g_wdswrapperwork );
		g_wdswrapperwork = NULL;
	}
}

//-----------------------------------------------------
//	External Functions
//-----------------------------------------------------

//--------------------------------------------------------------------------------
/**
	WDS���b�p�[���C�u�����������̔񓯊��������J�n���܂��B<BR>
	�����������������ɃR�[���o�b�N���������܂��B<BR>
	�R�[���o�b�N�֐��ŃG���[�ʒm���󂯎�����ꍇ�́A���C�u�����������I�ɉ�������̂�҂��Ă��������B
		@param	param WDS���b�p�[�������p�����[�^
		@return	WDSWRAPPER_ERRCODE_SUCCESS: �������������J�n
		@return	WDSWRAPPER_ERRCODE_INITIALIZED: WDS���b�p�[���C�u�����͏������ς�
		@return	WDSWRAPPER_ERRCODE_FAILURE: �������Ɏ��s
	@note
		�EWDS�R���g���[���X���b�h�͐������ォ��Ԍ��X�L���������s���Ă��܂�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperInitialize( WDSWrapperInitializeParam param )
{
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork != NULL )
		return WDSWRAPPER_ERRCODE_INITIALIZED;
	
	// �p�����[�^�̃T�j�e�B�`�F�b�N
	if( param.callback == NULL || param.alloc == NULL || param.free == NULL )
		return WDSWRAPPER_ERRCODE_FAILURE;
	
	// ���[�N�G���A��������
	g_wdswrapperwork = param.alloc( sizeof( WDSWrapperWork ) );
	if( g_wdswrapperwork == NULL )
		goto FAILURE;
	MI_CpuClear8( g_wdswrapperwork, sizeof( WDSWrapperWork ) );
	
	// �X�^�b�N�̈�EWDS���[�N�G���A�̏�����
	g_wdswrapperwork->stack = param.alloc( WDSWRAPPER_STACKSIZE );
	if( g_wdswrapperwork->stack == NULL )
		goto FAILURE;
	g_wdswrapperwork->wdswork = param.alloc( WDS_GetWorkAreaSize() );
	if( g_wdswrapperwork->wdswork == NULL )
		goto FAILURE;
	
	// �p�����[�^�̃R�s�[
	g_wdswrapperwork->initparam = param;
	
	// mutex�̏�����
	OS_InitMutex( &g_wdswrapperwork->mutex );
	
	// �X���b�h�̐���
	OS_CreateThread( &g_wdswrapperwork->thread,
					WDS_WrapperThreadFunc,
					NULL,
					g_wdswrapperwork->stack + WDSWRAPPER_STACKSIZE,
					WDSWRAPPER_STACKSIZE,
					g_wdswrapperwork->initparam.threadprio
				   );
	
	// ���[�N�G���A����p�̃X���b�h�f�X�g���N�^��ݒ�
	OS_SetThreadDestructor( &g_wdswrapperwork->thread, WDS_WrapperThreadDestructor );
	
	// �X���b�h�̎��s
	OS_WakeupThreadDirect( &g_wdswrapperwork->thread );
	
	// ����������
	return WDSWRAPPER_ERRCODE_SUCCESS;

FAILURE:
	if( g_wdswrapperwork != NULL ) {
		if( g_wdswrapperwork->stack != NULL ) {
			param.free( g_wdswrapperwork->stack );
			g_wdswrapperwork->stack = NULL;
		}
		if( g_wdswrapperwork->wdswork != NULL ) {
			param.free( g_wdswrapperwork->wdswork );
			g_wdswrapperwork->wdswork = NULL;
		}
		param.free( g_wdswrapperwork );
		g_wdswrapperwork = NULL;
	}
	return WDSWRAPPER_ERRCODE_FAILURE;
}

//--------------------------------------------------------------------------------
/**
	WDS�R���g���[���X���b�h���~���AWDS���b�p�[���C�u������������܂��B<BR>
	������������̒��O�ɃR�[���o�b�N���������܂��B<BR>
	�R�[���o�b�N�֐��ɃG���[���ʒm����邱�Ƃ͂���܂���B
		@param	�Ȃ�
		@return	WDSWRAPPER_ERRCODE_SUCCESS WDS���b�p�[������J�n
		@return	WDSWRAPPER_ERRCODE_OPERATING WDS���b�p�[��������s��
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED WDS���b�p�[���C�u����������������Ă��Ȃ�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCleanup( void )
{
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	// ����������s�����`�F�b�N
	if( g_wdswrapperwork->terminate == TRUE ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->terminate = TRUE;
	
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDS�R���g���[���X���b�h�̃X�e�[�g���Ԍ���M��ԂɕύX���܂��B<BR>
	�X�e�[�g�̕ύX�ɐ�������ƁA�Ȍ���̊Ԍ���M���������邽�тɃR�[���o�b�N���Ăяo����܂��B<BR>
	�R�[���o�b�N�֐��ŃG���[�ʒm���󂯎�����ꍇ�́A���C�u�����������I�ɉ�������̂�҂��Ă��������B
		@return	WDSWRAPPER_ERRCODE_SUCCESS: �X�e�[�g�ύX�ɐ���(�R�[���o�b�N������҂��Ă�������)
		@return	WDSWRAPPER_ERRCODE_FAILURE: �Ԍ���M�Ɉڍs�ł���X�e�[�g�łȂ�(���݂̃X�e�[�g���ێ�����܂�)
   		@return	WDSWRAPPER_ERRCIDE_OPERATING: ���łɃX�e�[�g���Ԍ���M���
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDS���b�p�[���C�u����������������Ă��Ȃ�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperStartScan( void )
{
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	// �X�e�[�g��IDLE��BEFOREINIT�ł��邱�Ƃ��m�F
	if( g_wdswrapperwork->state != WDSWRAPPER_STATE_IDLE && g_wdswrapperwork->state != WDSWRAPPER_STATE_BEFOREINIT ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	// �ăX�^�[�g�������s�����`�F�b�N
	if( g_wdswrapperwork->restart == TRUE ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->restart = TRUE;
	
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDS�R���g���[���X���b�h�̃X�e�[�g���Ԍ���M��~��Ԃɐݒ肵�܂��B<BR>
	�Ԍ���M�����ۂɒ�~�����ۂɃR�[���o�b�N���Ăяo����܂��B<BR>
	�R�[���o�b�N�ŃG���[���ʒm����邱�Ƃ͂���܂���B
		@return	WDSWRAPPER_ERRCODE_SUCCESS: �X�e�[�g�ύX�ɐ���
		@return	WDSWRAPPER_ERRCIDE_OPERATING: ���łɃX�e�[�g���Ԍ���M��~���
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDS���b�p�[���C�u����������������Ă��Ȃ�
	@note
		�X�e�[�g�ύX���s���Ă���Ԍ���M����~����܂łɂ�20ms���x�̎��Ԃ�������܂�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperStopScan( void )
{
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	// �Ԍ���M��~�������s�����`�F�b�N
	if( g_wdswrapperwork->idle == TRUE ) {
		OS_UnlockMutex( &g_wdswrapperwork->mutex );
		return WDSWRAPPER_ERRCODE_OPERATING;
	}
	
	g_wdswrapperwork->idle = TRUE;
	
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	���O�Ɋ��������Ԍ���M�ŗL���Ȑe�@�r�[�R�����󂯎�������m�F���܂�
		@param	�Ȃ�
		@return	WDSWRAPPER_ERRCODE_SUCCESS: ���O�Ɋ��������Ԍ���M�ŗL���Ȑe�@�r�[�R�����󂯎����
		@return	WDSWRAPPER_ERRCODE_FAILURE: ���O�Ɋ��������Ԍ���M�ŗL���Ȑe�@�r�[�R�����󂯎���Ă��Ȃ�
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDS���b�p�[���C�u����������������Ă��Ȃ�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCheckValidBeacon( void )
{
	int i;
	WDSWrapperErrCode ret = WDSWRAPPER_ERRCODE_FAILURE;
	
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	OS_LockMutex( &g_wdswrapperwork->mutex );
	
	if( g_wdswrapperwork->briefapinfonum > 0 ) {
		for( i = 0; i < g_wdswrapperwork->briefapinfonum; i++ ) {
			if( g_wdswrapperwork->briefapinfo[i].isvalid == TRUE ) {
				if( g_wdswrapperwork->briefapinfo[i].apinfo.infoflag & WDS_INFOFLAG_NOTIFY ) {
					ret = WDSWRAPPER_ERRCODE_SUCCESS;
					break;
				}
			}
		}
	}
	OS_UnlockMutex( &g_wdswrapperwork->mutex );
	
	return ret;
}

//--------------------------------------------------------------------------------
/**
	���O�Ɋ��������Ԍ���M�Ŏ󂯎�����r�[�R������Argument�̈�ɃZ�b�g���܂�
		@param	�Ȃ�
		@return	WDSWRAPPER_ERRCODE_SUCCESS: �e�@�r�[�R�����Argument�̈�ɃZ�b�g���ꂽ
		@return	WDSWRAPPER_ERRCODE_FAILURE: �e�@�r�[�R������Argument�̈�ɃZ�b�g�ł��Ȃ�����
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDS���b�p�[���C�u����������������Ă��Ȃ�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperSetArgumentParam( void )
{
	int err;
	
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;

	// TWL�p��Argument�V�X�e����������
	OS_InitDeliverArgInfo( &g_wdswrapperwork->deliverinfo, sizeof( g_wdswrapperwork->briefapinfo ) );
	
	// Argument�̈�ɏ�������
	err = OS_SetBinaryToDeliverArg( g_wdswrapperwork->briefapinfo, sizeof( g_wdswrapperwork->briefapinfo ) );
	if( err != OS_DELIVER_ARG_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_TPrintf( "WDS_WrapperSetArgumentParam: failed %d\n", err );
#endif
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	err = OS_EncodeDeliverArg();
	if( err != OS_DELIVER_ARG_SUCCESS ) {
#ifdef WDSWRAPPER_DEBUGPRINT
		OS_TPrintf( "OS_EncodeDeliverArg: failed %d\n", err );
#endif
		return WDSWRAPPER_ERRCODE_FAILURE;
	}
	
	
#ifdef WDSWRAPPER_DEBUGPRINT
	OS_TPrintf( "WDS_WrapperSetArgumentParam: success\n" );
#endif
	
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDS�R���g���[���X���b�h�����s�����m�F���܂�
		@param	�Ȃ�
		@return	WDSWRAPPER_ERRCODE_SUCCESS: WDS�R���g���[���X���b�h�����s��
		@return	WDSWRAPPER_ERRCODE_FAILURE: WDS�R���g���[���X���b�h�͒�~���Ă���
		@return	WDSWRAPPER_ERRCODE_UNINITIALIZED: WDS���b�p�[���C�u����������������Ă��Ȃ�
*///------------------------------------------------------------------------------
WDSWrapperErrCode WDS_WrapperCheckThreadRunning( void )
{
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return WDSWRAPPER_ERRCODE_UNINITIALIZED;
	
	if( OS_IsThreadTerminated( &g_wdswrapperwork->thread ) == TRUE )
		return WDSWRAPPER_ERRCODE_FAILURE;
	return WDSWRAPPER_ERRCODE_SUCCESS;
}

//--------------------------------------------------------------------------------
/**
	WDS�R���g���[���X���b�h���g�p���Ă���OSThread�ւ̃|�C���^�𓾂܂�
	@param	�Ȃ�
	@return	NULL: WDS���b�p�[���C�u����������������Ă��Ȃ�
	@return	NULL�ȊO: ���s���E���邢�͒�~����OSThread�ւ̃|�C���^
*///------------------------------------------------------------------------------
OSThread *WDS_WrapperGetOSThread( void )
{
	// �������ς݂��`�F�b�N
	if( g_wdswrapperwork == NULL )
		return NULL;
	
	return &g_wdswrapperwork->thread;
}
