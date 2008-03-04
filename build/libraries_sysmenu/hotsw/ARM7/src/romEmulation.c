/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<romEmulation.h>
#include	<customNDma.h>

// Define Data --------------------------------------------------------------
#define		BOOT_PAGE_NUM				8
#define		SECURE_PAGE_NUM				32



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
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// �J�[�h���荞�݂ɂ��DMA�R�s�[
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x00000090;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 PC = 111(�X�e�[�^�X���[�h) latency1 = 1 ��)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

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
	GCDCmd64 tempCnd, cnd;
	u32 n = 0;
    
    for(i=0; i<SECURE_PAGE_NUM; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
    	// �[���N���A
		MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    	// ���g���G���f�B�A���ō����
    	tempCnd.dw = page << 33;

    	// �r�b�N�G���f�B�A���ɂ���
		cnd.b[0] = tempCnd.b[7];
		cnd.b[1] = tempCnd.b[6];
		cnd.b[2] = tempCnd.b[5];
		cnd.b[3] = tempCnd.b[4];
		cnd.b[4] = tempCnd.b[3];
		cnd.b[5] = tempCnd.b[2];
		cnd.b[6] = tempCnd.b[1];
		cnd.b[7] = tempCnd.b[0];
    
		// MCCMD ���W�X�^�ݒ�
	    reg_HOTSW_MCCMD0 = *(u32 *)cnd.b;
		reg_HOTSW_MCCMD1 = *(u32 *)&cnd.b[4];

		// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

		// MCCNT1 ���W�X�^�ݒ� (START = 1 PC_MASK PC = 001(1�y�[�W���[�h)�� latency1 = 0xd)
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0xd & LATENCY1_MASK);
    
		// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
		while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        	*(cbd->pSecureSegBuf + j++) = reg_HOTSW_MCD1;
		}
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
    
	GCDCmd64 tempCnd, cnd;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // �[���N���A
	MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    // ���g���G���f�B�A���ō����
    tempCnd.dw = 0xa000000000000000;

    // �r�b�N�G���f�B�A���ɂ���
	cnd.b[0] = tempCnd.b[7];
	cnd.b[1] = tempCnd.b[6];
	cnd.b[2] = tempCnd.b[5];
	cnd.b[3] = tempCnd.b[4];
	cnd.b[4] = tempCnd.b[3];
	cnd.b[5] = tempCnd.b[2];
	cnd.b[6] = tempCnd.b[1];
	cnd.b[7] = tempCnd.b[0];
    
	// MCCMD ���W�X�^�ݒ�
    reg_HOTSW_MCCMD0 = *(u32 *)cnd.b;
	reg_HOTSW_MCCMD1 = *(u32 *)&cnd.b[4];

	// MCCNT1 ���W�X�^�ݒ� (START = 1 ��)
	reg_HOTSW_MCCNT1 = START_MASK;
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

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
