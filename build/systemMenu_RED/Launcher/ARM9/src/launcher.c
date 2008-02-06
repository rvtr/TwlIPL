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
#include "bannerCounter.h"
#include "sound.h"
#include <math.h>


// define data------------------------------------------

// �o�b�N���C�g�{�^���֌W
#define B_LIGHT_DW_BUTTON_TOP_X				( 0  )
#define B_LIGHT_DW_BUTTON_TOP_Y				( 22 * 8 )
#define B_LIGHT_DW_BUTTON_BOTTOM_X			( B_LIGHT_DW_BUTTON_TOP_X + 11 )
#define B_LIGHT_DW_BUTTON_BOTTOM_Y			( B_LIGHT_DW_BUTTON_TOP_Y + 16 )
#define B_LIGHT_UP_BUTTON_TOP_X				( 11  )
#define B_LIGHT_UP_BUTTON_TOP_Y				( 22 * 8 )
#define B_LIGHT_UP_BUTTON_BOTTOM_X			( B_LIGHT_UP_BUTTON_TOP_X + 22 )
#define B_LIGHT_UP_BUTTON_BOTTOM_Y			( B_LIGHT_UP_BUTTON_TOP_Y + 16 )

// �X�N���[���o�[�֌W
#define BAR_ZERO_X							( (WINDOW_WIDTH - ((ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM - 1) + ITEM_SIZE)) / 2)
#define BAR_ZERO_Y							WINDOW_HEIGHT - 32
#define BAR_HEIGHT							14
#define BAR_WIDTH							32 //((ITEM_SIZE + ITEM_INTERVAL) * 4 + ITEM_SIZE + 2)
#define BAR_LOOSENESS						2
#define ITEMDOT_PER_FRAME					((double)(ITEM_SIZE + ITEM_INTERVAL) / (double)FRAME_PER_SELECT)
#define FRAME_PER_ITEMDOT					((double)FRAME_PER_SELECT / (double)(ITEM_SIZE + ITEM_INTERVAL))
#define BAR_OFFSET							0				// �\����"��"�e�L�X�g���g���Ă���̂ŁA�^�b�`���W��␳����ړI��OFFSET
#define ITEM_SIZE							2
#define ITEM_INTERVAL						3

// �o�i�[�\���֌W
#define DOT_PER_FRAME			((BANNER_WIDTH + BANNER_INTERVAL) / FRAME_PER_SELECT)		// ����؂�Ȃ��Ɠ������J�N�J�N����͂�
#define FRAME_PER_SELECT		14															// �o�i�[����o�i�[�ւ̈ړ��ɂ�����t���[����
#define BANNER_FAR_LEFT_POS		(WINDOW_WIDTH/2 - BANNER_WIDTH*5/2 - BANNER_INTERVAL * 2)
#define BANNER_TOP				(WINDOW_HEIGHT/2 - 16)
#define WINDOW_WIDTH			256
#define WINDOW_HEIGHT			192
#define BANNER_WIDTH			32
#define BANNER_HEIGHT			32
#define BANNER_INTERVAL			24
#define TITLE_V_CENTER			39

#define MAX_SHOW_BANNER			6

#define MAX_LOAD_IMAGES			128

// �t�F�[�h�A�E�g�֌W
#define FADE_COUNT_PER_ALPHA	((FADE_COUNT_MAX - FADE_START) / ALPHA_MAX)
#define FADE_COUNT_MAX			124
#define ALPHA_MAX				31
#define FADE_START				62

// extern data------------------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_scr_data2[32 * 32];

// function's prototype declaration---------------------
static void LoadBannerFiles( void );
static void BannerInit( void );
static void SetDefaultBanner( TitleProperty *titleprop );
static void SetAffineAnimation( BOOL (*flipparam)[4] );
static void SetBannerCounter( TitleProperty *titleprop );
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
static int s_csr = 0;										// ��ʒ������W�ƁA���X�g�̈�ԍŏ��ɂ���o�i�[�̒������W�Ƃ̋�����
															// �ړ�����̂ɕK�v�ȃt���[�����ŕ\�����߂̕ϐ�
