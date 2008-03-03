/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.h
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_ROM_EMULATION_H__
#define __HOTSW_ROM_EMULATION_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================

// ��       �m�[�}�����[�h�̃R�}���h       ��
//	Rom Emulation�̃m�[�}�����[�h��Boot Segment(4Kbyte)�ǂݍ���
HotSwState ReadBootSegNormal_ROMEMU(CardBootData *cbd);


// ��       �Z�L���A���[�h�̃R�}���h       ��
//	Rom Emulation�̃Z�L���A���[�h��ID�ǂݍ���
HotSwState ReadIDSecure_ROMEMU(CardBootData *cbd);

//	Rom Emulation�̃Z�L���A���[�h��Secure Segment(16Kbyte)�ǂݍ���
HotSwState ReadSegSecure_ROMEMU(CardBootData *cbd);

//	Rom Emulation�̃Z�L���A���[�h��PN�W�F�l���[�^ON
HotSwState SwitchONPNGSecure_ROMEMU(CardBootData *cbd);

//	Rom Emulation�̃Z�L���A���[�h��PN�W�F�l���[�^OFF
HotSwState SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd);

//	Rom Emulation�̃Z�L���A���[�h�̃��[�h�ύX
HotSwState ChangeModeSecure_ROMEMU(CardBootData *cbd);


// ��       �Q�[�����[�h�̃R�}���h       ��
//	DS�J�[�hType1�̃Q�[�����[�h��ID�ǂݍ���
HotSwState ReadIDGame_ROMEMU(CardBootData *cbd);

//  DS�J�[�hType1�̃Q�[�����[�h�̎w��y�[�W�ǂݍ���
HotSwState ReadPageGame_ROMEMU(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_ROM_EMULATION_H__
