// from main.c


void myDEMOInit( void )
{
	DEMOInitCommon();
	DEMOInitVRAM();
	DEMOInitDisplayBitmap();
	DEMOHookConsole();
	DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
	DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
	DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:		 InitScreen

  Description:  BG �ʂ�ݒ肵�܂��B

  Arguments:	�Ȃ��B

  Returns:	  �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitScreen(void)
{
	GXRgb myPalette[16] =
	{
		GX_RGB( 0,  0,  0), GX_RGB(31, 31, 31), GX_RGB( 31,  31,  0), GX_RGB(31,  0,  0),
		GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
		GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
		GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
	};
	
	// BG 1 ��ݒ�
	G2S_SetBG1Control(
		GX_BG_SCRSIZE_TEXT_256x256,	 // �X�N���[���T�C�Y 256x256
		GX_BG_COLORMODE_16,			 // �J���[���[�h	 16�F
		GX_BG_SCRBASE_0x0000,		   // �X�N���[���x�[�X
		GX_BG_CHARBASE_0x00000,		 // �L�����N�^�x�[�X
		GX_BG_EXTPLTT_01				// �g���p���b�g�X���b�g
	);

	// BG1 ������
	GXS_SetVisiblePlane(GX_GetVisiblePlane() & ~GX_PLANEMASK_BG1);

	// �J���[�p���b�g��ݒ�
	GX_LoadBGPltt( myPalette, 0, sizeof(myPalette) );
}

/*---------------------------------------------------------------------------*
  Name:		 InitCanvas

  Description:  ������`��̏����������܂��B

  Arguments:	�Ȃ��B

  Returns:	  �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitCanvas(void)
{

	// BG�L�����N�^�x�[�X�ւ̃|�C���^
	GXCharFmt16* const pCharBase = (GXCharFmt16*)G2_GetBG1CharPtr();
	int cOffset = CHARACTER_OFFSET;
	
	TXT_LoadFont(&gFont, FONT_RESOURCE);

	// CharCanvas �̏������� BG �X�N���[���̐ݒ�
	// CharCanvas 0
	NNS_G2dCharCanvasInitForBG(
		&gCanvas[0],				// CharCanvas�ւ̃|�C���^
		pCharBase + cOffset,		// �g�p����L�����N�^��擪�ւ̃|�C���^
		CANVAS0_WIDTH,				// CharCanvas��
		CANVAS0_HEIGHT,				// CharCanvas����
		NNS_G2D_CHARA_COLORMODE_16	// �J���[���[�h
	);
	NNS_G2dMapScrToCharText(
		G2S_GetBG1ScrPtr(),			// �X�N���[���x�[�X�ւ̃|�C���^
		CANVAS0_WIDTH,				// CharCanvas��
		CANVAS0_HEIGHT,				// CharCanvas����
		CANVAS0_LEFT,				// CharCanvas�\���ʒu
		CANVAS0_TOP,				// CharCanvas�\���ʒu
		NNS_G2D_TEXT_BG_WIDTH_256,	// �X�N���[����
		cOffset,					// �g�p����L�����N�^��擪�̃L�����N�^�ԍ�
		0			// �J���[�p���b�g�ԍ�
	);

	// CharCanvas 0 ���g�����̃L�����N�^�����I�t�Z�b�g�ɉ��Z
	cOffset += CANVAS0_WIDTH * CANVAS0_HEIGHT;

	// CharCanvas 1
	NNS_G2dCharCanvasInitForBG(
		&gCanvas[1],
		pCharBase + cOffset,
		CANVAS1_WIDTH,
		CANVAS1_HEIGHT,
		NNS_G2D_CHARA_COLORMODE_16
	);
	NNS_G2dMapScrToCharText(
		G2S_GetBG1ScrPtr(),
		CANVAS1_WIDTH,
		CANVAS1_HEIGHT,
		CANVAS1_LEFT,
		CANVAS1_TOP,
		NNS_G2D_TEXT_BG_WIDTH_256,
		cOffset,
		0
	);

	// CharCanvas 1 ���g�����̃L�����N�^�����I�t�Z�b�g�ɉ��Z
	cOffset += CANVAS1_WIDTH * CANVAS1_HEIGHT;

  
	// TextCanvas �̏�����
	// CharCanvas ���� TextCanvas ��p�ӂ��邱�Ƃ��ł��܂����A
	// ������ TextCanvas ��1�� CharCanvas �����L���鎖���ł��܂��B
	// ������ CharCanvas ��1�� TextCanvas �Ŏg���ꍇ��
	// TextCanvas �� CharCanvas ��u�������Ȃ���g�����ɂȂ�܂��B

	// 1) ������ TextCanvas ��1�� CharCanvas �����L
	// CharCanvas 1 �� TextCanvas 0, 1 �ŋ��L
	{
		NNS_G2dTextCanvasInit(
			&gTextCanvas[0],	// TextCanvas�ւ̃|�C���^
			&gCanvas[0],		// �`����CharCanvas�ւ̃|�C���^
			(NNSG2dFont*)s_pFontBuffer,		// �`��ɗp����t�H���g�ւ̃|�C���^
			TEXT_HSPACE,		// ������
			TEXT_VSPACE		// �s��
		);
	}

	// 2) CharCanvas ���� TextCanvas ��p��
	// CharCanvas 2 �� TextCanvas 2 �P�ƂŎg�p
	{
		NNS_G2dTextCanvasInit(
			&gTextCanvas[1],
			&gCanvas[1],
			(NNSG2dFont*)s_pFontBuffer,
			TEXT_HSPACE,
			TEXT_VSPACE
		);
	}
}

