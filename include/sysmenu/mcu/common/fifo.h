/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu-test - include
  File:     fifo.h

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
#ifndef SYSM_MCU_COMMON_FIFO_H_
#define SYSM_MCU_COMMON_FIFO_H_

#include <twl/types.h>
#include <sysmenu/sysmenu_lib/common/pxi.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define MCUTEST_PXI_CONTINUOUS_PACKET_MAX   2

#define MCUTEST_PXI_DATA_SIZE_MAX    ((MCUTEST_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // 最大データ数

#define MCUTEST_PXI_START_BIT        0x02000000  // 先頭パケットを意味する
#define MCUTEST_PXI_RESULT_BIT       0x00008000  // PXIの応答を示す

#define MCUTEST_PXI_DATA_NUMS_SHIFT  16          // データ数位置
#define MCUTEST_PXI_DATA_NUMS_MASK   0x00ff0000  // データ数領域
#define MCUTEST_PXI_COMMAND_SHIFT    8           // コマンド格納部分の位置
#define MCUTEST_PXI_COMMAND_MASK     0x00007f00  // コマンド格納部分のマスク
#define MCUTEST_PXI_1ST_DATA_SHIFT   0           // 先頭パケットのデータ位置
#define MCUTEST_PXI_1ST_DATA_MASK    0x000000ff  // 先頭パケットのデータ領域

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

typedef enum McuTestCommand
{
    MCU_TEST_COMMAND_NULL = 0,

    MCU_TEST_COMMAND_READ_REGISTER,
    MCU_TEST_COMMAND_WRITE_REGISTER,

    MCU_TEST_COMMAND_INTERRUPT,

    MCU_TEST_COMMAND_MAX
}
SYSMMcuCommand;

// 応答定義
typedef enum SYSMMcuPxiResult
{
    MCU_PXI_RESULT_SUCCESS = 0,        // 処理成功 (void/void*型) // 場合により後続パケットあり
    MCU_PXI_RESULT_SUCCESS_TRUE = 0,   // 処理成功 (BOOL型)
    MCU_PXI_RESULT_SUCCESS_FALSE,      // 処理成功 (BOOL型)
    MCU_PXI_RESULT_INVALID_COMMAND,    // 不正なPXIコマンド
    MCU_PXI_RESULT_INVALID_PARAMETER,  // 不正なパラメータ
    MCU_PXI_RESULT_ILLEGAL_STATUS,     // MCUの状態により処理を実行不可
    MCU_PXI_RESULT_BUSY,               // 他のリクエストを実行中
    MCU_PXI_RESULT_FATAL_ERROR,        // その他何らかの原因で処理に失敗
    MCU_PXI_RESULT_MAX
}
SYSMMcuPxiResult;


/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* SYSM_MCU_COMMON_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
