/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     launcher.c

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
#include "launcher.h"


// define data------------------------------------------
#define LAUNCHER_ELEMENT_NUM			4							// ロゴメニューの項目数

#define B_LIGHT_BUTTON_TOP_X				24
#define B_LIGHT_BUTTON_TOP_Y				21
#define B_LIGHT_BUTTON_BOTTOM_X				( B_LIGHT_BUTTON_TOP_X + 7 )
#define B_LIGHT_BUTTON_BOTTOM_Y				( B_LIGHT_BUTTON_TOP_Y + 2 )


// extern data------------------------------------------

// function's prototype declaration---------------------
static void DrawBackLightSwitch(void);
static void DrawLauncher(u16 nowCsr, const MenuParam *pMenu);

// global variable -------------------------------------

// static variable -------------------------------------
static u16 s_csr = 0;													// メニューのカーソル位置
static const u16 *s_pStrLauncher[ LAUNCHER_ELEMENT_NUM ];				// ロゴメニュー用文字テーブルへのポインタリスト

// const data  -----------------------------------------
//===============================================
// Launcher.c
//===============================================
static const u16 *const s_pStrLauncherElemTbl[ LAUNCHER_ELEMENT_NUM ][ LANG_CODE_MAX ] = {
	{
		(const u16 *)L"DSカード",
		(const u16 *)L"DS Card",
		(const u16 *)L"DS Card(F)",
		(const u16 *)L"DS Card(G)",
		(const u16 *)L"DS Card(I)",
		(const u16 *)L"DS Card(S)",
	},
	{
		(const u16 *)L"ピクトチャット",
		(const u16 *)L"PictoChat",
		(const u16 *)L"PictoChat(F)",
		(const u16 *)L"PictoChat(G)",
		(const u16 *)L"PictoChat(I)",
		(const u16 *)L"PictoChat(S)",
	},
	{
		(const u16 *)L"DSダウンロードプレイ",
		(const u16 *)L"DS Download Play",
		(const u16 *)L"DS Download Play(F)",
		(const u16 *)L"DS Download Play(G)",
		(const u16 *)L"DS Download Play(I)",
		(const u16 *)L"DS Download Play(S)",
	},
	{
		(const u16 *)L"本体設定",
		(const u16 *)L"Machine Settings",
		(const u16 *)L"Machine Settings(F)",
		(const u16 *)L"Machine Settings(G)",
		(const u16 *)L"Machine Settings(I)",
		(const u16 *)L"Machine Settings(S)",
	},
};

static MenuPos s_launcherPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
};

static const MenuParam s_launcherParam = {
	LAUNCHER_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_launcherPos[0],
	(const u16 **)&s_pStrLauncher,
};

static const u16 *const str_backlight[] = {
	(const u16 *)L"BLT:ON ",
	(const u16 *)L"BLT:OFF",
};

//======================================================
// ランチャー
//======================================================

// きめうちバナー
#define DBGBNR
#ifdef DBGBNR

#define MAX_TITLE_PROPERTY 40
static BannerFile *banner;
static BannerFile *banner2;
static TitleProperty tp[MAX_TITLE_PROPERTY];
static GXOamAttr banner_oam_attr;
static GXOamAttr banner_oam_attr2;

// バナー画像のロード及びOBJ関係初期化
// 実際には、受け取ったリストからイメージデータのアドレスを取得してVRAMにイメージデータをロードし、
// 表示するぶんだけ毎フレームGXOamAttrにしてやる必要があるっぽい
// VRAMに格納したデータへは、リストのインデックスからアクセス？
static void BannerInit()
{
	int l;
	u32 size = CMN_LoadFile( (void **)&banner, "data/myGameBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&banner2, "data/myGameBanner2.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG0);      // display only OBJ&BG0
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);     // 2D mapping mode
    
	GX_LoadOBJPltt( banner->v1.pltt, 0, BNR_PLTT_SIZE );
	GX_LoadOBJ(banner->v1.image, 0x20, BNR_IMAGE_SIZE);
	G2_SetOBJAttr(  &banner_oam_attr,								// OAM pointer
					128,												// X position
					128,												// Y position
					0,												// Priority
					GX_OAM_MODE_NORMAL,								// Bitmap mode
					FALSE,											// mosaic off
					GX_OAM_EFFECT_NONE,								// affine off
					GX_OAM_SHAPE_32x32,								// 32x32 size
					GX_OAM_COLOR_16,								// 16 color
					1,												// charactor
					0,												// palette
					0);												// affine
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	
	GX_LoadOBJ(banner2->v1.image, 0x20+BNR_IMAGE_SIZE, BNR_IMAGE_SIZE);
	G2_SetOBJAttr(  &banner_oam_attr2,								// OAM pointer
					128,												// X position
					128,												// Y position
					0,												// Priority
					GX_OAM_MODE_NORMAL,								// Bitmap mode
					FALSE,											// mosaic off
					GX_OAM_EFFECT_NONE,								// affine off
					GX_OAM_SHAPE_32x32,								// 32x32 size
					GX_OAM_COLOR_16,								// 16 color
					17,												// charactor
					0,												// palette
					0);												// affine
	DC_FlushRange(&banner_oam_attr2, sizeof(banner_oam_attr));
	
	for(l=0;l<MAX_TITLE_PROPERTY;l++)
	{
		tp[l].titleID = 0;
		tp[l].pBanner = banner;
	}
}

