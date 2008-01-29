/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

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
extern void LCFG_VerifyAndRecoveryNTRSettings( void );

// function's prototype-------------------------------------------------------
static TitleProperty *SYSMi_CheckShortcutBoot( void );
static BOOL SYSMi_IsDebuggerBannerViewMode( void );
static int  SYSMi_IsValidCard( void );
static BOOL SYSMi_CheckEntryAddress( void );
static void SYSMi_CheckCardCloneBoot( void );

// global variable-------------------------------------------------------------
void *(*SYSMi_Alloc)( u32 size  );
void  (*SYSMi_Free )( void *ptr );

#define SYSM_DEBUG_
#ifdef SYSM_DEBUG_
SYSM_work		*pSysm;											// �f�o�b�K�ł�SYSM���[�N�̃E�H�b�`�p
ROM_Header_Short *pRomHeader;
#endif
// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// ============================================================================
//
// ������
//
// ============================================================================

// SystemMenu�̏�����
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef SYSM_DEBUG_
	pSysm = SYSMi_GetWork();
	pRomHeader = (ROM_Header_Short *)0x027fc000;
#endif /* SYSM_DEBUG_ */
	
	// �����`���[�̃}�E���g���Z�b�g
	SYSMi_SetLauncherMountInfo();
	
    // ARM7�R���|�[�l���g�p�v���e�N�V�������j�b�g�̈�ύX
    OS_SetProtectionRegion( 2, SYSM_OWN_ARM7_MMEM_ADDR, 512KB );
	
	SYSM_SetAllocFunc( pAlloc, pFree );
	PXI_SetFifoRecvCallback( SYSMENU_PXI_FIFO_TAG, SYSMi_PXIFifoRecvCallback );
	
	reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;							// PAUSE���W�X�^�̃`�F�b�N�t���O�̃Z�b�g
}


// �A���[�i�Đݒ�
void SYSM_SetArena( void )
{
	// ARM9�p�u�[�g�R�[�h�z�u�̂��߁A�A���[�iHi�ʒu��������
	OS_SetMainArenaHi( (void *)SYSM_OWN_ARM9_MMEM_ADDR_END );
}


// �V�X�e�����j���[���C�u�����p�������A���P�[�^�̐ݒ�
void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
	SYSMi_Alloc = pAlloc;
	SYSMi_Free  = pFree;
}


// ������Alloc
void *SYSM_Alloc( u32 size )
{
	void *p = SYSMi_Alloc( size );
	OS_TPrintf( "SYSM_Alloc : %08x  %xbytes\n", p, size );
	return p;
}


// ������Free
void SYSM_Free( void *ptr )
{
	OS_TPrintf( "SYSM_Free  : %08x\n", ptr );
	SYSMi_Free( ptr );
}


// ============================================================================
//
// ���擾
//
// ============================================================================