static int csr_v = 0;										// s_csr�̑��x�I�ϐ�

static TWLBannerFile *empty_banner;
static TWLBannerFile *nobanner_banner;
static TWLBannerFile *no_card_banner;
static GXOamAttr banner_oam_attr[MAX_SHOW_BANNER+10];// �A�t�B���p�����[�^���߂�֌W�ŏ����傫��
static int selected = 0;
static int bar_left = BAR_ZERO_X;
static fx32 s_selected_banner_size;
static BOOL s_wavstop = FALSE;
static BannerCounter banner_counter[LAUNCHER_TITLE_LIST_NUM];

//static StreamInfo strm; // stream info

// const data  -----------------------------------------
//const char filename[] = "data/fanfare.32.wav";

//===============================================
// Launcher.c
//===============================================

//======================================================
// �����`���[
//======================================================

// �o�i�[�\���֌W�i�b��j
#define DBGBNR
#ifdef DBGBNR


static void LoadBannerFiles( void )
{
	// �f�t�H���g�o�i�[�t�@�C���̓ǂݍ��݁B�ŏI�I�Ƀ��u�[�g���Ă��܂��̂ŁA��������͖���
	u32 size = CMN_LoadFile( (void **)&empty_banner, "data/EmptyBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&nobanner_banner, "data/NoBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
	size = CMN_LoadFile( (void **)&no_card_banner, "data/NoCardBanner.bnr", &g_allocator);
	NNS_G2D_ASSERT( size > 0 );
}

// �p���b�g�̓ǂݍ��݂�OBJ�֌W�̏�����
static void BannerInit( void )
{
	int l;
	LoadBannerFiles();
	
    MI_DmaFill32(3, banner_oam_attr, 192, sizeof(banner_oam_attr));     // let out of the screen if not display
	
	// OBJMode�̐ݒ�
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);     // 2D mapping mode
    
    // BannerCounter�̏�����
    for( l=0; l<LAUNCHER_TITLE_LIST_NUM; l++ )
    {
		BNC_initCounter( &banner_counter[l], empty_banner);
	}
	
	//OBJATTR�̏������c�c�\���O�ɂ͒l��M��
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

// TitleProperty��ID��pBanner���`�F�b�N���A
// ����̏�����pBanner�Ƀf�t�H���g�o�i�[���w�肷��
static void SetDefaultBanner( TitleProperty *titleprop )
{
	int l;
    
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		if( !titleprop[l].flags.isValid ) //isValid�t���O��FALSE�̎���Empty
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
		else if(titleprop[l].pBanner == NULL) //isValid�t���O��TRUE�Ńo�i�[��NULL�Ȃ�m�[�o�i�[
		{
			titleprop[l].pBanner = nobanner_banner;
		}
	}
}

