/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     hotsw_ctrl.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <sysmenu.h>

#include <firm/os/common/system.h>
#include <../ARM7/include/hotswTypes.h>
#include <../ARM7/include/customNDma.h>

// Extern -------------------------------------------------------------------
extern CardThreadData HotSwThreadData;

// define -------------------------------------------------------------------
#define HOTSW_READ_MSG_NUM				1
#define HOTSW_CARD_GAME_MODE_CALLBACK	1
#define HOTSW_CARD_INSERT_CALLBACK		2
#define HOTSW_CARD_PULLOUT_CALLBACK		3
#define HOTSW_CALLBACK_FUNCTION_NUM		3

// Function prototype -------------------------------------------------------
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);
static CardDataReadState ReadPageGame(u32 start_addr, void* buf, u32 size);

// Static Values ------------------------------------------------------------
static HotSwMessageForArm9	s_HotswMsg;
static OSMessage			s_HotswMsgBuffer[HOTSW_READ_MSG_NUM];
static OSMessageQueue  		s_HotswQueue;

static BOOL					s_ReadBusy;
static OSIrqFunction		s_HotswFuncTable[HOTSW_CALLBACK_FUNCTION_NUM];

static u16                  s_CardLockID;


// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init
  
  Description:  ����}�������̏�����
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_Init()
{
    s32 tempLockID;
    
    // �J�[�h�A�N�Z�X�p�̃��b�NID�̎擾
    while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
        // do nothing
    }
    s_CardLockID = (u16)tempLockID;
    
    // ���b�Z�[�W�L���[�̏�����
    OS_InitMessageQueue( &s_HotswQueue, &s_HotswMsgBuffer[0], HOTSW_READ_MSG_NUM );

    // Busy�t���O�𗎂Ƃ��Ă���
    s_ReadBusy = FALSE;
    
    // PXI������
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);

	while(!PXI_IsCallbackReady(PXI_FIFO_TAG_HOTSW, PXI_PROC_ARM7))
    {
		// do nothing
    }
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_EnableHotSWAsync
  
  Description:  PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm(�񓯊���)

  �� �����}�����ꎞ�I�ɗ}������ꍇ�͂�����̊֐����g���Ă��������B
 *---------------------------------------------------------------------------*/
void HOTSW_EnableHotSWAsync( BOOL enable )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    enable = enable ? 1 : 0;
    
	// ���݂̒l�Ɠ����Ȃ牽���������^�[��
	if( SYSMi_GetWork()->flags.hotsw.isEnableHotSW == enable ) {
		return;
	}

    msg.msg.finalize = FALSE;
    msg.msg.ctrl  	 = TRUE;
	msg.msg.value 	 = enable;
    
	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
	{
		// do nothing
	}
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_EnableHotSWAsync
  
  Description:  PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm(������)

  �� �����}�����ꎞ�I�ɗ}������ꍇ�͂�����̊֐����g���Ă��������B
 *---------------------------------------------------------------------------*/
void HOTSW_EnableHotSW( BOOL enable )
{
	HOTSW_EnableHotSWAsync( enable );

    while(HOTSW_isEnableHotSW() != enable){
		// do nothing
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_InvalidHotSWAsync
  
  Description:  PXI�ʐM��ARM7�Ɋ����}��������ʒm�B(�񓯊���)

  �� �A�v�����u�[�g������Ƃ��Ɋ����}����}������ꍇ�͂�����̊֐����g���Ă��������B
 *---------------------------------------------------------------------------*/
void HOTSW_InvalidHotSWAsync( void )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    
	// ���݂̒l�Ɠ����Ȃ牽���������^�[��
	if( SYSMi_GetWork()->flags.hotsw.isEnableHotSW == FALSE ) {
		return;
	}

    msg.msg.finalize = TRUE;
    msg.msg.ctrl     = TRUE;
	msg.msg.value    = FALSE;
    
	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
	{
		// do nothing
	}
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_InvalidHotSW
  
  Description:  PXI�ʐM��ARM7�Ɋ����}��������ʒm�B(������)

  �� �A�v�����u�[�g������Ƃ��Ɋ����}����}������ꍇ�͂�����̊֐����g���Ă��������B
 *---------------------------------------------------------------------------*/
