/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     tpCalib.c

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
#include "misc.h"
#include "DS_Setting.h"

// define data------------------------------------------
	// OKボタンLCD領域
#define OK_BUTTON_LT_X						2
#define OK_BUTTON_LT_Y						20
#define OK_BUTTON_RB_X						(OK_BUTTON_LT_X + 8)
#define OK_BUTTON_RB_Y						(OK_BUTTON_LT_Y + 2)

	// キャンセルボタンLCD領域
#define CANCEL_BUTTON_LT_X					11
#define CANCEL_BUTTON_LT_Y					20
#define CANCEL_BUTTON_RB_X					(CANCEL_BUTTON_LT_X+8)
#define CANCEL_BUTTON_RB_Y					(CANCEL_BUTTON_LT_Y+2)

	// リトライボタンLCD領域
#define RETRY_BUTTON_LT_X					20
#define RETRY_BUTTON_LT_Y					20
#define RETRY_BUTTON_RB_X					(RETRY_BUTTON_LT_X+8)
#define RETRY_BUTTON_RB_Y					(RETRY_BUTTON_LT_Y+2)

	// キャリブレーション用OBJデータ
const u16 bitmapOBJPoint[8 * 8 * 5];

	// 各種キャラクタデータサイズ
#define IMAGE_DATA							(bitmapOBJPoint)
#define IMAGE_DATA_SIZE						(sizeof(bitmapOBJPoint))
#define MY_CHAR_SIZE						(sizeof(myChar))

	// NITRO-LCDサイズ
#define DISP_X_SIZE							256
#define DISP_Y_SIZE							192

	// キャリブレーションシーケンス番号（CalibWork.seqの値）
enum {
	SEQ_INIT=0,      SEQ_INTERVAL_0, 
	SEQ_CALIBRATE_1, SEQ_INTERVAL_1, 
	SEQ_CALIBRATE_2, SEQ_INTERVAL_2, 
	SEQ_CHECK_PARAM, SEQ_INTERVAL_3, 
	SEQ_GET_POINT
};

	// キャリブレーション設定ワーク
typedef struct CalibWork {
	u32					seq;
	u16					release_count;
	u16					touch_count;
	u16					last_x;
	u16					last_y;
	TPData				sample[2];
	TPCalibrateParam 	calibrate;
}CalibWork;

// extern data------------------------------------------

// function's prototype declaration---------------------
void SEQ_TP_Calibration_init(void);
int  SEQ_TP_Calibration(void);

static void DisplayInit();
static BOOL GetSamplePointNow(TPData *data);
static BOOL WaitPanelReleaseNow( void );
static void ReturnMenu(void);

// global variable -------------------------------------

// static variable -------------------------------------
static CalibWork *cw;

// const data  -----------------------------------------

//======================================================
// function's description
//======================================================

/*---------------------------------------------------------------------------*
  Name:         SetPoint8x8

  Description:  Display a 8x8 OBJ on indicated point.

  Arguments:    x - position X.
                y - position Y.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void SetPoint8x8(u16 pos_x, u16 pos_y)
{
    G2_SetOBJAttr( &oamBakS[0],  			 // OAM number
                   pos_x - 4,                // X position
                   pos_y - 4,                // Y position
                   0,                        // Priority
                   GX_OAM_MODE_BITMAPOBJ,    // Bitmap mode
                   FALSE,                    // mosaic off
                   GX_OAM_EFFECT_NONE,       // affine off
                   GX_OAM_SHAPE_8x8,         // 8x8 size
                   GX_OAM_COLOR_16,          // 16 color
				   0x60,                     // charactor
                   15,                       // alpha
                   0);
}


/*---------------------------------------------------------------------------*
  Name:         SetPoint16x16

  Description:  Display a 16x16 OBJ on indicated point.

  Arguments:    x - position X.
                y - position Y.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void SetPoint16x16(u16 pos_x, u16 pos_y)
{
    G2_SetOBJAttr( &oamBakS[0],   			 // OAM number
                   pos_x - 8,                // X position
                   pos_y - 8,                // Y position
                   0,                        // Priority
                   GX_OAM_MODE_BITMAPOBJ,    // Bitmap mode
                   FALSE,                    // mosaic off
                   GX_OAM_EFFECT_NONE,       // affine off
                   GX_OAM_SHAPE_16x16,       // 16x16 size
                   GX_OAM_COLOR_16,          // 16 color
                   0x61,                     // charactor
                   15,                       // alpha
                   0);
}


/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics Initialization

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisplayInit() 
{
	GXS_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);       // 2D mapping OBJ
	/* Load charactor bitmap data */
	GXS_LoadOBJ( (const void *)IMAGE_DATA, 0x3000 /* 0 */, IMAGE_DATA_SIZE ); // Transfer OBJ bitmap data to VRAM
}


