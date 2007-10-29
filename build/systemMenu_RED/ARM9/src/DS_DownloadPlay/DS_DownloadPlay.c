/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_DownloadPlay.c

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

// ������ mb_private.h��PRINT_DEBUG��"1"�ɂ��Ă���ƁA�_�E�����[�h��ɐ���Ƀu�[�g�ł��Ȃ��Ȃ��Ă���̂ŁA���ӁB

#include <twl.h>
#include <nitro/mb.h>		// ������<nitro/mb.h>�Ƃ���̂�OK?
#include <sysmenu.h>
#include "mb_child.h"
#include "mb_private.h"


#include "misc.h"
#include "DS_Setting.h"
#include "DS_DownloadPlay.h"

// define data------------------------------------------
#define PARENT_VIEW_NUM						4						// �e�@��񃊃X�g�̈��ʕ\����
#define PLIST_X								1						// �e�@��񃊃X�g�̕\��X�ʒu
#define PLIST_Y								4						// �e�@��񃊃X�g�̕\��Y�ʒu
#define	MB_DMA_NO							2						// �}���`�u�[�g�pDMA�ԍ�
#define PRG_NEXT_SEQ_QUE_NUM				4						// ���̃v���O�����V�[�P���X���i�[����L���[�̐�
#define MY_GGID								0x00000000				// �}���`�u�[�g�pGGID
#define G_INTRO_X							1						// �Q�[�����e�����\��X�ʒu
#define G_INTRO_Y							4						// �Q�[�����e�����\��Y�ʒu
#define G_MEMBER_X							1						// �ʐM�����o�[�\��X�ʒu
#define G_MEMBER_Y							10						// �ʐM�����o�[�\��Y�ʒu
#define MSG_VIEW_COUNT						75						// ���b�Z�[�W�\��V�J�E���g��
#define MSG_BLINK_COUNT						12						// ���b�Z�[�W�_�ŃJ�E���g��

#define NINLOGO_LOAD_1D_CHAR											// Nintendo���S�f�[�^��1D�}�b�s���O�L�����N�^�Ń��[�h����B

	// �v���O�����V�[�P���X
typedef enum PrgSeq {
	PSEQ_READY = 0,
	PSEQ_INIT,
	PSEQ_INITIALIZING,
	PSEQ_ENDING,
	PSEQ_END,
	PSEQ_CANCELLED,
	PSEQ_DISCONNECTED_BY_PARENT,
	PSEQ_SCANNING,
	PSEQ_CONNECTING,
	PSEQ_CONNECT_SUCCEEDED,
	PSEQ_CONNECT_FAILED,
	PSEQ_REQ_REFUSED,
	PSEQ_MEMBER_FULL,
	PSEQ_DOWNLOAD_READY,
	PSEQ_DOWNLOADING,
	PSEQ_DOWNLOAD_COMPLETED,
	PSEQ_DOWNLOAD_FAILED,
	PSEQ_BOOT_REQ_WAIT,
	PSEQ_BOOT_READY,
	PSEQ_BOOT_START,
	PSEQ_BOOT_FAILED
}PrgSeq;

static char *str_prgSeq[] = {
	"PSEQ_READY",
	"PSEQ_INIT",
	"PSEQ_INITIALIZING",
	"PSEQ_ENDING",
	"PSEQ_END",
	"PSEQ_CANCELLED",
	"PSEQ_DISCONNECTED_BY_PARENT",
	"PSEQ_SCANNING",
	"PSEQ_CONNECTING",
	"PSEQ_CONNECT_SUCCEEDED",
	"PSEQ_CONNECT_FAILED",
	"PSEQ_REQ_REFUSED",
	"PSEQ_MEMBER_FULL",
	"PSEQ_DOWNLOAD_READY",
	"PSEQ_DOWNLOADING",
	"PSEQ_DOWNLOAD_COMPLETED",
	"PSEQ_DOWNLOAD_FAILED",
	"PSEQ_BOOT_REQ_WAIT",
	"PSEQ_BOOT_READY",
	"PSEQ_BOOT_START",
	"PSEQ_BOOT_FAILED",
};


	// �e�@��񔭌����ԃf�[�^
typedef struct FindTime {
	BOOL		find;												// �����������ǂ���
	int 		vcount;												// ��������
}FindTime;

	// ���b�Z�[�W�\���X�e�[�^�X
typedef struct MsgViewStatus {
	u16			handle;
	u16			vcount;
	u16			flag;
	u16			color;
	const u8	*str;
	int			(*nextProcessp)(void);
}MsgViewStatus;

	// ���v���O�����V�[�P���X���i�[�p�L���[
typedef struct PrgNextSeqQue {
	BOOL		lock;
	u8			pad;
	u8			num;
	u8			top;
	u8			bottom;
	PrgSeq		seq[ PRG_NEXT_SEQ_QUE_NUM ];
}PrgNextSeqQue;


// extern data------------------------------------------
extern void MBw_SetMaxScanTime( u16 time );
extern 	int MBw_GetScanChannel( void );
extern void UnCompNintendoLogo( u16 *NintendoLogoDatap, u16 *dstp, u32 *temp );

// function's prototype declaration---------------------
	// ���C���V�[�P���X
static void SEQ_DSDL_init( void );
static int  SEQ_DSDL_Connect( void );
static int  SEQ_DSDL_Download( void );
static BOOL CheckNextSeq_Connect( void );
static BOOL CheckNextSeq_Download( void );
	// �����R�[���o�b�N
static void CallbackChild_MB( u32 status, void *arg );					// �}���`�u�[�g�p�R�[���o�b�N�֐�
	// �\���n
static void DispTopScreen( void );
static void DispScanStatus( void );
static void DispParentNum( void );
static void DispParentList( void );
static void DispGameIntroduction( void );
static void DispPlayMember( void );
static void DispParentGameInfo( u32 view_no, u32 listNo, BOOL drawFixedDataFlag );
static void ClearDispParentGameInfo( u32 view_no, u32 listNo );
static void DispGameIntroduction_Core( u32 listNo );
static void DispPlayMember_Core( u32 listNo );
static void ClearDispGameIntroduction( void );
static void ClearDispPlayMember( void );
static void DecimalToString( u8 *dstp, const void *valuep, u8 drawLength, u8 size );
static void InitParentFindTime( void );
static void CountParentFindTime( void );
static BOOL CheckParentFindTime( u32 index );
static void CountAndDispTotalTime( void );
static void SetIconOBJ( u32 index );
static void ClearIconOBJ( u32 index );
static void SetDispMessage( u16 color, const u8 *str );
static BOOL DispMessage( void );
static void SetBlinkMessage( u16 color, const u8 *str );
static void DispBlinkMessage( void );
	// Nintendo���S�\��
static void InitDispNintendoLogo( void );
static BOOL DispNintendoLogo( void );
	// ���̑�
static void SetMyUserInfo( MBUserInfo *my );
static BOOL IsScanLock( void );
	// ���荞��
static void VBlankIntr_WDL( void );
	// �V�[�P���X�L���[����
static void InitPrgNextSeqQue( void );
static BOOL SetPrgNextSeqQue( PrgSeq seq );
static BOOL GetPrgNextSeqQue( PrgSeq *seqp );
static void LockPrgNextSeqQue( void );
static void UnlockPrgNextSeqQue( void );
static BOOL IsEmptyPrgNextSeqQue( void );

void SearchPatchAddress( void );

// global variable -------------------------------------

// static variable -------------------------------------
static int			(*wdProcess)( void );							// ���ݎ��s���̃v���Z�X�ւ̃|�C���^
static int			csrC;											// ���j���[�̃J�[�\���ʒu
static int  		csrC_old;										// ���j���[�̑O�J�[�\���ʒu
static PrgSeq		prgSeq;											// �v���O�����V�[�P���X
static PrgNextSeqQue nextSeq;										// ���̃v���O�����V�[�P���X���i�[����L���[
static BOOL			end_flag;										// �I���t���O
static MsgViewStatus msgStat;										// ���b�Z�[�W�\���X�e�[�^�X
static MBUserInfo	myUser;											// �����̃��[�U�[���
static u16			tgid = 0;
static u16			tp_touch_count = 0;
static u32			gameInfoLostCount = 0;
// �e�@���X�g�̕\���Ɏg�p
static int			unrenewalFlag;
static int			total_vcount;
static int			time_ms;										// �e�@��������ms
static int			time_sec;										// �e�@��������sec
static u16			msgHandle;										// ���b�Z�[�W�\���n���h��
static u16			dispGameInfoFlag;								// ���ɕ\�������Q�[�������r�b�g�Ŏ����B
static u16			dispIntroFlag;									// �Q�[���̏ڍ׃f�[�^�`��t���O
static u16			dispMemberFlag;									// �Q�[���̏ڍ׃f�[�^�`��t���O
static u16			gameName[ MB_GAME_NAME_LENGTH + 1 ];			// �e�@�Q�[���l�[��
static u16			userName[ MB_USER_NAME_LENGTH + 1 ];			// �e�@���[�U�[�l�[��
static u16			gameIntroduction[2][ MB_GAME_INTRO_LENGTH / 2 + 1 ];
																	// �e�@�Q�[�����e����
