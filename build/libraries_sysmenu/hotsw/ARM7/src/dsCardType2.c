/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType2.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardType2.h>
#include	<customNDma.h>

// Define Data --------------------------------------------------------------
#define		SECURE_SEGMENT_NUM					4
#define		ONE_SEGMENT_PAGE_NUM				8
#define		COMMAND_DECRYPTION_WAIT				25 		// 25ms
#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

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
  Name:         ReadRomEmulationData_DSType2
  
  Description:  DS�J�[�hType2��Rom�G�~�����[�V�����f�[�^�̓ǂݍ���
 *---------------------------------------------------------------------------*/
void ReadRomEmulationData_DSType2(CardBootData *cbd)
{
	u32 count=0;
    u32 temp;
    u32 *dst = cbd->romEmuBuf;
    
	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x3e000000;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT1 ���W�X�^�ݒ� (START = 1  W/R = 0  PC = 001 (1�y�[�W���[�h) ��)
	reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
    			             		 			 CNT1_FLD(1,0,0,0,  0,1,  0,0,  0,  0,0,0,  1540));

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
}

/*---------------------------------------------------------------------------*
  Name:         ReadIDNormal_DSType2
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h��ID�ǂݍ���
 *---------------------------------------------------------------------------*/
// ����

/*---------------------------------------------------------------------------*
  Name:         ReadBootSegNormal_DSType2
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h��Boot Segment�ǂݍ��� (Page0 �` 7)
 *---------------------------------------------------------------------------*/
void ReadBootSegNormal_DSType2(CardBootData *cbd)
{
	u32 		i = 0;
    u32 		*dst = cbd->pBootSegBuf->word;
    u64 		page = 0;
    GCDCmd64 	cndLE, cndBE;

    for(i=0; i<ONE_SEGMENT_PAGE_NUM; i++){
		// NewDMA�]���̏���
        HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pBootSegBuf->word + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
        
        // �[���N���A
		MI_CpuClear8(&cndLE, sizeof(GCDCmd64));
        
    	// ���g���G���f�B�A���ō����
		cndLE.dw  = 0x0  << 24;
		cndLE.dw |= page << 33;

    	// �r�b�N�G���f�B�A���ɂ���
		cndBE.b[0] = cndLE.b[7];
		cndBE.b[1] = cndLE.b[6];
		cndBE.b[2] = cndLE.b[5];
		cndBE.b[3] = cndLE.b[4];
		cndBE.b[4] = cndLE.b[3];
		cndBE.b[5] = cndLE.b[2];
		cndBE.b[6] = cndLE.b[1];
		cndBE.b[7] = cndLE.b[0];
    
		// MCCMD ���W�X�^�ݒ�
    	reg_HOTSW_MCCMD0 = *(u32 *)cndBE.b;
		reg_HOTSW_MCCMD1 = *(u32 *)&cndBE.b[4];

		// MCCNT1 ���W�X�^�ݒ� (START = 1  W/R = 0  PC = 001 (1�y�[�W���[�h) ��)
		reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        			             		 			 CNT1_FLD(1,0,0,0,  0,1,  0,0,  0,  0,0,0,  1540));

		// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
		OS_SleepThread(NULL);
        
        page++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal_DSType2
  
  Description:  DS�J�[�hType1�̃m�[�}�����[�h�̃��[�h�ύX
 *---------------------------------------------------------------------------*/
// Type1�Ɠ���


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
}


/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadIDSecure_DSType2(CardBootData *cbd)
{
	// NewDMA�]���̏���
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_RD_ID, cbd);
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 1 TRM = 0 PC = 0 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  1,  0,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  1,0,  0,0,  0,  0,1,1,  0));

	// 25ms�҂�
    OS_Sleep(COMMAND_DECRYPTION_WAIT);

    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 1 PC = 111 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  1,  0,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  1,7,  0,0,  0,  0,1,1,  0));

	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);
    
    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;
}

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_DSType2
  
  Description:  Secure�̈��ǂݍ��ފ֐�

  �� �{���Ȃ�A�w�肵���Z�O�����g��ǂރR�}���h�����ǁA�����4��A������
     �Ăяo���āASecure�̈�S����ǂݍ���ł���
 *---------------------------------------------------------------------------*/
void ReadSegSecure_DSType2(CardBootData *cbd)
{
    u32			i,j=0,k;
	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    for(i=0; i<SECURE_SEGMENT_NUM; i++){
		// �[���N���A
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

		// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

    	// MCCNT1 ���W�X�^�ݒ�
    	// (START = 1 W/R = 0 TRM = 0 PC = 000(0�y�[�W) CS = 1 Latency2 =0 SE = 1 DS = 1 Latency1 = 0��)
    	reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  0,  0,0,0,  0)) |
        		             		 	 			 CNT1_FLD(1,0,0,0,  1,0,  0,0,  0,  0,1,1,  0));

	    // 25ms�҂�
    	OS_Sleep(COMMAND_DECRYPTION_WAIT);
        
		for(k=0; k<ONE_SEGMENT_PAGE_NUM; k++){
			// NewDMA�]���̏���
			HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pSecureSegBuf + (PAGE_WORD_SIZE * j), PAGE_SIZE );

    		// MCCMD ���W�X�^�ݒ�
			reg_HOTSW_MCCMD0 = 0x0;
			reg_HOTSW_MCCMD1 = 0x0;
    		
    		// (START = 1 W/R = 0 TRM = 0 PC = 001(1�y�[�W���[�h) CS = 1 Latency2 = 0 SE = 1 DS = 1 Latency1 = 1540��)
            // latency1 : 1540 --> Output Latency = 230us �]���N���b�N�^�C�v = 0�Ŏ�����150ns ������ 230000 / 150 = 1533.33
    		reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  0,  0,0,0,     0)) |
        		             		 	 	 			 CNT1_FLD(1,0,0,0,  1,1,  0,0,  0,  0,1,1,  1540));

			// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
			OS_SleepThread(NULL);

            // �]���ς݃y�[�W��
            j++;
		}

        // �ǂݍ��݃Z�O�����g�ԍ��C���N�������g
		segNum++;
        
    	// �R�}���h�J�E���^�C���N�������g
		cbd->vbi++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchONPNGSecure_DSType2(CardBootData *cbd)
{
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_ON, cbd);
    
	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,1,1,  0));

    // 25ms�҂�
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,0,0,  0));

	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchOFFPNGSecure_DSType2(CardBootData *cbd)
{
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_OFF, cbd);
    
	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,1,1,  0));

    // 25ms�҂�
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,0,0,  0));

	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void ChangeModeSecure_DSType2(CardBootData *cbd)
{
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_CHG_MODE, cbd);
    
	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,1,1,  0));

    // 25ms�҂�
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 Latency1 = 0 ��)
    reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 			 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,0,0,  0));

	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;
}


// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_DSType2
  
  Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
// Type1�Ɠ���


/*---------------------------------------------------------------------------*
  Name:         ReadPageGame_DSType1
  
  Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 *---------------------------------------------------------------------------*/
void ReadPageGame_DSType2(u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
    GCDCmd64 	cndLE, cndBE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    OS_TPrintf("Src Addr : 0x%08x  Dst Addr : 0x%08x\n", start_addr, buf);
    OS_TPrintf("Read Game Segment  Page Count : %d   size : %x\n", loop, size);
    
    for(i=0; i<loop; i++){
		// NewDMA�]���̏���
		HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );

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
        
   		 // MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 001(1�y�[�W���[�h) CS = 1 SE = 1 DS = 1 latency1 = 1540 ��)
		reg_HOTSW_MCCNT1 = (u32)((reg_HOTSW_MCCNT1 & CNT1_MSK(0,0,1,0,  1,0,  1,0,  0,  0,0,0,     0)) |
    				             		 			 CNT1_FLD(1,0,0,0,  0,1,  0,1,  0,  0,1,1,  1540));

		// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
		OS_SleepThread(NULL);
    }
}