// ����2��(banner_oam_attr[2],banner_oam_attr[3])�̃o�i�[�̃A�t�B���p�����[�^�̐ݒ�
// flipparam�͍�h,��v,�Eh,�Ev�̏���
static void SetAffineAnimation( BOOL (*flipparam)[4] )
{
	MtxFx22 mtx;
	static double wav;
	fx32 param;
	u32 x,y;
	
	if(s_csr%FRAME_PER_SELECT == 0){			// �K���ɔg�ł����Ă݂�
		double s = sin(wav);
		s_selected_banner_size = FX32_HALF - (long)( 0x80 * ( s - 1 ) );
		param = s_selected_banner_size;
		if(!s_wavstop) wav += 0.1;
	}else{										// �K���ɑ傫����ς��Ă݂�
		param = FX32_HALF + FX32_HALF*(s_csr%FRAME_PER_SELECT)/FRAME_PER_SELECT;
		wav = 0;
	}
	
	// �������̃o�i�[
	mtx._00 = param * ( (*flipparam)[0] ? -1 : 1 );
	mtx._01 = 0;
	mtx._10 = 0;
	mtx._11 = param * ( (*flipparam)[1] ? -1 : 1 );
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
	// �A�t�B���ł̔��]���̓A���S���Y���̊֌W�ňʒu�␳���K�v�c�c
	G2_GetOBJPosition(&banner_oam_attr[2], &x, &y);
	G2_SetOBJPosition(&banner_oam_attr[2], (int)x-( mtx._00==-FX32_ONE ? 1 : 0 ), (int)y-( mtx._11==-FX32_ONE ? 1 : 0 ));

	// �����E�̃o�i�[
	param = FX32_ONE - FX32_HALF*(s_csr%FRAME_PER_SELECT)/FRAME_PER_SELECT;
	mtx._00 = param * ( (*flipparam)[2] ? -1 : 1 );
	mtx._11 = param * ( (*flipparam)[3] ? -1 : 1 );
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[4]), &mtx);
	// �A�t�B���ł̔��]���̓A���S���Y���̊֌W�ňʒu�␳���K�v�c�c
	G2_GetOBJPosition(&banner_oam_attr[3], &x, &y);
	G2_SetOBJPosition(&banner_oam_attr[3], (int)x-( mtx._00==-FX32_ONE ? 1 : 0 ), (int)y-( mtx._11==-FX32_ONE ? 1 : 0 ));
}

static void SetBannerCounter( TitleProperty *titleprop )
{
	int l;
	for( l=0; l<LAUNCHER_TITLE_LIST_NUM; l++ )
	{
		// nand���ꉞ����Z�b�g
		BNC_setBanner( &banner_counter[l], titleprop[l].pBanner);
		if( l==0 )
		{
			// �J�[�h�̏ꍇ�A�o�i�[�w�b�_��v1��CRC���������J�E���g�����Z�b�g
			if ( BNC_getBanner( &banner_counter[l] )->h.crc16_v1 != titleprop[l].pBanner->h.crc16_v1)
			{
				BNC_resetCount( &banner_counter[l] );
			}
		}
	}
}

// OAM�f�[�^�̐ݒ�
static void SetOAMAttr( void )
{
	int l;
	int div1 = s_csr / FRAME_PER_SELECT;
	int div2 = s_csr % FRAME_PER_SELECT;
	BOOL flipparam[4];

	for (l=0;l<MAX_SHOW_BANNER;l++)
	{
		int num = div1 - 2 + l;
		if(-1 < num && num < LAUNCHER_TITLE_LIST_NUM){
			// �o�i�[�J�E���^����t���[���f�[�^���擾���A�J�E���^���C���N�������g
			FrameAnimeData fad = BNC_getFADAndIncCount( &banner_counter[num] );
			
		    // �p���b�g�̃��[�h
			GX_LoadOBJPltt( fad.pltt, (u16)(l * BANNER_PLTT_SIZE), BANNER_PLTT_SIZE );
			G2_SetOBJMode(&banner_oam_attr[l], GX_OAM_MODE_NORMAL, l);
			
			// �o�i�[�摜�̃��[�h
			GX_LoadOBJ( fad.image, (u32)l*BANNER_IMAGE_SIZE , BANNER_IMAGE_SIZE);

			// �\���摜�̐ݒ�A�L�����N�^�l�[�����E128�o�C�g�ł��鎖�ɒ���
			banner_oam_attr[l].charNo = l*4;
			
			// �ʒu����уG�t�F�N�g�̐ݒ�
			if(l == 2 || l == 3)
			{
				// �����t�߂ő傫���Ȃ����菬�����Ȃ����肷���̃o�i�[
				G2_SetOBJEffect(&banner_oam_attr[l], GX_OAM_EFFECT_AFFINE_DOUBLE, l-2);
				G2_SetOBJPosition(&banner_oam_attr[l],
									BANNER_FAR_LEFT_POS - BANNER_WIDTH/2 + l*(BANNER_WIDTH + BANNER_INTERVAL) - div2 * DOT_PER_FRAME,
									BANNER_TOP - BANNER_HEIGHT/2 );
				flipparam[(l-2)*2] = fad.hflip;// �t���b�v���͈�U�ۑ�
				flipparam[(l-2)*2+1] = fad.vflip;
			}
			else
			{
				// ���̑��̃o�i�[
				GXOamEffect effect = GX_OAM_EFFECT_NONE;
				if( fad.vflip )
				{
					if( fad.hflip )
					{
						effect = GX_OAM_EFFECT_FLIP_HV;
					}
					else
					{
						effect = GX_OAM_EFFECT_FLIP_V;
					}
				}
				else if( fad.hflip )
				{
					effect = GX_OAM_EFFECT_FLIP_H;
				}
				banner_oam_attr[l].x = BANNER_FAR_LEFT_POS + l*(BANNER_WIDTH + BANNER_INTERVAL) - div2 * DOT_PER_FRAME;
				
				G2_SetOBJEffect(&banner_oam_attr[l],effect,0);
			}
		}else
		{
			// ����oam���S������ꏊ�ɂ̓o�i�[���Ȃ�
			G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NODISPLAY,0);
		}
	}
	
	// �A�t�B���p�����[�^�̐ݒ�
	SetAffineAnimation( &flipparam );
}

