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
#include "sound.h"
#include <math.h>
#include <sysmenu/mcu.h>
#include <sysmenu/util_menuAppManager.h>


// define data------------------------------------------

// バックライトボタン関係
#define B_LIGHT_DW_BUTTON_TOP_X				( 191  )
#define B_LIGHT_DW_BUTTON_TOP_Y				( 0 )
#define B_LIGHT_DW_BUTTON_BOTTOM_X			( B_LIGHT_DW_BUTTON_TOP_X + 11 )
#define B_LIGHT_DW_BUTTON_BOTTOM_Y			( B_LIGHT_DW_BUTTON_TOP_Y + 13 )
#define B_LIGHT_UP_BUTTON_TOP_X				( 235  )
#define B_LIGHT_UP_BUTTON_TOP_Y				( 0 )
#define B_LIGHT_UP_BUTTON_BOTTOM_X			( B_LIGHT_UP_BUTTON_TOP_X + 11 )
#define B_LIGHT_UP_BUTTON_BOTTOM_Y			( B_LIGHT_UP_BUTTON_TOP_Y + 13 )

// スクロールバー関係
#define BAR_ZERO_X							( (WINDOW_WIDTH - ((ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM - 1) + ITEM_SIZE)) / 2 - 3)
#define BAR_ZERO_Y							(WINDOW_HEIGHT - 32 + 1)
#define BAR_HEIGHT							14
#define BAR_WIDTH							14 //((ITEM_SIZE + ITEM_INTERVAL) * 4 + ITEM_SIZE + 2)
#define BAR_LOOSENESS						2
#define ITEMDOT_PER_FRAME					((double)(ITEM_SIZE + ITEM_INTERVAL) / (double)FRAME_PER_SELECT)
#define FRAME_PER_ITEMDOT					((double)FRAME_PER_SELECT / (double)(ITEM_SIZE + ITEM_INTERVAL))
#define BAR_OFFSET							0				// 表示に"■"テキストを使っているので、タッチ座標を補正する目的のOFFSET
#define ITEM_SIZE							2
#define ITEM_INTERVAL						3

// バナー表示関係
#define DOT_PER_FRAME			((BANNER_WIDTH + BANNER_INTERVAL) / FRAME_PER_SELECT)		// 割り切れないと動きがカクカクするはず
#define FRAME_PER_SELECT		14															// バナーからバナーへの移動にかかるフレーム数
#define BANNER_FAR_LEFT_POS		(WINDOW_WIDTH/2 - BANNER_WIDTH*5/2 - BANNER_INTERVAL * 2)
#define BANNER_TOP				(WINDOW_HEIGHT/2 + 4)
#define WINDOW_WIDTH			256
#define WINDOW_HEIGHT			192
#define BANNER_WIDTH			32
#define BANNER_HEIGHT			32
#define BANNER_INTERVAL			24
#define TITLE_V_CENTER			47

#define MAX_SHOW_BANNER			6

#define MAX_LOAD_IMAGES			128

// フェードアウト関係
#define FADE_COUNT_PER_ALPHA	((FADE_COUNT_MAX - FADE_START) / ALPHA_MAX)
#define FADE_COUNT_MAX			62
#define ALPHA_MAX				31
#define FADE_START				31

// extern data------------------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_scr_data2[32 * 32];

// function's prototype declaration---------------------
static void LoadBannerFiles( void );
static void BannerInit( void );
static void SetDefaultBanner( TitleProperty *titleprop );
static void SetAffineAnimation( BOOL (*flipparam)[4] );
static void SetOAMAttr( void );
static void BannerDraw( int selected, TitleProperty *titleprop);
static BOOL SelectCenterFunc( u16 *csr, TPData *tgt );
static BOOL SelectFunc( u16 *csr, TPData *tgt );
static void ProcessBackLightPads( void );
static TitleProperty *ProcessPads( TitleProperty *pTitleList );
static void MoveByScrollBar();
static void DrawScrollBar();
static void DrawBackLightSwitch(void);

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------
static int s_csr = 0;										// 画面中央座標と、リストの一番最初にあるバナーの中央座標との距離を
															// 移動するのに必要なフレーム数で表すための変数
