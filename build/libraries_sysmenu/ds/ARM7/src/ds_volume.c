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
    �萔��`
 *---------------------------------------------------------------------------*/

#define NUM_OF_MIC_PROBLEM_TITLE   128

/*---------------------------------------------------------------------------*
    �^�錾
 *---------------------------------------------------------------------------*/

typedef struct _DsMicProblemTitle
{
	u8 gamecode[4];  // NULL�I�[���A���C�����g�p��1�o�C�g�傫���T�C�Y��^����

} DsMicProblemTitle;

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
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
    �O���[�o���֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DS_SetSpeakerVolume

  Description:  DS�^�C�g���̃X�s�[�J���ʂ�ݒ肵�܂��B
                �}�C�N���͊֌W�Ŗ��̔�������^�C�g���͉��ʂ�NTR���x����
                �}���܂��B

  Arguments:    romHeaderNTR: DS�^�C�g����ROM�w�b�_

  Returns:      None
 *---------------------------------------------------------------------------*/
void DS_SetSpeakerVolume( void* romHeaderNTR )
{
	ROM_Header* dh   = romHeaderNTR;
	u32 arg_gamecode = *(u32 *)dh->s.game_code & 0x00ffffff;
	BOOL hit = FALSE;
	int i;

	// ���ׂ��Ȃ�ׂ����ɂ��邽�߃��X�g�ƈ�v�����ꍇ�ł��S���X�g���`�F�b�N����
	for (i=0;i<NUM_OF_MIC_PROBLEM_TITLE;i++)
	{
		// ���[�W�����Ɋ֌W�Ȃ���r����
		u32 list_gamecode = *(u32 *)sList[i].gamecode & 0x00ffffff;
		if ( list_gamecode == arg_gamecode)
		{
			hit = TRUE;
		}
	}

	if (hit)
	{
		// NTR����SP����(�}�C�N���͖��̂���DS�^�C�g���j
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_L_ADDR, (u8)(CDC1_ANGVOL_E | DS_ANGVOL_GAIN_SP_SPECIAL) );
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_R_ADDR, (u8)(CDC1_ANGVOL_E | DS_ANGVOL_GAIN_SP_SPECIAL) );
	}
	else
	{
		// NTR�������傫��SP����
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_L_ADDR, (u8)(CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX_SP) );
		CDC_WriteSpiRegisterEx( 1, REG_CDC1_SP_ANGVOL_R_ADDR, (u8)(CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX_SP) );
	}
}

