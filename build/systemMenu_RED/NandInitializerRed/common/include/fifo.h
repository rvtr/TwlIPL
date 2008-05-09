/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera-test - include
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
#ifndef TWL_KAMI_TEST_FIFO_H_
#define TWL_KAMI_TEST_FIFO_H_

#include <twl/types.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define PXI_FIFO_TAG_KAMITEST PXI_FIFO_TAG_USER_1

#define KAMI_PXI_CONTINUOUS_PACKET_MAX 10
#define KAMITEST_PXI_DATA_SIZE_MAX    ((KAMI_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // 最大データ数

#define KAMITEST_PXI_START_BIT        0x02000000  // 先頭パケットを意味する
#define KAMITEST_PXI_RESULT_BIT       0x00008000  // PXIの応答を示す

#define KAMITEST_PXI_COMMAND_SHIFT    8           // コマンド格納部分の位置
#define KAMITEST_PXI_COMMAND_MASK     0x00007f00  // コマンド格納部分のマスク
#define KAMITEST_PXI_DATA_NUMS_MASK   0x00ff0000  // データ数領域
#define KAMITEST_PXI_DATA_NUMS_SHIFT  16          // データ数位置
#define KAMITEST_PXI_1ST_DATA_MASK    0x000000ff  // 先頭パケットのデータ領域
#define KAMITEST_PXI_1ST_DATA_SHIFT   0           // 先頭パケットのデータ位置

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

typedef enum KAMIPxiResult
{
    KAMI_PXI_RESULT_SUCCESS = 0,        // 処理成功 (void/void*型) // 場合により後続パケットあり
    KAMI_PXI_RESULT_SUCCESS_TRUE = 0,   // 処理成功 (BOOL型)
    KAMI_PXI_RESULT_SUCCESS_FALSE,      // 処理成功 (BOOL型)
    KAMI_PXI_RESULT_INVALID_COMMAND,    // 不正なPXIコマンド
    KAMI_PXI_RESULT_INVALID_PARAMETER,  // 不正なパラメータ
    KAMI_PXI_RESULT_ILLEGAL_STATUS,     // KAMIの状態により処理を実行不可
    KAMI_PXI_RESULT_BUSY,               // 他のリクエストを実行中
    KAMI_PXI_RESULT_FATAL_ERROR,        // その他何らかの原因で処理に失敗
    KAMI_PXI_RESULT_MAX
}
KAMIPxiResult;


typedef enum KamiCommand
{
	KAMI_TEST_COMMAND,
    KAMI_EXE_FORMAT,
    KAMI_NAND_IO,
    KAMI_NVRAM_IO,
    KAMI_MCU_IO,
    KAMI_ARM7_IO,
    KAMI_CDC_GO_DSMODE,
    KAMI_CLEAR_NAND_ERRORLOG,
    KAMI_GET_CAMERA_MODULE_TYPE
}
KamiCommand;


typedef enum CameraModuleType
{
    CAMERA_MODULE_TYPE_UNKNOWN,
    CAMERA_MODULE_TYPE_SHARP,
    CAMERA_MODULE_TYPE_MICRON
}
CameraModuleType;


typedef struct CameraModuleTypes
{
    CameraModuleType in;
    CameraModuleType out;
}
CameraModuleTypes;

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_KAMI_TEST_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
