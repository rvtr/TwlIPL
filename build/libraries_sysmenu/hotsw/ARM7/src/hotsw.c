/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     hotsw.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include	<dsCardType1.h>
#include	<dsCardType2.h>
#include	<romEmulation.h>
#include	<customNDma.h>

//#define HOTSW_FORCE_CARD_B

// define -------------------------------------------------------------------
#define 	UNDEF_CODE							0xe7ffdeff	// ����`�R�[�h
#define 	ENCRYPT_DEF_SIZE					0x800		// 2KB  �� ARM9�풓���W���[���擪2KB

#define		DIGEST_HASH_BLOCK_SIZE_SHA1			(512/8)

#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

#define		HOTSW_THREAD_STACK_SIZE				1024		// �X�^�b�N�T�C�Y
#define		HOTSW_THREAD_PRIO					11			// �J�[�h�d��ON �� �Q�[�����[�h�̃X���b�h�D��x
#define 	HOTSW_MSG_BUFFER_NUM				32			// ��M�o�b�t�@�̐�
#define		HOTSW_INSERT_MSG_NUM				16			// �}�����荞�ݑ��M���b�Z�[�W�̐�
#define		HOTSW_PULLED_MSG_NUM				16			// �������荞�ݑ��M���b�Z�[�W�̐�

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

// enum ---------------------------------------------------------------------
typedef enum IntrType{
	HOTSW_INSERT 	= 0,
    HOTSW_PULLEDOUT = 1
}
IntrType;
    
// struct -------------------------------------------------------------------
typedef struct HotSwMessage{
	IntrType 	type;
	u32			hotswCount; 
}
HotSwMessage;

// �X���b�h�E���b�Z�[�W�֌W���܂Ƃ߂��\����
typedef struct CardThreadData{
    u64  				stack[HOTSW_THREAD_STACK_SIZE / sizeof(u64)];
	OSThread 			thread;

	u32 				idx_insert;
    u32					idx_pulledOut;
	
    HotSwMessage		hotswInsertMsg[HOTSW_INSERT_MSG_NUM];
    HotSwMessage		hotswPulledOutMsg[HOTSW_PULLED_MSG_NUM];
    OSMessageQueue   	hotswQueue;
	OSMessage			hotswMsgBuffer[HOTSW_MSG_BUFFER_NUM];
}
CardThreadData;

// Function prototype -------------------------------------------------------
static BOOL IsSwap(void);
static BOOL IsCardExist(void);
static u32 GetMcSlotShift(void);
static void SetMcSlotMode(u32 mode);
static BOOL CmpMcSlotMode(u32 mode);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackCardData(void);

static void McThread(void *arg);
static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static void GenVA_VB_VD(void);
static void LoadTable(void);
static void ReadRomEmulationData(void);
static void ReadIDNormal(void);
static void DecryptObjectFile(void);

static void SetHotSwState(BOOL busy);

static s32 LockExCard(u16 lockID);
static s32 UnlockExCard(u16 lockID);

static BOOL CheckArm7HashValue(void);
static BOOL CheckArm9HashValue(void);
static BOOL CheckExtArm7HashValue(void);
static BOOL CheckExtArm9HashValue(void);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);

// Static Values ------------------------------------------------------------
static char 				*encrypt_object_key ATTRIBUTE_ALIGN(4) = "encryObj";
static char					*rom_emu_info ATTRIBUTE_ALIGN(4)	   = "TWLD";

static u32					s_SecureSegBufSize, s_BootSegBufSize;

static u32					*s_pSecureSegBuffer;	// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�
static BootSegmentData		*s_pBootSegBuffer;		// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�

static CardBootData			s_cbData;
static CardThreadData		s_ctData;

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

static CardBootFunction  	s_funcTable[] = {
	// DS Card Type 1
    {					   ReadBootSegNormal_DSType1, ChangeModeNormal_DSType1,								// Normal���[�h�֐�
     ReadIDSecure_DSType1, ReadSegSecure_DSType1, 	  SwitchONPNGSecure_DSType1, ChangeModeSecure_DSType1,	// Secure���[�h�֐�
     ReadIDGame_DSType1,   ReadPageGame_DSType1},															// Game  ���[�h�֐�
	// DS Card Type 2
    {					   ReadBootSegNormal_DSType2, ChangeModeNormal_DSType2,								// Normal���[�h�֐�
     ReadIDSecure_DSType2, ReadSegSecure_DSType2, 	  SwitchONPNGSecure_DSType2, ChangeModeSecure_DSType2,	// Secure���[�h�֐�
     ReadIDGame_DSType2,   ReadPageGame_DSType2},															// Game  ���[�h�֐�
	// TWL Card Type 1
    {					   ReadBootSegNormal_DSType2, ChangeModeNormal_DSType2,								// Normal���[�h�֐�
     ReadIDSecure_DSType2, ReadSegSecure_DSType2, 	  SwitchONPNGSecure_DSType2, ChangeModeSecure_DSType2,	// Secure���[�h�֐�
     ReadIDGame_DSType2,   ReadPageGame_DSType2},															// Game  ���[�h�֐�
	// RomEmulation
    {					   ReadBootSegNormal_ROMEMU,  ChangeModeNormal_ROMEMU,								// Normal���[�h�֐�
     ReadIDSecure_ROMEMU,  ReadSegSecure_ROMEMU, 	  SwitchONPNGSecure_ROMEMU,  ChangeModeSecure_ROMEMU,	// Secure���[�h�֐�
     ReadIDGame_ROMEMU,    ReadPageGame_ROMEMU}																// Game  ���[�h�֐�
};


// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void HOTSW_Init(void)
{
	OS_InitTick();
    OS_InitThread();
    
	// ���荞�݃}�X�N�̐ݒ�
	SetInterrupt();

    // ���荞�݂̗L����
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

#ifdef SDK_ARM7
	// �`���b�^�����O�J�E���^�̒l��ݒ�
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~REG_MI_MC1_CC_MASK) |
                            (0x100 << REG_MI_MC1_CC_SHIFT));

	// Counter-A�̒l��ݒ�
    reg_MI_MC2 = 0x100;
#else
    // PXI�o�R��ARM7�Ƀ`���b�^�����O�J�E���^�E�J�E���^A�̒l��ݒ肵�Ă��炤�B�ݒ肳���܂ő҂B

#endif

	// �J�[�h�u�[�g�p�\���̂̏�����
	MI_CpuClear8(&s_cbData, sizeof(CardBootData));

    // �J�[�h�X���b�h�p�\���̂̏�����
	MI_CpuClear8(&s_ctData, sizeof(CardThreadData));
    
	// �J�[�h�u�[�g�p�X���b�h�̐���
	OS_CreateThread(&s_ctData.thread,
                    McThread,
                    NULL,
                    s_ctData.stack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    HOTSW_THREAD_PRIO
                    );

    // ���b�Z�[�W�L���[�̏�����
	OS_InitMessageQueue( &s_ctData.hotswQueue, &s_ctData.hotswMsgBuffer[0], HOTSW_MSG_BUFFER_NUM );
    
    // �X���b�h�N��
    OS_WakeupThreadDirect(&s_ctData.thread);

    // Boot Segment �o�b�t�@�̐ݒ�
	HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure Segment �o�b�t�@�̐ݒ�
    HOTSW_SetSecureSegmentBuffer((void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );
    
    // �J�[�h���}�����Ă�������X���b�h���N������
	if(IsCardExist()){
//		OS_PutString("Card Boot Start\n");

      	// ���M���b�Z�[�W���쐬 (�����}���񐔂��擾�E�J�[�h�}��)
		s_ctData.hotswInsertMsg[s_ctData.idx_insert].hotswCount = CARDi_GetSlotResetCount();
        s_ctData.hotswInsertMsg[s_ctData.idx_insert].type	   	= HOTSW_INSERT;
        
		// ���b�Z�[�W���M
    	OS_SendMessage(&s_ctData.hotswQueue, (OSMessage)&s_ctData.hotswInsertMsg[s_ctData.idx_insert], OS_MESSAGE_NOBLOCK);

        // ���b�Z�[�W�C���f�b�N�X���C���N�������g
        s_ctData.idx_insert = (s_ctData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
	}
    else{
//		OS_PutString("No Card...\n");
#ifdef DEBUG_USED_CARD_SLOT_B_
		SYSMi_GetWork()->flags.common.is1stCardChecked  = TRUE;
#endif
    }
}

/* -----------------------------------------------------------------
 * HOTSW_LoadCardData�֐�
 *
 * �J�[�h����f�[�^�����[�h����
 *
 * ��BootSegmentBuffer SecureSegmentBuffer�̐ݒ���s���Ă���
 *   ���̊֐����Ă�ł��������B
 * ----------------------------------------------------------------- */
BOOL HOTSW_LoadCardData(void)
{
	OSTick start;
	s32 tempLockID;
	BOOL retval = TRUE;

    start = OS_GetTick();

    // �X���b�g���X���b�v����Ă��猳�ɖ߂��B
#ifdef HOTSW_FORCE_CARD_B
    if(reg_MI_MC1 & 0x8000){
		reg_MI_MC1 = reg_MI_MC1 & 0xff;
    }
#endif //  HOTSW_FORCE_CARD_B
    
//	OS_TPrintf("---------------- Card Boot Start ---------------\n");
#ifdef SDK_ARM7
	// �J�[�h�d�����Z�b�g
	McPowerOff();
	McPowerOn();
#else // SDK_ARM9
	// ARM7��PXI�o�R�ŃJ�[�h�d��ON�����肢�BON�ɂȂ�܂ő҂B
    
#endif
    
	// �o�b�t�@��ݒ�
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

	// �o�b�t�@�̃N���A
    MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
	MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
    
	// �J�[�h�̃��b�NID���擾
	tempLockID = OS_GetLockID();
    if(tempLockID == OS_LOCK_ID_ERROR){
		retval = FALSE;
    }
    else{
    	s_cbData.lockID = (u16)tempLockID;
    }

    // �u�[�g�����J�n
	if(IsCardExist() && retval){
		// �J�[�h�̃��b�N
#ifdef DEBUG_USED_CARD_SLOT_B_
        LockExCard(s_cbData.lockID);
#else
        OS_LockCard(s_cbData.lockID);
#endif
        
		// �J�[�h����Key Table�����[�h����
        LoadTable();
        
    	// ---------------------- Normal Mode ----------------------
    	// �J�[�hID�ǂݍ���
		ReadIDNormal();
        s_funcTable[0].ReadID_G(&s_cbData);
        
		// �J�[�h�^�C�v�𔻕ʂ����āA�g���֐���ؑւ��� ID�̍ŏ�ʃr�b�g��1�Ȃ�3DM
        if(s_cbData.id_nml & 0x80000000){
			s_cbData.cardType = DS_CARD_TYPE_2;
//			OS_TPrintf("Card Type2\n");
        }
        else{
			s_cbData.cardType = DS_CARD_TYPE_1;
//			OS_TPrintf("Card Type1\n");
        }
		
		{
			u8 i;
            u8 *romEmuInf = (u8 *)s_cbData.romEmuBuf;
            
			// ���Œ��ARM9�Ɣr�����䂵�Ȃ��Ƃ����Ȃ��͈͂͂��ꂾ��
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );			// ARM9�Ɣr�����䂷��
			
	    	// Boot Segment�ǂݍ���
	    	s_funcTable[s_cbData.cardType].ReadBootSegment_N(&s_cbData);

            // Rom�G�~�����[�V���������擾
			ReadRomEmulationData();

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
                SYSMi_GetWork()->gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
                OS_TPrintf("SYSMi_GetWork()->gameCommondParam : 0x%08x\n",SYSMi_GetWork()->gameCommondParam);
            }
            else{
				SYSMi_GetWork()->gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
            }
            
			// ROM�w�b�_CRC���Z�o���ă`�F�b�N�BNintendo���SCRC���m�F�B
			SYSMi_GetWork()->cardHeaderCrc16_bak = SVC_GetCRC16( 65535, s_cbData.pBootSegBuf, 0x015e );
			OS_TPrintf( "RomHeaderCRC16 : calc = %04x  romh = %04x\n",
						SYSMi_GetWork()->cardHeaderCrc16_bak, s_cbData.pBootSegBuf->rh.s.header_crc16 );
			
			if( ( SYSMi_GetWork()->cardHeaderCrc16_bak != s_cbData.pBootSegBuf->rh.s.header_crc16 ) ||
				( 0xcf56 != s_cbData.pBootSegBuf->rh.s.nintendo_logo_crc16 ) ){
				retval = FALSE;
			}
			
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );		// ARM9�Ɣr�����䂷��
			OS_ReleaseLockID( id );
		}
		
		if( retval ) {
	        // NTR�J�[�h��TWL�J�[�h��
	        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
//				OS_TPrintf("TWL Card.\n");
	            s_cbData.twlFlg = TRUE;
	        }
            else{
                // NTR�J�[�h�̏ꍇ��RomHeader�o�b�t�@��1�y�[�W�ڈȍ~���N���A���Ă����B
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_CARD_ROM_HEADER_SIZE - PAGE_SIZE);
            }

            // Secure�R�}���h��PNG_ON�R�}���hetc�p�̃��C�e���V�����߂�(Latency1��Latency2�𑫂�)
			s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);
            
	    	// Key Table������
	    	GCDm_MakeBlowfishTableDS(&s_cbData.keyTable, &s_pBootSegBuffer->rh.s, s_cbData.keyBuf, 8);

			// �R�}���h�F�ؒl�E�R�}���h�J�E���^�����l�EPN�W�F�l���[�^�����l�̐���
            GenVA_VB_VD();
            
	    	// �Z�L���A���[�h�Ɉڍs
	    	s_funcTable[s_cbData.cardType].ChangeMode_N(&s_cbData);

	    	// ---------------------- Secure Mode ----------------------
			// PNG�ݒ�
			s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData);
	
	        // DS������������H�����l�ݒ� (���W�X�^�ݒ�)
			SetMCSCR();
	
			// ID�ǂݍ���
	    	s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData);

            // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
            if(s_cbData.id_nml != s_cbData.id_scr){
				return FALSE;
            }
            
	    	// Secure�̈��Segment�ǂݍ���
	    	s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData);

	    	// �Q�[�����[�h�Ɉڍs
			s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData);

	    	// ---------------------- Game Mode ----------------------
	    	// ID�ǂݍ���
			s_funcTable[s_cbData.cardType].ReadID_G(&s_cbData);

            // �J�[�hID�̔�r�����āA��v���Ȃ����FALSE��Ԃ�
            if(s_cbData.id_scr != s_cbData.id_gam){
				return FALSE;
            }
            
			// �풓���W���[���c����w���ɓ]��
			HOTSW_LoadStaticModule();

            // ARM9�풓���W���[���̐擪2KB�̈Í����̈�𕡍���
			DecryptObjectFile();
		}

        // �J�[�h�̃��b�N�J��
