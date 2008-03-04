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
// ■       ノーマルモードのコマンド       ■
//	ノーマルモードのID読み込み
HotSwState ReadIDNormal(CardBootData *cbd);

//	ノーマルモードのBoot Segment(4Kbyte)読み込み
HotSwState ReadBootSegNormal(CardBootData *cbd);

//	ノーマルモードからセキュアモードへの変更
HotSwState ChangeModeNormal(CardBootData *cbd);

//	ノーマルモードからセキュア２モードへの変更
HotSwState ChangeModeNormal2(CardBootData *cbd);

// カード側のKeyTableをロード
HotSwState LoadTable(void);

// デバッガのROMエミュレーション情報読み込み
HotSwState ReadRomEmulationData(CardBootData *cbd);


// ■       セキュアモードのコマンド       ■
//	セキュアモードのID読み込み
HotSwState ReadIDSecure(CardBootData *cbd);

//	セキュアモードのSecure Segment(16Kbyte)読み込み
HotSwState ReadSegSecure(CardBootData *cbd);

//	セキュアモードのPNジェネレータON
HotSwState SwitchONPNGSecure(CardBootData *cbd);

//	セキュアモードのPNジェネレータOFF
HotSwState SwitchOFFPNGSecure(CardBootData *cbd);

//	セキュアモードのモード変更
HotSwState ChangeModeSecure(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	ゲームモードのID読み込み
HotSwState ReadIDGame(CardBootData *cbd);

//  ゲームモードの指定ページ読み込み
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size);


// ■       内部関数  				     ■
//	LEコマンドをBEへ変換してレジスタへセット
void HOTSWi_SetCommand(GCDCmd64 *cndLE);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_DSCARD_COMMON_H__
