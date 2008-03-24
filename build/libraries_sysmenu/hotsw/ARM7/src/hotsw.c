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
#include 	<twl.h>
#include 	<twl/os/common/format_rom.h>
#include	<nitro/card/types.h>
#include	<sysmenu.h>
#include 	<hotswTypes.h>
#include	<blowfish.h>
#include	<dsCardCommon.h>
#include	<romEmulation.h>
#include	<customNDma.h>
#include 	<../build/libraries/mb/common/include/mb_fileinfo.h>

#define DEBUG_MODE

// define -------------------------------------------------------------------
#define		CHATTERING_COUNTER					0x600
#define		COUNTER_A							0x100

#define		CARD_EXIST_CHECK_INTERVAL			300

#define 	UNDEF_CODE							0xe7ffdeff	// ����`�R�[�h
#define 	ENCRYPT_DEF_SIZE					0x800		// 2KB  �� ARM9�풓���W���[���擪2KB

#define		DIGEST_HASH_BLOCK_SIZE_SHA1			(512/8)

#define 	SLOT_B_LOCK_BUF						HW_CTRDG_LOCK_BUF

#ifdef SDK_ARM9
#define 	reg_HOTSW_EXMEMCNT		 			reg_MI_EXMEMCNT
#define		HOTSW_EXMEMCNT_SELB_MASK			REG_MI_EXMEMCNT_MPB_MASK
#define		HOTSW_EXMEMCNT_SELB_SHIFT 			REG_MI_EXMEMCNT_MPB_SHIFT
#else  // SDK_ARM7
#define 	reg_HOTSW_EXMEMCNT		 			reg_MI_EXMEMCNT_L
#define		HOTSW_EXMEMCNT_SELB_MASK			0x0400
#define		HOTSW_EXMEMCNT_SELB_SHIFT 			10								 
#endif

// Function prototype -------------------------------------------------------
static BOOL IsSwap(void);
static u32 GetMcSlotShift(void);
static u32 GetMcSlotMask(void);
static void SetMcSlotMode(u32 mode);
static BOOL CmpMcSlotMode(u32 mode);

static void SetBootSegmentBuffer(void* buf, u32 size);
static void SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

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

static BOOL	isTwlModeLoad(void);

static void GenVA_VB_VD(void);
static HotSwState DecryptObjectFile(void);
static HotSwState LoadBannerData(void);
static HotSwState LoadStaticModule(void);
static HotSwState LoadCardData(void);
static HotSwState CheckCardAuthCode(void);

static HotSwState SelectErrorState(HotSwState nowState, HotSwState beforeState);

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

static char 				encrypt_object_key[] ATTRIBUTE_ALIGN(4) = "encryObj";
static char					rom_emu_info[] ATTRIBUTE_ALIGN(4)	    = "TWLD";

static u16					s_RscLockID;
static u16					s_CardLockID;
static u16					s_bondingOp;

static u32					s_BootSegBufSize, s_SecureSegBufSize, s_Secure2SegBufSize;

static BootSegmentData		*s_pBootSegBuffer;		// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�
static u32					*s_pSecureSegBuffer;	// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�
static u32					*s_pSecure2SegBuffer;	// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�

static CardBootData			s_cbData;

static BOOL 				s_IsPulledOut = TRUE;

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

