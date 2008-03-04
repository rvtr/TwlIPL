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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_ROM_EMULATION_H__
