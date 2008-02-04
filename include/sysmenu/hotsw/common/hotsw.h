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

#define SYSM_HOTSW_ENABLE_ROMEMU

// Function prototype -------------------------------------------------------
// ����}�������̏�����
void HOTSW_Init(void);

// �J�[�h����f�[�^�����[�h����BNormal���[�h��Secure���[�h��Game���[�h���s��
BOOL HOTSW_LoadCardData(void);

// ARM7,9�̏풓���W���[����W�J����֐�
void HOTSW_LoadStaticModule(void);

// Boot Segment �o�b�t�@�̎w��
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment �o�b�t�@�̎w��
void HOTSW_SetSecureSegmentBuffer(void* buf, u32 size);

// Rom�G�~�����[�V���������i�[���Ă���o�b�t�@�̃|�C���^��Ԃ�
#ifdef SDK_ARM7
void* HOTSW_GetRomEmulationBuffer(void);
#else // SDK_ARM9
SDK_INLINE void* HOTSW_GetRomEmulationBuffer(void)
{
	return (void*)HW_ISD_RESERVED;
}
#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __MY_CARD_H__