void HOTSW_InvalidHotSW( void )
{
	HOTSW_InvalidHotSWAsync();

    while(HOTSW_isEnableHotSW() != FALSE){
		// do nothing
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_FinalizeHotSW
  
  Description:  PXI�ʐM��ARM7�Ɋ����}��Finalize������ʒm
 *---------------------------------------------------------------------------*/
void HOTSW_FinalizeHotSWAsync( HotSwCardState cardState )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    
    msg.msg.finalize = TRUE;
    msg.msg.ctrl     = FALSE;
    msg.msg.cardState= (u8)cardState;
    
	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    {
    	// do nothing
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isFinalized
  
  Description:  �I��������������������Ԃ�
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isFinalized(void)
{
    return SYSMi_GetWork()->flags.hotsw.isFinalized;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isEnableHotSW
  
  Description:  �����}���̋���/�}���̏�Ԃ�Ԃ�
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isEnableHotSW(void)
{
    return SYSMi_GetWork()->flags.hotsw.isEnableHotSW;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isCardLoadCompleted
  
  Description:  �J�[�h�A�v���̃��[�h���������Ă��邩��Ԃ�
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isCardLoadCompleted(void)
{
    return SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isBusyHotSW
  
  Description:  �����}�����������ǂ�����Ԃ�
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isBusyHotSW(void)
{
    return SYSMi_GetWork()->flags.hotsw.isBusyHotSW;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_ReadCardDataAsync
  
  Description:  �J�[�h�f�[�^��ǂݏo���֐��B(�񓯊���)
  				�����ۂɓǂ�ł�̂�ARM7��

  �� �{�֐����Ăяo���āACARD_READ_SUCCESS���Ԃ��Ă��Ă��A�f�[�^��������
  	 ���[�h�ł��Ă���Ƃ͂�����܂���B
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
CardDataReadState HOTSW_ReadCardDataAsync(void* src, void* dest, u32 size)
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));

    if( !HOTSW_isGameMode() ){
		return CARD_READ_MODE_ERROR;
    }

    if( s_ReadBusy ){
		return CARD_READ_BUSY;
    }
    s_ReadBusy = TRUE;
    
	SYSMi_GetWork()->cardReadParam.src  = (u32)src;
    SYSMi_GetWork()->cardReadParam.dest = (u32)dest;
	SYSMi_GetWork()->cardReadParam.size = size;

    DC_FlushRange( &SYSMi_GetWork()->cardReadParam, sizeof(CardReadParam) );
	DC_FlushRange( dest, size );
    
    msg.msg.read = TRUE;
	
	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    {
    	// do nothing
    }

    return CARD_READ_SUCCESS;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         HOTSW_ReadCardData
  
  Description:  �J�[�h�f�[�^��ǂݏo���֐��B(������)
  				�����ۂɓǂ�ł�̂�ARM7��
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
CardDataReadState HOTSW_ReadCardData(void* src, void* dest, u32 size)
{
	HotSwMessageForArm9 	*msg;
	CardDataReadState   	retval;
    
	retval = HOTSW_ReadCardDataAsync( src, dest, size);

    if(retval != CARD_READ_SUCCESS){
		return retval;
    }
    
    OS_ReceiveMessage( &s_HotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK );

    return (CardDataReadState)msg->result;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isGameMode
  
  Description:  �J�[�h���Q�[�����[�h�ɂȂ������ǂ�����Ԃ�
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
BOOL HOTSW_isGameMode(void)
{
	return SYSMi_GetWork()->flags.hotsw.isCardGameMode ? TRUE : FALSE;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackPxi
  
  Description:  PXI���荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
    HotSwPxiMessageForArm9 d;

	MI_CpuClear8( &d, sizeof(HotSwPxiMessageForArm9) );
    
    d.data = data;

    if(d.msg.mode){
        if(s_HotswFuncTable[HOTSW_CARD_GAME_MODE_CALLBACK] != NULL){
			s_HotswFuncTable[HOTSW_CARD_GAME_MODE_CALLBACK]();
        }
    }

    if(d.msg.insert){
        if(s_HotswFuncTable[HOTSW_CARD_INSERT_CALLBACK] != NULL){
			s_HotswFuncTable[HOTSW_CARD_INSERT_CALLBACK]();
        }
    }

    if(d.msg.pullout){
        if(s_HotswFuncTable[HOTSW_CARD_PULLOUT_CALLBACK] != NULL){
			s_HotswFuncTable[HOTSW_CARD_PULLOUT_CALLBACK]();
        }
    }
	
    
    if(d.msg.read){
		s_ReadBusy = FALSE;
        
    	s_HotswMsg.isReadComplete	= (d.msg.read) ? TRUE : FALSE;
		s_HotswMsg.result			= (CardDataReadState)d.msg.result;

        HOTSW_TPrintf("%s %d  Rcev Error Code[Arm9]:%x\n", __FUNCTION__, __LINE__, s_HotswMsg.result);
        
		// ���b�Z�[�W���M
		OS_SendMessage( &s_HotswQueue, (OSMessage *)&s_HotswMsg, OS_MESSAGE_NOBLOCK);
    }
}


/*---------------------------------------------------------------------------*
  Name:         SetGameModeCallBackFunction
  
  Description:  �J�[�h��Game���[�h�ɂȂ������̃R�[���o�b�N�֐���ݒ�

  �� HOTSW_Init���Ăяo�����O�ɐݒ肵�Ă�������
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_SetGameModeCallBackFunction(OSIrqFunction function)
{
	s_HotswFuncTable[HOTSW_CARD_GAME_MODE_CALLBACK] = function;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         SetCardInsertCallBackFunction
  
  Description:  �J�[�h���h���������̃R�[���o�b�N�֐���ݒ�

  �� HOTSW_Init���Ăяo�����O�ɐݒ肵�Ă�������
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_SetCardInsertCallBackFunction(OSIrqFunction function)
{
	s_HotswFuncTable[HOTSW_CARD_INSERT_CALLBACK] = function;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         SetCardPullOutCallBackFunction
  
  Description:  �J�[�h�����������̃R�[���o�b�N�֐���ݒ�

  �� HOTSW_Init���Ăяo�����O�ɐݒ肵�Ă�������
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_SetCardPullOutCallBackFunction(OSIrqFunction function)
{
	s_HotswFuncTable[HOTSW_CARD_PULLOUT_CALLBACK] = function;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_ReadCardDataOnGameMode
  
  Description:  Game���[�h�̃f�[�^�ǂ݊֐�
 *---------------------------------------------------------------------------*/
CardDataReadState HOTSW_ReadCardDataOnGameMode(const void* src, void* dest, u32 size)
{
    CardDataReadState retval = CARD_READ_SUCCESS;

    static u8 page_buffer[512];
    u32 offset      	= (u32)src;
    u32 page_offset 	= (u32)(offset & -512);
    u32 buffer_offset 	= (u32)(offset % 512);
    u32 valid_length 	= 512 - buffer_offset;
    u32 remain_length;

    // �J�n�A�h���X���y�[�W�̓r��
    if ( offset % 512 )
    {
        retval = ReadPageGame(page_offset, page_buffer, 512);

        if (retval != HOTSW_SUCCESS)
        {
            return retval;
        }

        MI_CpuCopy8(page_buffer + buffer_offset, dest, (size < valid_length ? size : valid_length));

        dest   = (u8*)dest + valid_length;
        offset += valid_length;
        size   -= valid_length;
        if ( size < 0)
        {
            return retval;
        }
    }

    remain_length = (u32)(size % 512);
    retval = ReadPageGame(offset, dest, (u32)(size - remain_length));

    // �P�c���y�[�W�r��
    if( remain_length ){
        dest   = (u8*)dest + (size - remain_length);
        offset += size - remain_length;

        retval = ReadPageGame(offset, page_buffer, 512);

        if (retval != HOTSW_SUCCESS)
        {
            return retval;
        }

        MI_CpuCopy8(page_buffer, dest, remain_length);
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         ReadPageGame
  
  Description:  Game���[�h��Page�ǂ݊֐�
 *---------------------------------------------------------------------------*/
static CardDataReadState ReadPageGame(u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
	GCDCmd64	cndLE, cndBE;

#ifdef USE_NEW_DMA
	OSMessage	msg;
#endif
    
    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    // �J�[�h�̃��b�N
    CARD_LockRom(s_CardLockID);
    
    for(i=0; i<loop; i++){
	    if(!HOTSW_isGameMode()){
			return CARD_READ_MODE_ERROR;
    	}
        
#ifdef USE_NEW_DMA
		// NewDMA�]���̏���
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#endif
        
        // �R�}���h�쐬
		cndLE.dw  = HSWOP_G_OP_RD_PAGE;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

	    // �r�b�O�G���f�B�A���ɒ���
		cndBE.b[7] = cndLE.b[0];
		cndBE.b[6] = cndLE.b[1];
    	cndBE.b[5] = cndLE.b[2];
	    cndBE.b[4] = cndLE.b[3];
	    cndBE.b[3] = cndLE.b[4];
    	cndBE.b[2] = cndLE.b[5];
	    cndBE.b[1] = cndLE.b[6];
    	cndBE.b[0] = cndLE.b[7];

		//---- confirm CARD free
		while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){}

    	// MCCMD ���W�X�^�ݒ�
		reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
		reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];

		// MCCNT0 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
        
   		// MCCNT1 ���W�X�^�ݒ�
		reg_HOTSW_MCCNT1 = SYSMi_GetWork()->gameCommondParam | START_MASK | HOTSW_PAGE_1;

#ifdef USE_NEW_DMA
		// ���b�Z�[�W��M
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);
#else
		while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
            *((u32 *)buf + counter++) = reg_HOTSW_MCD1;
		}
#endif
    }

	// 100ns Wait
    OS_SpinWait( OS_NSEC_TO_CPUCYC(100) );

    // �J�[�h�̃��b�N�J��(�����b�NID�͊J�������Ɏ���������)
    CARD_UnlockRom(s_CardLockID);
    
    return CARD_READ_SUCCESS;
}
