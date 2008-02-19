/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __DSCARD_TYPE1_H__
#define __DSCARD_TYPE1_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================

// ��       �m�[�}�����[�h�̃R�}���h       ��
//	DS�J�[�hType1�̃m�[�}�����[�h��Boot Segment(4Kbyte)�ǂݍ���
HotSwState ReadBootSegNormal_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
HotSwState ChangeModeNormal_DSType1(CardBootData *cbd);


// ��       �Z�L���A���[�h�̃R�}���h       ��
//	DS�J�[�hType1�̃Z�L���A���[�h��ID�ǂݍ���
HotSwState ReadIDSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h��Secure Segment(16Kbyte)�ǂݍ���
HotSwState ReadSegSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h��PN�W�F�l���[�^ON
HotSwState SwitchONPNGSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h��PN�W�F�l���[�^OFF
HotSwState SwitchOFFPNGSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h�̃��[�h�ύX
HotSwState ChangeModeSecure_DSType1(CardBootData *cbd);


// ��       �Q�[�����[�h�̃R�}���h       ��
//	DS�J�[�hType1�̃Q�[�����[�h��ID�ǂݍ���
HotSwState ReadIDGame_DSType1(CardBootData *cbd);

//  DS�J�[�hType1�̃Q�[�����[�h�̎w��y�[�W�ǂݍ���
HotSwState ReadPageGame_DSType1(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE1_H__
