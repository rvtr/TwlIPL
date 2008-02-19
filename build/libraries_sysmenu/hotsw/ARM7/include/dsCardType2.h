/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __DSCARD_TYPE2_H__
#define __DSCARD_TYPE2_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================
// ■       ノーマルモードのコマンド       ■
//	DSカードType2のノーマルモードのBoot Segment(4Kbyte)読み込み
HotSwState ReadBootSegNormal_DSType2(CardBootData *cbd);

//	DSカードType2のノーマルモードのモード変更 (Type1と同じ処理)
#define ChangeModeNormal_DSType2	ChangeModeNormal_DSType1


// ■       セキュアモードのコマンド       ■
//	DSカードType2のセキュアモードのID読み込み
HotSwState ReadIDSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのSecure Segment(16Kbyte)読み込み
HotSwState ReadSegSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのPNジェネレータON
HotSwState SwitchONPNGSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのPNジェネレータOFF
HotSwState SwitchOFFPNGSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのモード変更
HotSwState ChangeModeSecure_DSType2(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	DSカードType2のゲームモードのID読み込み (Type1と同じ処理)
#define ReadIDGame_DSType2			ReadIDGame_DSType1

//  DSカードType2のゲームモードの指定ページ読み込み
HotSwState ReadPageGame_DSType2(CardBootData *cbd, u32 start_addr, void* buf, u32 size);
//#define ReadPageGame_DSType2		ReadPageGame_DSType1

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE2_H__