/*---------------------------------------------------------------------------*
  Name:         GetSamplePointNow

  Description:  Get touched point by OneTime Sampling.
                This function use TP_RequestSampling() and TP_WaitRawResult()

  Arguments:    None.

  Returns:      data - getton TouchPanel data.
                BOOL - if touched and got point this function returns TRUE.
                       else  FALSE.
 *---------------------------------------------------------------------------*/
static BOOL GetSamplePointNow(TPData *data)
{
    TPData temp;
    
    enum {
        OK_COUNT = 4, OK_RANGE = 50
    };
    
    // Detect a point pushed during definite time.
    while (TP_RequestRawSampling( &temp )) { };
    
    if (! temp.touch ) {
        cw->touch_count = 0;
        return FALSE;
    }
    
    if ( temp.validity != TP_VALIDITY_VALID ) {
        cw->touch_count = 0;
        return FALSE;
    }
    
    OS_Printf("( %d, %d )\n", temp.x, temp.y);
    
    cw->touch_count++;
    if ( cw->touch_count == 1 ) {
        cw->last_x = temp.x;
        cw->last_y = temp.y;
        return FALSE;
    }
    
    // if jump point from last frame, reset count.
    if ( (s32)(cw->last_x - temp.x) < - OK_RANGE ||
         (s32)(cw->last_x - temp.x) > OK_RANGE )
    {
        cw->touch_count  = 1;
        cw->last_x = temp.x;
        cw->last_y = temp.y;
        return FALSE;
    }
    
    if ( (s32)(cw->last_y - temp.y) < - OK_RANGE ||
         (s32)(cw->last_y - temp.y) > OK_RANGE )
    {
        cw->touch_count  = 1;
        cw->last_x = temp.x;
        cw->last_y = temp.y;
        return FALSE;
    }
    
    // if the point pressed during OK_COUNT, detect finish.
    if ( cw->touch_count == OK_COUNT ) {
        data->x        = (u16) ( (temp.x + cw->last_x) / 2 );
        data->y        = (u16) ( (temp.y + cw->last_y) / 2 );
        data->touch    = TP_TOUCH_ON;
        data->validity = TP_VALIDITY_VALID;
		cw->touch_count=0;
        return TRUE;
    }
    
    cw->last_x = temp.x;
    cw->last_y = temp.y;
    return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         WaitPanelReleaseNow

  Description:  Wait to released TouchPanel, using OneTime Sampling.
                This function is using TP_RequestSampling() and TP_WaitRawResult().
  
  Arguments:    None.

  Returns:      BOOL  - if TouchPanel is released , this returns TRUE.
                else FALSE.
  
 *---------------------------------------------------------------------------*/
static BOOL WaitPanelReleaseNow( void )
{
    TPData temp;
    
    enum {
        INTERVAL_CNT = 10
    };
    
    while (TP_RequestRawSampling( &temp )) {
		SVC_WaitByLoop(0x400);
	};
    
    if ( temp.touch ) {
        cw->release_count = 0;
        return FALSE;
    }
    
    cw->release_count++;
    if ( cw->release_count >= INTERVAL_CNT ) {
        cw->release_count = 0;
        return TRUE;
    } else {
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         SEQ_TP_Calibration

  Description:  Initialization and main loop

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
int SEQ_TP_Calibration(void)
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	BOOL tp_retry  = FALSE;
	
	switch (cw->seq) {
		case SEQ_INIT:
			mf_clearRect(0, 20, 4, 32);
			cw->seq=SEQ_INTERVAL_0;
			(void)DrawStringSJIS( 2, 21, CYAN,(const u8 *)"[B]:CANCEL");
			break;
		
		case SEQ_INTERVAL_0:
			// wait release TouchPanel
			if ( WaitPanelReleaseNow() ) {
				cw->seq=SEQ_CALIBRATE_1;
			}
		break;
		
		case SEQ_CALIBRATE_1:
			// detect first point.
			SetPoint8x8(32, 32);
			if ( GetSamplePointNow(&cw->sample[0]) ) {
				OS_Printf("OK! ( %d, %d )\n", cw->sample[0].x, cw->sample[0].y);
				cw->seq = SEQ_INTERVAL_1;
			}
		break;
		
		case SEQ_INTERVAL_1:
			// wait release TouchPanel
			if ( WaitPanelReleaseNow() ) {
				cw->seq = SEQ_CALIBRATE_2;
			}
		break;
		
		case SEQ_CALIBRATE_2:
			// detect second point.
			SetPoint8x8( DISP_X_SIZE - 32, DISP_Y_SIZE - 32 );
			if ( GetSamplePointNow(&cw->sample[1]) ) {
				OS_Printf("OK! ( %d, %d )\n", cw->sample[1].x, cw->sample[1].y);
				// Calculate and set calibration parameter from two detected point.
				(void)TP_CalcCalibrateParam( &cw->calibrate,
								        cw->sample[0].x, cw->sample[0].y, 32, 32,
								        cw->sample[1].x, cw->sample[1].y, DISP_X_SIZE - 32, DISP_Y_SIZE - 32 );
				TP_SetCalibrateParam( &cw->calibrate );
				
				OS_Printf("Calibrate param: \n");
				OS_Printf("\tx = %d, xDotSize = %d\n", cw->calibrate.x0, cw->calibrate.xDotSize / 0x100);
				OS_Printf("\ty = %d, yDotSize = %d\n", cw->calibrate.y0, cw->calibrate.yDotSize / 0x100);
				OS_Printf("Check calibrate param\n");
				
				cw->seq = SEQ_INTERVAL_2;
			}
		break;
		
		case SEQ_INTERVAL_2:
			// Wait release TouchPanel
			if ( WaitPanelReleaseNow() ) {
				cw->seq = SEQ_CHECK_PARAM;
			}
		break;
		
		case SEQ_CHECK_PARAM:
			// Verify Calibrattion Parameter.
			SetPoint8x8( DISP_X_SIZE / 2, DISP_Y_SIZE / 2 );
			if ( GetSamplePointNow(&tpd.raw) ) {
				TP_GetUnCalibratedPoint( &tpd.disp.x, &tpd.disp.y, DISP_X_SIZE / 2, DISP_Y_SIZE / 2 );
				cw->seq = SEQ_GET_POINT;
				mf_clearRect(  2, 21, 2, 10);
				(void)DrawStringSJIS( OK_BUTTON_LT_X,     OK_BUTTON_LT_Y,     HIGHLIGHT_C, (const u8 *)"   OK   ");
				(void)DrawStringSJIS( CANCEL_BUTTON_LT_X, CANCEL_BUTTON_LT_Y, HIGHLIGHT_C, (const u8 *)" CANCEL ");
				(void)DrawStringSJIS( RETRY_BUTTON_LT_X,  RETRY_BUTTON_LT_Y,  HIGHLIGHT_C, (const u8 *)" RETRY  ");
				(void)DrawStringSJIS( (u16)(OK_BUTTON_LT_X + 2),     (u16)(OK_BUTTON_LT_Y + 2),     CYAN,(const u8 *)"[A]");
				(void)DrawStringSJIS( (u16)(CANCEL_BUTTON_LT_X + 2), (u16)(CANCEL_BUTTON_LT_Y + 2), CYAN,(const u8 *)"[B]");
				(void)DrawStringSJIS( (u16)(RETRY_BUTTON_LT_X),      (u16)(RETRY_BUTTON_LT_Y + 2),  CYAN,(const u8 *)"[START]");
				{
					s32 xRange, yRange;
					xRange = tpd.raw.x - tpd.disp.x;
					yRange = tpd.raw.y - tpd.disp.y;
					OS_Printf("OK! ( %d, %d )\n", tpd.raw.x, tpd.raw.y);
					OS_Printf("Raw ( %d, %d )\n", tpd.disp.x, tpd.disp.y);
					OS_Printf("\txRange = %d, yRange = %d\n", xRange, yRange);
				}
			}
		break;
		
		case SEQ_GET_POINT:
			// Draw Marker by calibrated point.
			while ( TP_RequestRawSampling( &tpd.raw ) ) {
				SVC_WaitByLoop(0x400);
			}
			TP_GetCalibratedPoint( &tpd.disp, &tpd.raw );
			
			if ( tpd.raw.touch ) {
				SetPoint16x16( tpd.disp.x, tpd.disp.y );
				
				// [OK] [CANCEL] [RETRY]ボタン押下チェック
				tp_ok     = InRangeTp(OK_BUTTON_LT_X*8, OK_BUTTON_LT_Y*8-4,
						  			OK_BUTTON_RB_X*8, OK_BUTTON_RB_Y*8-4, &tpd.disp);
				tp_cancel = InRangeTp(CANCEL_BUTTON_LT_X*8, CANCEL_BUTTON_LT_Y*8-4,
						  			CANCEL_BUTTON_RB_X*8, CANCEL_BUTTON_RB_Y*8-4, &tpd.disp);
				tp_retry  = InRangeTp(RETRY_BUTTON_LT_X*8, RETRY_BUTTON_LT_Y*8-4,
						  			RETRY_BUTTON_RB_X*8, RETRY_BUTTON_RB_Y*8-4, &tpd.disp);
				
				if(tpd.raw.validity==TP_VALIDITY_VALID) {
					OS_Printf("( %3d, %3d ) -> ( %4d, %4d )\n",     tpd.disp.x, tpd.disp.y, tpd.raw.x, tpd.raw.y);
				}
			}
			
			if((pad.trg & PAD_BUTTON_A) || (tp_ok)) {
				GetSYSMWork()->ncd_invalid		= 0;
				GetNCDWork()->option.input_tp	= 1;				// タッチパネル入力フラグを立てる。
				GetNCDWork()->tp.raw_x1			= cw->sample[0].x;
				GetNCDWork()->tp.raw_y1			= cw->sample[0].y;
				GetNCDWork()->tp.dx1			= 32;
				GetNCDWork()->tp.dy1			= 32;
				GetNCDWork()->tp.raw_x2			= cw->sample[1].x;
				GetNCDWork()->tp.raw_y2			= cw->sample[1].y;
				GetNCDWork()->tp.dx2			= DISP_X_SIZE - 32;
				GetNCDWork()->tp.dy2			= DISP_Y_SIZE - 32;
				// ::::::::::::::::::::::::::::::::::::::::::::::
				// NVRAMへの書き込み
				// ::::::::::::::::::::::::::::::::::::::::::::::
				(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
				
				ReturnMenu();
				return 0;
			}else if((pad.trg & PAD_BUTTON_START) || (tp_retry)) {
				cw->seq = SEQ_INIT;
			}
		break;
	}
	
	/* flush cache of OAM buffers to main memory */
	DC_FlushRange( oamBakS, sizeof(oamBakS) );
	
	GXS_LoadOAM( oamBakS, 0, sizeof(oamBakS) );
	MI_DmaFill32( 3, oamBakS, 192, sizeof(oamBakS) );   				// Clear OAM buffer
	
	// Bボタンキャンセル
	if((pad.trg & PAD_BUTTON_B) || (tp_cancel)){
		(void)TP_CalcCalibrateParam(&cw->calibrate,
				GetNCDWork()->tp.raw_x1, GetNCDWork()->tp.raw_y1, (u16)GetNCDWork()->tp.dx1, (u16)GetNCDWork()->tp.dy1,
				GetNCDWork()->tp.raw_x2, GetNCDWork()->tp.raw_y2, (u16)GetNCDWork()->tp.dx2, (u16)GetNCDWork()->tp.dy2);
		TP_SetCalibrateParam(&cw->calibrate);
		ReturnMenu();
		return 0;
	}
	
	return 0;
}


// メニューに戻る
static void ReturnMenu(void)
{
	OS_Free(cw);										// キャリブレーション用変数の開放
	cw = NULL;
	OS_Printf("Free :CalibWork\n");
	SEQ_MainMenu_init();
}


// キャリブレーション設定の初期化
void SEQ_TP_Calibration_init(void)
{
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	MI_CpuClearFast(bgBakS, sizeof(bgBakS));
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS( 1, 0, YELLOW, (const u8 *)"TOUCH PANEL CALIBRATION");
	if( initialSet ) {
		(void)DrawStringSJIS( 8, 18, RED, (const u8 *)"Calibrate touch panel.");
	}
	
	DisplayInit();
	
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
	
	cw=OS_Alloc(sizeof(CalibWork));									// キャリブレーション用変数の確保
#ifdef __IPL2_DEBUG
	if(cw==NULL) OS_Panic("ARM9- Fail to allocate memory...\n");
#endif /* __IPL2_DEBUG */
	OS_Printf("Alloc :CalibWork\n");
	SVC_CpuClear(0x0000, cw, sizeof(CalibWork), 16);
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	cw->seq = SEQ_INIT;
}


// タッチパネル設定ポイント　キャラデータ
const u16 bitmapOBJPoint[8 * 8 * 5] = {
     0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,  //   0 char
     0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF,  //   0 char
     0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF,  //   0 char
     0xFFFF, 0xFFFF, 0xFFFF, 0x801F, 0x801F, 0xFFFF, 0xFFFF, 0xFFFF,  //   0 char
     0xFFFF, 0xFFFF, 0xFFFF, 0x801F, 0x801F, 0xFFFF, 0xFFFF, 0xFFFF,  //   0 char
     0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF,  //   0 char
     0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF,  //   0 char
     0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,  //   0 char

     0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,  //   1 char
     0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,  //   1 char
     0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF,  //   1 char
     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   1 char

     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   2 char
     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   2 char
     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   2 char
     0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x801F, 0x801F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  //   2 char

     0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x801F, 0x801F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  //   3 char
     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   3 char
     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   3 char
     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   3 char

     0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,  //   4 char
     0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF,  //   4 char
     0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,  //   4 char
     0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,  //   4 char
};

