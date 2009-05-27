/********************************************************************/
/*      main.c                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	メイン

	$Log: main.c,v $
	Revision 1.7  2005/04/06 08:26:57  yosiokat
	IPL2フォントチェックでのメモリ確保サイズ増加に伴い、ヒープサイズを0x100000に増加。
	
	Revision 1.6  2005/04/02 07:41:57  yosiokat
	・IPL2フォント表示に対応。
	・起動時の初期化ルーチンをinitFunc.cに分離。
	
	Revision 1.5  2005/04/01 05:46:48  yosiokat
	デバッグ変数＆表示追加。
	
	Revision 1.4  2005/03/31 05:58:17  yosiokat
	・NVRAMアクセス関数を修正し、別ソースに分離。
	・IPL2ヘッダ情報取得を専用関数を用いるよう変更。
	・GetIPL2CRC16のNVRAMアクセス関数を変更。
	
	Revision 1.3  2005/03/09 04:44:39  yosiokat
	機能追加。
	
	Revision 1.2  2004/09/23 11:28:32  Yosiokat
	本体フラッシュに格納されているIPL2 verを表示するよう変更。
	
	Revision 1.1.1.1  2004/08/31 06:20:24  Yosiokat
	no message
	

*/


#include <nitro.h>
#include "data.h"
#include "main.h"

// define data---------------------------------------------

// extern data---------------------------------------------

// function's prototype------------------------------------
static void VBlankIntr( void );

// global variables----------------------------------------

// static variables----------------------------------------


// const data----------------------------------------------



// ==================================================================
/* function */

static const char *isbn[4] = { "0-000000-00-0", "000-0000-000", "0000", " 000" };

// メイン
void NitroMain( void )
{
	/* OS 初期化 */
	OS_InitPrintServer();
	OS_Init();
//	OS_InitChina();			// SDK 2.01の場合
//	OS_InitChina( isbn );	// SDK 5.1 の場合
	PXI_InitFifo();
	OS_InitThread();
	OS_InitTick();
	OS_InitAlarm();
	FX_Init();
	TP_Init();
	(void)RTC_Init();
	
	/* GX 初期化 */
	GX_Init();
	GX_DispOff();
	GXS_DispOff();
	
	/* Vブランク割込設定 */
	(void)OS_SetIrqFunction( OS_IE_V_BLANK, VBlankIntr );
	(void)OS_EnableIrqMask( OS_IE_V_BLANK );
	(void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	(void)GX_VBlankIntr( TRUE );
	
	{	/* メモリ割り当ての初期化 */
		enum { MAIN_HEAP_SIZE = 0x100000 } ;
		void	*heapStart, *nstart;
		int		hh;
		
		nstart		= OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 16 );
		OS_SetMainArenaLo( nstart );
		heapStart	= OS_AllocFromMainArenaLo( (u32)MAIN_HEAP_SIZE, 32 );
		hh			= OS_CreateHeap( OS_ARENA_MAIN, heapStart, (void*)( (u32)heapStart + MAIN_HEAP_SIZE ) );
	 	(void)OS_SetCurrentHeap( OS_ARENA_MAIN, hh );
	}
	
	{	/* FS 初期化 */
		static u32 fs_tablework[0x100 / 4];
		FS_Init(FS_DMA_NOT_USE);
		(void)FS_LoadTable(fs_tablework, sizeof(fs_tablework));
	}
	
	// Disp初期化
	InitDisp();
	InitIPL2Font();
	/* 表示開始 */
	GX_DispOn();
	GXS_DispOn();
	
	// 初期メインループ関数の設定
	pad.cont = 0xffff;
	
	SEQ_DispNCD_init();
	nowProcess = SEQ_DispNCD;
	
	/* メインループ */
	while(1) {
		SVC_WaitVBlankIntr();
		ReadKeyPad();
		
		(void)nowProcess();
		
		OS_PrintServer();
	}
}


// Ｖブランク割り込み処理
static void VBlankIntr( void )
{
	// OAM, BG-VRAMの更新
	DC_FlushRange ( oamBakM,	sizeof(oamBakM) );
	DC_FlushRange ( oamBakS,	sizeof(oamBakS) );
	DC_FlushRange ( bgBakM,		sizeof(bgBakM) );
	DC_FlushRange ( bgBakS,		sizeof(bgBakS) );
	MI_CpuCopyFast( oamBakM,	(void*)HW_OAM,					 sizeof(oamBakM) );
	MI_CpuCopyFast( oamBakS,	(void*)HW_DB_OAM,				 sizeof(oamBakS) );
	MI_CpuCopyFast( bgBakM,		(void*)(HW_BG_VRAM + 0xf000),    sizeof(bgBakM) );
	MI_CpuCopyFast( bgBakS,		(void*)(HW_DB_BG_VRAM + 0xf000), sizeof(bgBakS) );
	
	// BG2キャラクタに転送。
	DC_FlushRange  ( context_m.canvas,    sizeof( canvas_m ) );
	GX_LoadBG2Char ( context_m.canvas, 0, sizeof( canvas_m ) );
	DC_FlushRange  ( context_s.canvas,    sizeof( canvas_s ) );
	GXS_LoadBG2Char( context_s.canvas, 0, sizeof( canvas_s ) );
	
    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}


// キー入力データ読み出し
void ReadKeyPad( void )
{
	u16 readData = PAD_Read();
	pad.trg	 = (u16)(readData & (readData ^ pad.cont));				// トリガ 入力
	pad.cont = readData;											//   ベタ 入力
}

