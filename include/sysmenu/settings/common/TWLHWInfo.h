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


#define HW_SIGNATURE_ENABLE_					// HW�Z�L���A���̏��������L���R���p�C���X�C�b�`

#include <twl.h>
#include <sysmenu/settings/common/TWLStoreFile.h>

#ifdef HW_SIGNATURE_ENABLE_
#include <sysmenu/acsign.h>
#endif // HW_SIGNATURE_ENABLE_

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
#define TWL_HWINFO_SERIALNO_LEN_MAX			15			// �{�̃V���A��No.��Max(�I�[�t���Ȃ̂ŁA14bytes�܂Ŋg����)
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

#define TWL_LANG_BITMAP_JAPAN		( ( 0x0001 << TWL_LANG_JAPANESE ) ) 	// JPN�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_AMERICA		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// AME�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_EUROPE		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_GERMAN  ) | \
									  ( 0x0001 << TWL_LANG_ITALIAN  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// EUR�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_AUSTRALIA	( ( 0x0001 << TWL_LANG_ENGLISH  ) ) 	// AUS�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_CHINA		( ( 0x0001 << TWL_LANG_SIMP_CHINESE ) ) // CHI�łł̑Ή�����r�b�g�}�b�v
#define TWL_LANG_BITMAP_KOREA		( ( 0x0001 << TWL_LANG_KOREAN ) ) 		// KOR�łł̑Ή�����r�b�g�}�b�v


// TWL_HW�m�[�}�����ݒ�f�[�^�i�����ŉ�����ی삷��K�v���Ȃ����́j
// ����{�A�ߋ�ver�݊����l�����āA�ǉ��������Ȃ����j�ŁB
typedef struct TWLHWNormalInfo{
	u8				rtcAdjust;							// RTC�����l
	u8				camera[ TWL_HWINFO_CAMERA_LEN ];	// �J�������
}TWLHWNormalInfo;	// 1025byte


// TWL_HW�Z�L���A���ݒ�f�[�^�i�����ŉ�����ی삷��K�v��������́j
// ����{�A�ߋ�ver�݊����l�����āA�ǉ��������Ȃ����j�ŁB
// ��SystemShared�̈�Ƀ��[�h���Ă���̂ŁA�T�C�Y���ς�������͒��ӂ���B
typedef struct TWLHWSecureInfo{
	u32				validLanguageBitmap;						// �{�̂ŗL���Ȍ���R�[�h���r�b�g��ŕ\��
	u8				region;										// ���[�W����
	u8				serialNo[ TWL_HWINFO_SERIALNO_LEN_MAX ];	// �V���A��No.�i�I�[�t��ASCII������j
}TWLHWSecureInfo;	// 20bytes


#ifdef SDK_ARM9


//=========================================================
// HW��񃊁[�h���C�g�֐�
//=========================================================
// Normal���
	// �����ϐ��ւ̃��[�h
extern TSFReadResult THW_ReadNormalInfo( void );
	// �����ϐ��̒l�����C�g�i��Ƀ��[�h���Ă����K�v������j
extern BOOL          THW_WriteNormalInfo( void );
	// ���ڒl���w�肵�Ẵ��C�g�i�J���p�j
extern BOOL          THW_WriteNormalInfoDirect( const TWLHWNormalInfo *pSrcInfo );
	// �t�@�C�����J�o���[
extern BOOL          THW_RecoveryNormalInfo( TSFReadResult err );
	// ��LRead,Write�֐��Ŏg�p�����f�t�H���g�l�̃Z�b�g�i�J���p�j
extern void          THW_SetDefaultNormalInfo( const TWLHWNormalInfo *pSrcInfo );

extern void          THW_ClearNormalInfoDirect( TWLHWNormalInfo *pDstInfo );
extern const TWLHWNormalInfo *THW_GetDefaultNormalInfo( void );
extern const TWLHWNormalInfo *THW_GetNormalInfo( void );

// Secure���
	// �����ϐ��ւ̃��[�h
extern TSFReadResult THW_ReadSecureInfo( void );
extern TSFReadResult THW_ReadSecureInfo_NoCheck( void );	// �����m�[�`�F�b�N�Ń��[�h
	// �����ϐ��̒l�����C�g�i��Ƀ��[�h���Ă����K�v������j
extern BOOL          THW_WriteSecureInfo( const u8 *pPrivKeyDER );	// pPrivKeyDER��NULL�Ȃ珐���Ȃ��Ń��C�g
	// ���ڒl���w�肵�Ẵ��C�g�i�J���p�j
extern BOOL          THW_WriteSecureInfoDirect( const TWLHWSecureInfo *pSrcInfo, const u8 *pPrivKeyDER );	// pPrivKeyDER��NULL�Ȃ珐���Ȃ��Ń��C�g
	// �t�@�C�����J�o���[
extern BOOL          THW_RecoverySecureInfo( TSFReadResult err );
	// ��LRead,Write�֐��Ŏg�p�����f�t�H���g�l�̃Z�b�g�i�J���p�j
extern void          THW_SetDefaultSecureInfo( const TWLHWSecureInfo *pSrcInfo );

extern void          THW_ClearSecureInfoDirect( TWLHWSecureInfo *pDstInfo );
extern const TWLHWSecureInfo *THW_GetDefaultSecureInfo( void );
extern const TWLHWSecureInfo *THW_GetSecureInfo( void );

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


// �J�������ւ̃|�C���^�̎擾
static inline const u8 *THW_GetCameraInfoPtr( void )
{
	return (const u8 *)GetHWN()->camera;
}


// ����r�b�g�}�b�v�l�̎擾
static inline u32 THW_GetValidLanguageBitmap( void )
{
	return	GetHWS()->validLanguageBitmap;
}


// ���[�W�����̎擾�B
static inline u8 THW_GetRegion( void )
{
	return	(u8)GetHWS()->region;
}


// �{�̃V���A��No.�̎擾
static inline void THW_GetSerialNo( u8 *pDst )
{
	MI_CpuCopy8( &GetHWS()->serialNo, pDst, TWL_HWINFO_SERIALNO_LEN_MAX );
}


// �{�̃V���A��No.�ւ̃|�C���^�̎擾
static inline const u8 *THW_GetSerialNoPtr( void )
{
	return (const u8 *)&GetHWS()->serialNo;
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
static inline void THW_SetCameraInfo( const u8 *pCamera )
{
	MI_CpuCopy8( pCamera, GetHWN()->camera, TWL_HWINFO_CAMERA_LEN );
}


// ����r�b�g�}�b�v�l�̃Z�b�g
static inline void THW_SetValidLanguageBitmap( u32 langBitmap )
{
	GetHWS()->validLanguageBitmap = langBitmap;
}


// ���[�W�����̃Z�b�g�B
static inline void THW_SetRegion( u8 region )
{
	GetHWS()->region = region;
}

// �{�̃V���A��No.�̃Z�b�g
static inline void THW_SetSerialNo( const u8 *pSrc )
{
	MI_CpuCopy8( pSrc, &GetHWS()->serialNo, TWL_HWINFO_SERIALNO_LEN_MAX );
}


#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// TWL_HWINFO_H_