// �p�����[�^���[�h
TitleProperty *SYSM_ReadParameters( void )
{
	TitleProperty *pBootTitle = NULL;
	u8 brightness = LCFG_TWL_BACKLIGHT_LEVEL_MAX;
	
	// ARM7�̃��Z�b�g�p�����[�^�擾����������̂�҂�
	while( !SYSMi_GetWork()->flags.common.isARM9Start ) {
		SVC_WaitByLoop( 0x1000 );
	}
#ifdef DEBUG_USED_CARD_SLOT_B_
	// ARM7�̃J�[�h�`�F�b�N������҂�
	while( !SYSMi_GetWork()->flags.common.is1stCardChecked ) {
		SVC_WaitByLoop( 0x1000 );
	}
#endif

	//-----------------------------------------------------
	// ���Z�b�g�p�����[�^�̔���i���Z�b�g�p�����[�^���L�����ǂ����́AARM7�ł���Ă���Ă���j
	//-----------------------------------------------------
	{
		if( SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ||		// ���S�f���X�L�b�v�H
			SYSMi_IsDebuggerBannerViewMode() ) {
			SYSM_SetLogoDemoSkip( TRUE );
		}
		
		if( SYSM_GetLauncherParamBody()->v1.bootTitleID ) {			// �A�v�����ڋN���̎w�肪�������烍�S�f�����΂��Ďw��A�v���N��
			pBootTitle = (TitleProperty *)&SYSM_GetLauncherParamBody()->v1;
		}
	}
	
	//-----------------------------------------------------
	// �ʎY�H���p�V���[�g�J�b�g�L�[ or
	// �����J�[�h�N��
	//-----------------------------------------------------
	if( pBootTitle == NULL ) {
		pBootTitle = SYSMi_CheckShortcutBoot();
	}
	
	//-----------------------------------------------------
	// HW���̃��[�h
	//-----------------------------------------------------
	// �m�[�}����񃊁[�h
	if( !LCFG_ReadHWNormalInfo() ) {
		OS_TPrintf( "HW Normal Info Broken!\n" );
		SYSMi_GetWork()->flags.common.isBrokenHWNormalInfo = TRUE;
	}
	// �Z�L���A��񃊁[�h
	if( !LCFG_ReadHWSecureInfo() ) {
		OS_TPrintf( "HW Secure Info Broken!\n" );
		SYSMi_GetWork()->flags.common.isBrokenHWSecureInfo = TRUE;
		SYSMi_GetWork()->flags.common.isFatalError = TRUE;
	}
	
	//-----------------------------------------------------
	// �{�̐ݒ�f�[�^�̃��[�h
	//-----------------------------------------------------
	if( LCFG_ReadTWLSettings() ) {									// NAND����TWL�{�̐ݒ�f�[�^�����[�h
		SYSM_CaribrateTP();											// �ǂݏo����TWL�{�̐ݒ�f�[�^�����Ƃ�TP�L�����u���[�V�����B
		brightness = (u8)LCFG_TSD_GetBacklightBrightness();
	}
	
	//-----------------------------------------------------
	// �e��f�o�C�X�ݒ�
	//-----------------------------------------------------
	// �o�b�N���C�g�P�x�ݒ�
	SYSM_SetBackLightBrightness( brightness );
	// RTC�␳
	SYSMi_WriteAdjustRTC();
	// RTC�l�̃`�F�b�N
	SYSMi_CheckRTC();
	
	LCFG_VerifyAndRecoveryNTRSettings();							// NTR�ݒ�f�[�^��ǂݏo���āATWL�ݒ�f�[�^�ƃx���t�@�C���A�K�v�Ȃ烊�J�o��
	
	//NAM�̏�����
	NAM_Init( SYSM_Alloc, SYSM_Free );
	
	return pBootTitle;
}


