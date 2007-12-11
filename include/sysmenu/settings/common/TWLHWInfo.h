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


#define HW_SIGNATURE_ENABLE_					// HWセキュア情報の署名処理有効コンパイルスイッチ

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

#define TWL_HWINFO_NORMAL_VERSION			1			// HW情報フォーマットバージョン(開始No.:1)
#define TWL_HWINFO_SECURE_VERSION			1			// HW情報フォーマットバージョン(開始No.:1)
#define TWL_HWINFO_SERIALNO_LEN_AMERICA		11			// 本体シリアルNo.長Max（北米向けは11桁）
#define TWL_HWINFO_SERIALNO_LEN_OTHERS		12			// 本体シリアルNo.長Max
#define TWL_HWINFO_SERIALNO_LEN_MAX			15			// 本体シリアルNo.長Max(終端付きなので、14bytesまで拡張可)
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

#define TWL_LANG_BITMAP_JAPAN		( ( 0x0001 << TWL_LANG_JAPANESE ) ) 	// JPN版での対応言語ビットマップ
#define TWL_LANG_BITMAP_AMERICA		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// AME版での対応言語ビットマップ
#define TWL_LANG_BITMAP_EUROPE		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_GERMAN  ) | \
									  ( 0x0001 << TWL_LANG_ITALIAN  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// EUR版での対応言語ビットマップ
#define TWL_LANG_BITMAP_AUSTRALIA	( ( 0x0001 << TWL_LANG_ENGLISH  ) ) 	// AUS版での対応言語ビットマップ
#define TWL_LANG_BITMAP_CHINA		( ( 0x0001 << TWL_LANG_SIMP_CHINESE ) ) // CHI版での対応言語ビットマップ
#define TWL_LANG_BITMAP_KOREA		( ( 0x0001 << TWL_LANG_KOREAN ) ) 		// KOR版での対応言語ビットマップ


// TWL_HWノーマル情報設定データ（署名で改ざん保護する必要がないもの）
// ※基本、過去ver互換を考慮して、追加しかしない方針で。
typedef struct TWLHWNormalInfo{
	u8				rtcAdjust;							// RTC調整値
	u8				camera[ TWL_HWINFO_CAMERA_LEN ];	// カメラ情報
}TWLHWNormalInfo;	// 1025byte


// TWL_HWセキュア情報設定データ（署名で改ざん保護する必要があるもの）
// ※基本、過去ver互換を考慮して、追加しかしない方針で。
// ※SystemShared領域にリードしているので、サイズが変わった時は注意する。
typedef struct TWLHWSecureInfo{
	u32				validLanguageBitmap;						// 本体で有効な言語コードをビット列で表現
	u8				region;										// リージョン
	u8				serialNo[ TWL_HWINFO_SERIALNO_LEN_MAX ];	// シリアルNo.（終端付きASCII文字列）
}TWLHWSecureInfo;	// 20bytes


#ifdef SDK_ARM9


//=========================================================
// HW情報リードライト関数
//=========================================================
// Normal情報
	// 内部変数へのリード
extern TSFReadResult THW_ReadNormalInfo( void );
	// 内部変数の値をライト（先にリードしておく必要がある）
extern BOOL          THW_WriteNormalInfo( void );
	// 直接値を指定してのライト（開発用）
extern BOOL          THW_WriteNormalInfoDirect( const TWLHWNormalInfo *pSrcInfo );
	// ファイルリカバリー
extern BOOL          THW_RecoveryNormalInfo( TSFReadResult err );
	// 上記Read,Write関数で使用されるデフォルト値のセット（開発用）
extern void          THW_SetDefaultNormalInfo( const TWLHWNormalInfo *pSrcInfo );

extern void          THW_ClearNormalInfoDirect( TWLHWNormalInfo *pDstInfo );
extern const TWLHWNormalInfo *THW_GetDefaultNormalInfo( void );
extern const TWLHWNormalInfo *THW_GetNormalInfo( void );

// Secure情報
	// 内部変数へのリード
extern TSFReadResult THW_ReadSecureInfo( void );
extern TSFReadResult THW_ReadSecureInfo_NoCheck( void );	// 署名ノーチェックでリード
	// 内部変数の値をライト（先にリードしておく必要がある）
extern BOOL          THW_WriteSecureInfo( const u8 *pPrivKeyDER );	// pPrivKeyDERがNULLなら署名なしでライト
	// 直接値を指定してのライト（開発用）
extern BOOL          THW_WriteSecureInfoDirect( const TWLHWSecureInfo *pSrcInfo, const u8 *pPrivKeyDER );	// pPrivKeyDERがNULLなら署名なしでライト
	// ファイルリカバリー
extern BOOL          THW_RecoverySecureInfo( TSFReadResult err );
	// 上記Read,Write関数で使用されるデフォルト値のセット（開発用）
extern void          THW_SetDefaultSecureInfo( const TWLHWSecureInfo *pSrcInfo );

extern void          THW_ClearSecureInfoDirect( TWLHWSecureInfo *pDstInfo );
extern const TWLHWSecureInfo *THW_GetDefaultSecureInfo( void );
extern const TWLHWSecureInfo *THW_GetSecureInfo( void );

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


// カメラ情報へのポインタの取得
static inline const u8 *THW_GetCameraInfoPtr( void )
{
	return (const u8 *)GetHWN()->camera;
}


// 言語ビットマップ値の取得
static inline u32 THW_GetValidLanguageBitmap( void )
{
	return	GetHWS()->validLanguageBitmap;
}


// リージョンの取得。
static inline u8 THW_GetRegion( void )
{
	return	(u8)GetHWS()->region;
}


// 本体シリアルNo.の取得
static inline void THW_GetSerialNo( u8 *pDst )
{
	MI_CpuCopy8( &GetHWS()->serialNo, pDst, TWL_HWINFO_SERIALNO_LEN_MAX );
}


// 本体シリアルNo.へのポインタの取得
static inline const u8 *THW_GetSerialNoPtr( void )
{
	return (const u8 *)&GetHWS()->serialNo;
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
static inline void THW_SetCameraInfo( const u8 *pCamera )
{
	MI_CpuCopy8( pCamera, GetHWN()->camera, TWL_HWINFO_CAMERA_LEN );
}


// 言語ビットマップ値のセット
static inline void THW_SetValidLanguageBitmap( u32 langBitmap )
{
	GetHWS()->validLanguageBitmap = langBitmap;
}


// リージョンのセット。
static inline void THW_SetRegion( u8 region )
{
	GetHWS()->region = region;
}

// 本体シリアルNo.のセット
static inline void THW_SetSerialNo( const u8 *pSrc )
{
	MI_CpuCopy8( pSrc, &GetHWS()->serialNo, TWL_HWINFO_SERIALNO_LEN_MAX );
}


#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// TWL_HWINFO_H_
