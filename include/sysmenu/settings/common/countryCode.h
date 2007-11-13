/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     countryCode.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-06#$
  $Rev: 104 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/


#ifndef	COUNTRY_CODE_H_
#define	COUNTRY_CODE_H_
#if		defined(SDK_CW)							// NTRConfigData�Ƀr�b�g�t�B�[���h���g���Ă���̂ŁA�R���p�C���ˑ��ŕs�����������\��������B
												// ����āACW�ȊO�̃R���p�C���̏ꍇ�́A���̃w�b�_�𖳌��ɂ��ăG���[���o������悤�ɂ��čĊm�F����B

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------


// ����ݒ�R�[�h
typedef enum TWLCountryCode{
	TWL_COUNTRY_UNDEFINED	= 0,		// ���ݒ�

	// JPN���[�W����
	TWL_COUNTRY_JAPAN		= 1,		// ���{

	// USA���[�W����
	TWL_COUNTRY_Anguilla	= 8,        // �A���M��
	TWL_COUNTRY_ANTIGUA_AND_BARBUDA,    // �A���e�B�O�A�E�o�[�u�[�_
	TWL_COUNTRY_ARGENTINA   = 10,		// �A���[���`��
	TWL_COUNTRY_ARUBA,                  // �A���o
	TWL_COUNTRY_BAHAMAS,                // �o�n�}
	TWL_COUNTRY_BARBADOS,               // �o���o�h�X
	TWL_COUNTRY_BELIZE,                 // �x���[�Y
	TWL_COUNTRY_BOLIVIA,                // �{���r�A
	TWL_COUNTRY_BRAZIL,                 // �u���W��
	TWL_COUNTRY_BRITISH_VIRGIN_ISLANDS, // �p�̃��@�[�W������
	TWL_COUNTRY_CANADA,                 // �J�i�_
	TWL_COUNTRY_CAYMAN_ISLANDS,         // �P�C�}������
	TWL_COUNTRY_CHILE       = 20,       // �`��
	TWL_COUNTRY_COLOMBIA,               // �R�����r�A
	TWL_COUNTRY_COSTA_RICA,             // �R�X�^���J
	TWL_COUNTRY_DOMINICA,               // �h�~�j�J��
	TWL_COUNTRY_DOMINICAN_REPUBLIC,     // �h�~�j�J���a��
	TWL_COUNTRY_ECUADOR,                // �G�N�A�h��
	TWL_COUNTRY_EL_SALVADOR,            // �G���T���o�h��
	TWL_COUNTRY_FRENCH_GUIANA,          // �t�����X�̃M�A�i
	TWL_COUNTRY_GRENADA,                // �O���i�_
	TWL_COUNTRY_GUADELOUPE,             // �O�A�h���[�v
	TWL_COUNTRY_GUATEMALA   = 30,       // �O�A�e�}��
	TWL_COUNTRY_GUYANA,                 // �K�C�A�i
	TWL_COUNTRY_HAITI,                  // �n�C�`
	TWL_COUNTRY_HONDURAS,               // �z���W�����X
	TWL_COUNTRY_JAMAICA,                // �W���}�C�J
	TWL_COUNTRY_MARTINIQUE,             // �}���e�B�j�[�N
	TWL_COUNTRY_MEXICO,                 // ���L�V�R
	TWL_COUNTRY_MONTSERRAT,             // �����g�Z���g
	TWL_COUNTRY_NETHERLANDS_ANTILLES,   // �I�����_�̃A���e�B��
	TWL_COUNTRY_NICARAGUA,              // �j�J���O�A
	TWL_COUNTRY_PANAMA      = 40,       // �p�i�}
	TWL_COUNTRY_PARAGUAY,               // �p���O�A�C
	TWL_COUNTRY_PERU,                   // �y���[
	TWL_COUNTRY_ST_KITTS_AND_NEVIS,     // �Z���g�L�b�c�E�l�C�r�X
	TWL_COUNTRY_ST_LUCIA,               // �Z���g���V�A
	TWL_COUNTRY_ST_VINCENT_AND_THE_GRENADINES,  // �Z���g�r���Z���g�E�O���i�f�B�[��
	TWL_COUNTRY_SURINAME,               // �X���i��
	TWL_COUNTRY_TRINIDAD_AND_TOBAGO,    // �g���j�_�[�h�E�g�o�S
	TWL_COUNTRY_TURKS_AND_CAICOS_ISLANDS,   // �^�[�N�X�E�J�C�R�X����
	TWL_COUNTRY_UNITED_STATES,          // �A�����J
	TWL_COUNTRY_URUGUAY     = 50,       // �E���O�A�C
	TWL_COUNTRY_US_VIRGIN_ISLANDS,      // �ė̃o�[�W������
	TWL_COUNTRY_VENEZUELA,              // �x�l�Y�G��

    // EUR, NAL ���[�W����
	TWL_COUNTRY_ALBANIA     = 64,       // �A���o�j�A
	TWL_COUNTRY_AUSTRALIA,              // �I�[�X�g�����A
	TWL_COUNTRY_AUSTRIA,                // �I�[�X�g���A
	TWL_COUNTRY_BELGIUM,                // �x���M�[
	TWL_COUNTRY_BOSNIA_AND_HERZEGOVINA, // �{�X�j�A�E�w���c�F�S�r�i
	TWL_COUNTRY_BOTSWANA,               // �{�c���i
	TWL_COUNTRY_BULGARIA    = 70,       // �u���K���A
	TWL_COUNTRY_CROATIA,                // �N���A�`�A
	TWL_COUNTRY_CYPRUS,                 // �L�v���X
	TWL_COUNTRY_CZECH_REPUBLIC,         // �`�F�R
	TWL_COUNTRY_DENMARK,                // �f���}�[�N
	TWL_COUNTRY_ESTONIA,                // �G�X�g�j�A
	TWL_COUNTRY_FINLAND,                // �t�B�������h
	TWL_COUNTRY_FRANCE,                 // �t�����X
	TWL_COUNTRY_GERMANY,                // �h�C�c
	TWL_COUNTRY_GREECE,                 // �M���V��
	TWL_COUNTRY_HUNGARY     = 80,       // �n���K���[
	TWL_COUNTRY_ICELAND,                // �A�C�X�����h
	TWL_COUNTRY_IRELAND,                // �A�C�������h
	TWL_COUNTRY_ITALY,                  // �C�^���A
	TWL_COUNTRY_LATVIA,                 // ���g�r�A
	TWL_COUNTRY_LESOTHO,                // ���\�g
	TWL_COUNTRY_LIECHTENSTEIN,          // ���q�e���V���^�C��
	TWL_COUNTRY_LITHUANIA,              // ���g�A�j�A
	TWL_COUNTRY_LUXEMBOURG,             // ���N�Z���u���N
	TWL_COUNTRY_MACEDONIA,              // �}�P�h�j�A
	TWL_COUNTRY_MALTA       = 90,       // �}���^
	TWL_COUNTRY_MONTENEGRO,             // �����e�l�O��
	TWL_COUNTRY_MOZAMBIQUE,             // ���U���r�[�N
	TWL_COUNTRY_NAMIBIA,                // �i�~�r�A
	TWL_COUNTRY_NETHERLANDS,            // �I�����_
	TWL_COUNTRY_NEW_ZEALAND,            // �j���[�W�[�����h
	TWL_COUNTRY_NORWAY,                 // �m���E�F�[
	TWL_COUNTRY_POLAND,                 // �|�[�����h
	TWL_COUNTRY_PORTUGAL,               // �|���g�K��
	TWL_COUNTRY_ROMANIA,                // ���[�}�j�A
	TWL_COUNTRY_RUSSIA      = 100,      // ���V�A
	TWL_COUNTRY_SERBIA,                 // �Z���r�A
	TWL_COUNTRY_SLOVAKIA,               // �X���o�L�A
	TWL_COUNTRY_SLOVENIA,               // �X���x�j�A
	TWL_COUNTRY_SOUTH_AFRICA,           // ��A�t���J
	TWL_COUNTRY_SPAIN,                  // �X�y�C��
	TWL_COUNTRY_SWAZILAND,              // �X���W�����h
	TWL_COUNTRY_SWEDEN,                 // �X�E�F�[�f��
	TWL_COUNTRY_SWITZERLAND,            // �X�C�X
	TWL_COUNTRY_TURKEY,                 // �g���R
	TWL_COUNTRY_UNITED_KINGDOM = 110,   // �C�M���X
	TWL_COUNTRY_ZAMBIA,                 // �U���r�A
	TWL_COUNTRY_ZIMBABWE,               // �W���o�u�G

    // TWN���[�W����
    TWL_COUNTRY_TAIWAN      = 128,      // ��p
    
    // KOR���[�W����
    TWL_COUNTRY_SOUTH_KOREA = 136,      // �؍�
    
    // HKG���[�W�����iWii�̍����X�g�ɑ��݁j
    TWL_COUNTRY_HONG_KONG   = 144,      // �z���R��
    TWL_COUNTRY_MACAU,                  // �}�J�I
    
    // ASI���[�W�����iWii�̍����X�g�ɑ��݁j
    TWL_COUNTRY_INDONESIA   = 152,      // �C���h�l�V�A
    
    // USA���[�W����
    TWL_COUNTRY_SINGAPORE   = 153,      // �V���K�|�[��
    
    // ASI���[�W�����i�Ăсj
    TWL_COUNTRY_THAILAND    = 154,      // �^�C
    TWL_COUNTRY_PHILIPPINES,            // �t�B���s��
    TWL_COUNTRY_MALAYSIA,               // �}���[�V�A
    
    // ����`���[�W�����iIQue���[�W�����H�j
    TWL_COUNTRY_CHINA       = 160,      // ����
    
    // USA���[�W����
    TWL_COUNTRY_UAE         = 168,      // �A���u�񒷍��A�M
    
    // ����`���[�W����
    TWL_COUNTRY_INDIA       = 169,      // �C���h
    TWL_COUNTRY_EGYPT       = 170,      // �G�W�v�g
    TWL_COUNTRY_OMAN,                   // �I�}�[��
    TWL_COUNTRY_QATAR,                  // �J�^�[��
    TWL_COUNTRY_KUWAIT,                 // �N�E�F�[�g
    TWL_COUNTRY_SAUDI_ARABIA,           // �T�E�W�A���r�A
    TWL_COUNTRY_SYRIA,                  // �V���A
    TWL_COUNTRY_BAHRAIN,                // �o�[���[��
    TWL_COUNTRY_JORDAN,                 // �����_��

    TWL_COUNTRY_OTHERS      = 254,
    TWL_COUNTRY_UNKNOWN     = 255
    
}TWLCountryCode;



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// COUNTRY_CODE_H_
