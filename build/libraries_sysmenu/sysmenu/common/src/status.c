/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     status.c

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

#include <twl.h>
#include <sysmenu.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------


// �����`���[�p�����[�^�̎擾
const LauncherParamBody *SYSM_GetLauncherParamBody( void )
{
	return (const LauncherParamBody *)&SYSMi_GetWork()->launcherParam.body;
}

/*** �t���O�Z�b�g�A���荞�݋֎~�� arm9 ****/
void SYSM_SetHeaderLoadCompleted( BOOL comp )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isHeaderLoadCompleted = comp;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetLoadFinished( BOOL finish )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isLoadFinished = finish;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetLoadSucceeded( BOOL succeed )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isLoadSucceeded = succeed;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetCardBoot( BOOL card )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isCardBoot = card;
	OS_RestoreInterrupts( mode );
}

// ���S�f���X�L�b�v���ǂ������Z�b�g
void SYSM_SetLogoDemoSkip( BOOL skip )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isLogoSkip = skip;
	OS_RestoreInterrupts( mode );
}

// TSD�L��/�������Z�b�g
void SYSM_SetValidTSD( BOOL valid )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm9.isValidTSD = valid;
	OS_RestoreInterrupts( mode );
}

/*** �t���O�Z�b�g�A���荞�݋֎~�� arm7 ****/
// �K�v�Ȃ���

void SYSM_SetHotStart( BOOL hot )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isHotStart = hot;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetValidLauncherParam( BOOL valid )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isValidLauncherParam = valid;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetResetRTC( BOOL reset )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isResetRTC = reset;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetNANDFatalError( BOOL fatal )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isNANDFatalError = fatal;
	OS_RestoreInterrupts( mode );
}

void SYSM_SetARM9Start( BOOL start )
{
	OSIntrMode mode = OS_DisableInterrupts();
	SYSMi_GetWork()->flags.arm7.isARM9Start = start;
	OS_RestoreInterrupts( mode );
}

/*** �����܂� �t���O�Z�b�g�֐� ***/

// �z�b�g�X�^�[�g���H
BOOL SYSM_IsHotStart( void )
{
	return (BOOL)SYSMi_GetWork()->flags.arm7.isHotStart;
}

// ���S�f���X�L�b�v���H
BOOL SYSM_IsLogoDemoSkip( void )
{
	return (BOOL)SYSMi_GetWork()->flags.arm9.isLogoSkip;
}

// IS�f�o�b�K�̃o�i�[�r���[���[�h�N�����ǂ����H
BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
	return ( SYSM_IsRunOnDebugger() &&
			 SYSM_IsExistCard() &&
			 SYSMi_GetWork()->romEmuInfo.isForceBannerViewMode );
}

// TSD�L���H
BOOL SYSM_IsValidTSD( void )
{
	return (BOOL)SYSMi_GetWork()->flags.arm9.isValidTSD;
}


// �L����TWL/NTR�J�[�h���������Ă��邩�H
BOOL SYSM_IsExistCard( void )
{
	return (BOOL)SYSMi_GetWork()->flags.hotsw.isExistCard;
}


// �����p�J�[�h���������Ă��邩�H
BOOL SYSM_IsInspectCard( void )
{
	return ( SYSM_IsExistCard() && SYSMi_GetWork()->flags.hotsw.isInspectCard );
}


// �L����TWL�J�[�h���������Ă��邩�H
BOOL SYSM_IsTWLCard( void );
BOOL SYSM_IsTWLCard( void )
{
	return ( SYSM_IsExistCard() && ( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) );
}


// �L����NTR�J�[�h���������Ă��邩�H
BOOL SYSM_IsNTRCard( void );
BOOL SYSM_IsNTRCard( void )
{
	return ( SYSM_IsExistCard() && ( SYSM_GetCardRomHeader()->platform_code == PLATFORM_CODE_NTR ) );
}


// �G���g���A�h���X�̐������`�F�b�N
BOOL SYSMi_CheckEntryAddress( void )
{
	// �G���g���A�h���X��MMEM��������WRAM�̃��[�h�\�̈�O�Ȃ�A�s���Ɣ���B
	if( !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= HW_MAIN_MEM ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  HW_TWL_MAIN_MEM_SHARED )
		 ) ||
		!( ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_MAIN_MEM ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  HW_TWL_MAIN_MEM_SHARED ) ) ||
		   ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_WRAM_BASE ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_NTR_ARM7_LOAD_WRAM_END ) )
		 ) ||
	    !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= (u32)SYSM_GetCardRomHeader()->main_ram_address ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  (u32)SYSM_GetCardRomHeader()->main_ram_address + SYSM_GetCardRomHeader()->main_size )
		 ) ||
	    !( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address >= (u32)SYSM_GetCardRomHeader()->sub_ram_address ) &&
		   ( (u32)SYSM_GetCardRomHeader()->sub_entry_address <  (u32)SYSM_GetCardRomHeader()->sub_ram_address + SYSM_GetCardRomHeader()->sub_size )
		 )
	 ) {
		OS_TPrintf("entry address invalid.\n");
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}