static int csr_v = 0;										// s_csrの速度的変数

static TWLBannerFile *empty_banner;
static TWLBannerFile *nobanner_banner;
static TWLBannerFile *no_card_banner;
static GXOamAttr banner_oam_attr[MAX_SHOW_BANNER+10];// アフィンパラメータ埋める関係で少し大きめ
static int selected = 0;
static int bar_left = BAR_ZERO_X;
static fx32 s_selected_banner_size;
static BOOL s_wavstop = FALSE;

static BOOL s_launcher_initialized = FALSE;

//static StreamInfo strm; // stream info

// const data  -----------------------------------------
//const char filename[] = "data/fanfare.32.wav";

//===============================================
// Launcher.c
//===============================================

//======================================================
// ランチャー
//======================================================

static void LoadBannerFiles( void )
{
	// デフォルトバナーファイルの読み込み。最終的にリブートしてしまうので、解放処理は無し
	u32 size = CMN_LoadFile( (void **)&empty_banner, "data/EmptyBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&nobanner_banner, "data/NoBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&no_card_banner, "data/NoCardBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
}

// パレットの読み込みやOBJ関係の初期化
static void BannerInit( void )
{
	int l;
	LoadBannerFiles();
	
    MI_DmaFill32(3, banner_oam_attr, 192, sizeof(banner_oam_attr));     // let out of the screen if not display
	
	// OBJModeの設定
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);     // 2D mapping mode
	
	//OBJATTRの初期化……表示前には値を弄る
	for(l=0;l<MAX_SHOW_BANNER;l++)
	{
		G2_SetOBJAttr(  &banner_oam_attr[l],							// OAM pointer
						0,												// X position
						BANNER_TOP,										// Y position
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

// TitlePropertyのIDとpBannerをチェックし、
// 特定の条件でpBannerにデフォルトバナーを指定する
static void SetDefaultBanner( TitleProperty *titleprop )
{
	int l;
    
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		if( !titleprop[l].flags.isValid ) //isValidフラグがFALSEの時はEmpty
		{
			if(l != 0)
			{
				titleprop[l].pBanner = empty_banner;
			}
			else
			{
				titleprop[l].pBanner = no_card_banner;
			}
			AMN_stepBannerAnime(l, TRUE); // バナーカウンタセットしなおし
		}
		else if(titleprop[l].pBanner == NULL) //isValidフラグがTRUEでバナーがNULLならノーバナー
		{
			titleprop[l].pBanner = nobanner_banner;
			AMN_stepBannerAnime(l, TRUE); // バナーカウンタセットしなおし
		}
	}
}

// 中央2枚(banner_oam_attr[2],banner_oam_attr[3])のバナーのアフィンパラメータの設定
// flipparamは左h,左v,右h,右vの順序
static void SetAffineAnimation( BOOL (*flipparam)[4] )
{
	MtxFx22 mtx;
	static double wav;
	fx32 param;
	u32 x,y;
	
	if(s_csr%FRAME_PER_SELECT == 0){			// 適当に波打たせてみる
		double s = sin(wav);
		s_selected_banner_size = FX32_HALF - (long)( 0x80 * ( s - 1 ) );
		param = s_selected_banner_size;
		if(!s_wavstop) wav += 0.1;
	}else{										// 適当に大きさを変えてみる
		param = FX32_HALF + FX32_HALF*(s_csr%FRAME_PER_SELECT)/FRAME_PER_SELECT;
		wav = 0;
	}
	
	// 中央左のバナー
	mtx._00 = param * ( (*flipparam)[0] ? -1 : 1 );
	mtx._01 = 0;
	mtx._10 = 0;
	mtx._11 = param * ( (*flipparam)[1] ? -1 : 1 );
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
	// アフィンでの反転時はアルゴリズムの関係で位置補正が必要……
	G2_GetOBJPosition(&banner_oam_attr[2], &x, &y);
	G2_SetOBJPosition(&banner_oam_attr[2], (int)x-( mtx._00==-FX32_ONE ? 1 : 0 ), (int)y-( mtx._11==-FX32_ONE ? 1 : 0 ));

	// 中央右のバナー
	param = FX32_ONE - FX32_HALF*(s_csr%FRAME_PER_SELECT)/FRAME_PER_SELECT;
	mtx._00 = param * ( (*flipparam)[2] ? -1 : 1 );
	mtx._11 = param * ( (*flipparam)[3] ? -1 : 1 );
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[4]), &mtx);
	// アフィンでの反転時はアルゴリズムの関係で位置補正が必要……
	G2_GetOBJPosition(&banner_oam_attr[3], &x, &y);
	G2_SetOBJPosition(&banner_oam_attr[3], (int)x-( mtx._00==-FX32_ONE ? 1 : 0 ), (int)y-( mtx._11==-FX32_ONE ? 1 : 0 ));
}

