/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     Card.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include 	<twl/os/common/format_rom.h>
//#include 	<istdbglib.h>

#include	<nitro/card/types.h>
#include	<sysmenu.h>
#include 	<hotswTypes.h>
#include	<blowfish.h>
#include	<dsCardType1.h>
#include	<dsCardType2.h>

// define -------------------------------------------------------------------
#define		STACK_SIZE							1024		// �X�^�b�N�T�C�Y
#define		MC_THREAD_PRIO						11			// �J�[�h�d��ON �� �Q�[�����[�h�̃X���b�h�D��x
#define		ML_THREAD_PRIO						12			// Boot Segment�ǂݍ��ݏI�������N������B�J�[�h�u�[�g�X���b�h���D��x��B

#define		DEBUG_CARD_TYPE						1			// DS Card Type1 = 0  DS Card Type2 = 1

#define 	UNDEF_CODE							0xe7ffdeff	// ����`�R�[�h
#define 	ENCRYPT_DEF_SIZE					0x800		// 2KB  �� ARM9�풓���W���[���擪2KB

// Function prototype -------------------------------------------------------
static BOOL IsCardExist(void);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackCardData(void);

static void McThread(void *arg);
static void StaticModuleLoadThread(void *arg);
static void LoadStaticModule_Secure(void);
static void McPowerOn(void);
static void SetMCSCR(void);

static void GenVA_VB_VD(void);
static void LoadTable(void);
static void ReadIDNormal(void);
static void DecryptObjectFile(void);
static void ReadPageNormalFromDebugger(u32 page, void* buf);

static void MIm_CardDmaCopy32(u32 dmaNo, const void *src, void *dest);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);

// Static Values ------------------------------------------------------------
static char 				*encrypt_object_key ATTRIBUTE_ALIGN(4) = "encryObj";

static u64  				s_MCStack[STACK_SIZE / sizeof(u64)];
static OSThread 			s_MCThread;

static u32					s_SecureSegBufSize, s_BootSegBufSize;

static u32					*s_pSecureSegBuffer;	// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�
static BootSegmentData		*s_pBootSegBuffer;		// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�

static CardBootData			s_cbData;

// -------------------------------------------------------------------

static CardBootFunction  	s_funcTable[] = {
	// DS Card Type 1
    {					   ReadBootSegNormal_DSType1, ChangeModeNormal_DSType1,								// Normal���[�h�֐�
     ReadIDSecure_DSType1, ReadSegSecure_DSType1, 	  SwitchONPNGSecure_DSType1, ChangeModeSecure_DSType1,	// Secure���[�h�֐�
     ReadIDGame_DSType1,   ReadPageGame_DSType1},															// Game  ���[�h�֐�
	// DS Card Type 2
    {					   ReadBootSegNormal_DSType2, ChangeModeNormal_DSType2,								// Normal���[�h�֐�
     ReadIDSecure_DSType2, ReadSegSecure_DSType2, 	  SwitchONPNGSecure_DSType2, ChangeModeSecure_DSType2,	// Secure���[�h�֐�
     ReadIDGame_DSType2,   ReadPageGame_DSType2}															// Game  ���[�h�֐�
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
	reg_MI_MC1 = (u32)((reg_MI_MC1 & 0xffff) | 0xc80000);

	// Counter-A�̒l��ݒ�
    reg_MI_MC2 = 0xc8;
#endif

	// �J�[�h�u�[�g�p�X���b�h�̐���
	OS_CreateThread(&s_MCThread,
                    McThread,
                    NULL,
                    s_MCStack + STACK_SIZE / sizeof(u64),
                    STACK_SIZE,
                    MC_THREAD_PRIO
                    );

    // �X���b�h�N��
    OS_WakeupThreadDirect(&s_MCThread);

    // Boot Segment �o�b�t�@�̐ݒ�
	HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure Segment �o�b�t�@�̐ݒ�
    HOTSW_SetSecureSegmentBuffer((void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );

	// �J�[�h�u�[�g�p�\���̂̏�����
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));
    
	OS_TPrintf("*** sizeof(ROM_Header) : 0x%08x\n", sizeof(ROM_Header));
}

