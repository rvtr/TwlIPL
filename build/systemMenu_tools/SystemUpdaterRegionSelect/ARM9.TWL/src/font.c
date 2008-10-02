/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/load/g2d_NFT_load.h> 
#include <nnsys/g2d/g2di_Char.h>
#include "font.h"
#include "kami_global.h"

// ������`��֐�������������Unicode�ɂ���
#define NNS_G2D_UNICODE

#define CANVAS_WIDTH        32      // �����`���̕�    (�L�����N�^�P��)
#define CANVAS_HEIGHT       24      // �����`���̍���  (�L�����N�^�P��)
#define CANVAS_LEFT         0       // �����`���̈ʒuX (�L�����N�^�P��)
#define CANVAS_TOP          0       // �����`���̈ʒuY (�L�����N�^�P��)

#define TEXT_HSPACE         1       // ������`�掞�̕����� (�s�N�Z���P��)
#define TEXT_VSPACE         1       // ������`�掞�̍s��   (�s�N�Z���P��)

#define CHARACTER_OFFSET    1       // �g�p����L�����N�^��̊J�n�ԍ�

// DrawText �ł̍����
#define TXT_DRAWTEXT_FLAG_DEFAULT   (NNS_G2D_VERTICALORIGIN_TOP | NNS_G2D_HORIZONTALORIGIN_LEFT | NNS_G2D_HORIZONTALALIGN_LEFT)

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �O���[�o���ϐ���`
 *---------------------------------------------------------------------------*/

NNSG2dFont          gFont;          // �t�H���g
NNSG2dCharCanvas    gCanvas;        // CharCanvas
NNSG2dCharCanvas    gCanvas2;       // CharCanvas
NNSG2dTextCanvas    gTextCanvas;    // TextCanvas
NNSG2dTextCanvas    gTextCanvas2;   // TextCanvas

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/
const int max_colors = 32;

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

// �o�b�N�h���b�v�F(��) + �� + �� + �� + �ő�t�H���g�K���� (32�K���܂�)
static u16 colorPalette[1 + 1 + 1 + 1 + max_colors*5] =
{
    GX_RGB(31, 31, 31), GX_RGB(0, 0, 0), GX_RGB(0, 31, 0), GX_RGB(31, 0, 0)
};

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static u32 LoadFile(void** ppFile, const char* fpath);

/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  �t�@�C�����������Ƀ��[�h���܂��B
                �t�@�C���f�[�^���s�v�ɂȂ����ꍇ��
                CMN_UnloadFile( *ppFile, pAlloc ) �Ńt�@�C���f�[�^��
                ������܂��B

  Arguments:    ppFile: �t�@�C�������[�h�����������A�h���X���󂯎��
                        �o�b�t�@�ւ̃|�C���^�B
                fpath:  ���[�h����t�@�C���̃p�X
                pAlloc: �A���P�[�^�ւ̃|�C���^

  Returns:      ���[�h�����t�@�C���̃t�@�C���T�C�Y��Ԃ��܂��B
                0 �̏ꍇ�̓t�@�C�����[�h�Ɏ��s��������\���܂��B
                ���̏ꍇ *ppFile �̒l�͖����ł��B
 *---------------------------------------------------------------------------*/
static u32 LoadFile(void** ppFile, const char* fpath)
{
    BOOL bSuccess;
    FSFile f;
    u32 length;
    u32 read;

    SDK_NULL_ASSERT( ppFile );
    SDK_NULL_ASSERT( fpath );

    FS_InitFile(&f);

    bSuccess = FS_OpenFile(&f, fpath);
    if( ! bSuccess )
    {
        OS_Warning("file (%s) not found", fpath);
        return 0;
    }

    length = FS_GetLength(&f);
    *ppFile = OS_AllocFromMain(length);
    if( *ppFile == NULL )
    {
        OS_Warning("cant allocate memory for file: %s", fpath);
        return 0;
    }

    read = (u32)FS_ReadFile(&f, *ppFile, (s32)length);
    if( read != length )
    {
        OS_Warning("fail to load file: %s", fpath);
        OS_FreeToMain(*ppFile);
        return 0;
    }

    bSuccess = FS_CloseFile(&f);
    if( ! bSuccess )
    {
        OS_Warning("fail to close file: %s", fpath);
    }

    return length;
}

/*---------------------------------------------------------------------------*
  Name:         InitCanvas

  Description:  ������`��̏����������܂��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitCanvas(void)
{
    GXCharFmt16* pCharBase = (GXCharFmt16*)G2_GetBG1CharPtr();
    GXCharFmt16* pCharBase2 = (GXCharFmt16*)G2_GetBG2CharPtr();
    int cOffset = CHARACTER_OFFSET;

    // �t�H���g��ǂݍ��݂܂�
    {
//      ���̃f���Ɠ����悤�� TXT_LoadFont ���g���Ă��t�H���g��ǂݍ��߂܂��B
//      ���̃f���ł� NNS_G2dFontInitUTF16 ���g���ꍇ�������Ă��܂��B
//        TXT_LoadFont( &gFont, "/data/fontu16.NFTR" );

        void* pFontFile;
        u32 size;

        size = LoadFile( &pFontFile, "/local/tbf_ww_s.NFTR" );
        NNS_G2D_ASSERT( size > 0 );

        NNS_G2dFontInitUTF16(&gFont, pFontFile);
        NNS_G2dPrintFont(&gFont);
    }

    // CharCanvas �̏�����
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pCharBase + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );
    NNS_G2dCharCanvasInitForBG(
        &gCanvas2,
        pCharBase2 + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // TextCanvas�̏�����
    NNS_G2dTextCanvasInit(
        &gTextCanvas,
        &gCanvas,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );
    NNS_G2dTextCanvasInit(
        &gTextCanvas2,
        &gCanvas2,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // �X�N���[����ݒ�
    NNS_G2dMapScrToCharText(
        G2_GetBG1ScrPtr(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        CHARACTER_OFFSET,
        0
    );
    NNS_G2dMapScrToCharText(
        G2_GetBG2ScrPtr(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        CHARACTER_OFFSET,
        0
    );

	// �p���b�g�����[�h
	{
	    // ���[�h�����t�H���g�ɉ����ăJ���[�p���b�g���쐬
	    {
	        const int nColors = MATH_IMin((1 << NNS_G2dFontGetBpp(&gFont)), max_colors);
	        int i;

			// ���n�p�p���b�g 31,20,10, 0
	        for( i = 0; i < nColors; ++i )
	        {
	            int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

	            colorPalette[i+TXT_COLOR_WHITE_BASE_START] = GX_RGB(level, level, level);
	        }

			// ���n�p�p���b�g 0, 10, 20, 31
	        for( i = 0; i < nColors; ++i )
	        {
	            int level = ((i) * (max_colors - 1) / (nColors - 1));

	            colorPalette[i+TXT_COLOR_BLACK_BASE_START] = GX_RGB(level, level, level);
	        }

			// ���F�n�p�p���b�g
	        for( i = 0; i < nColors; ++i )
	        {
	            int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

	            colorPalette[i+TXT_COLOR_FREE_BASE_START] = GX_RGB((int)(level*0.7), (int)(level*0.9), level);
	        }
	    }

	    // �J���[�p���b�g�����[�h
	    GX_LoadBGPltt(colorPalette, 0, sizeof(colorPalette));
	}
}

/*---------------------------------------------------------------------------*
  Name:         UpdateFreePltt

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void UpdateFreePltt(u16 color)
{
	const int nColors = MATH_IMin((1 << NNS_G2dFontGetBpp(&gFont)), max_colors);
	s32 i;

	// �t���[�p���b�g
    for( i = 0; i < nColors; ++i )
    {
	    int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

        colorPalette[i+TXT_COLOR_FREE_BASE_START] = 
			GX_RGB( (int)(level*((color & GX_RGBA_R_MASK)>>GX_RGBA_R_SHIFT)/32.0), 
					(int)(level*((color & GX_RGBA_G_MASK)>>GX_RGBA_G_SHIFT)/32.0), 
					(int)(level*((color & GX_RGBA_B_MASK)>>GX_RGBA_B_SHIFT)/32.0));
    }

    // �J���[�p���b�g�����[�h
    GX_LoadBGPltt(colorPalette, 0, sizeof(colorPalette));
}

/*---------------------------------------------------------------------------*
  Name:         InitFont

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void InitFont(void)
{
    InitCanvas();

	OS_WaitVBlankIntr();
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);
	OS_WaitVBlankIntr();
    NNS_G2dCharCanvasClear(&gCanvas2, TXT_COLOR_WHITE);
	OS_WaitVBlankIntr();
    NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0,   0, 256,  30);

    NNS_G2dTextCanvasDrawText(&gTextCanvas2, 55, 6,
        TXT_COLOR_BLACK_BASE, TXT_DRAWTEXT_FLAG_DEFAULT,
        (const char *)SYSTEM_UPDATER_NAME
    );
}