// OAMデータの設定
static void SetOAMAttr( void )
{
	int l;
	int div1 = s_csr / FRAME_PER_SELECT;
	int div2 = s_csr % FRAME_PER_SELECT;
	BOOL flipparam[4];

	AMN_stepBannerAnimeAll(FALSE);
	
	for (l=0;l<MAX_SHOW_BANNER;l++)
	{
		int num = div1 - 2 + l;
		if(-1 < num && num < LAUNCHER_TITLE_LIST_NUM){
			
		    // パレットのロード
			GX_LoadOBJPltt( AMN_getBannerPltt(num), (u16)(l * BANNER_PLTT_SIZE), BANNER_PLTT_SIZE );
			G2_SetOBJMode(&banner_oam_attr[l], GX_OAM_MODE_NORMAL, l);
			
			// バナー画像のロード
			GX_LoadOBJ( AMN_getBannerImage(num), (u32)l*BANNER_IMAGE_SIZE , BANNER_IMAGE_SIZE);

			// 表示画像の設定、キャラクタネーム境界128バイトである事に注意
			banner_oam_attr[l].charNo = l*4;
			
			// 位置およびエフェクトの設定
			if(l == 2 || l == 3)
			{
				// 中央付近で大きくなったり小さくなったりする二つのバナー
				G2_SetOBJEffect(&banner_oam_attr[l], GX_OAM_EFFECT_AFFINE_DOUBLE, l-2);
				G2_SetOBJPosition(&banner_oam_attr[l],
									BANNER_FAR_LEFT_POS - BANNER_WIDTH/2 + l*(BANNER_WIDTH + BANNER_INTERVAL) - div2 * DOT_PER_FRAME,
									BANNER_TOP - BANNER_HEIGHT/2 );
				flipparam[(l-2)*2] = AMN_getBannerHFlip(num);// フリップ情報は一旦保存
				flipparam[(l-2)*2+1] = AMN_getBannerVFlip(num);
			}
			else
			{
				// その他のバナー
				GXOamEffect effect = GX_OAM_EFFECT_NONE;
				if( AMN_getBannerVFlip(num) )
				{
					if( AMN_getBannerHFlip(num) )
					{
						effect = GX_OAM_EFFECT_FLIP_HV;
					}
					else
					{
						effect = GX_OAM_EFFECT_FLIP_V;
					}
				}
				else if( AMN_getBannerHFlip(num) )
				{
					effect = GX_OAM_EFFECT_FLIP_H;
				}
				banner_oam_attr[l].x = BANNER_FAR_LEFT_POS + l*(BANNER_WIDTH + BANNER_INTERVAL) - div2 * DOT_PER_FRAME;
				
				G2_SetOBJEffect(&banner_oam_attr[l],effect,0);
			}
		}else
		{
			// そのoamが担当する場所にはバナーがない
			G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NODISPLAY,0);
		}
	}
	
	// アフィンパラメータの設定
	SetAffineAnimation( &flipparam );
}