/* -----------------------------------------------------------------
 * HOTSW_Boot�֐�
 *
 * �J�[�h�N�����X�^�[�g
 *
 * ��BootSegmentBuffer SecureSegmentBuffer�̐ݒ���s���Ă���
 *   ���̊֐����Ă�ł��������B
 * ----------------------------------------------------------------- */
BOOL HOTSW_Boot(void)
{
	s32 tempLockID;
	BOOL retval = TRUE;

    OSTick start = OS_GetTick();
	
	OS_TPrintf("---------------- Card Boot Start ---------------\n");
	// �J�[�h�d��ON
	McPowerOn();

	// �Z�L���A�̈�̓ǂݍ��݃Z�O�����g�擪�ԍ�(Segment4 �` Segment7)
    s_cbData.secureSegNum = 4;

	// �o�b�t�@��ݒ�
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

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
		// �J�[�h����Key Table�����[�h����
        LoadTable();
        
    	// ---------------------- Normal Mode ----------------------
    	// �J�[�hID�ǂݍ���
		ReadIDNormal();

		ShowRomHeaderData();
        
		// �J�[�h�^�C�v�𔻕ʂ����āA�g���֐���ؑւ��� ID�̍ŏ�ʃr�b�g��1�Ȃ�3DM
        if(s_cbData.id_nml & 0x80000000){
			s_cbData.cardType = DS_CARD_TYPE_2;
            OS_TPrintf("Card Type2\n");
        }
        else{
			s_cbData.cardType = DS_CARD_TYPE_1;
            OS_TPrintf("Card Type1\n");
        }
		
		{
			// ���Œ��ARM9�Ɣr�����䂵�Ȃ��Ƃ����Ȃ��͈͂͂��ꂾ��
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );			// ARM9�Ɣr�����䂷��
			
	    	// Boot Segment�ǂݍ���
	    	s_funcTable[s_cbData.cardType].ReadBootSegment_N(&s_cbData);
			
            // Rom�G�~�����[�V�����f�[�^���擾
            if(s_cbData.cardType == DS_CARD_TYPE_1){
				// Type1�̏ꍇ
                MI_CpuCopy8(s_cbData.pBootSegBuf->rh.s.reserved_C, s_cbData.romEmuBuf, ROM_EMULATION_DATA_SIZE);
            }
            else if(s_cbData.cardType == DS_CARD_TYPE_2){
				// Type2�̏ꍇ
                ReadRomEmulationData_DSType2(&s_cbData);
            }

			// ROM�w�b�_CRC���Z�o���ă`�F�b�N�BNintendo���SCRC���m�F�B
			SYSMi_GetWork()->cardHeaderCrc16_bak = SVC_GetCRC16( 65535, s_cbData.pBootSegBuf, 0x015e );
			OS_TPrintf( "RomHeaderCRC16 : calc = %04x  romh = %04x\n",
						SYSMi_GetWork()->cardHeaderCrc16_bak, s_cbData.pBootSegBuf->rh.s.header_crc16 );
			
			if( ( SYSMi_GetWork()->cardHeaderCrc16_bak != s_cbData.pBootSegBuf->rh.s.header_crc16 ) ||
				( 0xcf56 != s_cbData.pBootSegBuf->rh.s.nintendo_logo_crc16 ) ){
				retval = FALSE;
			}
			
			SYSMi_GetWork()->isExistCard = retval;
			SYSMi_GetWork()->isCardStateChanged = TRUE;	// �{���͑}���P�ʂł����𗧂Ă�B
			
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );		// ARM9�Ɣr�����䂷��
			OS_ReleaseLockID( id );
		}
		
		if( retval ) {
	        // NTR�J�[�h��TWL�J�[�h��
	        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
				OS_TPrintf("TWL Card.\n");
	            s_cbData.twlFlg = TRUE;
	        }
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
	
	    	// Secure�̈��Segment�ǂݍ���
	    	s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData);
	
			// Arm9�̏풓���W���[�����w���ɓ]��
			LoadStaticModule_Secure();
            
	    	// �Q�[�����[�h�Ɉڍs
			s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData);
	
	    	// ---------------------- Game Mode ----------------------
	    	// ID�ǂݍ���
			s_funcTable[s_cbData.cardType].ReadID_G(&s_cbData);

            // ARM9�풓���W���[���̐擪2KB�̈Í����̈�𕡍���
			DecryptObjectFile();
            
			// �풓���W���[���c����w���ɓ]��
			HOTSW_LoadStaticModule();
	
			// �f�o�b�O�o��
			ShowRomHeaderData();
		}

		// ���ŏI�I�ɂ̓J�[�hID��HW_BOOT_CHECK_INFO_BUF�ɓ���Ȃ��ƁA�A�v���N����̃J�[�h������������肭���삵�Ȃ��̂Œ��ӁB
		//   ���̓X���b�gB���g�p���Ă���̂ŁA�m�[�P�A��OK.