static u16			playMember[ MB_MEMBER_MAX_NUM ][ MB_USER_NAME_LENGTH + 1];
																	// �ʐM�����o�[�����X�g
static u8			parentListNo[ PARENT_VIEW_NUM ][ 4 ] ATTRIBUTE_ALIGN(2);
																	// �e�@���X�g�ԍ�
static u32			objVramBuff[ (MB_ICON_DATA_SIZE * PARENT_VIEW_NUM + 0x20) / sizeof(u32) ] ATTRIBUTE_ALIGN(32);
static FindTime		findTime[ MB_GAME_INFO_RECV_LIST_NUM ];			// �e�@�������ԃf�[�^
static GXOamAttr	oamBakM[ 128 ]    ATTRIBUTE_ALIGN(32);			// OAM �o�b�N�A�b�v

static const MbBeaconRecvStatus *mbrsp;

// MB���C�u�����p���[�N
static u32			mbwork[ MB_CHILD_SYSTEM_BUF_SIZE / sizeof(u32) ];

// Nintendo���S�\��
static int loop_count;
static u32 ninLogoBuff[ 0x700 / sizeof(u32) ];

static MBDownloadFileInfo dlfileinfo;
static u8 output_buffer[ AUTH_BUFFER_LEN ];

// const data  -----------------------------------------
	// �\���p������
static const u8 str_sura[]   ATTRIBUTE_ALIGN(2)	= "/";
static const u8 str_period[] ATTRIBUTE_ALIGN(2)	= ".";
static const u8 str_sec[]    ATTRIBUTE_ALIGN(2)	= "sec";
static const u8 str_disconnected[] ATTRIBUTE_ALIGN(2)		= "disconnected by parent.";
static const u8 str_connect_succeeded[] ATTRIBUTE_ALIGN(2)	= "connect succeeded.";
static const u8 str_connect_failed[] ATTRIBUTE_ALIGN(2)		= "connect failed.";
static const u8 str_req_refused[] ATTRIBUTE_ALIGN(2)		= "req refused.";
static const u8 str_member_full[] ATTRIBUTE_ALIGN(2)		= "member full.";
static const u8 str_downloading[] ATTRIBUTE_ALIGN(2)		= "DOWNLOADING....";
static const u8 str_download_completed[] ATTRIBUTE_ALIGN(2) = "download completed.";
static const u8 str_download_cancelled[] ATTRIBUTE_ALIGN(2) = "download cancelled.";

	// ���j���[�֐��p�@�e�@���X�g�������񃊃X�g
static u8 *str_parentListNo[] ATTRIBUTE_ALIGN(2) = {
	parentListNo[0],
	parentListNo[1],
	parentListNo[2],
	parentListNo[3],
};

	// ���j���[�֐��p�@���j���[�\���f�[�^
static const MenuComponent childModeSel = {
	PARENT_VIEW_NUM,												// ���ڐ�
	PLIST_X,														// X�ʒu�i�L�������j
	PLIST_Y,														// Y�ʒu�i�@�V�@�@�j
	0,																// ���̍��ڂւ�X�ʒu�L������
	4,																// ���̍��ڂւ�Y�ʒu�L������
	3,
	WHITE,															// ��I��F
	HIGHLIGHT_Y,													// �I��F
	(const u8 **)&str_parentListNo,									// ���j���[���ڕ����񃊃X�g
};


//======================================================
// ���C�����[�v
//======================================================

// �����}���`�u�[�g�̃��C�����[�v
int DS_DownloadPlayMain(void)
{
	SEQ_DSDL_init();
	
	wdProcess = SEQ_DSDL_Connect;
	
	OS_TPrintf("MbBeaconRecvStatus:%d\n", sizeof(MbBeaconRecvStatus) );
	
	while(1) {
		OS_WaitIrq(1, OS_IE_V_BLANK);
		ReadKeyPad();
		mf_KEYPAD_rapid();
		
		mbrsp = MB_GetBeaconRecvStatus();
		
		if( wdProcess != NULL ) {
			if(wdProcess()) {
				return 0;
			}
		}
		
		//// �������� ////
		{
			static int touch = 0;
			TPData tmp1 ATTRIBUTE_ALIGN(32);
			TPData tmp2 ATTRIBUTE_ALIGN(32);
			
			while (TP_RequestRawSampling(&tmp1) != 0) {}
			TP_GetCalibratedPoint(&tmp2, &tmp1);
			
			if (tmp2.touch != touch)
			{
				tp_touch_count++;
				(void)DrawDecimalSJIS( 16, 0, RED, &tp_touch_count, 4, 2 );
				OS_Printf("touch change %d  validity = %d\n", tmp2.touch, tmp2.validity);
				touch = tmp2.touch;
			}
		}
		//// �����܂� ////
		
//		if (SYSM_IsCardPulledOut()) {								// �J�[�h�������o
		if ( 0 ) {
			OS_Printf("Card is pulled out.\n");
#ifdef __DEBUG
			OS_Terminate();
#endif
		}
		
		if (PAD_DetectFold() == TRUE) {								// �X���[�v���[�h�ւ̑J��
			SYSM_GoSleepMode();
		}
		
		OS_PrintServer();											// ARM7����̃v�����g�f�o�b�O����������
	}
	return 0;
}


//======================================================
// �����}���`�u�[�g�i�������j
//======================================================

// �����}���`�u�[�g�̏�����
static void SEQ_DSDL_init(void)
{
    GXS_SetVisiblePlane( GX_PLANEMASK_NONE );
    GX_SetVisiblePlane ( GX_PLANEMASK_NONE );
	
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr_WDL);
	GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);
	
	// NITRO�J�[�h��ROM���o�^�f�[�^�̃N���A
	MI_CpuClearFast( (void *)HW_ROM_HEADER_BUF, 0x160 );
	
	// �T�u�X�N���[���̃N���A
	MI_CpuClearFast( bgBakS, sizeof(bgBakS) );
	MI_CpuFillFast ( (void *)HW_OAM, 192 , HW_OAM_SIZE );
	MI_CpuFillFast ( (void *)oamBakS, 192 , sizeof(oamBakS) );		// 192�Ńt�B�����Ȃ��ƁA���g�pOBJ��(0, 0)�ɃL����No.0�őS���\������Ă��܂��B
	
	// ���C���X�N���[���̃N���A
	MI_CpuClearFast( (void *)HW_BG_VRAM,  0x10000 );
	MI_CpuClearFast( (void *)HW_OBJ_VRAM, 0x8000 );
	MI_CpuClearFast( bgBakM, sizeof(bgBakM) );
	MI_CpuFillFast ( (void *)oamBakM, 192 , sizeof(oamBakM) );		// 192�Ńt�B�����Ȃ��ƁA���g�pOBJ��(0, 0)�ɃL����No.0�őS���\������Ă��܂��B
	MI_CpuFillFast ( (void *)HW_OAM,  192 , HW_OAM_SIZE );
	MI_CpuCopyFast ( myPlttData, (void *)HW_BG_PLTT, sizeof(myPlttData) );	//  BG�p���b�g   �Z�b�g
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS  (  1, 0, WHITE,  (const u8 *)"NICKNAME=");
	(void)DrawStringSJIS  (  1, 2, LIGHTGREEN, (const u8 *)"PARENT NUM  =");
//	(void)DrawStringSJIS  (  1, 22, LIGHTGREEN, (const u8 *)"GINFO LOST COUNT =");
	(void)DrawStringSJISEx( 26, 2, CYAN, str_period,	PARENT_VIEW_NUM);
	(void)DrawStringSJISEx( 29, 2, CYAN, str_sec,		PARENT_VIEW_NUM);
	
	InitPrgNextSeqQue();
	unrenewalFlag		= 0;
	end_flag			= FALSE;
	prgSeq				= PSEQ_INIT;
	msgHandle			= 0x8000;
	dispGameInfoFlag	= 0;
	dispIntroFlag		= 0;
	dispMemberFlag		= 0;
	csrC_old			= 2;										// �ŏ��͂킴�ƈႤ�l�ɂ��Ă���
	csrC				= 0;
	{
		int i;
		for( i = 0; i < PARENT_VIEW_NUM; i++ ) {
			parentListNo[i][2] = '.';
			parentListNo[i][3] = 0x00;
		}
	}
	SetMyUserInfo( &myUser );										// MBUserInfo��NITRO�ݒ�f�[�^����ǂݏo���ăZ�b�g
	{
		// �j�b�N�l�[���\��
		u16 nickname[ MB_USER_NAME_LENGTH + 1 ];
		ExUTF16_LEtoSJIS_BE( (u8 *)nickname, (u16 *)myUser.name, myUser.nameLength );
		nickname[ myUser.nameLength ] = 0;
		(void)DrawStringSJIS( 8, 0, WHITE, (const u8 *)nickname );
	}
	
	InitParentFindTime();											// �e�@�������ԕϐ��̃N���A
	
	while( (SYSM_GetBootFlag() & BFLG_WM_INITIALIZED) == 0 ) {}
	
	LOADER_Init( NULL );
	
    GXS_SetVisiblePlane( GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1 );
    GX_SetVisiblePlane ( GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1 );
}


