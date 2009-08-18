 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest
  File:     common.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-08-30#$
  $Rev: 8166 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/
#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <twl.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define KEY_REPEAT_START    25  // キーリピート開始までのフレーム数
#define KEY_REPEAT_SPAN     10  // キーリピートの間隔フレーム数

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// キー入力情報
typedef struct KeyInfo
{
    u16 cnt;    // 未加工入力値
    u16 trg;    // 押しトリガ入力
    u16 up;     // 離しトリガ入力
    u16 rep;    // 押し維持リピート入力
} KeyInfo;


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

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
void InitCommon(void);

void ReadKey(KeyInfo* pKey);

void VBlankIntr(void);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* COMMON_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