#ifdef DEBUG_USED_CARD_SLOT_B_
        UnlockExCard(s_cbData.lockID);
#else
        OS_UnlockCard(s_cbData.lockID);
#endif
//		OS_TPrintf("-----------------------------------------------\n");
    }
    else{
//		OS_TPrintf("Card Not Found\n");
		retval = FALSE;
    }

    // �J�[�h���b�NID�̊J��
	OS_ReleaseLockID( s_cbData.lockID );

//	OS_TPrintf( "Load Card Time : %dms\n\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    return retval;
}

/* -----------------------------------------------------------------
 * HOTSW_GetRomEmulationBuffer�֐�
 *
 * Rom�G�~�����[�V���������i�[���Ă���o�b�t�@�ւ̃|�C���^��Ԃ�
 * ----------------------------------------------------------------- */
void* HOTSW_GetRomEmulationBuffer(void)
{
	return s_cbData.romEmuBuf;
}

/* -----------------------------------------------------------------
 * HOTSW_LoadStaticModule�֐�
 *
 * ARM7,9�̏풓���W���[����W�J����֐�
 * 
 * ���F��x�J�[�h�u�[�g���ăQ�[�����[�h�ɂȂ��Ă���Ăяo���Ă�������
 * ----------------------------------------------------------------- */
