/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - firm
  File:     firm_common.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef	FIRM_FORMAT_FIRM_COMMON_H_
#define	FIRM_FORMAT_FIRM_COMMON_H_

#include <firm/format/sign.h>
#include <firm/format/wram_regs.h>


#ifdef __cplusplus
extern "C" {
#endif


// 常駐モジュール情報
typedef struct
{
    u32     rom_offset;                /* 転送元 ROM オフセット */
    u32     decomp_size;               /* 展開サイズ */
    void   *ram_address;               /* 転送先 RAM オフセット */
    u32     size;                      /* 転送サイズ */
}
FIRMHeader_ModuleInfo;

// DS-IPL2ヘッダ
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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // FIRM_FORMAT_FIRM_COMMON_H_
