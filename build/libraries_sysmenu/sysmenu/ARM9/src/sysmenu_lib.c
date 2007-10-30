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
#include "sysmenu_define.h"
#include "sysmenu_card.h"
#include "spi.h"
#include "mb_child.h"

// define data-----------------------------------------------------------------

#define SCREEN_RED						0
#define SCREEN_YELLOW					1

typedef struct BannerCheckParam {
	u8		*srcp;
	u32		size;
}BannerCheckParam;

// extern data-----------------------------------------------------------------
extern void ReturnFromMain( void );
extern void	BootFuncEnd( void );

FS_EXTERN_OVERLAY( ipl2_data );
FS_EXTERN_OVERLAY( bm_mainp );

// function's prototype-------------------------------------------------------
static void SYSMi_WaitInitARM7( void );
static BOOL SYSMi_IsDebuggerBannerViewMode( void );

static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
void SYSM_Finalize( void );
void SYSM_RebootLauncher( void );
void SYSM_RebootTitle( u64 titleID );



static void INTR_SubpIRQ( void );

static void SYSMi_CheckCardLoadAddressAndSize( void );
static void LoadRomRegSizeAdjust( CARDRomRegion *romRegp, u32 load_limit_lo, u32 load_limit_hi );
static void SYSMi_ReadyBootNitroGame( void );
static BOOL SYSMi_CheckARM7LoadNITROCard( void );
static void SYSMi_MainpRegisterAndRamClear( BOOL isPlatformTWL );
static void ClearMemory( int addr1, int addr2 );

static void SYSMi_CopyInfoFromIPL1( void );

static void SYSMi_ReadNTRSetting( void );
static void SYSMi_ReadTWLSetting( void );
static void SYSMi_VerifyNTRSetting( void );
static BOOL SYSMi_CheckEntryAddress( void );
static void SYSMi_WriteAdjustRTC( void );
static BOOL	SYSMi_SendMessageToARM7( u32 msg );
static BOOL SYSMi_CheckNitroCardRightly( void );
static int  SYSMi_ExistCard( void );
static u32  SYSMi_SelectBootType( void );
static void SYSMi_DispInitialDebugData( void );
static void SYSMi_DispDebugData( void );

static void DispSingleColorScreen( int mode );

static void SYSMi_ReadCardBannerFile( void );
static void SYSMi_CheckCardCloneBoot( void );


// global variable-------------------------------------------------------------
#ifdef __SYSM_DEBUG
SharedWork		*swp;												// �f�o�b�K�ł�IPL1SharedWork�̃E�H�b�`�p
SYSM_work		*pSysm;											// �f�o�b�K�ł�SYSM���[�N�̃E�H�b�`�p
NitroConfigData *ncdp;												// �f�o�b�K�ł�NC�f�[�^�@�̃E�H�b�`�p
#endif

// static variable-------------------------------------------------------------
static BOOL			s_isBanner = FALSE;
static BannerFile	s_bannerBuf;

// const data------------------------------------------------------------------

static BannerCheckParam s_bannerCheckList[ BNR_VER_MAX ] = {
	{ (u8 *)&s_bannerBuf.v1, sizeof( BannerFileV1 ) },
	{ (u8 *)&s_bannerBuf.v2, sizeof( BannerFileV2 ) },
	{ (u8 *)&s_bannerBuf.v3, sizeof( BannerFileV3 ) },
};

#ifdef __DEBUG_SECURITY_CODE
static GXRgb security_detection_color[] = { GX_RGB( 31,  0,  0 ),
											GX_RGB( 31, 31,  0 ), };
#endif

// inline functions------------------------------------------------------------

static inline void DBG_SetRed(u32 y_pos)
{
	*(u16 *)(HW_DB_BG_VRAM + 0xf000 + 0x20*2*y_pos) = (1<<12) | 0x100;
	MI_CpuFill16(((u8 *)HW_DB_BG_VRAM + 0x20*0x100), 0x1111, 0x20);
}

// ============================================================================
// function's description
// ============================================================================