void HOTSW_LoadStaticModule(void)
{
#ifdef DEBUG_USED_CARD_SLOT_B_
	// �o�i�[���[�h
	if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
//		OS_TPrintf("  - Banner Loading...\n");
        s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
            									  s_cbData.pBootSegBuf->rh.s.banner_offset,
												  (u32 *)SYSM_CARD_BANNER_BUF,
	                                              sizeof(TWLBannerFile) );
	}
	
	if(IsCardExist())
	{
        SYSMi_GetWork()->flags.common.isValidCardBanner  = TRUE;
        SYSMi_GetWork()->flags.common.isCardStateChanged = TRUE;
        SYSMi_GetWork()->flags.common.isExistCard 		 = TRUE;
	}
	
#endif
    
	s_cbData.arm9Stc = (u32)s_cbData.pBootSegBuf->rh.s.main_ram_address;
	// �z�u��ƍĔz�u�����擾 & Arm9�̏풓���W���[���c����w���ɓ]��
	SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_STATIC, &s_cbData.arm9Stc, s_cbData.pBootSegBuf->rh.s.main_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC] , s_cbData.twlFlg);
    s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
        									  s_cbData.pBootSegBuf->rh.s.main_rom_offset  + SECURE_SEGMENT_SIZE,
                                 	  (u32 *)(s_cbData.arm9Stc 							  + SECURE_SEGMENT_SIZE),
                                              s_cbData.pBootSegBuf->rh.s.main_size        - SECURE_SEGMENT_SIZE);
    
	s_cbData.arm7Stc = (u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address;
    // �z�u��ƍĔz�u�����擾 & Arm7�̏풓���W���[�����w���ɓ]��
	SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_STATIC, &s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC] , s_cbData.twlFlg);
    s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                              s_cbData.pBootSegBuf->rh.s.sub_rom_offset,
                                 	   (u32 *)s_cbData.arm7Stc,
                                              s_cbData.pBootSegBuf->rh.s.sub_size);
    
	// TWL�ł̂݃��[�h
	if( s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL ) {
		u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
					 s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
        s_cbData.arm9Ltd = (u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address;
		// �z�u��ƍĔz�u�����擾 & Arm9�̏풓���W���[�����w���ɓ]���i��TWL�J�[�h�Ή����Ă��Ȃ��̂ŁA���ӁI�I�j
		SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_LTD_STATIC, &s_cbData.arm9Ltd, s_cbData.pBootSegBuf->rh.s.main_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC] , TRUE);
	    s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                                  s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset,
                                 		   (u32 *)SYSM_CARD_TWL_SECURE_BUF,
	                                          	  size);
		if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
		    s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                                      s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + SECURE_SEGMENT_SIZE,
	                             		 	  (u32 *)(s_cbData.arm9Ltd 								 + SECURE_SEGMENT_SIZE),
	                                          		  s_cbData.pBootSegBuf->rh.s.main_ltd_size 		 - size);
		}

		s_cbData.arm7Ltd = (u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address;
        // �z�u��ƍĔz�u�����擾 & Arm7�̏풓���W���[�����w���ɓ]��
		SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_LTD_STATIC, &s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_LTD_STATIC], TRUE);
	    s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                                  s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset,
	                             	 	   (u32 *)s_cbData.arm7Ltd,
	                                          	  s_cbData.pBootSegBuf->rh.s.sub_ltd_size);


		// �Z�L���A�̈�擪2K���̃n�b�V���l�����߂āAWork�̈�ɃR�s�[
        {
			u8		sha1data[DIGEST_SIZE_SHA1];
		    SVCHMACSHA1Context hash;

    		// �N���A
			MI_CpuClear8(sha1data, sizeof(sha1data));
    
    		// �n�b�V��������
			SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

    		// �Z�L���A�̈�擪2kb��UpDate
			SVC_HMACSHA1Update( &hash, s_cbData.pSecureSegBuf, ENCRYPT_DEF_SIZE );

			// �n�b�V���R���e�L�X�g��Work�̈�ɃR�s�[
		    MI_CpuCopy8( &hash, &SYSMi_GetWork2()->hmac_sha1_context, sizeof(SVCHMACSHA1Context) );
        }
        
        // Arm9�풓���W���[�� Hash�l�̃`�F�b�N
        if(!CheckArm9HashValue()){
			OS_PutString("�~Arm9 Static Module Hash Check Error...\n");
    	}
    	
    	// Arm7�풓���W���[�� Hash�l�̃`�F�b�N
    	if(!CheckArm7HashValue()){
			OS_PutString("�~Arm7 Static Module Hash Check Error...\n");
    	}
        
		// Arm9�g���풓���W���[�� Hash�l�̃`�F�b�N
        if(!CheckExtArm9HashValue()){
			OS_PutString("�~Arm9 Ltd Static Module Hash Check Error...\n");
    	}
        
        // Arm7�g���풓���W���[�� Hash�l�̃`�F�b�N
    	if(!CheckExtArm7HashValue()){
			OS_PutString("�~Arm7 Ltd Static Module Hash Check Error...\n");
    	}
	}
}