// TOP�X�N���[���ւ̕\��
static void DispTopScreen( void )
{
	SetTargetScreenSJIS( TOP_SCREEN );
	ClearAllStringSJIS();
	(void)DrawStringSJIS(  1, 0, YELLOW, (const u8 *)"WIRELESS DOWNLOAD");
	(void)DrawStringSJIS(  G_INTRO_X,  G_INTRO_Y,  LIGHTGREEN, (const u8 *)"GAME INTRODUCTION");
	(void)DrawStringSJIS(  G_MEMBER_X, G_MEMBER_Y, LIGHTGREEN, (const u8 *)"MEMBER LIST");
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


//======================================================
// �����}���`�u�[�g�i�e�@�T�[�` �` �ڑ��j
//======================================================

// �����}���`�u�[�g�̐ڑ��V�[�P���X
static int SEQ_DSDL_Connect(void)
{
	// ���݂�prgSeq�ɉ���������
	BOOL getSeqFlag = CheckNextSeq_Connect();
	
	switch ( prgSeq ) {
	  case PSEQ_INIT:												// �q�@���[�h�̊J�n�i���C�u�����̏������`StartScan�܂ł̗�������s�j
		if ( MB_Init( mbwork, &myUser, MY_GGID, tgid, MB_DMA_NO ) != WM_ERRCODE_SUCCESS ) {
			OS_Printf("MB init failed.\n");
			break;
		}
		MB_CommSetChildStateCallback( CallbackChild_MB );
		if ( MB_StartChild() != WM_ERRCODE_SUCCESS ) {
			OS_Printf("MB start failed.\n");
			break;
		}
		tgid++;
		DispTopScreen();
		prgSeq = PSEQ_INITIALIZING;
		break;
		
	  case PSEQ_INITIALIZING:
		break;
		
	  case PSEQ_SCANNING:											// �X�L�������͐����A���V�[�P���X�`�F�b�N���s���B
	  case PSEQ_ENDING:
		if( !getSeqFlag ) {
			return 0;
		}
		break;
		
	  case PSEQ_CONNECTING:											// �ڑ����̓L�[���͂��󂯕t�����Ƀ��^�[������B�i���V�[�P���X�`�F�b�N�͍s���j
		return 0;
		
	  case PSEQ_CONNECT_SUCCEEDED:									// ���b�Z�[�W�\����A�_�E�����[�h�V�[�P���X�Ɉڍs�B
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// �V�[�P���X���b�N����
			wdProcess = SEQ_DSDL_Download;
		}
		return 0;
		
	  case PSEQ_CONNECT_FAILED:										// ���b�Z�[�W�\����A�X�L�����ĊJ�B�i�X�L�����ĊJ���̂��̂́A���C�u�����������ōs���B�j
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// �V�[�P���X���b�N����
			prgSeq = PSEQ_SCANNING;
		}
		break;
		
	  case PSEQ_DISCONNECTED_BY_PARENT:								// �ؒf���o���b�Z�[�W�\���ݒ�B
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// �V�[�P���X���b�N����
			prgSeq = PSEQ_INIT;
		}
		break;
		
	  case PSEQ_CANCELLED:											// �I��
		return 0;
		
	  default:
		break;
	}
	
	
	//  �J�[�\���ړ�
	if(pad.trg & PAD_KEY_DOWN){										// �J�[�\���̈ړ�
		if( ++csrC == MB_GAME_INFO_RECV_LIST_NUM ){
			csrC = 0;
		}
	}
	if(pad.trg & PAD_KEY_UP){
		if( --csrC & 0x80 ){
			csrC = MB_GAME_INFO_RECV_LIST_NUM - 1;
		}
	}
	DrawMenu((u16)(csrC & (PARENT_VIEW_NUM - 1)), &childModeSel);	// ���X�g�ԍ��i�J�[�\���j�̕\��
	DispParentNum();												// �����e�@���̕\��
	DispParentList();												// �e�@���X�g�̕\��
	DispScanStatus();												// WM�X�L������Ԃ̕\��
	CountAndDispTotalTime();										// �g�[�^�����Ԃ̃J�E���g���\��
	CountParentFindTime();											// �e�@�������Ԃ̃J�E���g
	
	
	// �e�@�ւ̐ڑ�
	if(pad.trg & PAD_BUTTON_A) {
		if( mbrsp->usefulGameInfoFlag & (0x0001 << csrC) ) {
			if( MB_CommDownloadRequest( csrC ) == WM_ERRCODE_SUCCESS ) {	// �ڑ��J�n
				OS_Printf("start conntct to parent.\n");
				prgSeq = PSEQ_CONNECTING;
			}else {
				OS_Printf("this parent info is invalid.\n");
			}
		}
	}else if(pad.trg & PAD_BUTTON_B) {								// �I���J�n
		if( prgSeq == PSEQ_SCANNING ) {
			OS_Printf("MB End start\n");
			prgSeq	 = PSEQ_ENDING;
//			end_flag = TRUE;
			MB_End();
		}
	}else if( pad.trg & PAD_BUTTON_R ) {
		unrenewalFlag ^= 0x01;
//		MB_SetUnrenewalGameInfoFlag( (BOOL)unrenewalFlag );
	}
	
	csrC_old = csrC;												// ���J�[�\���ʒu�̑ޔ�
	return 0;
}


// ���V�[�P���X�̃`�F�b�N�i�R�[���o�b�N�Ń��C�����[�v�Ƃ͔񓯊��ɕԂ���鎟�V�[�P���X�ւ̈ڍs�����C�����[�v�̃v���O�����i�s�ɍ��킹��B�j
static BOOL CheckNextSeq_Connect(void)
{
	PrgSeq seq;
	// MB�R�[���o�b�N�ɂ���āA�V�[�P���X���ڍs���Ă����ꍇ�̏���
	if( GetPrgNextSeqQue( &seq) ) {									// MB�R�[���o�b�N�ɂ���āA�V�[�P���X���ڍs���Ă����ꍇ�̏���
		prgSeq = seq;
		OS_Printf("prgSeq = %s\n", str_prgSeq[ prgSeq ]);
		
		switch( seq ) {
		  case PSEQ_SCANNING:
			{
/*
				u8 *ver = MBw_GetWLVersion();
				(void)DrawStringSJIS( 21, 0, WHITE, (const u8 *)"WL ver=" );
				(void)DrawStringSJIS( 26, 0, WHITE, (const u8 *)ver );
*/
			}
			break;
			
		  case PSEQ_CONNECT_SUCCEEDED:								// �ڑ��������b�Z�[�W�\���ݒ�B
			SetDispMessage( YELLOW, str_connect_succeeded );
			LockPrgNextSeqQue();									// prgSeq�����b�N����B
			return FALSE;
			
		  case PSEQ_CONNECT_FAILED:									// �ڑ����s���b�Z�[�W�\���ݒ�B
			SetDispMessage( RED, str_connect_failed);
			LockPrgNextSeqQue();									// prgSeq�����b�N����B
			return FALSE;
			
		  case PSEQ_DISCONNECTED_BY_PARENT:							// �ؒf���o���b�Z�[�W�\���ݒ�B
			SetDispMessage( RED, str_disconnected);
			LockPrgNextSeqQue();									// prgSeq�����b�N����B
			return FALSE;
			
		  case PSEQ_CANCELLED:										// �I��
			{
				u32 i;
				for( i = 0; i < PARENT_VIEW_NUM; i++ ) {
					ClearIconOBJ( i );								// �S�A�C�R���̏���
				}
//				if( end_flag ) {
//					ClearAllStringSJIS();
//					SetTargetScreenSJIS( TOP_SCREEN );
//					ClearAllStringSJIS();
//					SetTargetScreenSJIS( BOTTOM_SCREEN );
//					(void)DrawStringSJIS( 11, 11, WHITE, (const u8 *)"���イ��傤���܂����B");
//				}else {
					prgSeq = PSEQ_INIT;
//				}
			}
			return FALSE;
		}
	}
	return TRUE;													// �V�[�P���X�ڍs���Ȃ������ꍇ��������FALSE���^�[���̃V�[�P���X�łȂ������ꍇ��TRUE���^�[���B
}


