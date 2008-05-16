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

// define -------------------------------------------------------------------
#define HOTSW_READ_MSG_NUM				1
#define HOTSW_CARD_GAME_MODE_CALLBACK	1
#define HOTSW_CARD_INSERT_CALLBACK		2
#define HOTSW_CARD_PULLOUT_CALLBACK		3
#define HOTSW_CALLBACK_FUNCTION_NUM		3

// Function prototype -------------------------------------------------------
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);

// Static Values ------------------------------------------------------------
static HotSwMessageForArm9	s_HotswMsg;
static OSMessage			s_HotswMsgBuffer[HOTSW_READ_MSG_NUM];
static OSMessageQueue  		s_HotswQueue;

static BOOL					s_ReadBusy;
static OSIrqFunction		s_HotswFuncTable[HOTSW_CALLBACK_FUNCTION_NUM];


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
  
  Description:  PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm
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
	
	msg.msg.value = enable;
	msg.msg.ctrl  = TRUE;

	OS_TPrintf("%s %d\n", __FUNCTION__, __LINE__);
    
	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
	{
		// do nothing
	}
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_FinalizeHotSW
  
  Description:  PXI�ʐM��ARM7�Ɋ����}��Finalize������ʒm
 *---------------------------------------------------------------------------*/
void HOTSW_FinalizeHotSWAsync( HotSwApliType apliType )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    
    msg.msg.finalize = TRUE;
    msg.msg.bootType = (u8)apliType;

    OS_TPrintf("%s %d\n", __FUNCTION__, __LINE__);
    
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

        OS_TPrintf("%s %d  Rcev Error Code[Arm9]:%x\n", __FUNCTION__, __LINE__, s_HotswMsg.result);
        
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
