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
// デバイス制御
//
// ============================================================================

// 輝度取得で呼ぶSYSM_ReadMcuRegisterAsyncのコールバック
static OSThreadQueue s_callback_queue;
static SYSMMcuResult s_callback_result;
static void MCUCallBack( SYSMMcuResult result, void *arg )
{
#pragma unused(arg)
	s_callback_result = result;
	OS_WakeupThread( &s_callback_queue );
}

// バックライト輝度取得
u8 SYSM_GetBackLightBlightness( void )
{
	u8 brightness;
#ifdef SDK_SUPPORT_PMIC_2
	if ( SYSMi_GetMcuVersion() <= 1 )
	{
		// X2以前
		brightness = s_brightness;
	}
	else
#endif // SDK_SUPPORT_PMIC_2
	{
		// X3以降
		while( 1 )
		{
			// BUSYだと失敗するので成功するまでトライ
			if ( MCU_RESULT_SUCCESS == SYSM_ReadMcuRegisterAsync( MCU_REG_BL_ADDR, &brightness, MCUCallBack, NULL ) )
			{
				OS_SleepThread( &s_callback_queue ); // 値が返ってくるまでスリープ
				break;
			}
		}
	}
	
	return brightness;
}

// バックライト輝度調整
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
		// X3以降はマイコンに保存するだけ
		while( 1 )
		{
			// BUSYだと失敗するので成功するまでトライ
			if ( MCU_RESULT_SUCCESS == SYSM_WriteMcuRegisterAsync( MCU_REG_BL_ADDR, brightness, NULL, NULL ) )
			{
				break;
			}
		}
	}
	
}


// ワイヤレスLEDの制御
void SYSMi_SetWirelessLED( BOOL enable )
{
	u8 value;
	// X3以降
	while( 1 )
	{
		// BUSYだと失敗するので成功するまでトライ
		if ( MCU_RESULT_SUCCESS == SYSM_ReadMcuRegisterAsync( MCU_REG_WIFI_ADDR, &value, MCUCallBack, NULL ) )
		{
			OS_SleepThread( &s_callback_queue ); // 値が返ってくるまでスリープ
			break;
		}
	}
	
	value = (u8)( ( value & ~MCU_REG_WIFI_LED_MASK ) | ( enable ? MCU_REG_WIFI_LED_MASK : 0 ) );
	
	while( 1 )
	{
		// BUSYだと失敗するので成功するまでトライ
		if ( MCU_RESULT_SUCCESS == SYSM_WriteMcuRegisterAsync( MCU_REG_WIFI_ADDR, value, NULL, NULL ) )
		{
			break;
		}
	}
}


// タッチパネルキャリブレーション
void SYSM_CaribrateTP( void )
{
	LCFGTWLTPCalibData store;
	TPCalibrateParam calibParam;
	
	// 本体設定データからキャリブレーション情報を取得
	LCFG_TSD_GetTPCalibration( &store );
	
	// TPキャリブレーション
	( void )TP_CalcCalibrateParam( &calibParam,							// タッチパネル初期化
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
	TP_SetCalibrateParam( &calibParam );
	OS_TPrintf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
}


// RTCクロック補正値をセット
void SYSMi_WriteAdjustRTC( void )
{
	RTCRawAdjust raw;
	raw.adjust = LCFG_THW_GetRTCAdjust();
	( void )RTCi_SetRegAdjust( &raw );
}


// 起動時のRTCチェック
void SYSMi_CheckRTC( void )
{
	RTCDate date;
	RTCTime	time;
	
	// RTCのリセット or おかしい値を検出した場合は初回起動シーケンスへ。
	( void )RTC_GetDateTime( &date, &time );
	if( !UTL_CheckRTCDate( &date ) ||
	    !UTL_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// 青デバッガではRTCの電池がないので、毎回ここにひっかかって設定データが片方クリアされてしまう。これを防ぐスイッチ。
		||
		SYSMi_GetWork()->flags.common.isResetRTC
#endif
		) {							// RTCの異常を検出したら、rtc入力フラグ＆rtcOffsetを0にしてNVRAMに書き込み。
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


// スリープモードへの遷移
void SYSM_GoSleepMode( void )
{
    // 蓋閉じ判定
    if ( ! PAD_DetectFold() )
    {
        return;
    }

    // デバッガ起動時にはスリープに入らない
    if ( ! SYSM_IsRunOnDebugger() || (OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER) )
    {
        // カード抜け無検出設定
        //   TWLではゲームカードの再ロードが可能なため
        //   スリープ時のカード抜け検出を無効化
        //   （DS-IPLではゲームカードが起動できなくなるので
        //     レジューム時のROM-IDチェックでエラーになると
        //     シャットダウンしていた）
        OSIntrMode enable = OS_DisableInterrupts();
        reg_MI_MCCNT0 &= ~REG_MI_MCCNT0_I_MASK;
        OS_ResetRequestIrqMask( OS_IE_CARD_IREQ );
        OS_RestoreInterrupts( enable );

        // スリープ遷移
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