static CardLoadFunction  	s_funcTable[] = {
	// DS Card Type 1
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
	// DS Card Type 2
	{ ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
	// TWL Card Type 1
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
	// RomEmulation
    {ReadIDSecure_ROMEMU, ReadSegSecure_ROMEMU, SwitchONPNGSecure_ROMEMU, ChangeModeSecure_ROMEMU}
};

// Global Values ------------------------------------------------------------
BLOWFISH_CTX 				HotSwBlowfishInitTableBufDS;
CardThreadData				HotSwThreadData;

#include <twl/ltdwram_end.h>

// ===========================================================================
// 	Function Describe
// ===========================================================================

/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init

  Description:  
 *---------------------------------------------------------------------------*/
void HOTSW_Init(u32 threadPrio)
{
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
	OS_CreateThread(&HotSwThreadData.monitorThread,
                    MonitorThread,
                    NULL,
                    HotSwThreadData.monitorStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );
    
    // ���b�Z�[�W�L���[�̏�����
	OS_InitMessageQueue( &HotSwThreadData.hotswQueue, &HotSwThreadData.hotswMsgBuffer[0], HOTSW_MSG_BUFFER_NUM );
	OS_InitMessageQueue( &HotSwThreadData.hotswDmaQueue, &HotSwThreadData.hotswDmaMsgBuffer[0], HOTSW_DMA_MSG_NUM );
    
    // �X���b�h�N��
    OS_WakeupThreadDirect(&HotSwThreadData.hotswThread);
	OS_WakeupThreadDirect(&HotSwThreadData.monitorThread);
	
    // Boot Segment �o�b�t�@�̐ݒ�
	SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure1,2 Segment �o�b�t�@�̐ݒ�
    SetSecureSegmentBuffer(HOTSW_MODE1, (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );
    SetSecureSegmentBuffer(HOTSW_MODE2, (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );
    
    // �J�[�h���}�����Ă�������X���b�h���N������
	if(HOTSW_IsCardExist()){
		// ���b�Z�[�W���M
    	OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

        // ���b�Z�[�W�C���f�b�N�X���C���N�������g
        HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
	}
    else{
		SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
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
    u32 romMode = HOTSW_ROM_MODE_NULL;

    start = OS_GetTick();

	// �J�[�h�̃��b�N
#ifndef DEBUG_USED_CARD_SLOT_B_
	CARD_LockRom(s_CardLockID);
#else
	LockExCard(s_CardLockID);
#endif

    // �J�[�h�d�����Z�b�g
	McPowerOff();
	McPowerOn();
    
	// �o�b�t�@��ݒ�
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

    // ���[�h�����J�n
	if(HOTSW_IsCardAccessible()){
		s_cbData.modeType = HOTSW_MODE1;
        
		// �J�[�h����Key Table�����[�h����
        retval = SelectErrorState(LoadTable(), retval);
        
    	// ---------------------- Normal Mode ----------------------
		romMode = HOTSW_ROM_MODE_NORMAL;

		{
			u8 i;
            u8 *romEmuInf = (u8 *)s_cbData.romEmuBuf;
            
			// �o�i�[���[�h���������āA�t���O�������I���܂�ARM9�Ɣr�����䂷��
            LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

    		// �J�[�hID�ǂݍ���
            retval = SelectErrorState(ReadIDNormal(&s_cbData), retval);

			// �J�[�h�^�C�v�𔻕ʂ��āA�g���֐���ؑւ��� ID�̍ŏ�ʃr�b�g��1�Ȃ�3DM
        	s_cbData.cardType = (s_cbData.id_nml & HOTSW_ROMID_1TROM_MASK) ? DS_CARD_TYPE_2 : DS_CARD_TYPE_1;
            
	    	// Boot Segment�ǂݍ���
            retval = SelectErrorState(ReadBootSegNormal(&s_cbData), retval);

            // Rom�G�~�����[�V���������擾
            retval = SelectErrorState(ReadRomEmulationData(&s_cbData), retval);

            // �擾����Rom�G�~�����[�V���������r
            s_cbData.debuggerFlg = TRUE;
            for(i=0; i<4; i++){
                if ( rom_emu_info[i] != romEmuInf[i] ){
					s_cbData.debuggerFlg = FALSE;
                    break;
                }
            }
            if(s_cbData.debuggerFlg){
				OS_PutString("Read Debugger\n");
				s_cbData.cardType = ROM_EMULATION;
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
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
                retval = SelectErrorState(HOTSW_CRC_CHECK_ERROR, retval);
			}
		}
		
		if( retval == HOTSW_SUCCESS ) {
	        // NTR�J�[�h��TWL�J�[�h��
#ifdef DEBUG_MODE
			if(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset && s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset && (s_cbData.id_nml & HOTSW_ROMID_TWLROM_MASK))
#else
			if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02)
#endif
            {
            	OS_TPrintf("TWL Card.\n");
	            s_cbData.twlFlg = TRUE;
	        }
            else{
                // NTR�J�[�h�̏ꍇ��RomHeader�o�b�t�@��1�y�[�W�ڈȍ~���N���A���Ă����B
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_CARD_ROM_HEADER_SIZE - PAGE_SIZE);
            }

            // Secure�R�}���h��PNG_ON�R�}���hetc�p�̃��C�e���V�����߂�(Latency1��Latency2�𑫂�)
			s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);

	    	// Key Table������
	    	GCDm_MakeBlowfishTableDS(&s_cbData, 8);

			// �R�}���h�F�ؒl�E�R�}���h�J�E���^�����l�EPN�W�F�l���[�^�����l�̐���
            GenVA_VB_VD();

	    	// �Z�L���A���[�h�Ɉڍs
            retval = SelectErrorState(ChangeModeNormal(&s_cbData), retval);

	    	// ---------------------- Secure Mode ----------------------
			romMode = HOTSW_ROM_MODE_SECURE;

			// PNG�ݒ�
            retval = SelectErrorState(s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData), retval);

	        // DS������������H�����l�ݒ� (���W�X�^�ݒ�)
			SetMCSCR();

			// ID�ǂݍ���
            retval = SelectErrorState(s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData), retval);

            // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
            if(s_cbData.id_nml != s_cbData.id_scr){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
            }

            if(retval == HOTSW_SUCCESS){
		    	// Secure�̈��Segment�ǂݍ���
                retval = SelectErrorState(s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData), retval);
            }

			// ��TWL�J�[�h�Ή� ��U���Z�b�g��Secure2���[�h�Ɉڍs
            // SCFG
            if((s_cbData.isLoadTypeTwl = isTwlModeLoad()) == TRUE){
               // Mode2�Ɉڍs���鏀��
				s_cbData.modeType = HOTSW_MODE2;

                // ---------------------- Reset ----------------------
				McPowerOff();
				McPowerOn();

                // ---------------------- Normal Mode ----------------------
                retval = SelectErrorState(ReadIDNormal(&s_cbData), retval);
                
  				// �擪1Page��������OK�B�f�[�^�͓ǂݎ̂ăo�b�t�@��
                retval = SelectErrorState(ReadBootSegNormal(&s_cbData), retval);

		    	// Key Table������
    			GCDm_MakeBlowfishTableDS(&s_cbData, 8);

				// �R�}���h�F�ؒl�E�R�}���h�J�E���^�����l�EPN�W�F�l���[�^�����l�̐���
        		GenVA_VB_VD();

	    		// �Z�L���A�Q���[�h�Ɉڍs
                retval = SelectErrorState(ChangeModeNormal2(&s_cbData), retval);

				// ---------------------- Secure2 Mode ----------------------
				// PNG�ݒ�
                retval = SelectErrorState(s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData), retval);

	    		// DS������������H�����l�ݒ� (���W�X�^�ݒ�)
				SetMCSCR();

        		// �Z�L���A�Q�J�[�hID�ǂݍ���
                retval = SelectErrorState(s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData), retval);

        		// Secure�Q�̈��Segment�ǂݍ���
                retval = SelectErrorState(s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData), retval);
            }
            
	    	// �Q�[�����[�h�Ɉڍs
            retval = SelectErrorState(s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData), retval);
            
	    	// ---------------------- Game Mode ----------------------
			romMode = HOTSW_ROM_MODE_GAME;

	    	// ID�ǂݍ���
            retval = SelectErrorState(ReadIDGame(&s_cbData), retval);

			// �o�i�[�t�@�C���̓ǂݍ���
            retval = SelectErrorState(LoadBannerData(), retval);

			// �r�����䂱���܂�(��CRC�`�F�b�N�܂łɃ~�X���Ȃ�������A�r�����䂱���܂�)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
            if(s_cbData.id_scr != s_cbData.id_gam){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
				goto end;
            }

			// �풓���W���[���c����w���ɓ]��
            retval = SelectErrorState(LoadStaticModule(), retval);
            
            // ARM9�풓���W���[���̐擪2KB�̈Í����̈�𕡍���
			(void)DecryptObjectFile();

			// �F�؃R�[�h�ǂݍ��݁����[�N�̈�ɃR�s�[
            retval = SelectErrorState(CheckCardAuthCode(), retval);
		}
        else{
			// �r�����䂱���܂�(��CRC�`�F�b�N�܂łɃ~�X����������A�����ŊJ������)
        	UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
            
			OS_TPrintf("ng... Card Data Load Skip ( state : %d )\n", retval);
        }
    }
    else{
        retval = SelectErrorState(HOTSW_PULLED_OUT_ERROR, retval);
    }

