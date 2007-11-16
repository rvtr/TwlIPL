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
#include <math.h>


// define data------------------------------------------
#define LAUNCHER_ELEMENT_NUM				4							// ロゴメニューの項目数

#define B_LIGHT_BUTTON_TOP_X				( 0  )
#define B_LIGHT_BUTTON_TOP_Y				( 22 * 8 )
#define B_LIGHT_BUTTON_BOTTOM_X				( B_LIGHT_BUTTON_TOP_X + 32 )
#define B_LIGHT_BUTTON_BOTTOM_Y				( B_LIGHT_BUTTON_TOP_Y + 16 )

#define CURSOR_PER_SELECT	14

// extern data------------------------------------------

extern u32 bg_char_data[16 * 3];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_scr_data2[32 * 32];

// function's prototype declaration---------------------
static void DrawBackLightSwitch(void);
static void DrawLauncher(u16 nowCsr, const MenuParam *pMenu);

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------
static int s_csr = 0;													// メニューのカーソル位置

static u64	old_titleIdArray[ LAUNCHER_TITLE_LIST_NUM ];

// const data  -----------------------------------------
//===============================================
// Launcher.c
//===============================================

//======================================================
// ランチャー
//======================================================

// バナー表示関係（暫定）
#define DBGBNR
#ifdef DBGBNR

static NTRBannerFile *empty_banner;
static NTRBannerFile *nobanner_banner;
static NTRBannerFile *no_card_banner;
static u8 image_index_list[ LAUNCHER_TITLE_LIST_NUM ];
static const int MAX_SHOW_BANNER = 6;
static GXOamAttr banner_oam_attr[MAX_SHOW_BANNER+10];// アフィンパラメータ埋める関係で少し大きめ
static u8 *pbanner_image_list[ LAUNCHER_TITLE_LIST_NUM ];
static int banner_count = 0;

static void LoadNTRBannerFiles()
{
	// ファイル読み込み部分。多分emptyバナーだけ読み込む事になる。本来、アプリ系は外部から取得
	// 最後に解放しないと駄目。だが、どこで解放すればいいのやら……
	u32 size = CMN_LoadFile( (void **)&empty_banner, "data/EmptyBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&nobanner_banner, "data/NoBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&no_card_banner, "data/NoCardBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
}

// パレットの読み込みやOBJ関係の初期化
static void NTRBannerInit()
{
	int l;
	LoadNTRBannerFiles();
	
	MI_CpuClearFast(old_titleIdArray, sizeof(old_titleIdArray) );
    MI_DmaFill32(3, banner_oam_attr, 192, sizeof(banner_oam_attr));     // let out of the screen if not display
	
	// ここでやるべきじゃない気がするOBJの設定
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);     // 2D mapping mode
    
    // パレット読み込み
    // 本来は、読み込んだバナーによってパレットも変更する必要があるのかもしれないが、どの道最大256色
    // 17個以上のバナーをそれぞれ違うパレットで同時に表示はできないはず。
	GX_LoadOBJPltt( empty_banner->v1.pltt, 0, BNR_PLTT_SIZE );
	
	//OBJATTRの初期化……後で値を弄って場所やらキャラクターを変えたりする
	for(l=0;l<MAX_SHOW_BANNER;l++)
	{
		G2_SetOBJAttr(  &banner_oam_attr[l],							// OAM pointer
						128,											// X position
						96,												// Y position
						1,												// Priority
						GX_OAM_MODE_NORMAL,								// Bitmap mode
						FALSE,											// mosaic off
						GX_OAM_EFFECT_NONE,								// affine off
						GX_OAM_SHAPE_32x32,								// 32x32 size
						GX_OAM_COLOR_16,								// 16 color
						0,												// charactor
						0,												// palette
						0);												// affine
	}
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
}

