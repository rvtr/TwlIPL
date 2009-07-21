/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_write_nandfirm.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-22#$
  $Rev: 1212 $
  $Author: kamikawa $
 *---------------------------------------------------------------------------*/

#ifndef KAMI_WRITE_NAND_FIRM
#define KAMI_WRITE_NAND_FIRM

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    å^íËã`
 *---------------------------------------------------------------------------*/

typedef void* (*KAMIAlloc)(u32 size);
typedef void  (*KAMIFree)(void* ptr);


// DS-IPL2ÉwÉbÉ_
typedef struct
{
	u16		reserved_0h[4];
	u32		ds_key;
	u16		ds_arm9_romAdr;
	u16		ds_arm9_ramAdr;
	u16		ds_arm7_romAdr;
	u16		ds_arm7_ramAdr;
	u16		ds_arm9_romOffsetUnit:3;
	u16		ds_arm9_ramOffsetUnit:3;
	u16		ds_arm7_romOffsetUnit:3;
	u16		ds_arm7_ramOffsetUnit:3;
	u16		:2;
	u16		ds_header_ver:2;
	u16		ds_data_romAdr;
	u64		card_key;
	u16		ncd_romAdr;
	u16		reserved_24h[2];
	u16		ds_data_crc16;
}
NORHeaderDS;

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/
BOOL NandfirmProcess(void);

static inline void MakeFullPathForSD(char* file_name, char* full_path)
{
	STD_CopyString( full_path, "rom:/data/" );
	STD_ConcatenateString( full_path, file_name );
}

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_WRITE_NAND_FIRM */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
