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
#include "sound.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define WIRELESS_FIRM_LOADING 1

#if( WIRELESS_FIRM_LOADING == 1 )
#include "loadWlanFirm.h"
#endif

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );
static void deleteTmp();

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static TitleProperty s_titleList[ LAUNCHER_TITLE_LIST_NUM ];

static u64 strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread strmThread;

static StreamInfo strm; // stream info

// const data------------------------------------------------------------------

const char filename[] = "data/fanfare.32.wav";

//#define DEBUG_LAUNCHER_DUMP
#ifdef DEBUG_LAUNCHER_DUMP
// �f�o�O�p�BSD��0x02ffc000����0x02ffe000�܂�dump.dat�Ƃ����_���v��f��
static void debugWriteToSD( void )
{
    FSFile dest;
    FS_InitFile( &dest );
    (void)FS_CreateFile("sdmc:/dump.dat", FS_PERMIT_W | FS_PERMIT_R);
    if ( !FS_OpenFileEx( &dest, "sdmc:/dump.dat", FS_FILEMODE_W ) ) return;
    FS_WriteFile( &dest, (void *)0x02ffc000, 0x2000 );
    if ( !FS_CloseFile( &dest ) ) return;
    OS_TPrintf( "debugWriteToSD:ok\n");
}
#endif

