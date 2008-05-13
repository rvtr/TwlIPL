/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     dsCardCommon.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#include 	<twl.h>
#include 	<firm/os/common/system.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include	<customNDma.h>

// extern -------------------------------------------------------------------
extern CardThreadData HotSwThreadData;

// define -------------------------------------------------------------------
#define		SECURE_SEGMENT_NUM					4
#define		ONE_SEGMENT_PAGE_NUM				8

#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

// static value -------------------------------------------------------------
static OSMessage	s_Msg;

// Function prototype -------------------------------------------------------
static HotSwState HOTSWi_ChangeModeNormal(CardBootData *cbd, u64 cmd);
static void PreSendSecureCommand(CardBootData *cbd, u32 *scrambleMask);

// ===========================================================================
// 	Function Describe
// ===========================================================================

/*---------------------------------------------------------------------------*
  Name:			HOTSWi_SetCommand

  Description:  �����ŗ^����ꂽ�R�}���h�̃G���f�B�A����ς��ă��W�X�^�ɃZ�b�g����
 *---------------------------------------------------------------------------*/
void HOTSWi_SetCommand(GCDCmd64 *cndLE)
{
	GCDCmd64 cndBE;

    // �r�b�O�G���f�B�A���ɒ���
	cndBE.b[7] = cndLE->b[0];
	cndBE.b[6] = cndLE->b[1];
    cndBE.b[5] = cndLE->b[2];
    cndBE.b[4] = cndLE->b[3];
    cndBE.b[3] = cndLE->b[4];
    cndBE.b[2] = cndLE->b[5];
    cndBE.b[1] = cndLE->b[6];
    cndBE.b[0] = cndLE->b[7];

	//---- confirm CARD free
	while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){}

    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
	reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];
}