// �V���[�g�J�b�g�N���̃`�F�b�N
static TitleProperty *SYSMi_CheckShortcutBoot( void )
{
	static TitleProperty s_bootTitle;
	
	MI_CpuClear8( &s_bootTitle, sizeof(TitleProperty) );
	
	//-----------------------------------------------------
	// IS�f�o�b�K�N�� or
	// �ʎY�H���p�V���[�g�J�b�g�L�[ or
	// �����J�[�h�N��
	//-----------------------------------------------------
	if( SYSM_IsExistCard() ) {
		if( ( SYSMi_GetWork()->flags.common.isOnDebugger &&		// IS�f�o�b�K���L������JTAG���܂��L���łȂ���
			  !( *(u8 *)( HW_SYS_CONF_BUF + HWi_WSYS09_OFFSET ) & HWi_WSYS09_JTAG_CPUJE_MASK ) ) ||
			SYSM_IsInspectCard() ||
			( ( PAD_Read() & PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ==
			  PAD_PRODUCTION_SHORTCUT_CARD_BOOT )
			) {
			s_bootTitle.flags.isAppRelocate = TRUE;
			s_bootTitle.flags.isAppLoadCompleted = TRUE;
			s_bootTitle.flags.isInitialShortcutSkip = TRUE;			// ����N���V�[�P���X���΂�
			s_bootTitle.flags.isLogoSkip = TRUE;					// ���S�f�����΂�
			s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
			s_bootTitle.flags.isValid = TRUE;
			s_bootTitle.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
			SYSM_SetLogoDemoSkip( s_bootTitle.flags.isLogoSkip );
			return &s_bootTitle;
		}
	}
	
	//-----------------------------------------------------
	// TWL�ݒ�f�[�^�����͎��̏���N���V�[�P���X�N��
	//-----------------------------------------------------
#if 0
#ifdef ENABLE_INITIAL_SETTINGS_
	if( !LCFG_TSD_IsSetTP() ||
		!LCFG_TSD_IsSetLanguage() ||
		!LCFG_TSD_IsSetDateTime() ||
		!LCFG_TSD_IsSetUserColor() ||
		!LCFG_TSD_IsSetNickname() ) {
		s_bootTitle.titleID = TITLE_ID_MACHINE_SETTINGS;
		s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
		s_bootTitle.flags.isValid = TRUE;
		return &s_bootTitle;
	}
#endif // ENABLE_INITIAL_SETTINGS_
#endif
	
	return NULL;													// �u�u�[�g���e����v�Ń��^�[��
}


// �����`���[�p�����[�^�̎擾
const LauncherParamBody *SYSM_GetLauncherParamBody( void )
{
	return (const LauncherParamBody *)&SYSMi_GetWork()->launcherParam.body;
}


// ���S�f���X�L�b�v���ǂ������Z�b�g
void SYSM_SetLogoDemoSkip( BOOL skip )
{
	SYSMi_GetWork()->flags.common.isLogoSkip = skip;
}


// ���S�f���X�L�b�v���H
BOOL SYSM_IsLogoDemoSkip( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isLogoSkip;
}


// IS�f�o�b�K�̃o�i�[�r���[���[�h�N�����ǂ����H
static BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
#ifdef __IS_DEBUGGER_BUILD
	return ( SYSMi_GetWork()->flags.common.isOnDebugger &&
			 SYSMi_IsValidCard() &&
			 SYSM_GetCardRomHeader()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif	// __IS_DEBUGGER_BUILD
}


// TP���[�h�\��Ԃ��H
BOOL SYSM_IsTPReadable( void )
{
	return TRUE;
}


// TSD�L��/�������Z�b�g
void SYSM_SetValidTSD( BOOL valid )
{
	SYSMi_GetWork()->flags.common.isValidTSD = valid;
}


// TSD�L���H
BOOL SYSM_IsValidTSD( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isValidTSD;
}


//======================================================================
//
//  �e��`�F�b�N
//
//======================================================================

// �L����TWL/NTR�J�[�h���������Ă��邩�H
BOOL SYSM_IsExistCard( void )
{
	return (BOOL)SYSMi_GetWork()->flags.common.isExistCard;
}


// �����p�J�[�h���������Ă��邩�H
BOOL SYSM_IsInspectCard( void )
{
	return ( SYSM_IsExistCard() && SYSM_GetCardRomHeader()->inspect_card );
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


// NTR,TWL�J�[�h���݃`�F�b�N 		�u���^�[���@1�F�J�[�h�F���@0�F�J�[�h�Ȃ��v
static int SYSMi_IsValidCard( void )
{
	if( ( SYSM_GetCardRomHeader()->nintendo_logo_crc16 == 0xcf56 ) &&
	    ( SYSM_GetCardRomHeader()->header_crc16 == SYSMi_GetWork()->cardHeaderCrc16 ) ) {
		return TRUE;												// NTR,TWL�J�[�h����iNintendo���SCRC�A�J�[�h�w�b�_CRC���������ꍇ�j
																	// ��Nintendo���S�f�[�^�̃`�F�b�N�́A�����̓s����A���S�\�����[�`���N����ɍs���܂��B
	}else {
		return FALSE;												// NTR,TWL�J�[�h�Ȃ�
	}
}

// �G���g���A�h���X�̐������`�F�b�N
static BOOL SYSMi_CheckEntryAddress( void )
{
	// �G���g���A�h���X��ROM���o�^�G���A��AGB�J�[�g���b�W�G���A�Ȃ�A�������[�v�ɓ���B
	if( !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= HW_MAIN_MEM ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT )
		 ) ||
		!( ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_MAIN_MEM ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_ARM7_LOAD_MMEM_LAST_ADDR ) ) ||
		   ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_WRAM    ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_ARM7_LOAD_WRAM_LAST_ADDR ) )
		 )
	 ) {
		OS_TPrintf("entry address invalid.\n");
#ifdef __DEBUG_SECURITY_CODE
		DispSingleColorScreen( SCREEN_YELLOW );
#endif
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}



// �N���[���u�[�g����
static void SYSMi_CheckCardCloneBoot( void )
{
#if 0
	u8 	*buffp         = (u8 *)&pTempBuffer;
	u32 total_rom_size = SYSM_GetCardRomHeader()->rom_valid_size ? SYSM_GetCardRomHeader()->rom_valid_size : 0x01000000;
	u32 file_offset    = total_rom_size & 0xFFFFFE00;
	
	if( !SYSMi_IsValidCard() ) {
		return;
	}
	
	DC_FlushRange( buffp, BNR_IMAGE_SIZE );
	CARD_ReadRom( 4, (void *)file_offset, buffp, BNR_IMAGE_SIZE );
	
	buffp += total_rom_size & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		SYSMi_GetWork()->flags.common.cloneBootMode = CLONE_BOOT_MODE;
	}else {
		SYSMi_GetWork()->flags.common.cloneBootMode = OTHER_BOOT_MODE;
	}
#endif
}


//======================================================================
//  �f�o�b�O
//======================================================================
