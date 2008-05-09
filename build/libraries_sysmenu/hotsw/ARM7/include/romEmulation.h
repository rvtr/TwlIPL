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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_ROM_EMULATION_H__
