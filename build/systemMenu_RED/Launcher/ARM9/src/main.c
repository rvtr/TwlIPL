/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

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
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void InitAllocator( NNSFndAllocator* pAllocator );
static void InitAllocSystem( void );
static BOOL CheckBootStatus( void );
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------
NNSFndAllocator g_allocator;

// static variable-------------------------------------------------------------
static BannerFile banner;											// �o�i�[�f�[�^

// const data------------------------------------------------------------------

#if 1

typedef struct CardStatus {
	u16		primarySlot;		// PULLOUT, DETECT, VALID, INVALID
	u16		secondarySlot;		// ����B
}CardStatus;


void TwlMain( void )
{
	enum {
		START = 0,
		LOGODEMO = 1,
		LAUNCHER_INIT = 2,
		LAUNCHER = 3,
		AUTHENTICATE = 4,
		BOOT = 5,
		STOP = 6
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty *pTitleList = NULL;
	
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	
	// ���荞�݋���--------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// �V�X�e�����j���[������----------
	SYSM_Init();											// OS_Init�̌�ŃR�[���B
	
	// �V�X�e���̏�����----------------
	InitAllocator( &g_allocator );
	CMN_InitFileSystem( &g_allocator );
	
	// ���Z�b�g�p�����[�^�̎擾--------
	if( SYSM_GetResetParam()->isLogoSkip ) {
		if( SYSM_GetResetParam()->bootTitleID ) {							// �A�v�����ڋN���̎w�肪�������烍�S�f�����΂��Ďw��A�v���N��
			pBootTitle = (TitleProperty *)SYSM_GetResetParam();
			state = AUTHENTICATE;
		}else {																// ����ȊO�̏ꍇ�́A���S�f�����΂��ă����`���[�N��
			state = LAUNCHER_INIT;
		}
	}
	
	// �R���e���g�i���\�[�X�j�t�@�C���̃��[�h
//	FS_ReadContentFile( ContentID );
	
	// ���L�R���e���g�t�@�C���̃��[�h
//	FS_ReadSharedContentFile( ContentID );
	
	// NAND�A�v�����X�g�̎擾----------
	(void)SYSM_GetNandTitleList( pTitleList );
	
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// V�u�����N���荞�ݑ҂�
		
		ReadKeyPad();											// �L�[���͂̎擾
		ReadTpData();											// TP���͂̎擾
		
		(void)SYSM_GetCardTitleList( pTitleList );				// �J�[�h�A�v�����X�g�̎擾�i�X���b�h�Ő����J�[�h�}����ʒm�������̂����C�����[�v�Ŏ擾�j
		
		switch( state ) {
		case START:
			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoMain() ) {
				state = LAUNCHER_INIT;
			}
			break;
		case LAUNCHER_INIT:
			InitBG();										// BG������
			LauncherInit( pTitleList );
			state = LAUNCHER;
			break;
		case LAUNCHER:
			pBootTitle = LauncherMain( pTitleList );
			if( pBootTitle ) {
				state = AUTHENTICATE;
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_LoadAndAuthenticateTitle( pBootTitle ) ) {	// �A�v�����[�h���F��
			case AUTH_PROCESSING:
				break;
			case AUTH_RESULT_SUCCEEDED:
				state = BOOT;
				break;
			case AUTH_RESULT_TITLE_POINTER_ERROR:
			case AUTH_RESULT_AUTHENTICATE_FAILED:
			case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
				state = STOP;
				break;
			}
			break;
		case BOOT:
			SYSM_Finalize();									// �I������
			return;
		case STOP:												// ��~
			break;
		}
	}
}
#endif


