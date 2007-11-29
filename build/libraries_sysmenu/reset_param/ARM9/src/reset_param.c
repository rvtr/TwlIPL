/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     reset_param.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-26#$
  $Rev: 256 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu/reset_param/ARM9/reset_param.h>

// define data-------------------------------------------------------
#define RPi_RESET_PARAM_MAGIC_CODE			"TRST"
#define RPi_RESET_PARAM_MAGIC_CODE_LEN		4

// ���Z�b�g�p�����[�^�A�h���X�̎擾�i�����C�u���������j
#define RPi_GetResetParamAddr()			( (ResetParam *)0x02000100 )


// extern data-------------------------------------------------------

// function's prototype----------------------------------------------
static void RPi_DoHardwareReset( void );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------

// ���S�̂��߁A���̊֐���itcm�ɔz�u
#include <nitro/code32.h>
#include <twl/itcm_begin.h>
static void RPi_DoHardwareReset( void )
{
	// ���Z�b�g���ߔ��s
	PM_ForceToResetHardware();
	OS_Terminate();
}

#include <twl/itcm_end.h>
#include <nitro/codereset.h>

// ResetParam���w�肵�ă��Z�b�g
void RP_Reset( u8 type, NAMTitleId id, BootFlags *flag )
{
	// ���[�J�[�R�[�h�ƃQ�[���R�[�h���R�s�[
	u32 *maker_code_dest_addr = (u32 *)((u32)(RPi_GetResetParamAddr()) + HW_PARAM_DELIVER_ARG_MAKERCODE_OFS);
	u32 *game_code_dest_addr = (u32 *)((u32)(RPi_GetResetParamAddr()) + HW_PARAM_DELIVER_ARG_GAMECODE_OFS);
	u16 *maker_code_src_addr = (u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
	u32 *game_code_src_addr = (u32 *)(HW_TWL_ROM_HEADER_BUF + 0xc);
	*maker_code_dest_addr = (u32)*maker_code_src_addr;
	*game_code_dest_addr = *game_code_src_addr;
	
	// ���Z�b�g�p�����[�^�̐ݒ�
	RPi_GetResetParamAddr()->body.v1.bootTitleID = id;
	RPi_GetResetParamAddr()->body.v1.flags = *flag;
	MI_CpuCopyFast( RPi_RESET_PARAM_MAGIC_CODE, (char *)&RPi_GetResetParamAddr()->header.magicCode, RPi_RESET_PARAM_MAGIC_CODE_LEN);
	RPi_GetResetParamAddr()->header.bodyLength = sizeof(ResetParamBody);
	RPi_GetResetParamAddr()->header.crc16 = SVC_GetCRC16( 65535, &RPi_GetResetParamAddr()->body, RPi_GetResetParamAddr()->header.bodyLength );
	RPi_GetResetParamAddr()->header.type = type;
	
	
	DC_FlushRange(RPi_GetResetParamAddr(), 0x100 );
	DC_WaitWriteBufferEmpty();
	
	RPi_DoHardwareReset();
}
