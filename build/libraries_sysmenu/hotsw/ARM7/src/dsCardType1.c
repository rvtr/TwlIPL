/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType1.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardType1.h>
#include	<customNDma.h>

// Function prototype -------------------------------------------------------
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd);
static void SetMCSCR(void);


// ===========================================================================
// 	Function Describe
// ===========================================================================

// ��--------------------------------------��
// ��       �m�[�}�����[�h�̃R�}���h       ��
// ��--------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDNormal_DSType1
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h��ID�ǂݍ���
 *---------------------------------------------------------------------------*/
// ����

/*---------------------------------------------------------------------------*
 * Name:         ReadBootSegNormal_DSType1
 * 
 * Description:  DS�J�[�hType1�̃m�[�}�����[�h��Boot Segment�ǂݍ���
 * 
 * CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
 *---------------------------------------------------------------------------*/
void ReadBootSegNormal_DSType1(CardBootData *cbd)
{
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->pBootSegBuf->word, BOOT_SEGMENT_SIZE );
    
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x00000000;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x4 << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeNormal_DSType1
 * 
 * Description:  DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
void ChangeModeNormal_DSType1(CardBootData *cbd)
{
	GCDCmd64 tempCnd, cnd;
    u64 vae64 = cbd->vae;

    // �[���N���A
	MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    // ���g���G���f�B�A���ō����
	tempCnd.dw  = cbd->vbi << 8;
	tempCnd.dw |= vae64 << 32;
    tempCnd.dw |= 0x3c00000000000000;

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

	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | LATENCY2_MASK & (0x18 << LATENCY2_SHIFT);
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
}


// ��--------------------------------------��
// ��       �Z�L���A���[�h�̃R�}���h       ��
// ��--------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         SetSecureCommand
  
  Description:  
 *---------------------------------------------------------------------------*/
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd)
{
	GCDCmd64 cndLE, cndBE;
    u64 data;

    // �[���N���A
	MI_CpuClear8(&cndLE, sizeof(GCDCmd64));
	data = (type == S_PNG_ON) ? (u64)cbd->vd : (u64)cbd->vae;
    
    cndLE.dw  = cbd->vbi;
    cndLE.dw |= data << 20;
    
    // comannd0����
	switch(type){
      case S_RD_ID:
        cndLE.dw |= 0x1000000000000000;
        break;
        
      case S_PNG_ON:
        cndLE.dw |= 0x4000000000000000;
        break;

      case S_PNG_OFF:
        cndLE.dw |= 0x6000000000000000;
        break;

      case S_CHG_MODE:
        cndLE.dw |= 0xa000000000000000;
        break;
    }

    if(!cbd->debuggerFlg){
    	// �R�}���h�̈Í���
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );
    }
    
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
}


/*---------------------------------------------------------------------------*
 * Name:         ReadIDSecure_DSType1
 * 
 * Description:
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
void ReadIDSecure_DSType1(CardBootData *cbd)
{
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_RD_ID, cbd);

	// MCCNT1 ���W�X�^�ݒ�
    // (START = 1 TRM = 1 PC = 111(�X�e�[�^�X���[�h) ���Rom�w�b�_���ɂ��C��)
	reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.secure_cmd_param
        				| START_MASK | TRM_MASK | PC_MASK & (0x7 << PC_SHIFT) | SE_MASK | DS_MASK ;
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadSegSecure_DSType1
 * 
 * Description:
 *---------------------------------------------------------------------------*/
void ReadSegSecure_DSType1(CardBootData *cbd)
{
    u32			i,j=0;
	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    for(i=0; i<4; i++){
		// NewDMA�]���̏���
	    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, (cbd->pSecureSegBuf + ONE_SEGMENT_WORD_SIZE*i), ONE_SEGMENT_SIZE );
        
		MI_CpuClear8(&cndLE, sizeof(GCDCmd64));
        
	    cndLE.dw  = cbd->vbi;
	    cndLE.dw |= vae << 20;
		cndLE.dw |= segNum << 44;
	    cndLE.dw |= 0x2000000000000000;
        
	    // �R�}���h�̈Í���
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );

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

		// MCCNT1 ���W�X�^�ݒ�
    	// (START = 1 TRM = 1 PC = 100(8�y�[�W���[�h) ���Rom�w�b�_���ɂ��C��)
		reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.secure_cmd_param
        					| START_MASK | TRM_MASK | PC_MASK & (0x4 << PC_SHIFT) | SE_MASK | DS_MASK;
        
    	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    	OS_SleepThread(NULL);
        
        // �ǂݍ��݃Z�O�����g�ԍ��C���N�������g
		segNum++;
        
    	// �R�}���h�J�E���^�C���N�������g
		cbd->vbi++;
    }
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchONPNGSecure_DSType1
 * 
 * Description:
 *---------------------------------------------------------------------------*/
void SwitchONPNGSecure_DSType1(CardBootData *cbd)
{
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_ON, cbd);

    // MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = (cbd->pBootSegBuf->rh.s.secure_cmd_param & CT_MASK) |
        				START_MASK | TRM_MASK | SE_MASK | DS_MASK | (cbd->secureLatency & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchOFFPNGSecure_DSType1
 * 
 * Description:
 *---------------------------------------------------------------------------*/
void SwitchOFFPNGSecure_DSType1(CardBootData *cbd)
{
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_OFF, cbd);

    // MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = (cbd->pBootSegBuf->rh.s.secure_cmd_param & CT_MASK) |
        				START_MASK | TRM_MASK | SE_MASK | DS_MASK | (cbd->secureLatency & LATENCY1_MASK);
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeSecure_DSType1
 * 
 * Description:
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
void ChangeModeSecure_DSType1(CardBootData *cbd)
{
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_CHG_MODE, cbd);

    // MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = (cbd->pBootSegBuf->rh.s.secure_cmd_param & CT_MASK) |
        				START_MASK | TRM_MASK | SE_MASK | DS_MASK | (cbd->secureLatency & LATENCY1_MASK);

    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;
}


// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
 * Name:         ReadIDGame_DSType1
 * 
 * Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
void ReadIDGame_DSType1(CardBootData *cbd)
{
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );

	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x000000B8;
	reg_HOTSW_MCCMD1 = 0x00000000;

   	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 111(�X�e�[�^�X���[�h) ���̑�Rom�w�b�_�̏��ɂ��܂���)
	reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.game_cmd_param |
        				START_MASK | (PC_MASK & (0x7 << PC_SHIFT));
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);
}

/*---------------------------------------------------------------------------*
  Name:         ReadPageGame_DSType1
  
  Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 *---------------------------------------------------------------------------*/
void ReadPageGame_DSType1(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
    GCDCmd64 	cndLE, cndBE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

//	OS_TPrintf("Src Addr : 0x%08x  Dst Addr : 0x%08x\n", start_addr, buf);
//	OS_TPrintf("Read Game Segment  Page Count : %d   size : %x\n", loop, size);
    
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
        
   		// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 001(1�y�[�W���[�h) ���̑�Rom�w�b�_�̏��ɂ��܂���)
		reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.game_cmd_param |
            				START_MASK | (PC_MASK & (0x1 << PC_SHIFT));
        
		// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
		while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
            *((u32 *)buf + counter++) = reg_HOTSW_MCD1;
		}
    }
}