//======================================================
// �����}���`�u�[�g�i�_�E�����[�h�j
//======================================================

// �����}���`�u�[�g�̃t�@�C���_�E�����[�h�V�[�P���X
static int SEQ_DSDL_Download(void)
{
	static int count_old = 0;
	// ���݂�prgSeq�ɉ���������
	BOOL getSeqFlag = CheckNextSeq_Download();
	
	switch(prgSeq) {
		
	  case PSEQ_REQ_REFUSED:
	  case PSEQ_MEMBER_FULL:										// ���b�Z�[�W�\����A��U�I�����ď����������蒼���B
		if( DispMessage() ) {
			UnlockPrgNextSeqQue();									// �V�[�P���X���b�N����
			prgSeq		= PSEQ_ENDING;
			wdProcess	= SEQ_DSDL_Connect;
			MB_End();
		}
		break;
		
		// �_�E�����[�h�t�@�C��������M���A�_�E�����[�h�������ł������
	  case PSEQ_DOWNLOAD_READY:
//		if (pad.trg & PAD_BUTTON_A) {								// �_�E�����[�h�J�n
			OS_Printf("Start download.\n");
			ClearStringSJIS_handle(msgHandle);
			(void)MB_CommStartDownload();
			SetBlinkMessage( YELLOW, str_downloading );
//		}
		break;
		
		// �_�E�����[�h��
	  case PSEQ_DOWNLOADING:
		if( getSeqFlag ) {
			DispBlinkMessage();
			msgHandle = msgStat.handle;
			
			if( 1 ) {
				u16 percent = MB_GetChildProgressPercentage();
				msgHandle = DrawDecimalSJIS( 16, 21, WHITE, &percent ,3, 2 );
			}
		}
		break;
		
		// �_�E�����[�h����
	  case PSEQ_DOWNLOAD_COMPLETED:
		if( DispMessage() ) {
			{
				msgHandle = DrawStringSJIS( 4, 21,  LIGHTGREEN, "Sign Digest....");
				if ( ACSignDigest ( output_buffer, &dlfileinfo ) ) {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  LIGHTGREEN, "Digest SUCCEEDED.");
				}else {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  RED,        "Digest FAILED.");
				}
				{
					int i;
					for ( i = 0; i < 120; i++ ) {
						SVC_WaitVBlankIntr();
					}
				}
				ClearStringSJIS_handle( msgHandle );
			}
			UnlockPrgNextSeqQue();									// �V�[�P���X���b�N����
			prgSeq = PSEQ_BOOT_REQ_WAIT;
			msgHandle = DrawStringSJIS( 4, 21,  WHITE, "Waiting BOOT-REQ from parent.");
		}
		break;
		
		// �e�@����̃u�[�g���҂�
	  case PSEQ_BOOT_REQ_WAIT:
		break;
		
		// �_�E�����[�h���������A�u�[�g�������ł������
//	  case PSEQ_BOOT_READY:
//		(void)LOADER_Start();										// ������ARM7�ɂ���āA�_�E�����[�h�v���O�����̍Ĕz�u���s���B
//		return 1;													// "1"�Ń��^�[�����邱�ƂŁA�����}���`�u�[�g�̃��C�����[�v���甲����IPL2�ɏ�����߂��B
		
		// �e�@����̃u�[�g�v����M�ɂ��A�N���ۃ`�F�b�N���s���B�iNintendo���S�\�����`�F�b�N�j
	  case PSEQ_BOOT_READY:
		if( DispNintendoLogo() ) {
			if( SYSM_CheckNinLogo( (u16 *)GetRomHeaderAddr()->nintendo_logo ) ) {
				prgSeq = PSEQ_BOOT_START;
			}else {
				SetDispMessage( RED, (const u8 *)"Illegal game data.");
				prgSeq = PSEQ_BOOT_FAILED;
			}
		}
		break;
		
		// �u�[�g�J�n
	  case PSEQ_BOOT_START:
		ClearStringSJIS_handle(msgHandle);
		
		SearchPatchAddress();
		
		(void)LOADER_Start();										// ������ARM7�ɂ���āA�_�E�����[�h�v���O�����̍Ĕz�u���s���B
		return 1;													// "1"�Ń��^�[�����邱�ƂŁA�����}���`�u�[�g�̃��C�����[�v���甲����IPL2�ɏ�����߂��B
		
		// �u�[�g���s
		// �e�@�ɂ���āA�ڑ����ؒf���ꂽ���
		// �L�����Z���������ꍇ
	  case PSEQ_BOOT_FAILED:
	  case PSEQ_DISCONNECTED_BY_PARENT:
	  case PSEQ_CANCELLED:
		if( DispMessage() ) {										// �w�胁�b�Z�[�W�������ԕ\��
			UnlockPrgNextSeqQue();									// �V�[�P���X���b�N����
			ClearStringSJIS_handle( msgHandle );
			prgSeq		= PSEQ_INIT;
			wdProcess	= SEQ_DSDL_Connect;
		}
		break;
		
	  default:
		break;
	}
	
	// B�{�^���ŏI���������s��
	if( pad.trg & PAD_BUTTON_B ) {
		MB_End();													// �L�����Z�����́A�e�@�T�[�`��Ԃ����蒼���B
	}
	
	DrawMenu((u16)(csrC & (PARENT_VIEW_NUM - 1)), &childModeSel);	// ���X�g�ԍ��i�J�[�\���j�̕\��
	DispParentNum();												// �����e�@���̕\��
	DispParentList();												// �e�@���X�g�̕\��
	DispScanStatus();												// WM�X�L������Ԃ̕\��
	CountAndDispTotalTime();										// �g�[�^�����Ԃ̃J�E���g���\��
	CountParentFindTime();											// �e�@�������Ԃ̃J�E���g
	return 0;
}


// ���V�[�P���X�̃`�F�b�N�i�R�[���o�b�N�Ń��C�����[�v�Ƃ͔񓯊��ɕԂ���鎟�V�[�P���X�ւ̈ڍs�����C�����[�v�̃v���O�����i�s�ɍ��킹��B�j
static BOOL CheckNextSeq_Download(void)
{
	PrgSeq seq;
	
	// MB�R�[���o�b�N�ɂ���āA�V�[�P���X���ڍs���Ă����ꍇ�̏���
	if( GetPrgNextSeqQue( &seq) ) {
		prgSeq = seq;
		OS_Printf("prgSeq = %s\n", str_prgSeq[ prgSeq ]);
		
		switch( seq ) {
			
		  case PSEQ_REQ_REFUSED:									// �ڑ����ۃ��b�Z�[�W�\���ݒ�
			SetDispMessage( RED, str_req_refused);
			LockPrgNextSeqQue();									// prgSeq�����b�N����B
			return FALSE;
			
		  case PSEQ_MEMBER_FULL:									// �����o�[FULL���b�Z�[�W�\���ݒ�B
			SetDispMessage( RED, str_member_full);
			LockPrgNextSeqQue();									// prgSeq�����b�N����B
			return FALSE;
			
		  case PSEQ_DOWNLOAD_READY:									// �_�E�����[�h��������
			ClearStringSJIS_handle( msgHandle );
			{
				msgHandle = DrawStringSJIS( 4, 21,  WHITE, "Sign Decript....");
				if ( ACSignDecrpto( output_buffer, &dlfileinfo ) ) {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  LIGHTGREEN, "Decript SUCCEEDED.");
				}else {
					ClearStringSJIS_handle( msgHandle );
					msgHandle = DrawStringSJIS( 4, 21,  RED,        "Decript FAILED.");
				}
				{
					int i;
					for ( i = 0; i < 120; i++ ) {
						SVC_WaitVBlankIntr();
					}
				}
				ClearStringSJIS_handle( msgHandle );
			}
			
			msgHandle = DrawStringSJIS( 4, 21,  WHITE, "PUSH [A] TO DOWNLOAD.");
			break;
			
		  case PSEQ_DOWNLOAD_COMPLETED:								// �_�E�����[�h����
			ClearStringSJIS_handle( msgHandle );
			SetDispMessage( YELLOW, str_download_completed );
			LockPrgNextSeqQue();									// �V�[�P���X���b�N
			return FALSE;
			
//		  case PSEQ_BOOT_READY:										// �u�[�g��������
//			ClearStringSJIS_handle( msgHandle );
//			break;
			
		  case PSEQ_BOOT_READY:										// �u�[�g��������
			ClearStringSJIS_handle( msgHandle );
			InitDispNintendoLogo();
			break;
			
		  case PSEQ_CANCELLED:
			ClearStringSJIS_handle( msgHandle );
			SetDispMessage( RED, str_download_cancelled );
			LockPrgNextSeqQue();									// �V�[�P���X���b�N
			return FALSE;
			
		  case PSEQ_DISCONNECTED_BY_PARENT:							// �ؒf���o���b�Z�[�W�\���ݒ�B
			ClearStringSJIS_handle( msgHandle );
			SetDispMessage( RED, str_disconnected );
			LockPrgNextSeqQue();									// prgSeq�����b�N����B
			return FALSE;
			
		  default:
			break;
		}
	}
	return TRUE;
}


