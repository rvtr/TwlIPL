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
//	DSカードのノーマルモードのID読み込み
HotSwState ReadIDNormal(CardBootData *cbd);

//	DSカードType1のノーマルモードのモード変更
HotSwState ChangeModeNormal(CardBootData *cbd);

// カード側のKeyTableをロード
HotSwState LoadTable(void);

// デバッガのROMエミュレーション情報読み込み
HotSwState ReadRomEmulationData(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	DSカードのゲームモードのID読み込み
HotSwState ReadIDGame(CardBootData *cbd);

//  DSカードのゲームモードの指定ページ読み込み
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_DSCARD_COMMON_H__
