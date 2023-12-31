/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     calibrationTP.c

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
#include "misc.h"
#include "MachineSetting.h"

// define data------------------------------------------
	// OKボタンLCD領域
#define OK_BUTTON_TOP_X						( 2 * 8 )
#define OK_BUTTON_TOP_Y						( 20 * 8 )
#define OK_BUTTON_BOTTOM_X					( OK_BUTTON_TOP_X + 8 * 8 )
#define OK_BUTTON_BOTTOM_Y					( OK_BUTTON_TOP_Y + 4 * 8 )

	// キャンセルボタンLCD領域
#define CANCEL_BUTTON_TOP_X					( 11 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 20 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( CANCEL_BUTTON_TOP_X + 8 * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( CANCEL_BUTTON_TOP_Y + 4 * 8 )

	// リトライボタンLCD領域
#define RETRY_BUTTON_TOP_X					( 20 * 8 )
#define RETRY_BUTTON_TOP_Y					( 20 * 8 )
#define RETRY_BUTTON_BOTTOM_X				( RETRY_BUTTON_TOP_X + 8 * 8 )
#define RETRY_BUTTON_BOTTOM_Y				( RETRY_BUTTON_TOP_Y + 4 * 8 )

	// キャリブレーション用OBJデータ
const u16 bitmapOBJPoint[8 * 8 * 5];

	// 各種キャラクタデータサイズ
#define IMAGE_DATA							(bitmapOBJPoint)
#define IMAGE_DATA_SIZE						(sizeof(bitmapOBJPoint))
#define MY_CHAR_SIZE						(sizeof(myChar))

	// NITRO-LCDサイズ
#define DISP_X_SIZE							256
#define DISP_Y_SIZE							192

enum {
	INIT=0,
	INTERVAL_0,
	CALIBRATE_1,
	INTERVAL_1,
	CALIBRATE_2,
	INTERVAL_2,
	CHECK_PARAM,
	INTERVAL_3,
	GET_POINT
};

	// キャリブレーション設定ワーク
typedef struct CalibWork {
	u32					seq;
	u16					release_count;
	u16					touch_count;
	u16					last_x;
	u16					last_y;
	TPData				sample[2];
	LCFGTWLTPCalibData	calibTemp;
}CalibWork;

// extern data------------------------------------------

// function's prototype declaration---------------------
static void DisplayInit();
static BOOL GetSamplePointNow(TPData *data);
static BOOL WaitPanelReleaseNow( void );
static void ReturnMenu(void);

// global variable -------------------------------------