end:
	if( retval == HOTSW_SUCCESS )
	{
        // �o�b�h�u���b�N��u��
        retval = SelectErrorState(HOTSWi_RefreshBadBlock(romMode), retval);
	}

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

//	OS_TPrintf( "Load Card Time : %dms\n\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         isTwlModeLoad

  Description:  �J�[�h�̃��[�h��TWL���[�h�ōs����DS���[�h�ōs���������߂�

  
  �� Bonding Op = 0 (���i��)
	�\�t�g�E�F�A | 	�@�@�@ DS�J�[�h  	    |  		   TWL�J�[�h
	------------------------------------------------------------------------
		DS�p	 | 	DS�J�[�h�ǂ݃V�[�P���X  |    DS�J�[�h�ǂ݃V�[�P���X(��1)
	   TWL�p	 |  �s���J�[�h�t���O����    |    TWL�J�[�h�ǂ݃V�[�P���X
	�n�C�u���b�g |  �s���J�[�h�t���O����	|    TWL�J�[�h�ǂ݃V�[�P���X



  �� Bonding Op = 0�ȊO (�J���p)
	�\�t�g�E�F�A | 	�@�@�@ DS�J�[�h  	    |  		   TWL�J�[�h
	------------------------------------------------------------------------
		DS�p	 | 	DS�J�[�h�ǂ݃V�[�P���X  |    DS�J�[�h�ǂ݃V�[�P���X(��1)
	   TWL�p	 |  DS�J�[�h�ǂ݃V�[�P���X	|    TWL�J�[�h�ǂ݃V�[�P���X
	�n�C�u���b�g |  DS�J�[�h�ǂ݃V�[�P���X	|    TWL�J�[�h�ǂ݃V�[�P���X


  ��1 [TODO] ����Ɋւ��Ă̓J�[�hG�Ƒ��k���Č��߂�
 *---------------------------------------------------------------------------*/
static BOOL	isTwlModeLoad(void)
{
    // TWL�J�[�h
	if(s_cbData.id_nml & HOTSW_ROMID_TWLROM_MASK){
        // PlatformCode��Twl or Hybrid�̏ꍇ
        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
			return TRUE;
        }
        else{
            // [TODO] �d�l�m�F
			return FALSE;
        }
    }
    // DS�J�[�h
    else{
        // ���i�ł̏ꍇ
		if(s_bondingOp == SCFG_OP_PRODUCT){
			// PlatformCode��Twl or Hybrid�̏ꍇ
            if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
				s_cbData.illegalCardFlg = TRUE;
                return FALSE;
            }
            else{
				return FALSE;
            }
        }
        // �J���p�̏ꍇ
        else{
            return FALSE;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_RefreshBadBlock

  Description:  �m�[�}�����[�h�܂��̓Q�[�����[�h�Ńo�b�h�u���b�N��u��
 *---------------------------------------------------------------------------*/
HotSwState HOTSWi_RefreshBadBlock(u32 romMode)
{
    HotSwState retval = HOTSW_SUCCESS;

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
        retval = SelectErrorState(pReadStatus(&s_cbData), retval);

        // �v�����x���Ɋւ�炸�o�b�h�u���b�N��u���i���i�J�[�h�ł͖ő��ɔ������Ȃ��j
   	    if ( s_cbData.romStatus & (HOTSW_ROMST_RFS_WARN_L1_MASK | HOTSW_ROMST_RFS_WARN_L2_MASK) )
       	{
            retval = SelectErrorState(pRefreshBadBlock(&s_cbData), retval);
        }
	}

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_GetRomEmulationBuffer

  Description:  Rom�G�~�����[�V���������i�[���Ă���o�b�t�@�ւ̃|�C���^��Ԃ�
 *---------------------------------------------------------------------------*/
void* HOTSW_GetRomEmulationBuffer(void)
{
	return s_cbData.romEmuBuf;
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

	// �o�i�[���[�h
	if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
        retval  = ReadPageGame(&s_cbData,	s_cbData.pBootSegBuf->rh.s.banner_offset,
											(u32 *)SYSM_CARD_BANNER_BUF,
	                                     	sizeof(TWLBannerFile) );

        // �o�i�[���[�h���������Ă�����e��t���OTRUE ���̑��̏ꍇ��FALSE (���̊֐��̊O�Ŕr�����䂳��Ă��邩�炱���ł͔r�����䂵�Ȃ���OK)
        state = (retval == HOTSW_SUCCESS) ? TRUE : FALSE;
        SYSMi_GetWork()->flags.hotsw.isValidCardBanner  = state;
        SYSMi_GetWork()->flags.hotsw.isExistCard 		= state;
	}
    else{
        // �o�i�[�f�[�^���o�^����Ă��Ȃ��ꍇ (���̊֐��̊O�Ŕr�����䂳��Ă��邩�炱���ł͔r�����䂵�Ȃ���OK)
        SYSMi_GetWork()->flags.hotsw.isValidCardBanner  = FALSE;
        SYSMi_GetWork()->flags.hotsw.isExistCard 		= TRUE;
    }

    if ( SYSMi_GetWork()->flags.hotsw.isExistCard )
    {
        SYSMi_GetWork()->flags.hotsw.isInspectCard = s_cbData.pBootSegBuf->rh.s.inspect_card;
    }
    else
    {
        SYSMi_GetWork()->flags.hotsw.isInspectCard = FALSE;
    }

   	// �f�o�b�K���
	if ( ! SYSMi_GetWork()->flags.hotsw.is1stCardChecked && s_cbData.debuggerFlg )
	{
		MI_CpuCopy8( HOTSW_GetRomEmulationBuffer(), &SYSMi_GetWork()->romEmuInfo, ROM_EMULATION_DATA_SIZE );
		SYSMi_GetWork()->flags.hotsw.isOnDebugger = s_cbData.debuggerFlg;
	}

    SYSMi_GetWork()->flags.hotsw.isCardStateChanged = TRUE;
    SYSMi_GetWork()->flags.hotsw.is1stCardChecked   = TRUE;

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
    u32	secure2SegEnd = (u32)(s_cbData.pBootSegBuf->rh.s.twl_card_keytable_area_rom_offset * TWLCARD_BORDER_OFFSET + SECURE_SEGMENT_SIZE);

    // �z�u��ƍĔz�u�����擾 & Arm9�̏풓���W���[���c����w���ɓ]��
	s_cbData.arm9Stc = (u32)s_cbData.pBootSegBuf->rh.s.main_ram_address;
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_STATIC, &s_cbData.arm9Stc, s_cbData.pBootSegBuf->rh.s.main_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC] , s_cbData.twlFlg)){
        if(arm9StcEnd > SECURE_SEGMENT_END){
            retval = SelectErrorState(
                ReadPageGame(&s_cbData, 	s_cbData.pBootSegBuf->rh.s.main_rom_offset 	+ SECURE_SEGMENT_SIZE,
                                  	(u32 *)(s_cbData.arm9Stc 							+ SECURE_SEGMENT_SIZE),
                                  			arm9StcEnd 									- SECURE_SEGMENT_END),
                retval);
       	}
    }
    else{
        retval = SelectErrorState(HOTSW_BUFFER_OVERRUN_ERROR, retval);
    }
    if(retval != HOTSW_SUCCESS){
		return retval;
    }

    // �z�u��ƍĔz�u�����擾 & Arm7�̏풓���W���[�����w���ɓ]��
	s_cbData.arm7Stc = (u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address;
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_STATIC, &s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC], s_cbData.twlFlg)){
		retval = SelectErrorState(
            ReadPageGame(&s_cbData, 	s_cbData.pBootSegBuf->rh.s.sub_rom_offset,
                              	 (u32 *)s_cbData.arm7Stc,
                              		    s_cbData.pBootSegBuf->rh.s.sub_size),
            retval);
    }
    else{
        retval = HOTSW_BUFFER_OVERRUN_ERROR;
    }
    if(retval != HOTSW_SUCCESS){
		return retval;
    }

	// �g���풓���W���[�������邩�Ȃ���
	if(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset && s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset) {
		u32 size 		 = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ? s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
        s_cbData.arm9Ltd = (u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address;
		// �z�u��ƍĔz�u�����擾 & Arm9�g���풓���W���[�����w���ɓ]��
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_LTD_STATIC, &s_cbData.arm9Ltd, s_cbData.pBootSegBuf->rh.s.main_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC] , TRUE)){
			// DS�J�[�h�ǂ݃V�[�P���X�ł����܂ŗ��Ă�����ASecure2�̈��Game���[�h�̃y�[�W�ǂ݊֐��œǂ�
            if(!s_cbData.isLoadTypeTwl){
				retval = SelectErrorState(ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset, (u32 *)SYSM_CARD_TWL_SECURE_BUF, size), retval);
            }
			if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
                retval = SelectErrorState(
                    ReadPageGame(&s_cbData,		s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + SECURE_SEGMENT_SIZE,
	     	    	               	 	(u32 *)(s_cbData.arm9Ltd 							   + SECURE_SEGMENT_SIZE),
	        	    	                   		s_cbData.pBootSegBuf->rh.s.main_ltd_size 	   - size),
                    retval);
			}
        }
        else{
			retval = HOTSW_BUFFER_OVERRUN_ERROR;
        }

		if(retval != HOTSW_SUCCESS){
			return retval;
    	}
        
        // �z�u��ƍĔz�u�����擾 & Arm7�g���풓���W���[�����w���ɓ]��
		s_cbData.arm7Ltd = (u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address;
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_LTD_STATIC, &s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_LTD_STATIC], TRUE)){
			
            retval = SelectErrorState(
                    ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset, (u32 *)s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size),
                    retval);
        }
        else{
            retval = SelectErrorState(HOTSW_BUFFER_OVERRUN_ERROR, retval);
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

#define MY_DEBUG
#ifdef  MY_DEBUG
        {
		BOOL flg = TRUE;
            
        // Arm9�풓���W���[�� Hash�l�̃`�F�b�N
        if(!CheckArm9HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
            flg = FALSE;
			OS_PutString("�~Arm9 Static Module Hash Check Error...\n");
    	}
    	
    	// Arm7�풓���W���[�� Hash�l�̃`�F�b�N
    	if(!CheckArm7HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
			flg = FALSE;
            OS_PutString("�~Arm7 Static Module Hash Check Error...\n");
    	}
        
		// Arm9�g���풓���W���[�� Hash�l�̃`�F�b�N
        if(!CheckExtArm9HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
            flg = FALSE;
			OS_PutString("�~Arm9 Ltd Static Module Hash Check Error...\n");
    	}
        
        // Arm7�g���풓���W���[�� Hash�l�̃`�F�b�N
    	if(!CheckExtArm7HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
            flg = FALSE;
			OS_PutString("�~Arm7 Ltd Static Module Hash Check Error...\n");
    	}
        retval = (retval == HOTSW_SUCCESS) ? state : retval;

            if(flg){
				OS_PutString("�� Static Module Load was Completed!!\n");
            }
            
        }
#endif
	}

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

    u8	*p = (u8 *)authBuf;
    
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
static void SetBootSegmentBuffer(void* buf, u32 size)
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
static void SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size)
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
    u32 dummy = 0;
	MATHRandContext32	rnd;					
    
	// ������������ VBlank�J�E���^�l����Ƃ���B
    MATH_InitRand32(&rnd, (u64)OS_GetVBlankCount());
    
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
  	s32 size 				= (s32)s_cbData.pBootSegBuf->rh.s.main_size;
	u32 *pEncBuf			= encDestBuf;
    u32 *pEncDes 			= s_cbData.pSecureSegBuf;
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
  	BOOL exist 				= TRUE;
    HotSwState retval		= HOTSW_SUCCESS;

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
			bufp += 2;	// ��������
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
  Name:         SelectErrorState

  Description:  ����܂ł̏��(beforeState)�Ɉُ킪��������A���̏�Ԃ�Ԃ�l�Ƃ���B
				�ُ킪�Ȃ���ΐV�������(nowState)��Ԃ�l�Ƃ���B
 *---------------------------------------------------------------------------*/
static HotSwState SelectErrorState(HotSwState nowState, HotSwState beforeState)
{
	return (beforeState == HOTSW_SUCCESS) ? nowState : beforeState;
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
    if( HOTSW_IsCardExist() && CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
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
  Name:         CmpMcSlotMode

  Description:  �����ŗ^�����ă��[�h�ƌ��݂̃J�[�h�X���b�g�̃��[�h���r
 *---------------------------------------------------------------------------*/
static BOOL CmpMcSlotMode(u32 mode)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    if((reg_MI_MC1 & GetMcSlotMask()) == (mode << GetMcSlotShift()))
#else
    if((reg_MI_MC1 & GetMcSlotMask()) == (mode >> GetMcSlotShift()))
#endif
    {
		return TRUE;
    }
    else{
		return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:		   McPowerOn

  Description: �X���b�g�d��ON
 *---------------------------------------------------------------------------*/
static void McPowerOn(void)
{
    // Counter-A�J�E���^�ݒ�l���B�܂ő҂�
	while(CmpMcSlotMode(SLOT_STATUS_MODE_11) == TRUE){
		OS_Sleep(1);
    }

    if(CmpMcSlotMode(SLOT_STATUS_MODE_00) == TRUE){
		// [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
        // VDD�̈�����ԑ҂�
        OS_Sleep(100);
        
    	// SCFG_MC1 �� Slot Status �� M1,M0 �� 01 �ɂ���
    	SetMcSlotMode(SLOT_STATUS_MODE_01);
		// 1ms�҂�
		OS_Sleep(1);

    	// SCFG_MC1 �� Slot Status �� M1,M0 �� 10 �ɂ���
    	SetMcSlotMode(SLOT_STATUS_MODE_10);

        // [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
        // RESB���グ��܂ł̑҂�����
		OS_Sleep(1);

		// ���Z�b�g��high�� (RESB = 1�ɂ���)
		reg_HOTSW_MCCNT1 = RESB_MASK;
    
		// [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
        // �J�[�h�֍ŏ��̃R�}���h�𑗂�܂ł̑҂�����
		OS_Sleep(100);
    }
}


/*---------------------------------------------------------------------------*
  Name:		   McPowerOff

  Description: �X���b�g�d��OFF
 *---------------------------------------------------------------------------*/
static void McPowerOff(void)
{
    // Counter-A�J�E���^�ݒ�l���B�܂ő҂�
	while(CmpMcSlotMode(SLOT_STATUS_MODE_11) == TRUE){
		OS_Sleep(1);
    }
    
    if(CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
    	// SCFG_MC1 �� Slot Status �� M1,M0 �� 11 �ɂ���
    	SetMcSlotMode(SLOT_STATUS_MODE_11);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 00 �ɂȂ�܂Ń|�[�����O
        while(CmpMcSlotMode(SLOT_STATUS_MODE_00) == FALSE){
			OS_Sleep(1);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         SetMCSCR
  
  Description:  ����������H�����l�ݒ背�W�X�^��ݒ肷��

  �����F���̊֐��̓Z�L���A���[�h�ŁAsPNG_ON�R�}���h�����s���Ă���Ăяo���Ă��������B
 *---------------------------------------------------------------------------*/
static void SetMCSCR(void)
{
	static u32 pnbL  		 = 0x879b9b05;
	static u8  pnbH 		 = 0x5c;
	static u8  pnaL1 		 = 0x60;
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
  Name:		   HotSwThread

  Description: �J�[�h�����E�}�������X���b�h

  [TODO:]�}���̃t���[�E�t���O�P�Aetc�̊m�F(���̏��A�����}�����Ă��^�C�g�����X�V����Ȃ�)
 *---------------------------------------------------------------------------*/
static void HotSwThread(void *arg)
{
	#pragma unused( arg )

//    BOOL 			isPulledOut = TRUE;
    HotSwState 		retval;
    HotSwMessage 	*msg;
    
    while(1){
        OS_ReceiveMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        if( msg->ctrl == TRUE ) {
            // [TODO]�Ƃ肠�����A������HOTSW��}���������_��isExistCard��FALSE�Ȃ�AHOTSW��Finalize������悤�ɂ���B
			SYSMi_GetWork()->flags.hotsw.isEnableHotSW = msg->value;
            // [TODO]�J�[�h������Ƃ��ƂȂ��Ƃ��ŏꍇ��������Finalize����������
            //		 PXI���b�Z�[�W���u�}���v�ƁuFinalize�v�ŕ����ď���
            //			�� �P�D�S�Ẵ��W�X�^���N���A����			(�J�[�h���������Ă��Ȃ���)
            //			   �Q�D��x�d���𗎂Ƃ���Nomal���[�h�ɂ���	(NAND�A�v�������N������ꍇ)
            //			   �R�D�K�v�ȃ��W�X�^���c���āA��̓N���A	(�������Ă���J�[�h���N������ꍇ)
//			HOTSW_Finalize();
        }
        
        while(1){
			// �����}���}���t���O�������Ă����珈�����Ȃ�
			if( !SYSMi_GetWork()->flags.hotsw.isEnableHotSW ) {
                OS_PutString("### HotSw is restrained...\n");
				break;
			}
            
            // �J�[�h���}�����Ă���
            if(HOTSW_IsCardExist()){
                // �O�̏�Ԃ��}��
                if(!s_IsPulledOut){
                    // �������Ȃ�����������
                    if(CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
	               		// �t���O�P�A
                        LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                        
						SYSMi_GetWork()->flags.hotsw.isExistCard 		 = TRUE;
                		SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
                        
                    	// �V�����J�[�h��ID������
                    	SYSMi_GetWork()->nCardID = s_cbData.id_gam;

            			// �J�[�h�f�[�^���[�h�����t���O
            			SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = TRUE;

                        UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

						break;
                    }
                }

                // �J�[�h�ǂݍ��݊J�n
       			retval = LoadCardData();

				// Debug�\��
                DebugPrintErrorMessage(retval);
                
                // �J�[�h�ǂ݂����s���Ă�����J�[�h���Ȃ����Ƃɂ���
                if(retval != HOTSW_SUCCESS){
                	LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
					SYSMi_GetWork()->flags.hotsw.isExistCard 		 = FALSE;
                	SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
                	SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
                    SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
                	UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
                
                	// �J�[�h�u�[�g�p�\���̂̏�����
					MI_CpuClear32(&s_cbData, sizeof(CardBootData));

					// �o�b�t�@�̃N���A
					MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
					MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
                    MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));

                    if(retval == HOTSW_PULLED_OUT_ERROR){
						// ���̃J�[�h�̏�Ԃ𒲂ׂ�
                    }
                    
					break;
                }
                
				// ��ԃt���O���X�V
                s_IsPulledOut = FALSE;
            }
            
            // �J�[�h�������Ă���
            else{
                LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
				SYSMi_GetWork()->flags.hotsw.isExistCard 		 = FALSE;
                SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
                SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
                SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
                UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
                
                // �J�[�h�u�[�g�p�\���̂̏�����
				MI_CpuClear32(&s_cbData, sizeof(CardBootData));

				// �o�b�t�@�̃N���A
				MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
				MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
                MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));
                
                s_IsPulledOut = TRUE;

				break;
            }
        }
		SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
    } // while loop
}


/*---------------------------------------------------------------------------*
  Name:		   MonitorThread

  Description: ���ۂ̃J�[�h��Ԃ�HotSwThread�ŏ�Ԃ��ׂāA�Ⴂ���������ꍇ��
  			   ���b�Z�[�W�𑗂�

  s_IsPulledOut : True  -> �J�[�h�Ȃ�		HOTSW_IsCardExist : True  -> �J�[�h����
  				  False -> �J�[�h����							False -> �J�[�h�Ȃ�
 *---------------------------------------------------------------------------*/
static void MonitorThread(void *arg)
{
	#pragma unused( arg )
    
	BOOL isPullOutNow;
    
    while(1){
		// [TODO] �J�[�h�f�[�^���[�h���͑ҋ@����悤�ɂ���
		OS_Sleep(CARD_EXIST_CHECK_INTERVAL);
        
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
            }

            (void)OS_RestoreInterrupts( enabled );

            OS_PutString(">>> Card State Error\n");
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCard

  Description: �J�[�hB�������荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCard(void)
{
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
  Name:			InterruptCallbackCardDet

  Description:  �J�[�hB�}�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardDet(void)
{
	CARDi_ResetSlotStatus();
    
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
  Name:			InterruptCallbackNDma

  Description:  �J�[�hB �f�[�^�]���I�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackNDma(void)
{
	// ���b�Z�[�W���M
//    OS_SendMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&HotSwThreadData.hotswDmaMsg[HotSwThreadData.idx_dma], OS_MESSAGE_NOBLOCK);

	// ���b�Z�[�W�C���f�b�N�X���C���N�������g
//    HotSwThreadData.idx_dma = (HotSwThreadData.idx_dma+1) % HOTSW_DMA_MSG_NUM;
    
    OS_PutString("��\n");
}


/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackPxi

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
  Name:			AllocateExCardBus

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
  Name:			AllocateExCardBus

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
  Name:			FreeExCardBus

  Description:  �X���b�gB�ւ̃A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
static void FreeExCardBus(void)
{
#ifdef  SDK_ARM9
    SetExCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
#endif
}


/*---------------------------------------------------------------------------*
  Name:			LockSlotB

  Description:  �X���b�gB�����b�N����
 *---------------------------------------------------------------------------*/
static s32 LockExCard(u16 lockID)
{
    return OS_LockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, AllocateExCardBus);
}


/*---------------------------------------------------------------------------*
  Name:			UnlockSlotB

  Description:  �X���b�gB�����b�N����
 *---------------------------------------------------------------------------*/
static s32 UnlockExCard(u16 lockID)
{
    return OS_UnlockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, FreeExCardBus);
}


/*---------------------------------------------------------------------------*
  Name:			SetInterruptCallback
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
  	SetInterruptCallback( OS_IE_NDMA2 		, InterruptCallbackNDma );
#else
	SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
	SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
	SetInterruptCallback( OS_IE_NDMA2		, InterruptCallbackNDma );
#endif
}


#include <twl/os/common/systemCall.h>
/*---------------------------------------------------------------------------*
  Name:			CheckArm7HashValue

  Description:  Arm7�풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
static BOOL CheckArm7HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
	BOOL	retval = TRUE;

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
  Name:			CheckArm9HashValue

  Description:  Arm9�풓���W���[���̃n�b�V���`�F�b�N

  �� �擪2K�̕��������s����O�̃f�[�^�̃n�b�V�����ׂ�
 *---------------------------------------------------------------------------*/
static BOOL CheckArm9HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
	BOOL	retval = TRUE;
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
  Name:			CheckExtArm7HashValue

  Description:  Arm7�g���풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
static BOOL CheckExtArm7HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
	BOOL	retval = TRUE;

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
  Name:			CheckExtArm9HashValue

  Description:  Arm9�g���풓���W���[���̃n�b�V���`�F�b�N
 *---------------------------------------------------------------------------*/
static BOOL CheckExtArm9HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
    u32 	size;
	BOOL	retval = TRUE;
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


/*---------------------------------------------------------------------------*
  Name:			HOTSW_CardIF_Polling

  Description:  IF�t���O���|�[�����O���Ċ����}���������s��

  ����}���ł̓|�[�����O�̓_���H
 *---------------------------------------------------------------------------*/
void HOTSW_CardIF_Polling(void)
{
	// ����
    if(reg_OS_IF & OS_IE_CARD_B_IREQ){
        OSIntrMode enabled = OS_DisableInterrupts();
        
		OS_PutString("pulled out\n");
        
    	HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
    	HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
    	HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;
    
		// ���b�Z�[�W���M
    	OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

    	// ���b�Z�[�W�C���f�b�N�X���C���N�������g
    	HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

        reg_OS_IF |= OS_IE_CARD_B_DET;

        (void)OS_RestoreInterrupts( enabled );
	}

    // �}��
    if(reg_OS_IF & OS_IE_CARD_B_DET){
		OSIntrMode enabled = OS_DisableInterrupts();
        
		OS_PutString("insert\n");

        // �J�[�h���C�u�����F�}���񐔃C���N�������g
		CARDi_IncrementCount();

		HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
    	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
    	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

		// ���b�Z�[�W���M
    	OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

		// ���b�Z�[�W�C���f�b�N�X���C���N�������g
    	HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

        reg_OS_IF |= OS_IE_CARD_B_IREQ;

        (void)OS_RestoreInterrupts( enabled );
    }
}




// **************************************************************************
//
//							   Debug�p�\���֐�
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
    
    OS_TPrintf("title Name         : %s\n", 	s_pBootSegBuffer->rh.s.title_name);
    OS_TPrintf("initial Code       : %x\n\n", 	*(u32 *)s_pBootSegBuffer->rh.s.game_code);

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
    OS_TPrintf("�g���@�\���䃌�W�X�^		 (MC_B(d24))   : %08x\n", reg_SCFG_EXT);
    OS_TPrintf("MC I/F���䃌�W�X�^�P		 (slot status) : %08x\n", reg_MI_MC1);
    OS_TPrintf("MC I/F���䃌�W�X�^�Q		 (Counter-A)   : %04x\n", reg_MI_MC2);
    OS_TPrintf("MC �R���g���[�����W�X�^0	 (SEL etc)     : %04x\n", reg_HOTSW_MCCNT0);
    OS_TPrintf("MC �R���g���[�����W�X�^1	 (START etc)   : %08x\n", reg_HOTSW_MCCNT1);
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