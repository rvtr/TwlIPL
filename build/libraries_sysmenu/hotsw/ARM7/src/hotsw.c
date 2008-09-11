/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     hotsw.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#include    <twl.h>
#include    <twl/os/common/format_rom.h>
#include    <twl/mcu.h>
#include    <nitro/card/types.h>
#include    <firm/os/common/system.h>
#include    <sysmenu.h>
#include    <hotswTypes.h>
#include    <blowfish.h>
#include    <dsCardCommon.h>
#include    <romEmulation.h>
#include    <customNDma.h>
#include    <../build/libraries/mb/common/include/mb_fileinfo.h>

//#define HOTSW_DISABLE_FORCE_CARD_OFF

// �J�[�h�d��ON����ROM�w�b�_���[�h�܂ł̊��ԂɃX���[�v�ɓ��鎞�̃����Z�O�΍􂵂Ȃ��ꍇ
//#define HOWSW_ENABLE_DEEP_SLEEP_WHILE_INSERT_CARD

// define -------------------------------------------------------------------
#define     CHATTERING_COUNTER                  0x1988      // 100ms�� (0x1988 * 15.3us = 100000us)
//#define   CHATTERING_COUNTER                  0x264c      // 150ms�� (0x264c * 15.3us = 150001us)
//#define   CHATTERING_COUNTER                  0x3310      // 200ms�� (0x3310 * 15.3us = 200001us)
//#define   CHATTERING_COUNTER                  0x4c98      // 300ms�� (0x4c98 * 15.3us = 300002us)

#define     COUNTER_A                           0x264c      // 150ms�� (0x264c * 15.3us = 150001us)

#define     CARD_EXIST_CHECK_POLLING_TIME       100
#define     CARD_INSERT_CHECK_INTERVAL          5

#define     UNDEF_CODE                          0xe7ffdeff  // ����`�R�[�h
#define     ENCRYPT_DEF_SIZE                    0x800       // 2KB  �� ARM9�풓���W���[���擪2KB

#define     DIGEST_HASH_BLOCK_SIZE_SHA1         (512/8)

#define     SLOT_B_LOCK_BUF                     HW_CTRDG_LOCK_BUF

#define     HOTSW_DSTV_GAME_CODE                'JSNU'

#ifdef SDK_ARM9
#define     reg_HOTSW_EXMEMCNT                  reg_MI_EXMEMCNT
#define     HOTSW_EXMEMCNT_SELB_MASK            REG_MI_EXMEMCNT_MPB_MASK
#define     HOTSW_EXMEMCNT_SELB_SHIFT           REG_MI_EXMEMCNT_MPB_SHIFT
#else  // SDK_ARM7
#define     reg_HOTSW_EXMEMCNT                  reg_MI_EXMEMCNT_L
#define     HOTSW_EXMEMCNT_SELB_MASK            0x0400
#define     HOTSW_EXMEMCNT_SELB_SHIFT           10
#endif

// enum ---------------------------------------------------------------------
typedef enum HotSwCallBackType{
    HOTSW_CHANGE_GAMEMODE = 0,
    HOTSW_CARD_INSERT,
    HOTSW_CARD_PULLOUT
} HotSwCallBackType;

// Function prototype -------------------------------------------------------
static u32 GetMcSlotMode(void);
static void SetMcSlotMode(u32 mode);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCardDet(void);
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);
#ifndef USE_NEW_DMA
static void InterruptCallbackCardData(void);
#endif

static void LockHotSwRsc(OSLockWord* word);
static void UnlockHotSwRsc(OSLockWord* word);

static void HotSwThread(void *arg);
static void MonitorThread(void *arg);

static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static BOOL isTwlModeLoad(void);
static HotSwState ReadSecureModeCardData(void);
static void ClearCardFlgs(void);

static void FinalizeHotSw(HotSwCardState state);
static void ForceNitroModeToFinalize(void);
static void ForceNormalModeToFinalize(void);
static BOOL ChangeGameMode(void);
static void ClearCardIrq(void);
static void ClearAllCardRegister(void);

static HotSwState LoadCardData(void);
static void RegisterRomEmuInfo(void);
static void GenVA_VB_VD(void);

static HotSwState ReadImageReturnErrorCode(void* dest, s32 offset, s32 length, void* arg);
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg);

static HotSwState LoadBannerData(void);
static HotSwState CheckCardAuthCode(void);

#ifndef USE_WRAM_LOAD
static HotSwState DecryptObjectFile(void);
static HotSwState LoadStaticModule(void);

static HotSwState CheckStaticModuleHash(void);
static BOOL CheckArm7HashValue(void);
static BOOL CheckArm9HashValue(void);
static BOOL CheckExtArm7HashValue(void);
static BOOL CheckExtArm9HashValue(void);
#else
static void ReadCardData(u32 src, u32 dest, u32 size);
static void SendPxiMessage(HotSwCallBackType type);
#endif

static void DebugPrintErrorMessage(HotSwState state);

HotSwState HOTSWi_RefreshBadBlock(u32 romMode);

static void CheckCardInsert(BOOL cardExist);
static void CheckCardPullOut(BOOL cardExist);
static void SendInsertMessage(void);

static void PulledOutSequence(void);

// Static Values ------------------------------------------------------------
#include <twl/ltdwram_begin.h>

static char                 encrypt_object_key[] ATTRIBUTE_ALIGN(4) = "encryObj";

static u16                  s_RscLockID;
static u16                  s_CardLockID;
static u16					s_PollingLockID;
static u16                  s_bondingOp;

static u32                  s_BootSegBufSize, s_SecureSegBufSize, s_Secure2SegBufSize;

static u32					s_gameID;

static BootSegmentData      *s_pBootSegBuffer;
static u32                  *s_pSecureSegBuffer;
static u32                  *s_pSecure2SegBuffer;

static CardBootData         s_cbData ATTRIBUTE_ALIGN(4);
static SYSMRomEmuInfo       s_romEmuInfo;
static BOOL                 s_isRomEmu;
static BOOL                 s_debuggerFlg;

static BOOL                 s_isPulledOut = TRUE;
static BOOL                 s_pollingThreadSleepFlg = FALSE;

static volatile BOOL		s_isBusyMonitorThread;

// HMACSHA1�̌�
static u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = {
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24
};

static CardSecureModeFunction s_funcTable[] = {
    // Game Card
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // RomEmulation
    { ReadIDSecure_ROMEMU, ReadSegSecure_ROMEMU, SwitchONPNGSecure_ROMEMU, ChangeModeSecure_ROMEMU }
};

// Global Values ------------------------------------------------------------
BLOWFISH_CTX                HotSwBlowfishInitTableBufDS;
CardThreadData              HotSwThreadData;

#include <twl/ltdwram_end.h>

// ===========================================================================
//  Function Describe
// ===========================================================================

/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init

  Description:
 *---------------------------------------------------------------------------*/
void HOTSW_Init(u32 threadPrio)
{
    SYSM_work* sw = SYSMi_GetWork();

    OS_InitTick();
    OS_InitThread();

#ifndef INITIAL_KEYTABLE_PRELOAD
    // ��������ɑ��̗p�r��WRAM_0���g�p�ł���悤�Ƀ��[�J���o�b�t�@�փR�s�[���Ă���
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &HotSwBlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
#endif

    // PXI������
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);

#ifdef USE_WRAM_LOAD
    while(!PXI_IsCallbackReady(PXI_FIFO_TAG_HOTSW, PXI_PROC_ARM9))
    {
        // do nothing
    }