//======================================================
// �R�[���o�b�N
//======================================================

// �q�@���[�h�ł̃R�[���o�b�N
static void CallbackChild_MB( u32 status, void *arg )
{
	switch (status)
	{
	  case MB_COMM_CSTATE_INIT_COMPLETE:							// ����������
		OS_Printf("CB: init completed.\n");
		(void)SetPrgNextSeqQue( PSEQ_SCANNING );
		break;
		
//	  case MB_COMM_CSTATE_END_COMPLETE:								// �}���`�u�[�g�I��
//		OS_Printf("CB: end completed.\n");
//		(void)SetPrgNextSeqQue( PSEQ_END );
//		break;

	  case MB_COMM_CSTATE_REQ_REFUSED:								// �e�@����̃G���g���[����
	    OS_Printf("CB: request refused.\n");
		(void)SetPrgNextSeqQue( PSEQ_REQ_REFUSED );
	    break;

	  case MB_COMM_CSTATE_MEMBER_FULL:								// �Q�[��������ɒB����
	    OS_Printf("CB: member full.\n");
		(void)SetPrgNextSeqQue( PSEQ_MEMBER_FULL );
	    break;
		
	  case MB_COMM_CSTATE_CONNECT:									// �e�@�ւ̐ڑ�����
		OS_Printf("CB: connected.\n");
		(void)SetPrgNextSeqQue( PSEQ_CONNECT_SUCCEEDED );
		break;
		
	  case MB_COMM_CSTATE_CONNECT_FAILED:							// �e�@�ւ̐ڑ����s
		OS_Printf("CB: connect failed!\n");
		(void)SetPrgNextSeqQue( PSEQ_CONNECT_FAILED );
		break;
		
	  case MB_COMM_CSTATE_DLINFO_ACCEPTED:							// �e�@����_�E�����[�h������M
		OS_Printf("CB: dlinfo accepted.\n");
		(void)SetPrgNextSeqQue( PSEQ_DOWNLOAD_READY );
		MI_CpuCopy16( (void *)arg, (void *)&dlfileinfo, sizeof( MBDownloadFileInfo ) ) ;
		break;
		
	  case MB_COMM_CSTATE_RECV_PROCEED:								// ��M�J�n
		OS_Printf("CB: recv proceed.\n");
		(void)SetPrgNextSeqQue( PSEQ_DOWNLOADING );
		break;
		
	  case MB_COMM_CSTATE_RECV_COMPLETE:							// ��M����
		OS_Printf("CB: recv completed.\n");
		(void)SetPrgNextSeqQue( PSEQ_DOWNLOAD_COMPLETED );
		break;
		
	  case MB_COMM_CSTATE_BOOT_READY:								// �u�[�g��������
		OS_Printf("CB: boot ready.\n");
//		GetSYSMWork()->mb_flag = 1;
//		GetSYSMWork()->mb_ggid = *(u32 *)( MB_GetBeaconRecvStatus()->list[ pCwork->connectTargetNo ].bssDesc.gameInfo.ggid );
//		GetSYSMWork()->mb_ggid = MB_GetBeaconRecvStatus()->list[ pCwork->connectTargetNo ].gameInfo.ggid;
		(void)SetPrgNextSeqQue( PSEQ_BOOT_READY );
		break;
		
	  case MB_COMM_CSTATE_CANCELLED:								// �_�E�����[�h�L�����Z��
		OS_Printf("CB: download cancel.\n");
		(void)SetPrgNextSeqQue( PSEQ_CANCELLED );
		break;
		
	  case MB_COMM_CSTATE_DISCONNECTED_BY_PARENT:					// �e�@����̐ؒf�ʒm
		OS_Printf("CB: disconnected by parent!\n");
		(void)SetPrgNextSeqQue( PSEQ_DISCONNECTED_BY_PARENT );
		break;
		
	  case MB_COMM_CSTATE_GAMEINFO_LOST:
		gameInfoLostCount++;
		break;
	}
}


//======================================================
// �\���n�T�u���[�`��
//======================================================

// �X�L������Ԃ̕\��
static void DispScanStatus(void)
{
#if 0
	int channel = MBw_GetScanChannel();
	
	(void)DrawHexSJIS( 13, 2, WHITE, &channel, 2);
	if( IsScanLock() ) {											// �X�L�������b�N���Ȃ�A���̎��Ԃ�\��
		(void)DrawStringSJIS( 15, 2, RED,   (const u8 *)"LOCK");
		(void)DrawHexSJIS   ( 19, 2, WHITE, &mbrsp->nowLockTimeCount, 4);
	}else {
		(void)DrawStringSJIS( 15, 2, RED, (const u8 *)"          ");
	}
#endif
}


// ���������e�@���̕\��
static void DispParentNum(void)
{
	int i;
	int num = 0;
	
	for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++) {				// �e�@���̃J�E���g
		if( mbrsp->usefulGameInfoFlag & (0x0001 << i) )	num++;
	}
	(void)DrawDecimalSJIS( 10, 2, LIGHTGREEN, &num, 2, 4);
}


// �e�@��񃊃X�g�̕\��
static void DispParentList(void)
{
	u32 i;
	u32 listNo		= (u32)( csrC     & ~(PARENT_VIEW_NUM - 1) );	// ���ݕ\�����̃��X�g�̐擪���Z�o�i~�\�����Ń}�X�N�j
	u32 listNo_old	= (u32)( csrC_old & ~(PARENT_VIEW_NUM - 1) );	// �O��\���������X�g�̐擪���Z�o
	
//	(void)DrawDecimalSJISEx( 15, 22, WHITE, &gameInfoLostCount, 8, 4, 0);
	
	// �Q�[�����e�����̕\��
	DispGameIntroduction();
	
	// �ʐM�����o�[�̕\��
	DispPlayMember();
	
	// �e�@���X�g�̕\��
	for( i = 0; i < PARENT_VIEW_NUM; i++ ) {
		if( (listNo != listNo_old) || !(mbrsp->usefulGameInfoFlag & (0x0001 << listNo)) ){
			if( dispGameInfoFlag & (0x0001 << listNo_old) ) {		// �\�����X�g�ʒu���ύX or �Y���e�@���L���łȂ��Ȃ����ꍇ�A�O��\���f�[�^�����ɕ\���ς݂Ȃ�N���A����B
				ClearDispParentGameInfo( i, listNo_old);
				dispGameInfoFlag ^= 0x0001 << listNo_old;
			}
		}
		
		if( mbrsp->usefulGameInfoFlag & (0x0001 << listNo) ) {		// �Y�����X�g���̃f�[�^���L���ŁA�����\���Ȃ�\������B
			DispParentGameInfo( i, listNo , !(dispGameInfoFlag & (0x0001 << listNo)) );
			dispGameInfoFlag |= 0x0001 << listNo;
		}
		if( csrC != csrC_old ) {
			DecimalToString( parentListNo[i], &listNo, 2, 2);		// �r���[��ʂ̐擪���X�g�i���o�[����𕶎���o�b�t�@�ɓ����B
		}
		listNo++;
		listNo_old++;
	}
}


// �Q�[�����e�����̕\��
static void DispGameIntroduction( void )
{
	BOOL clear = FALSE;
	
	if( ( ( csrC != csrC_old ) && ( dispIntroFlag & (0x0001 << csrC_old) ) )
	 || ( !(mbrsp->usefulGameInfoFlag & (0x0001 << csrC)) && ( dispIntroFlag & (0x0001 << csrC) ) ) ) {
		ClearDispGameIntroduction();								// �J�[�\���������āA�O�J�[�\���ʒu�̏ڍו\�����s���Ă������A
		dispIntroFlag	= 0;
		clear			= TRUE;
	}
	
	if( (mbrsp->usefulGameInfoFlag & (0x0001 << csrC)) && !( dispIntroFlag & (0x0001 << csrC) ) ) {
		if ( !clear ){												// ���݃J�[�\���ʒu�̃Q�[����񂪗L���ŁA�܂��ڍו\�����s���Ă��Ȃ�������\���B
			ClearDispGameIntroduction();
		}
		DispGameIntroduction_Core( (u32)csrC );
		dispIntroFlag	= (u16)( 0x0001 << csrC );					// �J�[�\�����ړ����ꂽ�����A���݈ʒu�̃Q�[����񂪖��\���̎��ɂ̂ݕ\�� or �N���A���s���B
	}
}


