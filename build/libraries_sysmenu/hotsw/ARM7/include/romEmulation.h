/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.h
 *---------------------------------------------------------------------------*/
#ifndef __ROM_EMULATION_H__
#define __ROM_EMULATION_H__

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
void ReadBootSegNormal_ROMEMU(CardBootData *cbd);

//	Rom Emulationのノーマルモードのモード変更
void ChangeModeNormal_ROMEMU(CardBootData *cbd);


// ■       セキュアモードのコマンド       ■
//	Rom EmulationのセキュアモードのID読み込み
void ReadIDSecure_ROMEMU(CardBootData *cbd);

//	Rom EmulationのセキュアモードのSecure Segment(16Kbyte)読み込み
void ReadSegSecure_ROMEMU(CardBootData *cbd);

//	Rom EmulationのセキュアモードのPNジェネレータON
void SwitchONPNGSecure_ROMEMU(CardBootData *cbd);

//	Rom EmulationのセキュアモードのPNジェネレータOFF
void SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd);

//	Rom Emulationのセキュアモードのモード変更
void ChangeModeSecure_ROMEMU(CardBootData *cbd);


// ■       ゲームモードのコマンド       ■
//	DSカードType1のゲームモードのID読み込み
void ReadIDGame_ROMEMU(CardBootData *cbd);

//  DSカードType1のゲームモードの指定ページ読み込み
void ReadPageGame_ROMEMU(CardBootData *cbd, u32 start_addr, void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __ROM_EMULATION_H__