//		*(u32 *)HW_BOOT_CHECK_INFO_BUF = s_cbData.id_gam;
//		*(u32 *)HW_RED_RESERVED = s_cbData.id_gam;
		SYSMi_GetWork()->nCardID = s_cbData.id_gam;
		
        OS_TPrintf("-----------------------------------------------\n\n");
    }
    else{
		OS_TPrintf("Card Not Found\n");
		retval = FALSE;
    }

    // �J�[�h���b�NID�̊J��
	OS_ReleaseLockID( s_cbData.lockID );

	OS_TPrintf( "Load Card Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
#ifdef DEBUG_USED_CARD_SLOT_B_
	SYSMi_GetWork()->is1stCardChecked  = TRUE;
#endif
  
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
	    OS_TPrintf("  - Banner Loading...\n");
	    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.banner_offset,
												  (u32 *)SYSM_CARD_BANNER_BUF,
	                                              sizeof(TWLBannerFile) );
		SYSMi_GetWork()->isValidCardBanner = TRUE;
		SYSMi_GetWork()->is1stCardChecked  = TRUE;
	}
#endif
	
    OS_TPrintf("  - Arm9 Static Module Loading...\n");
    // Arm9�̏풓���W���[���c����w���ɓ]��
    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.main_rom_offset  + SECURE_SEGMENT_SIZE,
                                 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.main_ram_address + SECURE_SEGMENT_SIZE),
                                              s_cbData.pBootSegBuf->rh.s.main_size        - SECURE_SEGMENT_SIZE);

    OS_TPrintf("  - Arm7 Static Module Loading...\n");
    // Arm7�̏풓���W���[�����w���ɓ]��
    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.sub_rom_offset,
                                 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address),
                                              s_cbData.pBootSegBuf->rh.s.sub_size);

	// TWL�ł̂݃��[�h
	if( s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL ) {
		u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
					 s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
	    OS_TPrintf("  - Arm9 Ltd. Static Module Loading...\n");
	    // Arm9�̏풓���W���[�����w���ɓ]���i��TWL�J�[�h�Ή����Ă��Ȃ��̂ŁA���ӁI�I�j
		
		s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset,
                                 		   (u32 *)SYSM_CARD_TWL_SECURE_BUF,
	                                          	  size);
		if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
		    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset,
	                             		 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address),
	                                          		  s_cbData.pBootSegBuf->rh.s.main_ltd_size);
		}

	    OS_TPrintf("  - Arm7 Ltd. Static Module Loading...\n");
	    // Arm7�̏풓���W���[�����w���ɓ]��
	    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset,
	                             	 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address),
	                                          	  s_cbData.pBootSegBuf->rh.s.sub_ltd_size);
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
	reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,  0)) |
        		             		 		     CNT1_FLD(1,0,0,0,  0,5,  0,0,  0,  0,0,0,  0));
    
	// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        temp = reg_HOTSW_MCD1;
	}
}