/* -----------------------------------------------------------------
 * HOTSW_SetBootSegmentBuffer�֐�
 *
 * Boot Segment �o�b�t�@�̎w��
 *
 * ���F�J�[�h�u�[�g�������͌Ăяo���Ȃ��悤�ɂ���
 * ----------------------------------------------------------------- */
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size)
{
	SDK_ASSERT(size > BOOT_SEGMENT_SIZE);

    s_pBootSegBuffer = (BootSegmentData *)buf;
    s_BootSegBufSize = size;

    s_cbData.pBootSegBuf = s_pBootSegBuffer;

    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pBootSegBuffer, size);

    OS_TPrintf("*** Boot Segm   Address : 0x%08x\n", s_pBootSegBuffer);
}

/* -----------------------------------------------------------------
 * HOTSW_SetSecureSegmentBuffer�֐�
 *
 * Secure Segment �o�b�t�@�̎w��
 * 
 * ���F�J�[�h�u�[�g�������͌Ăяo���Ȃ��悤�ɂ���
 * ----------------------------------------------------------------- */
void HOTSW_SetSecureSegmentBuffer(void* buf, u32 size)
{
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);
    
	s_pSecureSegBuffer = (u32 *)buf;
	s_SecureSegBufSize = size;

	s_cbData.pSecureSegBuf = s_pSecureSegBuffer;
    
    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pSecureSegBuffer, size);

    OS_TPrintf("*** Scr Seg Buf Address : 0x%08x\n", s_pSecureSegBuffer);
}


/* -----------------------------------------------------------------
 * GenVA_VB_VD�֐�
 *
 * �R�}���h�F�ؒl�E�R�}���h�J�E���^�EPN�W�F�l���[�^�����l�̐���
 * ----------------------------------------------------------------- */
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

/* -----------------------------------------------------------------
 * LoadTable�֐�
 *
 * �J�[�h���� Key Table �����[�h����֐��B
 *
 * �����̊֐��͊J���J�[�h�p�ɔ��s���Ȃ��Ƃ����Ȃ��B
 *   ���i�ŃJ�[�h�̏ꍇ�A���̃R�}���h�͖��������݌v
 * ----------------------------------------------------------------- */
static void LoadTable(void)
{
	u32 temp;
    
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0000009f;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

    // MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 101(16�y�[�W) latency1 = 0(�K�v�Ȃ�����) ��)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x5 << PC_SHIFT);
    
	// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        temp = reg_HOTSW_MCD1;
	}
}

/*---------------------------------------------------------------------------*
  Name:         ReadRomEmulationData
  
  Description:  Rom�G�~�����[�V�����f�[�^�̓ǂݍ���
 *---------------------------------------------------------------------------*/
static void ReadRomEmulationData(void)
{
	u32 count=0;
    u32 temp;
    u32 *dst = s_cbData.romEmuBuf;

    // �ʎY�pCPU�ł͕����A�N�Z�X�h�~�̂��߃��[�h���Ȃ�
    if ( ! (*(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK) )
    {
        return;
    }

	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x3e000000;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT1 ���W�X�^�ݒ� (START = 1  PC = 001(1�y�[�W���[�h)�� latency1 = 0x5fe)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0x5fe & LATENCY1_MASK);
    
	// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        if(count >= ROM_EMULATION_START_OFS && count < ROM_EMULATION_END_OFS){
        	*dst++ = reg_HOTSW_MCD1;
        }
        else{
			temp = reg_HOTSW_MCD1;
        }
        count+=4;
	}

   	MI_CpuCopyFast(s_cbData.romEmuBuf, (void*)HW_ISD_RESERVED, 32);
}

/* -----------------------------------------------------------------
 * ReadIDNormal�֐�
 *
 * �m�[�}�����[�h���̃J�[�hID��ǂݍ��ފ֐�
 * ----------------------------------------------------------------- */
void ReadIDNormal(void)
{
	// �J�[�h���荞�݂ɂ��DMA�R�s�[
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &s_cbData.id_nml, sizeof(s_cbData.id_nml) );
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x00000090;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 PC = 111(�X�e�[�^�X���[�h) latency1 = 1 ��)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
}

/* -----------------------------------------------------------------
 * DecryptObjectFile�֐�
 *
 * �Z�L���A�̈�擪2KB�̈Í����̈�𕜍���
 *
 * ���F�Z�L���A���[�h���A�܂��̓Z�L���A���[�h�O�ɂ��̊֐����ĂԂƁA
 * �@�@����ɃR�}���h�̈Í������s���Ȃ��Ȃ�܂��B
 * ----------------------------------------------------------------- */
