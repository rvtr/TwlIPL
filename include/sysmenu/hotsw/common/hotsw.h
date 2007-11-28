/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     Card.h
 *---------------------------------------------------------------------------*/
#ifndef __MY_CARD_H__
#define __MY_CARD_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototype -------------------------------------------------------
// ����}�������̏�����
void HOTSW_Init(void);

// �J�[�h�N���BNormal���[�h��Secure���[�h��Game���[�h���s��
BOOL HOTSW_Boot(void);

// ARM7,9�̏풓���W���[����W�J����֐�
void HOTSW_LoadStaticModule(void);

// Boot Segment �o�b�t�@�̎w��
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment �o�b�t�@�̎w��
void HOTSW_SetSecureSegmentBuffer(void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __MY_CARD_H__