/* -----------------------------------------------------------------
 * ReadIDNormal�֐�
 *
 * �m�[�}�����[�h���̃J�[�hID��ǂݍ��ފ֐�
 * ----------------------------------------------------------------- */
static void ReadIDNormal(void)
{
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x00000090;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 111(�X�e�[�^�X���[�h) latency1 = 2320(�K�v�Ȃ�����) ��)
	reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,7,  0,0,  0,  0,0,0,  2320));
    
	// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
		s_cbData.id_nml = reg_HOTSW_MCD1;
	}

}


/* -----------------------------------------------------------------
 * ReadPageNormalFromDebugger�֐�
 *
 * Rom�G�~�����[�V��������ǂ�
 * ----------------------------------------------------------------- */
void ReadPageNormalFromDebugger(u32 page, void* buf)
{
    GCDCmd64 le , be;
    u64 page_data = page;
//    u32 i=0;

	// �[���N���A
	MI_CpuClear8(&le, sizeof(GCDCmd64));
    
   	// �R�}���h�쐬
	le.dw = (page_data << 33);
    
    // �r�b�O�G���f�B�A���ɒ���
	be.b[7] = le.b[0];
	be.b[6] = le.b[1];
	be.b[5] = le.b[2];
	be.b[4] = le.b[3];
	be.b[3] = le.b[4];
	be.b[2] = le.b[5];
	be.b[1] = le.b[6];
    be.b[0] = le.b[7];

	// MCCMD ���W�X�^�ݒ�
	reg_MI_MCCMD0_B = *(u32*)be.b;
	reg_MI_MCCMD1_B = *(u32*)&be.b[4];

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 001(1�y�[�W���[�h) CS = 1 SE = 1 DS = 1 latency1 = 20 ��)
	reg_MI_MCCNT1_B = (u32)((reg_MI_MCCNT1_B & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,   0)) |
       			           		 			   CNT1_FLD(1,0,0,0,  0,1,  0,0,  0,  0,0,0,  20));
    
	// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
	while(reg_MI_MCCNT1_B & START_FLG_MASK){
		while(!(reg_MI_MCCNT1_B & READY_FLG_MASK)){}
        *( ((u32 *)buf)++ ) = reg_MI_MCD1_B;
	}
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

		OS_PutString("�� DecryptObjectFile : Success!\n");
            
   		bufp[0] = UNDEF_CODE;
   		bufp[1] = UNDEF_CODE;
   		while ((restSize -= 8) > 0) {
			bufp += 2;	// ��������
			DecryptByBlowfish(tableBufp, &(bufp)[1], &(bufp)[0]);
   		}
   	} 
   	else{
		OS_PutString("�� DecryptObjectFile : Error...\n");
		// CpuClearFast32(UNDEF_CODE, pEncBuf, size); // ����`�R�[�h�ŃN���A
   	}
   	MI_CpuCopy32(pEncBuf, pEncDes, (u32)size);
}


/* -----------------------------------------------------------------
 * IsCardExist�֐�
 *
 * �J�[�h�̑��ݔ���
 *
 * ��SCFG_MC1��Slot ���[�h�I���t���O�����Ă���
 *
 * ���[�h�I���t���O�� 10 (�S�Ă̒[�q����L���o��) �̎��������Ă���Ɣ���
   		Slot A �̏ꍇ if((reg_MI_MC1 & 0x0c) == 0x08)
		Slot B �̏ꍇ if((reg_MI_MC1 & 0xc0) == 0x80)
 * ----------------------------------------------------------------- */
static BOOL IsCardExist(void)
{
    if((reg_MI_MC1 & SLOT_STATUS_MODE_SELECT_MSK) == SLOT_STATUS_MODE_10){
		return TRUE;
    }
    else{
		return FALSE;
    }
}