// ��------------------------------------��
// ��       �m�[�}�����[�h�̃R�}���h     ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
  Name:			ReadIDNormal

  Description:  �m�[�}�����[�h���̃J�[�hID��ǂݍ��ފ֐�
 *---------------------------------------------------------------------------*/
HotSwState ReadIDNormal(CardBootData *cbd)
{
	GCDCmd64 cndLE;

	// �J�[�h���荞�݂ɂ��DMA�R�s�[
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_nml, sizeof(cbd->id_nml) );

   	// ���g���G���f�B�A���ō����
	cndLE.dw  = HSWOP_N_OP_RD_ID;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

    // MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_STAT | (0x1 & LATENCY1_MASK);
    
    // ���b�Z�[�W��M
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadBootSegNormal
  
  Description:  �m�[�}�����[�h��Boot Segment�ǂݍ���
 
  CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
 *---------------------------------------------------------------------------*/
HotSwState ReadBootSegNormal(CardBootData *cbd)
{
	u32 		i, loop, pc, size;
    u32 		*dst = cbd->pBootSegBuf->word;
    u32			temp;
    u64 		page = 0;
	GCDCmd64 	cndLE;

    if(cbd->cardType == DS_CARD_TYPE_1){
    	loop = 0x1UL;
    	pc   = 0x4UL;
    	size = BOOT_SEGMENT_SIZE;
    }
    else{
    	loop = ONE_SEGMENT_PAGE_NUM;
    	pc   = 0x1UL;
    	size = PAGE_SIZE;
    }

    // secure2���[�h�ڍs�ׁ̈ABoot Segment��1�y�[�W���ǂݍ��ށB�f�[�^�͎̂ăo�b�t�@�Ɋi�[
    if(cbd->modeType == HOTSW_MODE2){
    	loop = 0x1UL;
    	pc   = 0x1UL;
    	size = PAGE_SIZE;
    }
    
    for(i=0; i<loop; i++){
    	if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}

        if(cbd->modeType == HOTSW_MODE1){
			// NewDMA�]���̏���
        	HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, dst + (u32)(PAGE_WORD_SIZE*i), size );
        }
        else{
			// NewDMA�]���i�ǂݎ̂āj�̏���
			// Mode2�̂Ƃ��́A�f�[�^���̂Ă�B
    		HOTSW_NDmaPipe_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &temp, size );
        }

    	// ���g���G���f�B�A���ō����
		cndLE.dw  = HSWOP_N_OP_RD_PAGE;
		cndLE.dw |= page << HSWOP_N_RD_PAGE_ADDR_SHIFT;

		// MCCMD ���W�X�^�ݒ�
		HOTSWi_SetCommand(&cndLE);

		// MCCNT0 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
        
		// MCCNT1 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (pc << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;

    	// ���b�Z�[�W��M
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
        
        page++;
    }

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:			ReadStatusNormal

  Description:  �m�[�}�����[�h�ŃX�e�[�^�X��ǂݍ���
 *---------------------------------------------------------------------------*/
HotSwState ReadStatusNormal(CardBootData *cbd)
{
	GCDCmd64 cndLE;

	cbd->romStatus = 0;

  	if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
   	}

	// �J�[�h���荞�݂ɂ��DMA�R�s�[
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->romStatus, sizeof(cbd->romStatus) );

   	// ���g���G���f�B�A���ō����
	cndLE.dw  = HSWOP_N_OP_RD_STAT;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );

	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~SCRAMBLE_MASK) | START_MASK | HOTSW_PAGE_STAT;

    // ���b�Z�[�W��M
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:			RefreshBadBlockNormal

  Description:  �m�[�}�����[�h�Ńo�b�h�u���b�N��u��
 *---------------------------------------------------------------------------*/
HotSwState RefreshBadBlockNormal(CardBootData *cbd)
{
	GCDCmd64 cndLE;

  	if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
   	}

   	// ���g���G���f�B�A���ō����
	cndLE.dw  = HSWOP_N_OP_RD_STAT;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~SCRAMBLE_MASK) | START_MASK | HOTSW_PAGE_0;

    // �J�[�h�f�[�^�]���I���܂ő҂�
    HOTSW_WaitCardCtrl();

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal
  
  Description:  �m�[�}�����[�h����Z�L���A���[�h�ւ̕ύX
  
  CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal(CardBootData *cbd)
{
	return HOTSWi_ChangeModeNormal(cbd, HSWOP_N_OP_CHG_MODE);
}


/*---------------------------------------------------------------------------*
  Name:         ChangeModeNorma2
  
  Description:  �m�[�}�����[�h����Z�L���A�Q���[�h�ւ̕ύX
  
  CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal2(CardBootData *cbd)
{
	return HOTSWi_ChangeModeNormal(cbd, HSWOP_N_OP_CHG2_MODE);
}


static HotSwState HOTSWi_ChangeModeNormal(CardBootData *cbd, u64 cmd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // ���g���G���f�B�A���ō����
    cndLE.dw  = cmd;
	cndLE.dw |= cbd->vbi << HSWOP_N_VBI_SHIFT;
	cndLE.dw |= (u64)cbd->vae << HSWOP_N_VAE_SHIFT;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~SCRAMBLE_MASK) | START_MASK | HOTSW_PAGE_0;

    // �J�[�h�f�[�^�]���I���܂ő҂�
	HOTSW_WaitCardCtrl();
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:			LoadTable

  Description:  �J�[�h���� Key Table �����[�h����֐�

  �����̊֐��͊J���J�[�h�p�ɔ��s���Ȃ��Ƃ����Ȃ��B
  �@���i�ŃJ�[�h�̏ꍇ�A���̃R�}���h�͖��������
 *---------------------------------------------------------------------------*/
HotSwState LoadTable(void)
{
	GCDCmd64 cndLE;
	u32 temp;
    
	// NewDMA�]���i�ǂݎ̂āj�̏���
	HOTSW_NDmaPipe_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &temp, HOTSW_LOAD_TABLE_SIZE );
    
    // ���g���G���f�B�A���ō����
    cndLE.dw  = HSWOP_N_OP_LD_TABLE;
    
	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
    
    // MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_16 | LATENCY2_MASK & (0x18 << LATENCY2_SHIFT);
    
    // ���b�Z�[�W��M
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadRomEmulationInfo
  
  Description:  Rom�G�~�����[�V�������̓ǂݍ���
 *---------------------------------------------------------------------------*/
HotSwState ReadRomEmulationInfo(SYSMRomEmuInfo *info)
{
	u32 count=0;
    u32 temp;
    u32 *dst = (void*)info;

    // �ʎY�pCPU�ł͕����A�N�Z�X�h�~�̂��߃��[�h���Ȃ�
    if ( ! (*(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK) )
    {
        return HOTSW_SUCCESS;
    }
    
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x3e000000;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1  PC = 001(1�y�[�W���[�h)�� latency1 = 0x5fe)
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_1 | (0x5fe & LATENCY1_MASK);

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

    return HOTSW_SUCCESS;
}


// ��--------------------------------------��
// ��       �Z�L���A���[�h�̃R�}���h       ��
// ��--------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         SetSecureCommand
  
  Description:  �����ŗ^����ꂽ�R�}���h��Blowfish�ňÍ������ă��W�X�^�ɃZ�b�g
 *---------------------------------------------------------------------------*/
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd)
{
	GCDCmd64 cndLE;
    u64 data;

    // comannd0����
	switch(type){
      case S_RD_ID:
        cndLE.dw = HSWOP_S_OP_RD_ID;
        break;
        
      case S_PNG_ON:
        cndLE.dw = HSWOP_S_OP_PNG_ON;
        break;

      case S_PNG_OFF:
        cndLE.dw = HSWOP_S_OP_PNG_OFF;
        break;

      case S_CHG_MODE:
        cndLE.dw = HSWOP_S_OP_CHG_MODE;
        break;
    }

	// �R�}���h�쐬
	data = (type == S_PNG_ON) ? (u64)cbd->vd : (u64)cbd->vae;
    cndLE.dw |= cbd->vbi;
    cndLE.dw |= data << HSWOP_S_VA_SHIFT;
    
    if(!HOTSWi_IsRomEmulation()){
    	// �R�}���h�̈Í���
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );
    }

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);
}


/*---------------------------------------------------------------------------*
  Name:         PreSendSecureCommand
  
  Description:  
 *---------------------------------------------------------------------------*/
static void PreSendSecureCommand(CardBootData *cbd, u32 *scrambleMask)
{
    // �� TWL-ROM��NTR-3DM�Ή�
    if(cbd->cardType == DS_CARD_TYPE_2){
		u32 latency = (u32)cbd->pBootSegBuf->rh.s.secure_cmd_latency * 0x100;

		// MCCNT0 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
        
		// MCCNT1 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT1 = START_MASK | *scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

		// �Z�L���A�R�}���h�ԃ��C�e���V�҂�
    	OS_Sleep( OS_CPUCYC_TO_MSEC(latency) );
    }
    // �� TWL-XtraROM��NTR-MROM�Ή�
    else{
		*scrambleMask |= TRM_MASK;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure
  
  Description:
 
  CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure(CardBootData *cbd)
{
	u32 scrambleMask;
	u32	*buf = (cbd->modeType == HOTSW_MODE1) ? &cbd->id_scr : &cbd->id_scr2;
    
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // �X�N�����u���̐ݒ�
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_RD_ID, cbd);

	// �R�}���h���񑗐M�iNTR-MROM�̓��C�e���V�N���b�N�ݒ�ύX�̂݁j
	PreSendSecureCommand(cbd, &scrambleMask);

	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, buf, sizeof(buf) );

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_STAT | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    // ���b�Z�[�W��M
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure
  
  Description:
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure(CardBootData *cbd)
{
    u32 		scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
	u32			*buf = (cbd->modeType == HOTSW_MODE1) ? cbd->pSecureSegBuf : cbd->pSecure2SegBuf;
    u32			loop, pc, size, interval, i, j=0, k;
	u64			segNum = 4;
	GCDCmd64 	cndLE;

    if(cbd->cardType == DS_CARD_TYPE_1){
    	loop	 = 0x1UL;
    	pc   	 = 0x4UL;
    	size 	 = ONE_SEGMENT_SIZE;
        interval = ONE_SEGMENT_WORD_SIZE;
    }
    else{
    	loop 	 = ONE_SEGMENT_PAGE_NUM;
    	pc   	 = 0x1UL;
    	size 	 = PAGE_SIZE;
        interval = PAGE_WORD_SIZE;
    }
    
    for(i=0; i<SECURE_SEGMENT_NUM; i++){
		if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}

	    cndLE.dw  = HSWOP_S_OP_RD_SEG;
	    cndLE.dw |= cbd->vbi;
	    cndLE.dw |= (u64)cbd->vae << HSWOP_S_VA_SHIFT;
		cndLE.dw |= segNum << HSWOP_S_VC_SHIFT;
        
	    // �R�}���h�̈Í���
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );

		// MCCMD ���W�X�^�ݒ�
		HOTSWi_SetCommand(&cndLE);

		// �R�}���h���񑗐M�iNTR-MROM�̓��C�e���V�N���b�N�ݒ�ύX�̂݁j
		PreSendSecureCommand(cbd, &scrambleMask);

        for(k=0; k<loop; k++){
			// NewDMA�]���̏���
		    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, buf + (interval*j), size );

			// MCCNT0 ���W�X�^�ݒ�
			reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );

			// MCCNT1 ���W�X�^�ݒ�
			reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (pc << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    		// ���b�Z�[�W��M
			OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

            // �]���ς݃y�[�W��
            j++;
        }

        // �ǂݍ��݃Z�O�����g�ԍ��C���N�������g
		segNum++;

    	// �R�}���h�J�E���^�C���N�������g
		cbd->vbi++;
    }
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure
  
  Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // �X�N�����u���̐ݒ�
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_ON, cbd);

	// �R�}���h���񑗐M�iNTR-MROM�̓��C�e���V�N���b�N�ݒ�ύX�̂݁j
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I���܂ő҂�
	HOTSW_WaitCardCtrl();
    
    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure
  
  Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // �X�N�����u���̐ݒ�
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_OFF, cbd);

	// �R�}���h���񑗐M�iNTR-MROM�̓��C�e���V�N���b�N�ݒ�ύX�̂݁j
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I���܂ő҂�
	HOTSW_WaitCardCtrl();
    
    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure
  
  Description:
 
  CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // �X�N�����u���̐ݒ�
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_CHG_MODE, cbd);

	// �R�}���h���񑗐M�iNTR-MROM�̓��C�e���V�N���b�N�ݒ�ύX�̂݁j
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I���܂ő҂�
	HOTSW_WaitCardCtrl();
    
    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame
  
  Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
