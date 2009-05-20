/********************************************************************/
/*      initFunc.c                                                  */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	�������֐��Q

	$Log: initFunc.c,v $
	Revision 1.2.10.1  2007/01/22 07:36:16  yosiokat
	NAT-IPL2�ւ̑Ή��B
	
	Revision 1.2.2.1  2006/02/06 11:34:35  yosiokat
	GetIPL2CRC16��USG-IPL2-WW�łł̃R�[�h��NVRAM�i�[�t�H�[�}�b�g���ύX���ꂽ���Ƃ�
	�Ή�������B
	
	Revision 1.2  2005/04/02 07:42:19  yosiokat
	IPL2�t�H���g��"s"�ɕύX�B
	
	Revision 1.1  2005/04/02 06:41:18  yosiokat
	main.c����N�����̏������֐��𕪗��B
	

*/


#include <nitro.h>
#include "data.h"
#include "main.h"

// define data---------------------------------------------

// extern data---------------------------------------------

// function's prototype------------------------------------

// global variables----------------------------------------
u32 ipl2_crc32	= 0;

// static variables----------------------------------------


// const data----------------------------------------------


// ==================================================================
/* function */

// �\���n������
void InitDisp( void )
{
	// ���C��LCD
	{
		GX_SetBankForBG ( GX_VRAM_BG_128_A );						// �@�V�@�@�@�@�@BG �p
		GX_SetBankForOBJ( GX_VRAM_OBJ_32_FG );						// �@�V�@�@�@�@�@OBJ�p
		GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
	    G2_SetBG1Control( GX_BG_SCRSIZE_TEXT_256x256,				// 256pix x 256pix text
	                      GX_BG_COLORMODE_16,						// use 256 colors mode
	                      GX_BG_SCRBASE_0xf000,						// screen base offset + 0x0000 is the address for BG #0 screen
	                      GX_BG_CHARBASE_0x00000,					// character base offset + 0x04000 is the address for BG #0 characters
	                      GX_BG_EXTPLTT_01 							// use BGExtPltt slot #0 if BGExtPltt is enabled
	                      );
	    G2_SetBG1Priority( 3 );
	    G2_BG1Mosaic( FALSE );
		G2_SetBG2ControlText( GX_BG_SCRSIZE_TEXT_256x256,
   	                      GX_BG_COLORMODE_16,        
       	                  GX_BG_SCRBASE_0xf800,
           	              GX_BG_CHARBASE_0x10000
                         );
	    G2_SetBG2Priority( 2 );
	    G2_BG2Mosaic( FALSE );
		MI_CpuClearFast( (void *)HW_BG_VRAM,   0x20000 );			// BG -VRAM �N���A
		MI_CpuClearFast( (void *)HW_OBJ_VRAM,  0x8000 );			// OBJ-VRAM �N���A
		MI_CpuClearFast( (void *)HW_PLTT,      HW_PLTT_SIZE );		// �p���b�g �N���A		// NitroSDK�ł�crt0.c�ŃN���A���Ă���B
		MI_CpuFill32( (void *)HW_OAM, 192, HW_OAM_SIZE );			// OAM      �N���A		// NitroSDK�ł�crt0.c�ŃN���A���Ă���B
		MI_CpuCopyFast( myPlttData, (void *)( HW_BG_PLTT + 2 * 16 ), sizeof(myPlttData) );	//  BG�p���b�g   �Z�b�g
		MI_CpuCopyFast( myPlttData, (void *)( HW_BG_PLTT + 2 * 16 ), sizeof(myPlttData) );	// OBJ�p���b�g   �Z�b�g
	}
	// �T�uLCD
	{
		GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);
	    GX_SetBankForSubBG ( GX_VRAM_SUB_BG_128_C );               	// VRAM-C for BGs
	    GX_SetBankForSubOBJ( GX_VRAM_SUB_OBJ_128_D );              	// VRAM-D for BGs
	    GXS_SetGraphicsMode( GX_BGMODE_0 );                       	// BGMODE is 0
	    G2S_SetBG1Control( GX_BG_SCRSIZE_TEXT_256x256,				// 256pix x 256pix text
	                       GX_BG_COLORMODE_16,						// use 256 colors mode
	                       GX_BG_SCRBASE_0xf000,					// screen base offset + 0x0000 is the address for BG #0 screen
	                       GX_BG_CHARBASE_0x00000,					// character base offset + 0x04000 is the address for BG #0 characters
	                       GX_BG_EXTPLTT_01 						// use BGExtPltt slot #0 if BGExtPltt is enabled
	                       );
	    G2S_SetBG1Priority( 3 );
	    G2S_BG1Mosaic( FALSE );
	    G2S_SetBG2ControlText( GX_BG_SCRSIZE_TEXT_256x256,
   	                       GX_BG_COLORMODE_16,        
       	                   GX_BG_SCRBASE_0xf800,
           	               GX_BG_CHARBASE_0x10000
               	          );
	    G2S_SetBG2Priority( 2 );
	    G2S_BG2Mosaic( FALSE );
	    GXS_SetVisiblePlane( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2  );					// display only BG #0
		MI_CpuFill32( (void *)HW_DB_OAM, 192, HW_OAM_SIZE );		// OAM      �N���A
		MI_CpuClearFast( (void *)HW_DB_BG_VRAM,  0x20000 );			// BG -VRAM �N���A
		MI_CpuClearFast( (void *)HW_DB_OBJ_VRAM, 0x20000 );			// OBJ -VRAM �N���A
		MI_CpuCopyFast( myPlttData, (void *)( HW_DB_BG_PLTT + 2 * 16 ), sizeof(myPlttData) );	//  BG�p���b�g   �Z�b�g
		MI_CpuCopyFast( myPlttData, (void *)( HW_DB_BG_PLTT + 2 * 16 ), sizeof(myPlttData) );	// OBJ�p���b�g   �Z�b�g
	}
}


//---------------------------------------------------------------------------
//  IPL2�t�H���g������
//---------------------------------------------------------------------------
void InitIPL2Font( void )
{
	// �t�H���g�C���X�^���X�̍쐬�B
	// �`��p�̃L�����N�^�o�b�t�@���쐬�B
	fnt_CreateFont( &font_m, NITRO_LC_Font_s_bnfr, NITRO_LC_Font_s_bncmp );
	fnt_CreateDrawContext( &context_m, canvas_m, CANVAS_WIDTH, CANVAS_HEIGHT, FNT_DCTYPE_4BITS, &font_m );
	
	fnt_CreateFont( &font_s, NITRO_LC_Font_s_bnfr, NITRO_LC_Font_s_bncmp );
	fnt_CreateDrawContext( &context_s, canvas_s, CANVAS_WIDTH, CANVAS_HEIGHT, FNT_DCTYPE_4BITS, &font_s );
	
	InitIPL2FontBG();
}

//---------------------------------------------------------------------------
//  IPL2�t�H���g�\���p�X�N���[�����p���b�g������
//---------------------------------------------------------------------------
void InitIPL2FontBG( void )
{
	int x, y;
	
	GX_LoadBGPltt ( s_Palette, 0, sizeof(s_Palette) );
	GXS_LoadBGPltt( s_Palette, 0, sizeof(s_Palette) );
	for ( y = 0 ; y < (CANVAS_HEIGHT >> 3) ; ++y ) {
		for ( x = 0 ; x < (CANVAS_WIDTH >> 3) ; ++x ) {
			screen_m[ y * 32 + x] = (u16)(y * 32 + x);
			screen_s[ y * 32 + x] = (u16)(y * 32 + x);
		}
	}
	DC_FlushRange ( screen_m,    (CANVAS_WIDTH >> 3) * (CANVAS_HEIGHT >> 3) * 2 );
	GX_LoadBG2Scr ( screen_m, 0, (CANVAS_WIDTH >> 3) * (CANVAS_HEIGHT >> 3) * 2 );
	MI_CpuFillFast( canvas_m, 0, sizeof(canvas_m) );
	
	DC_FlushRange ( screen_s,    (CANVAS_WIDTH >> 3) * (CANVAS_HEIGHT >> 3) * 2 );
	GXS_LoadBG2Scr( screen_s, 0, (CANVAS_WIDTH >> 3) * (CANVAS_HEIGHT >> 3) * 2 );
	MI_CpuFillFast( canvas_s, 0, sizeof(canvas_s) );
}
