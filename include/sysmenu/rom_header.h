/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     rom_header.h

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

#ifndef _ROM_HEADER_H_
#define _ROM_HEADER_H_

#include <twl.h>


#ifdef __cplusplus
extern "C" {
#endif

#define NINTENDO_LOGO_LENGTH				0x9c		// Nintendo���S�f�[�^�T�C�Y

//----------------------------------------------------------------------
// �t�@�C���l�[���e�[�u��
//----------------------------------------------------------------------

typedef struct ROM_FNT{
  char		entry_name[FS_FILE_NAME_MAX];	// �t�@�C���� (�I�[ \0 �͏Ȃ�)
} ROM_FNT;

//----------------------------------------------------------------------
// �t�@�C���A���P�[�V�����e�[�u��
//----------------------------------------------------------------------
typedef struct {
  void			*top;						// �t�@�C���̐擪 ROM �A�h���X 
  void			*bottom;					// �t�@�C���̍ŏI ROM �A�h���X
} ROM_FAT;

//----------------------------------------------------------------------
// �I�[�o�[���C�e�[�u��
//----------------------------------------------------------------------
typedef struct {
	u32			id;							// �I�[�o�[���C ID 
	void		*ram_address;				// ���[�h�擪�ʒu 
	u32			ram_size;					// ���[�h�T�C�Y 
	u32			bss_size;					// bss �̈�T�C�Y 
	void		*sinit_init;				// static initializer �擪�A�h���X 
	void		*sinit_init_end;			// static initializer �ŏI�A�h���X
	u32			file_id;					// �I�[�o�[���C�t�@�C��ID
	u32			rsv;						// �\��B
} ROM_OVT;

//----------------------------------------------------------------------
// ROM�w�b�_
//----------------------------------------------------------------------
typedef struct {
	//
	// 0x000 System Reserved
	//
	char		title_name[12];				// Soft title name
	u32			game_code;					// Game code
	
	u16			maker_code;					// Maker code
	u8			machine_code;				// Machine code
	u8			rom_type;					// Rom type
	u8			rom_size;					// Rom size
	
	u8			agbRomCycle1st;				// 1�Z�O��AGBRom�A�N�Z�X�T�C�N��1st.
	u8			agbRomCycle2nd;				// 1�Z�O��AGBRom�A�N�Z�X�T�C�N��2nd.
	u8			reserved_A[7];				// System Reserved A ( Set ALL 0 )
	
	u8			soft_version;				// Soft version
	u8			comp_arm9_boot_area:1;		// Compress arm9 boot area
	u8			comp_arm7_boot_area:1;		// Compress arm7 boot area
	u8			inspectCard:1;				// �����J�[�h�t���O
	u8			disableClearMemoryPad:1;	// IPL2�������p�b�h�N���A�E�f�B�Z�[�u���t���O
	u8			enableAgbRomCycle:1;
	u8			:2;
	u8			disableDetectPullOut:1;		// �J�[�h�������o�f�B�Z�[�u���t���O
	
	//
	// 0x020 for Static modules (Section:B)
	//
	//	ARM9
	u32			main_rom_offset;			// ROM offset
	void*		main_entry_address;			// Entry point
	void*		main_ram_address;			// RAM address
	u32			main_size;					// Module size
	
	//	ARM7
	u32			sub_rom_offset;				// ROM offset
	void*		sub_entry_address;			// Entry point
	void*		sub_ram_address;			// RAM address
	u32			sub_size;					// Module size
	
	//
	// 0x040 for File Name Table[FNT] (Section:C)
	//
	ROM_FNT*	fnt_offset;					// ROM offset
	u32			fnt_size;					// Table size
	
	//
	// 0x048 for File Allocation Table[FAT] (Section:E)
	//
	ROM_FAT*	fat_offset;					// ROM offset
	u32			fat_size;					// Table size
	
	//
	// 0x050 for Overlay Tables[OVT] (Section:D)
	//
	//	ARM9
	ROM_OVT*	main_ovt_offset;			// ROM offset
	u32			main_ovt_size;				// Table size
	
	//	ARM7
	ROM_OVT*	sub_ovt_offset;				// ROM offset
	u32			sub_ovt_size;				// Table size
	
	// 0x060 for ROM control parameter
	u32			game_cmd_param;				// Game command parameter
	u32			secure_cmd_param;			// Secure command parameter

    u32			banner_offset;				// Banner ROM offset

	u16			secure_area_crc16;			// Secure area CRC-16
	u16			secure_cmd_latency;			// Secure command latency ((param+2)*256 system cycles)
	u8			ctrl_reserved_B[16];		// Ctrl Reserved B (Set 0)
	
	// 0x080 - 0x0C0 System Reserved
	u32			total_rom_size;
	u8			reserved_B[60];				// System Reserved B (Set 0)
	
	// 0x0C0 for NINTENDO logo data
	u8			nintendo_logo[ NINTENDO_LOGO_LENGTH ];	// NINTENDO logo data
	u16 		nintendo_logo_crc16;		//            CRC-16
	
	// 0x15E ROM header CRC-16
	u16			header_crc16;				// ROM header CRC-16
	
	//
	// 0x0160 - 0x0180 System Reserved
	//
	u32			dbgRomAddr;					// �f�o�b�K���j�^ROM�A�h���X
	s32			dbgRomSize;					// �f�o�b�K���j�^ROM�T�C�Y
	u32			dbgArm9RamAddr;				// �f�o�b�K���j�^ARM9-RAM�A�h���X
	u32			dbgArm7RamAddr;				// �f�o�b�K���j�^ARM7-RAM�A�h���X
	u8			reserved_C[16];				// Debugger Reserved (Set ALL 0)
	
} RomHeader;


//----------------------------------------------------------------------
// ROM�w�b�_�A�h���X�l��
//----------------------------------------------------------------------

#define GetRomHeaderAddr()  ((RomHeader *)HW_ROM_HEADER_BUF)

//�EROM�w�b�_�A�h���X���l�����܂��B


#ifdef __cplusplus

#endif

#endif  // _ROM_HEADER_H_



