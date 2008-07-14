/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ds_volume.c

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

#include <twl.h>
#include <twl/cdc.h>
#include <sysmenu.h>
#include <sysmenu/ds.h>

/*---------------------------------------------------------------------------*
    型宣言
 *---------------------------------------------------------------------------*/

typedef struct _DsSpecialSpeakerVolumeTitle
{
	u8 gamecode[6];  // NULL終端兼アライメント用に2バイト大きいサイズを与える
	u8 rom_version;
	u8 volume;
} DsSpecialSpeakerVolumeTitle;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/

static DsSpecialSpeakerVolumeTitle sList[] ATTRIBUTE_ALIGN(32) =
{
	{ "AKEJ", 0, 0x40 },
	{ "ABXJ", 0, 0x40 },
	{ "ALHJ", 0, 0x40 },
	{ "YFSJ", 0, 0x40 },
	{ "AY8E", 0, 0x40 },
	{ "YONJ", 0, 0x40 },
	{ "ACCE", 0, 0x40 },
	{ "AN9E", 0, 0x40 },
	{ "A5HE", 0, 0x40 },
	{ "A5IE", 0, 0x40 },
	{ "AMHE", 0, 0x40 },
	{ "A3TX", 0, 0x40 },
	{ "YCQE", 0, 0x40 },
	{ "YBOE", 0, 0x40 },
	{ "ADAE", 5, 0x40 },
	{ "APAE", 5, 0x40 },
	{ "ACZY", 0, 0x40 },
	{ "APYJ", 0, 0x40 },
	{ "AY7P", 0, 0x40 },
	{ "AWHP", 0, 0x40 },
	{ "AWHE", 0, 0x40 },
	{ "AOIJ", 0, 0x40 },
	{ "AOIJ", 1, 0x40 },
	{ "YO9J", 0, 0x40 },
	{ "YYKJ", 0, 0x40 }
};

/*---------------------------------------------------------------------------*
    グローバル関数定義
 *---------------------------------------------------------------------------*/

void DS_CheckSpeakerVolume( void* romHeaderNTR )
{
	ROM_Header* dh = romHeaderNTR;
	int i;
	int limit = sizeof(sList)/sizeof(sList[0]);
	for (i=0;i<limit;i++)
	{
		if (*(u32 *)sList[i].gamecode == *(u32 *)dh->s.game_code && 
            sList[i].rom_version == dh->s.rom_version)
		{
		    CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_L_ADDR, (u8)(CDC1_ANGVOL_E | sList[i].volume) );
		    CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_R_ADDR, (u8)(CDC1_ANGVOL_E | sList[i].volume) );
			break;
		}
	}
}