// 活線挿抜対応のため、毎回VRAMへのイメージデータロード判定をしている
static void NTRBannerDraw(int cursor, int selected, TitleProperty *titleprop)
{
	static int count = 0;
	
	int l;
	MtxFx22 mtx;
	
	/*
	static int testcount=0;
	testcount++;
    if( (testcount/5)%2 ==1 ) titleprop[1].titleID = 0;//1secごとにTitlePropertyの2を変化させてみる活線挿抜擬似テスト
    else
    {
		titleprop[1].titleID = 1;
		titleprop[1].pBanner = download_banner;
	}
	*/
    
    // TitleProperty弄り
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		if(titleprop[l].titleID == 0) //IDがゼロの時はEmpty
		{
			if(l != 0)
			{
				titleprop[l].pBanner = empty_banner;
			}
			else
			{
				titleprop[l].pBanner = no_card_banner;
			}
		}
		else if(titleprop[l].pBanner == NULL) //IDがゼロじゃないのにバナーがNULLならノーバナー
		{
			titleprop[l].pBanner = nobanner_banner;
		}
	}
	
    // TitlePropertyを見てVRAMにキャラクタデータをロード
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		if(titleprop[l].titleID != old_titleIdArray[l])
		{
			// titleID変更されていたら、一からVRAMへのバナー画像ロードしなおし
			banner_count = 0;
			break;
		}
	}
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		u8 m;
		u8 *pban=((NTRBannerFile *)titleprop[l].pBanner)->v1.image;
		for(m=0;m<banner_count;m++){
			if(pban == pbanner_image_list[m]){
				image_index_list[l]=m;
				break;
			}
		}
		if(m == banner_count)
		{
			if(banner_count<LAUNCHER_TITLE_LIST_NUM-1){
				GX_LoadOBJ(pban, (u32)m*BNR_IMAGE_SIZE , BNR_IMAGE_SIZE);
				pbanner_image_list[m] = pban;
				banner_count++;
				image_index_list[l]=m;
			}
			else
			{	// バナー画像リストオーバー時は、titleIDが更新されずにバナーのみ入れ替わっている（アニメーション？）
				// 下の実装では少し不安。一から全部ロードしなおすほうが安心。
				GX_LoadOBJ(pban, (u32)image_index_list[l]*BNR_IMAGE_SIZE , BNR_IMAGE_SIZE);
				pbanner_image_list[image_index_list[l]] = pban;
			}
		}
		
		old_titleIdArray[l] = titleprop[l].titleID;// 後の参照用
	}

	count++;
	
	
	// アフィンパラメータだけ設定しておく
	{
		static double wav;
		if(cursor%CURSOR_PER_SELECT == 0){
			double s = sin(wav);
			mtx._00 = 0x880 - (long)(0x80*s);
			wav += 0.1;
		}else{
			mtx._00 = FX32_HALF + FX32_HALF*(cursor%CURSOR_PER_SELECT)/CURSOR_PER_SELECT;
			wav = 0;
		}
	}
	mtx._01 = 0;
	mtx._10 = 0;
	mtx._11 = mtx._00;
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
	mtx._00 = FX32_ONE - FX32_HALF*(cursor%CURSOR_PER_SELECT)/CURSOR_PER_SELECT;
	mtx._11 = mtx._00;
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[4]), &mtx);
	
	// OAMデータを弄って位置など変更
	for (l=0;l<MAX_SHOW_BANNER;l++)
	{
		int num = cursor/CURSOR_PER_SELECT - 2 + l;
		if(-1 < num && num < LAUNCHER_TITLE_LIST_NUM){
			banner_oam_attr[l].charNo = image_index_list[num]*4;
			
			if(l == 2 || l == 3)
			{
				G2_SetOBJEffect(&banner_oam_attr[l], GX_OAM_EFFECT_AFFINE_DOUBLE, l-2);
				G2_SetOBJPosition(&banner_oam_attr[l], 128-32-64-48+l*56-(cursor%CURSOR_PER_SELECT)*4, 96-16);
			}else
			{
				banner_oam_attr[l].x = 128-16-64-48+l*56-(cursor%CURSOR_PER_SELECT)*4;
				G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NONE,0);
			}
		}else
		{
			G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NODISPLAY,0);
		}
	}
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));
	
	// アプリ名表示
	{
		NNSG2dChar *str = ((NTRBannerFile *)titleprop[selected].pBanner)->v1.comment[ TSD_GetLanguage() ];
		int width = NNS_G2dTextCanvasGetStringWidth(&gTextCanvas, str, NULL);
		PutStringUTF16( (256-width)/2, 48, TXT_COLOR_BLACK, str );
	}
}

