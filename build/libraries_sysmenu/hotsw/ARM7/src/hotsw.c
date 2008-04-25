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

#define DEBUG_MODE

//#define HOTSW_DISABLE_FORCE_CARD_OFF

// �J�[�h�d��ON����ROM�w�b�_���[�h�܂ł̊��ԂɃX���[�v�ɓ��鎞�̃����Z�O�΍􂵂Ȃ��ꍇ
//#define HOWSW_ENABLE_DEEP_SLEEP_WHILE_INSERT_CARD

// define -------------------------------------------------------------------
#define     CHATTERING_COUNTER                  0x1988      // 100ms�� (0x1988 * 15.3us = 100000us)
#define     COUNTER_A                           0x51C       //  20ms�� ( 0x51C * 15.3us =  20012us)

#define     CARD_EXIST_CHECK_INTERVAL           100

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

// Function prototype -------------------------------------------------------
static BOOL IsSwap(void);
static u32 GetMcSlotShift(void);
static u32 GetMcSlotMask(void);
static u32 GetMcSlotMode(void);
static void SetMcSlotMode(u32 mode);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackNDma(void);
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);

static void LockHotSwRsc(OSLockWord* word);
static void UnlockHotSwRsc(OSLockWord* word);

static void HotSwThread(void *arg);
static void MonitorThread(void *arg);

static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static BOOL isTwlModeLoad(void);
static HotSwState ReadSecureModeCardData(void);
static void ClearCaradFlgs(void);

static void RegisterRomEmuInfo(void);
static void GenVA_VB_VD(void);
static HotSwState DecryptObjectFile(void);
static HotSwState LoadBannerData(void);
static HotSwState LoadStaticModule(void);
static HotSwState LoadCardData(void);
static HotSwState CheckCardAuthCode(void);

static HotSwState CheckStaticModuleHash(void);

static s32 LockExCard(u16 lockID);
static s32 UnlockExCard(u16 lockID);

static BOOL CheckArm7HashValue(void);
static BOOL CheckArm9HashValue(void);
static BOOL CheckExtArm7HashValue(void);
static BOOL CheckExtArm9HashValue(void);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);
static void DebugPrintErrorMessage(HotSwState state);

HotSwState HOTSWi_RefreshBadBlock(u32 romMode);

// Static Values ------------------------------------------------------------
#include <twl/ltdwram_begin.h>

static char                 encrypt_object_key[] ATTRIBUTE_ALIGN(4) = "encryObj";

static u16                  s_RscLockID;
static u16                  s_CardLockID;
static u16                  s_bondingOp;

static u32                  s_BootSegBufSize, s_SecureSegBufSize, s_Secure2SegBufSize;

static BootSegmentData      *s_pBootSegBuffer;
static u32                  *s_pSecureSegBuffer;
static u32                  *s_pSecure2SegBuffer;

static CardBootData         s_cbData;
static SYSMRomEmuInfo       s_romEmuInfo;
static BOOL                 debuggerFlg;

static BOOL                 s_IsPulledOut = TRUE;
static BOOL                 s_isHotSwBusy = FALSE;

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
    // DS Card Type 1
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // DS Card Type 2
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // TWL Card Type 1
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // RomEmulation
    {ReadIDSecure_ROMEMU, ReadSegSecure_ROMEMU, SwitchONPNGSecure_ROMEMU, ChangeModeSecure_ROMEMU}
};

#ifdef DHT_TEST
#include <twl/os/ARM7/debugLED.h>
#include <sysmenu/dht/dht.h>
DHTFile* dht;
static DHTPhase2Work* p2work = (void*)0x02e80000;
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    HotSwState retval;
    if ( offset % 512 )
    {
        static u8 page_buffer[512];
        u32 page_offset = (u32)(offset & -512);
        u32 buffer_offset = (u32)(offset % 512);
        u32 valid_length = 512 - buffer_offset;
        retval = ReadPageGame((CardBootData*)arg, page_offset, page_buffer, 512);
        if (retval != HOTSW_SUCCESS)
        {
            return FALSE;
        }
        MI_CpuCopy8(page_buffer + buffer_offset, dest, (length < valid_length ? length : valid_length));
        dest = (u8*)dest + valid_length;
        offset += valid_length;
        length -= valid_length;
        if ( length < 0)
        {
            return TRUE;
        }
    }
    retval = ReadPageGame((CardBootData*)arg, (u32)offset, dest, (u32)length);
    return (retval == HOTSW_SUCCESS);
}
#endif

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

