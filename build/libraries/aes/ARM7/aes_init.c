/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - aes
  File:     aes_init.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <firm/aes.h>
#include <firm/pxi.h>

/*---------------------------------------------------------------------------*
  Name:         AESi_InitKeysForApp

  Description:  set IDs depending on the application.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    game_code   game code

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitKeysForApp( u8 game_code[4] )
{
    AES_Lock();
    AES_WaitKey();

    reg_AES_AES_ID_A2 = AES_IDS_ID0_C(game_code);
    reg_AES_AES_ID_A3 = AES_IDS_ID0_D(game_code);
    AES_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_InitKeysForHard

  Description:  set IDs depending on the system hardware.
                you SHOULD NOT use this for standard applications.

  Arguments:    fuse    camouflaged fuse id

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitKeysForHard( u8 fuse[8] )
{
    AES_Lock();
    AES_WaitKey();

    reg_AES_AES_ID_B2 = *(u32*)&fuse[4];
    reg_AES_AES_ID_B3 = *(u32*)&fuse[0];
    reg_AES_AES_ID_D0 = *(u32*)&fuse[0];
    reg_AES_AES_ID_D3 = *(u32*)&fuse[4];
    AES_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         AESi_InitSeedWithRomHeader

  Description:  set SEED/KEY from ROM header

  Arguments:    rom_header      ROM header

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitSeedWithRomHeader( ROM_Header* rom_header )
{
    AES_Lock();
    AES_WaitKey();
    if ( !rom_header )
    {
        return;
    }
    if ( rom_header->s.developer_encrypt )
    {
        AES_SetKeyA( (AESKey*)rom_header->s.title_name );
    }
    else
    {
        AES_SetKeySeedA( (AESKeySeed*)rom_header->s.main_ltd_static_digest );
    }
    AES_Unlock();
}
