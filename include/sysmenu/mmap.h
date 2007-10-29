/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mmap.c

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

#ifndef	__MMAP_H__
#define	__MMAP_H__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------

#define RETURN_FROM_MAIN_ARM9_FUNCP			0x023fee00					// NITRO�Q�[���u�[�g����ARM9�ŏI�����̓���A�h���X
#define RETURN_FROM_MAIN_ARM7_FUNCP			0x0380f600					// NITRO�Q�[���u�[�g����ARM7�ŏI�����̓���A�h���X

#define SYSM_ADDR_TOP						0x02300000					// SYSM���z�u�����擪�A�h���X
#define SYSM_ADDR_BOTTOM					0x023fe000					// SYSM���z�u�����ŏI�A�h���X

#define SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT		( SYSM_ADDR_TOP - 0x80000 )					// 0x02800000

#define SYSM_ARM9_LOAD_MMEM_LAST_ADDR		( SYSM_ADDR_TOP - 0x80000 )					// SYSM�����[�h�\��NITRO�J�[�h�����u�[�g�R�[�h�̃��C���������ŏI�A�h���X
#define SYSM_ARM7_LOAD_MMEM_LAST_ADDR		( SYSM_ADDR_BOTTOM )						// SYSM�����[�h�\��NITRO�J�[�h�����u�[�g�R�[�h�̃��C���������ŏI�A�h���X
#define SYSM_ARM7_LOAD_WRAM_LAST_ADDR		( RETURN_FROM_MAIN_ARM7_FUNCP & ~0x0fff )	// SYSM�����[�h�\��NITRO�J�[�h�����u�[�g�R�[�h�̃��C���������ŏI�A�h���X
#define SYSM_ARM7_LOAD_BUF_ADDR				( SYSM_ADDR_TOP - 0x40000 )					// SYSM��NITRO�J�[�hARM7�����u�[�g�R�[�h�̃��[�h���s���ۂ̃��[�h�o�b�t�@�A�h���X
#define SYSM_ARM7_LOAD_BUF_SIZE				( SYSM_ADDR_TOP - SYSM_ARM7_LOAD_BUF_ADDR )	// SYSM��NITRO�J�[�hARM7�R�[�h���[�h�o�b�t�@�T�C�Y

#define UNCOMP_TEMP_BUF						( SYSM_ARM7_LOAD_BUF_ADDR )					// ���k�W�J�p�f�[�^�ꎞ�i�[�o�b�t�@�A�h���X
#define UNCOMP_TEMP_BUF_SIZE				( SYSM_ARM7_LOAD_BUF_SIZE )					// ���k�W�J�p�f�[�^�ꎞ�i�[�o�b�t�@�T�C�Y

#define NITRO_CARD_SECURE_SIZE				0x4000										// NITRO�J�[�h�̃Z�L���A�̈�T�C�Y(16Kbytes)

#define SYSROM9_NINLOGO_ADR					0xffff0020					// ARM9�V�X�e��ROM���̔C�V�����S�i�[�A�h���X
#define AGB_CARTRIDGE_NIN_LOGO_DATA			(HW_CTRDG_ROM + 4)			// AGB�J�[�g���b�W��Nintendo���S�f�[�^�i�[�A�h���X


#ifdef __cplusplus
}
#endif

#endif		// __MMAP_H__