#endif

    // ���荞�݃}�X�N�̐ݒ�
    SetInterrupt();

    // ���荞�݂̗L����
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // �`���b�^�����O�J�E���^�̒l��ݒ�
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~REG_MI_MC1_CC_MASK) |
                       (CHATTERING_COUNTER << REG_MI_MC1_CC_SHIFT));

    // Counter-A�̒l��ݒ�
    reg_MI_MC2 = COUNTER_A;

    // Bonding Option�̎擾
    s_bondingOp = SCFG_REG_GetBondingOption();

    // �\���̂̏�����
    MI_CpuClear8(&s_cbData, sizeof(CardBootData));

    // �J�[�h�X���b�h�p�\���̂̏�����
    MI_CpuClear8(&HotSwThreadData, sizeof(CardThreadData));

    // HotSw���\�[�X�̔r������pLock ID�̎擾(�J�����Ȃ��Ŏ���������)
    {
        s32 tempLockID;
        // ARM9�Ɣr������p�̃��b�NID�̎擾
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            // do nothing
        }
        s_RscLockID = (u16)tempLockID;

        // �J�[�h�A�N�Z�X�p�̃��b�NID�̎擾
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            // do nothing
        }
        s_CardLockID = (u16)tempLockID;

        // [Debug]
        // �|�[�����O�X���b�h�p�̃��b�NID�̎擾
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            // do nothing
        }
        s_PollingLockID = (u16)tempLockID;
    }

    // �J�[�h�̏�ԊĎ��p�X���b�h�̐��� ( DS�e���r�΍� )
    OS_CreateThread(&HotSwThreadData.monitorThread,
                    MonitorThread,
                    NULL,
                    HotSwThreadData.monitorStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // �J�[�h�f�[�^���[�h�p�X���b�h�̐��� ���J�[�h��ԊĎ��X���b�h�ƗD��x���������ǁA��ɍ쐬�����������̃X���b�h���D�悳���
    OS_CreateThread(&HotSwThreadData.hotswThread,
                    HotSwThread,
                    NULL,
                    HotSwThreadData.hotswStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // ���b�Z�[�W�L���[�̏�����
    OS_InitMessageQueue( &HotSwThreadData.hotswQueue,            &HotSwThreadData.hotswMsgBuffer[0],         HOTSW_MSG_BUFFER_NUM );
    OS_InitMessageQueue( &HotSwThreadData.hotswDmaQueue,         &HotSwThreadData.hotswDmaMsgBuffer[0],      HOTSW_DMA_MSG_NUM );
    OS_InitMessageQueue( &HotSwThreadData.hotswPollingCtrlQueue, &HotSwThreadData.hotswPollingCtrlBuffer[0], HOTSW_POLLING_CTRL_BUFFER_NUM );

    // �X���b�h�N��
    OS_WakeupThreadDirect(&HotSwThreadData.hotswThread);
    OS_WakeupThreadDirect(&HotSwThreadData.monitorThread);

    // �o�b�t�@�̐ݒ�
    HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_APP_ROM_HEADER_SIZE );
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE1, (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE2, (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );

    // �J�[�h���}�����Ă�������X���b�h���N������
    if(HOTSW_IsCardExist()){
        // ���b�Z�[�W���M
        OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

        // ���b�Z�[�W�C���f�b�N�X���C���N�������g
        HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
    }
    else{
        sw->flags.hotsw.is1stCardChecked  = TRUE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_CopyInitialKeyTable

  Description:  DS�݊�Blowfish�e�[�u����WRAM�o�R�Ń��[�J���ɃR�s�[����

  �����̊֐���HOTSW_Init���Ă΂��O�ɌĂ�ŉ������B
 *---------------------------------------------------------------------------*/
#ifdef INITIAL_KEYTABLE_PRELOAD
void HOTSW_CopyInitialKeyTable(void)
{
    // ARM9��Key Table�̗p�ӂ��o����܂Ń|�[�����O
    while( !SYSMi_GetWork()->flags.hotsw.isKeyTableLoadReady ){
        OS_SpinWait( 0x400 );
    }

    // ��������ɑ��̗p�r��WRAM_0���g�p�ł���悤�Ƀ��[�J���o�b�t�@�փR�s�[���Ă���
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &HotSwBlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
}
#endif


/*---------------------------------------------------------------------------*
  Name:         LoadCardData

  Description:  �J�[�h����f�[�^�����[�h����

  ��BootSegmentBuffer SecureSegmentBuffer�̐ݒ���s���Ă���
  ���̊֐����Ă�ł��������B
 *---------------------------------------------------------------------------*/
static HotSwState LoadCardData(void)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 romMode = HOTSW_ROM_MODE_NULL;
    
    // �J�[�h�̃��b�N
    CARD_LockRom(s_CardLockID);

    // �J�[�h�d�����Z�b�g
    McPowerOff();
#ifndef HOWSW_ENABLE_DEEP_SLEEP_WHILE_INSERT_CARD
    MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, FALSE );
#endif
    McPowerOn();

    // �o�b�t�@��ݒ�
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;
    s_cbData.pSecure2SegBuf= s_pSecure2SegBuffer;

    // ���[�h�����J�n
    if(HOTSW_IsCardAccessible()){
        s_cbData.modeType = HOTSW_MODE1;

        // �J�[�h����Key Table�����[�h����
        state  = LoadTable();
        retval = (retval == HOTSW_SUCCESS) ? state : retval;

        // ---------------------- Normal Mode ----------------------
        romMode = HOTSW_ROM_MODE_NORMAL;

        // �J�[�hID�ǂݍ���
        state  = ReadIDNormal(&s_cbData);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;

        // �J�[�h�^�C�v�𔻕ʂ����āA�g���֐���ؑւ��� ID�̍ŏ�ʃr�b�g��1�Ȃ�3DM
        s_cbData.cardType = (s_cbData.id_nml & HOTSW_ROMID_1TROM_MASK) ? DS_CARD_TYPE_2 : DS_CARD_TYPE_1;

        {
            SYSMRomEmuInfo *romEmuInfo = (void *)&s_romEmuInfo;
            BOOL enableDeepSleep = TRUE;

            // �o�i�[���[�h���������āA�t���O�������I���܂�ARM9�Ɣr�����䂷��
            LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // Boot Segment�ǂݍ���
            state  = ReadBootSegNormal(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // �����Z�O�����ݎ��̓J�[�h�o�X�d�����f�B�[�v�X���[�v�iPFM�j�ɓ���Ȃ�
            // �i�X���[�v���V���b�g�_�E���΍�B�J�[�h���[�h���ɃX���[�v���Ă��ǂ��悤�ɑ��ڂɐݒ�j
            if ( *(u32*)(s_cbData.pBootSegBuf->rh.s.game_code) == HOTSW_DSTV_GAME_CODE )
            {
                enableDeepSleep = FALSE;
            }
            MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, enableDeepSleep );

            // ARM9/7�ŕs�������������Ȃ��悤��Rom�G�~�����[�V������񃍁[�h�͏���̂�
            if ( ! SYSMi_GetWork()->flags.hotsw.is1stCardChecked )
            {
                // Rom�G�~�����[�V���������擾
                //�i���i�{�̂Ȃ�����ō\���̂��N���A���邾���j
                state  = ReadRomEmulationInfo(&s_romEmuInfo);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // �擾����Rom�G�~�����[�V���������r
                s_debuggerFlg = TRUE;
                if ( romEmuInfo->magic_code != SYSM_ROMEMU_INFO_MAGIC_CODE ){
                    s_debuggerFlg = FALSE;
                }
            }
            // ��ROM�G�~�����[�V��������ǂ܂Ȃ���΁A�f�o�b�K��ŃJ�[�h�A�N�Z�X���ł��Ȃ��Ȃ邽�߁A�����`���[���g���f�o�b�O�ł��Ȃ��Ȃ�B
            //   ����āAROM�G�~�����[�V������񃊁[�h�������f�o�b�K�r���h���ȊO�ɐ؂邱�Ƃ͂ł��Ȃ��B

            // �����Rom�G�~�����[�V���������g�p
            s_isRomEmu = FALSE;
            if(HOTSWi_IsRomEmulation()){
                s_isRomEmu = TRUE;
                HOTSW_PutString("Read Emulation ROM\n");
                s_cbData.cardType = ROM_EMULATION;
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
                HOTSW_TPrintf("SYSMi_GetWork()->gameCommondParam : 0x%08x\n", s_cbData.gameCommondParam);
            }
            else{
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
            }
            SYSMi_GetWork()->gameCommondParam = s_cbData.gameCommondParam;

            // CRC�`�F�b�N
            if( !UTL_CheckAppCRC16( &s_cbData.pBootSegBuf->rh.s ) ) {
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_CRC_CHECK_ERROR : retval;
            }

            if(s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL){
                s_cbData.twlFlg = TRUE;
            }

            // �A�v���W�����v�̃f�o�b�O����ROM�G�~�����[�V������񂾂��K�v�ȏꍇ
            if(SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly){
                SYSMi_GetWork()->flags.hotsw.isExistCard = TRUE;
                RegisterRomEmuInfo();
                // �r�����䂱���܂�
                UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                goto finalize;
            }
        }

        if( retval == HOTSW_SUCCESS ) {
            if(!s_cbData.twlFlg){
                if ( !s_cbData.pBootSegBuf->rh.s.exFlags.enable_nitro_whitelist_signature )
                // NTR�J�[�h�̏ꍇ��RomHeader�o�b�t�@��1�y�[�W�ڈȍ~���N���A���Ă����B
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_APP_ROM_HEADER_SIZE - PAGE_SIZE);
            }

            // Secure�R�}���h��PNG_ON�R�}���hetc�p�̃��C�e���V�����߂�(Latency1��Latency2�𑫂�)
            s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);

            // Key Table������
            MakeBlowfishTableDS(&s_cbData, 8);

            // �R�}���h�F�ؒl�E�R�}���h�J�E���^�����l�EPN�W�F�l���[�^�����l�̐���
            GenVA_VB_VD();

            // �Z�L���A���[�h�Ɉڍs
            state  = ChangeModeNormal(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ---------------------- Secure Mode ----------------------
            romMode = HOTSW_ROM_MODE_SECURE;

            // Secure���[�h��ID��SecureSegment��ǂݍ���
            state  = ReadSecureModeCardData();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ��TWL�J�[�h�Ή� ��U���Z�b�g��Secure2���[�h�Ɉڍs
            // SCFG
            if((s_cbData.isLoadTypeTwl = isTwlModeLoad()) == TRUE){
                HOTSW_PutString("Read Mode : TwlCard\n");

               // Mode2�Ɉڍs���鏀��
                s_cbData.modeType = HOTSW_MODE2;

                // ---------------------- Reset ----------------------
                McPowerOff();
                McPowerOn();

                // ---------------------- Normal Mode ----------------------
                // �擪1Page��������OK�B�f�[�^�͓ǂݎ̂ăo�b�t�@��
                state  = ReadBootSegNormal(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // Key Table������
                MakeBlowfishTableTWL(&s_cbData, 8, s_bondingOp);

                // �R�}���h�F�ؒl�E�R�}���h�J�E���^�����l�EPN�W�F�l���[�^�����l�̐���
                GenVA_VB_VD();

                // �Z�L���A�Q���[�h�Ɉڍs
                state  = ChangeModeNormal2(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // ---------------------- Secure2 Mode ----------------------
                // Secure2���[�h��ID��SecureSegment��ǂݍ���
                state  = ReadSecureModeCardData();
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
            }

            // �Q�[�����[�h�Ɉڍs
            state  = s_funcTable[s_isRomEmu].ChangeMode_S(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ---------------------- Game Mode ----------------------
            romMode = HOTSW_ROM_MODE_GAME;

            if(retval != HOTSW_SUCCESS || s_cbData.illegalCardFlg){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_ILLEGAL_CARD_ERROR : retval;

                // �r�����䂱���܂�
                UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                goto finalize;
            }

            // ID�ǂݍ���
            state  = ReadIDGame(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
            {
                u32 secure_ID = (s_cbData.modeType == HOTSW_MODE1) ? s_cbData.id_scr : s_cbData.id_scr2;
                if(secure_ID != s_cbData.id_gam){
                    retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;

                    // �r�����䂱���܂�
                    UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                    goto finalize;
                }
            }

            // �o�i�[�t�@�C���̓ǂݍ���
            state  = LoadBannerData();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // �r�����䂱���܂�(��CRC�`�F�b�N�܂łɃ~�X���Ȃ�������A�r�����䂱���܂�)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // �F�؃R�[�h�ǂݍ��݁����[�N�̈�ɃR�s�[
            state  = CheckCardAuthCode();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

#ifndef USE_WRAM_LOAD
            // �풓���W���[���c����w���ɓ]��
            state  = LoadStaticModule();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ARM9�풓���W���[���̐擪2KB�̈Í����̈�𕡍���
            (void)DecryptObjectFile();
#endif
        }
        else{
            // �r�����䂱���܂�(��CRC�`�F�b�N�܂łɃ~�X����������A�����ŊJ������)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            HOTSW_TPrintf("ng... Card Data Load Skip ( state : %d )\n", retval);
        }
    }
    else{
        state = HOTSW_PULLED_OUT_ERROR;
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }

finalize:
    if( retval == HOTSW_SUCCESS )
    {
        // �o�b�h�u���b�N��u��
        state = HOTSWi_RefreshBadBlock(romMode);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }

end:
    // �J�[�hDMA�I���m�F
#ifdef USE_NEW_DMA
    HOTSW_WaitNDmaCtrl(HOTSW_NDMA_NO);
#else
    HOTSW_WaitDmaCtrl(HOTSW_DMA_NO);
#endif

    // �J�[�h�A�N�Z�X�I���m�F
    HOTSW_WaitCardCtrl();

    // �J�[�h�̃��b�N�J��(�����b�NID�͊J�������Ɏ���������)
    CARD_UnlockRom(s_CardLockID);
    
    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         isTwlModeLoad

  Description:  �J�[�h�̃��[�h��TWL���[�h�ōs����DS���[�h�ōs���������߂�

  �� �ǂݍ��݃V�[�P���X�̏ڍׂ� docs/�Q�[���J�[�h/�J�[�h�ǂݍ��݃V�[�P���X�\.xls �Q��
 *---------------------------------------------------------------------------*/
static BOOL isTwlModeLoad(void)
{
    // TWL�J�[�h
    if(s_cbData.id_nml & HOTSW_ROMID_TWLROM_MASK){
        // NAND�A�v���̏ꍇ
        if(s_cbData.pBootSegBuf->rh.s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK){
            return FALSE;
        }
        else{
            // PlatformCode��Twl or Hybrid�̏ꍇ
            if(s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL){
                return TRUE;
            }
            else{
                return FALSE;
            }
        }
    }
    // DS�J�[�h
    else{
        // PlatformCode��Twl or Hybrid�̏ꍇ
        if(s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL){
            // ���i�p�{�� (�ŏI�I�ɂ͊J���p�{�̂�)
#ifdef HOTSW_FINAL_VERSION
            if(s_bondingOp == SCFG_OP_PRODUCT || !s_debuggerFlg)
#else
            if(s_bondingOp == SCFG_OP_PRODUCT)
#endif
            {
                s_cbData.illegalCardFlg = TRUE;
            }
        }

        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ReadSecureModeCardData

  Description:  �Z�L���A���[�h�̊e��ǂݍ��݂��s���֐�

  �� s_cbData.modeType�̐ݒ�͂��̊֐��̊O�ōs��
 *---------------------------------------------------------------------------*/
static HotSwState ReadSecureModeCardData(void)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;

    u32 secure_ID;

    // PNG�ݒ�
    state  = s_funcTable[s_isRomEmu].SetPNG_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // DS������������H�����l�ݒ� (���W�X�^�ݒ�)
    SetMCSCR();

    // ID�ǂݍ���
    state  = s_funcTable[s_isRomEmu].ReadID_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
    secure_ID = (s_cbData.modeType == HOTSW_MODE1) ? s_cbData.id_scr : s_cbData.id_scr2;
    if(s_cbData.id_nml != secure_ID){
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
    }

    if(retval == HOTSW_SUCCESS){
        // Secure�̈��Segment�ǂݍ���
        state  = s_funcTable[s_isRomEmu].ReadSegment_S(&s_cbData);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_RefreshBadBlock

  Description:  �m�[�}�����[�h�܂��̓Q�[�����[�h�Ńo�b�h�u���b�N��u��
 *---------------------------------------------------------------------------*/
HotSwState HOTSWi_RefreshBadBlock(u32 romMode)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;

    HotSwState (*pReadStatus)(CardBootData *cbd);
    HotSwState (*pRefreshBadBlock)(CardBootData *cbd);

    if ( ! ( romMode == HOTSW_ROM_MODE_NORMAL || romMode == HOTSW_ROM_MODE_GAME ) )
    {
        return HOTSW_SUCCESS;
    }

    if ( romMode == HOTSW_ROM_MODE_NORMAL )
    {
        pReadStatus = ReadStatusNormal;
        pRefreshBadBlock = RefreshBadBlockNormal;
    }
    else
    if ( romMode == HOTSW_ROM_MODE_GAME )
    {
        pReadStatus = ReadStatusGame;
        pRefreshBadBlock = RefreshBadBlockGame;
    }

    // �X�e�[�^�X�Ή�ROM�̂݃X�e�[�^�X�ǂݍ���
    if ( s_cbData.id_nml & HOTSW_ROMID_RFSSUP_MASK )
    {
        state = pReadStatus(&s_cbData);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
        // �v�����x���Ɋւ�炸�o�b�h�u���b�N��u���i���i�J�[�h�ł͖ő��ɔ������Ȃ��j
        if ( s_cbData.romStatus & (HOTSW_ROMST_RFS_WARN_L1_MASK | HOTSW_ROMST_RFS_WARN_L2_MASK) )
        {
            state = pRefreshBadBlock(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
            while (1)
            {
                state = pReadStatus(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
                if ( s_cbData.romStatus & HOTSW_ROMST_RFS_READY_MASK ||
                     (retval != HOTSW_SUCCESS) )
                {
                    break;
                }
                OS_Sleep(1);
            }
        }
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_IsRomEmulation

  Description:  ROM���G�~�����[�V�������Ă��邩�H
 *---------------------------------------------------------------------------*/
BOOL HOTSWi_IsRomEmulation(void)
{
    return s_debuggerFlg && s_romEmuInfo.isEnableSlot1;
}

/*---------------------------------------------------------------------------*
  Name:         RegisterRomEmuInfo

  Description:  ROM�G�~�����[�V��������o�^
 *---------------------------------------------------------------------------*/
static void RegisterRomEmuInfo(void)
{
    SYSM_work* sw = SYSMi_GetWork();

    // �f�o�b�K���
    if ( ! sw->flags.hotsw.is1stCardChecked && s_debuggerFlg )
    {
        MI_CpuCopy8( &s_romEmuInfo, &(sw->romEmuInfo), ROM_EMULATION_DATA_SIZE );
        sw->flags.hotsw.isOnDebugger = s_debuggerFlg;
    }

    sw->flags.hotsw.isCardStateChanged = TRUE;
    sw->flags.hotsw.is1stCardChecked   = TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         LoadBannerData

  Description:  �o�i�[�f�[�^��ǂݍ���

  ���F�Q�[�����[�h�ɂȂ��Ă���Ăяo���Ă�������
 *---------------------------------------------------------------------------*/
static HotSwState LoadBannerData(void)
{
    BOOL state;
    HotSwState retval = HOTSW_SUCCESS;
    SYSM_work* sw = SYSMi_GetWork();

    // �o�i�[���[�h
    if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
        retval  = ReadPageGame(&s_cbData,   s_cbData.pBootSegBuf->rh.s.banner_offset,
                                            (u32 *)SYSM_CARD_BANNER_BUF,
                                            sizeof(TWLBannerFile) );

        // �o�i�[���[�h���������Ă�����e��t���OTRUE ���̑��̏ꍇ��FALSE (���̊֐��̊O�Ŕr�����䂳��Ă��邩�炱���ł͔r�����䂵�Ȃ���OK)
        state = (retval == HOTSW_SUCCESS) ? TRUE : FALSE;
        sw->flags.hotsw.isValidCardBanner  = state;
        sw->flags.hotsw.isExistCard        = state;
    }
    else{
        // �o�i�[�f�[�^���o�^����Ă��Ȃ��ꍇ (���̊֐��̊O�Ŕr�����䂳��Ă��邩�炱���ł͔r�����䂵�Ȃ���OK)
        sw->flags.hotsw.isValidCardBanner  = FALSE;
        sw->flags.hotsw.isExistCard        = TRUE;
    }

    if ( sw->flags.hotsw.isExistCard )
    {
        sw->flags.hotsw.isInspectCard = s_cbData.pBootSegBuf->rh.s.inspect_card;
    }
    else
    {
        sw->flags.hotsw.isInspectCard = FALSE;
    }

    // �f�o�b�K����o�^
    RegisterRomEmuInfo();

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         ReadImageReturnErrorCode

  Description:  �J�[�h���璆�r���[�ȃT�C�Y(page�r��)�̃f�[�^��ǂݏo���֐�
                �G���[�R�[�h��Ԃ�
 *---------------------------------------------------------------------------*/
static HotSwState ReadImageReturnErrorCode(void* dest, s32 offset, s32 length, void* arg)
{
    HotSwState retval = HOTSW_SUCCESS;

    static u8 page_buffer[512];
    u32 page_offset = (u32)(offset & -512);
    u32 buffer_offset = (u32)(offset % 512);
    u32 valid_length = 512 - buffer_offset;
    u32 remain_length;

    // �J�n�A�h���X���y�[�W�̓r��
    if ( offset % 512 )
    {
        retval = ReadPageGame((CardBootData*)arg, page_offset, page_buffer, 512);

        if (retval != HOTSW_SUCCESS)
        {
            return retval;
        }

        MI_CpuCopy8(page_buffer + buffer_offset, dest, (length < valid_length ? length : valid_length));

        dest = (u8*)dest + valid_length;
        offset += valid_length;
        length -= valid_length;
        if ( length < 0)
        {
            return retval;
        }
    }

    remain_length = (u32)(length % 512);
    retval = ReadPageGame((CardBootData*)arg, (u32)offset, dest, (u32)(length - remain_length));

    if (retval != HOTSW_SUCCESS)
    {
        return retval;
    }

    // �P�c���y�[�W�r��
    if( remain_length ){
        dest   = (u8*)dest + (length - remain_length);
        offset += length - remain_length;

        retval = ReadPageGame((CardBootData*)arg, (u32)offset, page_buffer, 512);

        if (retval != HOTSW_SUCCESS)
        {
            return retval;
        }

        MI_CpuCopy8(page_buffer, dest, remain_length);
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         ReadImage

  Description:  �J�[�h���璆�r���[�ȃT�C�Y(page�r��)�̃f�[�^��ǂݏo���֐�
                ���������TRUE��Ԃ�
 *---------------------------------------------------------------------------*/
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    return (ReadImageReturnErrorCode(dest, offset, length, arg) == HOTSW_SUCCESS);
}


/*---------------------------------------------------------------------------*
  Name:         ReadCardData

  Description:  ARM9����ʒm���ꂽ�͈͂̃f�[�^���J�[�h����ǂݍ���
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
static void ReadCardData(u32 src, u32 dest, u32 size)
{
    u32 sendSize = 0;
    HotSwState state = HOTSW_SUCCESS;

    if(!SYSMi_GetWork()->flags.hotsw.isCardGameMode){
        state = HOTSW_MODE_ERROR;
    }

    if(SYSMi_GetWork()->flags.hotsw.isCardStateChanged){
        state = HOTSW_PULLED_OUT_ERROR;
    }
    
    // �J�[�h�̃��b�N
    CARD_LockRom(s_CardLockID);

    // [Debug]
    ReadIDGame(&s_cbData);
    if(s_cbData.id_gam != s_gameID){
		state = HOTSW_GAMEMODE_ID_CHECK_ERROR;
    }

	// [Debug]
    if(!(reg_MI_EXMEMCNT_L & REG_MI_EXMEMCNT_L_MP_MASK)){
		state = HOTSW_BUS_LOCK_ERROR;
    }
    
    while(size > 0 && state == HOTSW_SUCCESS){
        // --- Boot Segment
        if(src >= HOTSW_BOOTSEGMENT_AREA_OFS && src < HOTSW_KEYTABLE_AREA_OFS){
            sendSize = ((src + size) > HOTSW_KEYTABLE_AREA_OFS) ? HOTSW_KEYTABLE_AREA_OFS - src : size;
            MI_CpuCopy8((u32 *)(SYSM_CARD_ROM_HEADER_BAK + (src - HOTSW_BOOTSEGMENT_AREA_OFS)), (u32 *)dest, sendSize);
        }

        // --- Key Table
        else if(src >= HOTSW_KEYTABLE_AREA_OFS && src < HOTSW_SECURE_AREA_OFS){
            sendSize = ((src + size) > HOTSW_SECURE_AREA_OFS) ? HOTSW_SECURE_AREA_OFS - src : size;
            state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
        }
        // --- Secure Segment
        else if(src >= HOTSW_SECURE_AREA_OFS && src < HOTSW_GAME_AREA_OFS){
            sendSize = ((src + size) > HOTSW_GAME_AREA_OFS) ? HOTSW_GAME_AREA_OFS - src : size;
            MI_CpuCopy8((u32 *)((u32)s_cbData.pSecureSegBuf + (src - HOTSW_SECURE_AREA_OFS)), (u32 *)dest, sendSize);
        }
        // --- Game Segment
        else if(src >= HOTSW_GAME_AREA_OFS){
            // KeyTable2���ݒ肳��Ă���ꍇ
            if(s_cbData.isLoadTypeTwl){
                u32 keyTable2Adr = (u32)s_cbData.pBootSegBuf->rh.s.twl_card_keytable_area_rom_offset * TWLCARD_BORDER_OFFSET;
                u32 Secure2Adr   = keyTable2Adr + HOTSW_SECURE2_AREA_OFS;
                u32 Game2Adr     = keyTable2Adr + HOTSW_GAME2_AREA_OFS;

                // --- Game Segment
                if(src < keyTable2Adr){
                    sendSize = ((src + size) > keyTable2Adr) ? keyTable2Adr - src : size;
                    state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
                }
                // --- Key Table2
                else if(src >= keyTable2Adr && src < Secure2Adr){
                    sendSize = ((src + size) > Secure2Adr) ? Secure2Adr - src : size;
                    state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
                }
                // --- Secure2 Segment
                else if(src >= Secure2Adr && src < Game2Adr){
                    sendSize = ((src + size) > Game2Adr) ? Game2Adr - src : size;
                    MI_CpuCopy8((u32 *)((u32)s_cbData.pSecure2SegBuf + (src - Secure2Adr)), (u32 *)dest, sendSize);
                }
                // --- Game2 Segment
                else{
                    sendSize = size;
                    state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
                }
            }
            // --- Game Segment
            else{
                sendSize = size;
                state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
            }
        }

        size -= sendSize;
        src  += sendSize;
        dest += sendSize;
    }

    // [Debug]
    ReadIDGame(&s_cbData);
    if(s_cbData.id_gam != s_gameID){
        if(state == HOTSW_SUCCESS){
        	state = HOTSW_GAMEMODE_ID_CHECK_ERROR;
        }
    }
    
    // �J�[�h�̃A�����b�N
    CARD_UnlockRom(s_CardLockID);
    
    {
        HotSwPxiMessageForArm9  msg;
        CardDataReadState       retval;

        switch(state){
          case HOTSW_SUCCESS:
            retval = CARD_READ_SUCCESS;
            break;

          case HOTSW_TIME_OUT:
            retval = CARD_READ_TIME_OUT;
            break;

          case HOTSW_PULLED_OUT_ERROR:
            retval = CARD_READ_PULLED_OUT_ERROR;
            break;

          case HOTSW_BUFFER_OVERRUN_ERROR:
            retval = CARD_READ_BUFFER_OVERRUN_ERROR;
            break;

          case HOTSW_MODE_ERROR:
            retval = CARD_READ_MODE_ERROR;
            break;

          case HOTSW_GAMEMODE_ID_CHECK_ERROR:
            retval = CARD_READ_ID_CHECK_ERROR;
            break;

          case HOTSW_BUS_LOCK_ERROR:
            retval = CARD_READ_BUS_LOCK_ERROR;
            break;
            
          default:
            retval = CARD_READ_UNEXPECTED_ERROR;
            break;
        }

        MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm9));

        msg.msg.read   = TRUE;
        msg.msg.result = (u8)retval;

        while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
        {
            // do nothing
        }
    }
}
#endif


/*---------------------------------------------------------------------------*
  Name:         LoadStaticModule

  Description:  ARM7,9�̏풓���W���[����W�J����֐�

  ���F�Q�[�����[�h�ɂȂ��Ă���Ăяo���Ă�������
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static HotSwState LoadStaticModule(void)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 arm9StcEnd    = s_cbData.pBootSegBuf->rh.s.main_rom_offset     + s_cbData.pBootSegBuf->rh.s.main_size;
    u32 arm9LtdStcEnd = s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + s_cbData.pBootSegBuf->rh.s.main_ltd_size;
    u32 secure2SegEnd = (u32)(s_cbData.pBootSegBuf->rh.s.twl_card_keytable_area_rom_offset * TWLCARD_BORDER_OFFSET + SECURE_SEGMENT_SIZE);

    // �z�u��ƍĔz�u�����擾 & Arm9�̏풓���W���[���c����w���ɓ]��
    s_cbData.arm9Stc = (u32)s_cbData.pBootSegBuf->rh.s.main_ram_address;
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_STATIC, &s_cbData.arm9Stc, s_cbData.pBootSegBuf->rh.s.main_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC] , s_cbData.twlFlg)){
        if(arm9StcEnd > SECURE_SEGMENT_END){
            state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.main_rom_offset + SECURE_SEGMENT_SIZE, (u32 *)(s_cbData.arm9Stc + SECURE_SEGMENT_SIZE), MATH_ROUNDUP( arm9StcEnd, SYSM_ALIGNMENT_LOAD_MODULE ) - SECURE_SEGMENT_END);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
    }
    else{
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
    }
    if(retval != HOTSW_SUCCESS){
        return retval;
    }

    // �z�u��ƍĔz�u�����擾 & Arm7�̏풓���W���[�����w���ɓ]��
    s_cbData.arm7Stc = (u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address;
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_STATIC, &s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC], s_cbData.twlFlg)){
        state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_rom_offset, (u32 *)s_cbData.arm7Stc, MATH_ROUNDUP( s_cbData.pBootSegBuf->rh.s.sub_size, SYSM_ALIGNMENT_LOAD_MODULE ) );
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }
    else{
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
    }
    if(retval != HOTSW_SUCCESS){
        return retval;
    }

    // �g���풓���W���[���̓ǂݍ���
    if( s_cbData.twlFlg ) {
        u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ? s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
        s_cbData.arm9Ltd = (u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address;
        // �z�u��ƍĔz�u�����擾 & Arm9�̏풓���W���[�����w���ɓ]��
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_LTD_STATIC, &s_cbData.arm9Ltd, s_cbData.pBootSegBuf->rh.s.main_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC] , TRUE)){
            if(!s_cbData.isLoadTypeTwl){
                // Secure2�̈�ǂ�
                state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset, (u32 *)SYSM_CARD_TWL_SECURE_BUF, MATH_ROUNDUP( size, SYSM_ALIGNMENT_LOAD_MODULE ));
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
            }

            // (Arm9Ltd�̈� - Secure2�̈�)���̓ǂݍ���
            if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
                state  = ReadPageGame(&s_cbData,        s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + SECURE_SEGMENT_SIZE,
                                                (u32 *)(s_cbData.arm9Ltd                               + SECURE_SEGMENT_SIZE),
                                                        MATH_ROUNDUP( s_cbData.pBootSegBuf->rh.s.main_ltd_size, SYSM_ALIGNMENT_LOAD_MODULE )       - size);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
            }
        }
        else{
            retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
        }

        if(retval != HOTSW_SUCCESS){
            return retval;
        }

        // �z�u��ƍĔz�u�����擾 & Arm7�̏풓���W���[�����w���ɓ]��
        s_cbData.arm7Ltd = (u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address;
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_LTD_STATIC, &s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_LTD_STATIC], TRUE)){
            state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset, (u32 *)s_cbData.arm7Ltd, MATH_ROUNDUP( s_cbData.pBootSegBuf->rh.s.sub_ltd_size, SYSM_ALIGNMENT_LOAD_MODULE ) );
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
        else{
            retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
        }
        if(retval != HOTSW_SUCCESS){
            return retval;
        }

        // �Z�L���A�̈�擪2K���̃n�b�V���l�����߂āAWork�̈�ɃR�s�[
        {
            SVCHMACSHA1Context hash;

            // �n�b�V��������
            SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

            // �Z�L���A�̈�擪2kb��UpDate
            SVC_HMACSHA1Update( &hash, s_cbData.pSecureSegBuf, ENCRYPT_DEF_SIZE );

            // Arm9�Ɣr�����䂷��
            LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);

            // �n�b�V���R���e�L�X�g��Work�̈�ɃR�s�[
            MI_CpuCopy8( &hash, &SYSMi_GetWork2()->hmac_sha1_context, sizeof(SVCHMACSHA1Context) );

            // Arm9�Ƃ̔r������A�����܂�
            UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
        }

//#define MY_DEBUG
#ifdef  MY_DEBUG
        (void)CheckStaticModuleHash();
#endif
    }

    return retval;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckCardAuthCode

  Description:  Rom Header�̔F�؃R�[�h�A�h���X��ǂ�ŁA�N���[���u�[�g�Ή������肷��
 *---------------------------------------------------------------------------*/
static HotSwState CheckCardAuthCode(void)
{
    u32 authBuf[PAGE_SIZE/sizeof(u32)];
    u32 auth_offset   = s_cbData.pBootSegBuf->rh.s.rom_valid_size ? s_cbData.pBootSegBuf->rh.s.rom_valid_size : 0x01000000;
    u32 page_offset   = auth_offset & 0xFFFFFE00;
    HotSwState retval = HOTSW_SUCCESS;

    u8  *p = (u8 *)authBuf;

    if(!HOTSW_IsCardAccessible()){
        return HOTSW_PULLED_OUT_ERROR;
    }

    retval = ReadPageGame( &s_cbData, page_offset, authBuf, MB_AUTHCODE_SIZE );

    p += auth_offset & 0x000001FF;
    if( *p++ == 'a' && *p == 'c' ) {
        HOTSW_PutString("  �� Clone Boot Mode\n");
        SYSMi_GetWork()->cloneBootMode = SYSM_CLONE_BOOT_MODE;
    }else {
        HOTSW_PutString("  �� Other Boot Mode\n");
        SYSMi_GetWork()->cloneBootMode = SYSM_OTHER_BOOT_MODE;
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_SetBootSegmentBuffer

  Description:  Boot Segment �o�b�t�@�̎w��

  ���F�J�[�h�f�[�^���[�h���͌Ăяo���Ȃ��悤�ɂ���
 *---------------------------------------------------------------------------*/
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size)
{
    SDK_ASSERT(size > BOOT_SEGMENT_SIZE);

    s_pBootSegBuffer = (BootSegmentData *)buf;
    s_BootSegBufSize = size;

    s_cbData.pBootSegBuf = s_pBootSegBuffer;

    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pBootSegBuffer, size);
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_SetSecureSegmentBuffer

  Description:  Secure Segment �o�b�t�@�̎w��

  ���F�J�[�h�f�[�^���[�h���͌Ăяo���Ȃ��悤�ɂ���
 *---------------------------------------------------------------------------*/
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size)
{
    HOTSWi_SetSecureSegmentBuffer(type, buf, size);
    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pSecureSegBuffer, size);
}

void HOTSWi_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size)
{
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);

    if(type == HOTSW_MODE1){
        s_pSecureSegBuffer = (u32 *)buf;
        s_SecureSegBufSize = size;

        s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

    }
    else{
        s_pSecure2SegBuffer = (u32 *)buf;
        s_Secure2SegBufSize = size;

        s_cbData.pSecure2SegBuf = s_pSecure2SegBuffer;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GenVA_VB_VD

  Description:  �R�}���h�F�ؒl�E�R�}���h�J�E���^�EPN�W�F�l���[�^�����l�̐���
 *---------------------------------------------------------------------------*/
static void GenVA_VB_VD(void)
{
    SYSM_work* sw = SYSMi_GetWork();
    u32 dummy = 0;
    MATHRandContext32 rnd;
    u64 fuse64 = SCFG_ReadFuseData();
    u32 fuse32[2];

    // �O�̂���eFuseID�𐄑��ł��Ȃ�����
    fuse32[0] = ((u32*)&fuse64)[0];
    fuse32[1] = ((u32*)&fuse64)[1];
    EncryptByBlowfish(&s_cbData.keyTable, &fuse32[0], &fuse32[1]);

    // ������������
    // �`�b�N��RTC���񃍁[�h�l���Q�[���R�[�h��eFuseID����Ƃ���B
    // �i�N������x�ɕω�����p�����[�^�Ƒg�ݍ��킹��B
    //   V�J�E���^��2130�T�C�N���ω����Ȃ��̂ŌŒ�l�ɂȂ�₷���B�j
    // �N�����A�Q�[���J�[�h���A�{�̖��ɃX�N�����u�����ω����邱�ƂɂȂ�B
    MATH_InitRand32(&rnd, OS_GetTick() ^ sw->Rtc1stData.words[0] ^ sw->Rtc1stData.words[1] ^ *(u32*)(s_cbData.pBootSegBuf->rh.s.game_code) ^ fuse32[0] ^ fuse32[1]);

    s_cbData.vae = MATH_Rand32(&rnd, 0);
    s_cbData.vbi = MATH_Rand32(&rnd, 0);
    s_cbData.vd  = MATH_Rand32(&rnd, 0);
    dummy        = MATH_Rand32(&rnd, 0);

    EncryptByBlowfish(&s_cbData.keyTable, &s_cbData.vae, &s_cbData.vbi);
    EncryptByBlowfish(&s_cbData.keyTable, &s_cbData.vd , &dummy);

    s_cbData.vae &= 0xffffff;
    s_cbData.vbi &= 0xfffff;
    s_cbData.vd  &= 0xffffff;
}


/*---------------------------------------------------------------------------*
  Name:         DecryptObjectFile

  Description:  �Z�L���A�̈�擪2KB�̈Í����̈�𕜍���
 *---------------------------------------------------------------------------*/
static u32 encDestBuf[ENCRYPT_DEF_SIZE/sizeof(u32)];

#ifndef USE_WRAM_LOAD
static HotSwState DecryptObjectFile(void)
#else
BOOL HOTSW_DecryptObjectFile(void* dest)
#endif
{
    u8  i;
    s32 restSize;
    s32 size                = (s32)s_cbData.pBootSegBuf->rh.s.main_size;
    u32 *pEncBuf            = encDestBuf;
#ifndef USE_WRAM_LOAD
    u32 *pEncDes            = s_cbData.pSecureSegBuf;
#else
    u32 *pEncDes            = (u32 *)dest;
#endif
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
    BOOL exist              = TRUE;
#ifndef USE_WRAM_LOAD
    HotSwState retval       = HOTSW_SUCCESS;
#else
    BOOL       retval       = TRUE;
#endif

    if (size > ENCRYPT_DEF_SIZE) {
        size = ENCRYPT_DEF_SIZE;
    }
    restSize = size;

    // �ǂݍ��񂾃Z�L���A�̈���o�b�t�@����ꎞ�o�b�t�@�ɃR�s�[
    MI_CpuCopy32(pEncDes, pEncBuf, (u32)size);

    // �Z�L���A�̈�擪8�o�C�g��Blowfish�ŕ�����
    DecryptByBlowfish(&s_cbData.keyTable, &(pEncBuf)[1], &(pEncBuf)[0]);

    // Key Table��ϊ�
    s_cbData.keyBuf[1] = (s_cbData.keyBuf[1] << 1);
    s_cbData.keyBuf[2] = (s_cbData.keyBuf[2] >> 1);
    InitBlowfishKeyAndTableDS(&s_cbData.keyTable, s_cbData.keyBuf, 8);

    // ������x�Z�L���A�̈�擪8�o�C�g��Blowfish�ŕ�����
    DecryptByBlowfish(&s_cbData.keyTable, &(pEncBuf)[1], &(pEncBuf)[0]);
    for ( i=0; i<8; i++ ){
        // �擪8�o�C�g���������̌��� "encryObj" �ƂȂ��Ă����畡��������
        if ( encrypt_object_key[i] != ((char*)pEncBuf)[i] ){
            exist = FALSE;
            break;
        }
    }

    // �Í����I�u�W�F�N�g�L����
    if ( exist ){
        u32 *bufp  = pEncBuf;

        bufp[0] = UNDEF_CODE;
        bufp[1] = UNDEF_CODE;
        while ((restSize -= 8) > 0) {
            bufp += 2;  // ��������
            DecryptByBlowfish(tableBufp, &(bufp)[1], &(bufp)[0]);
        }
    }
    else{
#ifndef USE_WRAM_LOAD
        retval = HOTSW_DATA_DECRYPT_ERROR;
#else
        retval = FALSE;
#endif
        MI_NDmaFill( HOTSW_NDMA_NO, pEncBuf, UNDEF_CODE, (u32)size ); // ����`�R�[�h�ŃN���A
    }
    MI_CpuCopy32(pEncBuf, pEncDes, (u32)size);

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         LockHotSwRsc

  Description:  ���L���[�N�̃��\�[�X�̔r������p�@lock���s��
 *---------------------------------------------------------------------------*/
static void LockHotSwRsc(OSLockWord* word)
{
    while(OS_TryLockByWord( s_RscLockID, word, NULL ) != OS_LOCK_SUCCESS){
        OS_Sleep(1);
    }
}


/*---------------------------------------------------------------------------*
  Name:         UnlockHotSwRsc

  Description:  ���L���[�N�̃��\�[�X�̔r������p�@Unlock���s��
 *---------------------------------------------------------------------------*/
static void UnlockHotSwRsc(OSLockWord* word)
{
    OS_UnlockByWord( s_RscLockID, word, NULL );
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_IsCardExist

  Description:  SCFG_MC1��CDET�t���O�����āA�J�[�h�̑��ݔ�����s��
 *---------------------------------------------------------------------------*/
BOOL HOTSW_IsCardExist(void)
{
    if( !(reg_MI_MC1 & REG_MI_MC_SL1_CDET_MASK) ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_IsCardAccessible

  Description:  SCFG_MC1��CDET�t���O��M(���[�h)�����āA�J�[�h�X���b�g�ɃA�N�Z�X�ł����Ԃ����肷��
 *---------------------------------------------------------------------------*/
BOOL HOTSW_IsCardAccessible(void)
{
    if( HOTSW_IsCardExist() && (GetMcSlotMode() == SLOT_STATUS_MODE_10)){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotMode

  Description:  �X���b�g�̌��݂̃��[�h��Ԃ�
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotMode(void)
{
    return reg_MI_MC1 & REG_MI_MC_SL1_MODE_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         SetMcSlotMode

  Description:  �J�[�h�X���b�g�̃��[�h�ݒ�
 *---------------------------------------------------------------------------*/
static void SetMcSlotMode(u32 mode)
{
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~REG_MI_MC_SL1_MODE_MASK) | mode);
}


/*---------------------------------------------------------------------------*
  Name:        McPowerOn

  Description: �X���b�g�d��ON
 *---------------------------------------------------------------------------*/
static void McPowerOn(void)
{
    // Counter-A�J�E���^�ݒ�l���B�܂ő҂�
    while(GetMcSlotMode() == SLOT_STATUS_MODE_11){
        OS_Sleep(1);
    }

    if(GetMcSlotMode() == SLOT_STATUS_MODE_00){
        // 3DM�̃��Z�b�g�҂�
        OS_Sleep(1);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 01 �ɂ���
        SetMcSlotMode(SLOT_STATUS_MODE_01);

        // VDD�̈�����ԑ҂�
        OS_Sleep(10);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 10 �ɂ���
        SetMcSlotMode(SLOT_STATUS_MODE_10);

        // RESB���グ��܂ł̑҂�����
        OS_Sleep(27);

        // ���Z�b�g��high�� (RESB = 1�ɂ���)
        reg_HOTSW_MCCNT1 = RESB_MASK;

        // �J�[�h�֍ŏ��̃R�}���h�𑗂�܂ł̑҂�����
        OS_Sleep(120);
    }
}


/*---------------------------------------------------------------------------*
  Name:        McPowerOff

  Description: �X���b�g�d��OFF
 *---------------------------------------------------------------------------*/
static void McPowerOff(void)
{
    // Counter-A�J�E���^�ݒ�l���B�܂ő҂�
    while(GetMcSlotMode() == SLOT_STATUS_MODE_11){
        OS_Sleep(1);
    }

    if(GetMcSlotMode() == SLOT_STATUS_MODE_10){
        // SCFG_MC1 �� Slot Status �� M1,M0 �� 11 �ɂ���
        SetMcSlotMode(SLOT_STATUS_MODE_11);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 00 �ɂȂ�܂Ń|�[�����O
        while(GetMcSlotMode() != SLOT_STATUS_MODE_00){
            OS_Sleep(1);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_TurnCardPowerOn

  Description:  �f�o�b�K�ʐM�p�ɃJ�[�h�X���b�g�̓d����ON�ɂ���B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void HOTSWi_TurnCardPowerOn(u32 slot)
{
    u32 shift;
    u32 mask = slot >= 2 ? (u32)REG_MI_MC_SL2_CDET_MASK : (u32)REG_MI_MC_SL1_CDET_MASK;

    // �J�[�h���������Ă��Ȃ����ON���Ȃ�
    if ( reg_MI_MC & mask )
    {
        return;
    }

    shift = slot >= 2 ? (u32)REG_MI_MC_SL2_MODE_SHIFT : (u32)REG_MI_MC_SL1_MODE_SHIFT;
    mask  = slot >= 2 ? (u32)REG_MI_MC_SL2_MODE_MASK : (u32)REG_MI_MC_SL1_MODE_MASK;

    // �d���������Ă���Œ��Ȃ�҂�
    if ( (reg_MI_MC & mask) == (0x3 << shift) )
    {
        OS_Sleep(1);
    }

    if ( (reg_MI_MC & mask) == (0x0 << shift) )
    {
        // 3DM�̃��Z�b�g�҂�
        OS_Sleep(1);

        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x1 << shift));
        // VDD�̈������
        OS_Sleep(10);

        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x2 << shift));
        // RESB��Low����
        OS_Sleep(27);

        reg_MI_MCCNT1_B = REG_MI_MCCNT1_RESB_MASK;
        // RESB��High����
        // �J�[�h�֍ŏ��̃R�}���h�𑗂�܂ł̑҂�����
        OS_Sleep(120);
    }
}


/*---------------------------------------------------------------------------*
  Name:         SetMCSCR

  Description:  ����������H�����l�ݒ背�W�X�^��ݒ肷��

  �����F���̊֐��̓Z�L���A���[�h�ŁAsPNG_ON�R�}���h�����s���Ă���Ăяo���Ă��������B
 *---------------------------------------------------------------------------*/
static void SetMCSCR(void)
{
    static u32 pnbL          = 0x879b9b05;
    static u8  pnbH          = 0x5c;
    static u8  pnaL1         = 0x60;
    static u8  pnaL0Table[8] = { 0xe8, 0x4d, 0x5a, 0xb1, 0x17, 0x8f, 0x99, 0xd5 };

    u32 pnaL = s_cbData.vd << 15 | pnaL1 << 8 | pnaL0Table[(s_cbData.pBootSegBuf->rh.s.rom_type & 0x7)];
    u8  pnaH = (u8)((s_cbData.vd >> 17) & 0x7f);

    // SCR A
    reg_HOTSW_MCSCR0 = pnaL;

    // SCR B
    reg_HOTSW_MCSCR1 = pnbL;

    // [d0 -d6 ] -> SCR A
    // [d16-d22] -> SCR B
    reg_HOTSW_MCSCR2 = (u32)(pnaH | pnbH << 16);

    // MCCNT1 ���W�X�^�ݒ� (SCR = 1��)
    reg_HOTSW_MCCNT1 = SCR_MASK;
}


/*---------------------------------------------------------------------------*
  Name:        HotSwThread

  Description: �J�[�h�����E�}�������X���b�h
 *---------------------------------------------------------------------------*/
static void HotSwThread(void *arg)
{
    #pragma unused( arg )

    HotSwState              retval;
    HotSwMessageForArm7     *msg;

    while(1){
        OS_ReceiveMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        SYSMi_GetWork()->flags.hotsw.isBusyHotSW = TRUE;

        if( msg->ctrl == TRUE ) {
            SYSMi_GetWork()->flags.hotsw.isEnableHotSW = msg->value;
            s_pollingThreadSleepFlg = msg->value ? FALSE : TRUE;

            if(msg->value){
#ifndef USE_WRAM_LOAD
                s_isPulledOut = TRUE;
#endif
                OS_SendMessage(&HotSwThreadData.hotswPollingCtrlQueue,
                               (OSMessage *)&HotSwThreadData.hotswPollingCtrlMsg[HotSwThreadData.idx_polling],
                               OS_MESSAGE_NOBLOCK);
                HotSwThreadData.idx_polling = (HotSwThreadData.idx_polling+1) % HOTSW_POLLING_CTRL_BUFFER_NUM;
            }
            else{
                if(msg->finalize == FALSE){
                    ClearCardFlgs();
                }
            }
        }

        if( msg->finalize == TRUE && msg->ctrl == FALSE) {
            FinalizeHotSw( msg->state );
        }

#ifdef USE_WRAM_LOAD
        if( msg->read == TRUE ){
            ReadCardData(SYSMi_GetWork()->cardReadParam.src,
                         SYSMi_GetWork()->cardReadParam.dest,
                         SYSMi_GetWork()->cardReadParam.size);

            SYSMi_GetWork()->flags.hotsw.isBusyHotSW = FALSE;

            continue;
        }

        if( msg->type == HOTSW_INSERT ){
            SendPxiMessage(HOTSW_CARD_INSERT);
        }
#endif

        while(1){
            if( !SYSMi_GetWork()->flags.hotsw.isEnableHotSW ) {
                HOTSW_PutString("### HotSw is restrained...\n");
                break;
            }

            if(HOTSW_IsCardExist()){
                if(!s_isPulledOut){
                    if(GetMcSlotMode() == SLOT_STATUS_MODE_10){
                        // [Debug]
						s_gameID = s_cbData.id_gam;
                        
                        LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                        if( msg->ctrl && msg->value ){
                            SYSMi_GetWork()->flags.hotsw.isExistCard        = TRUE;
                            SYSMi_GetWork()->flags.hotsw.isCardStateChanged = TRUE;
                            if( s_cbData.pBootSegBuf->rh.s.banner_offset ){
                                SYSMi_GetWork()->flags.hotsw.isValidCardBanner = TRUE;
                            }
                        }
                        SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = TRUE;
#ifdef USE_WRAM_LOAD
                        SYSMi_GetWork()->flags.hotsw.isCardGameMode      = TRUE;
#endif
                        UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
#ifdef USE_WRAM_LOAD
                        SendPxiMessage(HOTSW_CHANGE_GAMEMODE);
#endif
                        HOTSW_PutString("ok!\n");

                        break;
                    }
                }

                retval = LoadCardData();

                DebugPrintErrorMessage(retval);

                s_isPulledOut = FALSE;

                // �G���[����
                if(retval != HOTSW_SUCCESS){
                    McPowerOff();
                    PulledOutSequence();
                    break;
                }
            }

            // �J�[�h�������Ă���
            else{
                PulledOutSequence();
                break;
            }
        } // Card Read while loop

        if( !SYSMi_GetWork()->flags.hotsw.is1stCardChecked ){
            LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
            SYSMi_GetWork()->flags.hotsw.is1stCardChecked = TRUE;
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
        }

        SYSMi_GetWork()->flags.hotsw.isBusyHotSW       = FALSE;
    } // while loop
}


/*---------------------------------------------------------------------------*
  Name:        PulledOutSequence

  Description: �J�[�h��������
 *---------------------------------------------------------------------------*/
static void PulledOutSequence(void)
{
#ifdef USE_WRAM_LOAD
    SendPxiMessage(HOTSW_CARD_PULLOUT);
#endif

    ClearCardFlgs();

    MI_CpuClear32(&s_cbData, sizeof(CardBootData));

    MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
    MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
    MI_CpuClearFast(s_pSecure2SegBuffer, s_Secure2SegBufSize);
    MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));

    s_isPulledOut = TRUE;

	// [Debug]
    s_gameID = 0;
    
    // �����Z�O�̃X���[�v���V���b�g�_�E���΍��߂�
    MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, TRUE );
}


/*---------------------------------------------------------------------------*
  Name:        ClearCardFlgs

  Description: �J�[�h�f�[�^�o�b�t�@�̃o�b�t�@�ƃt���O�̌�n��
 *---------------------------------------------------------------------------*/
static void ClearCardFlgs(void)
{
    // �t���O����
    LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
    SYSMi_GetWork()->flags.hotsw.isExistCard         = FALSE;
    SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
    SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
    SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
#ifdef USE_WRAM_LOAD
    SYSMi_GetWork()->flags.hotsw.isCardGameMode      = FALSE;
#endif
    UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
}


/*---------------------------------------------------------------------------*
  Name:        FinalizeHotSw

  Description: �A�v���N�����ɁA�����}���֌W�̌�n�����s���B
 *---------------------------------------------------------------------------*/
static void FinalizeHotSw(HotSwCardState state)
{
    ROM_Header* rh = (void*)SYSM_APP_ROM_HEADER_BUF;
    static BOOL finalized = FALSE;

    if(finalized){
        return;
    }
    finalized = TRUE;

    // �J�[�h�}�����o��~
    (void)OS_DisableIrqMask( HOTSW_IF_CARD_DET );

    // �|�[�����O�X���b�h�����쒆�͑҂�
    while(s_isBusyMonitorThread){
		OS_Sleep(1);
    }

    // �|�[�����O�X���b�h������
    OS_KillThread( &HotSwThreadData.monitorThread, NULL );

    SYSMi_GetWork()->appCardID = 0;

    // �J�[�h�A�N�Z�X�ł��Ȃ��Ȃ�A(�ꉞ)�X���b�g�d��OFF���ă��W�X�^�N���A
    if(!HOTSW_IsCardAccessible()){
        McPowerOff();
        ClearAllCardRegister();
        goto final;
    }

    switch(state){
        // Slot Power Off
      case HOTSW_CARD_STATE_POWER_OFF:
        McPowerOff();
        break;

        // Normal���[�h�Ɉڍs
      case HOTSW_CARD_STATE_NORMAL_MODE:
        ForceNormalModeToFinalize();
        break;

        // Game���[�h�Ɉڍs
      case HOTSW_CARD_STATE_GAME_MODE:
        ForceNitroModeToFinalize();
        break;

        // ��ԃL�[�v
      case HOTSW_CARD_STATE_KEEP:
        SYSMi_GetWork()->appCardID = s_cbData.id_gam;
        break;

        // else
      default:
        ClearAllCardRegister();
        McPowerOff();
    }

final:
    ClearCardIrq();

    // �f�o�b�K�ł�TWL�J�[�h�X���b�g�Q��d��ON
    //�i����ON�Ȃ牽�����Ȃ��j
    // NAND�A�v���N�����̓f�t�H���g�ł̓J�[�hI/F��OFF�ɂ��邪�A
    // �f�o�b�K���j�^�ł̓J�[�hI/F���K�v�Ȃ��߃X���b�g�Q���g�p
    if ( SYSM_IsRunOnDebugger() )
    {
        HOTSWi_TurnCardPowerOn( 2 );
    }

    // �I�������ʒm
    SYSMi_GetWork()->gameCommondParam = s_cbData.gameCommondParam;
    SYSMi_GetWork()->flags.hotsw.isFinalized = TRUE;
}


/*---------------------------------------------------------------------------*
  Name:        ForceNitroModeToFinalize

  Description: Nitro�݊���Game Mode�ɋ����ڍs������
 *---------------------------------------------------------------------------*/
static void ForceNitroModeToFinalize(void)
{
    s_cbData.id_gam = 0;

    if(!ChangeGameMode()){
        ClearAllCardRegister();
        McPowerOff();

        HOTSW_PutString("Failed To Change Game Mode... Card Slot Power Off\n");
    }

    SYSMi_GetWork()->appCardID = s_cbData.id_gam;
}


/*---------------------------------------------------------------------------*
  Name:        ForceNormalModeToFinalize

  Description: Normal Mode�ɋ����ڍs������
 *---------------------------------------------------------------------------*/
static void ForceNormalModeToFinalize(void)
{
    CARD_LockRom(s_CardLockID);

    McPowerOff();  // ����OFF�ɂȂ��Ă��邽�ߎ����I�ɂ͖���
    McPowerOn();

    s_cbData.modeType = HOTSW_MODE2;
    (void)LoadTable();
    (void)ReadIDNormal(&s_cbData);
    (void)ReadBootSegNormal(&s_cbData);

    CARD_UnlockRom(s_CardLockID);

    s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
    SYSMi_GetWork()->appCardID = s_cbData.id_nml;

    HOTSW_PutString("- game card on flg is TRUE : now Normal Mode\n");
}


/*---------------------------------------------------------------------------*
  Name:        ChangeGameMode

  Description: Nitro�݊���RomHeader��ǂ�ŁAGame Mode�Ɉڍs������
 *---------------------------------------------------------------------------*/
static BOOL ChangeGameMode(void)
{
    HotSwState state;

    CARD_LockRom(s_CardLockID);

    McPowerOff();
    McPowerOn();

    s_cbData.pBootSegBuf = s_pBootSegBuffer;
    s_cbData.modeType = HOTSW_MODE1;

    // ---------------------- Normal Mode ----------------------
    state = LoadTable();
    state = ReadIDNormal(&s_cbData);
    state = ReadBootSegNormal(&s_cbData);

    if(s_isRomEmu){
        s_cbData.cardType = ROM_EMULATION;
        s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
    }
    else{
        s_cbData.cardType = (s_cbData.id_nml & HOTSW_ROMID_1TROM_MASK) ? DS_CARD_TYPE_2 : DS_CARD_TYPE_1;
        s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
    }
    s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);

    MI_CpuClear32(&s_cbData.keyTable, sizeof(BLOWFISH_CTX));
    MI_CpuClear32(s_cbData.keyBuf, sizeof(s_cbData.keyBuf));

    MakeBlowfishTableDS(&s_cbData, 8);
    GenVA_VB_VD();
    state = ChangeModeNormal(&s_cbData);

    // ---------------------- Secure Mode ----------------------
    state = s_funcTable[s_isRomEmu].SetPNG_S(&s_cbData);
    SetMCSCR();
    state = s_funcTable[s_isRomEmu].ChangeMode_S(&s_cbData);

    // ---------------------- Game Mode ----------------------
    state = ReadIDGame(&s_cbData);

    if(s_cbData.id_nml != s_cbData.id_gam){
        state = HOTSW_ID_CHECK_ERROR;
    }

    HOTSW_TPrintf("Card Normal ID : 0x%08x\n", s_cbData.id_nml);
    HOTSW_TPrintf("Card Game   ID : 0x%08x\n", s_cbData.id_gam);

#ifdef USE_NEW_DMA
    HOTSW_WaitNDmaCtrl(HOTSW_NDMA_NO);
#else
    HOTSW_WaitDmaCtrl(HOTSW_DMA_NO);
#endif

    HOTSW_WaitCardCtrl();

    CARD_UnlockRom(s_CardLockID);

    if(state == HOTSW_SUCCESS){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:        SendPxiMessage

  Description: ARM9�Ƀ��b�Z�[�W�𑗂�
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
static void SendPxiMessage(HotSwCallBackType type)
{
    HotSwPxiMessageForArm9  msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm9));

    switch(type){
      case HOTSW_CHANGE_GAMEMODE:
        msg.msg.mode = TRUE;
        break;

      case HOTSW_CARD_INSERT:
        msg.msg.insert = TRUE;
        break;

      case HOTSW_CARD_PULLOUT:
        msg.msg.pullout = TRUE;
        break;

      default:
        return;
    }

    while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    {
        // do nothing
    }
}
#endif


/*---------------------------------------------------------------------------*
  Name:        ClearCardIrq

  Description: �J�[�h�֘A�̊��荞�݂��N���A����
 *---------------------------------------------------------------------------*/
#define REGCLEAR_16     0x0000
#define REGCLEAR_32     0x00000000UL

static void ClearCardIrq(void)
{
    // HotSw�Ŏg���Ă��銄�荞�݂𖳌��ɂ���
    OSIntrMode irq = OS_DisableInterrupts();
    (void)OS_DisableIrqMask( HOTSW_IF_CARD_DET );
    (void)OS_ResetRequestIrqMask( HOTSW_IF_CARD_DET );
    (void)OS_RestoreInterrupts( irq );
}


/*---------------------------------------------------------------------------*
  Name:        ClearAllCardRegister

  Description: �J�[�h�֘A�̑S�Ẵ��W�X�^���N���A����B
 *---------------------------------------------------------------------------*/
static void ClearAllCardRegister(void)
{
    ClearCardIrq();

    // �R�}���h�ݒ背�W�X�^���N���A [�e32bit]
    reg_HOTSW_MCCMD0 = REGCLEAR_32;
    reg_HOTSW_MCCMD1 = REGCLEAR_32;

    // �������J�[�h�C�l�[�u���Ƃ����荞�݃C�l�[�u���Ƃ����N���A [16bit�� d15-d8���֌W����]
    reg_HOTSW_MCCNT0 &= HOTSW_E2PROM_CTRL_MASK;

    // latency�Ƃ�start�t���O�Ƃ��̃��W�X�^���N���A [32bit]
    reg_HOTSW_MCCNT1 = REGCLEAR_32;

    // �J�[�h����̃f�[�^�����܂郌�W�X�^���N���A [32bit]
    reg_HOTSW_MCD1 = REGCLEAR_32;

    // ����������H�����l�ݒ背�W�X�^���N���A [�e32bit]
    reg_HOTSW_MCSCR0 = REGCLEAR_32;
    reg_HOTSW_MCSCR1 = REGCLEAR_32;
    reg_HOTSW_MCSCR2 = REGCLEAR_32;

    // Slot Status,SWP���N���A [d15-d0 Slot Status,SWP]
    reg_MI_MC1 &= REG_MI_MC1_CC_MASK;
}


/*---------------------------------------------------------------------------*
  Name:        MonitorThread

  Description: ���ۂ̃J�[�h��Ԃ�HotSwThread�ŏ�Ԃ��ׂāA�Ⴂ���������ꍇ��
               ���b�Z�[�W�𑗂�

  s_isPulledOut : True  -> �J�[�h�Ȃ�       HOTSW_IsCardExist : True  -> �J�[�h����
                  False -> �J�[�h����                           False -> �J�[�h�Ȃ�
 *---------------------------------------------------------------------------*/
static void MonitorThread(void *arg)
{
    #pragma unused( arg )

    u32  count = 0;
    BOOL isCardExist;

    while(1){
        // �J�[�h�f�[�^���[�h���͑ҋ@
        do{
            OS_Sleep(CARD_EXIST_CHECK_POLLING_TIME);

            count++;
        }
        while( SYSMi_GetWork()->flags.hotsw.isBusyHotSW );

        // �|�[�����O�X���b�h�}���t���O���オ���Ă���A�X���[�v�B�}���t���O�����肽��N���B
        if(s_pollingThreadSleepFlg){
            OSMessage msg;
            OS_ReceiveMessage(&HotSwThreadData.hotswPollingCtrlQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);
        }

		s_isBusyMonitorThread = TRUE;
        
        isCardExist = HOTSW_IsCardExist();

        CheckCardPullOut(isCardExist);

        if(count >= CARD_INSERT_CHECK_INTERVAL){
            CheckCardInsert(isCardExist);
            count = 0;
        }

        s_isBusyMonitorThread = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:        CheckCardInsert

  Description: �J�[�h�͑}�����Ă���̂ɁA�����`���[���F�����ĂȂ������烁�b�Z�[�W�𑗂�
 *---------------------------------------------------------------------------*/
static void CheckCardInsert(BOOL cardExist)
{
    // �J�[�h�͑}�����Ă���̂ɁA�����`���[���F�����ĂȂ������烁�b�Z�[�W�𑗂�
    if(cardExist && s_isPulledOut){
        OSIntrMode enabled = OS_DisableInterrupts();

        SendInsertMessage();

        (void)OS_RestoreInterrupts( enabled );
    }
    // �J�[�h�͑}�����Ă��āA�����`���[���F�����Ă�����Game���[�h��ID�ǂ݂����āA�����ĂȂ������ׂ�
    else if(cardExist && !s_isPulledOut && !SYSMi_GetWork()->flags.hotsw.isBusyHotSW){
		OS_PutString("GameMode ID Check...\n");
        
        // �J�[�h�̃��b�N
    	CARD_LockRom(s_PollingLockID);

        ReadIDGame(&s_cbData);

        if(s_cbData.id_gam != s_gameID){
            OSIntrMode enabled;
            
			McPowerOff();

            enabled = OS_DisableInterrupts();
            
			PulledOutSequence();
			SendInsertMessage();
            
            (void)OS_RestoreInterrupts( enabled );
        }
        
	    // �J�[�h�̃��b�N�J��(�����b�NID�͊J�������Ɏ���������)
    	CARD_UnlockRom(s_PollingLockID);
    }
}


static void SendInsertMessage(void)
{
	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

	// ���b�Z�[�W���L���[�̐擪�ɓ����
	OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

	// ���b�Z�[�W�C���f�b�N�X���C���N�������g
	HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
}


/*---------------------------------------------------------------------------*
  Name:        CheckCardPullOut

  Description: �J�[�h�͔�����Ă���̂ɁA�����`���[���F�����Ă����烁�b�Z�[�W�𑗂�
 *---------------------------------------------------------------------------*/
static void CheckCardPullOut(BOOL cardExist)
{
    if(!cardExist && !s_isPulledOut){
        OSIntrMode enabled;

#ifndef HOTSW_DISABLE_FORCE_CARD_OFF
        {
            u32 mode = GetMcSlotMode();
            if(mode == SLOT_STATUS_MODE_01 || mode == SLOT_STATUS_MODE_10){
                SetMcSlotMode(SLOT_STATUS_MODE_11);
            }
        }
#endif
        enabled = OS_DisableInterrupts();

        HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
        HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
        HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;

        // ���b�Z�[�W���L���[�̐擪�ɓ����
        OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

        // ���b�Z�[�W�C���f�b�N�X���C���N�������g
        HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

        (void)OS_RestoreInterrupts( enabled );
    }
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCardDet

  Description:  �J�[�hB�}�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardDet(void)
{
    HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
    HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
    HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

    // ���b�Z�[�W���M
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

    // ���b�Z�[�W�C���f�b�N�X���C���N�������g
    HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

    HOTSW_PutString("��\n");
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackPxi

  Description:  PXI���荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
    HotSwPxiMessageForArm7 d;

    d.data = data;

#ifndef USE_WRAM_LOAD
    HOTSW_TPrintf("... Pxi Message - value:%x  ctrl:%x  finalize:%x  bootType:%x\n",
                                d.msg.value, d.msg.ctrl, d.msg.finalize, d.msg.cardState);
#else
    HOTSW_TPrintf("... Pxi Message - value:%x  ctrl:%x  finalize:%x  read:%x  bootType:%x\n",
                                d.msg.value, d.msg.ctrl, d.msg.finalize, d.msg.read, d.msg.cardState);

    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].read      = (d.msg.read) ? TRUE : FALSE;
#endif
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].ctrl      = (d.msg.ctrl) ? TRUE : FALSE;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].finalize  = (d.msg.finalize) ? TRUE : FALSE;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].value     = d.msg.value;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].state     = (HotSwCardState)d.msg.cardState;

    // ���b�Z�[�W���M
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl], OS_MESSAGE_NOBLOCK);

    // ���b�Z�[�W�C���f�b�N�X���C���N�������g
    HotSwThreadData.idx_ctrl = (HotSwThreadData.idx_ctrl+1) % HOTSW_CTRL_MSG_NUM;
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCardData

  Description:  �J�[�hB �f�[�^�]���I�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
#ifndef USE_NEW_DMA
static void InterruptCallbackCardData(void)
{
    // DMA�����I��
    MI_StopDma(HOTSW_DMA_NO);

    // ���b�Z�[�W���M
    OS_SendMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&HotSwThreadData.hotswDmaMsg[HotSwThreadData.idx_dma], OS_MESSAGE_NOBLOCK);

    // ���b�Z�[�W�C���f�b�N�X���C���N�������g
    HotSwThreadData.idx_dma = (HotSwThreadData.idx_dma+1) % HOTSW_DMA_MSG_NUM;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         SetInterruptCallback
                SetInterruptCallbackEx

  Description:  ���荞�݃R�[���o�b�N�֐��Ɗ��荞�݋��̐ݒ���s��
 *---------------------------------------------------------------------------*/
static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func )
{
    (void)OS_SetIrqFunction(intr_bit, func);
    (void)OS_EnableIrqMask(intr_bit);
}

static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func )
{
    (void)OS_SetIrqFunctionEx(intr_bit, func);
    (void)OS_EnableIrqMaskEx(intr_bit);
}


/*---------------------------------------------------------------------------*
  Name:         SetInterrupt

  Description:  ���荞�݃R�[���o�b�N�֐�����x�ɐݒ肷��֐�
 *---------------------------------------------------------------------------*/
static void SetInterrupt(void)
{
    SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );

#ifdef USE_NEW_DMA
    (void)OS_EnableIrqMask(OS_IE_NDMA2);
#else
    SetInterruptCallback( OS_IE_CARD_DATA  , InterruptCallbackCardData );
#endif
}


/*---------------------------------------------------------------------------*
  Name:         CheckStaticModuleHash

  Description:  �풓���W���[���̃n�b�V���`�F�b�N���s��
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
#include <twl/os/common/systemCall.h>
static HotSwState CheckStaticModuleHash(void)
{
    BOOL flg = TRUE;

    // Arm9�풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckArm9HashValue()){
        flg = FALSE;
        HOTSW_PutString("�~Arm9 Static Module Hash Check Error...\n");
    }

    // Arm7�풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckArm7HashValue()){
        flg = FALSE;
        HOTSW_PutString("�~Arm7 Static Module Hash Check Error...\n");
    }

    // Arm9�g���풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckExtArm9HashValue()){
        flg = FALSE;
        HOTSW_PutString("�~Arm9 Ltd Static Module Hash Check Error...\n");
    }

    // Arm7�g���풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckExtArm7HashValue()){
        flg = FALSE;
        HOTSW_PutString("�~Arm7 Ltd Static Module Hash Check Error...\n");
    }

    if(flg){
        HOTSW_PutString("*** Static Module Load was Completed!!\n");
    }

    return flg ? HOTSW_SUCCESS : HOTSW_HASH_CHECK_ERROR;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckArm7HashValue

  Description:  Arm7�풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckArm7HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;

    // �N���A
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ARM7�풓���W���[����Hash�l�ƍ�
    SVC_CalcHMACSHA1( sha1data,
                      (u32 *)(s_cbData.arm7Stc),
                      s_cbData.pBootSegBuf->rh.s.sub_size,
                      s_digestDefaultKey,
                      sizeof(s_digestDefaultKey) );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.sub_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckArm9HashValue

  Description:  Arm9�풓���W���[���̃n�b�V���`�F�b�N

  �� �擪2K�̕��������s����O�̃f�[�^�̃n�b�V�����ׂ�
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckArm9HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;
    SVCHMACSHA1Context hash;

    // �N���A
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // �n�b�V��������
    SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

    // �Z�L���A�̈敪UpDate
    SVC_HMACSHA1Update( &hash, s_cbData.pSecureSegBuf, SECURE_SEGMENT_SIZE );

    // �Q�[���̈敪UpDate
    SVC_HMACSHA1Update( &hash, (u32 *)(s_cbData.arm9Stc + SECURE_SEGMENT_SIZE), s_cbData.pBootSegBuf->rh.s.main_size - SECURE_SEGMENT_SIZE );

    // Hash�l�擾
    SVC_HMACSHA1GetHash( &hash, sha1data );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.main_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm7HashValue

  Description:  Arm7�g���풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckExtArm7HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;

    // �N���A
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ARM7�풓���W���[����Hash�l�ƍ�
    SVC_CalcHMACSHA1( sha1data,
                      (u32 *)s_cbData.arm7Ltd,
                      s_cbData.pBootSegBuf->rh.s.sub_ltd_size,
                      s_digestDefaultKey,
                      sizeof(s_digestDefaultKey) );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.sub_ltd_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm9HashValue

  Description:  Arm9�g���풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckExtArm9HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    u32     size;
    BOOL    retval = TRUE;
    SVCHMACSHA1Context hash;

    // Arm9�g���풓���W���[���̃Z�L���A�̈敪�̃T�C�Y���擾
    size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
             s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;

    // �N���A
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // �n�b�V��������
    SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

    // �Z�L���A�̈敪UpDate
    SVC_HMACSHA1Update( &hash, (u32 *)SYSM_CARD_TWL_SECURE_BUF, size );

    // �Q�[���̈敪UpDate (Arm9�g���풓���W���[����Secure�̈�Ŏ��܂��Ă��炱���͔�΂�)
    if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ){
        SVC_HMACSHA1Update( &hash, (u32 *)(s_cbData.arm9Ltd + SECURE_SEGMENT_SIZE), s_cbData.pBootSegBuf->rh.s.main_ltd_size - size );
    }

    // Hash�l�擾
    SVC_HMACSHA1GetHash( &hash, sha1data );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.main_ltd_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_GetCardBootData

  Description:
 *---------------------------------------------------------------------------*/
void *HOTSWi_GetCardBootData(void)
{
    return &s_cbData;
}


/*---------------------------------------------------------------------------*
  Name:         DebugPrintErrorMessage

  Description:
 *---------------------------------------------------------------------------*/
static void DebugPrintErrorMessage(HotSwState state)
{
    switch(state){
      case HOTSW_SUCCESS:
        HOTSW_PutString("   - Success\n");
        break;

      case HOTSW_TIME_OUT:
        HOTSW_PutString("   - Error 1 : TimeOut\n");
        break;

      case HOTSW_CARD_LOCK_ERROR:
        HOTSW_PutString("   - Error 2 : Slot Lock\n");
        break;

      case HOTSW_CRC_CHECK_ERROR:
        HOTSW_PutString("   - Error 3 : CRC Check\n");
        break;

      case HOWSW_REGION_CHECK_ERROR:
        HOTSW_PutString("   - Error 3 : Region Check\n");
        break;

      case HOTSW_HASH_CHECK_ERROR:
        HOTSW_PutString("   - Error 4 : Hash Check\n");
        break;

      case HOTSW_ID_CHECK_ERROR:
        HOTSW_PutString("   - Error 5 : ID Check\n");
        break;

      case HOTSW_PULLED_OUT_ERROR:
        HOTSW_PutString("   - Error 6 : Pulled Out\n");
        break;

      case HOTSW_DATA_DECRYPT_ERROR:
        HOTSW_PutString("   - Error 7 : Data Decrypt\n");
        break;

      case HOTSW_BUFFER_OVERRUN_ERROR:
        HOTSW_PutString("   - Error 8 : Buffer OverRun\n");
        break;

      case HOTSW_UNEXPECTED_ERROR:
        HOTSW_PutString("   - Error 9 : Unexpected\n");
        break;

      default :
        HOTSW_PutString("   - illigal Error\n");
        break;
    }
}