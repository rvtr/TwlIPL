/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType1.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include	<customNDma.h>

// define -------------------------------------------------------------------
#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

// Function prototype -------------------------------------------------------


// ===========================================================================
// 	Function Describe
// ===========================================================================

// ��------------------------------------��
// ��       �m�[�}�����[�h�̃R�}���h     ��
// ��------------------------------------��
/* -----------------------------------------------------------------
 * ReadIDNormal�֐�
 *
 * �m�[�}�����[�h���̃J�[�hID��ǂݍ��ފ֐�
 * ----------------------------------------------------------------- */
HotSwState ReadIDNormal(CardBootData *cbd)
{
	// �J�[�h���荞�݂ɂ��DMA�R�s�[
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_nml, sizeof(cbd->id_nml) );

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
 * Name:         ChangeModeNormal
 * 
 * Description:  Type1�̃m�[�}�����[�h�̃��[�h�ύX
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal(CardBootData *cbd)
{
	GCDCmd64 tempCnd, cnd;
    u64 vae64 = cbd->vae;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
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
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~SCRAMBLE_MASK) |
        				START_MASK | (PC_MASK & (0x0 << PC_SHIFT));
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

    return HOTSW_SUCCESS;
}

/* -----------------------------------------------------------------
 * LoadTable�֐�
 *
 * �J�[�h���� Key Table �����[�h����֐��B
 *
 * �����̊֐��͊J���J�[�h�p�ɔ��s���Ȃ��Ƃ����Ȃ��B
 *   ���i�ŃJ�[�h�̏ꍇ�A���̃R�}���h�͖��������
 * ----------------------------------------------------------------- */
HotSwState LoadTable(void)
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

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ReadRomEmulationData
  
  Description:  Rom�G�~�����[�V�����f�[�^�̓ǂݍ���
 *---------------------------------------------------------------------------*/
HotSwState ReadRomEmulationData(CardBootData *cbd)
{
	u32 count=0;
    u32 temp;
    u32 *dst = cbd->romEmuBuf;

    // �ʎY�pCPU�ł͕����A�N�Z�X�h�~�̂��߃��[�h���Ȃ�
    if ( ! (*(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK) )
    {
        return HOTSW_SUCCESS;
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

   	MI_CpuCopyFast(cbd->romEmuBuf, (void*)HW_ISD_RESERVED, 32);

    return HOTSW_SUCCESS;
}

// ��------------------------------------��
// ��       �Q�[�����[�h�̃R�}���h       ��
// ��------------------------------------��
/*---------------------------------------------------------------------------*
 * Name:         ReadIDGame
 * 
 * Description:  �Q�[�����[�h��ID��ǂݍ���
 *---------------------------------------------------------------------------*/
HotSwState ReadIDGame(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA�]���̏���
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );

	// MCCMD ���W�X�^�ݒ�
	reg_HOTSW_MCCMD0 = 0x000000B8;
	reg_HOTSW_MCCMD1 = 0x00000000;

   	// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 111(�X�e�[�^�X���[�h) ���̑�Rom�w�b�_�̏��ɂ��܂���)
	reg_HOTSW_MCCNT1 = cbd->gameCommondParam |
        				START_MASK | (PC_MASK & (0x7 << PC_SHIFT));
    
    // �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
    OS_SleepThread(NULL);

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadPageGame
 * 
 * Description:  �Q�[�����[�h�ŁA�w�肳�ꂽ�y�[�W���w��o�b�t�@�Ɏw��T�C�Y����ǂݍ���
 *
 * CT=150ns  Pagecount=1page  Latency=RomHeader�Ŏw��̒l
 *---------------------------------------------------------------------------*/
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
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
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
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
        
   		// MCCNT1 ���W�X�^�ݒ� (START = 1 W/R = 0 PC = 001(1�y�[�W���[�h) ���̑�Rom�w�b�_�̏��ɂ��܂���)
		reg_HOTSW_MCCNT1 = cbd->gameCommondParam |
            				START_MASK | (PC_MASK & (0x1 << PC_SHIFT));
        
		// �J�[�h�f�[�^�]���I�����荞�݂��N����܂ŐQ��(���荞�݃n���h���̒��ŋN�������)
		OS_SleepThread(NULL);
    }

    return HOTSW_SUCCESS;
}