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
#define BAR_LOOSENESS						0
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

// �t�F�[�h�A�E�g�֌W
#define FADE_COUNT_PER_ALPHA	(FADE_COUNT_MAX / ALPHA_MAX)
#define FADE_COUNT_MAX			124
#define ALPHA_MAX				31

// extern data------------------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_scr_data2[32 * 32];

// function's prototype declaration---------------------
static void LoadBannerFiles( void );
static void BannerInit( void );
static void SetDefaultBanner( TitleProperty *titleprop );
static void LoadBannerToVRAM( TitleProperty *titleprop );
static void SetAffineAnimation( int cursor );
static void BannerDraw(int cursor, int selected, TitleProperty *titleprop);
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

static u64	old_titleIdArray[ LAUNCHER_TITLE_LIST_NUM ];

static TWLBannerFile *empty_banner;
static TWLBannerFile *nobanner_banner;
static TWLBannerFile *no_card_banner;
static u8 image_index_list[ LAUNCHER_TITLE_LIST_NUM ];
static GXOamAttr banner_oam_attr[MAX_SHOW_BANNER+10];// �A�t�B���p�����[�^���߂�֌W�ŏ����傫��
static u8 *pbanner_image_list[ LAUNCHER_TITLE_LIST_NUM ];
static int banner_count = 0;
static int selected = 0;

// const data  -----------------------------------------

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
	
	MI_CpuClearFast(old_titleIdArray, sizeof(old_titleIdArray) );
    MI_DmaFill32(3, banner_oam_attr, 192, sizeof(banner_oam_attr));     // let out of the screen if not display
	
	// OBJMode�̐ݒ�
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);     // 2D mapping mode
	
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
		else if(titleprop[l].pBanner == NULL) //ID���[������Ȃ��̂Ƀo�i�[��NULL�Ȃ�m�[�o�i�[
		{
			titleprop[l].pBanner = nobanner_banner;
		}
	}
}

// VRAM�ւ̃o�i�[�C���[�W�f�[�^���[�h
static void LoadBannerToVRAM( TitleProperty *titleprop )
{
	int l;
    
    // �f�t�H���g�o�i�[��TitleProperty�ɖ��ߍ���
    SetDefaultBanner( titleprop );
	
    // TitleProperty������VRAM�ɃL�����N�^�f�[�^�����[�h
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		if(titleprop[l].titleID != old_titleIdArray[l])
		{
			// titleID�ύX����Ă�����A�ꂩ��VRAM�ւ̃o�i�[�摜���[�h���Ȃ���
			banner_count = 0;
			break;
		}
	}
	for(l=0;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		u8 m;
		u8 *pban=((TWLBannerFile *)titleprop[l].pBanner)->v1.image;
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
			{	// �o�i�[�摜���X�g�I�[�o�[���́AtitleID���X�V���ꂸ�Ƀo�i�[�̂ݓ���ւ���Ă���i�A�j���[�V�����H�j
				// ���̎����ł͏����s���B�ꂩ��S�����[�h���Ȃ����ق������S�B
				GX_LoadOBJ(pban, (u32)image_index_list[l]*BNR_IMAGE_SIZE , BNR_IMAGE_SIZE);
				pbanner_image_list[image_index_list[l]] = pban;
			}
		}
		
		old_titleIdArray[l] = titleprop[l].titleID;// ��̎Q�Ɨp
	}
}

// �A�t�B���p�����[�^�̐ݒ�
static void SetAffineAnimation( int cursor )
{
	MtxFx22 mtx;
	static double wav;
	if(cursor%FRAME_PER_SELECT == 0){			// �K���ɔg�ł����Ă݂�
		double s = sin(wav);
		mtx._00 = FX32_HALF - (long)( 0x80 * ( s - 1 ) );
		wav += 0.1;
	}else{										// �K���ɑ傫����ς��Ă݂�
		mtx._00 = FX32_HALF + FX32_HALF*(cursor%FRAME_PER_SELECT)/FRAME_PER_SELECT;
		wav = 0;
	}
	mtx._01 = 0;
	mtx._10 = 0;
	mtx._11 = mtx._00;
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
	mtx._00 = FX32_ONE - FX32_HALF*(cursor%FRAME_PER_SELECT)/FRAME_PER_SELECT;
	mtx._11 = mtx._00;
	G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[4]), &mtx);
}