static void BannerDraw()
{
	banner_oam_attr.x++;
	banner_oam_attr.y++;
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr2,BNR_IMAGE_SIZE, sizeof(banner_oam_attr));
}

#endif //DBGBNR


// ランチャーの初期化
void LauncherInit( TitleProperty *pTitleList )
{
#pragma unused( pTitleList )
	
	int i;
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	DrawBackLightSwitch();
	
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
	for( i = 0; i < LAUNCHER_ELEMENT_NUM; i++ ) {
		s_pStrLauncher[ i ] = s_pStrLauncherElemTbl[ i ][ GetNCDWork()->option.language ];
	}
	
	if( !SYSM_IsNITROCard() ) {
		s_launcherPos[ 0 ].enable = FALSE;		// DSカードが無い時は、先頭要素を無効にする。
	}
	
	DrawMenu( s_csr, &s_launcherParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	InitGetAndDrawRtcData( RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y );
	
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	#ifdef DBGBNR
	BannerInit();
	#endif
}


// ランチャーメイン
TitleProperty *LauncherMain( TitleProperty *pTitleList )
{
#pragma unused( pTitleList )
	
	static BOOL touch_bl = FALSE;
	BOOL tp_bl_on_off	 = FALSE;
	BOOL tp_select		 = FALSE;
	u16	 csr_old;
	
	// RTC情報の取得＆表示
	GetAndDrawRtcData();
	
	//--------------------------------------
	//  バックライトON,OFF制御
	//--------------------------------------
	if(tpd.disp.touch) {
		BOOL range = InRangeTp( B_LIGHT_BUTTON_TOP_X*8,    B_LIGHT_BUTTON_TOP_Y*8-4,
							    B_LIGHT_BUTTON_BOTTOM_X*8, B_LIGHT_BUTTON_BOTTOM_Y*8-4, &tpd.disp );
		if( range && !touch_bl ) {
			touch_bl	 = TRUE;
			tp_bl_on_off = TRUE;
		}
	}else {
		touch_bl = FALSE;
	}
	
	if( (pad.trg & PAD_BUTTON_R) || (tp_bl_on_off) ) {
		GetNCDWork()->option.backLightOffFlag ^= 0x01;
		DrawBackLightSwitch();
	}
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if(pad.trg & PAD_KEY_DOWN){										// カーソルの移動
		if( ++s_csr == LAUNCHER_ELEMENT_NUM ) {
			s_csr = 0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr = LAUNCHER_ELEMENT_NUM - 1;
		}
	}
	csr_old = s_csr;
	tp_select = SelectMenuByTP( &s_csr, &s_launcherParam );
	
	DrawMenu( s_csr, &s_launcherParam );
	
	#ifdef DBGBNR
	BannerDraw();
	#endif
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {					// メニュー項目への分岐
		if( s_launcherPos[ 0 ].enable ) {
			NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
			return NULL;
		}
	}
	
	return NULL;
}

#if 0
// ランチャー描画
static void DrawLauncher(u16 nowCsr, const MenuParam *pMenu)
{
	int i;
	int color;
	
	for( i = 0; i < pMenu->num; i++ ) {
		if(i == nowCsr)	{
			if( !pMenu->pos[ i ].enable ) {
				color = pMenu->disable_color;
			}else {
				color = pMenu->select_color;
			}
		}else {
			color = pMenu->normal_color;
		}
		PutStringUTF16( pMenu->pos[ i ].x, pMenu->pos[ i ].y, color, (pMenu->str_elem)[ i ] );
	}
}
#endif

// バックライトスイッチの表示
static void DrawBackLightSwitch(void)
{
	u16		color;
	
	if( GetNCDWork()->option.backLightOffFlag ) {
		color = TXT_COLOR_BLACK;
	}else {
		color = TXT_COLOR_RED;
	}
	
	PutStringUTF16( B_LIGHT_BUTTON_TOP_X, B_LIGHT_BUTTON_TOP_Y, color,
					str_backlight[ GetNCDWork()->option.backLightOffFlag ] );
}

