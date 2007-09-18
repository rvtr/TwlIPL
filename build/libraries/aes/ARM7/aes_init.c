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

/*---------------------------------------------------------------------------*
  Name:         AESi_InitGameKeys

  Description:  set IDs depending on the application.
                you SHOULD NOT touch any ID registers after this call.

  Arguments:    u8[4]   game code

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_InitGameKeys( u8 game_code[4] )
{
    while (reg_AES_AES_CNT & REG_AES_AES_CNT_E_MASK)
    {
    }

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