// �o�i�[�֌W�̕`��
// �����}���Ή��̂��߁A����VRAM�ւ̃C���[�W�f�[�^���[�h��������Ă���
static void BannerDraw(int cursor, int selected, TitleProperty *titleprop)
{
	int l;
	int div1 = cursor / FRAME_PER_SELECT;
	int div2 = cursor % FRAME_PER_SELECT;
	
	LoadBannerToVRAM( titleprop );

	// �A�t�B���p�����[�^������ɐݒ肵�Ă���
	SetAffineAnimation( cursor );

	// OAM�f�[�^��M���Ĉʒu�ȂǕύX
	for (l=0;l<MAX_SHOW_BANNER;l++)
	{
		int num = div1 - 2 + l;
		if(-1 < num && num < LAUNCHER_TITLE_LIST_NUM){
			banner_oam_attr[l].charNo = image_index_list[num]*4;
		    // �p���b�g�̃��[�h
		    // �K�v�ȃp���b�g���ς��̂ŁA���x���x����ւ�
			GX_LoadOBJPltt( titleprop[num].pBanner->v1.pltt, (u16)(l * BNR_PLTT_SIZE), BNR_PLTT_SIZE );
			G2_SetOBJMode(&banner_oam_attr[l], GX_OAM_MODE_NORMAL, l);
			
			if(l == 2 || l == 3)	// �����t�߂ő傫���Ȃ����菬�����Ȃ����肷���̃o�i�[
			{
				G2_SetOBJEffect(&banner_oam_attr[l], GX_OAM_EFFECT_AFFINE_DOUBLE, l-2);
				G2_SetOBJPosition(&banner_oam_attr[l],
									BANNER_FAR_LEFT_POS - BANNER_WIDTH/2 + l*(BANNER_WIDTH + BANNER_INTERVAL) - div2 * DOT_PER_FRAME,
									BANNER_TOP - BANNER_HEIGHT/2 );
			}
			else					// ���̑��̃o�i�[
			{
				banner_oam_attr[l].x = BANNER_FAR_LEFT_POS + l*(BANNER_WIDTH + BANNER_INTERVAL) - div2 * DOT_PER_FRAME;
				G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NONE,0);
			}
		}else
		{
			G2_SetOBJEffect(&banner_oam_attr[l],GX_OAM_EFFECT_NODISPLAY,0);
		}
	}
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));
	
	// �A�v�����\��
	{
		NNSG2dChar *str = ((TWLBannerFile *)titleprop[selected].pBanner)->v1.comment[ TSD_GetLanguage() ];
		NNSG2dTextRect rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str );
		PutStringUTF16( (WINDOW_WIDTH-rect.width)>>1, TITLE_V_CENTER - (rect.height>>1), TXT_COLOR_BLACK, str );
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
	
	ChangeUserColor( TSD_GetUserColor() );
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
	
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.", SYSMENU_VER );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ );
	G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, 
			GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ, 0,ALPHA_MAX);

	GX_DispOn();
	GXS_DispOn();
	
	#ifdef DBGBNR
	BannerInit();
	#endif
}

