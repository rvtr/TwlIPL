/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     WDS.c

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

//**********************************************************************
/**
//	@file		WDS.c
//	@brief		店頭親機がMP送信する接続先AP情報を受信
//
//	@author		M.Okuno
//	@date		2008/02/05
//	@version	01.00
//
***********************************************************************/
#include <sysmenu/WDS.h>
#include <nitro/crypto/rc4.h>

//-----------------------------------------------------
//	Macros
//-----------------------------------------------------
/**
	@brief	スキャンバッファサイズ
*/
#define	WDS_SCAN_BUF_SIZE		1024

/**
	@brief	親機AP情報GGID
*/
#define	WDS_APINFO_GGID			0x00000857

/**
	@brief	ステータス
*/
enum 
{
	WDS_STATUS_INIT ,			///< 初期化中
	WDS_STATUS_STARTSCAN ,		///< スキャン開始
	WDS_STATUS_ENDSCAN ,		///< スキャン終了
	WDS_STATUS_END				///< 終了
};

//-----------------------------------------------------
//	Structs
//-----------------------------------------------------
/**
	@brief	ワーク領域
*/
typedef struct WDSWork
{
	u8				wmwork[ WM_SYSTEM_BUF_SIZE ];	///< WMライブラリ用バッファ
	
	u8				scanbuf[ WDS_SCAN_BUF_SIZE ] ATTRIBUTE_ALIGN(32);	///< スキャンバッファ
	WMScanExParam	scanparam;						///< スキャンパラメータ
	WDSCallbackFunc	scancb;							///< スキャン用コールバック
	WDSApInfo		apinfo[ WDS_APINFO_MAX ];		///< スキャン出来たAP情報
	u16				linklevel[ WDS_APINFO_MAX ];	///< 電波強度
	u16				tgid[ WDS_APINFO_MAX ];			///< 複数のビーコンを受け取った際に識別用に使うTGID
	s32				apnum;							///< スキャン出来たAP情報数
	u32				status;							///< ステータス
	
	MATHCRC32Context	crcContext;
	MATHCRC32Table		crcTable;
} WDSWork;

//-----------------------------------------------------
//	Variables
//-----------------------------------------------------
static WDSWork	*gWdsWork	= NULL;

//--------------------------------------------------------------------------------
/**	UTF8 → UCS2 へ変換します
		@param	<1> UTF8 文字
		@param	<2> バイト判定
		@return	UCS2 文字
	@note
		browse_unicode.cpp から移植。
*///------------------------------------------------------------------------------
static u16	bu_UTF8_To_UCS2( const u8 *c, s32 *bytes_used )
{
	u16		ret;
	u8		b1, b2, b3;
	
	if( (*c & 0x80) == 0 ) {
		// 1バイト、ASCII
		ret			= *c;
		*bytes_used	= 1;
	}
	else if( (*c & 0xe0) == 0xc0 ) {
		// 2バイト、欧州文字とか？
		b1	= (u8)(c[0] & 0x1f);
		b2	= (u8)(c[1] & 0x3f);
		ret	= (u16)(( b1 << 6 ) | b2 );
		*bytes_used	= 2;
	}
	else if( (*c & 0xf0) == 0xe0 ) {
		// 3バイト、日本語はここかな
		b1	= (u8)(c[0] & 0x0f);
		b2	= (u8)(c[1] & 0x3f);
		b3	= (u8)(c[2] & 0x3f);
		ret	= (u16)(( b1 << 12 ) | ( b2 << 6 ) | b3 );
		*bytes_used	= 3;
	}
	else {
		// 想定外の値の場合、ASCIIのハテナを返しとく
		ret			= 0x3F;
		*bytes_used	= 1;
	}
	return ret;
}

