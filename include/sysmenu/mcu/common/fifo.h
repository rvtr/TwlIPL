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
    �萔��`
 *---------------------------------------------------------------------------*/
#define MCUTEST_PXI_CONTINUOUS_PACKET_MAX   2

#define MCUTEST_PXI_DATA_SIZE_MAX    ((MCUTEST_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // �ő�f�[�^��

#define MCUTEST_PXI_START_BIT        0x02000000  // �擪�p�P�b�g���Ӗ�����
#define MCUTEST_PXI_RESULT_BIT       0x00008000  // PXI�̉���������

#define MCUTEST_PXI_DATA_NUMS_SHIFT  16          // �f�[�^���ʒu
#define MCUTEST_PXI_DATA_NUMS_MASK   0x00ff0000  // �f�[�^���̈�
#define MCUTEST_PXI_COMMAND_SHIFT    8           // �R�}���h�i�[�����̈ʒu
#define MCUTEST_PXI_COMMAND_MASK     0x00007f00  // �R�}���h�i�[�����̃}�X�N
#define MCUTEST_PXI_1ST_DATA_SHIFT   0           // �擪�p�P�b�g�̃f�[�^�ʒu
#define MCUTEST_PXI_1ST_DATA_MASK    0x000000ff  // �擪�p�P�b�g�̃f�[�^�̈�

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

// ������`
typedef enum SYSMMcuPxiResult
{
    MCU_PXI_RESULT_SUCCESS = 0,        // �������� (void/void*�^) // �ꍇ�ɂ��㑱�p�P�b�g����
    MCU_PXI_RESULT_SUCCESS_TRUE = 0,   // �������� (BOOL�^)
    MCU_PXI_RESULT_SUCCESS_FALSE,      // �������� (BOOL�^)
    MCU_PXI_RESULT_INVALID_COMMAND,    // �s����PXI�R�}���h
    MCU_PXI_RESULT_INVALID_PARAMETER,  // �s���ȃp�����[�^
    MCU_PXI_RESULT_ILLEGAL_STATUS,     // MCU�̏�Ԃɂ�菈�������s�s��
    MCU_PXI_RESULT_BUSY,               // ���̃��N�G�X�g�����s��
    MCU_PXI_RESULT_FATAL_ERROR,        // ���̑����炩�̌����ŏ����Ɏ��s
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
