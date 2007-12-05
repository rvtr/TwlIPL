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


#define HW_SECURE_INFO_WRITE_ENABLE_					// HWセキュア情報のライト許可コンパイルスイッチ

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

#define TWL_HWINFO_NORMAL_VERSION			1			// HW情報フォーマットバージョン(開始No.:1)
#define TWL_HWINFO_SECURE_VERSION			1			// HW情報フォーマットバージョン(開始No.:1)
#define TWL_HWINFO_SERIALNO_LEN_AMERICA		11			// 本体シリアルNo.長Max（北米向けは11桁）
#define TWL_HWINFO_SERIALNO_LEN_OTHERS		12			// 本体シリアルNo.長Max
#define TWL_HWINFO_SERIALNO_LEN_MAX			16			// 本体シリアルNo.長Max
#define TWL_HWINFO_CAMERA_LEN				1024		// カメラ情報 [TODO]サイズ未定


// リージョンコード（販社別になる見込み）
typedef enum TWLRegionCode {
	TWL_REGION_JAPAN     = 0,   // NCL
	TWL_REGION_AMERICA   = 1,   // NOA
	TWL_REGION_EUROPE    = 2,   // NOE
	TWL_REGION_AUSTRALIA = 3,   // NAL
	TWL_REGION_CHINA     = 4,   // IQue
	TWL_REGION_KOREA     = 5,   // NOK
	TWL_REGION_MAX
}TWLRegion;


// 本体シリアルNo.
typedef struct TWLSerialNo {
	u8				length;								// シリアルNo.長
	u8				no[ TWL_HWINFO_SERIALNO_LEN_MAX ];	// シリアルNo.（ASCII文字列）
}TWLSerialNo;


// TWL_HWセキュア情報設定データ（署名で改ざん保護する必要があるもの）
// ※基本、過去ver互換を考慮して、追加しかしない方針で。
typedef struct TWLHWSecureInfo{
	u8				region;								// リージョン
	TWLSerialNo		serialNo;							// 本体シリアルNo.
}TWLHWSecureInfo;	// 18bytes


// TWL_HWノーマル情報設定データ（署名で改ざん保護する必要がないもの）
// ※基本、過去ver互換を考慮して、追加しかしない方針で。
typedef struct TWLHWNormalInfo{
	u8				rtcAdjust;							// RTC調整値
	u8				camera[ TWL_HWINFO_CAMERA_LEN ];	// カメラ情報
}TWLHWNormalInfo;	// 1025byte


#ifdef SDK_ARM9


//=========================================================
// HW情報リードライト関数
//=========================================================
// Normal情報
	// 内部変数へのリードfs
extern TSFReadResult THW_ReadNormalInfo( void );
	// 内部変数の値のライト
extern BOOL          THW_WriteNormalInfo( void );
	// 直接値を指定してのライト（開発用）
extern BOOL          THW_WriteNormalInfoDirect( const TWLHWNormalInfo *pSrcInfo );
	// ファイルリカバリー
extern BOOL          THW_RecoveryNormalInfo( TSFReadResult err );
	// 上記Read,Write関数で使用されるデフォルト値のセット（開発用）
extern void          TWH_SetNormalDefaultValue( const TWLHWNormalInfo *pSrcInfo );

// Secure情報
	// リード
extern TSFReadResult THW_ReadSecureInfo( void );
	// 下記コンパイルスイッチ定義時のみ有効
#ifdef HW_SECURE_INFO_WRITE_ENABLE_
	// ライト
extern BOOL          THW_WriteSecureInfo( const u8 *pPrivKeyDER );
	// 直接値を指定してのライト（開発用）
extern BOOL          THW_WriteSecureInfoDirect( const TWLHWSecureInfo *pSrcInfo, const u8 *pPrivKeyDER );
	// ファイルリカバリー
extern BOOL          THW_RecoverySecureInfo( TSFReadResult err );
#endif // HW_SECURE_INFO_WRITE_ENABLE_
	// 上記Read,Write関数で使用されるデフォルト値のセット（開発用）
extern void          TWH_SetSecureDefaultValue( const TWLHWSecureInfo *pSrcInfo );


//=========================================================
// （下記アクセス関数が使用するstatic変数）
//=========================================================
extern TWLHWNormalInfo   s_hwInfoN;
extern TWLHWSecureInfo   s_hwInfoS;
#define GetHWN()		( &s_hwInfoN )
#define GetHWS()		( &s_hwInfoS )


//=========================================================
// データ取得（THW_ReadNormalInfo, THW_ReadSecureInfoで内部ワークに読み出した情報の取得）
//=========================================================

// RTCオフセット値の取得
static inline u8 THW_GetRTCAdjust( void )
{
	return	GetHWN()->rtcAdjust;
}


// カメラ情報の取得
static inline void THW_GetCameraInfo( u8 *pDst )
{
	MI_CpuCopy8( GetHWN()->camera, pDst, TWL_HWINFO_CAMERA_LEN );
}


// リージョンの取得。
static inline u8 THW_GetRegion( void )
{
	return	(u8)GetHWS()->region;
}


// 本体シリアルNo.の取得
static inline void THW_GetSerialNo( TWLSerialNo *pDst )
{
	MI_CpuCopy8( &GetHWS()->serialNo, pDst, sizeof(TWLSerialNo) );
}


//=========================================================
// データセット（TSD_ReadSettingsで内部ワークに読み出した情報への値セット）
//=========================================================

// RTCオフセット値のセット
static inline void THW_SetRTCAdjust( u8 adjust )
{
	GetHWN()->rtcAdjust = adjust;
}


// カメラ情報のセット
static inline void THW_SetCameraInfo( u8 *pCamera )
{
	MI_CpuCopy8( pCamera, GetHWN()->camera, TWL_HWINFO_CAMERA_LEN );
}


// リージョンのセット。
static inline void THW_SetRegion( u8 region )
{
	GetHWS()->region = region;
}

// 本体シリアルNo.のセット
static inline void THW_SetSerialNo( TWLSerialNo *pSrc )
{
	MI_CpuCopy8( pSrc, &GetHWS()->serialNo, sizeof(TWLSerialNo) );
}


#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// TWL_HWINFO_H_
