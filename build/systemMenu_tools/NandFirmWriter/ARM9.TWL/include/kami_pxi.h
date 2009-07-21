/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_pxi.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-25#$
  $Rev: 2003 $
  $Author: kamikawa $
 *---------------------------------------------------------------------------*/

#ifndef TWL_KAMI_TEST_KAMITEST_H_
#define TWL_KAMI_TEST_KAMITEST_H_

#include "fifo.h"

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// �������ʒ�`
typedef enum KAMIResult
{
    KAMI_RESULT_SUCCESS = 0,
    KAMI_RESULT_SUCCESS_TRUE = 0,
    KAMI_RESULT_SUCCESS_FALSE,
    KAMI_RESULT_INVALID_COMMAND,
    KAMI_RESULT_INVALID_PARAMETER,
    KAMI_RESULT_ILLEGAL_STATUS,    
    KAMI_RESULT_BUSY,
    KAMI_RESULT_FATAL_ERROR,
    KAMI_RESULT_SEND_ERROR,
    KAMI_RESULT_MAX
}
KAMIResult;


typedef enum {
	FORMAT_MODE_QUICK,	// Quick�t�H�[�}�b�g
	FORMAT_MODE_FULL	// Full�t�H�[�}�b�g(�e�p�[�e�B�V��������0xFF�Ŗ��߂�j
} FormatMode;

// �R�[���o�b�N
typedef void (*KAMICallback)(KAMIResult result, void *arg);


void KamiPxiInit( void );

KAMIResult kamiNandIo(u32 block, void* buffer, u32 count, BOOL is_read);

// (�d�v)
// ARM7���ǂݏ������邽�߃��[�h�O��Invalidate�A���C�g�O�̓t���b�V�����Ă��������B
// 
static KAMIResult kamiNandRead(u32 block, void* buffer, u32 count)
{
	return kamiNandIo(block, buffer, count, TRUE);
}
static KAMIResult kamiNandWrite(u32 block, void* buffer, u32 count)
{
	return kamiNandIo(block, buffer, count, FALSE);
}

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_KAMI_TEST_KAMITEST_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
