/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - mcu
  File:     mcu_firm.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <twl/i2c/ARM7/i2c.h>
#include "mcu_firm.h"

//#define PRINT_DEBUG
//#define PRINT_DEBUG_MINI  // rough version

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#undef PRINT_DEBUG_MINI     // because of the alternative option
#define DBG_PRINT_PROFILE_INIT  OSTick debug, d
#define DBG_PRINT_PROFILE_BEGIN()   (debug = OS_GetTick())
#define DBG_PRINT_PROFILE_END()     (d=(int)OS_TicksToMilliSeconds(OS_GetTick()-debug), (d ? OS_TPrintf("(%d msec)\n", d) : (void)0))
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_PRINT_PROFILE_INIT
#define DBG_PRINT_PROFILE_BEGIN()   ((void)0)
#define DBG_PRINT_PROFILE_END()     ((void)0)
#endif
#ifdef PRINT_DEBUG_MINI
#include <nitro/os/common/printf.h>
#define DBG_PRINT_FUNC()        OS_TPrintf("%s(0x%02X, 0x%02X);\n", __func__, I2CiDeviceAddrTable[id], reg)
#define DBG_PRINT_FUNC1(data)   OS_TPrintf("%s(0x%02X, 0x%02X, 0x%02X);\n", __func__, I2CiDeviceAddrTable[id], reg, (data))
#define DBG_PRINT_ERR()         OS_TPrintf("  Failed(%d) @ %d\n", error, r)
#else
#define DBG_PRINT_FUNC()        ((void)0)
#define DBG_PRINT_FUNC1(data)   ((void)0)
//#define DBG_PRINT_ERR()         ((void)0)
#define DBG_PRINT_ERR()         OS_TPrintf("%s: I2C Error (0x%X, 0x%X) %d/%d.\n", __func__, I2CiDeviceAddrTable[id], reg, r+1, RETRY_COUNT);
//#define DBG_PRINT_ERR()         OS_TPanic("%s: I2C Error (0x%X, 0x%X) %d/%d.\n", __func__, I2CiDeviceAddrTable[id], reg, r+1, RETRY_COUNT);
#endif


static const u8 I2CiDeviceAddrTable[I2C_SLAVE_NUM] =
    {
        I2C_ADDR_CAMERA_MICRON_IN,
        I2C_ADDR_CAMERA_MICRON_OUT,
        I2C_ADDR_CAMERA_SHARP_IN,
        I2C_ADDR_CAMERA_SHARP_OUT,
        I2C_ADDR_MICRO_CONTROLLER,
        I2C_ADDR_DEBUG_LED,
        I2C_ADDR_DEBUGGER,
    };

static BOOL slowRate = 0;

static inline void I2Ci_Start( void )
{
    reg_OS_I2C_CNT = (u8)((1 << REG_OS_I2C_CNT_E_SHIFT) |
                          (1 << REG_OS_I2C_CNT_I_SHIFT) |   // 割り込み禁止は IE にて行うことで仕様統一
                          (I2C_WRITE << REG_OS_I2C_CNT_RW_SHIFT) |
                          (0 << REG_OS_I2C_CNT_ACK_SHIFT) |
                          (1 << REG_OS_I2C_CNT_START_SHIFT));
}

static inline void I2Ci_Continue( I2CReadWrite rw )
{
    reg_OS_I2C_CNT = (u8)((1 << REG_OS_I2C_CNT_E_SHIFT) |
                          (1 << REG_OS_I2C_CNT_I_SHIFT) |
                          (rw << REG_OS_I2C_CNT_RW_SHIFT) |
                          (rw << REG_OS_I2C_CNT_ACK_SHIFT));
}

static inline void I2Ci_Stop( I2CReadWrite rw )
{
    reg_OS_I2C_CNT = (u8)((1 << REG_OS_I2C_CNT_E_SHIFT) |
                          (1 << REG_OS_I2C_CNT_I_SHIFT) |
                          (rw << REG_OS_I2C_CNT_RW_SHIFT) |
                          (0 << REG_OS_I2C_CNT_ACK_SHIFT) |
                          (1 << REG_OS_I2C_CNT_STOP_SHIFT));
}