// static variable -------------------------------------
static CalibWork *s_pTPC;
static GXOamAttr s_oamBak[ 128 ] ATTRIBUTE_ALIGN(32);					// OAM バックアップ

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
    G2_SetOBJAttr( &s_oamBak[0],  			 // OAM number
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
    G2_SetOBJAttr( &s_oamBak[0],   			 // OAM number
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
	MI_CpuClearFast( (void *)HW_OAM, 0x20 );
	MI_CpuClearFast( s_oamBak, sizeof(s_oamBak) );
	GX_SetOBJVRamModeBmp( GX_OBJVRAMMODE_BMP_1D_128K );       // 2D mapping OBJ
	/* Load charactor bitmap data */
	GX_LoadOBJ( (const void *)IMAGE_DATA, 0x3000 /* 0 */, IMAGE_DATA_SIZE ); // Transfer OBJ bitmap data to VRAM
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
        s_pTPC->touch_count = 0;
        return FALSE;
    }
    
    if ( temp.validity != TP_VALIDITY_VALID ) {
        s_pTPC->touch_count = 0;
        return FALSE;
    }
    
    OS_Printf("( %d, %d )\n", temp.x, temp.y);
    
    s_pTPC->touch_count++;
    if ( s_pTPC->touch_count == 1 ) {
        s_pTPC->last_x = temp.x;
        s_pTPC->last_y = temp.y;
        return FALSE;
    }
    
    // if jump point from last frame, reset count.
    if ( (s32)(s_pTPC->last_x - temp.x) < - OK_RANGE ||
         (s32)(s_pTPC->last_x - temp.x) > OK_RANGE )
    {
        s_pTPC->touch_count  = 1;
        s_pTPC->last_x = temp.x;
        s_pTPC->last_y = temp.y;
        return FALSE;
    }
    
    if ( (s32)(s_pTPC->last_y - temp.y) < - OK_RANGE ||
         (s32)(s_pTPC->last_y - temp.y) > OK_RANGE )
    {
        s_pTPC->touch_count  = 1;
        s_pTPC->last_x = temp.x;
        s_pTPC->last_y = temp.y;
        return FALSE;
    }
    
    // if the point pressed during OK_COUNT, detect finish.
    if ( s_pTPC->touch_count == OK_COUNT ) {
        data->x        = (u16) ( (temp.x + s_pTPC->last_x) / 2 );
        data->y        = (u16) ( (temp.y + s_pTPC->last_y) / 2 );
        data->touch    = TP_TOUCH_ON;
        data->validity = TP_VALIDITY_VALID;
		s_pTPC->touch_count=0;
        return TRUE;
    }
    
    s_pTPC->last_x = temp.x;
    s_pTPC->last_y = temp.y;
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
        s_pTPC->release_count = 0;
        return FALSE;
    }
    
    s_pTPC->release_count++;
    if ( s_pTPC->release_count >= INTERVAL_CNT ) {
        s_pTPC->release_count = 0;
        return TRUE;
    } else {
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         TP_Calibration

  Description:  Initialization and main loop

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
int TP_CalibrationMain( void )
{
	BOOL tp_ok     = FALSE;
	BOOL tp_cancel = FALSE;
	BOOL tp_retry  = FALSE;
	
	switch (s_pTPC->seq) {
		case INIT:
			NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL,
										0 * 8 , 20 * 8, 32 * 8, 4 * 8 );
			s_pTPC->seq = INTERVAL_0;
			PutStringUTF16( 2 * 8, 21 * 8, TXT_COLOR_CYAN, (const u16 *)L"[B]:CANCEL" );
			break;
		
		case INTERVAL_0:
			// wait release TouchPanel
			if ( WaitPanelReleaseNow() ) {
				s_pTPC->seq = CALIBRATE_1;
			}
		break;
		
		case CALIBRATE_1:
			// detect first point.
			SetPoint8x8( 32, 32 );
			if ( GetSamplePointNow( &s_pTPC->sample[ 0 ] ) ) {
				OS_Printf( "OK! ( %d, %d )\n", s_pTPC->sample[0].x, s_pTPC->sample[0].y );
				s_pTPC->seq = INTERVAL_1;
			}
		break;
		
		case INTERVAL_1:
			// wait release TouchPanel
			if ( WaitPanelReleaseNow() ) {
				s_pTPC->seq = CALIBRATE_2;
			}
		break;
		
		case CALIBRATE_2:
			// detect second point.
			SetPoint8x8( DISP_X_SIZE - 32, DISP_Y_SIZE - 32 );
			if ( GetSamplePointNow( &s_pTPC->sample[ 1 ] ) ) {
				TPCalibrateParam calibrate;
				
				OS_Printf( "OK! ( %d, %d )\n", s_pTPC->sample[1].x, s_pTPC->sample[1].y );
				// Calculate and set calibration parameter from two detected point.
				(void)TP_CalcCalibrateParam(
					&calibrate,
					s_pTPC->sample[0].x, s_pTPC->sample[0].y, 32, 32,
					s_pTPC->sample[1].x, s_pTPC->sample[1].y, DISP_X_SIZE - 32, DISP_Y_SIZE - 32 );
				TP_SetCalibrateParam( &calibrate );
				
				OS_Printf( "Calibrate param: \n" );
				OS_Printf( "\tx = %d, xDotSize = %d\n", calibrate.x0, calibrate.xDotSize / 0x100 );
				OS_Printf( "\ty = %d, yDotSize = %d\n", calibrate.y0, calibrate.yDotSize / 0x100 );
				OS_Printf( "Check calibrate param\n" );
				
				s_pTPC->seq = INTERVAL_2;
			}
		break;
		
		case INTERVAL_2:
			// Wait release TouchPanel
			if ( WaitPanelReleaseNow() ) {
				s_pTPC->seq = CHECK_PARAM;
			}
		break;
		
		case CHECK_PARAM:
			// Verify Calibrattion Parameter.
			SetPoint8x8( DISP_X_SIZE / 2, DISP_Y_SIZE / 2 );
			if ( GetSamplePointNow( &tpd.raw ) ) {
				TP_GetUnCalibratedPoint( &tpd.disp.x, &tpd.disp.y, DISP_X_SIZE / 2, DISP_Y_SIZE / 2 );
				s_pTPC->seq = GET_POINT;
				
				NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL,
											2 * 8 , 21 * 8, 10 * 8, 2 * 8 );
				PutStringUTF16( OK_BUTTON_TOP_X,             OK_BUTTON_TOP_Y,             TXT_COLOR_CYAN, (const u16 *)L"   OK   ");
				PutStringUTF16( CANCEL_BUTTON_TOP_X,         CANCEL_BUTTON_TOP_Y,         TXT_COLOR_CYAN, (const u16 *)L" CANCEL ");
				PutStringUTF16( RETRY_BUTTON_TOP_X,          RETRY_BUTTON_TOP_Y,          TXT_COLOR_CYAN, (const u16 *)L" RETRY  ");
				PutStringUTF16( OK_BUTTON_TOP_X + 2 * 8,     OK_BUTTON_TOP_Y + 2 * 8,     TXT_COLOR_CYAN, (const u16 *)L"[A]");
				PutStringUTF16( CANCEL_BUTTON_TOP_X + 2 * 8, CANCEL_BUTTON_TOP_Y + 2 * 8, TXT_COLOR_CYAN, (const u16 *)L"[B]");
				PutStringUTF16( RETRY_BUTTON_TOP_X,          RETRY_BUTTON_TOP_Y + 2 * 8,  TXT_COLOR_CYAN, (const u16 *)L"[START]");
				{
					s32 xRange, yRange;
					xRange = tpd.raw.x - tpd.disp.x;
					yRange = tpd.raw.y - tpd.disp.y;
					OS_TPrintf( "OK! ( %d, %d )\n", tpd.raw.x, tpd.raw.y );
					OS_TPrintf( "Raw ( %d, %d )\n", tpd.disp.x, tpd.disp.y );
					OS_TPrintf( "\txRange = %d, yRange = %d\n", xRange, yRange );
				}
			}
		break;
		
		case GET_POINT:
			// Draw Marker by calibrated point.
			while ( TP_RequestRawSampling( &tpd.raw ) ) {
				SVC_WaitByLoop( 0x400 );
			}
			TP_GetCalibratedPoint( &tpd.disp, &tpd.raw );
			
			if ( tpd.raw.touch ) {
				SetPoint16x16( tpd.disp.x, tpd.disp.y );
				
				// [OK] [CANCEL] [RETRY]ボタン押下チェック
				tp_ok     = WithinRangeTP(	OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y,
						  				OK_BUTTON_BOTTOM_X, OK_BUTTON_BOTTOM_Y, &tpd.disp );
				tp_cancel = WithinRangeTP(	CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
						  				CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
				tp_retry  = WithinRangeTP(	RETRY_BUTTON_TOP_X, RETRY_BUTTON_TOP_Y,
						  				RETRY_BUTTON_BOTTOM_X, RETRY_BUTTON_BOTTOM_Y, &tpd.disp );
				
				if( tpd.raw.validity==TP_VALIDITY_VALID ) {
					OS_TPrintf( "( %3d, %3d ) -> ( %4d, %4d )\n", tpd.disp.x, tpd.disp.y, tpd.raw.x, tpd.raw.y );
				}
			}
			
			if( ( pad.trg & PAD_BUTTON_A ) || tp_ok ) {
				s_pTPC->calibTemp.data.raw_x1	= s_pTPC->sample[0].x;
				s_pTPC->calibTemp.data.raw_y1	= s_pTPC->sample[0].y;
				s_pTPC->calibTemp.data.dx1		= 32;
				s_pTPC->calibTemp.data.dy1		= 32;
				s_pTPC->calibTemp.data.raw_x2	= s_pTPC->sample[1].x;
				s_pTPC->calibTemp.data.raw_y2	= s_pTPC->sample[1].y;
				s_pTPC->calibTemp.data.dx2		= DISP_X_SIZE - 32;
				s_pTPC->calibTemp.data.dy2		= DISP_Y_SIZE - 32;
				
				LCFG_TSD_SetTPCalibration( &s_pTPC->calibTemp );
				// ::::::::::::::::::::::::::::::::::::::::::::::
				// TWL設定データファイルへの書き込み
				// ::::::::::::::::::::::::::::::::::::::::::::::
				if( !MY_WriteTWLSettings() ) {
					OS_TPrintf( "TWL settings write failed.\n" );
				}
				
				ReturnMenu();
				return 0;
			}else if( ( pad.trg & PAD_BUTTON_START ) || tp_retry ) {
				s_pTPC->seq = INIT;
			}
		break;
	}
	
	/* flush cache of OAM buffers to main memory */
	DC_FlushRange( s_oamBak, sizeof(s_oamBak) );
	GX_LoadOAM( s_oamBak, 0, sizeof(s_oamBak) );
	MI_DmaFill32( 3, s_oamBak, 192, sizeof(s_oamBak) );   				// Clear OAM buffer
	
	// Bボタンキャンセル
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ){
		// キャリブレーションを設定前の状態に戻す。
		{
			TPCalibrateParam calibrate;
			LCFG_TSD_GetTPCalibration( &s_pTPC->calibTemp );
			(void)TP_CalcCalibrateParam(
					&calibrate,
					s_pTPC->calibTemp.data.raw_x1, s_pTPC->calibTemp.data.raw_y1, (u16)s_pTPC->calibTemp.data.dx1, (u16)s_pTPC->calibTemp.data.dy1,
					s_pTPC->calibTemp.data.raw_x2, s_pTPC->calibTemp.data.raw_y2, (u16)s_pTPC->calibTemp.data.dx2, (u16)s_pTPC->calibTemp.data.dy2 );
			TP_SetCalibrateParam( &calibrate );
		}
		ReturnMenu();
		return 0;
	}
	
	return 0;
}


