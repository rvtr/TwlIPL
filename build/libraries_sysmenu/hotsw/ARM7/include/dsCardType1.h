/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __DSCARD_TYPE1_H__
#define __DSCARD_TYPE1_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================

// ■       ノーマルモードのコマンド       ■
//	DSカードType1のノーマルモードのBoot Segment(4Kbyte)読み込み
HotSwState ReadBootSegNormal_DSType1(CardBootData *cbd);

//	DSカードType1のノーマルモードのモード変更
HotSwState ChangeModeNormal_DSType1(CardBootData *cbd);


// ■       セキュアモードのコマンド       ■
//	DSカードType1のセキュアモードのID読み込み
HotSwState ReadIDSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのSecure Segment(16Kbyte)読み込み
HotSwState ReadSegSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのPNジェネレータON
HotSwState SwitchONPNGSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのPNジェネレータOFF
HotSwState SwitchOFFPNGSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのモード変更
HotSwState ChangeModeSecure_DSType1(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	DSカードType1のゲームモードのID読み込み
HotSwState ReadIDGame_DSType1(CardBootData *cbd);

//  DSカードType1のゲームモードの指定ページ読み込み
HotSwState ReadPageGame_DSType1(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE1_H__
