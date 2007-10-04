/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - aes
  File:     aes_util.c

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
  Name:         AESi_AddCounter

  Description:  calculate updated counter

  Arguments:    pCounter    pointer to the counter at offset 0
                nums        offset in blocks (AES_BLOCK_SIZE)

  Returns:      None
 *---------------------------------------------------------------------------*/
void AESi_AddCounter(AESCounter* pCounter, u32 nums)
{
    u32 data = 0;
    int i;
    for (i = 0; i < 16; i++)
    {
        data += pCounter->bytes[i] + (nums & 0xFF);
        pCounter->bytes[i] = (u8)(data & 0xFF);
        data >>= 8;
        nums >>= 8;
        if ( !data && !nums )
        {
            break;
        }
    }
}

