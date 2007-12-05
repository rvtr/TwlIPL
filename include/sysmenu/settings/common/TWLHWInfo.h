/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     HWInfo.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/


#ifndef	TWL_HW_INFO_H_
#define	TWL_HW_INFO_H_


#define HW_SECURE_INFO_WRITE_ENABLE_					// HW�Z�L���A���̃��C�g���R���p�C���X�C�b�`

#include <twl.h>
#include <sysmenu/settings/common/TWLStoreFile.h>

#ifdef HW_SECURE_INFO_WRITE_ENABLE_
#include <sysmenu/acsign.h>
#endif // HW_SECURE_INFO_WRITE_ENABLE_

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define TWL_HWINFO_FILE_LENGTH				( 16 * 1024 )
#define TWL_HWINFO_NORMAL_PATH				"nand:/sys/HWINFO_N.dat"
#define TWL_HWINFO_SECURE_PATH				"nand:/sys/HWINFO_S.dat"

#define TWL_HWINFO_NORMAL_VERSION			1			// HW���t�H�[�}�b�g�o�[�W����(�J�nNo.:1)
#define TWL_HWINFO_SECURE_VERSION			1			// HW���t�H�[�}�b�g�o�[�W����(�J�nNo.:1)
#define TWL_HWINFO_SERIALNO_LEN_AMERICA		11			// �{�̃V���A��No.��Max�i�k�Č�����11���j
#define TWL_HWINFO_SERIALNO_LEN_OTHERS		12			// �{�̃V���A��No.��Max
#define TWL_HWINFO_SERIALNO_LEN_MAX			16			// �{�̃V���A��No.��Max
#define TWL_HWINFO_CAMERA_LEN				1024		// �J������� [TODO]�T�C�Y����


// ���[�W�����R�[�h�i�̎ЕʂɂȂ錩���݁j
typedef enum TWLRegionCode {
	TWL_REGION_JAPAN     = 0,   // NCL
	TWL_REGION_AMERICA   = 1,   // NOA
	TWL_REGION_EUROPE    = 2,   // NOE
	TWL_REGION_AUSTRALIA = 3,   // NAL
	TWL_REGION_CHINA     = 4,   // IQue
	TWL_REGION_KOREA     = 5,   // NOK
	TWL_REGION_MAX
}TWLRegion;


// �{�̃V���A��No.
typedef struct TWLSerialNo {
	u8				length;								// �V���A��No.��
	u8				no[ TWL_HWINFO_SERIALNO_LEN_MAX ];	// �V���A��No.�iASCII������j
}TWLSerialNo;


// TWL_HW�Z�L���A���ݒ�f�[�^�i�����ŉ�����ی삷��K�v��������́j
// ����{�A�ߋ�ver�݊����l�����āA�ǉ��������Ȃ����j�ŁB
typedef struct TWLHWSecureInfo{
	u8				region;								// ���[�W����
	TWLSerialNo		serialNo;							// �{�̃V���A��No.
}TWLHWSecureInfo;	// 18bytes


// TWL_HW�m�[�}�����ݒ�f�[�^�i�����ŉ�����ی삷��K�v���Ȃ����́j
// ����{�A�ߋ�ver�݊����l�����āA�ǉ��������Ȃ����j�ŁB
typedef struct TWLHWNormalInfo{
	u8				rtcAdjust;							// RTC�����l
	u8				camera[ TWL_HWINFO_CAMERA_LEN ];	// �J�������
}TWLHWNormalInfo;	// 1025byte


#ifdef SDK_ARM9


//=========================================================
// HW��񃊁[�h���C�g�֐�
//=========================================================
// Normal���
	// �����ϐ��ւ̃��[�hfs
extern TSFReadResult THW_ReadNormalInfo( void );
	// �����ϐ��̒l�̃��C�g
extern BOOL          THW_WriteNormalInfo( void );
	// ���ڒl���w�肵�Ẵ��C�g�i�J���p�j
