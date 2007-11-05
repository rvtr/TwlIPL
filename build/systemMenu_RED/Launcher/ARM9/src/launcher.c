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
static int s_csr = 0;													// メニューのカーソル位置
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

// バナー表示関係（暫定）
#define DBGBNR
#ifdef DBGBNR

static BannerFile *empty_banner;
static BannerFile *nobanner_banner;
static BannerFile *card_banner;
static BannerFile *download_banner;
static u8 image_index_list[TITLE_PROPERTY_NUM];
static const int MAX_SHOW_BANNER = 6;
static GXOamAttr banner_oam_attr[MAX_SHOW_BANNER];
static u8 *pbanner_image_list[TITLE_PROPERTY_NUM*2];
static int banner_count = 0;

static void LoadBannerFiles()
{
	// ファイル読み込み部分。多分emptyバナーだけ読み込む事になる。本来、アプリ系は外部から取得
	// 最後に解放しないと駄目。だが、どこで解放すればいいのやら……
	u32 size = CMN_LoadFile( (void **)&empty_banner, "data/EmptyBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&nobanner_banner, "data/NoBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&card_banner, "data/CardBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&download_banner, "data/DownloadBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
}

// パレットの読み込みやOBJ関係の初期化
static void BannerInit()
{
	int l;
	LoadBannerFiles();
	
	// ここでやるべきじゃない気がするBGとOBJの設定
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG0);      // display only OBJ&BG0
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);     // 2D mapping mode
    
    // パレット読み込み
	GX_LoadOBJPltt( empty_banner->v1.pltt, 0, BNR_PLTT_SIZE );
	
	//OBJATTRの初期化……後で値を弄って場所やらキャラクターを変えたりする
	for(l=0;l<MAX_SHOW_BANNER;l++)
	{
		G2_SetOBJAttr(  &banner_oam_attr[l],							// OAM pointer
						128,											// X position
						96,												// Y position
						0,												// Priority
						GX_OAM_MODE_NORMAL,								// Bitmap mode
						FALSE,											// mosaic off
						GX_OAM_EFFECT_NONE,								// affine off
						GX_OAM_SHAPE_32x32,								// 32x32 size
						GX_OAM_COLOR_16,								// 16 color
						0,												// charactor
						0,												// palette
						0);												// affine
	}
	G2_SetOBJEffect(&banner_oam_attr[2], GX_OAM_EFFECT_AFFINE_DOUBLE, 0);
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
}

// 活線挿抜対応のため、毎回VRAMへのイメージデータロード判定をしている
static void BannerDraw(int cursor, int selected, TitleProperty *titleprop)
{
	static int count = 0;
	int l;
	
    // TitleProperty弄り
	for(l=0;l<TITLE_PROPERTY_NUM;l++)
	{
		if(titleprop[l].titleID == 0)
		{
			titleprop[l].pBanner = empty_banner;
		}
		else if(titleprop[l].pBanner == NULL)
		{
			titleprop[l].pBanner = nobanner_banner;
		}
	}
	
    // TitlePropertyを見てVRAMにキャラクタデータをロード
	for(l=0;l<TITLE_PROPERTY_NUM;l++)
	{
		u8 m;
		u8 *pban=((BannerFile *)titleprop[l].pBanner)->v1.image;
		for(m=0;m<banner_count;m++){
			if(pban == pbanner_image_list[m]) break;
		}
		if(m == banner_count)
		{
			if(banner_count<TITLE_PROPERTY_NUM*2-1){//バナー画像バッファオーバー時の処理かなり適当……あとで要変更
				GX_LoadOBJ(pban, (u32)m*BNR_IMAGE_SIZE , BNR_IMAGE_SIZE);
				pbanner_image_list[m] = pban;
				banner_count++;
			}
		}
		image_index_list[l]=m;// 後の参照用
	}

	count++;
	
	// OAMデータを弄って位置など変更
	for (l=0;l<MAX_SHOW_BANNER;l++)
	{
		int num = cursor/12 - 2 + l;
		if(-1 < num && num < TITLE_PROPERTY_NUM){
			banner_oam_attr[l].charNo = image_index_list[num]*4;
			if(l == 2)
			{
				GXOamAffine *test = (GXOamAffine *)&banner_oam_attr[2];
				MtxFx22 mtx;
				mtx._00 = FX32_ONE;
				mtx._01 = 0;
				mtx._10 = 0;
				mtx._11 = FX32_ONE;
				G2_SetOBJPosition(&banner_oam_attr[l], 128-32-(cursor%12)*4, 96-16);
				G2_SetOBJAffine((GXOamAffine *)&banner_oam_attr[l], &mtx);
				test=test+1;
				test=test-1;
			}else
			{
				banner_oam_attr[l].x = 128-16-64-32+l*48-(cursor%12)*4;
				G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NONE,0);
			}
		}else
		{
			G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NODISPLAY,0);
		}
	}
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));
	
	// ゲーム名表示
	{
		NNSG2dChar *str = ((BannerFile *)titleprop[selected].pBanner)->v1.gameName[GetNCDWork()->option.language];
		int width = NNS_G2dTextCanvasGetStringWidth(&gTextCanvas, str, NULL);
		PutStringUTF16( (256-width)/2, 48, TXT_COLOR_BLACK, str );
	}
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
	
	//DrawMenu( s_csr, &s_launcherParam );
	
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
	static BOOL touch_bl = FALSE;
	BOOL tp_bl_on_off	 = FALSE;
	BOOL tp_select		 = FALSE;
	static int csr_v = 0;
	static int selected = 0;
	
	// 文字描画クリア
	NNS_G2dCharCanvasClear(&gCanvas,TXT_COLOR_WHITE);
	
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	DrawBackLightSwitch();
	
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
	if(pad.cont & PAD_KEY_RIGHT){										// バナー選択
		if(csr_v == 0) csr_v = 1;
	}
	if( pad.cont & PAD_KEY_LEFT ){
		if(csr_v == 0) csr_v = -1;
	}
	s_csr += csr_v;
	if((TITLE_PROPERTY_NUM-1)*12 < s_csr) s_csr = (TITLE_PROPERTY_NUM-1)*12;
	if( s_csr < 0 ) s_csr = 0;
	if(s_csr%12 == 0){
		csr_v = 0;
		selected = s_csr/12;
	}
	// tp_select = SelectMenuByTP( &s_csr, &s_launcherParam );
	
	// DrawMenu( s_csr, &s_launcherParam );
	
	#ifdef DBGBNR
	BannerDraw( s_csr, selected, pTitleList );
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