static inline void I2Ci_StopPhase1( I2CReadWrite rw )
{
    reg_OS_I2C_CNT = (u8)((1 << REG_OS_I2C_CNT_E_SHIFT) |
                          (1 << REG_OS_I2C_CNT_I_SHIFT) |
                          (rw << REG_OS_I2C_CNT_RW_SHIFT) |
                          (0 << REG_OS_I2C_CNT_ACK_SHIFT));
}
static inline void I2Ci_StopPhase2( void )
{
    reg_OS_I2C_CNT = (u8)((1 << REG_OS_I2C_CNT_E_SHIFT) |
                          (1 << REG_OS_I2C_CNT_I_SHIFT) |
                          (1 << REG_OS_I2C_CNT_STOP_SHIFT) |
                          (1 << REG_OS_I2C_CNT_NT_SHIFT));
}

static inline void I2Ci_WaitEx( void )  // support slowRate
{
    DBG_PRINT_PROFILE_INIT;
    I2Ci_Wait();
    DBG_PRINT_PROFILE_BEGIN();
    SVC_WaitByLoop(slowRate);
    DBG_PRINT_PROFILE_END();
}

static inline void I2Ci_StopEx( I2CReadWrite rw )   // support slowRate
{
    if (slowRate)
    {
        I2Ci_StopPhase1(rw);
        I2Ci_Wait();
        SVC_WaitByLoop(slowRate);
        I2Ci_StopPhase2();
    }
    else
    {
        I2Ci_Stop(rw);
    }
}

static inline void I2Ci_SetData( u8 data )
{
    DBG_PRINTF("%02X", data);
    reg_OS_I2C_DAT = data;
}

static inline BOOL I2Ci_GetResult( void )
{
    I2Ci_Wait();
    DBG_PRINTF("%c", (reg_OS_I2C_CNT & REG_OS_I2C_CNT_ACK_MASK) ? '.' : '*');
    return (BOOL)((reg_OS_I2C_CNT & REG_OS_I2C_CNT_ACK_MASK) >> REG_OS_I2C_CNT_ACK_SHIFT);
}

static inline BOOL I2Ci_SendStart( I2CSlave id )
{
    DBG_PRINTF("\n");
    I2Ci_Wait();
    I2Ci_SetData( (u8)(I2CiDeviceAddrTable[id] | I2C_WRITE) );
    I2Ci_Start();
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_SendMiddle( u8 data )
{
    I2Ci_WaitEx();
    I2Ci_SetData( data );
    I2Ci_Continue( I2C_WRITE );
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_SendLast( u8 data )
{
    I2Ci_WaitEx();
    I2Ci_SetData( data );
    I2Ci_StopEx( I2C_WRITE );
    return I2Ci_GetResult();
}

#define SLOW_RATE_DEFAULT   0x50
#define SLOW_RATE_SHORT     0x140
#define SLOW_RATE_LONG      (HW_CPU_CLOCK_ARM7 / 13)    // 300msec
#define SLOW_RATE_ENTER     (HW_CPU_CLOCK_ARM7 / 160)   // 25msec

BOOL MCU_WriteFirm(const unsigned char* hex)
{
    BOOL result = TRUE;
    BOOL temp;

    if ( !hex )
    {
        return FALSE;   // no data
    }

    I2C_Lock();

    slowRate = SLOW_RATE_DEFAULT;

    // start phase
    result &= I2Ci_SendStart( I2C_SLAVE_MICRO_CONTROLLER );
    result &= I2Ci_SendMiddle( 0x77 );        // free register 7
    result &= I2Ci_SendMiddle( 0x4A );        // goto firm writing mode

    slowRate = SLOW_RATE_LONG;

    // main phase
    while ( hex[0] == ':' && ( hex[3] < '2' || (hex[3] == '2' && hex[4] < '4') )) // フォーマットが正しく0x2400以前のアドレスである場合に処理する
    {
        // データ終端チェック (基本的にこの前で終了している)
        if ( !MI_CpuComp8( hex, ":00000001FF", 11) )
        {
            break;
        }
        // 無視行チェック
        if ( hex[1] == '1' && hex[2] == '0' && !MI_CpuComp8( &hex[9], "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 32 ) )
        {
            while ( *hex++ != '\n' )
            {
                // skip
            }
            continue;
        }
        // 最初の1文字 (':'のはず)
        result &= I2Ci_SendMiddle( *hex++ );

        slowRate = SLOW_RATE_SHORT;

        // 通常出力
        temp = TRUE;    /* 1回遅延させることで'\n'の結果を無視する */
        do
        {
            result &= temp;
            temp = I2Ci_SendMiddle( *hex );
        }
        while ( *hex++ != '\n' );

        slowRate = SLOW_RATE_ENTER;
    }

    // stop phase (only 2nd call)
    I2Ci_WaitEx();
    I2Ci_StopPhase2();

    I2C_Unlock();

    return result;
}

