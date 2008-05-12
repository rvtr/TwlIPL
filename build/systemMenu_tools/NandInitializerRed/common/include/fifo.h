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
    �萔��`
 *---------------------------------------------------------------------------*/
#define PXI_FIFO_TAG_KAMITEST PXI_FIFO_TAG_USER_1

#define KAMI_PXI_CONTINUOUS_PACKET_MAX 10
#define KAMITEST_PXI_DATA_SIZE_MAX    ((KAMI_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // �ő�f�[�^��

#define KAMITEST_PXI_START_BIT        0x02000000  // �擪�p�P�b�g���Ӗ�����
#define KAMITEST_PXI_RESULT_BIT       0x00008000  // PXI�̉���������

#define KAMITEST_PXI_COMMAND_SHIFT    8           // �R�}���h�i�[�����̈ʒu
#define KAMITEST_PXI_COMMAND_MASK     0x00007f00  // �R�}���h�i�[�����̃}�X�N
#define KAMITEST_PXI_DATA_NUMS_MASK   0x00ff0000  // �f�[�^���̈�
#define KAMITEST_PXI_DATA_NUMS_SHIFT  16          // �f�[�^���ʒu
#define KAMITEST_PXI_1ST_DATA_MASK    0x000000ff  // �擪�p�P�b�g�̃f�[�^�̈�
#define KAMITEST_PXI_1ST_DATA_SHIFT   0           // �擪�p�P�b�g�̃f�[�^�ʒu

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

typedef enum KAMIPxiResult
{
    KAMI_PXI_RESULT_SUCCESS = 0,        // �������� (void/void*�^) // �ꍇ�ɂ��㑱�p�P�b�g����
    KAMI_PXI_RESULT_SUCCESS_TRUE = 0,   // �������� (BOOL�^)
    KAMI_PXI_RESULT_SUCCESS_FALSE,      // �������� (BOOL�^)
    KAMI_PXI_RESULT_INVALID_COMMAND,    // �s����PXI�R�}���h
    KAMI_PXI_RESULT_INVALID_PARAMETER,  // �s���ȃp�����[�^
    KAMI_PXI_RESULT_ILLEGAL_STATUS,     // KAMI�̏�Ԃɂ�菈�������s�s��
    KAMI_PXI_RESULT_BUSY,               // ���̃��N�G�X�g�����s��
    KAMI_PXI_RESULT_FATAL_ERROR,        // ���̑����炩�̌����ŏ����Ɏ��s
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
