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
void ReadBootSegNormal_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
void ChangeModeNormal_DSType1(CardBootData *cbd);


// ��       �Z�L���A���[�h�̃R�}���h       ��
//	DS�J�[�hType1�̃Z�L���A���[�h��ID�ǂݍ���
void ReadIDSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h��Secure Segment(16Kbyte)�ǂݍ���
void ReadSegSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h��PN�W�F�l���[�^ON
void SwitchONPNGSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h��PN�W�F�l���[�^OFF
void SwitchOFFPNGSecure_DSType1(CardBootData *cbd);

//	DS�J�[�hType1�̃Z�L���A���[�h�̃��[�h�ύX
void ChangeModeSecure_DSType1(CardBootData *cbd);


// ��       �Q�[�����[�h�̃R�}���h       ��
//	DS�J�[�hType1�̃Q�[�����[�h��ID�ǂݍ���
void ReadIDGame_DSType1(CardBootData *cbd);

//  DS�J�[�hType1�̃Q�[�����[�h�̎w��y�[�W�ǂݍ���
void ReadPageGame_DSType1(u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE1_H__
