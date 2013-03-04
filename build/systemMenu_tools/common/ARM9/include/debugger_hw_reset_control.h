/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     debugger_hw_reset_control.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef DEBUGGER_HW_RESET_CONTROL_H_
#define DEBUGGER_HW_RESET_CONTROL_H_


#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetDisable

  Description:  IS-TWL-DEBUGGERでのハードウェアリセットを禁止します。
                この機能はデバッガディゼーブルフラグを指定したSRL 
				でのみ有効です。内部動作としては、5秒毎にカードアクセスを
				行うスレッドを生成起動しています。IS-TWL-DEBUGGERは
				カードアクセスを監視していて10秒間カードアクセスがない
				場合にハードウェアリセットを許可する仕組みになっています。

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetDisable( void );

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetEnable

  Description:  IS-TWL-DEBUGGERでのハードウェアリセットを許可します。
                この機能はデバッガディゼーブルフラグを指定したSRL 
				でのみ有効です。実際にハードウェアリセットが可能になるには
				最大で10秒かかります。

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetEnable( void );

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* DEBUGGER_HW_RESET_CONTROL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