//--------------------------------------------------------------------------------
/**	WM_StartScanEx() 用コールバック
		@param	<1> パラメータ
*///------------------------------------------------------------------------------
static void	WDSScanCallback( void *arg )
{
	WMStartScanExCallback	*pParam	= (WMStartScanExCallback*)arg;
	u32						i, j;
	
	// スキャンの結果得られたデータはARM7からDMA経由でメインメモリに直接書き込まれる
	// そのためARM9のデータキャッシュをいったん無効化しなければデータを正常に読めないことに注意
	DC_InvalidateRange(gWdsWork->scanbuf, WDS_SCAN_BUF_SIZE);
	
	// 親機を発見したか判定
	if( pParam->errcode == WM_ERRCODE_SUCCESS && pParam->state == WM_STATECODE_PARENT_FOUND ) {
		// 発見した親機の中に店頭用親機があるか検索
		for( i = 0 ; i < pParam->bssDescCount ; i++ ) {
			// GGID 判定
			if( WM_IsValidGameInfo( &pParam->bssDesc[i]->gameInfo, sizeof(WMGameInfo) ) && pParam->bssDesc[i]->gameInfo.ggid == WDS_APINFO_GGID ) {
				// AP 情報発見
				CRYPTORC4FastContext	rc4context;
				u8						rc4key[ 8 ];
				BOOL					duplicated;
				const u32				magic	= 'WDS!';
				
				// ビーコンの重複チェック
				duplicated = FALSE;
				for( j = 0 ; j < gWdsWork->apnum ; j++ )
				{
					if( gWdsWork->tgid[j] == pParam->bssDesc[i]->gameInfo.tgid )
					{
						duplicated = TRUE;
						break;
					}
				}
				if( duplicated == TRUE )
					continue;
				
				// デバッグ表示
				OS_TPrintf( "Found AP GGID : %08x TGID : %04x\n", pParam->bssDesc[i]->gameInfo.ggid, pParam->bssDesc[i]->gameInfo.tgid );
				
				// ビーコン内容をコピーし、暗号化解除
				MI_CpuCopy8( pParam->bssDesc[i]->gameInfo.userGameInfo, &gWdsWork->apinfo[gWdsWork->apnum], sizeof(WDSApInfo) );
				MI_CpuCopy8( &magic, &rc4key[0], 4 );
				MI_CpuCopy8( &pParam->bssDesc[i]->bssid[2], &rc4key[4], 4 );
				CRYPTO_RC4FastInit( &rc4context, rc4key, 8 );
				CRYPTO_RC4FastEncrypt( &rc4context, &gWdsWork->apinfo[gWdsWork->apnum], sizeof(WDSApInfo), &gWdsWork->apinfo[gWdsWork->apnum] );
				
				// その他のデータを設定
				gWdsWork->linklevel[gWdsWork->apnum]	= pParam->linkLevel[i];
				gWdsWork->tgid[gWdsWork->apnum]			= pParam->bssDesc[i]->gameInfo.tgid;
				gWdsWork->apnum = ( gWdsWork->apnum + 1 ) % WDS_APINFO_MAX;
			}
		}
	}
	
	// スキャン終了
	gWdsWork->status	= WDS_STATUS_ENDSCAN;
	
	// コールバック実行
	if( gWdsWork->scancb )
		gWdsWork->scancb( arg );
}

//--------------------------------------------------------------------------------
/**	WDSライブラリが使用するワークエリアのサイズを得ます
		@return 0 以外 : ワークエリアのサイズ
	@note
		ライブラリ初期化時に使用します
*///------------------------------------------------------------------------------
size_t WDS_GetWorkAreaSize( void )
{
	return sizeof( WDSWork );
}

//--------------------------------------------------------------------------------
/**	WDSを初期化します
		@param	<1> 呼び出し元によって確保されたバッファへのポインタを指定します
		@param	<2> 処理が完了した際に呼び出されるコールバック関数へのポインタ
		@param	<3> WM_InitializeのdmaNoに準ずる引数
		@return 0      : 成功 ( コールバックを待つこと )
				0 以外 : 失敗
	@note
		<1> のサイズはWDS_SYSTEM_BUF_SIZEだけ必要であり、
		かつ 32バイトアラインされている必要があります。
		<2> のコールバックは割り込みハンドラ内から呼び出されます。
		WDS 関連のコールバック関数内では呼び出せません。
		関数内部でWM_Initialize()を実行する為、WMはREADYステートからIDLEステートに移行します。
*///------------------------------------------------------------------------------
int	WDS_Initialize( void *wdsWork, WDSCallbackFunc callback, u16 dmaNo )
{
	WMErrCode	errcode;
	
	SDK_ASSERT( wdsWork );
	
	if( callback == NULL )
		return -1;
	
	// ワーク用バッファ設定
	gWdsWork	= (WDSWork*)wdsWork;
	
	// メモリ初期化
	MI_CpuClear8( gWdsWork, WDS_GetWorkAreaSize() );
	gWdsWork->status	= WDS_STATUS_INIT;
	
	// WM ライブラリ初期化
	errcode	= WM_Initialize( gWdsWork->wmwork, callback, dmaNo );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}


