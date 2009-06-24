/*---------------------------------------------------------------------------*
  Project:  TwlIPL
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

#include <twl.h>
#include <twl/sea.h>
#include <twl/lcfg.h>
#include <twl/na.h>
#include <nitro/fs/sysarea.h>
#include <sysmenu/namut.h>
#include <sysmenu/util.h>
#include "misc.h"
#include "MachineSetting.h"
#include "getSysMenuVersion.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------
int (*g_pNowProcess)( void );
BOOL g_isValidTSD;
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------


// ============================================================================
// function's description
// ============================================================================
void TwlMain(void)
{
	
	// ������----------------------------------
    OS_Init();
	OS_InitTick();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
    SEA_Init();
	
    GX_Init();
	GX_SetPower(GX_POWER_ALL);										// �e���W�b�N �p���[ON
	FS_Init( FS_DMA_NOT_USE );
	
	SND_Init();
	SNDEX_Init();

	// ���荞�݋���----------------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// �f�o�C�X������-------------------------------
	TP_Init();
	(void)RTC_Init();
	
	// �V�X�e���̏�����------------------
	InitAllocator();
	
	// NAM���C�u����������
	NAM_Init( Alloc, Free );        // NAMUT���C�u������NAM���C�u�������g�p���Ă���
	NAMUT_Init( Alloc, Free );
	
	// ���{���Ȃ烉���`���[����̃p�����[�^�`�F�b�N���s���A
	//   ����N���V�[�P���X�ɓ���p�X������
	
	{
		OS_TPrintf( "LCFGTWLOwnerInfo       : 0x%04x\n", sizeof(LCFGTWLOwnerInfo) );
		OS_TPrintf( "LCFGTWLParentalControl : 0x%04x\n", sizeof(LCFGTWLParentalControl) );
		OS_TPrintf( "LCFGTWLSettingsData    : 0x%04x\n", sizeof(LCFGTWLSettingsData) );
	}
	
	(void) NAMUT_PrintInstalledTitleETicketType();
	
	// ::::::::::::::::::::::::::::::::::::::::::::::
	// TWL�ݒ�f�[�^�t�@�C���̓ǂݍ���
	// ::::::::::::::::::::::::::::::::::::::::::::::
	g_isValidTSD = TRUE;
    {
        u8 *pBuffer = Alloc( LCFG_READ_TEMP );
        if( pBuffer ) {
			// NAND����TWL�{�̐ݒ�f�[�^�����[�h
			BOOL isRead = LCFG_ReadTWLSettings( (u8 (*)[LCFG_READ_TEMP])pBuffer );
			
			// ���[�h���s�t�@�C�������݂���ꍇ�́A�t�@�C�������J�o��
			if( LCFG_RecoveryTWLSettings() ) {
				if( isRead ) {
					// �~���[�f�[�^�̂����A����ł����[�h�ł��Ă����Ȃ牽�����Ȃ��B
				}else {
					// ���[�h�Ɋ��S�Ɏ��s���Ă����ꍇ�́A�t���b�V�����V�[�P���X�ցB
					LCFG_TSD_SetFlagFinishedBrokenTWLSettings( FALSE );
					(void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );	// LCFG_READ_TEMP > LCFG_WRITE_TEMP �Ȃ̂ŁApBuffer�����̂܂ܗ��p
				}
			}else {
				// ���J�o�����s���́AFALTAL�G���[
				UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
				g_isValidTSD = FALSE;
			}
            Free( pBuffer );
        }else {
			// �������m�ۂ��ł��Ȃ��������́AFATAL�G���[
			g_isValidTSD = FALSE;
		}
	}
	
	UTL_CaribrateTP( LCFG_TSD_GetTPCalibrationPtr() );
	
	// ::::::::::::::::::::::::::::::::::::::::::::::
	// SystemMenu�o�[�W����etc.�̓ǂݍ���
	// ::::::::::::::::::::::::::::::::::::::::::::::
	{
        u8 *pBuffer = Alloc( NA_VERSION_DATA_WORK_SIZE );
		
        if( pBuffer &&
			ReadSystemMenuVersionData( pBuffer, NA_VERSION_DATA_WORK_SIZE ) ) {
			// ���[�h����
		}else {
			// FATAL�G���[
			UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
		}
        Free( pBuffer );
	}
	
	// �o�[�W�������̕\��
	{
		char str_ver[ TWL_SYSMENU_VER_STR_LEN / sizeof(u16) ];
		int len = sizeof(str_ver);
		OS_TPrintf( "SystemMenuVersionData\n" );
		// ������
		if( STD_ConvertStringUnicodeToSjis( str_ver, &len, GetSystemMenuVersionString(), NULL, NULL ) == STD_RESULT_SUCCESS ) {
			OS_TPrintf( "  Version(str)       : %s\n", str_ver );
		}
		// ���l
		OS_TPrintf( "  Version(num)       : %d.%d\n", GetSystemMenuMajorVersion(), GetSystemMenuMinorVersion() );
		// ���[�U�[�̈�MAX�T�C�Y�̕\��
		OS_TPrintf( "  TotalUserAreadSize : 0x%08x\n", FSi_GetTotalUserAreaSize() );
		// EULA URL�̕\��
		OS_TPrintf( "  EULA URL           : %s\n", GetEULA_URL() );
		// NUP HostName�̕\��
		OS_TPrintf( "  NUP HostName       : %s\n", GetNUP_HostName() );
		// SystemMenuVersion���̃^�C���X�^���v�̎擾
		OS_TPrintf( "  Timestamp          : %08x\n", GetSystemMenuVersionTimeStamp() );
	}
	InitBG();
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	MachineSettingInit();
	// ���C�����[�v----------------------------
	while ( 1 ) {
		OS_WaitIrq( 1, OS_IE_V_BLANK );								// V�u�����N���荞�ݑ҂�
		
		ReadKeyPad();												// �L�[���͂̎擾
		
		(void)g_pNowProcess();
		
		GetAndDrawRTCData( &g_rtcDraw, FALSE );
	}
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

