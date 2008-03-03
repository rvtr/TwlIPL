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
  
  Description:  Type2�̃m�[�}�����[�h��ID�ǂݍ���
 *---------------------------------------------------------------------------*/
// ����

/*---------------------------------------------------------------------------*
 * Name:         ReadBootSegNormal_DSType2
 * 
 * Description:  Type2�̃m�[�}�����[�h��Boot Segment�ǂݍ���
 * 
 * CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
 *---------------------------------------------------------------------------*/
HotSwState ReadBootSegNormal_DSType2(CardBootData *cbd)
{
	u32 		i = 0;
    u32			loop = ONE_SEGMENT_PAGE_NUM;
    u32 		*dst = cbd->pBootSegBuf->word;
    u64 		page = 0;
    GCDCmd64 	cndLE, cndBE;
    
    for(i=0; i<ONE_SEGMENT_PAGE_NUM; i++){
    	if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
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

		// MCCNT1 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x1 << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;

		// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
		OS_SleepThread(NULL);

        page++;
    }

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeNormal_DSType2
 * 
 * Description:  Type2�̃m�[�}�����[�h�̃��[�h�ύX
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
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
 * Name:         ReadIDSecure_DSType2
 * 
 * Description:  �f�o�b�K��ǂݍ��񂾏ꍇ��SCRAMBLE_MASK -> CS SE DS ���}�X�N
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // �X�N�����u���̐ݒ�
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
	// NewDMA�]���̏���
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_RD_ID, cbd);
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
	// 25ms�҂�
    OS_Sleep(COMMAND_DECRYPTION_WAIT);

    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);
    
    // �R�}���h�J�E���^�C���N�������g
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadSegSecure_DSType2
 * 
 * Description:  Secure�̈��ǂݍ��ފ֐�
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=1page
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure_DSType2(CardBootData *cbd)
{
    u32			i,j=0,k;
	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    // �X�N�����u���̐ݒ�
    u32 scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    for(i=0; i<SECURE_SEGMENT_NUM; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
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

		// MCCNT1 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
        
	    // 25ms�҂�
    	OS_Sleep(COMMAND_DECRYPTION_WAIT);
        
		for(k=0; k<ONE_SEGMENT_PAGE_NUM; k++){
			// NewDMA�]���̏���
			HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pSecureSegBuf + (PAGE_WORD_SIZE * j), PAGE_SIZE );

    		// MCCMD ���W�X�^�ݒ�
			reg_HOTSW_MCCMD0 = 0x0;
			reg_HOTSW_MCCMD1 = 0x0;
    		
			// MCCNT1 ���W�X�^�ݒ�
			reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x1 << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
            
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

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchONPNGSecure_DSType2
 * 
 * Description:  PN�W�F�l���[�^��ON�ɂ���
 * 
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // �X�N�����u���̐ݒ�
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_ON, cbd);
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 SE = 1 DS = 1 Latency1 = 0 ��)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
    // 25ms�҂�
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT1 ���W�X�^�ݒ� (START = 1 SE = 1 DS = 1 Latency1 = 0 ��)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchOFFPNGSecure_DSType2
 * 
 * Description:  PN�W�F�l���[�^��OFF����
 * 
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // �X�N�����u���̐ݒ�
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_PNG_OFF, cbd);
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    // 25ms�҂�
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ�
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeSecure_DSType2
 * 
 * Description:  Game���[�h�Ɉڍs����
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // �X�N�����u���̐ݒ�
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // �R�}���h�쐬�E�ݒ�
	SetSecureCommand(S_CHG_MODE, cbd);
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 SE = 1 DS = 1 Latency1 = 0 ��)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
    // 25ms�҂�
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 ���W�X�^�ݒ� (START = 1 SE = 1 DS = 1 Latency1 = 0 ��)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
	// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
	OS_SleepThread(NULL);

    // �R�}���h�J�E���^�C���N�������g
    cbd->vbi++;

    return HOTSW_SUCCESS;
}


// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_DSType2
  
  Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
// ����


/*---------------------------------------------------------------------------*
 * Name:         ReadPageGame_DSType2
 * 
 * Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 *---------------------------------------------------------------------------*/
// ����
