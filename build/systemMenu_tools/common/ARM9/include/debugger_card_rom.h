/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     debugger_card_rom.h

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
#ifndef DEBUGGER_HW_CARD_ROM_H_
#define DEBUGGER_HW_CARD_ROM_H_


#ifdef  __cplusplus
extern "C" {
#endif

#define DEBUGGER_COMMAND_LOOK_SCREEN   0x00000001
#define DEBUGGER_COMMAND_ALREADY       0x00000002
#define DEBUGGER_COMMAND_NOW_UPDATE    0x00000003
#define DEBUGGER_COMMAND_CANCELED      0x00000004
#define DEBUGGER_COMMAND_FINISHED      0x00000005

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomIDCoreEx

  Description:  カード ID の読み出し。

  Arguments:    dontCare : gRD_IDの don't care ビットにセットする値

  Returns:      カード ID
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomIDCoreEx(u32 dontCare);

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* DEBUGGER_HW_CARD_ROM_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
