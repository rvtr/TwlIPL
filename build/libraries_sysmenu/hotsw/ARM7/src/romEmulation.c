/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include 	<romEmulation.h>
#include	<customNDma.h>

// Define Data --------------------------------------------------------------
#define		BOOT_PAGE_NUM				8
#define		SECURE_PAGE_NUM				32

// static value -------------------------------------------------------------
static OSMessage	s_Msg;

// extern -------------------------------------------------------------------
extern CardThreadData HotSwThreadData;



// ===========================================================================
// 	Function Describe
// ===========================================================================

// ��--------------------------------------��
// ��       �m�[�}�����[�h�̃R�}���h       ��
// ��--------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDNormal_ROMEMU
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h��ID�ǂݍ���
 *---------------------------------------------------------------------------*/
// ����

/*---------------------------------------------------------------------------*
  Name:         ReadBootSegNormal_ROMEMU
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h��Boot Segment�ǂݍ���
 *---------------------------------------------------------------------------*/
// ����

/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal_ROMEMU
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
 *---------------------------------------------------------------------------*/
// ����


// ��--------------------------------------��
// ��       �Z�L���A���[�h�̃R�}���h       ��
// ��--------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure_ROMEMU(CardBootData *cbd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

#ifndef USE_CPU_COPY
	// �J�[�h���荞�݂ɂ��DMA�R�s�[
	HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
#endif
    
  	// ���g���G���f�B�A���ō����
   	cndLE.dw  = HSWOP_N_OP_RD_ID;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );

	// MCCNT1 ���W�X�^�ݒ� (START = 1 PC = 111(�X�e�[�^�X���[�h) latency1 = 1 ��)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

#ifndef USE_CPU_COPY
    // ���b�Z�[�W��M
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
#else
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
		cbd->id_scr = reg_HOTSW_MCD1;
	}
#endif
    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure_ROMEMU(CardBootData *cbd)
{
	u32 i,j=0;
    u64 page = 0x20;
	GCDCmd64 cndLE;
	u32 n = 0;
    
    for(i=0; i<SECURE_PAGE_NUM; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
#ifndef USE_CPU_COPY
		// NewDMA�]���̏���
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, (u32 *)cbd->pSecureSegBuf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#endif
        
    	// ���g���G���f�B�A���ō����
    	cndLE.dw  = HSWOP_N_OP_RD_PAGE;
    	cndLE.dw |= page << HSWOP_N_RD_PAGE_ADDR_SHIFT;

		// MCCMD ���W�X�^�ݒ�
		HOTSWi_SetCommand(&cndLE);

		// MCCNT0 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );

		// MCCNT1 ���W�X�^�ݒ� (START = 1 PC_MASK PC = 001(1�y�[�W���[�h)�� latency1 = 0xd)
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0xd & LATENCY1_MASK);

#ifndef USE_CPU_COPY
    	// ���b�Z�[�W��M
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
#else
    	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
    		*((u32 *)cbd->pSecureSegBuf + j++) = reg_HOTSW_MCD1;
		}
#endif

        page++;
    }

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // ���g���G���f�B�A���ō����
    cndLE.dw = HSWOP_S_OP_CHG_MODE;

	// MCCMD ���W�X�^�ݒ�
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 ��)
	reg_HOTSW_MCCNT1 = START_MASK;
    
    HOTSW_WaitCardCtrl();

    return HOTSW_SUCCESS;
}

// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_ROMEMU
  
  Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
// ����

/*---------------------------------------------------------------------------*
  Name:         ReadPageGame_ROMEMU
  
  Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 *---------------------------------------------------------------------------*/
// ����
