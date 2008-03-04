/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_DSCARD_COMMON_H__
#define __HOTSW_DSCARD_COMMON_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================
// ��       �m�[�}�����[�h�̃R�}���h       ��
//	�m�[�}�����[�h��ID�ǂݍ���
HotSwState ReadIDNormal(CardBootData *cbd);

//	�m�[�}�����[�h��Boot Segment(4Kbyte)�ǂݍ���
HotSwState ReadBootSegNormal(CardBootData *cbd);

//	�m�[�}�����[�h����Z�L���A���[�h�ւ̕ύX
HotSwState ChangeModeNormal(CardBootData *cbd);

//	�m�[�}�����[�h����Z�L���A�Q���[�h�ւ̕ύX
HotSwState ChangeModeNormal2(CardBootData *cbd);

// �J�[�h����KeyTable�����[�h
HotSwState LoadTable(void);

// �f�o�b�K��ROM�G�~�����[�V�������ǂݍ���
HotSwState ReadRomEmulationData(CardBootData *cbd);


// ��       �Z�L���A���[�h�̃R�}���h       ��
//	�Z�L���A���[�h��ID�ǂݍ���
HotSwState ReadIDSecure(CardBootData *cbd);

//	�Z�L���A���[�h��Secure Segment(16Kbyte)�ǂݍ���
HotSwState ReadSegSecure(CardBootData *cbd);

//	�Z�L���A���[�h��PN�W�F�l���[�^ON
HotSwState SwitchONPNGSecure(CardBootData *cbd);

//	�Z�L���A���[�h��PN�W�F�l���[�^OFF
HotSwState SwitchOFFPNGSecure(CardBootData *cbd);

//	�Z�L���A���[�h�̃��[�h�ύX
HotSwState ChangeModeSecure(CardBootData *cbd);


// ��       �Q�[�����[�h�̃R�}���h       ��
//	�Q�[�����[�h��ID�ǂݍ���
HotSwState ReadIDGame(CardBootData *cbd);

//  �Q�[�����[�h�̎w��y�[�W�ǂݍ���
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size);


// ��       �����֐�  				     ��
//	LE�R�}���h��BE�֕ϊ����ă��W�X�^�փZ�b�g
void HOTSWi_SetCommand(GCDCmd64 *cndLE);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_DSCARD_COMMON_H__