#endif //DBGBNR


// ランチャーの初期化
void LauncherInit( TitleProperty *pTitleList )
{
#pragma unused( pTitleList )
	
	GX_DispOff();
	GXS_DispOff();
	
	ChangeUserColor( TSD_GetUserColor() );
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    
    // BGデータのロード処理
	GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));
	
	// フェードアウト用BGデータ作成とロード
	SVC_CpuClear( 0x0004, &bg_scr_data2, sizeof(bg_scr_data2), 16 );
	DC_FlushRange(&bg_scr_data2, sizeof(bg_scr_data2));
	GX_LoadBG2Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG2Scr(bg_scr_data2, 0, sizeof(bg_scr_data2));
			
	DrawBackLightSwitch();
	
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.", SYSMENU_VER );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ );
	G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, 
			GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ, 0,31);

	GX_DispOn();
	GXS_DispOn();
	
	#ifdef DBGBNR
	NTRBannerInit();
	#endif
}


static int selected = 0;

// ROMのローディング中
void LauncherLoading( TitleProperty *pTitleList )
{
	static int fadecount = 0;
	
	// RTC情報の取得＆表示
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	// 描画関係
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	DrawBackLightSwitch();
	
	#ifdef DBGBNR
	NTRBannerDraw( s_csr, selected, pTitleList );
	#endif
	
	// これだと93フレームでフェードアウト終わる
	G2_ChangeBlendAlpha( fadecount/3, 31-(fadecount/3) );
	if(fadecount < 93) fadecount++;
}

// ランチャーメイン
TitleProperty *LauncherMain( TitleProperty *pTitleList )
{
	static BOOL touch_bl_bak = FALSE;
	BOOL touch_bl_trg = FALSE;
	static int csr_v = 0;
	BOOL tp_select = FALSE;
	TitleProperty *ret = NULL;
	
	// RTC情報の取得＆表示
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	//--------------------------------------
	//  バックライトON,OFF制御
	//--------------------------------------
	if(tpd.disp.touch) {
		BOOL touch_bl = WithinRangeTP(	B_LIGHT_BUTTON_TOP_X,    B_LIGHT_BUTTON_TOP_Y,
										B_LIGHT_BUTTON_BOTTOM_X, B_LIGHT_BUTTON_BOTTOM_Y, &tpd.disp );
		touch_bl_trg = ( touch_bl && tpd.disp.touch && !touch_bl_bak ) ;
		touch_bl_bak = tpd.disp.touch;
	}else {
		touch_bl_bak = FALSE;
	}
	
	if( (pad.trg & PAD_BUTTON_START) || touch_bl_trg ) {
		SYSM_SetBackLightBrightness( (u8)( ( TSD_GetBacklightBrightness() + 1 ) & 0x07 ) );
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
	if((LAUNCHER_TITLE_LIST_NUM-1)*CURSOR_PER_SELECT < s_csr) s_csr = (LAUNCHER_TITLE_LIST_NUM-1)*CURSOR_PER_SELECT;
	if( s_csr < 0 ) s_csr = 0;
	if(s_csr%CURSOR_PER_SELECT == 0){
		csr_v = 0;
		selected = s_csr/CURSOR_PER_SELECT;
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {					// メニュー項目への分岐
		if(pTitleList[selected].titleID != 0)
		{
			ret = &pTitleList[selected];
		}
	}
	
	// 描画関係
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	DrawBackLightSwitch();
	
	#ifdef DBGBNR
	NTRBannerDraw( s_csr, selected, pTitleList );
	#endif
	
	return ret;
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
	PrintfSJIS( B_LIGHT_BUTTON_TOP_X, B_LIGHT_BUTTON_TOP_Y, TXT_COLOR_RED,
					"BL:%d\n", TSD_GetBacklightBrightness() );
}

