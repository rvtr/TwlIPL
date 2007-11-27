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

#include <twl/aes/ARM7/lo.h>

/*---------------------------------------------------------------------------*
  Name:         AESi_InitKeysForApp

  Description:  set IDs depending on the application.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    game_code   game code

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitKeysForApp( u8 game_code[4] )
{
    AESi_WaitKey();

    reg_AES_AES_ID_B2 = AES_IDS_ID0_C(game_code);
    reg_AES_AES_ID_B3 = AES_IDS_ID0_D(game_code);

    reg_AES_AES_ID_C0 = AES_IDS_ID1_A(game_code);
    reg_AES_AES_ID_C1 = AES_IDS_ID1_B(game_code);

    // set dummy without seed[3]
    reg_AES_AES_SEED_A0 = 1;
    reg_AES_AES_SEED_A1 = 2;
    reg_AES_AES_SEED_A2 = 3;
    reg_AES_AES_SEED_B0 = 4;
    reg_AES_AES_SEED_B1 = 5;
    reg_AES_AES_SEED_B2 = 6;
    reg_AES_AES_SEED_C0 = 7;
    reg_AES_AES_SEED_C1 = 8;
    reg_AES_AES_SEED_C2 = 9;
    reg_AES_AES_SEED_D0 = 10;
    reg_AES_AES_SEED_D1 = 11;
    reg_AES_AES_SEED_D2 = 12;

    reg_AES_AES_KEY_A0 = 1;
    reg_AES_AES_KEY_A1 = 2;
    reg_AES_AES_KEY_A2 = 3;
    reg_AES_AES_KEY_A3 = 3;
    reg_AES_AES_KEY_B0 = 4;
    reg_AES_AES_KEY_B1 = 5;
    reg_AES_AES_KEY_B2 = 6;
    reg_AES_AES_KEY_B3 = 6;
    reg_AES_AES_KEY_C0 = 7;
    reg_AES_AES_KEY_C1 = 8;
    reg_AES_AES_KEY_C2 = 9;
    reg_AES_AES_KEY_C3 = 9;
    reg_AES_AES_KEY_D0 = 10;
    reg_AES_AES_KEY_D1 = 11;
    reg_AES_AES_KEY_D2 = 12;
    reg_AES_AES_KEY_D3 = 12;
}

/*---------------------------------------------------------------------------*
  Name:         AESi_RecvSeed

  Description:  set SEED/KEY from ARM9 via PXI.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_RecvSeed( void )
{
    AESKey seed;
    PXI_RecvDataByFifo( PXI_FIFO_TAG_DATA, &seed, AES_BLOCK_SIZE );
    AESi_WaitKey();
    AESi_SetKeySeedA((AESKeySeed*)&seed);    // APP
    //AESi_WaitKey();
    //AESi_SetKeySeedB((AESKeySeed*)&seed);    // APP & HARD
    //AESi_WaitKey();
    //AESi_SetKeySeedC((AESKeySeed*)&seed);    //
    //AESi_WaitKey();
    //AESi_SetKeySeedD((AESKeySeed*)&seed);    // HARD
    AESi_WaitKey();
    AESi_SetKeyC(&seed);        // Direct
}