HotSwState ReadIDGame(CardBootData *cbd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );

   	// ���g���G���f�B�A���ō����
	cndLE.dw  = HSWOP_G_OP_RD_ID;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
   	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = cbd->gameCommondParam | START_MASK | HOTSW_PAGE_STAT;
    
    // ���b�Z�[�W��M
#ifndef USE_WRAM_LOAD
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
#else
    HOTSW_WaitDmaCtrl(HOTSW_NDMA_NO);
#endif
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadPageGame
  
  Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 
  CT=150ns  Pagecount=1page  Latency=RomHeader�Ŏw��̒l
 *---------------------------------------------------------------------------*/
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
	GCDCmd64	cndLE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    for(i=0; i<loop; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
		// NewDMA�]���̏���
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );

        // �R�}���h�쐬
		cndLE.dw  = HSWOP_G_OP_RD_PAGE;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

		// MCCMD ���W�X�^�ݒ�
		HOTSWi_SetCommand(&cndLE);

		// MCCNT0 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
        
   		// MCCNT1 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT1 = cbd->gameCommondParam | START_MASK | HOTSW_PAGE_1;
        
		// ���b�Z�[�W��M
#ifndef USE_WRAM_LOAD
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
#else
    	HOTSW_WaitDmaCtrl(HOTSW_NDMA_NO);
