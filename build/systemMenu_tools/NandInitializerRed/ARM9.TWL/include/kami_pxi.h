/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_pxi.h

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

KAMIResult ExeFormatAsync(FormatMode format_mode, KAMICallback callback);
KAMIResult ExeFormat(FormatMode format_mode);
KAMIResult kamiNandIo(u32 block, void* buffer, u32 count, BOOL is_read);
KAMIResult kamiNvramIo(u32 address, void* buffer, u32 size, BOOL is_read);
KAMIResult kamiMcuWriteFirm(void* buffer );
KAMIResult kamiMcuIo(u32 reg_no, void* buffer, u32 value, BOOL is_read);
KAMIResult kamiARM7Io(u32 addr, u32* buffer, u32 value, BOOL is_read);
KAMIResult kamiCDC_GoDsMode( void );
KAMIResult kamiClearNandErrorLog( void );
KAMIResult kamiGetCameraModuleTypesAsync( CameraModuleTypes *pTypes, KAMICallback callback, void* arg );
KAMIResult kamiGetCameraModuleTypes( CameraModuleTypes *pTypes );

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
static KAMIResult kamiNvramRead(u32 adress, void* buffer, u32 size)
{
	return kamiNvramIo(adress, buffer, size, TRUE);
}
static KAMIResult kamiNvramWrite(u32 adress, void* buffer, u32 size)
{
	return kamiNvramIo(adress, buffer, size, FALSE);
}
static KAMIResult kamiMcuRead(u8 reg_no, void* buffer)
{
	return kamiMcuIo((u32)reg_no, buffer, 0, TRUE);
}
static KAMIResult kamiMcuWrite(u8 reg_no, u8 value)
{
	return kamiMcuIo((u32)reg_no, NULL, (u32)value, FALSE);
}
static KAMIResult kamiARM7Read(u32 addr, void* buffer)
{
	return kamiARM7Io(addr, buffer, 0, TRUE);
}
static KAMIResult kamiARM7Write(u32 addr, u8 value)
{
	return kamiARM7Io(addr, NULL, value, FALSE);
}

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_KAMI_TEST_KAMITEST_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