// SystemMenu�̏�����
void SYSM_Init( void )
{
#ifdef __SYSM_DEBUG
	pSysm = GetSYSMWork();
	ncdp  = GetNCDWork();
//	SYSMi_DispInitialDebugData();									// �����f�o�b�O���\��
#endif /* __SYSM_DEBUG */
	
	TP_Init();
	RTC_Init();
	
	// WRAM�ݒ�͂���H
//	MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
//	MI_SetWramBank(MI_WRAM_ARM7_ALL);
	
	SVC_CpuClearFast(0x0000, (u16 *)GetSYSMWork(), sizeof(SYSM_work));	// SYSM���[�N�̃N���A
	
	// ��IS�f�o�b�K���ǂ����̔���B�@BootROM����̃p�����[�^���n���H
	SYSMi_WaitInitARM7();
}


// ARM7���̏������҂�
static void SYSMi_WaitInitARM7( void )
{
/*	while( !( SYSM_GetBootFlag() & BFLG_ARM7_INIT_COMPLETED ) ) {
		SVC_WaitByLoop(0x1000);										// ARM7�̏��������I������̂�҂B
	}
*/
	reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;							// PAUSE���W�X�^�̃`�F�b�N�t���O�̃Z�b�g
	
	SYSMi_ReadTWLSetting();											// NAND����TWL�{�̐ݒ�f�[�^�����[�h
	PMm_SetBackLightBrightness();
	
	SYSMi_ReadNTRSetting();											// NOR ����NTR�{�̐ݒ�f�[�^�����[�h
	SYSMi_VerifyNTRSetting();										// NVRAM��NTR�{�̐ݒ�f�[�^�����[�h���A�s��v�ӏ��������NTR�������J�o���B
	
	SYSM_CaribrateTP();												// �ǂݏo����TWL�{�̐ݒ�f�[�^�����Ƃ�TP�L�����u���[�V�����B
	SYSMi_WriteAdjustRTC();											// �ǂݏo����TWL�{�̐ݒ�f�[�^�����Ƃ�RTC�N���b�N�␳�l���Z�b�g�B
	
	SYSMi_CheckCardCloneBoot();										// �J�[�h���N���[���u�[�g���`�F�b�N
	SYSMi_ReadCardBannerFile();										// �J�[�h�o�i�[�t�@�C���̓ǂݏo���B
	
	// ==============================================================
	// �f�o�b�K�Ή��R�[�h
#ifdef __IS_DEBUGGER_BUILD
	if( GetSYSMWork()->isOnDebugger ) {
		if( SYSMi_ExistCard() &&
			!SYSMi_IsDebuggerBannerViewMode() ){					// �f�o�b�K�㓮��̏ꍇ�́A���̒��ŃJ�[�h�u�[�g�܂ł���Ă��܂��B
			SYSM_GetResetParam()->isLogoSkip  = TRUE;
			SYSM_GetResetParam()->bootTitleID = SYSM_GetCardTitleID();
		}
	}else {
		while( 1 ) {}												// IS�f�o�b�K�r���h��IS�f�o�b�K�����o�ł��Ȃ��������~�B
	}
#endif // __IS_DEBUGGER_BUILD
	// ==============================================================
}


int SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
#pragma unused( pTitleList_Card )
	return 0;
}


int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand )
{
#pragma unused( pTitleList_Nand )
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, ���̏������w�肵�ă^�C�g�����X�g���擾����B
															// return : *TitleProperty Array
	return 0;
}


// �w��^�C�g�����u�[�g�\�ȃ|�C���^���`�F�b�N
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}


// �w��^�C�g���̔F�؁����[�h�@���P�t���[������I����B
AuthResult SYSM_LoadAndAuthenticateTitle( TitleProperty *pBootTitle )
{
	// ���C���������̃N���A
	// DS�_�E�����[�h�v���C�̎��́AROM�w�b�_��ޔ�����
	// �A�v�����[�h
	// �A�v���F��
	
	// �p�����[�^�`�F�b�N
	if( !SYSMi_CheckTitlePointer( pBootTitle ) ) {
		return AUTH_RESULT_TITLE_POINTER_ERROR;
	}
	// �G���g���A�h���X�̐��������`�F�b�N
	if( !SYSMi_CheckEntryAddress() ) {
		return AUTH_RESULT_ENTRY_ADDRESS_ERROR;
	}
	
	return AUTH_RESULT_SUCCEEDED;
}