//--------------------------------------------------------------------------------
/**	WDSを初期化します(指定したスキャン結果を内部的に保持した状態での初期化)
		@param	<1> 呼び出し元によって確保されたバッファへのポインタを指定します
		@param	<2> 処理が完了した際に呼び出されるコールバック関数へのポインタ
		@param	<3> WM_InitializeのdmaNoに準ずる引数
		@param	<4> 事前にWDSを使用して取得したWDSBriefInfoの配列
		@return 0      : 成功 ( コールバックを待つこと )
				0 以外 : 失敗
	@note
		<1> のサイズはWDS_SYSTEM_BUF_SIZEだけ必要であり、
		かつ 32バイトアラインされている必要があります。
		<2> のコールバックは割り込みハンドラ内から呼び出されます。
		WDS 関連のコールバック関数内では呼び出せません。
		関数内部でWM_Initialize()を実行する為、WMはREADYステートからIDLEステートに移行します。
*///------------------------------------------------------------------------------
int	WDS_InitializeEx( void *wdsWork, WDSCallbackFunc callback, u16 dmaNo, WDSBriefApInfo *apinfo )
{
	WMErrCode	errcode;
	int			i;
	
	SDK_ASSERT( wdsWork );
	
	if( callback == NULL )
		return -1;
	
	// ワーク用バッファ設定
	gWdsWork	= (WDSWork*)wdsWork;
	
	// メモリ初期化
	MI_CpuClear8( gWdsWork, WDS_GetWorkAreaSize() );
	gWdsWork->status	= WDS_STATUS_INIT;
	
	// 初期値をコピー
	for( i = 0 ; i < WDS_APINFO_MAX ; i++ )
	{
		if( apinfo[i].isvalid == TRUE )
		{
			gWdsWork->apnum++;
			gWdsWork->apinfo[i]		= apinfo[i].apinfo;
			gWdsWork->linklevel[i]	= apinfo[i].rssi;
		}
	}
	
	// WM ライブラリ初期化
	errcode	= WM_Initialize( gWdsWork->wmwork, callback, dmaNo );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	
	// 初期値を入れた関係上、ステータスをスキャン完了にする
	gWdsWork->status = WDS_STATUS_ENDSCAN;
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	WDSを終了します
		@param	<1> 処理が完了した際に呼び出されるコールバック関数へのポインタ
		@return 0      : 成功 ( コールバックを待つこと )
				0 以外 : 失敗
	@note
		<1> のコールバックは割り込みハンドラ内から呼び出されます。
		WDS 関連のコールバック関数内では呼び出せません。
*///------------------------------------------------------------------------------
int	WDS_End( WDSCallbackFunc callback )
{
	WMErrCode	errcode;
	
	if( callback == NULL )
		return -1;
	
	// WDS 終了
	gWdsWork->status	= WDS_STATUS_END;
	gWdsWork			= NULL;
	
	// WM 終了
	errcode	= WM_End( callback );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}

//--------------------------------------------------------------------------------
/**	DS配信システム親機ビーコンをスキャンを開始します
		@param	<1> 処理が完了した際に呼び出されるコールバック関数へのポインタ
		@return 0      : 成功 ( コールバックを待つこと )
				0 以外 : 失敗
	@note
		<1> のコールバックは割り込みハンドラ内から呼び出されます。
		WDS 関連のコールバック関数内では呼び出せません。
		関数内部でWM_StartScane()を実行する為、WMはIDLEステートの場合はSCANステートに移行します。
*///------------------------------------------------------------------------------
int	WDS_StartScan( WDSCallbackFunc callback )
{
	WMErrCode	errcode;
	
	SDK_ASSERT( gWdsWork );
	
	if( callback == NULL )
		return -1;
	
	// スキャンパラメータ設定
	// スキャンチャンネルは 1, 7, 13
	// 300ms～600ms スキャンを行う
	gWdsWork->scanparam.scanBuf			= (WMBssDesc*)&gWdsWork->scanbuf;
	gWdsWork->scanparam.scanBufSize		= WDS_SCAN_BUF_SIZE;
	gWdsWork->scanparam.channelList		= WM_GetAllowedChannel();
	gWdsWork->scanparam.maxChannelTime	= WM_GetDispersionScanPeriod();
	gWdsWork->scanparam.scanType		= WM_SCANTYPE_PASSIVE;
	MI_CpuFill8( gWdsWork->scanparam.bssid, 0xFF, WDS_MACADDR_BUF_SIZE );
	
	// コールバック保存
	gWdsWork->scancb	= callback;
	
	// スキャン開始
	gWdsWork->status	= WDS_STATUS_STARTSCAN;
	errcode	= WM_StartScanEx( WDSScanCallback, &gWdsWork->scanparam );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}

//--------------------------------------------------------------------------------
/**	DS配信システム親機ビーコンをスキャンを終了します
		@param	<1> 処理が完了した際に呼び出されるコールバック関数へのポインタ
		@return 0      : 成功 ( コールバックを待つこと )
				0 以外 : 失敗
	@note
		<1> のコールバックは割り込みハンドラ内から呼び出されます。
		WDS 関連のコールバック関数内では呼び出せません。
		関数内部でWM_EndScan()を実行する為、WMはIDLEステートに移行します。
*///------------------------------------------------------------------------------
int	WDS_EndScan( WDSCallbackFunc callback )
{
	WMErrCode	errcode;
	
	SDK_ASSERT( gWdsWork );
	
	if( callback == NULL )
		return -1;
	
	// スキャン終了
	gWdsWork->status	= WDS_STATUS_ENDSCAN;
	errcode	= WM_EndScan( callback );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}

//--------------------------------------------------------------------------------
/**	APビーコン情報の数を取得します
		@return 0 以上 : 成功
				0 未満 : 失敗 ( スキャン中 )
	@note
		スキャン中に実行した場合はエラー値を返します。
*///------------------------------------------------------------------------------
int	WDS_GetApInfoNum( void )
{
	SDK_ASSERT( gWdsWork );
	
	// スキャン中判定
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	return gWdsWork->apnum;
}

//--------------------------------------------------------------------------------
/**	APビーコン情報をインデックスを指定して取得します
		@param	<1> 取得するAPビーコン情報のインデックス値(0～15)
		@param	<2> 取得するAPビーコン情報を書き込む領域へのポインタ
		@return 0      : 成功
				0 以外 : 失敗 ( スキャン中、又はインデックスの示す位置にデータがない )
	@note
		スキャン中に実行した場合はエラー値を返します。
*///------------------------------------------------------------------------------
int	WDS_GetApInfoByIndex( int index, WDSBriefApInfo *briefapinfo )
{
	SDK_ASSERT( gWdsWork );
	
	// スキャン中判定
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// インデックスエラー判定
	if( index < 0 || index >= gWdsWork->apnum )
		return -1;
	
	// メモリ初期化
	MI_CpuClear8( briefapinfo, sizeof(WDSBriefApInfo) );
	
	// その他設定
	briefapinfo->isvalid		= TRUE;
	briefapinfo->rssi			= gWdsWork->linklevel[index];
	MI_CpuCopy8(&gWdsWork->apinfo[index], &briefapinfo->apinfo, sizeof(WDSApInfo));
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	APビーコン情報を全て取得します
		@param	<1> 取得するAPビーコン情報を書き込む配列の先頭へのポインタ(WDS_APINFO_MAX個の要素が必要)
		@return 0      : 成功
				0 以外 : 失敗 ( スキャン中、又はインデックスの示す位置にデータがない )
	@note
		スキャン中に実行した場合はエラー値を返します。
*///------------------------------------------------------------------------------
int	WDS_GetApInfoAll( WDSBriefApInfo *briefapinfo )
{
	int index;
	
	SDK_ASSERT( gWdsWork );
	
	// スキャン中判定
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// メモリ初期化
	MI_CpuClear8( briefapinfo, sizeof(WDSBriefApInfo)*WDS_APINFO_MAX );
	for( index = 0 ; index < WDS_APINFO_MAX ; index++ )
	{
		briefapinfo[index].isvalid = FALSE;
	}
	
	// 指定インデックスのAPビーコン情報をコピー
	// ホットスポット名を設定 ( UTF-8 の場合は UTF-16 に変換 )
	// 全てのAPビーコン情報をコピー
	for( index = 0 ; index < gWdsWork->apnum ; index++ ) {
		if( WDS_GetApInfoByIndex( index, briefapinfo+index ) == -1 )
			break;
	}
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	APビーコン情報をDWCの自動接続先として設定します
		@param	<1> 自動接続先として設定するAPビーコン情報のインデックス値(0～15)
		@return 0      : 成功
				0 以外 : 失敗 ( スキャン中、又はインデックスの示す位置にデータがない )
	@note
		スキャン中に実行した場合はエラー値を返します。
*///------------------------------------------------------------------------------
int	WDS_SetConnectTargetByIndex( int index )
{
	SDK_ASSERT( gWdsWork );
	
	// スキャン中判定
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// インデックスエラー判定
	if( index < 0 || index >= gWdsWork->apnum )
		return -1;
	
	// 指定 SSID へ接続
	//DWC_AC_SetSpecifyAp2( gWdsWork->apinfo[index].ssid, (u16)((int)gWdsWork->apinfo[index].channel-1), gWdsWork->apinfo[index].wepkey, gWdsWork->apinfo[index].encryptflag );
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	APビーコン情報をDWCの自動接続先として設定します
		@param	<1> 自動接続先として設定するAPビーコン情報
		@return 0      : 成功
				0 以外 : 失敗 ( スキャン中、又はインデックスの示す位置にデータがない )
	@note
		スキャン中に実行した場合はエラー値を返します。
*///------------------------------------------------------------------------------
int	WDS_SetConnectTargetByBriefApInfo( WDSBriefApInfo *briefapinfo )
{
#pragma unused(briefapinfo)
	
	SDK_ASSERT( gWdsWork );
	
	// スキャン中判定
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// 指定 SSID へ接続
	//DWC_AC_SetSpecifyAp2( briefapinfo->apinfo.ssid, (u16)((int)briefapinfo->apinfo.channel-1), briefapinfo->apinfo.wepkey, briefapinfo->apinfo.encryptflag );
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	APビーコン情報のAP説明文をUTF-16で得ます
		@param	<1> AP説明文を取得する対象のAPビーコン情報
				<2> AP説明文を書き込むバッファへのポインタ(WDS_HOTSPOTNAME_UTF16_BUF_SIZEバイト)
		@return 0      : 成功
				0 以外 : 失敗 ( スキャン中、又はインデックスの示す位置にデータがない )
	@note
		スキャン中に実行した場合はエラー値を返します。
*///------------------------------------------------------------------------------
int	WDS_GetApDescriptionUTF16( WDSBriefApInfo *briefapinfo, void *outbuf )
{
	SDK_ASSERT( briefapinfo->isvalid );
	
	// バッファをクリア
	MI_CpuClear8( outbuf, WDS_HOTSPOTNAME_UTF16_BUF_SIZE );
	
	// 指定インデックスのAPビーコン情報をコピー
	// ホットスポット名を設定 ( UTF-8 の場合は UTF-16 に変換 )
	if( (briefapinfo->apinfo.hotspotid & WDS_HOTSPOT_ENCODE_MASK) == WDS_HOTSPOT_ENCODE_UTF8 ) {
		// UTF-8
		u8		*pStr		= briefapinfo->apinfo.hotspotname;
		u8		*pEndStr	= briefapinfo->apinfo.hotspotname + WDS_HOTSPOTNAME_BUF_SIZE;
		u16		*pUcs2		= (u16*)outbuf;
		s32		temp;
		
		// UTF-16に変換
		while( pStr != pEndStr && *pStr != '\0' ) {
			*pUcs2	= bu_UTF8_To_UCS2( pStr, &temp );
			pStr	+= temp;
			pUcs2	+= 1;
		}
	}
	else {
		// UTF-16
		MI_CpuCopy8( briefapinfo->apinfo.hotspotname, outbuf, WDS_HOTSPOTNAME_BUF_SIZE );
	}
	
	return 0;
}
