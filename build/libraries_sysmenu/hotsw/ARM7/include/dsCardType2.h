/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __DSCARD_TYPE2_H__
#define __DSCARD_TYPE2_H__

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

//	DS�J�[�hType2�̃m�[�}�����[�h�̃��[�h�ύX (Type1�Ɠ�������)
#define ChangeModeNormal_DSType2	ChangeModeNormal_DSType1


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


// ��       �Q�[�����[�h�̃R�}���h       ��
//	DS�J�[�hType2�̃Q�[�����[�h��ID�ǂݍ��� (Type1�Ɠ�������)
#define ReadIDGame_DSType2			ReadIDGame_DSType1

//  DS�J�[�hType2�̃Q�[�����[�h�̎w��y�[�W�ǂݍ���
#define ReadPageGame_DSType2		ReadPageGame_DSType1

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE2_H__
