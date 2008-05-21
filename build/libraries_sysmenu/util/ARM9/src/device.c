/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

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
#include <sysmenu.h>
#include <sysmenu/mcu.h>
#include <spi.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
#ifdef SDK_FINALROM
u32 PM_SendUtilityCommandAsync(u32 number, u16 parameter, u16* retValue, PMCallback callback, void *arg);
u32 PM_SendUtilityCommand(u32 number, u16 parameter, u16* retValue);
u32 PMi_WriteRegister(u16 registerAddr, u16 data);
u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg);
#endif // SDK_FINALROM

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static u8 s_brightness;
// const data------------------------------------------------------------------

// ============================================================================
//
// �f�o�C�X����
//
// ============================================================================

// �P�x�擾�ŌĂ�SYSM_ReadMcuRegisterAsync�̃R�[���o�b�N
static OSThreadQueue s_callback_queue;
static SYSMMcuResult s_callback_result;
static void MCUCallBack( SYSMMcuResult result, void *arg )
{
#pragma unused(arg)
	s_callback_result = result;
	OS_WakeupThread( &s_callback_queue );
}

// �o�b�N���C�g�P�x�擾
u8 SYSM_GetBackLightBlightness( void )
{
	u8 brightness;
#ifdef SDK_SUPPORT_PMIC_2
	if ( SYSMi_GetMcuVersion() <= 1 )
	{
		// X2�ȑO
		brightness = s_brightness;
	}
	else
#endif // SDK_SUPPORT_PMIC_2
	{
		// X3�ȍ~
		while( 1 )
		{
			// BUSY���Ǝ��s����̂Ő�������܂Ńg���C
			if ( MCU_RESULT_SUCCESS == SYSM_ReadMcuRegisterAsync( MCU_REG_BL_ADDR, &brightness, MCUCallBack, NULL ) )
			{
				OS_SleepThread( &s_callback_queue ); // �l���Ԃ��Ă���܂ŃX���[�v
				break;
			}
		}
	}
	
	return brightness;
}

// �o�b�N���C�g�P�x����
void SYSM_SetBackLightBrightness( u8 brightness )
{
	if( brightness > BACKLIGHT_BRIGHTNESS_MAX ) {
		OS_TPrintf( "Backlight brightness over! Change brightenss forcibly : %d -> %d\n", brightness, BACKLIGHT_BRIGHTNESS_MAX );
		brightness = BACKLIGHT_BRIGHTNESS_MAX;
	}
#ifdef SDK_SUPPORT_PMIC_2
	if ( SYSMi_GetMcuVersion() <= 1 )
	{
		s_brightness = brightness;
		( void )PMi_WriteRegister( REG_PMIC_BL_BRT_B_ADDR, (u8)(s_brightness * 2) );
	}
	else
#endif // SDK_SUPPORT_PMIC_2
	{
		// X3�ȍ~�̓}�C�R���ɕۑ����邾��
		while( 1 )
		{
			// BUSY���Ǝ��s����̂Ő�������܂Ńg���C
			if ( MCU_RESULT_SUCCESS == SYSM_WriteMcuRegisterAsync( MCU_REG_BL_ADDR, brightness, NULL, NULL ) )
			{
				break;
			}
		}
	}
	
}


// ���C�����XLED�̐���
void SYSMi_SetWirelessLED( BOOL enable )
{
	u8 value;
	// X3�ȍ~
	while( 1 )
	{
		// BUSY���Ǝ��s����̂Ő�������܂Ńg���C
		if ( MCU_RESULT_SUCCESS == SYSM_ReadMcuRegisterAsync( MCU_REG_WIFI_ADDR, &value, MCUCallBack, NULL ) )
		{
			OS_SleepThread( &s_callback_queue ); // �l���Ԃ��Ă���܂ŃX���[�v
			break;
		}
	}
	
	value = (u8)( ( value & ~MCU_REG_WIFI_LED_MASK ) | ( enable ? MCU_REG_WIFI_LED_MASK : 0 ) );
	
	while( 1 )
	{
		// BUSY���Ǝ��s����̂Ő�������܂Ńg���C
		if ( MCU_RESULT_SUCCESS == SYSM_WriteMcuRegisterAsync( MCU_REG_WIFI_ADDR, value, NULL, NULL ) )
		{
			break;
		}
	}
}