static BOOL my_EqualNString(NNSG2dChar *src, NNSG2dChar *dst, int size )
{
	int l;
	for( l=0;l<size;l++)
	{
		if(*src != *dst)
		{
			return FALSE;
		}
		
		src++;
		dst++;
		
		if(*src == 0x0000)
		{
			break;
		}
	}
	return TRUE;
}

// バナー関係の描画
// 思ったよりVRAMへのロードが高速だったので、
// 特に難しいことを考えず表示するイメージデータだけ毎フレームVRAMにロード
static void BannerDraw(int selected, TitleProperty *titleprop)
{
	static int fadecount = 0;
	static u16 old_gameName[BANNER_LANG_LENGTH];
	NNSG2dChar *str;
	
	// デフォルトバナーをTitlePropertyに埋め込み
    SetDefaultBanner( titleprop );
	
	// OAMデータの設定
	SetOAMAttr();
	
	// アプリ名表示
	str = ((TWLBannerFile *)titleprop[selected].pBanner)->v1.gameName[ LCFG_TSD_GetLanguage() ];
	if( !my_EqualNString( old_gameName, str, BANNER_LANG_LENGTH ) )
	{
		NNSG2dChar *str = ((TWLBannerFile *)titleprop[selected].pBanner)->v1.gameName[ LCFG_TSD_GetLanguage() ];
		NNSG2dTextRect rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str );
		NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL, 0, 24, WINDOW_WIDTH, 48 );
		PutStringUTF16( (WINDOW_WIDTH-rect.width)>>1, TITLE_V_CENTER - (rect.height>>1), TXT_COLOR_BLACK, str );
		MI_CpuCopy8( str, old_gameName, BANNER_LANG_LENGTH * 2 );
	}
	
	if(fadecount < (FADE_COUNT_MAX - FADE_START)) {
		fadecount += 1;
		G2_ChangeBlendAlpha( ALPHA_MAX-((fadecount)/FADE_COUNT_PER_ALPHA), (fadecount)/FADE_COUNT_PER_ALPHA );
	}
}

// ランチャーの初期化
void LauncherInit( TitleProperty *pTitleList )
{
#pragma unused( pTitleList )
	if(s_launcher_initialized)
	{
		return;
	}
	
	s_launcher_initialized = TRUE;
	InitBG();										// BG初期化
	
	GX_DispOff();
	GXS_DispOff();
	
	ChangeUserColor( LCFG_TSD_GetUserColor() );
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
	PutStringUTF16(   0,  0, TXT_COLOR_BLUE, L"SYSTEM MENU" );
	PrintfSJIS( 128,  0, TXT_COLOR_BLUE, "IPL:%s", g_strIPLSvnRevision );
	PrintfSJIS( 128, 12, TXT_COLOR_BLUE, "SDK:%s", g_strSDKSvnRevision );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ );
	G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, 
			GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ, ALPHA_MAX,0);

	GX_DispOn();
	GXS_DispOn();
	
	// streamInfo初期化
	//FS_InitFile(&strm.file);
	//strm.isPlay = FALSE;
	
	BannerInit();
}

