/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/

#include 	<twl.h>

#include 	<sysmenu/card/common/blowfish.h>
#include 	<sysmenu/card/common/Card.h>
#include	<sysmenu/card/common/dsCardType2.h>

// Define Data --------------------------------------------------------------
#define		SECURE_SEGMENT_NUM					4
#define		ONE_SEGMENT_PAGE_NUM				8
#define		COMMAND_DECRYPTION_WAIT				25 // 25ms


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
	#pragma unused( cbd )
    
	u32 i = 0, j = 0;
	GCDCmd64 tempCnd, cnd;
    u64 page = 0;

    for(i=0; i<ONE_SEGMENT_PAGE_NUM; i++){
        // �[���N���A
		MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
        
    	// ���g���G���f�B�A���ō����
		tempCnd.dw  = 0x0  << 24;
		tempCnd.dw |= page << 33;
//    	tempCnd.dw |= 0x0  << 56;

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
    	reg_MCCMD0 = *(u32 *)cnd.b;
		reg_MCCMD1 = *(u32 *)&cnd.b[4];

		// MCCNT1 ���W�X�^�ݒ� (START = 1  W/R = 0  PC = 001 (1�y�[�W���[�h) ��)
		reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        			             		 CNT1_FLD(1,0,0,0,  0,1,  0,0,  0,  0,0,0,  1540));
    
		// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
		while(reg_MCCNT1 & START_FLG_MASK){
			while(!(reg_MCCNT1 & READY_FLG_MASK)){}
        	*(cbd->pBootSegBuf->word + j++) = reg_MCD1;
		}

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
	reg_MCCMD0 = *(u32*)cndBE.b;
	reg_MCCMD1 = *(u32*)&cndBE.b[4];
}


/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadIDSecure_DSType2(CardBootData *cbd)
{
	OSTick start;
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_RD_ID, cbd);
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 1 TRM = 0 PC = 0 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  1,  0,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  1,0,  0,0,  0,  0,1,1,  0));

	// 25ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < COMMAND_DECRYPTION_WAIT){}

    // MCCMD ���W�X�^�ݒ�
	reg_MCCMD0 = 0x0;
	reg_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 1 PC = 111 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  1,  0,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  1,7,  0,0,  0,  0,1,1,  0));

	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
		cbd->id_scr = reg_MCD1;
	}
    
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
    OSTick		start;

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
		reg_MCCMD0 = *(u32*)cndBE.b;
		reg_MCCMD1 = *(u32*)&cndBE.b[4];

		// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
		reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

    	// MCCNT1 ���W�X�^�ݒ�
    	// (START = 1 W/R = 0 TRM = 0 PC = 000(0�y�[�W) CS = 1 Latency2 =0 SE = 1 DS = 1 Latency1 = 0��)
    	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  0,  0,0,0,  0)) |
        		             		 	 CNT1_FLD(1,0,0,0,  1,0,  0,0,  0,  0,1,1,  0));

		// 25ms�҂� (latency�Őݒ�ł���ȏ��wait���K�v������)
    	start = OS_GetTick();
    	while(OS_TicksToMilliSeconds(OS_GetTick()-start) < COMMAND_DECRYPTION_WAIT){}
        
		for(k=0; k<ONE_SEGMENT_PAGE_NUM; k++){
    		// MCCMD ���W�X�^�ݒ�
			reg_MCCMD0 = 0x0;
			reg_MCCMD1 = 0x0;
    		
    		// (START = 1 W/R = 0 TRM = 0 PC = 001(1�y�[�W���[�h) CS = 1 Latency2 = 0 SE = 1 DS = 1 Latency1 = 1540��)
            // latency1 : 1540 --> Output Latency = 230us �]���N���b�N�^�C�v = 0�Ŏ�����150ns ������ 230000 / 150 = 1533.33
    		reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  0,  0,0,0,     0)) |
        		             		 	 	 CNT1_FLD(1,0,0,0,  1,1,  0,0,  0,  0,1,1,  1540));

			// MCCNT���W�X�^��RDY�t���O���|�[�����O���āA�t���O����������f�[�^��MCD1���W�X�^�ɍēx�Z�b�g�B�X�^�[�g�t���O��0�ɂȂ�܂Ń��[�v�B
    		while(reg_MCCNT1 & START_FLG_MASK){
				while(!(reg_MCCNT1 & READY_FLG_MASK)){}
                *(cbd->pSecureSegBuf + j++) = reg_MCD1;
			}
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
	OSTick start;
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_ON, cbd);
    
	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,1,1,  0));

	// 25ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < COMMAND_DECRYPTION_WAIT){}

    // MCCMD ���W�X�^�ݒ�
	reg_MCCMD0 = 0x0;
	reg_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,0,0,  0));

    while(reg_MCCNT1 & START_FLG_MASK){}

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchOFFPNGSecure_DSType2(CardBootData *cbd)
{
	OSTick start;
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_OFF, cbd);
    
	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,1,1,  0));

	// 25ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < COMMAND_DECRYPTION_WAIT){}

    // MCCMD ���W�X�^�ݒ�
	reg_MCCMD0 = 0x0;
	reg_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,0,0,  0));

    while(reg_MCCNT1 & START_FLG_MASK){}

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_DSType2
  
  Description:  
 *---------------------------------------------------------------------------*/
void ChangeModeSecure_DSType2(CardBootData *cbd)
{
	OSTick start;
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_CHG_MODE, cbd);
    
	// MCCNT0 ���W�X�^�ݒ� (E = 1  I = 1  SEL = 0��)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 SE = 1 DS = 1 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,1,1,  0));

	// 25ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < COMMAND_DECRYPTION_WAIT){}

    // MCCMD ���W�X�^�ݒ�
	reg_MCCMD0 = 0x0;
	reg_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 TRM = 0 PC = 000 Latency1 = 0 ��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,0,  0,0,  0,  0,0,0,  0));

    while(reg_MCCNT1 & START_FLG_MASK){}

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