// �^�b�`�p�l���L�����u���[�V����
void SYSM_CaribrateTP( void )
{
	LCFGTWLTPCalibData store;
	TPCalibrateParam calibParam;
	
	// �{�̐ݒ�f�[�^����L�����u���[�V���������擾
	LCFG_TSD_GetTPCalibration( &store );
	
	// TP�L�����u���[�V����
	( void )TP_CalcCalibrateParam( &calibParam,							// �^�b�`�p�l��������
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
	TP_SetCalibrateParam( &calibParam );
	OS_TPrintf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
}


// RTC�N���b�N�␳�l���Z�b�g
void SYSMi_WriteAdjustRTC( void )
{
	RTCRawAdjust raw;
	raw.adjust = LCFG_THW_GetRTCAdjust();
	( void )RTCi_SetRegAdjust( &raw );
}


// �N������RTC�`�F�b�N
void SYSMi_CheckRTC( void )
{
	RTCDate date;
	RTCTime	time;
	
	// RTC�̃��Z�b�g or ���������l�����o�����ꍇ�͏���N���V�[�P���X�ցB
	( void )RTC_GetDateTime( &date, &time );
	if( !UTL_CheckRTCDate( &date ) ||
	    !UTL_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�ł�RTC�̓d�r���Ȃ��̂ŁA���񂱂��ɂЂ��������Đݒ�f�[�^���Е��N���A����Ă��܂��B�����h���X�C�b�`�B
		||
		SYSMi_GetWork()->flags.common.isResetRTC
#endif
		) {							// RTC�ُ̈�����o������Artc���̓t���O��rtcOffset��0�ɂ���NVRAM�ɏ������݁B
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		LCFG_TSD_SetRTCOffset( 0 );
		LCFG_TSD_SetRTCLastSetYear( 0 );
		{
			u8 *pBuffer = SYSM_Alloc( LCFG_WRITE_TEMP );
			if( pBuffer != NULL ) {
				LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
				SYSM_Free( pBuffer );
			}
		}
	}
}


// �X���[�v���[�h�ւ̑J��
void SYSM_GoSleepMode( void )
{
    // �W������
    if ( ! PAD_DetectFold() )
    {
        return;
    }

    // �f�o�b�K�N�����ɂ̓X���[�v�ɓ���Ȃ�
    if ( ! SYSM_IsRunOnDebugger() || (OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER) )
    {
        // �J�[�h���������o�ݒ�
        //   TWL�ł̓Q�[���J�[�h�̍ă��[�h���\�Ȃ���
        //   �X���[�v���̃J�[�h�������o�𖳌���
        //   �iDS-IPL�ł̓Q�[���J�[�h���N���ł��Ȃ��Ȃ�̂�
        //     ���W���[������ROM-ID�`�F�b�N�ŃG���[�ɂȂ��
        //     �V���b�g�_�E�����Ă����j
        OSIntrMode enable = OS_DisableInterrupts();
        reg_MI_MCCNT0 &= ~REG_MI_MCCNT0_I_MASK;
        OS_ResetRequestIrqMask( OS_IE_CARD_IREQ );
        OS_RestoreInterrupts( enable );

        // �X���[�v�J��
    	PM_GoSleepMode( PM_TRIGGER_COVER_OPEN |
	    				PM_TRIGGER_RTC_ALARM,
		    			0,
			    		0 );
    }
}


#ifdef SDK_FINALROM
/*---------------------------------------------------------------------------*
  Name:         PMi_WriteRegisterAsync

  Description:  send write register command to ARM7

  Arguments:    registerAddr : PMIC register number (0-3)
                data         : data written to PMIC register
                callback     : callback function
                arg          : callback argument

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PMi_UTIL_WRITEREG, (u16)((registerAddr<<16) | (data&0xff)), NULL, callback, arg);
}

u32 PMi_WriteRegister(u16 registerAddr, u16 data)
{
	return PM_SendUtilityCommand(PMi_UTIL_WRITEREG, (u16)((registerAddr<<16) | (data&0xff)), NULL);
}
#endif // SDK_FINALROM
