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
  
  Description:  活栓挿抜処理の初期化
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_Init()
{
    s32 tempLockID;
    
    // カードアクセス用のロックIDの取得
    while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
        // do nothing
    }
    s_CardLockID = (u16)tempLockID;
    
    // メッセージキューの初期化
    OS_InitMessageQueue( &s_HotswQueue, &s_HotswMsgBuffer[0], HOTSW_READ_MSG_NUM );

    // Busyフラグを落としておく
    s_ReadBusy = FALSE;
    
    // PXI初期化
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
  
  Description:  PXI通信でARM7に活線挿抜有効／無効を通知(非同期版)

  ※ 活線挿抜を一時的に抑制する場合はこちらの関数を使ってください。
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
  
  Description:  PXI通信でARM7に活線挿抜有効／無効を通知(同期版)

  ※ 活線挿抜を一時的に抑制する場合はこちらの関数を使ってください。
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
  
  Description:  PXI通信でARM7に活線挿抜無効を通知。(非同期版)

  ※ アプリをブートさせるときに活線挿抜を抑制する場合はこちらの関数を使ってください。
 *---------------------------------------------------------------------------*/
void HOTSW_InvalidHotSWAsync( void )
{
	HotSwPxiMessageForArm7 msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm7));
    
	// 現在の値と同じなら何もせずリターン
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
  
  Description:  PXI通信でARM7に活線挿抜無効を通知。(同期版)

  ※ アプリをブートさせるときに活線挿抜を抑制する場合はこちらの関数を使ってください。
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
  
  Description:  PXI通信でARM7に活線挿抜Finalize処理を通知
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
  
  Description:  終了処理が完了したかを返す
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isFinalized(void)
{
    return SYSMi_GetWork()->flags.hotsw.isFinalized;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isEnableHotSW
  
  Description:  活線挿抜の許可/抑制の状態を返す
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isEnableHotSW(void)
{
    return SYSMi_GetWork()->flags.hotsw.isEnableHotSW;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isCardLoadCompleted
  
  Description:  カードアプリのロードが完了しているかを返す
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isCardLoadCompleted(void)
{
    return SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isBusyHotSW
  
  Description:  活線挿抜処理中かどうかを返す
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isBusyHotSW(void)
{
    return SYSMi_GetWork()->flags.hotsw.isBusyHotSW;
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
        
		// メッセージ送信
		OS_SendMessage( &s_HotswQueue, (OSMessage *)&s_HotswMsg, OS_MESSAGE_NOBLOCK);
    }
}


/*---------------------------------------------------------------------------*
  Name:         SetGameModeCallBackFunction
  
  Description:  カードがGameモードになった時のコールバック関数を設定

  ※ HOTSW_Initが呼び出される前に設定してください
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_SetGameModeCallBackFunction(OSIrqFunction function)
{
	s_HotswFuncTable[HOTSW_CARD_GAME_MODE_CALLBACK] = function;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         SetCardInsertCallBackFunction
  
  Description:  カードが刺さった時のコールバック関数を設定

  ※ HOTSW_Initが呼び出される前に設定してください
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_SetCardInsertCallBackFunction(OSIrqFunction function)
{
	s_HotswFuncTable[HOTSW_CARD_INSERT_CALLBACK] = function;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         SetCardPullOutCallBackFunction
  
  Description:  カードが抜けた時のコールバック関数を設定

  ※ HOTSW_Initが呼び出される前に設定してください
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
void HOTSW_SetCardPullOutCallBackFunction(OSIrqFunction function)
{
	s_HotswFuncTable[HOTSW_CARD_PULLOUT_CALLBACK] = function;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_ReadCardDataOnGameMode
  
  Description:  Gameモードのデータ読み関数
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

    // 開始アドレスがページの途中
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

    // ケツがページ途中
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
  
  Description:  GameモードのPage読み関数
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

    // カードのロック
    CARD_LockRom(s_CardLockID);
    
    for(i=0; i<loop; i++){
	    if(!HOTSW_isGameMode()){
			return CARD_READ_MODE_ERROR;
    	}
        
#ifdef USE_NEW_DMA
		// NewDMA転送の準備
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#endif
        
        // コマンド作成
		cndLE.dw  = HSWOP_G_OP_RD_PAGE;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

	    // ビッグエンディアンに直す
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

    	// MCCMD レジスタ設定
		reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
		reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];

		// MCCNT0 レジスタ設定
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
        
   		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = SYSMi_GetWork()->gameCommondParam | START_MASK | HOTSW_PAGE_1;

#ifdef USE_NEW_DMA
		// メッセージ受信
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

    // カードのロック開放(※ロックIDは開放せずに持ち続ける)
    CARD_UnlockRom(s_CardLockID);
    
    return CARD_READ_SUCCESS;
}