// メニューに戻る
static void ReturnMenu( void )
{
	// キャリブレーション用変数の開放
	Free( s_pTPC );
	s_pTPC = NULL;
	OS_Printf("Free :CalibWork\n");
	MachineSettingInit();
}


// キャリブレーション設定の初期化
void TP_CalibrationInit( void )
{
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"TOUCH PANEL CALIBRATION" );
	
	DisplayInit();
	
	s_pTPC = Alloc( sizeof(CalibWork) );				// キャリブレーション用変数の確保
	if( s_pTPC == NULL ) {
		OS_Panic("ARM9- Fail to allocate memory...\n");
	}
	SVC_CpuClear( 0x0000, s_pTPC, sizeof(CalibWork), 16 );
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_OBJ );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	s_pTPC->seq = INIT;
}


// タッチパネル設定ポイント　キャラデータ
const u16 bitmapOBJPoint[8 * 8 * 5] = {
     0x0000, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000,  //   0 char
     0xFC00, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0xFC00,  //   0 char
     0xFC00, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0xFC00,  //   0 char
     0xFC00, 0xFC00, 0xFC00, 0x801F, 0x801F, 0xFC00, 0xFC00, 0xFC00,  //   0 char
     0xFC00, 0xFC00, 0xFC00, 0x801F, 0x801F, 0xFC00, 0xFC00, 0xFC00,  //   0 char
     0xFC00, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0xFC00,  //   0 char
     0xFC00, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0xFC00,  //   0 char
     0x0000, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000,  //   0 char

     0x0000, 0x0000, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000, 0x0000,  //   1 char
     0x0000, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000,  //   1 char
     0xFC00, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0xFC00,  //   1 char
     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   1 char

     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   2 char
     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   2 char
     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   2 char
     0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x801F, 0x801F, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00,  //   2 char

     0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x801F, 0x801F, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00,  //   3 char
     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   3 char
     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   3 char
     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   3 char

     0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00,  //   4 char
     0xFC00, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0xFC00,  //   4 char
     0x0000, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000,  //   4 char
     0x0000, 0x0000, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000, 0x0000,  //   4 char
};