/* -----------------------------------------------------------------
 * McThread B �֐�
 * ----------------------------------------------------------------- */
static void McThread(void *arg)
{
	#pragma unused( arg )
    
    while(1){
		OS_SleepThread(NULL);

        // �J�[�h�u�[�g
        HOTSW_Boot();
    }
}

/* -----------------------------------------------------------------
 * StaticModuleLoadThread �֐�
 * ----------------------------------------------------------------- */
static void StaticModuleLoadThread(void *arg)
{
	#pragma unused( arg )
    
    while(1){
		OS_SleepThread(NULL);


    }
}

static void LoadStaticModule_Secure(void)
{
    if(s_cbData.pBootSegBuf->rh.s.main_size >= SECURE_SEGMENT_SIZE){
		MI_DmaCopy32(1, s_cbData.pSecureSegBuf, s_cbData.pBootSegBuf->rh.s.main_ram_address, SECURE_SEGMENT_SIZE);
    }
    else{
		MI_DmaCopy32(1, s_cbData.pSecureSegBuf, s_cbData.pBootSegBuf->rh.s.main_ram_address, s_cbData.pBootSegBuf->rh.s.main_size);
    }
}

/* -----------------------------------------------------------------
 * McPowerOn�֐�
 * ----------------------------------------------------------------- */
static void McPowerOn(void)
{
	OSTick start;

    // SCFG_MC1 �� Slot Status �� M1,M0 �� 11 �ɂ���
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | 0xc0);
	// 10ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // SCFG_MC1 �� Slot Status �� M1,M0 �� 00 �ɂ���
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | 0x00);
	// 10ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // SCFG_MC1 �� Slot Status �� M1,M0 �� 01 �ɂ���
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_01);
	// 10ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // SCFG_MC1 �� Slot Status �� M1,M0 �� 10 �ɂ���
	reg_MI_MC1 	= (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_10);
	// 10ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // ���Z�b�g��high�� (RESB = 1�ɂ���)
	reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(1,1,0,1,1,1,1,1,1,1,1,1,1)) |
                             		 CNT1_FLD(0,0,1,0,0,0,0,0,0,0,0,0,0));
	// 10ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    OS_TPrintf("MC Power ON\n");
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
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(1,1,1,1,  1,1,  1,1,  1,  0,1,1,  1)) |
           		             		 CNT1_FLD(0,0,0,0,  0,0,  0,0,  0,  1,0,0,  0));
}

/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCard
				InterruptCallbackCardDet
  				InterruptCallbackCardData
  
  Description:  �e�튄�荞�݃R�[���o�b�N�֐�
 *---------------------------------------------------------------------------*/
// �J�[�hB����
static void InterruptCallbackCard(void)
{
    // M�� 10 ���� 11 �ɑJ��
	reg_MI_MC1 = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_11);
    
    // �J�[�h�u�[�g�p�\���̂̏�����
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));

    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pBootSegBuffer, s_BootSegBufSize);
    MI_CpuClear8(s_pSecureSegBuffer, s_SecureSegBufSize);

    // MC_CNT1��������
    reg_HOTSW_MCCNT1 = 0x0;

    // �J�[�h���b�NID�̊J��
	OS_ReleaseLockID( s_cbData.lockID );
	
#ifdef USE_SLOT_A
	OS_SetIrqCheckFlagEx(OS_IE_CARD_A_IREQ);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_IREQ);
#endif
}

// �J�[�hB�}��
static void InterruptCallbackCardDet(void)
{
    // �J�[�h�u�[�g�X���b�h���N������
	OS_WakeupThreadDirect(&s_MCThread);

#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DET);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DET);
#endif
}

// �J�[�hB �f�[�^�]���I��
static void InterruptCallbackCardData(void)
{
#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DATA);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DATA);
#endif
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
	OS_TPrintf("Debug Data -------------------------------\n");
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