extern BOOL          THW_WriteNormalInfoDirect( const TWLHWNormalInfo *pSrcInfo );
	// �t�@�C�����J�o���[
extern BOOL          THW_RecoveryNormalInfo( TSFReadResult err );
	// ��LRead,Write�֐��Ŏg�p�����f�t�H���g�l�̃Z�b�g�i�J���p�j
extern void          TWH_SetNormalDefaultValue( const TWLHWNormalInfo *pSrcInfo );

// Secure���
	// ���[�h
extern TSFReadResult THW_ReadSecureInfo( void );
	// ���L�R���p�C���X�C�b�`��`���̂ݗL��
#ifdef HW_SECURE_INFO_WRITE_ENABLE_
	// ���C�g
extern BOOL          THW_WriteSecureInfo( const u8 *pPrivKeyDER );
	// ���ڒl���w�肵�Ẵ��C�g�i�J���p�j
extern BOOL          THW_WriteSecureInfoDirect( const TWLHWSecureInfo *pSrcInfo, const u8 *pPrivKeyDER );
	// �t�@�C�����J�o���[
extern BOOL          THW_RecoverySecureInfo( TSFReadResult err );
#endif // HW_SECURE_INFO_WRITE_ENABLE_
	// ��LRead,Write�֐��Ŏg�p�����f�t�H���g�l�̃Z�b�g�i�J���p�j
extern void          TWH_SetSecureDefaultValue( const TWLHWSecureInfo *pSrcInfo );


//=========================================================
// �i���L�A�N�Z�X�֐����g�p����static�ϐ��j
//=========================================================
extern TWLHWNormalInfo   s_hwInfoN;
extern TWLHWSecureInfo   s_hwInfoS;
#define GetHWN()		( &s_hwInfoN )
#define GetHWS()		( &s_hwInfoS )


//=========================================================
// �f�[�^�擾�iTHW_ReadNormalInfo, THW_ReadSecureInfo�œ������[�N�ɓǂݏo�������̎擾�j
//=========================================================

// RTC�I�t�Z�b�g�l�̎擾
static inline u8 THW_GetRTCAdjust( void )
{
	return	GetHWN()->rtcAdjust;
}


// �J�������̎擾
static inline void THW_GetCameraInfo( u8 *pDst )
{
	MI_CpuCopy8( GetHWN()->camera, pDst, TWL_HWINFO_CAMERA_LEN );
}


// ���[�W�����̎擾�B
static inline u8 THW_GetRegion( void )
{
	return	(u8)GetHWS()->region;
}


// �{�̃V���A��No.�̎擾
static inline void THW_GetSerialNo( TWLSerialNo *pDst )
{
	MI_CpuCopy8( &GetHWS()->serialNo, pDst, sizeof(TWLSerialNo) );
}


//=========================================================
// �f�[�^�Z�b�g�iTSD_ReadSettings�œ������[�N�ɓǂݏo�������ւ̒l�Z�b�g�j
//=========================================================

// RTC�I�t�Z�b�g�l�̃Z�b�g
static inline void THW_SetRTCAdjust( u8 adjust )
{
	GetHWN()->rtcAdjust = adjust;
}


// �J�������̃Z�b�g
static inline void THW_SetCameraInfo( u8 *pCamera )
{
	MI_CpuCopy8( pCamera, GetHWN()->camera, TWL_HWINFO_CAMERA_LEN );
}


// ���[�W�����̃Z�b�g�B
static inline void THW_SetRegion( u8 region )
{
	GetHWS()->region = region;
}

// �{�̃V���A��No.�̃Z�b�g
static inline void THW_SetSerialNo( TWLSerialNo *pSrc )
{
	MI_CpuCopy8( pSrc, &GetHWS()->serialNo, sizeof(TWLSerialNo) );
}


#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// TWL_HWINFO_H_