// ROMのローディング中のランチャーフェードアウト
BOOL LauncherFadeout( TitleProperty *pTitleList )
{
	static int fadecount = 0;
	
	// 描画関係
	
	// 輝度表示
	DrawBackLightSwitch();
	
	DrawScrollBar( pTitleList );
	
	BannerDraw( selected, pTitleList );
	
	// 描画少し追加
	{
		MtxFx22 mtx;
		static double wa;
		double s = sin(wa*2.2);
		double c = cos(wa*2.2);
		mtx._00 = (fx32)((s_selected_banner_size*c) * (1.0 + wa));
		mtx._01 = (fx32)((s_selected_banner_size*s) * (1.0 + wa));
		mtx._10 = (fx32)((s_selected_banner_size*-s) * (1.0 + wa));
		mtx._11 = (fx32)((s_selected_banner_size*c) * (1.0 + wa));
		
		//if( c!=0 ) mtx._00 = (fx32)((s_selected_banner_size/c) * (1.0 + wa));
		//else mtx._00 = 0x8fff;
		//mtx._01 = 0;
		//mtx._10 = 0;
		//mtx._11 = (fx32)(s_selected_banner_size * (1.0 + wa));
		G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
		wa += 0.1;
	}
	
	// OAMをVRAMへロード
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));

	// RTC情報の取得＆表示
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	// フェードアウトのカウント処理
	if(fadecount >= FADE_START)
	{
		G2_ChangeBlendAlpha( (fadecount-FADE_START)/FADE_COUNT_PER_ALPHA, ALPHA_MAX-((fadecount-FADE_START)/FADE_COUNT_PER_ALPHA) );
	}
	if(fadecount < FADE_COUNT_MAX) {
		fadecount++;
		return FALSE;
	}else {
		// ディスプレイOFFにしないと起動時にノイズが表示される
		GX_DispOff();
		GXS_DispOff();
		return TRUE;
	}
}

// ProcessPadsのSelectSomethingByTPで使うSelectSomethingFuncの実装
static BOOL SelectCenterFunc( u16 *csr, TPData *tgt )
{
	// 単純な実装例
	int x = WINDOW_WIDTH/2 - BANNER_WIDTH;
	int y = BANNER_TOP - BANNER_HEIGHT/2;
	if(WithinRangeTP( x, y, x+BANNER_WIDTH*2, y+BANNER_HEIGHT*2, tgt ))
	{
		*csr = (u16)1;
		return TRUE;
	}
	
	return FALSE;
}

static BOOL SelectFunc( u16 *csr, TPData *tgt )
{
	int l;
	
	for(l=0; l<2; l++)
	{
		int x = 1*8 + l*28*8;
		int y = 20*8;
		if(WithinRangeTP( x, y, x+16, y+16, tgt ))
		{
			*csr = (u16)l;
			return TRUE;
		}
	}
	return FALSE;
}

static void ProcessBackLightPads( void )
{
	static BOOL up_bl_bak = FALSE;
	static BOOL dw_bl_bak = FALSE;
	BOOL up_bl_trg = FALSE;
	BOOL dw_bl_trg = FALSE;
	u8 brightness = 0;
	
	if(tpd.disp.touch) {
		BOOL up_bl = WithinRangeTP(	B_LIGHT_UP_BUTTON_TOP_X,    B_LIGHT_UP_BUTTON_TOP_Y,
									B_LIGHT_UP_BUTTON_BOTTOM_X, B_LIGHT_UP_BUTTON_BOTTOM_Y, &tpd.disp );
		BOOL dw_bl = WithinRangeTP(	B_LIGHT_DW_BUTTON_TOP_X,    B_LIGHT_DW_BUTTON_TOP_Y,
									B_LIGHT_DW_BUTTON_BOTTOM_X, B_LIGHT_DW_BUTTON_BOTTOM_Y, &tpd.disp );
		up_bl_trg = ( up_bl && tpd.disp.touch && !up_bl_bak ) ;
		dw_bl_trg = ( dw_bl && tpd.disp.touch && !dw_bl_bak ) ;
		up_bl_bak = tpd.disp.touch;
		dw_bl_bak = tpd.disp.touch;
	}else {
		up_bl_bak = FALSE;
		dw_bl_bak = FALSE;
	}
	
	if( (pad.trg & PAD_KEY_UP) || up_bl_trg ) {
		(void)UTL_GetBacklightBrightness( &brightness );
		if( ++brightness > BACKLIGHT_BRIGHTNESS_MAX ) {
			brightness = BACKLIGHT_BRIGHTNESS_MAX;
		}
		(void)UTL_SetBacklightBrightness( brightness );
	}
	if( ( pad.trg & PAD_KEY_DOWN) || dw_bl_trg ) {
		(void)UTL_GetBacklightBrightness( &brightness );
		if( --brightness < 0 ) {
			brightness = 0;
		}
		(void)UTL_SetBacklightBrightness( brightness );
	}
}

