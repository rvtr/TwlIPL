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
void ReadBootSegNormal_DSType1(CardBootData *cbd);

//	DSカードType1のノーマルモードのモード変更
void ChangeModeNormal_DSType1(CardBootData *cbd);


// ■       セキュアモードのコマンド       ■
//	DSカードType1のセキュアモードのID読み込み
void ReadIDSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのSecure Segment(16Kbyte)読み込み
void ReadSegSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのPNジェネレータON
void SwitchONPNGSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのPNジェネレータOFF
void SwitchOFFPNGSecure_DSType1(CardBootData *cbd);

//	DSカードType1のセキュアモードのモード変更
void ChangeModeSecure_DSType1(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	DSカードType1のゲームモードのID読み込み
void ReadIDGame_DSType1(CardBootData *cbd);

//  DSカードType1のゲームモードの指定ページ読み込み
void ReadPageGame_DSType1(u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE1_H__