// ============================================================================
// function's description
// ============================================================================
/*
void TwlMain(void)
{
	typedef enum PrgState {
		STATE_START = 1,
		STATE_LOGO_DISP,
		STATE_LOGO_MENU,
		STATE_WAIT_BOOT
	}PrgState;
	
	PrgState prg_state     = STATE_START;
	BOOL     boot_decision = FALSE;
	
	// ������----------------------------------
	SYSM_Init();													// �V�X�e�����j���[�֘A�f�[�^�̏������iTwlMain�̐擪�ŃR�[�����ĉ������B�j
	
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	GX_SetPower(GX_POWER_ALL);										// �e���W�b�N �p���[ON
	
	// ���荞�݋���----------------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// �f�o�C�X������-------------------------------
#ifndef __TP_OFF
	TP_Init();
#endif
	(void)RTC_Init();
	
	// �V�X�e���̏�����------------------
	InitAllocator( &g_allocator );
	CMN_InitFileSystem( &g_allocator );

//	InitAllocSystem();
	
	// ARM7�������҂�--------------------------
//	if( SYSM_WaitARM7Init() ) {										// ARM7���̏��������I���̂�҂��Ă��烁�C�����[�v�J�n
//		return;														// TRUE���Ԃ��ꂽ��A�f�o�b�K�u�[�g�Ȃ̂Ń��^�[��
//	}
	
	// ���C�����[�v----------------------------
	while(1){
		OS_WaitIrq(1, OS_IE_V_BLANK);								// V�u�����N���荞�ݑ҂�
		ReadKeyPad();												// �L�[���͂̎擾
		
		if(SYSM_IsTPReadable()) {
			ReadTpData();											// TP���͂̎擾
		}
		
//		if(SYSM_Main()) {											// IPL2�V�X�e���̃��C��
//			return;													// TRUE���A���Ă����烁�C�����[�v���烊�^�[���iNITRO�Q�[���N�����j
//		}
		
		switch(prg_state) {
		  case STATE_START:
			boot_decision = CheckBootStatus();						// �u�[�g��Ԃ��`�F�b�N����B�i�V���[�g�J�b�g�N����R���p�C���X�C�b�`�ɂ�鋭���N���j
//			if( !SYSM_GetBannerFile( &banner ) ) {					// �o�i�[�f�[�^�̃��[�h
//				OS_Printf("ROM banner data read failed.\n");
//			}
			prg_state = STATE_LOGO_DISP;
			break;
			
			//-----------------------------------
			// NITRO���S�\��
			//-----------------------------------
		  case STATE_LOGO_DISP:
			if( LogoMain() ) {										// ���S�\�����[�`���i��BFLG_GAMEBOY_LOGO_OFF�̎��͑��I���j
				InitBG();										// BG������
				LauncherInit();									// �u�[�g�����莞�̂݃��S���j���[������������B
				
				prg_state = STATE_LOGO_MENU;
			}
			break;													// ��NITRO�J�[�h�������łȂ��ꍇ�́A���̂܂ܖ������[�v�B
			
			//-----------------------------------
			// ���S���j���[�ŋN�����[�h�I��
			//-----------------------------------
		  case STATE_LOGO_MENU:
			{
				IPL2BootType command = LauncherMain( boot_decision );
				
				switch(command) {
				case BOOT_TYPE_UNSOLVED:
					break;
					
				case BOOT_TYPE_NITRO:
//					if( !SYSM_BootNITRO() ) {
//						(void)DrawStringSJIS( 4,  20, RED, (const u8 *)"This NITRO card is invalid!!");
//					}
					break;
					
				case BOOT_TYPE_PICT_CHAT:
//					(void)SYSM_BootPictChat();
					break;
					
				case BOOT_TYPE_WIRELESS_BOOT:
//					(void)SYSM_BootDSDownloadPlay();
					break;
					
				case BOOT_TYPE_BMENU:
//					(void)SYSM_BootMachineSetting();
					break;
					
				default:
					OS_Panic( "ERROR: boot code failed : %d\n", command );
				}
				if(command) {
					prg_state = STATE_WAIT_BOOT;
				}
			}
			break;
		  case STATE_WAIT_BOOT:
//			SYSM_PermitToBootSelectedTarget();
			break;
		}
		
		if ( PAD_DetectFold() == TRUE ) {							// �X���[�v���[�h�ւ̑J��
//			SYSM_FinalizeCardPulledOut();
			SYSM_GoSleepMode();
//			(void)SYSM_IsCardPulledOut();							// �J�[�h�������o�R�}���h���s
//			SYSM_FinalizeCardPulledOut();
																	// �J�[�h�������o
//			if ( SYSM_IsCardPulledOut() ) {
			if ( 0 ) {
				(void)PM_ForceToPowerOff();
			}
		}
		
//		if (SYSM_IsCardPulledOut()) {								// �J�[�h�������o
		if ( 0 ) {
			OS_Printf("Card is pulled out.\n");
			OS_Terminate();
		}
		
		OS_PrintServer();											// ARM7����̃v�����g�f�o�b�O����������
		
		//---- BG-VRAM�̍X�V
//		DC_FlushRange ( bgBakS,  sizeof(bgBakS) );
//		MI_CpuCopyFast( bgBakS, (void*)(HW_DB_BG_VRAM+0xf000), sizeof(bgBakS) );
	}
}
*/
	

// �A���P�[�^�̏�����
static void InitAllocator( NNSFndAllocator* pAllocator )
{
    u32   arenaLow      = MATH_ROUNDUP  ((u32)OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 16);
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo(heapSize, 16);
    NNSFndHeapHandle    heapHandle;
    SDK_NULL_ASSERT( pAllocator );

    heapHandle = NNS_FndCreateExpHeap(heapMemory, heapSize);
    SDK_ASSERT( heapHandle != NNS_FND_HEAP_INVALID_HANDLE );

    NNS_FndInitAllocatorForExpHeap(pAllocator, heapHandle, 4);
}

#if 0
// malloc�V�X�e���̏�����
static void InitAllocSystem(void)
{
	void*			tempLo;
	OSHeapHandle	hh;
	
	tempLo	= OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 16);
	OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
	OS_TPrintf( "ArenaLo : %08x  ArenaHi : %08x\n", OS_GetMainArenaLo(), OS_GetMainArenaHi() );
	
	hh		= OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
	if(hh < 0) {
		OS_Panic("ARM9: Fail to create heap...\n");
	}
	hh		= OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}