#ifndef USE_LOCAL_KEYTABLE
    // ��������ɑ��̗p�r��WRAM_0���g�p�ł���悤�Ƀ��[�J���o�b�t�@�փR�s�[���Ă���
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &HotSwBlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
#endif
    // PXI������
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);

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
            OS_PutString("Error - Can't Get Lock ID\n");
        }
        s_RscLockID = (u16)tempLockID;

        // �J�[�h�A�N�Z�X�p�̃��b�NID�̎擾
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            OS_PutString("Error - Can't Get Lock ID\n");
        }
        s_CardLockID = (u16)tempLockID;
    }

    // �J�[�h�f�[�^���[�h�p�X���b�h�̐���
    OS_CreateThread(&HotSwThreadData.hotswThread,
                    HotSwThread,
                    NULL,
                    HotSwThreadData.hotswStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // �J�[�h�̏�ԊĎ��p�X���b�h�̐��� ( DS�e���r�΍� )
    // [TODO] �D��x�̐ݒ�
    OS_CreateThread(&HotSwThreadData.monitorThread,
                    MonitorThread,
                    NULL,
                    HotSwThreadData.monitorStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // ���b�Z�[�W�L���[�̏�����
    OS_InitMessageQueue( &HotSwThreadData.hotswQueue, &HotSwThreadData.hotswMsgBuffer[0], HOTSW_MSG_BUFFER_NUM );

    // ���b�Z�[�W�L���[�̏�����
    OS_InitMessageQueue( &HotSwThreadData.hotswDmaQueue, &HotSwThreadData.hotswDmaMsgBuffer[0], HOTSW_DMA_MSG_NUM );

    // �X���b�h�N��
    OS_WakeupThreadDirect(&HotSwThreadData.hotswThread);
    OS_WakeupThreadDirect(&HotSwThreadData.monitorThread);

    // Boot Segment �o�b�t�@�̐ݒ�
    HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure1 Segment �o�b�t�@�̐ݒ�
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE1, (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );

    // Secure2 Segment �o�b�t�@�̐ݒ�
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
  Name:         LoadCardData

  Description:  �J�[�h����f�[�^�����[�h����

  ��BootSegmentBuffer SecureSegmentBuffer�̐ݒ���s���Ă���
  ���̊֐����Ă�ł��������B
 *---------------------------------------------------------------------------*/
static HotSwState LoadCardData(void)
{
    OSTick start;
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 romMode = HOTSW_ROM_MODE_NULL;

    start = OS_GetTick();

    s_isHotSwBusy = TRUE;

    // �J�[�h�̃��b�N
#ifndef DEBUG_USED_CARD_SLOT_B_
    CARD_LockRom(s_CardLockID);
#else
    LockExCard(s_CardLockID);
#endif

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
                state  = ReadRomEmulationInfo(&s_romEmuInfo);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // �擾����Rom�G�~�����[�V���������r
                debuggerFlg = TRUE;
                if ( romEmuInfo->magic_code != SYSM_ROMEMU_INFO_MAGIC_CODE ){
                    debuggerFlg = FALSE;
                }
            }

            // �����Rom�G�~�����[�V���������g�p
            if(HOTSWi_IsRomEmulation()){
                OS_PutString("Read Emulation ROM\n");
                s_cbData.cardType = ROM_EMULATION;
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
                OS_TPrintf("SYSMi_GetWork()->gameCommondParam : 0x%08x\n", s_cbData.gameCommondParam);
            }
            else{
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
            }
            SYSMi_GetWork()->gameCommondParam = s_cbData.gameCommondParam;

            // ROM�w�b�_CRC���Z�o���ă`�F�b�N�BNintendo���SCRC���m�F�B
            SYSMi_GetWork()->cardHeaderCrc16_bak = SVC_GetCRC16( 65535, s_cbData.pBootSegBuf, 0x015e );
            OS_TPrintf( "RomHeaderCRC16 : calc = %04x  romh = %04x\n",
                        SYSMi_GetWork()->cardHeaderCrc16_bak, s_cbData.pBootSegBuf->rh.s.header_crc16 );

            if( ( SYSMi_GetWork()->cardHeaderCrc16_bak != s_cbData.pBootSegBuf->rh.s.header_crc16 ) ||
                ( 0xcf56 != s_cbData.pBootSegBuf->rh.s.nintendo_logo_crc16 ) ){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_CRC_CHECK_ERROR : retval;
            }

            // �A�v���W�����v�̃f�o�b�O����ROM�G�~�����[�V������񂾂��K�v�ȏꍇ
            if(SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly){
                SYSMi_GetWork()->flags.hotsw.isExistCard = TRUE;
                RegisterRomEmuInfo();
                // �r�����䂱���܂�
                UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                goto end;
            }
        }

        if( retval == HOTSW_SUCCESS ) {
            // NTR�J�[�h��TWL�J�[�h��
#ifdef DEBUG_MODE
            if(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset && s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset)
#else
            if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02)
#endif
            {
                OS_TPrintf("TWL Card.\n");
                s_cbData.twlFlg = TRUE;
            }
            else{
#ifdef DHT_TEST
                if ( !s_cbData.pBootSegBuf->rh.s.enable_nitro_whitelist_signature )
#endif
                // NTR�J�[�h�̏ꍇ��RomHeader�o�b�t�@��1�y�[�W�ڈȍ~���N���A���Ă����B
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_CARD_ROM_HEADER_SIZE - PAGE_SIZE);
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
                OS_PutString("Read Mode : TwlCard\n");

               // Mode2�Ɉڍs���鏀��
                s_cbData.modeType = HOTSW_MODE2;

                // ---------------------- Reset ----------------------
                McPowerOff();
                McPowerOn();

                // ---------------------- Normal Mode ----------------------
                state  = ReadIDNormal(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // �擪1Page��������OK�B�f�[�^�͓ǂݎ̂ăo�b�t�@��
                state  = ReadBootSegNormal(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // Key Table������
                MakeBlowfishTableDS(&s_cbData, 8);

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
            state  = s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ---------------------- Game Mode ----------------------
            romMode = HOTSW_ROM_MODE_GAME;

            // ID�ǂݍ���
            state  = ReadIDGame(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // �o�i�[�t�@�C���̓ǂݍ���
            state  = LoadBannerData();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // �r�����䂱���܂�(��CRC�`�F�b�N�܂łɃ~�X���Ȃ�������A�r�����䂱���܂�)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
            if(s_cbData.id_scr != s_cbData.id_gam){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
                goto finalize;
            }

            // �풓���W���[���c����w���ɓ]��
            state  = LoadStaticModule();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ARM9�풓���W���[���̐擪2KB�̈Í����̈�𕡍���
            (void)DecryptObjectFile();

            // �F�؃R�[�h�ǂݍ��݁����[�N�̈�ɃR�s�[
            state  = CheckCardAuthCode();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
        else{
            // �r�����䂱���܂�(��CRC�`�F�b�N�܂łɃ~�X����������A�����ŊJ������)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            OS_TPrintf("ng... Card Data Load Skip ( state : %d )\n", retval);
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
    HOTSW_WaitDmaCtrl(HOTSW_NDMA_NO);

    // �J�[�h�A�N�Z�X�I���m�F
    HOTSW_WaitCardCtrl();

    // �J�[�h�̃��b�N�J��(�����b�NID�͊J�������Ɏ���������)
#ifndef DEBUG_USED_CARD_SLOT_B_
    CARD_UnlockRom(s_CardLockID);
#else
    UnlockExCard(s_CardLockID);
#endif

    s_isHotSwBusy = FALSE;

//  OS_TPrintf( "Load Card Time : %dms\n\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         isTwlModeLoad

  Description:  �J�[�h�̃��[�h��TWL���[�h�ōs����DS���[�h�ōs���������߂�


  �� Bonding Op = 0 (���i��)
    �\�t�g�E�F�A |  �@�@�@ DS�J�[�h         |          TWL�J�[�h
    ------------------------------------------------------------------------
        DS�p     |  DS�J�[�h�ǂ݃V�[�P���X  |    DS�J�[�h�ǂ݃V�[�P���X(��1)
       TWL�p     |  �s���J�[�h�t���O����    |    TWL�J�[�h�ǂ݃V�[�P���X
    �n�C�u���b�g |  �s���J�[�h�t���O����    |    TWL�J�[�h�ǂ݃V�[�P���X

  �� Bonding Op = 0�ȊO (�J���p)
    �\�t�g�E�F�A |  �@�@�@ DS�J�[�h         |          TWL�J�[�h
    ------------------------------------------------------------------------
        DS�p     |  DS�J�[�h�ǂ݃V�[�P���X  |    DS�J�[�h�ǂ݃V�[�P���X(��1)
       TWL�p     |  DS�J�[�h�ǂ݃V�[�P���X  |    TWL�J�[�h�ǂ݃V�[�P���X
    �n�C�u���b�g |  DS�J�[�h�ǂ݃V�[�P���X  |    TWL�J�[�h�ǂ݃V�[�P���X


  ��1 [TODO] ����Ɋւ��Ă̓J�[�hG�Ƒ��k���Č��߂�
 *---------------------------------------------------------------------------*/
static BOOL isTwlModeLoad(void)
{
    // TWL�J�[�h
    if(s_cbData.id_nml & HOTSW_ROMID_TWLROM_MASK){
        // PlatformCode��Twl or Hybrid�̏ꍇ
        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
            OS_PutString("TWL Card : TWL Application : Read Sequence -> TWL\n");
            return TRUE;
        }
        else{
            // [TODO] �d�l�m�F
            OS_PutString("TWL Card : NTR Application : Read Sequence -> NTR\n");
            return FALSE;
        }
    }
    // DS�J�[�h
    else{
        // ���i�ł̏ꍇ
        if(s_bondingOp == SCFG_OP_PRODUCT){
            // PlatformCode��Twl or Hybrid�̏ꍇ
            if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
                OS_PutString("NTR Card : TWL Application : Illegal Card\n");
                s_cbData.illegalCardFlg = TRUE;
                return FALSE;
            }
            else{
                OS_PutString("NTR Card : NTR Application : Read Sequence -> NTR\n");
                return FALSE;
            }
        }
        // �J���p�̏ꍇ
        else{
            OS_PutString("Bonding Option Development : NTR Card : Read Sequence -> NTR\n");
            return FALSE;
        }
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

    // PNG�ݒ�
    state  = s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // DS������������H�����l�ݒ� (���W�X�^�ݒ�)
    SetMCSCR();

    // ID�ǂݍ���
    state  = s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
    if(s_cbData.id_nml != s_cbData.id_scr){
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
    }

    if(retval == HOTSW_SUCCESS){
        // Secure�̈��Segment�ǂݍ���
        state  = s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData);
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
        }
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_IsRunOnDebugger

  Description:  IS�f�o�b�K��œ��삵�Ă��邩�H
 *---------------------------------------------------------------------------*/
BOOL HOTSWi_IsRunOnDebugger(void)
{
    return debuggerFlg;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_IsRomEmulation

  Description:  ROM���G�~�����[�V�������Ă��邩�H
 *---------------------------------------------------------------------------*/
BOOL HOTSWi_IsRomEmulation(void)
{
    return debuggerFlg &&
#ifndef DEBUG_USED_CARD_SLOT_B_
           s_romEmuInfo.isEnableSlot1;
#else
           s_romEmuInfo.isEnableSlot2;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         RegisterRomEmuInfo

  Description:  ROM�G�~�����[�V��������o�^
 *---------------------------------------------------------------------------*/
static void RegisterRomEmuInfo(void)
{
    SYSM_work* sw = SYSMi_GetWork();

    // �f�o�b�K���
    if ( ! sw->flags.hotsw.is1stCardChecked && debuggerFlg )
    {
        MI_CpuCopy8( &s_romEmuInfo, &(sw->romEmuInfo), ROM_EMULATION_DATA_SIZE );
        sw->flags.hotsw.isOnDebugger = debuggerFlg;
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
  Name:         LoadStaticModule

  Description:  ARM7,9�̏풓���W���[����W�J����֐�

  ���F�Q�[�����[�h�ɂȂ��Ă���Ăяo���Ă�������
 *---------------------------------------------------------------------------*/
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
#ifdef DHT_TEST
    else
    {
        SVCHMACSHA1Context ctx;
        const u8* hash0;
        const u8* hash1;
        if ( !s_cbData.pBootSegBuf->rh.s.enable_nitro_whitelist_signature ) // �z���C�g���X�g�G���g��
        {
            const DHTDatabase* db;
            while (!dht)
            {
                OS_Sleep(1);
            }

            OS_TPrintf("Search DHT...");
            db = DHT_GetDatabase(dht, &s_cbData.pBootSegBuf->rh.s);
            if ( !db )
            {
                OS_TPrintf(" Failed.\n");
OS_SetDebugLED(0xFF);
while(1){ OS_WaitVBlankIntr(); }
                return HOTSW_HASH_CHECK_ERROR;
            }
            OS_TPrintf(" Done.\n");
            hash0 = db->hash[0];
            hash1 = db->hash[1];
        }
        else    // �}�X�^�����O�ς݃G���g��
        {
            hash0 = s_cbData.pBootSegBuf->rh.s.nitro_whitelist_phase1_digest;
            hash1 = s_cbData.pBootSegBuf->rh.s.nitro_whitelist_phase2_diegst;
        }

        OS_TPrintf("DHT Pahse1...");
        DHT_CheckHashPhase1Init(&ctx, (ROM_Header_Short*)s_cbData.pBootSegBuf);
        if( s_cbData.pBootSegBuf->rh.s.main_size > SECURE_SEGMENT_SIZE )
        {
            DHT_CheckHashPhase1Update(&ctx, s_cbData.pSecureSegBuf, SECURE_SEGMENT_SIZE);
            DHT_CheckHashPhase1Update(&ctx, (u32 *)(s_cbData.arm9Stc + SECURE_SEGMENT_SIZE), s_cbData.pBootSegBuf->rh.s.main_size - SECURE_SEGMENT_SIZE );
        }
        else
        {
            DHT_CheckHashPhase1Update(&ctx, s_cbData.pSecureSegBuf, s_cbData.pBootSegBuf->rh.s.main_size);
        }

        DHT_CheckHashPhase1Update(&ctx, (u32 *)s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size);
        if ( !DHT_CheckHashPhase1Final(&ctx, hash0) )
        {
            OS_TPrintf(" Failed.\n");
OS_SetDebugLED(0xAA);
while(1){ OS_WaitVBlankIntr(); }
            return HOTSW_HASH_CHECK_ERROR;
        }
        OS_TPrintf(" Done.\n");

        OS_TPrintf("DHT Pahse2...");
        if ( !DHT_CheckHashPhase2(hash1, &s_cbData.pBootSegBuf->rh.s, p2work, ReadImage, &s_cbData) )
        {
            OS_TPrintf(" Failed.\n");
OS_SetDebugLED(0xCC);
while(1){ OS_WaitVBlankIntr(); }
            return HOTSW_HASH_CHECK_ERROR;
        }
        OS_TPrintf(" Done.\n");
    }
#endif

    return retval;
}


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
        OS_PutString("  �� Clone Boot Mode\n");
        SYSMi_GetWork()->cloneBootMode = SYSM_CLONE_BOOT_MODE;
    }else {
        OS_PutString("  �� Other Boot Mode\n");
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
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);

    if(type == HOTSW_MODE1){
        s_pSecureSegBuffer = (u32 *)buf;
        s_SecureSegBufSize = size;

        s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

        // �o�b�t�@�̏�����
        MI_CpuClear8(s_pSecureSegBuffer, size);
    }
    else{
        s_pSecure2SegBuffer = (u32 *)buf;
        s_Secure2SegBufSize = size;

        s_cbData.pSecure2SegBuf = s_pSecure2SegBuffer;

        // �o�b�t�@�̏�����
        MI_CpuClear8(s_pSecure2SegBuffer, size);
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

    // ������������
    // �`�b�N��RTC���񃍁[�h�l����Ƃ���B
    // �i�N������x�ɕω�����p�����[�^�Ƒg�ݍ��킹��B
    //   V�J�E���^��2130�T�C�N���ω����Ȃ��̂ŌŒ�l�ɂȂ�₷���B�j
    MATH_InitRand32(&rnd, OS_GetTick() ^ sw->Rtc1stData.words[0] ^ sw->Rtc1stData.words[1]);

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

  ���F�Z�L���A�̈��ǂݍ���ł��炱�̊֐����Ăяo���Ă�������
 *---------------------------------------------------------------------------*/
static u32 encDestBuf[ENCRYPT_DEF_SIZE/sizeof(u32)];

static HotSwState DecryptObjectFile(void)
{
    u8  i;
    s32 restSize;
    s32 size                = (s32)s_cbData.pBootSegBuf->rh.s.main_size;
    u32 *pEncBuf            = encDestBuf;
    u32 *pEncDes            = s_cbData.pSecureSegBuf;
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
    BOOL exist              = TRUE;
    HotSwState retval       = HOTSW_SUCCESS;

    if (size > ENCRYPT_DEF_SIZE) {
        size = ENCRYPT_DEF_SIZE;
    }
    restSize = size;

    // �ǂݍ��񂾃Z�L���A�̈���o�b�t�@����ꎞ�o�b�t�@�ɃR�s�[
    MI_CpuCopy32(s_cbData.pSecureSegBuf, pEncBuf, (u32)size);

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
        retval = HOTSW_DATA_DECRYPT_ERROR;

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
#ifndef DEBUG_USED_CARD_SLOT_B_
    u32 mask = (u32)(REG_MI_MC_SL1_CDET_MASK << GetMcSlotShift());
#else
    u32 mask = (u32)(REG_MI_MC_SL2_CDET_MASK >> GetMcSlotShift());
#endif

    if( !(reg_MI_MC1 & mask) ){
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
  Name:         IsSwap

  Description:  SCFG_MC1��SWP�t���O�����āA�X���b�g���X���b�v����Ă��邩���肷��
 *---------------------------------------------------------------------------*/
static BOOL IsSwap(void)
{
    if( reg_MI_MC1 & REG_MI_MC1_SWP_MASK ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotShift

  Description:  �J�[�h�X���b�g�̃V�t�g�r�b�g���̎擾
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotShift(void)
{
    return (u32)(IsSwap() * REG_MI_MC_SL2_CDET_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotMask

  Description:  �J�[�h�X���b�g�̃V�t�g�r�b�g���̎擾
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotMask(void)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    return (u32)(REG_MI_MC_SL1_MODE_MASK << GetMcSlotShift());
#else
    return (u32)(REG_MI_MC_SL2_MODE_MASK >> GetMcSlotShift());
#endif
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotMode

  Description:  �X���b�g�̌��݂̃��[�h��Ԃ�
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotMode(void)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    return (reg_MI_MC1 & GetMcSlotMask()) >> GetMcSlotShift();
#else
    return (reg_MI_MC1 & GetMcSlotMask()) << GetMcSlotShift();
#endif
}


/*---------------------------------------------------------------------------*
  Name:         SetMcSlotMode

  Description:  �J�[�h�X���b�g�̃��[�h�ݒ�
 *---------------------------------------------------------------------------*/
static void SetMcSlotMode(u32 mode)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~GetMcSlotMask()) | (mode << GetMcSlotShift()));
#else
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~GetMcSlotMask()) | (mode >> GetMcSlotShift()));
#endif
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
        // [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
        // VDD�̈�����ԑ҂�
        OS_Sleep(100);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 01 �ɂ���
        SetMcSlotMode(SLOT_STATUS_MODE_01);
        // 10ms�҂�
        OS_Sleep(10);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 10 �ɂ���
        SetMcSlotMode(SLOT_STATUS_MODE_10);

        // [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
        // RESB���グ��܂ł̑҂�����
        OS_Sleep(1);

        // ���Z�b�g��high�� (RESB = 1�ɂ���)
        reg_HOTSW_MCCNT1 = RESB_MASK;

        // [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
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
        OS_SpinWait( OS_MSEC_TO_CPUCYC(1) );
    }

    if ( (reg_MI_MC & mask) == (0x0 << shift) )
    {
        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x1 << shift));
        // VDD�̈������
        OS_SpinWait( OS_MSEC_TO_CPUCYC(100) );

        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x2 << shift));
        // RESB��Low����
        OS_SpinWait( OS_MSEC_TO_CPUCYC(1) );

        reg_MI_MCCNT1_B = REG_MI_MCCNT1_RESB_MASK;

        // RESB��High����
        OS_SpinWait( OS_MSEC_TO_CPUCYC(100) );
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

  [TODO:]�}���̃t���[�E�t���O�P�Aetc�̊m�F(���̏��A�����}�����Ă��^�C�g�����X�V����Ȃ�)
 *---------------------------------------------------------------------------*/
static void HotSwThread(void *arg)
{
    #pragma unused( arg )

    HotSwState      retval;
    HotSwMessage    *msg;

    while(1){
        OS_ReceiveMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        if( msg->ctrl == TRUE ) {
            // [TODO]�Ƃ肠�����A������HOTSW��}���������_��isExistCard��FALSE�Ȃ�AHOTSW��Finalize������悤�ɂ���B
            SYSMi_GetWork()->flags.hotsw.isEnableHotSW = msg->value;
            // [TODO]�J�[�h������Ƃ��ƂȂ��Ƃ��ŏꍇ��������Finalize����������
            //       PXI���b�Z�[�W���u�}���v�ƁuFinalize�v�ŕ����ď���
            //          �� �P�D�S�Ẵ��W�X�^���N���A����           (�J�[�h���������Ă��Ȃ���)
            //             �Q�D��x�d���𗎂Ƃ���Nomal���[�h�ɂ���  (NAND�A�v�������N������ꍇ)
            //             �R�D�K�v�ȃ��W�X�^���c���āA��̓N���A   (�������Ă���J�[�h���N������ꍇ)
            //
            // NAND�A�v���N�����̏I���������m�F�ł�����A�ABOOTAPI��KillThread�ŃX���b�h���E����悤�ɂ��Ă����B
            // �X���b�h���E���O�ɁAIREQ��DET���荞�݂𖳌��ɂ��Ă����B
//          HOTSW_Finalize();
        }

        while(1){
            if( !SYSMi_GetWork()->flags.hotsw.isEnableHotSW ) {
                SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
                OS_PutString("### HotSw is restrained...\n");
                break;
            }

            if(HOTSW_IsCardExist()){
                if(!s_IsPulledOut){
                    if(GetMcSlotMode() == SLOT_STATUS_MODE_10){
                        LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                        SYSMi_GetWork()->flags.hotsw.isExistCard         = TRUE;
                        SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;

                        SYSMi_GetWork()->nCardID = s_cbData.id_gam;

                        SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = TRUE;

                        UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                        OS_PutString("ok!\n");

                        break;
                    }
                }

                retval = LoadCardData();

                DebugPrintErrorMessage(retval);

                if(retval != HOTSW_SUCCESS){
                    McPowerOff();

                    ClearCaradFlgs();

                    s_IsPulledOut = TRUE;

                    break;
                }

                s_IsPulledOut = FALSE;
            }

            // �J�[�h�������Ă���
            else{
                ClearCaradFlgs();

                MI_CpuClear32(&s_cbData, sizeof(CardBootData));

                MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
                MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
                MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));

                s_IsPulledOut = TRUE;

                // �����Z�O�̃X���[�v���V���b�g�_�E���΍��߂�
                MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, TRUE );

                break;
            }
        }
        SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
    } // while loop
}


/*---------------------------------------------------------------------------*
  Name:        ClearCaradFlgs

  Description: �J�[�h�f�[�^�o�b�t�@�̃o�b�t�@�ƃt���O�̌�n��
 *---------------------------------------------------------------------------*/
static void ClearCaradFlgs(void)
{
    // �t���O����
    LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
    SYSMi_GetWork()->flags.hotsw.isExistCard         = FALSE;
    SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
    SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
    SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
    UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
}


/*---------------------------------------------------------------------------*
  [TODO:] Finalize�����̑O�ɂ��ׂ�����

�ENAND�A�v���N�����A�J�[�h�d��OFF

�ETWL NAND�A�v���Ȃ�
        �Q�[���J�[�hON�t���O(NAND��RomHeader TWL�g�����t���O)�����āA
        NAND�A�v���N������J�[�h�d��(OFF��)ON�ɂ���Normal���[�h�ɂ���

�ENTR NAND�A�v���Ȃ�
        �J�[�h��ʖ�킸�AGAME���[�h�ɑJ��(NTR�݊���RomHeader�̂ݓǂ�)
        �g���̈�c���Ă����Ă��悢����
        TWL�J�[�h���������Ă��āAGame2���[�h�ɂȂ��Ă�����A
        �d��OFF����ON���āAGame���[�h�ɑJ�ڂ�����B
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:        MonitorThread

  Description: ���ۂ̃J�[�h��Ԃ�HotSwThread�ŏ�Ԃ��ׂāA�Ⴂ���������ꍇ��
               ���b�Z�[�W�𑗂�

  s_IsPulledOut : True  -> �J�[�h�Ȃ�       HOTSW_IsCardExist : True  -> �J�[�h����
                  False -> �J�[�h����                           False -> �J�[�h�Ȃ�
 *---------------------------------------------------------------------------*/
static void MonitorThread(void *arg)
{
    #pragma unused( arg )

    BOOL isPullOutNow;

    while(1){
        // �J�[�h�f�[�^���[�h���͑ҋ@
        do{
            OS_Sleep(CARD_EXIST_CHECK_INTERVAL);
        }
        while(s_isHotSwBusy);

        // ���݃J�[�h�������Ă��邩
        isPullOutNow = !HOTSW_IsCardExist();

        // ��Ԃ̔�r
        if(s_IsPulledOut != isPullOutNow){
            OSIntrMode enabled = OS_DisableInterrupts();

            // �{���͔����Ă��ꍇ
            if(isPullOutNow){
                HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
                HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
                HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;

                // ���b�Z�[�W���L���[�̐擪�ɓ����
                OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

                // ���b�Z�[�W�C���f�b�N�X���C���N�������g
                HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

                OS_PutString(">>> Card State Error : PulledOut\n");
            }

            // �{���͑}�����Ă����ꍇ
            else{
                HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
                HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
                HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

                // ���b�Z�[�W���L���[�̐擪�ɓ����
                OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

                // ���b�Z�[�W�C���f�b�N�X���C���N�������g
                HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

                OS_PutString(">>> Card State Error : Insert\n");
            }

            (void)OS_RestoreInterrupts( enabled );
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCard

  Description: �J�[�hB�������荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCard(void)
{
    // �X���b�g�d��ON���͋���OFF
    // �i�G�~�����[�V����ROM�͋��R�l�N�^�ł̋���OFF�Ńf�[�^�������邱�Ƃ�����j
#ifndef HOTSW_DISABLE_FORCE_CARD_OFF
//    if ( ! HOTSWi_IsRomEmulation() )
    {
        u32 mode = GetMcSlotMode();
        if(mode == SLOT_STATUS_MODE_01 || mode == SLOT_STATUS_MODE_10){
            SetMcSlotMode(SLOT_STATUS_MODE_11);
        }
        OS_TPrintf("slot status: %x\n", mode);
    }
#endif

    HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
    HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
    HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;

    // ���b�Z�[�W���M
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

    // ���b�Z�[�W�C���f�b�N�X���C���N�������g
    HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

    OS_PutString("��\n");
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

    OS_PutString("��\n");
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackPxi

  Description:  PXI���荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
    HotSwPxiMessage d;

    d.data = data;

    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].ctrl  = (d.msg.ctrl) ? TRUE : FALSE;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].value = d.msg.value;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].type  = HOTSW_CONTROL;

    // ���b�Z�[�W���M
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl], OS_MESSAGE_NOBLOCK);

    // ���b�Z�[�W�C���f�b�N�X���C���N�������g
    HotSwThreadData.idx_ctrl = (HotSwThreadData.idx_ctrl+1) % HOTSW_CTRL_MSG_NUM;
}


/*---------------------------------------------------------------------------*
  Name:         AllocateExCardBus

  Description:  �A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9
static inline void SetExCardProcessor(MIProcessor proc)
{
    reg_HOTSW_EXMEMCNT =
        (u16)((reg_HOTSW_EXMEMCNT & ~HOTSW_EXMEMCNT_SELB_MASK) | (proc << HOTSW_EXMEMCNT_SELB_SHIFT));
}
#endif


/*---------------------------------------------------------------------------*
  Name:         AllocateExCardBus

  Description:  �X���b�gB�ւ̃A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
static void AllocateExCardBus(void)
{
#ifdef  SDK_ARM9
    // preset reset flag with status of disable interrupts in OSi_DoTryLockByWord
    if ( ! ( reg_MI_MC & REG_MI_MC_SL2_CDET_MASK ) )
    {
        reg_MI_MCCNT1 |= REG_MI_MCCNT2_RESB_MASK;
    }
    SetExCardProcessor(MI_PROCESSOR_ARM9);    // Arm9���œ��삵�Ă���ꍇ
#endif
}


/*---------------------------------------------------------------------------*
  Name:         FreeExCardBus

  Description:  �X���b�gB�ւ̃A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
static void FreeExCardBus(void)
{
#ifdef  SDK_ARM9
    SetExCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
#endif
}


/*---------------------------------------------------------------------------*
  Name:         LockSlotB

  Description:  �X���b�gB�����b�N����
 *---------------------------------------------------------------------------*/
static s32 LockExCard(u16 lockID)
{
    return OS_LockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, AllocateExCardBus);
}


/*---------------------------------------------------------------------------*
  Name:         UnlockSlotB

  Description:  �X���b�gB�����b�N����
 *---------------------------------------------------------------------------*/
static s32 UnlockExCard(u16 lockID)
{
    return OS_UnlockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, FreeExCardBus);
}


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
#ifndef DEBUG_USED_CARD_SLOT_B_
    SetInterruptCallback( OS_IE_CARD_A_IREQ , InterruptCallbackCard );
    SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );
    (void)OS_EnableIrqMask(OS_IE_NDMA2);
#else
    SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
    SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
    (void)OS_EnableIrqMask(OS_IE_NDMA2);
#endif
}


#include <twl/os/common/systemCall.h>
/*---------------------------------------------------------------------------*
  Name:         CheckStaticModuleHash

  Description:  �풓���W���[���̃n�b�V���`�F�b�N���s��
 *---------------------------------------------------------------------------*/
static HotSwState CheckStaticModuleHash(void)
{
    BOOL flg = TRUE;

    // Arm9�풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckArm9HashValue()){
        flg = FALSE;
        OS_PutString("�~Arm9 Static Module Hash Check Error...\n");
    }

    // Arm7�풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckArm7HashValue()){
        flg = FALSE;
        OS_PutString("�~Arm7 Static Module Hash Check Error...\n");
    }

    // Arm9�g���풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckExtArm9HashValue()){
        flg = FALSE;
        OS_PutString("�~Arm9 Ltd Static Module Hash Check Error...\n");
    }

    // Arm7�g���풓���W���[�� Hash�l�̃`�F�b�N
    if(!CheckExtArm7HashValue()){
        flg = FALSE;
        OS_PutString("�~Arm7 Ltd Static Module Hash Check Error...\n");
    }

    if(flg){
        OS_PutString("*** Static Module Load was Completed!!\n");
    }

    return flg ? HOTSW_SUCCESS : HOTSW_HASH_CHECK_ERROR;
}


/*---------------------------------------------------------------------------*
  Name:         CheckArm7HashValue

  Description:  Arm7�풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------*
  Name:         CheckArm9HashValue

  Description:  Arm9�풓���W���[���̃n�b�V���`�F�b�N

  �� �擪2K�̕��������s����O�̃f�[�^�̃n�b�V�����ׂ�
 *---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm7HashValue

  Description:  Arm7�g���풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm9HashValue

  Description:  Arm9�g���풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
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






// **************************************************************************
//
//                             Debug�p�\���֐�
//
// **************************************************************************
/*---------------------------------------------------------------------------*
  Name:         ShowRomHeaderData

  Description:
 *---------------------------------------------------------------------------*/
static void ShowRomHeaderData(void)
{
    OS_TPrintf("\nDebug Data -------------------------------\n");
    OS_TPrintf("1. Normal Mode ID  : 0x%08x\n"  , s_cbData.id_nml);
    OS_TPrintf("2. Secure Mode ID  : 0x%08x\n"  , s_cbData.id_scr);
    OS_TPrintf("3. Game   Mode ID  : 0x%08x\n"  , s_cbData.id_gam);

    OS_TPrintf("title Name         : %s\n",     s_pBootSegBuffer->rh.s.title_name);
    OS_TPrintf("initial Code       : %x\n\n",   *(u32 *)s_pBootSegBuffer->rh.s.game_code);

    OS_TPrintf("platform Code      : 0x%02x\n\n", s_cbData.pBootSegBuf->rh.s.platform_code);

    OS_TPrintf("main rom offset    : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_rom_offset);
    OS_TPrintf("main entry addr    : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_entry_address);
    OS_TPrintf("main ram   addr    : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_ram_address);
    OS_TPrintf("main size          : 0x%08x\n\n", s_cbData.pBootSegBuf->rh.s.main_size);

    OS_TPrintf("sub  rom offset    : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_rom_offset);
    OS_TPrintf("sub  entry addr    : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_entry_address);
    OS_TPrintf("sub  ram   addr    : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_ram_address);
    OS_TPrintf("sub  size          : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_size);

    if(s_cbData.twlFlg){
    OS_TPrintf("\nLtd main rom offset: 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset);
    OS_TPrintf("Ltd main ram   addr: 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address);
    OS_TPrintf("Ltd main size      : 0x%08x\n\n", s_cbData.pBootSegBuf->rh.s.main_ltd_size);

    OS_TPrintf("Ltd Sub rom offset : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset);
    OS_TPrintf("Ltd Sub ram   addr : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address);
    OS_TPrintf("Ltd Sub size       : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_ltd_size);
    }

    OS_TPrintf("------------------------------------------\n\n");
}

/*---------------------------------------------------------------------------*
  Name:         ShowRegisterData

  Description:
 *---------------------------------------------------------------------------*/
static void ShowRegisterData(void)
{
    OS_TPrintf("----------------------------------------------------------\n");
    OS_TPrintf("�g���@�\���䃌�W�X�^         (MC_B(d24))   : %08x\n", reg_SCFG_EXT);
    OS_TPrintf("MC I/F���䃌�W�X�^�P         (slot status) : %08x\n", reg_MI_MC1);
    OS_TPrintf("MC I/F���䃌�W�X�^�Q         (Counter-A)   : %04x\n", reg_MI_MC2);
    OS_TPrintf("MC �R���g���[�����W�X�^0     (SEL etc)     : %04x\n", reg_HOTSW_MCCNT0);
    OS_TPrintf("MC �R���g���[�����W�X�^1     (START etc)   : %08x\n", reg_HOTSW_MCCNT1);
    OS_TPrintf("----------------------------------------------------------\n");
}

/*---------------------------------------------------------------------------*
  Name:         DebugPrintErrorMessage

  Description:
 *---------------------------------------------------------------------------*/
static void DebugPrintErrorMessage(HotSwState state)
{
    switch(state){
      case HOTSW_SUCCESS:
        OS_PutString("   - Success\n");
        break;

      case HOTSW_TIME_OUT:
        OS_PutString("   - Error 1 : TimeOut\n");
        break;

      case HOTSW_CARD_LOCK_ERROR:
        OS_PutString("   - Error 2 : Slot Lock\n");
        break;

      case HOTSW_CRC_CHECK_ERROR:
        OS_PutString("   - Error 3 : CRC Check\n");
        break;

      case HOTSW_HASH_CHECK_ERROR:
        OS_PutString("   - Error 4 : Hash Check\n");
        break;

      case HOTSW_ID_CHECK_ERROR:
        OS_PutString("   - Error 5 : ID Check\n");
        break;

      case HOTSW_PULLED_OUT_ERROR:
        OS_PutString("   - Error 6 : Pulled Out\n");
        break;

      case HOTSW_DATA_DECRYPT_ERROR:
        OS_PutString("   - Error 7 : Data Decrypt\n");
        break;

      case HOTSW_BUFFER_OVERRUN_ERROR:
        OS_PutString("   - Error 8 : Buffer OverRun\n");
        break;

      case HOTSW_UNEXPECTED_ERROR:
        OS_PutString("   - Error 9 : Unexpected\n");
        break;

      default :
        OS_PutString("   - illigal Error\n");
        break;
    }
}