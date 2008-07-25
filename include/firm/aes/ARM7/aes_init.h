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
  Name:         AESi_PreInitKeys

  Description:  reset IDs preset by bootrom.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_PreInitKeys( void );

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
  Name:         AESi_ResetAesKeyA

  Description:  set SEED/ID/KEYs filler data for slot-C

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_ResetAesKeyA( void )
{
    AES_Lock();
    AES_WaitKey();

    MI_CpuCopy32( (u32*)AESi_ResetAesKeyA + 0, (u32*)REG_AES_KEY_A1_ADDR, 40 );

    AES_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_ResetAesKeyB

  Description:  set SEED/ID/KEYs filler data for slot-C

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_ResetAesKeyB( void )
{
    AES_Lock();
    AES_WaitKey();

    MI_CpuCopy32( (u32*)AESi_ResetAesKeyB + 1, (u32*)REG_AES_KEY_B1_ADDR, 40 );

    AES_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_ResetAesKeyB

  Description:  set SEED/ID/KEYs filler data for slot-C

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AESi_ResetAesKeyC( void )
{
    AES_Lock();
    AES_WaitKey();

    MI_CpuCopy32( (u32*)AESi_ResetAesKeyC + 2, (u32*)REG_AES_KEY_C1_ADDR, 40 );

    AES_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_ResetAesKeyD

  Description:  set SEED/ID/KEYs filler data for slot-D

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#if 0   // rebootƒ‰ƒCƒuƒ‰ƒŠ‚ÉˆÚ“®.
static inline void AESi_ResetAesKeyD( void )
{
    AES_Lock();
    AES_WaitKey();

    MI_CpuCopy32( (u32*)AESi_ResetAesKeyD + 3, (u32*)REG_AES_KEY_D1_ADDR, 40 );

    AES_Unlock();
}
#endif
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
//    AESi_ResetAesKeyC();
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
    AESi_InitSeedWithRomHeader( (ROM_Header*)HW_TWL_ROM_HEADER_BUF );
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_AES_AES_INIT_H_ */
#endif