#endif

// �u�[�g��Ԃ��m�F���A���S�\���L���𔻒f����-------
static BOOL CheckBootStatus(void)
{
	BOOL boot_decision		= FALSE;								// �u�u�[�g���e����v��
	BOOL other_shortcut_off	= FALSE;
	
	//-----------------------------------------------------
	// �f�o�b�O�p�R���p�C���X�C�b�`�ɂ�鋓��
	//-----------------------------------------------------
	{
#ifdef __FORCE_BOOT_BMENU											// ���u�[�g���j���[�����N���X�C�b�`��ON���H
		SYSM_SetBootFlag( BFLG_BOOT_BMENU );
		return TRUE;												// �u�u�[�g���e����v�Ń��^�[��
#endif /* __FORCE_BOOT_BMENU */
		
#ifdef __LOGO_SKIP													// ���f�o�b�O�p���S�X�L�b�v
		SetLogoEnable( FALSE );										// ���S�\���X�L�b�v
#endif /* __LOGO_SKIP */
	}
	
	
	//-----------------------------------------------------
	// NITRO�ݒ�f�[�^�����͎��̐ݒ胁�j���[�V���[�g�J�b�g�N��
	//-----------------------------------------------------
#ifdef __DIRECT_BOOT_BMENU_ENABLE									// ��NITRO�ݒ�f�[�^�����͎��̃u�[�g���j���[���ڋN���X�C�b�`��ON���H
	if( ( (GetNCDWork()->option.input_tp == 0)
		||(GetNCDWork()->option.input_language == 0)
		||(GetNCDWork()->option.input_rtc == 0)
		||(GetNCDWork()->option.input_favoriteColor == 0)
		||(GetNCDWork()->option.input_nickname == 0) ) ) {		// TP,����,RTC,�j�b�N�l�[�����Z�b�g����Ă��Ȃ���΁A���S�\�����Q�[�����[�h���s�킸�A�u�[�g���j���[���V���[�g�J�b�g�N���B
		
		if( ( pad.cont & PAD_PRODUCTION_NITRO_SHORTCUT ) == PAD_PRODUCTION_NITRO_SHORTCUT ) {
			other_shortcut_off = TRUE;								// �ʎY�H���p�̃L�[�V���[�g�J�b�g��������Ă�����A�ݒ胁�j���[�N���͂Ȃ��B
		}else if( !SYSM_IsInspectNITROCard() )  {					// �A���A�ʎY�p�̃L�[�V���[�g�J�b�g��������Ă��鎞���ANITRO�����J�[�h���������Ă��鎞�́A�u�[�g���j���[�ւ̃V���[�g�J�b�g�N���͍s��Ȃ��B
			SYSM_SetBootFlag( BFLG_BOOT_BMENU );
			SetLogoEnable( FALSE );
			return TRUE;											// �u�u�[�g���e����v�Ń��^�[��
		}
	}
#endif /* __DIRECT_BOOT_BMENU_ENABLE */
	
	
	//-----------------------------------------------------
	// �L�[�V���[�g�J�b�g�N��
	//-----------------------------------------------------
	if( !other_shortcut_off && !GetNCDWork()->option.autoBootFlag ) {
																	// ���V���[�g�J�b�gON���I�[�g�N��OFF�̎�
		u32 nowBootFlag = 0;
		
		if(pad.cont & PAD_BUTTON_R){								// R�{�^�������N���Ȃ�A���S�\���Ȃ���AGB�Q�[����
			SetLogoEnable( FALSE );
			nowBootFlag = BFLG_BOOT_AGB;
		}else if(pad.cont & PAD_BUTTON_L){							// L�{�^�������N���Ȃ�A���S�\�����NITRO�Q�[����
			nowBootFlag = BFLG_BOOT_NITRO;
		}else if(pad.cont & PAD_BUTTON_B){							// B�{�^�������N���Ȃ�A���S�\����Ƀu�[�g���j���[��
			nowBootFlag = BFLG_BOOT_BMENU;
		}
		if( nowBootFlag ) {
			SYSM_SetBootFlag( nowBootFlag );
			return TRUE;											// �u�u�[�g���e����v�Ń��^�[��
		}
	}
	
	
	//-----------------------------------------------------
	// �����N���I�v�V�����L�����̋���
	//-----------------------------------------------------
#ifndef __SYSM_DEBUG
	if( GetNCDWork()->option.autoBootFlag ) {
		if ( SYSM_IsNITROCard() ) {									// NITRO�J�[�h�݂̂̎���NITRO�N��
			SYSM_SetBootFlag( BFLG_BOOT_NITRO );
			return TRUE;											// �u�u�[�g���e����v�Ń��^�[��
		}
	}
#endif /* __SYSM_DEBUG */
	
	return FALSE;													// �u�u�[�g���e����v�Ń��^�[��
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

