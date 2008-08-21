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

#include <firm/os.h>
#include <firm/aes.h>
#include <firm/pxi.h>

// ïœçXâ¬
#define AES_IDS_ID0_C(c)    (((unsigned long)c[0] << 0) | ((unsigned long)c[1] << 8) | ((unsigned long)c[2] << 16) | ((unsigned long)c[3] << 24))
#define AES_IDS_ID0_D(c)    (((unsigned long)c[3] << 0) | ((unsigned long)c[2] << 8) | ((unsigned long)c[1] << 16) | ((unsigned long)c[0] << 24))


/*---------------------------------------------------------------------------*
  Name:         AESi_PreInitKeys

  Description:  reset IDs preset by bootrom.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_PreInitKeys( void )
{
    AES_WaitKey();

    reg_AES_AES_ID_B2 = reg_OS_CHIP_ID1 ^ *(const u32*)&OSi_GetFromFirmAddr()->aes_key[2][0];
    reg_AES_AES_ID_D1 = reg_OS_CHIP_ID0 ^ *(const u32*)&OSi_GetFromFirmAddr()->aes_key[2][8];
    reg_AES_AES_ID_D2 = reg_OS_CHIP_ID1 ^ *(const u32*)&OSi_GetFromFirmAddr()->aes_key[2][4];
}

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
    if ( rom_header->s.developer_encrypt_old || rom_header->s.exFlags.developer_encrypt )
    {
        AES_SetKeyA( (AESKey*)rom_header->s.title_name );
    }
    else
    {
        AES_SetKeySeedA( (AESKeySeed*)rom_header->s.main_ltd_static_digest );
    }
    AES_Unlock();
}