// ���C��
void TwlMain( void )
{
    enum {
        LOGODEMO_INIT = 0,
        LOGODEMO = 1,
        LAUNCHER_INIT = 2,
        LAUNCHER = 3,
        LOAD_START = 4,
        LOADING = 5,
        AUTHENTICATE = 6,
        BOOT = 7,
        STOP = 8
    };
    u32 state = LOGODEMO_INIT;
    TitleProperty *pBootTitle = NULL;
    OSTick start, end = 0;
    BOOL direct_boot = FALSE;

#ifdef DEBUG_LAUNCHER_DUMP
    // you should comment out to clear GX/G2/DMA/TM/PAD register in reboot.c to retreive valid boot time
    STD_TSPrintf((char*)0x02FFCFC0, "\nLauncher Boot Time: %lld usec\n", OS_TicksToMicroSeconds(reg_OS_TM3CNT_L * (1024/64)));
    STD_TSPrintf((char*)0x02FFCFF0, "HOTSTART(0x%08x): %02x\n", HW_NAND_FIRM_HOTSTART_FLAG, *(u8 *)HW_NAND_FIRM_HOTSTART_FLAG);
#endif
    // �V�X�e�����j���[������----------
    SYSM_Init( Alloc, Free );                       // OS_Init�̑O�ŃR�[������K�v����B
    OS_Init();
    SYSM_SetArena();                                // OS_Init�̌�ŃR�[������K�v����B

    // OS������------------------------
    OS_InitTick();
    PM_Init();

    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    SYSM_InitPXI();                                 // ���荞�݋���ɃR�[������K�v����B
	while ( ! PXI_IsCallbackReady( PXI_FIFO_TAG_HOTSW, PXI_PROC_ARM7 ) )
    {
    }
    
    FS_Init( FS_DMA_NOT_USE );

#ifdef DEBUG_LAUNCHER_DUMP
    // debug
    debugWriteToSD();
#endif

    GX_Init();
    PM_Init();
    TP_Init();
    RTC_Init();
    SND_Init();// sound init

    OS_TPrintf( "SYSM_work size = 0x%x\n", sizeof(SYSM_work) );

    // ���荞�݋���--------------------
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // �V�X�e���̏�����----------------
    InitAllocator();                                            // ��SYSM_Init�ȊO��SYSM���C�u�����֐����ĂԑO��
                                                                //   Alloc, Free�œo�^�����������A���P�[�^�����������Ă��������B


    // �e��p�����[�^�̎擾------------
	pBootTitle = SYSM_ReadParameters();   		               // �{�̐ݒ�f�[�^�A���Z�b�g�p�����[�^�̃��[�h�A�����p�I�[�g�N���J�[�h����A�ʎY���C���p�L�[�V���[�g�J�b�g�N�����蓙�̃��[�h
	
	if( SYSMi_GetWork()->flags.common.isFatalError ) {
		// FATAL�G���[����
	}
	if( SYSMi_GetWork()->flags.common.isInitialSettings ) {
		// ����N���V�[�P���X����
	}

    (void)SYSM_GetCardTitleList( s_titleList );                 // �J�[�h�A�v�����X�g�̎擾�i�J�[�h�A�v����s_titleList[0]�Ɋi�[�����j

    // bootType��LAUNCHER_BOOTTYPE_TEMP�łȂ��ꍇ�Atmp�t�H���_���̃f�[�^������
    if( !pBootTitle || pBootTitle->flags.bootType != LAUNCHER_BOOTTYPE_TEMP )
    {
        deleteTmp();
    }

    // �u�_�C���N�g�u�[�g�łȂ��v�Ȃ�
    if( !pBootTitle ) {
        // �A�v���ԃp�����^���N���A
        // TODO:���炩����NTR�J�[�h�̃Z�L���A�̈��ޔ������ɒ���0x2000000���烍�[�h���Ă���ꍇ���e�͂Ȃ������̂Œ���
        MI_CpuClearFast((void *)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE);

        // NAND & �J�[�h�A�v�����X�g�擾
        (void)SYSM_GetNandTitleList( s_titleList, LAUNCHER_TITLE_LIST_NUM );    // NAND�A�v�����X�g�̎擾�i�����A�v����s_titleList[1]����i�[�����j
    }

    // �u�_�C���N�g�u�[�g�łȂ��v��������
    // �u�_�C���N�g�u�[�g�����A���S�f���\���v�̎��A�e�탊�\�[�X�̃��[�h------------
    if( !pBootTitle ||
        ( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
//      FS_ReadContentFile( ContentID );                        // �^�C�g�������\�[�X�t�@�C���̃��[�h
//      FS_ReadSharedContentFile( ContentID );                  // ���L�R���e���g�t�@�C���̃��[�h
    }

    // �J�n�X�e�[�g�̔���--------------

    if( pBootTitle ) {
        // �_�C���N�g�u�[�g�Ȃ�A���S�A�����`���[���΂��ă��[�h�J�n
        if( pBootTitle->flags.isLogoSkip ) {
            state = LOAD_START;
        }else {
            state = LOGODEMO_INIT;
        }
        direct_boot = TRUE;
    }else if( SYSM_IsLogoDemoSkip() ) {
        // ���S�f���X�L�b�v���w�肳��Ă�����A�����`���[�N��
        state = LAUNCHER_INIT;
    }else {
        // �����Ȃ��Ȃ�A���S�f���N��
        state = LOGODEMO_INIT;
    }

// �����`���[���Ε\�����Ȃ��o�[�W����
#ifdef DO_NOT_SHOW_LAUNCHER
	if(direct_boot == FALSE)
	{
		state = STOP;
	}else
	{
		state = LOAD_START;
	}
#endif

    // �`�����l�������b�N����
    SND_LockChannel((1 << L_CHANNEL) | (1 << R_CHANNEL), 0);

    /* �X�g���[���X���b�h�̋N�� */
    OS_CreateThread(&strmThread,
                    StrmThread,
                    NULL,
                    strmThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, STREAM_THREAD_PRIO);
    OS_WakeupThreadDirect(&strmThread);


    // �����t�@�[���E�F�A�𖳐����W���[���Ƀ_�E�����[�h����B
#if( WIRELESS_FIRM_LOADING == 1 )
    if( FALSE == InstallWlanFirmware() ) {
        OS_TPrintf( "ERROR: Wireless firmware download failed!\n" );
    }
#endif


    // ���C�����[�v--------------------
    while( 1 ) {
        OS_WaitIrq(1, OS_IE_V_BLANK);                           // V�u�����N���荞�ݑ҂�

        // �`�q�l�V�R�}���h������M
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        ReadKeyPad();                                           // �L�[���͂̎擾
        ReadTP();                                               // TP���͂̎擾

        switch( state ) {
        case LOGODEMO_INIT:
            LogoInit();
            // ���炷�e�X�g
            FS_InitFile(&strm.file);
            strm.isPlay = FALSE;
            PlayStream(&strm, filename);

            state = LOGODEMO;
            break;
        case LOGODEMO:
            if( LogoMain() ) {
                if( !direct_boot ) {
                    state = LAUNCHER_INIT;
                }else {
                    state = LOAD_START;
                }
            }
            break;
        case LAUNCHER_INIT:
            LauncherInit( s_titleList );
            state = LAUNCHER;
            break;
        case LAUNCHER:
            pBootTitle = LauncherMain( s_titleList );
            if( pBootTitle ) {
                state = LOAD_START;
            }
            break;
        case LOAD_START:
            SYSM_StartLoadTitle( pBootTitle );
            state = LOADING;

            start = OS_GetTick();

            break;
        case LOADING:
            if( SYSM_IsLoadTitleFinished() ) {
				SYSM_StartAuthenticateTitle( pBootTitle );
                state = AUTHENTICATE;
            }
			if( !direct_boot )
			{
				(void)LauncherFadeout( s_titleList ); // �_�C���N�g�u�[�g�łȂ��Ƃ��̓t�F�[�h�A�E�g���s��
			}
            if( ( end == 0 ) &&
                SYSM_IsLoadTitleFinished() ) {
                end = OS_GetTick();
                OS_TPrintf( "Load Time : %dms\n", OS_TicksToMilliSeconds( end - start ) );
            }
            break;
        case AUTHENTICATE:
            if( ( direct_boot || ( !direct_boot && LauncherFadeout( s_titleList ) ) ) &&
                SYSM_IsAuthenticateTitleFinished() )
            {
	            switch ( SYSM_TryToBootTitle( pBootTitle ) ) {   // �A�v���F�،��ʎ擾or�u�[�g   �������Fnever return
	            case AUTH_RESULT_TITLE_LOAD_FAILED:
	            case AUTH_RESULT_TITLE_POINTER_ERROR:
	            case AUTH_RESULT_AUTHENTICATE_FAILED:
	            case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
	                state = STOP;
	                // [TODO:]�N���A�����ق����ǂ��f�[�^�i���Ȃǁj������Ώ���
	                break;
	            }
            }
            break;
        case STOP:                                              // ��~
            break;
        }

        // �J�[�h�A�v�����X�g�̎擾�i�X���b�h�Ő����J�[�h�}����ʒm�������̂����C�����[�v�Ŏ擾�j
        (void)SYSM_GetCardTitleList( s_titleList );

        // �R�}���h�t���b�V��
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
    }
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);                              // V�u�����N�����`�F�b�N�̃Z�b�g
}

// ============================================================================
// �f�B���N�g������
// ============================================================================

// nand��tmp�f�B���N�g���̒��g������
static void deleteTmp()
{
	if( FS_DeleteFile( OS_TMP_APP_PATH ) )
	{
		OS_TPrintf( "deleteTmp: deleted File '%s' \n", OS_TMP_APP_PATH );
	}else
	{
		FSResult res = FS_GetArchiveResultCode("nand");
		if( FS_RESULT_SUCCESS == res )
		{
			OS_TPrintf( "deleteTmp: File '%s' not exists.\n", OS_TMP_APP_PATH );
		}else
		{
			OS_TPrintf( "deleteTmp: delete File '%s' failed. Error code = %d.\n", OS_TMP_APP_PATH, res );
		}
	}
}
