/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __DSCARD_TYPE2_H__
#define __DSCARD_TYPE2_H__

#ifdef __cplusplus
extern "C" {
#endif


#include 	<twl.h>
#include 	"Card.h"

// ===========================================================================
// 	Function Describe
// ===========================================================================

// ■       ノーマルモードのコマンド       ■
//	DSカードType2のノーマルモードのBoot Segment(4Kbyte)読み込み
void ReadBootSegNormal_DSType2(CardBootData *cbd);

// ■       セキュアモードのコマンド       ■
//	DSカードType2のセキュアモードのID読み込み
void ReadIDSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのSecure Segment(16Kbyte)読み込み
void ReadSegSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのPNジェネレータON
void SwitchONPNGSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのPNジェネレータOFF
void SwitchOFFPNGSecure_DSType2(CardBootData *cbd);

//	DSカードType2のセキュアモードのモード変更
void ChangeModeSecure_DSType2(CardBootData *cbd);

// ■       ゲームモードのコマンド       ■
// Type1と同じコマンドを使う

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __DSCARD_TYPE2_H__