static TitleProperty *ProcessPads( TitleProperty *pTitleList )
{
	SelectSomethingFunc func[1]={SelectCenterFunc};
	BOOL tp_select = FALSE;
	u16 dummy;
	u16 tp_lr = 3;
	TitleProperty *ret = NULL;
	// バックライト関係のキー処理
	ProcessBackLightPads();
	
	// その他のキー処理
	if( tpd.disp.touch )
	{
		int x = WINDOW_WIDTH/2 - BANNER_WIDTH;
		int y = BANNER_TOP - BANNER_HEIGHT/2;
		if(WithinRangeTP( x, y, x+BANNER_WIDTH*2, y+BANNER_HEIGHT*2, &tpd.disp ))
		{
			s_wavstop = TRUE;
		}else
		{
			s_wavstop = FALSE;
		}
		(void) SelectFunc( &tp_lr, &tpd.disp );
	}else
	{
		s_wavstop = FALSE;
	}

	if( (pad.cont & (PAD_BUTTON_START | PAD_BUTTON_SELECT | PAD_BUTTON_X )) == (PAD_BUTTON_START | PAD_BUTTON_SELECT | PAD_BUTTON_X ) ) {
		OS_DoApplicationJump( NULL, OS_APP_JUMP_NORMAL );
	}
	
	if(pad.cont & PAD_KEY_RIGHT || tp_lr == 1){										// バナー選択
		if(csr_v == 0) csr_v = 1;
	}
	if( pad.cont & PAD_KEY_LEFT || tp_lr == 0){
		if(csr_v == 0) csr_v = -1;
	}
	s_csr += csr_v;
	if((LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT < s_csr) s_csr = (LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT;
	if( s_csr < 0 ) s_csr = 0;

	selected = (s_csr + FRAME_PER_SELECT/2)/FRAME_PER_SELECT;
	if(s_csr%FRAME_PER_SELECT == 0){
		csr_v = 0;
		
		// バナーが中央にあるときだけ決定可能
		tp_select = SelectSomethingByTP(&dummy, func, 1 );
		
		if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {					// メニュー項目への分岐
			if( pTitleList[selected].flags.isValid )
			{
				//PlayStream(&strm, filename);
				ret = &pTitleList[selected];
				// ブートするアプリのindex番号とTitleIDを本体設定に保存する。（実際の保存は、SYSMライブラリがブート時に行います。）
				LCFG_TSD_SetLastTimeBootSoftIndex( (u8)selected );
				LCFG_TSD_SetLastTimeBootSoftTitleID ( pTitleList[selected].titleID );
			}
		}
	}

    // HOTSW許可・抑制テスト
    {
        static BOOL hotswEnable = TRUE;

    	if( pad.trg & PAD_BUTTON_X ){
        	hotswEnable ^= TRUE;

            // 活線挿抜許可・抑制
			HOTSW_EnableHotSWAsync( hotswEnable );

            // 活線挿抜が指定した状態になるまでポーリング
    		while(HOTSW_isEnableHotSW() != hotswEnable){
				// do nothing
    		}

            NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL, 0, 175, 100, 13 );
    	}
        if(hotswEnable){
			PutStringUTF16(   0, 175, TXT_COLOR_BLUE, L"HotSw Enable" );
        }
        else{
			PutStringUTF16(   0, 175, TXT_COLOR_RED, L"HotSw Disable" );
        }
    }
    
	return ret;
}

// スクロールバーによるスクロール
static void MoveByScrollBar( void )
{
	static double vx = 0;
	static int last_move_dir = 0;
	static int last_bar_left;
	
	// スクロールバーによるスクロール
	{
		static BOOL holding = FALSE;
		static BOOL ground_holding = FALSE;
		static int dx;
		static const int list_len = 4;
		static int list_x[list_len];
		int l;
		
		if( !ground_holding && !holding && vx == 0)
		{
			bar_left = (int)(BAR_ZERO_X + (ITEMDOT_PER_FRAME * s_csr));
		}
		
		// 慣性移動
		if(vx!=0)
		{
			bar_left += (int)vx;
			s_csr = (u16)((bar_left - BAR_ZERO_X) * FRAME_PER_ITEMDOT);
			vx = vx * 0.9;
			if(vx*vx < 1)
			{
				vx = 0;
			}
		}
		
		if(tpd.disp.touch)
		{
			if(holding)
			{
				if ( tpd.disp.x - dx < bar_left - BAR_LOOSENESS)
				{
					bar_left = tpd.disp.x - dx + BAR_LOOSENESS;
				}
				else if ( tpd.disp.x - dx > bar_left + BAR_LOOSENESS)
				{
					bar_left = tpd.disp.x - dx - BAR_LOOSENESS;
				}
				s_csr = (u16)((bar_left - BAR_ZERO_X) * FRAME_PER_ITEMDOT);
				
				// 移動履歴追記
				for( l=0; l<list_len-1; l++ )
				{
					list_x[l] = list_x[l+1];
				}
				list_x[list_len-1] = tpd.disp.x;
			}
			else if(WithinRangeTP(bar_left+5-BAR_WIDTH/2, BAR_ZERO_Y+BAR_OFFSET,bar_left+5+BAR_WIDTH/2,BAR_ZERO_Y+BAR_OFFSET+BAR_HEIGHT,&tpd.disp) ||
					( ground_holding && WithinRangeTP(bar_left+5-BAR_WIDTH/2, 0,bar_left+5+BAR_WIDTH/2,WINDOW_HEIGHT,&tpd.disp) )
			)
			{
				holding = TRUE;
				dx = tpd.disp.x - bar_left;
				// 移動履歴初期化
				for( l=0; l<list_len; l++ )
				{
					list_x[l] = tpd.disp.x;
				}
			}
			else if(ground_holding)
			{
				bar_left += ( tpd.disp.x > bar_left ? 1 : -1 );
				s_csr = (u16)((bar_left - BAR_ZERO_X) * FRAME_PER_ITEMDOT);
			}
			else if( WithinRangeTP(BAR_ZERO_X+BAR_OFFSET, BAR_ZERO_Y+BAR_OFFSET,BAR_ZERO_X + (ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM),BAR_ZERO_Y+BAR_OFFSET+BAR_HEIGHT,&tpd.disp) )
			{
				ground_holding = TRUE;
			}
		}
		else
		{
			int det = s_csr % FRAME_PER_SELECT;
			if(holding)
			{
				holding = FALSE;
				
				// 移動履歴から速度算出
				for( l=1; l<list_len; l++ )
				{
					int ax = list_x[l] - list_x[l-1];
					vx += (double)ax;
					if(l == list_len-1 && ax == 0)
					{
						vx = 0;
					}
				}
				vx /= (list_len-1);
			}
			if(ground_holding)
			{
				ground_holding = FALSE;
			}
			
			// タッチパネル放置状態でカーソルがアイコンの中間にある場合は最後に動いてた方に移動
			if(vx == 0 && det != 0 && csr_v == 0)
			{
				csr_v = last_move_dir;
			}
		}
	}
	
	// タッチパッドによるスクロール後の調整
	if( BAR_ZERO_X + (ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM - 1) < bar_left )
	{
		bar_left = BAR_ZERO_X + (ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM - 1);
		vx = -vx;
	}
	if( bar_left < BAR_ZERO_X )
	{
		bar_left = BAR_ZERO_X;
		vx = -vx;
	}
	if((LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT < s_csr) s_csr = (LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT;
	if( s_csr < 0 ) s_csr = 0;
	
	// 最後に動いた方向の記録
	if(bar_left != last_bar_left)
		last_move_dir = ( bar_left > last_bar_left ? 1 : -1);
	last_bar_left = bar_left;
}

static void DrawScrollBar( TitleProperty *pTitleList )
{
	int l;
	static int col_count = 0;
	static int col_count_d = 1;
	static const int COL_FLAME_MAX = 30;
	static const int COL_NUM = 15;
	static const int COL_DIV = (COL_FLAME_MAX / COL_NUM);
	int colc_cold;
	static int oldx;
	
	col_count += col_count_d;
	if(col_count < 0)
	{
		col_count = 0;
		col_count_d = 1;
	}
	if(COL_FLAME_MAX <= col_count)
	{
		col_count = COL_FLAME_MAX - 1;
		col_count_d = -1;
	}
	
	colc_cold = col_count/COL_DIV;
	
	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL, oldx, BAR_ZERO_Y, 12, 13 );
	for(l=0; l<LAUNCHER_TITLE_LIST_NUM; l++)
	{
		PutStringUTF16( (int)(BAR_ZERO_X + l * (ITEM_SIZE + ITEM_INTERVAL)),
						BAR_ZERO_Y,
						(pTitleList[l].flags.isValid ? (TXT_UCOLOR_G0 + colc_cold) : TXT_COLOR_BLACK),
						(const u16 *)L"・" );
	}
	for(l=0; l<4; l++)
	{
		oldx = (int)(bar_left - l%2);
		PutStringUTF16( oldx, BAR_ZERO_Y - l/2, TXT_UCOLOR_G1, (const u16 *)L"□" );
	}
}

// ランチャーメイン
TitleProperty *LauncherMain( TitleProperty *pTitleList )
{
	TitleProperty *ret = NULL;

//#define DBGLP
#ifdef DBGLP
typedef struct NandFirmResetParameter {
	u8		isHotStart :1;
	u8		isResetSW :1;
	u8		rsv :5;
	u8		isValid :1;
}NandFirmResetParameter;
	{
		static BOOL a=FALSE;
		if( pad.cont & PAD_BUTTON_Y )
		{
			u8 *p = (u8 *)SYSMi_GetLauncherParamAddr();
			int l;
			if( a )
			{
				return ret;
			}
			NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
			for(l=0;l<16;l++)
				PrintfSJIS( 0, l*12, TXT_COLOR_RED, "%.02x%.02x%.02x%.02x %.02x%.02x%.02x%.02x",
							*(p+l*8), *(p+1+l*8), *(p+2+l*8), *(p+3+l*8), *(p+4+l*8), *(p+5+l*8), *(p+6+l*8), *(p+7+l*8) );
			a=TRUE;
			return ret;
		}
		if( a ){
			NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
			a=FALSE;
		}
	}
#endif

	// キー及びタッチ制御
	ret = ProcessPads( pTitleList );
	MoveByScrollBar();
	
	// 描画関係
	DrawBackLightSwitch();
	
	DrawScrollBar( pTitleList );
	
	BannerDraw( selected, pTitleList );
	
	// OAMをVRAMへロード
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));
	
	// RTC情報の取得＆表示
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	return ret;
}

// バックライトスイッチの表示
static void DrawBackLightSwitch(void)
{
	static int old_brightness = -1;
	u8 brightness;
	
	(void)UTL_GetBacklightBrightness( &brightness );
	
	// 1フレーム前の古い値と輝度値が違っていたら描画しなおし
	if( old_brightness != brightness )
	{
		old_brightness = brightness;
		
		NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL, B_LIGHT_DW_BUTTON_TOP_X + 24, B_LIGHT_DW_BUTTON_TOP_Y, 40, 13 );
		PutStringUTF16( B_LIGHT_DW_BUTTON_TOP_X, B_LIGHT_DW_BUTTON_TOP_Y, TXT_COLOR_RED,
						L"\xE01c　　　\xE01b" );
		PrintfSJIS( B_LIGHT_DW_BUTTON_TOP_X + 11, B_LIGHT_DW_BUTTON_TOP_Y, TXT_COLOR_RED,
						"BL:%2d\n", brightness );
	}
}