// �ʐM�����o�[�̃N���A���\��
static void DispPlayMember( void )
{
	BOOL clear = FALSE;
	
	if( ( ( csrC != csrC_old ) && ( dispMemberFlag & (0x0001 << csrC_old) ) )
	 || ( !(mbrsp->validGameInfoFlag & (0x0001 << csrC)) && ( dispMemberFlag & (0x0001 << csrC) ) ) ) {
		ClearDispPlayMember();										// �J�[�\���ʒu�̃Q�[���ڍ׏���\�����Ă��āA���̃Q�[����񂪖����ɂȂ�����N���A�B
		dispMemberFlag	= 0;
		clear			= TRUE;
	}
	
	if( (mbrsp->validGameInfoFlag & (0x0001 << csrC)) && !( dispMemberFlag & (0x0001 << csrC) ) ) {
		if ( !clear ){												// ���݃J�[�\���ʒu�̃Q�[����񂪗L���ŁA�܂��ڍו\�����s���Ă��Ȃ�������\���B
			ClearDispPlayMember();
		}
		DispPlayMember_Core( (u32)csrC );
		dispMemberFlag	= (u16)( 0x0001 << csrC );					// �J�[�\�����ړ����ꂽ�����A���݈ʒu�̃Q�[����񂪖��\���̎��ɂ̂ݕ\�� or �N���A���s���B
	}
}


// �w�肳�ꂽ�e�@���̕\��
static void DispParentGameInfo( u32 view_no, u32 listNo, BOOL drawFixedDataFlag)
{
	if(drawFixedDataFlag) {											// �ω����Ȃ����̕\��
		
		MI_CpuClear16( gameName, MB_GAME_NAME_LENGTH * 2 );
		MI_CpuClear16( userName, MB_USER_NAME_LENGTH * 2 );
		ExUTF16_LEtoSJIS_BE( (u8 *)gameName, (u16 *)mbrsp->list[ listNo ].gameInfo.fixed.gameName, MB_GAME_NAME_LENGTH * 2 );
		ExUTF16_LEtoSJIS_BE( (u8 *)userName, (u16 *)mbrsp->list[ listNo ].gameInfo.fixed.parent.name, (u16)( mbrsp->list[ listNo ].gameInfo.fixed.parent.nameLength * 2));
//		MI_CpuCopy16( mbrsp->list[ listNo ].gameInfo.fixed.gameName, gameName, MB_GAME_NAME_LENGTH * 2);
//		MI_CpuCopy16( mbrsp->list[ listNo ].gameInfo.fixed.parent.name, userName, mbrsp->list[ listNo ].gameInfo.fixed.parent.nameLength * 2);
		gameName[ MB_GAME_NAME_LENGTH ] = 0;
		userName[ mbrsp->list[ listNo ].gameInfo.fixed.parent.nameLength ] = 0;
		
		(void)ClearStringSJISEx( gameName, view_no);
		(void)ClearStringSJISEx( userName, view_no);
		(void)DrawStringSJISEx(  9, (PLIST_Y + (int)view_no*4),     WHITE, gameName, view_no);
		(void)DrawStringSJISEx(  9, (PLIST_Y + (int)view_no*4 + 2), WHITE, userName, view_no);
		
		(void)DrawStringSJISEx( 26, (PLIST_Y + (int)view_no*4),     YELLOW, str_sura, view_no);
		(void)DrawDecimalSJIS ( 27, (PLIST_Y + (int)view_no*4),     YELLOW, &mbrsp->list[ listNo ].gameInfo.fixed.maxPlayerNum, 2, 1);
		
		// �e�@�������Ԃ̕\��
		(void)CheckParentFindTime( listNo );						// �����Őe�@�𔭌����Ă��邩���Ċm�F���Ă����B
		time_ms		= (int)findTime[listNo].vcount * 17;
		time_sec	= time_ms / 1000;
		time_ms		= (time_ms % 1000) / 10;
		(void)DrawDecimalSJISEx( 23, (PLIST_Y + (int)view_no*4 + 2), CYAN, &time_sec, 4, 4, view_no);
		(void)DrawDecimalSJISEx( 27, (PLIST_Y + (int)view_no*4 + 2), CYAN, &time_ms,  2, 4, view_no);
		(void)DrawStringSJISEx ( 26, (PLIST_Y + (int)view_no*4 + 2), CYAN, str_period, view_no);
		(void)DrawStringSJISEx ( 29, (PLIST_Y + (int)view_no*4 + 2), CYAN, str_sec, view_no);
		
		SetIconOBJ( listNo );										// �A�C�R���̕\��
	}
	{	// �ω�������i�������o�[���E�e�@�����J�E���g�̕\���j
		(void)DrawDecimalSJIS( 24, (PLIST_Y + (int)view_no*4),     YELLOW, &mbrsp->list[ listNo ].gameInfo.volat.nowPlayerNum, 2, 1);
		(void)DrawHexSJIS    ( 19, (PLIST_Y + (int)view_no*4 + 2), CYAN,   &mbrsp->list[ listNo ].lifetimeCount, 4);
		{
			u16 color = RED;
			if ( mbrsp->validGameInfoFlag & ( 0x0001 << listNo ) ) {
				color = CYAN;
			}
			(void)DrawHexSJIS    ( 19, (PLIST_Y + (int)view_no*4), color,   &mbrsp->list[ listNo ].gameInfo.seqNoVolat, 2);
		}
	}
}


// �w�肳�ꂽ�e�@���̕\���N���A
static void ClearDispParentGameInfo( u32 view_no, u32 listNo)
{
	// �������̕ϐ��̃A�h���X�́ADispParentGameInfo���̂��̂ƈ�v���ĂȂ��ƁA�\�����N���A���邱�Ƃ��ł��Ȃ��̂Œ��ӁB�i�����ϐ��ŕ\������Ƃ��̓A�h���X���ς���Ă��܂��j
	// �i�����DrawStringSJIS�֐��̎d�l�j
	ClearStringSJISEx( gameName,   view_no );						// �Q�[����
	ClearStringSJISEx( userName,   view_no );						// ���[�U�[��
	ClearStringSJISEx( &time_sec,  view_no );						// �e�@��������sec
	ClearStringSJISEx( &time_ms,   view_no );						// �e�@��������ms
	ClearStringSJISEx( (void *)str_sura,   view_no );				// "/"
	ClearStringSJISEx( (void *)str_period, view_no );				// "."
	ClearStringSJISEx( (void *)str_sec,    view_no );				// "sec"
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].lifetimeCount );				// �e�@�����J�E���g
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].gameInfo.volat.nowPlayerNum );	// �v���C�l��
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].gameInfo.fixed.maxPlayerNum );	// �ő�v���C�l��
	ClearStringSJIS ( (void *)&mbrsp->list[ listNo ].gameInfo.seqNoVolat );	// �ő�v���C�l��
	
	ClearIconOBJ( listNo );											// �A�C�R���̏���
}


