/*---------------------------------------------------------------------------*
  Project:  TwlIPL - AES - include
  File:     aes_init.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_AES_INIT_H_
#define TWL_AES_AES_INIT_H_

#include <twl/os/common/format_rom.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    ŠÖ”’è‹`
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         AESi_InitKeysForApp

  Description:  set IDs depending on the application.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    game_code   game code

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitKeysForApp( u8 game_code[4] );

/*---------------------------------------------------------------------------*
  Name:         AESi_InitKeysForHard

  Description:  set IDs depending on the system hardware.
                you SHOULD NOT use this for standard applications.

  Arguments:    fuse    camouflaged fuse id

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitKeysForHard( u8 fuse[8] );

/*---------------------------------------------------------------------------*
  Name:         AESi_ResetAesKey

  Description:  set SEED/ID/KEYs filler data without slot-D

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_ResetAesKey( void )
{
    AES_Lock();
    AES_WaitKey();

    MI_CpuCopy32( (u32*)AESi_ResetAesKey,    (u32*)REG_AES_KEY_A0_ADDR+1, 40 );
    MI_CpuCopy32( (u32*)AESi_ResetAesKey+10, (u32*)REG_AES_KEY_B0_ADDR+1, 40 );
    MI_CpuCopy32( (u32*)AESi_ResetAesKey+20, (u32*)REG_AES_KEY_C0_ADDR+1, 40 );

    AES_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_InitKeysFIRM

  Description:  set IDs depending on the application.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    game_code   game code

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_InitKeysFIRM( void )
{
    AESi_InitKeysForApp( (u8*)((ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF)->game_code );
//    AESi_ResetAesKey();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_InitSeedWithRomHeader

  Description:  set SEED/KEY from ROM header

  Arguments:    rom_header      ROM header

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitSeedWithRomHeader( ROM_Header* rom_header );

/*---------------------------------------------------------------------------*
  Name:         AESi_InitSeedWithRomHeader

  Description:  set SEED/KEY from ROM header in HW_TWL_ROM_HEADER_BUF

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_InitSeed( void )
{
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_AES_AES_INIT_H_ */
#endif