// ROM�̃��[�f�B���O���̃����`���[�t�F�[�h�A�E�g
BOOL LauncherFadeout( TitleProperty *pTitleList )
{
	static int fadecount = 0;
	
	// �`��֌W
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	DrawBackLightSwitch();
	DrawScrollBar( pTitleList );
	
	#ifdef DBGBNR
	BannerDraw( s_csr, selected, pTitleList );
	#endif
	
	// �`�揭���ǉ�
	{
		MtxFx22 mtx;
		static double wa;
		double s = cos(wa);
		if( s!=0 ) mtx._00 = (fx32)(FX32_HALF/s);
		else mtx._00 = 0x8fff;
		mtx._01 = 0;
		mtx._10 = 0;
		mtx._11 = FX32_HALF;
		G2_SetOBJAffine((GXOamAffine *)(&banner_oam_attr[0]), &mtx);
		wa += 0.1;
	}
	
	DC_FlushRange(&banner_oam_attr, sizeof(banner_oam_attr));
	GX_LoadOAM(&banner_oam_attr, 0, sizeof(banner_oam_attr));

	// RTC���̎擾���\��
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	// �t�F�[�h�A�E�g�̃J�E���g����
	G2_ChangeBlendAlpha( fadecount/FADE_COUNT_PER_ALPHA, ALPHA_MAX-(fadecount/FADE_COUNT_PER_ALPHA) );
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
		brightness = TSD_GetBacklightBrightness() + 1;
		if( brightness > TWL_BACKLIGHT_LEVEL_MAX ) {
			brightness = 0;
		}
		SYSM_SetBackLightBrightness( (u8)brightness );
	}
	if( ( pad.trg & PAD_BUTTON_SELECT) || dw_bl_trg ) {
		brightness = TSD_GetBacklightBrightness() - 1;
		if( brightness < 0 ) {
			brightness = TWL_BACKLIGHT_LEVEL_MAX;
		}
		SYSM_SetBackLightBrightness( (u8)brightness );
	}
}

static TitleProperty *ProcessPads( TitleProperty *pTitleList )
{
	SelectSomethingFunc func[1]={SelectCenterFunc};
	BOOL tp_select = FALSE;
	u16 dummy;
	u16 tp_lr = 3;
	TitleProperty *ret = NULL;
	
	// �o�b�N���C�g�֌W�̃L�[����
	ProcessBackLightPads();
	
	// ���̑��̃L�[����
	if( tpd.disp.touch )
	{
		(void) SelectFunc( &tp_lr, &tpd.disp );
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
	if(s_csr%FRAME_PER_SELECT == 0){
		csr_v = 0;
		selected = s_csr/FRAME_PER_SELECT;
		
		// �o�i�[�������ɂ���Ƃ���������\
		tp_select = SelectSomethingByTP(&dummy, func, 1 );
		
		if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {					// ���j���[���ڂւ̕���
			if( pTitleList[selected].flags.isValid )
			{
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
		int bar_left = (int)(BAR_ZERO_X + (ITEMDOT_PER_FRAME * s_csr));
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
	if((LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT < s_csr) s_csr = (LAUNCHER_TITLE_LIST_NUM-1)*FRAME_PER_SELECT;
	if( s_csr < 0 ) s_csr = 0;
}

static void DrawScrollBar( TitleProperty *pTitleList )
{
	int l;
	for(l=0; l<LAUNCHER_TITLE_LIST_NUM; l++)
	{
		PutStringUTF16( (int)(BAR_ZERO_X + l * (ITEM_SIZE + ITEM_INTERVAL)), BAR_ZERO_Y, (pTitleList[l].flags.isValid ? TXT_UCOLOR_G1 : TXT_COLOR_BLACK), (const u16 *)L"�E" );
	}
	for(l=0; l<4; l++)
	{
		PutStringUTF16( (int)(BAR_ZERO_X + (ITEMDOT_PER_FRAME * s_csr) - l%2), BAR_ZERO_Y - l/2, TXT_UCOLOR_G1, (const u16 *)L"��" );
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
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "TWL-SYSTEM MENU ver.%06x", SYSMENU_VER );
	DrawBackLightSwitch();
	
	DrawScrollBar( pTitleList );
	
	#ifdef DBGBNR
	BannerDraw( s_csr, selected, pTitleList );
	#endif
	
	// RTC���̎擾���\��
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
	
	return ret;
}

// �o�b�N���C�g�X�C�b�`�̕\��
static void DrawBackLightSwitch(void)
{
	PutStringUTF16( B_LIGHT_DW_BUTTON_TOP_X, B_LIGHT_DW_BUTTON_TOP_Y, TXT_COLOR_RED,
					L"\xE01c\xE01b" );
	PrintfSJIS( B_LIGHT_DW_BUTTON_TOP_X + 24, B_LIGHT_DW_BUTTON_TOP_Y, TXT_COLOR_RED,
					"BL:%d\n", TSD_GetBacklightBrightness() );
}
