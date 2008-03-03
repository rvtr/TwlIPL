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
//	DS�J�[�h�̃m�[�}�����[�h��ID�ǂݍ���
HotSwState ReadIDNormal(CardBootData *cbd);

//	DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
HotSwState ChangeModeNormal(CardBootData *cbd);

// �J�[�h����KeyTable�����[�h
HotSwState LoadTable(void);

// �f�o�b�K��ROM�G�~�����[�V�������ǂݍ���
HotSwState ReadRomEmulationData(CardBootData *cbd);


// ��       �Q�[�����[�h�̃R�}���h       ��
//	DS�J�[�h�̃Q�[�����[�h��ID�ǂݍ���
HotSwState ReadIDGame(CardBootData *cbd);

//  DS�J�[�h�̃Q�[�����[�h�̎w��y�[�W�ǂݍ���
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_DSCARD_COMMON_H__
