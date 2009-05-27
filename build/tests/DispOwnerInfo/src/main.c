/********************************************************************/
/*      main.c                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	���C��

	$Log: main.c,v $
	Revision 1.7  2005/04/06 08:26:57  yosiokat
	IPL2�t�H���g�`�F�b�N�ł̃������m�ۃT�C�Y�����ɔ����A�q�[�v�T�C�Y��0x100000�ɑ����B
	
	Revision 1.6  2005/04/02 07:41:57  yosiokat
	�EIPL2�t�H���g�\���ɑΉ��B
	�E�N�����̏��������[�`����initFunc.c�ɕ����B
	
	Revision 1.5  2005/04/01 05:46:48  yosiokat
	�f�o�b�O�ϐ����\���ǉ��B
	
	Revision 1.4  2005/03/31 05:58:17  yosiokat
	�ENVRAM�A�N�Z�X�֐����C�����A�ʃ\�[�X�ɕ����B
	�EIPL2�w�b�_���擾���p�֐���p����悤�ύX�B
	�EGetIPL2CRC16��NVRAM�A�N�Z�X�֐���ύX�B
	
	Revision 1.3  2005/03/09 04:44:39  yosiokat
	�@�\�ǉ��B
	
	Revision 1.2  2004/09/23 11:28:32  Yosiokat
	�{�̃t���b�V���Ɋi�[����Ă���IPL2 ver��\������悤�ύX�B
	
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

// ���C��
void NitroMain( void )
{
	/* OS ������ */
	OS_InitPrintServer();
	OS_Init();
//	OS_InitChina();			// SDK 2.01�̏ꍇ
//	OS_InitChina( isbn );	// SDK 5.1 �̏ꍇ
	PXI_InitFifo();
	OS_InitThread();
	OS_InitTick();
	OS_InitAlarm();
	FX_Init();
	TP_Init();
	(void)RTC_Init();
	
	/* GX ������ */
	GX_Init();
	GX_DispOff();
	GXS_DispOff();
	
	/* V�u�����N�����ݒ� */
	(void)OS_SetIrqFunction( OS_IE_V_BLANK, VBlankIntr );
	(void)OS_EnableIrqMask( OS_IE_V_BLANK );
	(void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	(void)GX_VBlankIntr( TRUE );
	
	{	/* ���������蓖�Ă̏����� */
		enum { MAIN_HEAP_SIZE = 0x100000 } ;
		void	*heapStart, *nstart;
		int		hh;
		
		nstart		= OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 16 );
		OS_SetMainArenaLo( nstart );
		heapStart	= OS_AllocFromMainArenaLo( (u32)MAIN_HEAP_SIZE, 32 );
		hh			= OS_CreateHeap( OS_ARENA_MAIN, heapStart, (void*)( (u32)heapStart + MAIN_HEAP_SIZE ) );
	 	(void)OS_SetCurrentHeap( OS_ARENA_MAIN, hh );
	}
	
	{	/* FS ������ */
		static u32 fs_tablework[0x100 / 4];
		FS_Init(FS_DMA_NOT_USE);
		(void)FS_LoadTable(fs_tablework, sizeof(fs_tablework));
	}
	
	// Disp������
	InitDisp();
	InitIPL2Font();
	/* �\���J�n */
	GX_DispOn();
	GXS_DispOn();
	
	// �������C�����[�v�֐��̐ݒ�
	pad.cont = 0xffff;
	
	SEQ_DispNCD_init();
	nowProcess = SEQ_DispNCD;
	
	/* ���C�����[�v */
	while(1) {
		SVC_WaitVBlankIntr();
		ReadKeyPad();
		
		(void)nowProcess();
		
		OS_PrintServer();
	}
}


// �u�u�����N���荞�ݏ���
static void VBlankIntr( void )
{
	// OAM, BG-VRAM�̍X�V
	DC_FlushRange ( oamBakM,	sizeof(oamBakM) );
	DC_FlushRange ( oamBakS,	sizeof(oamBakS) );
	DC_FlushRange ( bgBakM,		sizeof(bgBakM) );
	DC_FlushRange ( bgBakS,		sizeof(bgBakS) );
	MI_CpuCopyFast( oamBakM,	(void*)HW_OAM,					 sizeof(oamBakM) );
	MI_CpuCopyFast( oamBakS,	(void*)HW_DB_OAM,				 sizeof(oamBakS) );
	MI_CpuCopyFast( bgBakM,		(void*)(HW_BG_VRAM + 0xf000),    sizeof(bgBakM) );
	MI_CpuCopyFast( bgBakS,		(void*)(HW_DB_BG_VRAM + 0xf000), sizeof(bgBakS) );
	
	// BG2�L�����N�^�ɓ]���B
	DC_FlushRange  ( context_m.canvas,    sizeof( canvas_m ) );
	GX_LoadBG2Char ( context_m.canvas, 0, sizeof( canvas_m ) );
	DC_FlushRange  ( context_s.canvas,    sizeof( canvas_s ) );
	GXS_LoadBG2Char( context_s.canvas, 0, sizeof( canvas_s ) );
	
    //---- ���荞�݃`�F�b�N�t���O
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}


// �L�[���̓f�[�^�ǂݏo��
void ReadKeyPad( void )
{
	u16 readData = PAD_Read();
	pad.trg	 = (u16)(readData & (readData ^ pad.cont));				// �g���K ����
	pad.cont = readData;											//   �x�^ ����
}

