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
#define HOTSW_READ_MSG_NUM	1

// Function prototype -------------------------------------------------------
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);

// Static Values ------------------------------------------------------------
HotSwMessageForArm9		s_HotswMsg;
OSMessage				s_HotswMsgBuffer[HOTSW_READ_MSG_NUM];
OSMessageQueue  		s_HotswQueue;

BOOL					s_ReadBusy;

// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init
  
  Description:  活栓挿抜処理の初期化
 *---------------------------------------------------------------------------*/
void HOTSW_Init()
{
    // PXI初期化
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);

    // メッセージキューの初期化
    OS_InitMessageQueue( &s_HotswQueue, &s_HotswMsgBuffer[0], HOTSW_READ_MSG_NUM );

    // Busyフラグを落としておく
    s_ReadBusy = FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_EnableHotSWAsync
  
  Description:  PXI通信でARM7に活線挿抜有効／無効を通知
 *---------------------------------------------------------------------------*/
void HOTSW_EnableHotSWAsync( BOOL enable )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    enable = enable ? 1 : 0;
    
	// 現在の値と同じなら何もせずリターン
	if( SYSMi_GetWork()->flags.hotsw.isEnableHotSW == enable ) {
		return;
	}
	
	msg.msg.value = enable;
	msg.msg.ctrl  = TRUE;

	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
	{
		// do nothing
	}
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_FinalizeHotSW
  
  Description:  PXI通信でARM7に活線挿抜Finalize処理を通知
 *---------------------------------------------------------------------------*/
void HOTSW_FinalizeHotSWAsync( HotSwApliType apliType )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    
    msg.msg.finalize = TRUE;
    msg.msg.bootType = (u8)apliType;

	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    {
    	// do nothing
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isEnableHotSW
  
  Description:  活線挿抜の許可/抑制の状態を返す
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isEnableHotSW(void)
{
    return SYSMi_GetWork()->flags.hotsw.isEnableHotSW ? TRUE : FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isCardLoadCompleted
  
  Description:  カードアプリのロードが完了しているかを返す
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isCardLoadCompleted(void)
{
    return SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted ? TRUE : FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_ReadCardDataAsync
  
  Description:  カードデータを読み出す関数。(非同期版)
  				※実際に読んでるのはARM7側

  ※ 本関数を呼び出して、CARD_READ_SUCCESSが返ってきても、データが正しく
  	 リードできているとはかぎりません。
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
  
  Description:  カードデータを読み出す関数。(同期版)
  				※実際に読んでるのはARM7側
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
  
  Description:  カードがゲームモードになったかどうかを返す
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
BOOL HOTSW_isGameMode(void)
{
	return SYSMi_GetWork()->flags.hotsw.isCardGameMode ? TRUE : FALSE;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackPxi
  
  Description:  PXI割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
    HotSwPxiMessageForArm9 d;

    d.data = data;
    
    if(d.msg.read){
		s_ReadBusy = FALSE;
        
    	s_HotswMsg.isGameMode		= (d.msg.mode) ? TRUE : FALSE;
    	s_HotswMsg.isInsert			= (d.msg.insert) ? TRUE : FALSE;
    	s_HotswMsg.isPulledOut		= (d.msg.pullout) ? TRUE : FALSE;
    	s_HotswMsg.isReadComplete	= (d.msg.read) ? TRUE : FALSE;
		s_HotswMsg.result			= (CardDataReadState)d.msg.result;

        OS_TPrintf("%s %d  Rcev Error Code[Arm9]:%x\n", __FUNCTION__, __LINE__, s_HotswMsg.result);
        
		// メッセージ送信
		OS_SendMessage( &s_HotswQueue, (OSMessage *)&s_HotswMsg, OS_MESSAGE_NOBLOCK);
    }
}