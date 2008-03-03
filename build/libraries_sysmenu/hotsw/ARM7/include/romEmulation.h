/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.h
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_ROM_EMULATION_H__
#define __HOTSW_ROM_EMULATION_H__

#include 	<twl.h>
#include 	<hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 	Function Describe
// ===========================================================================

// ■       ノーマルモードのコマンド       ■
//	Rom EmulationのノーマルモードのBoot Segment(4Kbyte)読み込み
HotSwState ReadBootSegNormal_ROMEMU(CardBootData *cbd);


// ■       セキュアモードのコマンド       ■
//	Rom EmulationのセキュアモードのID読み込み
HotSwState ReadIDSecure_ROMEMU(CardBootData *cbd);

//	Rom EmulationのセキュアモードのSecure Segment(16Kbyte)読み込み
HotSwState ReadSegSecure_ROMEMU(CardBootData *cbd);

//	Rom EmulationのセキュアモードのPNジェネレータON
HotSwState SwitchONPNGSecure_ROMEMU(CardBootData *cbd);

//	Rom EmulationのセキュアモードのPNジェネレータOFF
HotSwState SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd);

//	Rom Emulationのセキュアモードのモード変更
HotSwState ChangeModeSecure_ROMEMU(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	DSカードType1のゲームモードのID読み込み
HotSwState ReadIDGame_ROMEMU(CardBootData *cbd);

//  DSカードType1のゲームモードの指定ページ読み込み
HotSwState ReadPageGame_ROMEMU(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_ROM_EMULATION_H__
