/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __DSCARD_TYPE2_H__
#define __DSCARD_TYPE2_H__

#ifdef __cplusplus
extern "C" {
#endif


#include 	<twl.h>
#include 	"Card.h"

// ===========================================================================
// 	Function Describe
// ===========================================================================

// ��       �m�[�}�����[�h�̃R�}���h       ��
//	DS�J�[�hType2�̃m�[�}�����[�h��Boot Segment(4Kbyte)�ǂݍ���
void ReadBootSegNormal_DSType2(CardBootData *cbd);

// ��       �Z�L���A���[�h�̃R�}���h       ��
//	DS�J�[�hType2�̃Z�L���A���[�h��ID�ǂݍ���
void ReadIDSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h��Secure Segment(16Kbyte)�ǂݍ���
void ReadSegSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h��PN�W�F�l���[�^ON
void SwitchONPNGSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h��PN�W�F�l���[�^OFF
void SwitchOFFPNGSecure_DSType2(CardBootData *cbd);

//	DS�J�[�hType2�̃Z�L���A���[�h�̃��[�h�ύX
void ChangeModeSecure_DSType2(CardBootData *cbd);

// ��       �Q�[�����[�h�̃R�}���h       ��
// Type1�Ɠ����R�}���h���g��

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE2_H__
