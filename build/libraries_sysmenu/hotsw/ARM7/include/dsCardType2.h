/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_DSCARD_TYPE2_H__
#define __HOTSW_DSCARD_TYPE2_H__

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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_DSCARD_TYPE2_H__