#endif
    }

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadStatusGame
  
  Description:  �Q�[�����[�h�ŃX�e�[�^�X��ǂݍ���
 *---------------------------------------------------------------------------*/
HotSwState ReadStatusGame(CardBootData *cbd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->romStatus, sizeof(cbd->romStatus) );

   	// ���g���G���f�B�A���ō����
	cndLE.dw  = HSWOP_G_OP_RD_STAT;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
   	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 111(�X�e�[�^�X���[�h) ���̑�Rom�w�b�_�̏��ɂ��܂���)
	reg_HOTSW_MCCNT1 = cbd->gameCommondParam | START_MASK | HOTSW_PAGE_STAT;
    
    // ���b�Z�[�W��M
#ifndef USE_WRAM_LOAD
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
#else
    	HOTSW_WaitDmaCtrl(HOTSW_NDMA_NO);
#endif
    
    return HOTSW_SUCCESS;
}


/* -----------------------------------------------------------------
  RefreshBadBlockGame�֐�
 
  �Q�[�����[�h�Ńo�b�h�u���b�N��u��
 * ----------------------------------------------------------------- */
HotSwState RefreshBadBlockGame(CardBootData *cbd)
{
	GCDCmd64 cndLE;

  	if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
   	}

   	// ���g���G���f�B�A���ō����
	cndLE.dw  = HSWOP_G_OP_RD_STAT;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
   	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = cbd->gameCommondParam | START_MASK | HOTSW_PAGE_0;

    // �J�[�h�f�[�^�]���I���܂ő҂�
	HOTSW_WaitCardCtrl();

    return HOTSW_SUCCESS;
}
