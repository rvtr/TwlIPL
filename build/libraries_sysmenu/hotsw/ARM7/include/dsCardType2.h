/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_DSCARD_TYPE2_H__
#define __HOTSW_DSCARD_TYPE2_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================
// ��       �m�[�}�����[�h�̃R�}���h       ��
//	DS�J�[�hType2�̃m�[�}�����[�h��Boot Segment(4Kbyte)�ǂݍ���
HotSwState ReadBootSegNormal_DSType2(CardBootData *cbd);


// ��       �Z�L���A���[�h�̃R�}���h       ��
//	DS�J�[�hType2�̃Z�L���A���[�h��ID�ǂݍ���
HotSwState ReadIDSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h��Secure Segment(16Kbyte)�ǂݍ���
HotSwState ReadSegSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h��PN�W�F�l���[�^ON
HotSwState SwitchONPNGSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h��PN�W�F�l���[�^OFF
HotSwState SwitchOFFPNGSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h�̃��[�h�ύX
HotSwState ChangeModeSecure_DSType2(CardBootData *cbd);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_DSCARD_TYPE2_H__