// �o�i�[�֌W�̕`��
// �v�������VRAM�ւ̃��[�h�������������̂ŁA
// ���ɓ�����Ƃ��l�����\������C���[�W�f�[�^�������t���[��VRAM�Ƀ��[�h
static void BannerDraw(int selected, TitleProperty *titleprop)
{
	static int fadecount = 0;
	static int old_selected = -1;
	
	// �f�t�H���g�o�i�[��TitleProperty�ɖ��ߍ���
    SetDefaultBanner( titleprop );
	
	// �o�i�[�J�E���^�̃o�i�[�Z�b�g
	SetBannerCounter( titleprop );
	
	// OAM�f�[�^�̐ݒ�
	SetOAMAttr();
	
	// OAM��VRAM�փ��[�h
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));
	
	// �A�v�����\��
	if(selected != old_selected)
	{
		NNSG2dChar *str = ((TWLBannerFile *)titleprop[selected].pBanner)->v1.gameName[ LCFG_TSD_GetLanguage() ];
		NNSG2dTextRect rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str );
		NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL, 0, 24, WINDOW_WIDTH, 32 );
		PutStringUTF16( (WINDOW_WIDTH-rect.width)>>1, TITLE_V_CENTER - (rect.height>>1), TXT_COLOR_BLACK, str );
		old_selected = selected;
	}
	
	if(fadecount < (FADE_COUNT_MAX - FADE_START)) {
		fadecount += 2;
		G2_ChangeBlendAlpha( ALPHA_MAX-((fadecount)/FADE_COUNT_PER_ALPHA), (fadecount)/FADE_COUNT_PER_ALPHA );
	}
}

#endif //DBGBNR


// �����`���[�̏�����
void LauncherInit( TitleProperty *pTitleList )
{
#pragma unused( pTitleList )
	
	InitBG();										// BG������
	
	GX_DispOff();
	GXS_DispOff();
	
	ChangeUserColor( LCFG_TSD_GetUserColor() );
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    
    // BG�f�[�^�̃��[�h����
	GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));
	
	// �t�F�[�h�A�E�g�pBG�f�[�^�쐬�ƃ��[�h
	SVC_CpuClear( 0x0004, &bg_scr_data2, sizeof(bg_scr_data2), 16 );
	DC_FlushRange(&bg_scr_data2, sizeof(bg_scr_data2));
	GX_LoadBG2Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG2Scr(bg_scr_data2, 0, sizeof(bg_scr_data2));
			
	DrawBackLightSwitch();
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ );
	G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, 
			GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ, ALPHA_MAX,0);

	GX_DispOn();
	GXS_DispOn();
	
	// streamInfo������
	//FS_InitFile(&strm.file);
	//strm.isPlay = FALSE;
	
	#ifdef DBGBNR
	BannerInit();
	#endif
}

