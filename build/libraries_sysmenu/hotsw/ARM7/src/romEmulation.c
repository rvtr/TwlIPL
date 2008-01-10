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
void ReadBootSegNormal_ROMEMU(CardBootData *cbd)
{
	u32 i,j=0;
    u64 page = 0;
	GCDCmd64 tempCnd, cnd;
//	u32 n = 0;
    
    for(i=0; i<BOOT_PAGE_NUM; i++){
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
        	*(cbd->pBootSegBuf->word + j++) = reg_HOTSW_MCD1;
		}

        page++;
    }

/*
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->pBootSegBuf->word, BOOT_SEGMENT_SIZE );
    
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x00000000;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 PC_MASK PC = 100(8�y�[�W���[�h)�� latency1 = 0xd)
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x4 << PC_SHIFT) | (0xd & LATENCY1_MASK);
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
*/
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal_ROMEMU
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
 *---------------------------------------------------------------------------*/
void ChangeModeNormal_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	GCDCmd64 tempCnd, cnd;

    // �[���N���A
	MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    // ���g���G���f�B�A���ō����
    tempCnd.dw = 0x3c00000000000000;

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
}


// ��--------------------------------------��
// ��       �Z�L���A���[�h�̃R�}���h       ��
// ��--------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadIDSecure_ROMEMU(CardBootData *cbd)
{
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
}

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadSegSecure_ROMEMU(CardBootData *cbd)
{
	u32 i,j=0;
    u64 page = 0x20;
	GCDCmd64 tempCnd, cnd;
	u32 n = 0;
    
    for(i=0; i<SECURE_PAGE_NUM; i++){
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
//			OS_TPrintf("Secure Data Address : %08x\n", (cbd->pSecureSegBuf + j));
/*			OS_TPrintf("%02x ",reg_HOTSW_MCD1);
            if(!(n++ % 0xf)){
				OS_PutString("\n");
            }*/
		}
        page++;
    }

/*	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    // NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pSecureSegBuf, SECURE_SEGMENT_SIZE );

    // �R�}���h�\���̏�����
	MI_CpuClear8(&cndLE, sizeof(GCDCmd64));

    // �R�}���h�쐬
	cndLE.dw = 0x20 << 33;

    // �r�b�O�G���f�B�A���ɒ���(�Í�����)
	cndBE.b[7] = cndLE.b[0];
	cndBE.b[6] = cndLE.b[1];
    cndBE.b[5] = cndLE.b[2];
    cndBE.b[4] = cndLE.b[3];
    cndBE.b[3] = cndLE.b[4];
    cndBE.b[2] = cndLE.b[5];
    cndBE.b[1] = cndLE.b[6];
    cndBE.b[0] = cndLE.b[7];

    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
	reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 PC_MASK PC = 110(32�y�[�W���[�h)�� latency1 = 0xd)
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x6 << PC_SHIFT) | (0xd & LATENCY1_MASK);
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
*/
}

/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchONPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void ChangeModeSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	GCDCmd64 tempCnd, cnd;

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
}


// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_ROMEMU
  
  Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
void ReadIDGame_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	// �J�[�h���荞�݂ɂ��DMA�R�s�[
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );
    
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x000000B8;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 PC = 111(�X�e�[�^�X���[�h) latency1 = 1 ��)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
}

/*---------------------------------------------------------------------------*
  Name:         ReadPageGame_ROMEMU
  
  Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 *---------------------------------------------------------------------------*/
void ReadPageGame_ROMEMU(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
{
	#pragma unused( cbd )
    
    u32 		loop, counter=0;
	u64			i, page;
    GCDCmd64 	cndLE, cndBE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    OS_TPrintf("Src Addr : 0x%08x  Dst Addr : 0x%08x\n", start_addr, buf);
    OS_TPrintf("Read Game Segment  Page Count : %d   size : %x\n", loop, size);

    for(i=0; i<loop; i++){
		// �[���N���A
		MI_CpuClear8(&cndLE, sizeof(GCDCmd64));

        // �R�}���h�쐬
		cndLE.dw  = (page + i) << 33;
		cndLE.dw |= 0xB700000000000000;
        
        // �r�b�O�G���f�B�A���ɒ���(�Í�����)
		cndBE.b[7] = cndLE.b[0];
		cndBE.b[6] = cndLE.b[1];
    	cndBE.b[5] = cndLE.b[2];
    	cndBE.b[4] = cndLE.b[3];
    	cndBE.b[3] = cndLE.b[4];
   		cndBE.b[2] = cndLE.b[5];
    	cndBE.b[1] = cndLE.b[6];
	    cndBE.b[0] = cndLE.b[7];

    	// MCCMD ���W�X�^�ݒ�
		reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
		reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];
        
		// MCCNT1 ���W�X�^�ݒ� (START = 1 PC = 111(�X�e�[�^�X���[�h) latency1 = d ��)
		reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0xd & LATENCY1_MASK);
        
		// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
		while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
            *((u32 *)buf + counter++) = reg_HOTSW_MCD1;
		}
    }
}