// �u�[�g�̂��߂̏I������
void SYSM_Finalize( void )
{
	// ARM7�ւ̃u�[�g�ʒm
	// ���W�X�^�ERAM�N���A
	
	// ���u�[�g���Ƀv���e�N�V�������j�b�g��OFF�ɂ��Ȃ���΁A�s���ȃA�h���X�ł̋N����h����̂ł́H
	
	u32 i;
	
	// �u�[�g�̑O����
	MI_CpuCopyFast( (void *)ReturnFromMain, (void *)RETURN_FROM_MAIN_ARM9_FUNCP, (u32)( (u32)BootFuncEnd - (u32)ReturnFromMain ) );
	DC_StoreRange ( (void *)ReturnFromMain, 0x200 );		// �Q�[���u�[�g���̍ŏI���������C���������̌��̕��ɃR�s�[�i��SYSM���s���̃X�^�b�N�㏸�Ŕj�󂳂�Ȃ��悤�ɁA���̃^�C�~���O�ŃR�s�[����B�j
	
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {				// DMA�̒�~
		MI_StopDma( i );
	}
	SYSM_FinalizeCardPulledOut();							// �J�[�h�������o�I������
	SYSMi_MainpRegisterAndRamClear( TRUE );					// ���W�X�^��RAM�N���A
	( void )GX_VBlankIntr(FALSE);
	( void )OS_SetIrqFunction(OS_IE_SUBP, INTR_SubpIRQ);
	( void )OS_SetIrqMask(OS_IE_SUBP);						// �T�u�v���Z�b�T���荞�݂݂̂����B
	reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;		// ARM9�X�e�[�g�� "0x0f" ��
	GetSYSMWork()->mainp_state = MAINP_STATE_WAIT_BOOT_REQ;
															// ������FIFO�̓N���A�ς݂Ȃ̂ŁA�g��Ȃ��B
	// ARM7����̒ʒm�҂�
	OS_WaitIrq(1, OS_IE_SUBP);								// SVC_WaitIntr(0,OS_IE_SUBP);����ύX�B
	
	// ���荞�݂��N���A���čŏI�u�[�g�V�[�P���X�ցB
	reg_PXI_SUBPINTF &= 0x0f00;								// �T�u�v���Z�b�T���荞�݋��t���O���N���A
	( void )OS_DisableIrq();
	( void )OS_SetIrqMask(0);
	( void )OS_ResetRequestIrqMask( (u32)~0 );
}


// �����`���[�����u�[�g
void SYSM_RebootLauncher( void )
{
}


// �ċN���^�C�g�����w�肵�Ẵ��u�[�g
void SYSM_RebootTitle( u64 titleID )
{
#pragma unused( titleID )
	
}


#if 0
// NITRO�N����ARM7�ɒʒm
BOOL SYSM_BootCard( void )
{																	// Nintendo���S�`�F�b�N�́A���̃^�C�~���O�ōs���B

	( void )SYSMi_SendMessageToARM7(MSG_BOOT_TYPE_CARD);	// ARM7�ɃJ�[�h�N����ʒm�B

	if( SYSM_CheckNinLogo( (u16 *)GetRomHeaderAddr()->nintendo_logo ) == FALSE
	 || GetSYSMWork()->enableCardNormalOnly == TRUE ) {	// NORMAL�J�[�h��Ή���
		SYSM_SetBootFlag( BFLG_ILLEGAL_NITRO_CARD );
		return FALSE;
	}else {
		SYSM_SetBootFlag( BFLG_BOOT_DECIDED | BFLG_BOOT_NITRO );
		return TRUE;
	}
}
#endif

#if 0
// TP���[�h�\���ǂ����𒲂ׂ�B
BOOL SYSM_IsTPReadable( void )
{
	if( SYSM_GetBootFlag() & BFLG_BOOT_DECIDED )	return FALSE;
	else											return TRUE;
}
#endif