// ROM�̃��[�f�B���O���̃����`���[�t�F�[�h�A�E�g
BOOL LauncherFadeout( TitleProperty *pTitleList )
{
	static int fadecount = 0;
	
	// �`��֌W
    //NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	//PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	//DrawBackLightSwitch();
	DrawScrollBar( pTitleList );
	
	#ifdef DBGBNR
	BannerDraw( selected, pTitleList );
	#endif
	
	// �`�揭���ǉ�
	{
		MtxFx22 mtx;
		static double wa;
		double s = cos(wa);
		if( s!=0 ) mtx._00 = (fx32)((s_selected_banner_size/s) * (1.0 + wa/3));
		else mtx._00 = 0x8fff;
		mtx._01 = 0;
		mtx._10 = 0;
		mtx._11 = (fx32)(s_selected_banner_size * (1.0 + wa/3));
		G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
		wa += 0.1;
	}
	
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));

	// RTC���̎擾���\��
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	// �t�F�[�h�A�E�g�̃J�E���g����
	if(fadecount >= FADE_START)
	{
		G2_ChangeBlendAlpha( (fadecount-FADE_START)/FADE_COUNT_PER_ALPHA, ALPHA_MAX-((fadecount-FADE_START)/FADE_COUNT_PER_ALPHA) );
	}
	if(fadecount < FADE_COUNT_MAX) {
		fadecount++;
		return FALSE;
	}else {
		// �f�B�X�v���COFF�ɂ��Ȃ��ƋN�����Ƀm�C�Y���\�������
		GX_DispOff();
		GXS_DispOff();
		return TRUE;
	}
}

// ProcessPads��SelectSomethingByTP�Ŏg��SelectSomethingFunc�̎���
static BOOL SelectCenterFunc( u16 *csr, TPData *tgt )
{
	// �P���Ȏ�����
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
		int x = 11*8 + l*6*8;
		int y = 17*8;
		if(WithinRangeTP( x, y, x+32, y+16, tgt ))
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
	int brightness;
	
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
	
	if( (pad.trg & PAD_BUTTON_START) || up_bl_trg ) {
		brightness = LCFG_TSD_GetBacklightBrightness() + 1;
		if( brightness > LCFG_TWL_BACKLIGHT_LEVEL_MAX ) {
			brightness = 0;
		}
		SYSM_SetBackLightBrightness( (u8)brightness );
		DrawBackLightSwitch();
	}
	if( ( pad.trg & PAD_BUTTON_SELECT) || dw_bl_trg ) {
		brightness = LCFG_TSD_GetBacklightBrightness() - 1;
		if( brightness < 0 ) {
			brightness = LCFG_TWL_BACKLIGHT_LEVEL_MAX;
		}
		SYSM_SetBackLightBrightness( (u8)brightness );
		DrawBackLightSwitch();
	}
}

static TitleProperty *ProcessPads( TitleProperty *pTitleList )
{
	SelectSomethingFunc func[1]={SelectCenterFunc};
	BOOL tp_select = FALSE;
	u16 dummy;
	u16 tp_lr = 3;
	TitleProperty *ret = NULL;
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, FALSE, FALSE, 0};
	
	// �o�b�N���C�g�֌W�̃L�[����
	ProcessBackLightPads();
	
	// ���̑��̃L�[����
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

	if( pad.trg & PAD_BUTTON_B ) {
		OS_SetLauncherParamAndResetHardware( NULL, &tempflag );
	}
	
	if(pad.cont & PAD_KEY_RIGHT || tp_lr == 1){										// �o�i�[�I��
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
		
		// �o�i�[�������ɂ���Ƃ���������\
		tp_select = SelectSomethingByTP(&dummy, func, 1 );
		
		if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {					// ���j���[���ڂւ̕���
			if( pTitleList[selected].flags.isValid )
			{
				//PlayStream(&strm, filename);
				ret = &pTitleList[selected];
			}
		}
	}
	
	return ret;
}

