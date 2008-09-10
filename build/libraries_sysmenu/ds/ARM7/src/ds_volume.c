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
    定数定義
 *---------------------------------------------------------------------------*/

#define NUM_OF_MIC_PROBLEM_TITLE   128

/*---------------------------------------------------------------------------*
    型宣言
 *---------------------------------------------------------------------------*/

typedef struct _DsMicProblemTitle
{
	u8 gamecode[4];  // NULL終端兼アライメント用に1バイト大きいサイズを与える

} DsMicProblemTitle;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/

static DsMicProblemTitle sList[NUM_OF_MIC_PROBLEM_TITLE] ATTRIBUTE_ALIGN(32) =
{
	"ABX",
	"YO9",
	"ALH",
    "ACC",
    "YCQ",
    "YYK",
    "AZW",
    "AKA",
    "AN9",
    "AKE",
    "YFS",
    "YG8",
    "AY7",
    "YON",
    "A5H",
    "A5I",
    "AMH",
    "A3T",
    "YBO",
    "ADA",
    "APA",
    "CPU",
    "APY",
    "AWH",
    "AXB",

    "A4U",
    "A8N",
    "ABJ",
    "ABN",
    "ACL",
    "ART",
    "AVT",
    "AWY",
    "AXJ",
    "AYK",
    "YB2",
    "YB3",
    "YCH",
    "YFE",
    "YGD",
    "YKR",
    "YRM",
    "YW2",
    "AJU",
    "ACZ",
    "AHD",
    "ANR",
    "YT3",
    "AVI",
    "AV2",

    "AV3",
    "AV4",
    "AV5",
    "AV6",
    "YNZ",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",

    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",

    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",
    "000",

    "000",
    "000",
    "000"
};

/*---------------------------------------------------------------------------*
    グローバル関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DS_SetSpeakerVolume

  Description:  DSタイトルのスピーカ音量を設定します。
                マイク入力関係で問題の発生するタイトルは音量をNTRレベルに
                抑えます。

  Arguments:    romHeaderNTR: DSタイトルのROMヘッダ

  Returns:      None
 *---------------------------------------------------------------------------*/
void DS_SetSpeakerVolume( void* romHeaderNTR )
{
	ROM_Header* dh   = romHeaderNTR;
	u32 arg_gamecode = *(u32 *)dh->s.game_code & 0x00ffffff;
	BOOL hit = FALSE;
	int i;

	// 負荷をなるべく一定にするためリストと一致した場合でも全リストをチェックする
	for (i=0;i<NUM_OF_MIC_PROBLEM_TITLE;i++)
	{
		// リージョンに関係なく比較する
		u32 list_gamecode = *(u32 *)sList[i].gamecode & 0x00ffffff;
		if ( list_gamecode == arg_gamecode)
		{
			hit = TRUE;
		}
	}

	if (hit)
	{
		// NTR並のSP音量(マイク入力問題のあるDSタイトル）
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_L_ADDR, (u8)(CDC1_ANGVOL_E | DS_ANGVOL_GAIN_SP_SPECIAL) );
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_R_ADDR, (u8)(CDC1_ANGVOL_E | DS_ANGVOL_GAIN_SP_SPECIAL) );
	}
	else
	{
		// NTRよりもやや大きいSP音量
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_L_ADDR, (u8)(CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX_SP) );
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_R_ADDR, (u8)(CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX_SP) );
	}
}