// ARM7-ARM9���L���\�[�X��bootFlag�ւ̒l�̃Z�b�g
void SYSM_SetBootFlag( u32 value )
{
	BOOL preIrq = OS_DisableIrq();
	LockVariable *lockp = &GetSYSMWork()->boot_flag;
	( void )OS_LockByWord(  BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	lockp->value |=  value;
	( void )OS_UnLockByWord(BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	( void )OS_RestoreIrq( preIrq );
}


void SYSM_ClearBootFlag( u32 value )
{
	BOOL preIrq = OS_DisableIrq();
	LockVariable *lockp = &GetSYSMWork()->boot_flag;
	( void )OS_LockByWord(  BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	lockp->value &= ~value;
	( void )OS_UnLockByWord(BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	( void )OS_RestoreIrq( preIrq );
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// �T�u�v���Z�b�T���荞��
static void INTR_SubpIRQ( void )
{
	OS_SetIrqCheckFlag( OS_IE_SUBP );
}


// ============================================================================
// �A�v���N������
// ============================================================================

// �G���g���A�h���X�̐������`�F�b�N
static BOOL SYSMi_CheckEntryAddress( void )
{
	// �G���g���A�h���X��ROM���o�^�G���A��AGB�J�[�g���b�W�G���A�Ȃ�A�������[�v�ɓ���B
	if(   !(   ( (u32)GetRomHeaderAddr()->main_entry_address >= HW_MAIN_MEM              )
			&& ( (u32)GetRomHeaderAddr()->main_entry_address <  SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT ) )
	   || !(    (   ( (u32)GetRomHeaderAddr()->sub_entry_address  >= HW_MAIN_MEM      )
			     && ( (u32)GetRomHeaderAddr()->sub_entry_address  <  SYSM_ARM7_LOAD_MMEM_LAST_ADDR ) )
			 || (   ( (u32)GetRomHeaderAddr()->sub_entry_address  >= HW_WRAM    )
				 && ( (u32)GetRomHeaderAddr()->sub_entry_address  <  SYSM_ARM7_LOAD_WRAM_LAST_ADDR ) ) ) )
	{
		OS_TPrintf("entry address invalid.\n");
#ifdef __DEBUG_SECURITY_CODE
		DispSingleColorScreen( SCREEN_YELLOW );
#endif
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}


// ARM7�ɂ��NITRO�Q�[���̃��[�h�������m�F����B
static BOOL SYSMi_CheckARM7LoadNITROCard( void )
{
	if( SYSMi_ExistCard()
//		&& !( SYSM_GetBootFlag() & BFLG_LOAD_CARD_COMPLETED )
		) {
		return FALSE;
	}
	return TRUE;
}



// SystemMenu�Ŏg�p�������W�X�^���������̃N���A
static void SYSMi_MainpRegisterAndRamClear( BOOL isPlatformTWL )
{
	// �Ōオ�T�u�v���Z�b�T���荞�ݑ҂��Ȃ̂ŁAIME�̓N���A���Ȃ��B
	( void )OS_SetIrqMask(0);
	( void )OS_ResetRequestIrqMask( (u32)~0 );
	
	// �������N���A
	GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);							// VRAM     �N���A
	MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
	( void )GX_DisableBankForLCDC();
//	MI_CpuClearFast((void *)HW_ITCM,		HW_ITCM_SIZE);			// ITCM     �N���A  ��ITCM�ɂ�SDK�̃R�[�h�������Ă���̂ŁAgameBoot.c�ŃN���A����B
//	MI_CpuClearFast((void *)HW_DTCM,		HW_DTCM_SIZE-0x800);	// DTCM     �N���A	��DTCM�̓X�^�b�N&SDK�ϐ�����Ȃ̂ŁA�Ō��gameBoot.c�ŃN���A���Ă���B
	MI_CpuClearFast((void *)HW_OAM,			HW_OAM_SIZE);			// OAM      �N���A
	MI_CpuClearFast((void *)HW_PLTT,		HW_PLTT_SIZE);			// �p���b�g �N���A
	MI_CpuClearFast((void *)HW_DB_OAM,		HW_DB_OAM_SIZE);		// OAM      �N���A
	MI_CpuClearFast((void *)HW_DB_PLTT,		HW_DB_PLTT_SIZE);		// �p���b�g �N���A
	
	// ���W�X�^�N���A
	MI_CpuClearFast((void*)(HW_REG_BASE + 0x8),    0x12c);			// BG0CNT    �` KEYCNT
	MI_CpuClearFast((void*)(HW_REG_BASE + 0x280),  0x40);			// DIVCNT    �` SQRTD3
	MI_CpuClearFast((void*)(HW_REG_BASE + 0x1000), 0x6e);			// DISP1CNT1 �` DISPBRTCNT1
	CP_SetDiv32_32( 0, 1 );
	reg_PXI_SUBP_FIFO_CNT	= 0x4008;
	reg_GX_DISPCNT			= 0;
	reg_GX_DISPSTAT			= 0;									// �� reg_GX_VCOUNT�̓x�^�N���A�ł��Ȃ��̂ŁA���̐擪�����̃N���A�𕪗�����B
	
	
	// NTR�̎��ɂ́A�o�i�[�����鎞�́AMCCNT�̃J�[�h�C�l�[�u���r�b�g��"1"�ŁA�����Ƃ��ɂ�"0"�ɂȂ��Ă������A
	// NTR�N���̎��ɂ́A�����ł�����𓥏P���Ȃ��ƃ_�������B�B�B
	
	// �N���A���Ă��Ȃ����W�X�^�́AVCOUNT, PIFCNT, MC-, EXMEMCNT, IME, RBKCNT1, PAUSE, POWLCDCNT, �S3D�n�ł��B
	if( isPlatformTWL ) {
		// TWL��p���W�X�^�̃N���A
	}
}


// ============================================================================
// �T�u���[�`��
// ============================================================================

// ���S�f���X�L�b�v���H
BOOL SYSM_IsLogoDemoSkip( void )
{
	// ���V�X�e���A�v������̃n�[�h���Z�b�g�ɂ�郍�S�f����΂�������ɓ����B
	
	return SYSMi_IsDebuggerBannerViewMode();
}

// IS�f�o�b�K�̃o�i�[�r���[���[�h�N�����ǂ����H
static BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
#ifdef __IS_DEBUGGER_BUILD
	return ( GetSYSMWork()->isOnDebugger &&
			 SYSMi_ExistCard() &&
			 GetRomHeaderAddr()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif	// __IS_DEBUGGER_BUILD
}


// NITRO�ݒ�f�[�^�̓ǂݏo���B
static void SYSMi_ReadNTRSetting( void )
{
	RTCDate date;
	RTCTime	time;
	
	GetSYSMWork()->ncd_invalid = NVRAMm_ReadNitroConfigData( GetNCDWork() );
																	// NVRAM����NITRO�ݒ�f�[�^�����[�h�B
	if( GetSYSMWork()->ncd_invalid ) {								// ���[�h����NITRO�ݒ�f�[�^��������������A0�N���A�������̂��g�p�B
		OS_TPrintf(" NCD destroyed.\n" );
		SVC_CpuClearFast( 0x0000, GetNCDExWork(), sizeof(NitroConfigDataEx) );
		GetNCDExWork()->version					= NITRO_CONFIG_DATA_EX_VERSION;
		GetNCDWork()->option.backLightBrightness= 2;				// �o�׎��Ɠ����l�ɁB
		GetNCDWork()->option.language			= LANG_ENGLISH;		// �v���Z�b�g���K�v�ȃf�[�^�̓v���Z�b�g���Ă���
		GetNCDWork()->option.destroyFlashFlag	= 1;
		GetNCDWork()->owner.birthday.month		= 1;
		GetNCDWork()->owner.birthday.day		= 1;
		GetNCDExWork()->valid_language_bitmap	= VALID_LANG_BITMAP;
	}
	
	// RTC�̃��Z�b�g or ���������l�����o�����ꍇ�͏���N���V�[�P���X�ցB
	( void )RTC_GetDateTime( &date, &time );
	if( !SYSM_CheckRTCDate( &date ) ||
	    !SYSM_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�ł�RTC�̓d�r���Ȃ��̂ŁA���񂱂��ɂЂ��������Đݒ�f�[�^���Е��N���A����Ă��܂��B�����h���X�C�b�`�B
		|| ( GetSYSMWork()->rtcStatus & 0x01 )
#endif
		) {							// RTC�ُ̈�����o������Artc���̓t���O��rtcOffset��0�ɂ���NVRAM�ɏ������݁B
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		GetNCDWork()->option.input_rtc		= 0;
		GetNCDWork()->option.rtcOffset		= 0;
		GetNCDWork()->option.rtcLastSetYear = 0;
		( void )NVRAMm_WriteNitroConfigData( GetNCDWork() );
	}
}


// TWL�ݒ�f�[�^�̓ǂݏo��
static void SYSMi_ReadTWLSetting( void )
{
	
}


// NTR�ݒ��TWL�ݒ���x���t�@�C���āA�s��v������΁ANTR�ݒ���X�V
static void SYSMi_VerifyNTRSetting( void )
{
}


// RTC�̓��t�����������`�F�b�N
BOOL SYSM_CheckRTCDate( RTCDate *datep )
{
	if(	 ( datep->year >= 100 )
	  || ( datep->month < 1 ) || ( datep->month > 12 )
	  || ( datep->day   < 1 ) || ( datep->day   > 31 )
	  || ( datep->week >= RTC_WEEK_MAX ) ) {
		return FALSE;
	}
	return TRUE;
}


// RTC�̎��������������`�F�b�N
BOOL SYSM_CheckRTCTime( RTCTime *timep )
{
	if(  ( timep->hour   > 23 )
	  || ( timep->minute > 59 )
	  || ( timep->second > 59 ) ) {
		return FALSE;
	}
	return TRUE;
}


// �o�i�[�t�@�C���̓ǂݍ��݂̎���
static void SYSMi_ReadCardBannerFile( void )
{
	s32 lockCardID;
	BannerFile *pBanner = &s_bannerBuf;
	
	if( ( !SYSMi_ExistCard() ) || ( *(void** )BANNER_ROM_OFFSET == NULL ) ) {
		s_isBanner = FALSE;
		return;
	}
	
	// ROM�J�[�h����̃o�i�[�f�[�^�̃��[�h
	if ( ( lockCardID = OS_GetLockID() ) > 0 ) {
		( void )OS_LockCard( (u16 )lockCardID );
		DC_FlushRange( pBanner, sizeof(BannerFile) );
		SYSM_ReadCard(*(void** )BANNER_ROM_OFFSET, pBanner, sizeof(BannerFile) );
		( void )OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	// �o�i�[�f�[�^�̐���`�F�b�N
	{
		int i;
		u16 calc_crc = 0xffff;
		u16 *hd_crcp = (u16 *)&pBanner->h.crc16_v1;
		BannerCheckParam *chkp = &s_bannerCheckList[ 0 ];
		
		s_isBanner  = TRUE;
		
		for( i = 0; i < BNR_VER_MAX; i++ ) {
			if( i < pBanner->h.version ) {
			    calc_crc = SVC_GetCRC16( calc_crc, chkp->srcp, chkp->size );
				if( calc_crc != *hd_crcp++ ) {
					s_isBanner =  FALSE;
					break;
				}
			}else {
				MI_CpuClear16( chkp->srcp, chkp->size );
			}
			chkp++;
		}
		if( !s_isBanner ) {
			MI_CpuClear16( &s_bannerBuf, sizeof(BannerFile) );
		}
	}
}


// �N���[���u�[�g����
static void SYSMi_CheckCardCloneBoot( void )
{
	s32	lockCardID;
	u8 	*buffp         = (u8 *)&s_bannerBuf;		// �o�i�[�p�o�b�t�@���e���|�����Ƃ��Ďg�p
	u32 total_rom_size = GetRomHeaderAddr()->total_rom_size ? GetRomHeaderAddr()->total_rom_size : 0x01000000;
	u32 file_offset    = total_rom_size & 0xFFFFFE00;
	
	if( !SYSMi_ExistCard() ) {
		return;
	}
	
	if ( ( lockCardID = OS_GetLockID() ) > 0 ) {
		( void )OS_LockCard( (u16 )lockCardID );
		DC_FlushRange( buffp, BNR_IMAGE_SIZE );
		SYSM_ReadCard( (void *)file_offset, buffp, BNR_IMAGE_SIZE );
		( void )OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	buffp += total_rom_size & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		GetSYSMWork()->clone_boot_mode = CLONE_BOOT_MODE;
	}else {
		GetSYSMWork()->clone_boot_mode = OTHER_BOOT_MODE;
	}
}


// �^�b�`�p�l���L�����u���[�V����
void SYSM_CaribrateTP( void )
{
#ifndef __TP_OFF
	TPCalibrateParam calibrate;
	
	( void )TP_CalcCalibrateParam( &calibrate,							// �^�b�`�p�l��������
			GetNCDWork()->tp.raw_x1, GetNCDWork()->tp.raw_y1, (u16)GetNCDWork()->tp.dx1, (u16)GetNCDWork()->tp.dy1,
			GetNCDWork()->tp.raw_x2, GetNCDWork()->tp.raw_y2, (u16)GetNCDWork()->tp.dx2, (u16)GetNCDWork()->tp.dy2 );
	TP_SetCalibrateParam( &calibrate );
	OS_Printf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			GetNCDWork()->tp.raw_x1, GetNCDWork()->tp.raw_y1, (u16)GetNCDWork()->tp.dx1, (u16)GetNCDWork()->tp.dy1,
			GetNCDWork()->tp.raw_x2, GetNCDWork()->tp.raw_y2, (u16)GetNCDWork()->tp.dx2, (u16)GetNCDWork()->tp.dy2 );
#endif
}


// RTC�N���b�N�␳�l���Z�b�g
static void SYSMi_WriteAdjustRTC( void )
{	
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�p�r���h���͕␳���Ȃ��B
	RTCRawAdjust raw;
	
	raw.adjust = GetNCDWork()->option.rtcClockAdjust;			// ncd_invalid���ɂ�rtcClockAdjust��
																	// 0�N���A����Ă��邽�ߕ␳�@�\�͎g�p����Ȃ�
	( void )RTCi_SetRegAdjust( &raw );
#endif /* __IS_DEBUGGER_BUILD */
}


// FIFO�o�R��ARM7�Ƀ��b�Z�[�W�ʒm�B��PXI_FIFO_TAG_USER_1���g�p�B
static BOOL	SYSMi_SendMessageToARM7(u32 msg)
{
#pragma unused(msg)
	return TRUE;
}


// NTR,TWL�J�[�h���݃`�F�b�N 		�u���^�[���@1�F�J�[�h�F���@0�F�J�[�h�Ȃ��v
static int SYSMi_ExistCard( void )
{
	if( ( GetRomHeaderAddr()->nintendo_logo_crc16 == 0xcf56 ) &&
	    ( GetRomHeaderAddr()->header_crc16 == GetSYSMWork()->cardHeaderCrc16) ) {
		return TRUE;												// NTR,TWL�J�[�h����iNintendo���SCRC�A�J�[�h�w�b�_CRC���������ꍇ�j
																	// ��Nintendo���S�f�[�^�̃`�F�b�N�́A�����̓s����A���S�\�����[�`���N����ɍs���܂��B
	}else {
		return FALSE;												// NTR,TWL�J�[�h�Ȃ�
	}
}


// Nintendo���S�`�F�b�N			�u���^�[���@1:Nintendo���S�F�������@0�F���s�v
BOOL SYSM_CheckNinLogo(u16 *logo_cardp)
{
	u16 *logo_orgp	= (u16 *)SYSROM9_NINLOGO_ADR;					// ARM9�̃V�X�e��ROM�̃��S�f�[�^�ƃJ�[�g���b�W���̂��̂��r
	u16 length		= NINTENDO_LOGO_LENGTH >> 1;
	
	while(length--) {
		if(*logo_orgp++ != *logo_cardp++) return FALSE;
	}
	return TRUE;
}


// �X���[�v���[�h�ւ̑J��
void SYSM_GoSleepMode( void )
{
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�p�r���h���̓X���[�v���Ȃ��B
	PM_GoSleepMode( (PMWakeUpTrigger)( (PAD_DetectFold() ? PM_TRIGGER_COVER_OPEN : 0) | PM_TRIGGER_RTC_ALARM ),
					0,
					0 );
#endif /* __IS_DEBUGGER_BUILD */
}


// �o�b�N���C�g�P�x����
void PMm_SetBackLightBrightness( void )
{
	( void )PMi_WriteRegister( 4, (u16)NCD_GetBackLightBrightness() );
	( void )PM_SetBackLight( PM_LCD_ALL, PM_BACKLIGHT_ON );
}


//======================================================================
//  �f�o�b�O�֐�
//======================================================================

// �����f�[�^�̃f�o�b�O�\��
#ifdef __SYSM_DEBUG
static void SYSMi_DispInitialDebugData( void )
{
	OS_Printf("SYSM version      :20%x\n", SYSMENU_VER);
	if( GetMovedInfoFromIPL1Addr()->isOnDebugger )	OS_Printf("Run On IS-DEBUGGER\n");
	else 											OS_Printf("Run On IS-EMULATOR\n");
	if(GetMovedInfoFromIPL1Addr()->rtcStatus & 0x01)	OS_Printf("RTC reset is detected!\n");
	if(GetMovedInfoFromIPL1Addr()->rtcError)			OS_Printf("RTC error is detected!\n");
#if 0
	OS_Printf("NvDate       :%4d\n",sizeof(NvDate));
	OS_Printf("NvNickname   :%4d\n",sizeof(NvNickname));
	OS_Printf("NvComment    :%4d\n",sizeof(NvComment));
	OS_Printf("NvOwnerInfo  :%4d\n",sizeof(NvOwnerInfo));
	OS_Printf("NvAlarm      :%4d\n",sizeof(NvAlarm));
	OS_Printf("NvTpCalibData:%4d\n",sizeof(NvTpCalibData));
	OS_Printf("NvOption     :%4d\n",sizeof(NvOption));
	OS_Printf("NCD          :%4d\n",sizeof(NitroConfigData));
	OS_Printf("NCDStore     :%4d\n",sizeof(NCDStore));
#endif
#if 0
	{	// ROM_HEADER_BUFF�̓��e�������o��
		int i,j;
		u32 *romhp = (u32 *)GetRomHeaderAddr();
		OS_Printf("ROM Header Buff\n  ");
		for(i = 0; i < 6; i++) {
			for(j = 0; j < 4; j++) OS_Printf("    0x%8x", *romhp++);
			OS_Printf("\n  ");
		}
		OS_Printf("\n");
	}
	{	// ROM_HEADER_BUFF�̓��e�������o��
		int i,j;
		u32 *romhp = (u32 *)MB_CARD_ROM_HEADER_ADDRESS;
		OS_Printf("MB Card ROM Header Buff\n  ");
		for(i = 0; i < 6; i++) {
			for(j = 0; j < 4; j++) OS_Printf("    0x%8x", *romhp++);
			OS_Printf("\n  ");
		}
		OS_Printf("\n");
	}
#endif  /* 0 */
}
#endif /* __SYSM_DEBUG */



#ifdef __DEBUG_SECURITY_CODE
// �Z�L�����e�B��������Ɠ����Ă��邩���m�F����f�o�b�O�R�[�h
static void DispSingleColorScreen( int mode )
{
	( void )OS_DisableIrq();
	GX_LoadBGPltt  ( &security_detection_color[ mode ], 0, sizeof(GXRgb) );
	GXS_LoadBGPltt ( &security_detection_color[ mode ], 0, sizeof(GXRgb) );
	GX_DispOn();
	GXS_DispOn();
	GX_SetGraphicsMode ( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
	GXS_SetGraphicsMode( GX_BGMODE_0 );
	GX_SetMasterBrightness( 0 );
	GXS_SetMasterBrightness( 0 );
    GX_SetVisiblePlane ( GX_PLANEMASK_NONE );
    GXS_SetVisiblePlane( GX_PLANEMASK_NONE );
}
#endif