// �X�N���[���o�[�ɂ��X�N���[��
// ���\�K���Ȏ����B
// �{���A�o�[�̃z�[���h���̓o�[���W�𒆐S�ɓ������ׂ��B
static void MoveByScrollBar( void )
{
	// �X�N���[���o�[�ɂ��X�N���[��
	{
		static BOOL holding = FALSE;
		static int dx;
		
		if(!holding)
		{
			bar_left = (int)(BAR_ZERO_X + (ITEMDOT_PER_FRAME * s_csr));
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
			}
			else if(WithinRangeTP(bar_left+5-BAR_WIDTH/2, BAR_ZERO_Y+BAR_OFFSET,bar_left+5+BAR_WIDTH/2,BAR_ZERO_Y+BAR_OFFSET+BAR_HEIGHT,&tpd.disp))
			{
				holding = TRUE;
				dx = tpd.disp.x - bar_left;
			}
		}
		else
		{
			if(holding)
			{
				int det = s_csr % FRAME_PER_SELECT;
				holding = FALSE;
				csr_v = (det < FRAME_PER_SELECT/2) ? (det == 0 ? 0 : -1) : 1;
			}
		}
	}
	
	// �^�b�`�p�b�h�ɂ��X�N���[����̒���
	if( BAR_ZERO_X + (ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM - 1) < bar_left )
		bar_left = BAR_ZERO_X + (ITEM_SIZE + ITEM_INTERVAL) * (LAUNCHER_TITLE_LIST_NUM - 1);
	if( bar_left < BAR_ZERO_X ) bar_left = BAR_ZERO_X;
	if((LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT < s_csr) s_csr = (LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT;
	if( s_csr < 0 ) s_csr = 0;
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
						(const u16 *)L"�E" );
	}
	for(l=0; l<4; l++)
	{
		oldx = (int)(bar_left - l%2);
		PutStringUTF16( oldx, BAR_ZERO_Y - l/2, TXT_UCOLOR_G1, (const u16 *)L"��" );
	}
}

// �����`���[���C��
TitleProperty *LauncherMain( TitleProperty *pTitleList )
{
	TitleProperty *ret = NULL;
	
	// �L�[�y�у^�b�`����
	ret = ProcessPads( pTitleList );
	MoveByScrollBar();
	
	// �`��֌W
    //NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	//PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	//DrawBackLightSwitch();
	
	DrawScrollBar( pTitleList );
	
	#ifdef DBGBNR
	BannerDraw( selected, pTitleList );
	#endif
	
	// RTC���̎擾���\��
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	return ret;
}

// �o�b�N���C�g�X�C�b�`�̕\��
static void DrawBackLightSwitch(void)
{
	NNS_G2dCharCanvasClearArea( &gCanvas, TXT_COLOR_NULL, B_LIGHT_DW_BUTTON_TOP_X + 24, B_LIGHT_DW_BUTTON_TOP_Y, 40, 13 );
	PutStringUTF16( B_LIGHT_DW_BUTTON_TOP_X, B_LIGHT_DW_BUTTON_TOP_Y, TXT_COLOR_RED,
					L"\xE01c\xE01b" );
	PrintfSJIS( B_LIGHT_DW_BUTTON_TOP_X + 24, B_LIGHT_DW_BUTTON_TOP_Y, TXT_COLOR_RED,
					"BL:%d\n", LCFG_TSD_GetBacklightBrightness() );
}
