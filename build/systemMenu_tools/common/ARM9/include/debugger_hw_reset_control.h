/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     debugger_hw_reset_control.h

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
#ifndef DEBUGGER_HW_RESET_CONTROL_H_
#define DEBUGGER_HW_RESET_CONTROL_H_


#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetDisable

  Description:  IS-TWL-DEBUGGER�ł̃n�[�h�E�F�A���Z�b�g���֎~���܂��B
                ���̋@�\�̓f�o�b�K�f�B�[�[�u���t���O���w�肵��SRL 
				�ł̂ݗL���ł��B��������Ƃ��ẮA5�b���ɃJ�[�h�A�N�Z�X��
				�s���X���b�h�𐶐��N�����Ă��܂��BIS-TWL-DEBUGGER��
				�J�[�h�A�N�Z�X���Ď����Ă���10�b�ԃJ�[�h�A�N�Z�X���Ȃ�
				�ꍇ�Ƀn�[�h�E�F�A���Z�b�g��������d�g�݂ɂȂ��Ă��܂��B

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetDisable( void );

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetEnable

  Description:  IS-TWL-DEBUGGER�ł̃n�[�h�E�F�A���Z�b�g�������܂��B
                ���̋@�\�̓f�o�b�K�f�B�[�[�u���t���O���w�肵��SRL 
				�ł̂ݗL���ł��B���ۂɃn�[�h�E�F�A���Z�b�g���\�ɂȂ�ɂ�
				�ő��10�b������܂��B

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetEnable( void );

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* DEBUGGER_HW_RESET_CONTROL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