static u32 encDestBuf[ENCRYPT_DEF_SIZE/sizeof(u32)];

static void DecryptObjectFile(void)
{
    u8  i;
  	s32 restSize;
  	s32 size 				= (s32)s_cbData.pBootSegBuf->rh.s.main_size;
	u32 *pEncBuf			= encDestBuf;
    u32 *pEncDes 			= s_cbData.pSecureSegBuf;
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
  	BOOL exist 				= TRUE;

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
		OS_PutString("�� DecryptObjectFile : Error...\n");

        MI_NDmaFill( HOTSW_DMA_NO, pEncBuf, UNDEF_CODE, (u32)size ); // ����`�R�[�h�ŃN���A
   	}
   	MI_CpuCopy32(pEncBuf, pEncDes, (u32)size);
}

/* -----------------------------------------------------------------
 * IsSwap�֐�
 *
 * �J�[�h�̃X���b�v����
 *
 * ��SCFG_MC1��SWP�t���O�����Ă���
 * ----------------------------------------------------------------- */
static BOOL IsSwap(void)
{
    if( reg_MI_MC1 & REG_MI_MC1_SWP_MASK ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

/* -----------------------------------------------------------------
 * IsCardExist�֐�
 *
 * �J�[�h�̑��ݔ���
 *
 * ��SCFG_MC1��CDET�t���O�����Ă���
 * ----------------------------------------------------------------- */
static BOOL IsCardExist(void)
{
    u32 mask = (u32)(REG_MI_MC_SL2_CDET_MASK >> GetMcSlotShift());

    if( !(reg_MI_MC1 & mask) ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

/* -----------------------------------------------------------------
 * GetMcSlotShift�֐�
 *
 * �J�[�h�X���b�g�̃V�t�g�r�b�g���̎擾
 * ----------------------------------------------------------------- */
static u32 GetMcSlotShift(void)
{
    return (u32)(IsSwap() * REG_MI_MC_SL2_CDET_SHIFT);
}

/* -----------------------------------------------------------------
 * SetMcSlotMode�֐�
 *
 * �J�[�h�X���b�g�̃��[�h�ݒ�
 * ----------------------------------------------------------------- */
static void SetMcSlotMode(u32 mode)
{
    u32 mask = (u32)(REG_MI_MC_SL2_MODE_MASK >> GetMcSlotShift());

    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~mask) | (mode >> GetMcSlotShift()));
}

/* -----------------------------------------------------------------
 * CmpMcSlotMode�֐�
 *
 * �J�[�h�X���b�g�̃��[�h��r
 * ----------------------------------------------------------------- */
static BOOL CmpMcSlotMode(u32 mode)
{
    u32 mask = (u32)(REG_MI_MC_SL2_MODE_MASK >> GetMcSlotShift());

    if((reg_MI_MC1 & mask) == (mode >> GetMcSlotShift())){
		return TRUE;
    }
    else{
		return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:		   McPowerOn

  Description: �X���b�gB �d��ON�֐�
 *---------------------------------------------------------------------------*/
static void McPowerOn(void)
{
    if(CmpMcSlotMode(SLOT_STATUS_MODE_00) == TRUE){
    	// SCFG_MC1 �� Slot Status �� M1,M0 �� 01 �ɂ���
    	SetMcSlotMode(SLOT_STATUS_MODE_01);
		// 1ms�҂�
		OS_Sleep(1);

    	// SCFG_MC1 �� Slot Status �� M1,M0 �� 10 �ɂ���
    	SetMcSlotMode(SLOT_STATUS_MODE_10);
		// 1ms�҂�
		OS_Sleep(1);

		// ���Z�b�g��high�� (RESB = 1�ɂ���)
		reg_HOTSW_MCCNT1 = RESB_MASK;
    
		// 100ms�҂�
		OS_Sleep(100);
    }
}

/*---------------------------------------------------------------------------*
  Name:		   McPowerOff

  Description: �X���b�gB �d��OFF�֐�
 *---------------------------------------------------------------------------*/
static void McPowerOff(void)
{
    if(CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
    	// SCFG_MC1 �� Slot Status �� M1,M0 �� 11 �ɂ���
    	SetMcSlotMode(SLOT_STATUS_MODE_11);

        // SCFG_MC1 �� Slot Status �� M1,M0 �� 00 �ɂȂ�܂Ń|�[�����O
        while(CmpMcSlotMode(SLOT_STATUS_MODE_00) == FALSE){}

		// 100ms�҂� [TODO:]�҂����Ԃ͎b��l�B���q����ɐ��l�𑪒肵�Ă��炤�B
		OS_Sleep(100);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetMCSCR
  
  Description:  ����������H�����l�ݒ背�W�X�^��ݒ肷��

  �����F���̊֐��̓Z�L���A���[�h�ŁA
		sPNG_ON�R�}���h�����s���Ă���Ăяo���Ă��������B
 *---------------------------------------------------------------------------*/
static void SetMCSCR(void)
{
	u32 pna_l = (u32)(PNA_BASE_VALUE | (s_cbData.vd << 15));
    u32 pna_h = (u32)(s_cbData.vd >> 17);
    
    // SCR A
	reg_HOTSW_MCSCR0 = pna_l;

    // SCR B
	reg_HOTSW_MCSCR1 = PNB_L_VALUE;

    // [d0 -d6 ] -> SCR A
    // [d16-d22] -> SCR B
    reg_HOTSW_MCSCR2 = (u32)(pna_h | PNB_H_VALUE << 16);

	// MCCNT1 ���W�X�^�ݒ� (SCR = 1��)
    reg_HOTSW_MCCNT1 = SCR_MASK;
}

/*---------------------------------------------------------------------------*
  Name:		   McThread

  Description: �J�[�h�����E�}�������X���b�h
 *---------------------------------------------------------------------------*/
static void McThread(void *arg)
{
	#pragma unused( arg )

	BOOL 	retval;
    HotSwMessage *msg;
    
    while(1){
		OS_ReceiveMessage(&s_ctData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        // �J�[�h�f�[�^���[�h�����t���O�����낷
		SYSMi_GetWork()->flags.common.isCardLoadCompleted = FALSE;
        
       	// �J�[�h�}�����荞�݂ɂ��X���b�h�N��
       	if(msg->type == HOTSW_INSERT){
			OS_PutString("-> Insert\n");

			// �����}���}���t���O�������Ă����珈�����Ȃ�
			if( !SYSMi_GetWork()->flags.common.isEnableHotSW ) {
#ifdef DEBUG_USED_CARD_SLOT_B_
				SYSMi_GetWork()->flags.common.is1stCardChecked  = TRUE;
#endif
				continue;
			}
                
            // �J�[�h���}���Ă���
            if(IsCardExist()){
            	// HotSw��busy��Ԃɂ��� 
				SetHotSwState(TRUE);
                
            	// �J�[�h�ǂݍ��݊J�n
       			retval = HOTSW_LoadCardData();

                // HotSw��free��Ԃɂ���
            	SetHotSwState(FALSE);
            
            	// �J�[�h�ǂݍ��݂Ɏ��s���Ă��ꍇ or �}���񐔕s��v or �J�[�h������Ă�
            	if(!retval || !CARDi_IsPulledOutEx(msg->hotswCount) || !IsCardExist()){
					OS_TPrintf("Load Card Data Retval : %x  HotSwCount : %d(msg : %d)  IsCardExist : %d\n",
                    	       retval, CARDi_GetSlotResetCount(),msg->hotswCount,IsCardExist());
                
                	// �t���O����
			    	SYSMi_GetWork()->flags.common.isValidCardBanner  = FALSE;
        			SYSMi_GetWork()->flags.common.isCardStateChanged = FALSE;
					SYSMi_GetWork()->flags.common.isExistCard 		 = FALSE;
                
					// �J�[�h���b�N�̊J��
#ifdef DEBUG_USED_CARD_SLOT_B_
        			UnlockExCard(s_cbData.lockID);
#else
        			OS_UnlockCard(s_cbData.lockID);
#endif

                    // �J�[�h���b�NID�̊J��
					OS_ReleaseLockID( s_cbData.lockID );
                    
                    OS_PutString("### Card Read Error\n");
				}
                // �J�[�h�ǂݍ��݁E��Ԃɖ�肪�Ȃ��ꍇ
                else{
					u16 id = (u16)OS_GetLockID();
					(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
					if( SYSMi_GetWork()->flags.arm9.reqChangeHotSW ) {
						SYSMi_GetWork()->flags.common.isEnableHotSW  = SYSMi_GetWork()->flags.arm9.nextHotSWStatus;
						SYSMi_GetWork()->flags.arm9.reqChangeHotSW   = 0;
						SYSMi_GetWork()->flags.arm9.nextHotSWStatus  = 0;
					}
					(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
					OS_ReleaseLockID( id );

                	// �V�����J�[�h��ID������
                	SYSMi_GetWork()->nCardID = s_cbData.id_gam;

                	// �f�o�b�K���
					MI_CpuCopy8( HOTSW_GetRomEmulationBuffer(), &SYSMi_GetWork()->romEmuInfo, ROM_EMULATION_DATA_SIZE );
					SYSMi_GetWork()->flags.common.isOnDebugger = s_cbData.debuggerFlg;

            		// �J�[�h�f�[�^���[�h�����t���O
            		SYSMi_GetWork()->flags.common.isCardLoadCompleted = TRUE;
                }
			} // end [if(IsCardExist())]
            
#ifdef DEBUG_USED_CARD_SLOT_B_
			SYSMi_GetWork()->flags.common.is1stCardChecked  = TRUE;
#endif
        }
            
        // �J�[�h�������荞�݂ɂ��X���b�h�N��
    	else if(msg->type == HOTSW_PULLEDOUT){
			OS_PutString("-> Init\n\n");

            // HOTSW�}���t���O�������Ă���A�������Ȃ�
			if( !SYSMi_GetWork()->flags.common.isEnableHotSW ) {
				continue;
			}
            
            // �J�[�h��������Ă���
            if(!IsCardExist()){
            	// �t���O�P�A
            	{
            		u16 id = (u16)OS_GetLockID();
					(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
					SYSMi_GetWork()->flags.common.isExistCard 		  = FALSE;
            		SYSMi_GetWork()->flags.common.isValidCardBanner   = FALSE;
            		SYSMi_GetWork()->flags.common.isCardStateChanged  = TRUE;
					(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
					OS_ReleaseLockID( id );
            	}

            	// �J�[�h�u�[�g�p�\���̂̏�����
				MI_CpuClear32(&s_cbData, sizeof(CardBootData));
	    	}
        }
    } // while loop
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCard

  Description: �J�[�hB�������荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCard(void)
{
	OS_TPrintf("*** Pulled Out Interrupt\n");
    
    // ���M���b�Z�[�W���쐬 (�����}���񐔂��擾�E�J�[�h�}��)
	s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut].hotswCount = CARDi_GetSlotResetCount();
    s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut].type	   = HOTSW_PULLEDOUT;
    
	// ���b�Z�[�W���M
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

    // ���b�Z�[�W�C���f�b�N�X���C���N�������g
    s_ctData.idx_pulledOut = (s_ctData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

//	OS_TPrintf(" - idx_pulledOut : %d\n", s_ctData.idx_pulledOut);
    
#ifdef USE_SLOT_A
	OS_SetIrqCheckFlagEx(OS_IE_CARD_A_IREQ);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_IREQ);
#endif
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCardDet

  Description:  �J�[�hB�}�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardDet(void)
{
	OS_TPrintf("*** Insert Interrupt\n");
    
    // SDK�̃J�[�h��Ԃ����Z�b�g����
	CARDi_ResetSlotStatus();

    // ���M���b�Z�[�W���쐬 (�����}���񐔂��擾�E�J�[�h�}��)
	s_ctData.hotswInsertMsg[s_ctData.idx_insert].hotswCount = CARDi_GetSlotResetCount();
    s_ctData.hotswInsertMsg[s_ctData.idx_insert].type	    = HOTSW_INSERT;
    
	// ���b�Z�[�W���M
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswInsertMsg[s_ctData.idx_insert], OS_MESSAGE_NOBLOCK);

	// ���b�Z�[�W�C���f�b�N�X���C���N�������g
    s_ctData.idx_insert = (s_ctData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

//	OS_TPrintf(" - idx_insert : %d\n", s_ctData.idx_insert);
    
#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DET);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DET);
#endif
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCardData

  Description:  �J�[�hB �f�[�^�]���I�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardData(void)
{
	// �f�[�^�]���I���҂��܂ŐQ�Ă����̂��N����
    OS_WakeupThreadDirect(&s_ctData.thread);

#ifdef USE_SLOT_A
	OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DATA);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DATA);
#endif
}

/*---------------------------------------------------------------------------*
  Name:			SetHotSwState

  Description:  HotSw�̏�Ԃ�ݒ肷��
 *---------------------------------------------------------------------------*/
static void SetHotSwState(BOOL busy)
{
	u16 id = (u16)OS_GetLockID();
	(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );

    SYSMi_GetWork()->flags.common.isBusyHotSW = busy ? 1 : 0;

	(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
	OS_ReleaseLockID( id );
}

/*---------------------------------------------------------------------------*
  Name:			AllocateExCardBus

  Description:  �A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
static inline void SetExCardProcessor(MIProcessor proc)
{
    reg_HOTSW_EXMEMCNT =
        (u16)((reg_HOTSW_EXMEMCNT & ~HOTSW_EXMEMCNT_SELB_MASK) | (proc << HOTSW_EXMEMCNT_SELB_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:			AllocateExCardBus

  Description:  �X���b�gB�ւ̃A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
static void AllocateExCardBus(void)
{
#ifdef  SDK_ARM9
    SetExCardProcessor(MI_PROCESSOR_ARM9);    // Arm9���œ��삵�Ă���ꍇ
#else
    SetExCardProcessor(MI_PROCESSOR_ARM7);	  // Arm7���œ��삵�Ă���ꍇ
#endif
}

/*---------------------------------------------------------------------------*
  Name:			FreeExCardBus

  Description:  �X���b�gB�ւ̃A�N�Z�X����ݒ肷��
 *---------------------------------------------------------------------------*/
static void FreeExCardBus(void)
{
    SetExCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
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
#ifdef USE_SLOT_A
  	SetInterruptCallback( OS_IE_CARD_A_IREQ , InterruptCallbackCard );
  	SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );
  	SetInterruptCallback( OS_IE_CARD_A_DATA , InterruptCallbackCardData );
#else
	SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
	SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
	SetInterruptCallback( OS_IE_CARD_B_DATA , InterruptCallbackCardData );
#endif
}

/* -----------------------------------------------------------------
 * CheckHashValue�֐�
 *
 * �풓���W���[���E�g���풓���W���[���̃n�b�V�����v�Z���āA
 * �J�[�h���̃n�b�V���l�Ɣ�ׂ�B
 * ----------------------------------------------------------------- */
#include <twl/os/common/systemCall.h>

// ----------------------------------------------------------------------
// 		Arm7�풓���W���[���̃n�b�V���`�F�b�N
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------
// 		Arm9�풓���W���[���̃n�b�V���`�F�b�N
//
//		�� �擪2K�̕��������s����O�̃f�[�^�̃n�b�V�����ׂ�
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------
// 		Arm7�g���풓���W���[���̃n�b�V���`�F�b�N
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------
// 		Arm9�g���풓���W���[���̃n�b�V���`�F�b�N
// ----------------------------------------------------------------------
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
