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

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
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
  Name:         AESi_RecvSeed

  Description:  set SEED/KEY from ARM9 via PXI.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_RecvSeed( void );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_AES_AES_INIT_H_ */
#endif
