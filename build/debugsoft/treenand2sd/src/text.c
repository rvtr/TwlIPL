/********************************************************************

�^�C�g���F
�@�e�L�X�g�u�q�`�l�G�~�����[�V����

********************************************************************/
#include "fontdata.h"
#include "text.h"

/************************************************************************

���O���[�o���ϐ��錾

 ************************************************************************/
/* �e�L�X�g�u�q�`�l�{�� */
TEXT_VRAM_TYPE wText;

/************************************************************************

���v���g�^�C�v�錾

 ************************************************************************/
static void UpdateTextVram( void );
static void Init( void );



/************************************************************************

���V�X�e���֐��Q

 ************************************************************************/

/************************************************************************

�@������

�T�v�F
  �L�����E�p���b�g�̓]��
  �a�f�O�R���g���[��

  ***********************************************************************/
static void Init( void ){

    GX_SetBankForBG(GX_VRAM_BG_128_A);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    G2_BlendNone();                                 // no 2D alpha blending or brightness change
    G2_BG0Mosaic(FALSE);
    GX_SetVisibleWnd(GX_WNDMASK_NONE);              // �E�C���h�E����
    G2_SetBG0Offset(0, 0);                          //�I�t�Z�b�g�O
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);

    //�\�����\�[�X�̍ă��[�h
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,        // 256pix x 256pix text
                     GX_BG_COLORMODE_16,                // use 256 colors mode
                     GX_BG_SCRBASE_0x0000,              // screen base offset + 0x0000 is the address for BG #0 screen
                     GX_BG_CHARBASE_0x04000,            // character base offset + 0x04000 is the address for BG #0 characters
                     GX_BG_EXTPLTT_01                   // use BGExtPltt slot #0 if BGExtPltt is enabled
                     );
    GX_LoadBG0Char(CharData_Sample, 0, sizeof(CharData_Sample));
    GX_LoadBGPltt(PlttData_Sample, 0, sizeof(PlttData_Sample));

}

/************************************************************************

 �e�L�X�g�u�q�`�l�T�[�r�X������

���ӎ����F
  �v���O�����J�n���Ɉ�x�����R�[�����Ă��������B

  ***********************************************************************/
extern void wInitTextVram( void ){

    static BOOL Initialized = FALSE;

    if( !Initialized ){

        /* �����ݒ�������Ȃ� */
        Init();

        // V�u�����N���荞�ݐݒ�
        OS_InitIrqTable();
        OS_SetIrqFunction(OS_IE_V_BLANK, UpdateTextVram);
        (void)OS_EnableIrqMask(OS_IE_V_BLANK);
        (void)OS_EnableIrq();
        (void)GX_VBlankIntr(TRUE);
        (void)OS_EnableInterrupts();

        Initialized = TRUE;
    }
}

/************************************************************************

  �e�L�X�g�u�q�`�l�̍X�V���[�`��

���ӎ����F
�@�����݂̃R�[���o�b�N���[�`���ł��B

  ��ʂ������I�ɍX�V���邽�߂ɒ���I�ɌĂяo���Ă��������B
  �i��L��InitTextVram���g�p����ꍇ�͕s�v�j

  ***********************************************************************/
static void UpdateTextVram( void ){

    MI_CpuCopy16( wText.Map, G2_GetBG0ScrPtr(), sizeof(wText.Map) );
    DC_StoreRange( G2_GetBG0ScrPtr(), sizeof(wText.Map) );

}