// �Q�[�����e�����̕\���i������)
static void DispGameIntroduction_Core( u32 listNo )
{
	int i;
	const u16 *strp = mbrsp->list[ listNo ].gameInfo.fixed.gameIntroduction;
	u16 *dstp;
	u16	gameIntroTmp[ MB_GAME_INTRO_LENGTH / 2 + 1 ];
	
	MI_CpuClear16( gameIntroduction[0], (MB_GAME_INTRO_LENGTH / 2 + 1) * sizeof(u16) );
	MI_CpuClear16( gameIntroduction[1], (MB_GAME_INTRO_LENGTH / 2 + 1) * sizeof(u16) );
	
	dstp = gameIntroTmp;
	for ( i = 0; i < MB_GAME_INTRO_LENGTH / 2; i++ ) {				// �o�C�g�P�ʂł̃R�s�[�Ȃ̂ŁA1�s���́AMB_GAME_INTRO_LENGTH/2*2�B
		if( ( *strp == NULL) || ( *strp == 0x000a ) ) {
			*dstp++ = 0x0000;
		}else {
			*dstp++ = *strp++;
		}
	}
	ExUTF16_LEtoSJIS_BE( (u8 *)gameIntroduction[ 0 ], gameIntroTmp, MB_GAME_INTRO_LENGTH / 2 );
	
	if( *strp == 0x000a ) {
		strp++;
	}
	
	dstp = gameIntroTmp;
	for ( i = 0; i < MB_GAME_INTRO_LENGTH / 2; i++ ) {
		if( ( *strp == NULL) || ( *strp == 0x000a ) ) {
			*dstp++ = 0x00;
		}else {
			*dstp++ = *strp++;
		}
	}
	ExUTF16_LEtoSJIS_BE( (u8 *)gameIntroduction[ 1 ], gameIntroTmp, MB_GAME_INTRO_LENGTH / 2 + 1 );
	
	ClearDispGameIntroduction();
	
	SetTargetScreenSJIS( TOP_SCREEN );
	(void)DrawStringSJIS(  ( G_INTRO_X + 1 ), ( G_INTRO_Y + 2 ), WHITE, (const u8 *)gameIntroduction[0] );
	(void)DrawStringSJIS(  ( G_INTRO_X + 1 ), ( G_INTRO_Y + 4 ), WHITE, (const u8 *)gameIntroduction[1] );
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// �Q�[�����e�����\���̃N���A
static void ClearDispGameIntroduction( void )
{
	SetTargetScreenSJIS( TOP_SCREEN );
	ClearStringSJIS( (void *)gameIntroduction[0] );
	ClearStringSJIS( (void *)gameIntroduction[1] );
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// �ʐM�����o�[�̕\���i�������j
static void DispPlayMember_Core( u32 listNo )
{
	int n, count_x;
	u16 nameLength;
	int pos_x, pos_y;
	const MBUserInfo *memberp = &mbrsp->list[ listNo ].gameInfo.volat.member[ 0 ];
	u16 *dstp;
	
	SetTargetScreenSJIS( TOP_SCREEN );
	
	MI_CpuClear16( playMember, sizeof(playMember) );
	count_x	= 0;
	pos_x	= G_MEMBER_X + 1;
	pos_y	= G_MEMBER_Y + 2;
	for ( n = 0; n < MB_MEMBER_MAX_NUM; n++ ) {
		
		if( memberp->nameLength > MB_USER_NAME_LENGTH ) {			// ���O���̃`�F�b�N
			nameLength = MB_USER_NAME_LENGTH;
		}else {
			nameLength = memberp->nameLength;
		}
		dstp = playMember[ n ];
		if( mbrsp->list[ listNo ].gameInfo.volat.nowPlayerFlag & (0x0002 << n ) ) {
			
			ExUTF16_LEtoSJIS_BE( (u8 *)dstp, (u16 *)memberp->name, nameLength );
			playMember[ n ][ nameLength ] = 0;
			(void)DrawStringSJIS( pos_x, pos_y, WHITE, (const u8 *)playMember[ n ] );
		}else {
			MI_CpuCopy16( (void *)"----------", (void *)playMember[ n ], MB_USER_NAME_LENGTH + 1);
			(void)DrawStringSJIS( pos_x, pos_y, WHITE, playMember[ n ] );
		}
		memberp++;
		
		if( ++count_x == 3 ) {										// �\���ʒu�̎Z�o
			pos_x	-= 20;
			pos_y	+= 2;
			count_x	 = 0;
		}else {
			pos_x	+= 10;
		}
	}
	
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// �ʐM�����o�[�\���̃N���A
static void ClearDispPlayMember( void )
{
	int i;
	
	SetTargetScreenSJIS( TOP_SCREEN );
	for( i = 0; i < MB_MEMBER_MAX_NUM + 1; i++ ) {
		ClearStringSJIS( (void *)playMember[ i ] );
	}
	SetTargetScreenSJIS( BOTTOM_SCREEN );
}


// 10�i�f�[�^�𕶎���ɕϊ�
static void DecimalToString(u8 *dstp, const void *valuep, u8 drawLength, u8 size)
{
	u16 count;
	u32 mask, divisor, target;
	
	mask = 0xff;
	while(--size > 0) {
		mask = (mask << 8) | 0xff;
	}
	target=(*(u32 *)valuep) & mask;
	count=10;
	divisor=1000000000;
	while(count) {
		CP_SetDiv32_32(target, divisor);
		if (count <= drawLength) {
			*dstp++ = (u8)(CP_GetDivResult32()+0x0030);
		}
		target=(u32)CP_GetDivRemainder32();
		CP_SetDiv32_32(divisor, 10);
		divisor=(u32)CP_GetDivResult32();
		count--;
	}
}


// �e�@�������ԕϐ��̃N���A
static void InitParentFindTime( void )
{
	int i;
	for( i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++) {
		findTime[i].find	= FALSE;
		findTime[i].vcount	= 0;
	}
	total_vcount		= 0;
}


// �e�@�������Ԃ̃J�E���g
static void CountParentFindTime( void )
{
	u32 i;
	
	for( i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++ ) {
		if( !CheckParentFindTime( i ) ) {
			findTime[i].vcount = total_vcount;						// �e�@��񂪌�����܂ŃJ�E���g
		}
	}
}


// �e�@��񂪑��������ǂ����`�F�b�N���āA�������ԃJ�E���g���~����B
static BOOL CheckParentFindTime( u32 index )
{
	if( mbrsp->validGameInfoFlag & (0x01 << index) ) {				// �e�@��񂪔������ꂽ�Ȃ�A�J�E���g���~����B
		findTime[ index ].find = TRUE;
	}
	return findTime[ index ].find;
}


// �g�[�^�����Ԃ̃J�E���g���\��
static void CountAndDispTotalTime( void )
{
	total_vcount++;
	time_ms		= total_vcount * 17;
	time_sec	= time_ms / 1000;
	time_ms		= (time_ms % 1000) / 10;
	(void)DrawDecimalSJISEx( 23, 2, CYAN, &time_sec, 4, 4,	PARENT_VIEW_NUM);
	(void)DrawDecimalSJISEx( 27, 2, CYAN, &time_ms,  2, 4,	PARENT_VIEW_NUM);
}


// �A�C�R��OBJ�̃��[�h
static void SetIconOBJ( u32 index )
{
	u16 view_no			= (u16)( index & 0x03 );
	const MBIconInfo *iconp	= &mbrsp->list[ index ].gameInfo.fixed.icon;
	
	GXS_LoadOBJPltt( iconp->palette, (u32)( MB_ICON_PALETTE_SIZE * view_no ), MB_ICON_PALETTE_SIZE );
	MI_CpuCopyFast(  iconp->data,    (void *)((u32)objVramBuff + 0x20 + MB_ICON_DATA_SIZE * view_no), MB_ICON_DATA_SIZE );
	G2_SetOBJAttr( (GXOamAttr*)&oamBakS[ view_no ],					// OAM pointer
					8 * (PLIST_X + 3),								// X position
					8 *  PLIST_Y + (view_no * 32),					// Y position
					0,												// Priority
					GX_OAM_MODE_NORMAL,								// Bitmap mode
					FALSE,											// mosaic off
					GX_OAM_EFFECT_NONE,								// affine off
					GX_OAM_SHAPE_32x32,								// 16x16 size
					GX_OAM_COLOR_16,								// 16 color
					1 + view_no * 16,								// charactor
					view_no,										// palette
					0);												// affine
}

// �A�C�R��OBJ�̃N���A
static void ClearIconOBJ( u32 index )
{
	u16 view_no		= (u16)( index & 0x03 );
	u16 *oamp		= (u16 *)&oamBakS[ view_no ];
	
	*oamp++  = 192;													// Y���W����ʊO�ɁB�i�I�[��0�N���A�ł̓_���j
	*oamp++  = 0;
	*oamp++  = 0;
	*oamp    = 0;
}


// ���b�Z�[�W�\���̃Z�b�g
static void SetDispMessage( u16 color, const u8 *str )
{
	msgStat.vcount	 = MSG_VIEW_COUNT;
	msgStat.color	 = color;
	msgStat.str		 = str;
}


// ���b�Z�[�W�\��
static BOOL DispMessage(void)
{
	if( msgStat.vcount == MSG_VIEW_COUNT ) {
		msgStat.handle = DrawStringSJIS( 4, 21, msgStat.color, msgStat.str );
	}
	
	if ( --msgStat.vcount == 0 ) {
		ClearStringSJIS_handle( msgStat.handle );
		return TRUE;
	}
	return FALSE;
}


// ���b�Z�[�W�\���̃Z�b�g�i�_�ŕ\���j
static void SetBlinkMessage( u16 color, const u8 *str )
{
	msgStat.vcount	 = MSG_BLINK_COUNT;
	msgStat.flag	 = 1;
	msgStat.color	 = color;
	msgStat.str		 = str;
}


// ���b�Z�[�W�\��
static void DispBlinkMessage( void )
{
	if( ( msgStat.vcount == MSG_BLINK_COUNT ) && msgStat.flag ) {
		msgStat.handle = DrawStringSJIS( 4, 21, msgStat.color, msgStat.str );
	}
	
	if ( --msgStat.vcount == 0 ) {
		ClearStringSJIS_handle( msgStat.handle );
		msgStat.flag 	^= 0x01;
		msgStat.vcount	 = MSG_BLINK_COUNT;
	}
}


//=============================================================================
// Nintendo���S�\��
//=============================================================================

// Nintendo���S�\���̏�����
static void InitDispNintendoLogo( void )
{
	int i;
	u16 *palettep = (u16 *)(HW_OBJ_PLTT + 0x20 * 15 ) + 1;		// �p���b�g15�̃J���[1�ɁB
	*palettep++ = 0x7fff;
	*palettep   = 0x1111;
	
	// TOP��ʂ̑S�\���N���A
	SetTargetScreenSJIS( TOP_SCREEN );
	ClearAllStringSJIS();
	SetTargetScreenSJIS( BOTTOM_SCREEN );
	
	// ��ʏ�Ɏ�M�����Q�[����Nintendo���S��\������B
#ifdef NINLOGO_LOAD_1D_CHAR
	// 1D�}�b�s���O�ł̃��[�h���\��
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
	SYSM_LoadNintendoLogo1D( (u16 *)GetRomHeaderAddr()->nintendo_logo, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
//	SYSM_LoadNintendoLogo1D( (u16 *)SYSROM9_NINLOGO_ADR, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
	
	for ( i = 0; i < 3; i++ ) {
		G2_SetOBJAttr( &oamBakM[ 10 + i ],						// OAM pointer
					72 + 32 * i,								// X position
					88,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_32x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x2 + i * 4,								// charactor
					15,											// palette
					0);											// affine
	}
	{
		G2_SetOBJAttr( &oamBakM[ 13 ],							// OAM pointer
					72 + 32 * 3,								// X position
					88,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_8x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x2 + 3 * 4,								// charactor
					15,											// palette
					0);											// affine
	}
	for ( i = 0; i < 3; i++ ) {
		G2_SetOBJAttr( &oamBakM[ 14 + i ],						// OAM pointer
					72 + 32 * i,								// X position
					96,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_32x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x0f + i * 4,								// charactor
					15,											// palette
					0);											// affine
	}
	{
		G2_SetOBJAttr( &oamBakM[ 17 ],							// OAM pointer
					72 + 32 * 3,								// X position
					96,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_8x8,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0xf + 3 * 4,								// charactor
					15,											// palette
					0);											// affine
	}
#else
	// 2D�}�b�s���O�ł̃��[�h���\��
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_2D );
	SYSM_LoadNintendoLogo2D( (u16 *)GetRomHeaderAddr()->nintendo_logo, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
//	SYSM_LoadNintendoLogo2D( (u16 *)SYSROM9_NINLOGO_ADR, (u16 *)( HW_OBJ_VRAM + 0x40 ), 1, ninLogoBuff );
	
	for ( i = 0; i < 4; i++ ) {
		G2_SetOBJAttr( &oamBakM[11 + i],						// OAM pointer
					72 + 32 * i,								// X position
					88,											// Y position
					0,											// Priority
					GX_OAM_MODE_NORMAL,							// Bitmap mode
					FALSE,										// mosaic off
					GX_OAM_EFFECT_NONE,							// affine off
					GX_OAM_SHAPE_32x16,							// 16x16 size
					GX_OAM_COLOR_16,							// 16 color
					0x2 + i * 4,								// charactor
					15,											// palette
					0);											// affine
	}
#endif
	
	loop_count = 120;
}


// Nintendo���S�\��
static BOOL DispNintendoLogo( void )
{
	if( --loop_count == 0 ) {
		{
			int i;
			for ( i = 0; i < 4; i++ ) {
				G2_SetOBJPosition( &oamBakM[ 11 + i ], 0, 192 );
			}
		}
		return TRUE;
	}
	return FALSE;
}


//=============================================================================
// ���̑��T�u���[�`��
//=============================================================================

// �����̃��[�U�[����NITRO�ݒ�f�[�^����ǂݏo���ăZ�b�g
static void SetMyUserInfo( MBUserInfo *my )
{
	NvNickname *nickname = NCD_GetNickname();
	
	MI_CpuCopy16( nickname->str, my->name, (u32)( nickname->length * 2 ) );
	my->nameLength		= nickname->length;
	my->playerNo		= 0;										// �Ƃ肠�����O�ɂ���B
	my->favoriteColor	= NCD_GetFavoriteColor();
}


// ���݃X�L�������b�N�����ǂ������擾����B
static BOOL IsScanLock(void)
{
	return mbrsp->nowScanTargetFlag ? TRUE : FALSE;
}


//=============================================================================
// ���荞�݃��[�`��
//=============================================================================

// V�u�����N���荞��
static void VBlankIntr_WDL(void)
{
	// OAM, BG-VRAM�̍X�V
	DC_FlushRange (oamBakM,		sizeof(oamBakM));
	DC_FlushRange (oamBakS,		sizeof(oamBakS));
	DC_FlushRange (bgBakM,		sizeof(bgBakM));
	DC_FlushRange (bgBakS,		sizeof(bgBakS));
	DC_FlushRange (objVramBuff,	sizeof(objVramBuff));
	MI_CpuCopyFast(oamBakM,		(void*)HW_OAM,					 sizeof(oamBakM));
	MI_CpuCopyFast(oamBakS,		(void*)HW_DB_OAM,				 sizeof(oamBakS));
	MI_CpuCopyFast(bgBakM,		(void*)(HW_BG_VRAM + 0xf000),    sizeof(bgBakM));
	MI_CpuCopyFast(bgBakS,		(void*)(HW_DB_BG_VRAM + 0xf000), sizeof(bgBakS));
	MI_CpuCopyFast(objVramBuff,	(void*)HW_DB_OBJ_VRAM,			 sizeof(objVramBuff));
	//---- ���荞�݃`�F�b�N�t���O
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


//=============================================================================
// �v���O�����V�[�P���X�L���[����
//=============================================================================

// �L���[������
static void InitPrgNextSeqQue( void )
{
	MI_CpuClear16( &nextSeq, sizeof(nextSeq) );
}


// ����PrgSeq���L���[�ɃZ�b�g����B
static BOOL SetPrgNextSeqQue( PrgSeq seq )
{
	BOOL preIRQ = OS_DisableIrq();
	BOOL retval = FALSE;
	
	if( nextSeq.num != PRG_NEXT_SEQ_QUE_NUM ) {
		nextSeq.seq[ nextSeq.bottom ] =seq;
		nextSeq.num++;
		nextSeq.bottom++;
		if( nextSeq.bottom == PRG_NEXT_SEQ_QUE_NUM ) {
			nextSeq.bottom = 0;
		}
		retval = TRUE;
	}
	(void)OS_RestoreIrq( preIRQ );
	return retval;
}


// ����PrgSeq���L���[������o��
static BOOL GetPrgNextSeqQue( PrgSeq *seqp )
{
	BOOL preIRQ = OS_DisableIrq();
	BOOL retval = FALSE;
	
	if( ( !nextSeq.lock ) && ( nextSeq.num != 0 ) ) {				// �L���[�����b�N����Ă��炸�A����Seq�����݂���Ȃ�A�L���[������o���B
		*seqp = nextSeq.seq[ nextSeq.top ];
		nextSeq.num--;
		nextSeq.top++;
		if( nextSeq.top == PRG_NEXT_SEQ_QUE_NUM ) {
			nextSeq.top = 0;
		}
		retval = TRUE;
	}
	(void)OS_RestoreIrq( preIRQ );
	return retval;
}


// �L���[�����b�N����B�i���b�N����GetPrgNextSeqQue�Ŏ�Seq�����o���Ȃ��Ȃ�܂��B�j
static void LockPrgNextSeqQue(void)
{
	nextSeq.lock = TRUE;
}


// �L���[�̃��b�N�����B
static void UnlockPrgNextSeqQue(void)
{
	nextSeq.lock = FALSE;
}

// �L���[���󂩁H
static BOOL IsEmptyPrgNextSeqQue(void)
{
	return nextSeq.num ? FALSE : TRUE;
}



static const u32 searchCode[] = {
	0xe59f1028,
	0xe59f2028,
	0xe1d200b0,
	0xe2100001,
	0x1afffffc,
	0xe1d100b0,
	0xe3500006,
	0x0afffff9,
	0xe1d100b0,
	0xe3500005,
	0x0afffff6,
	0xe12fff1e,
	0x04808214,
	0x0480819c,
};


void SearchPatchAddress( void )
{
	MBDownloadFileInfo *dlfinfo = (MBDownloadFileInfo *)MB_DOWNLOAD_FILEINFO_ADDRESS;
	BOOL find = FALSE;
	u32 *tgtp = (u32 *)MB_ARM7_STATIC_RECV_BUFFER;
	u32 size  = dlfinfo->seg[ 2 ].size >> 2;
	
	while( size-- ) {
		if( *tgtp++ == searchCode[0] ) {
			u32 *srcp   = (u32 *)&searchCode[ 1 ];
			u32 *checkp = tgtp ;
			int i = sizeof( searchCode ) / sizeof(u32) - 1;
			while( i-- ) { 
				if( *srcp++ != *checkp++ ) break;
			}
			if( i < 0 ) {
				u32 addr = (u32)tgtp - 0x04 + 0x18;
				OS_TPrintf("Found! -> 0x%08x\n", addr );
				find = TRUE;
			}
		}
	}
	if( !find ) {
		OS_TPrintf("Not found.\n");
	}
}

