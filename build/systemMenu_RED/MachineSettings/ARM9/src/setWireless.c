/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setFreeSoftBox.c

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

//
// <����>
// �ȉ��̖����̐ݒ�
// ++ ON/OFF�t���O
//
// <��O����>
//

// �e�X�g�\��
#if 1
#define DEBUGPRINT              OS_TPrintf
#else
#define DEBUGPRINT(...)         ((void)0)
#endif

// define data----------------------------------

// �\�t�g�E�F�A�L�[�{�[�hLCD�̈�
#define CLIST_LT_X                          23
#define CLIST_LT_Y                          50
#define CLIST_MARGIN                        14
#define CLIST_KEY_PER_SEGMENT               5
#define CLIST_SEGMENT_INTERVAL              7

// �L�����Z���{�^���̈�
#define CANCEL_BUTTON_TOP_X                 ( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y                 ( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X              ( CANCEL_BUTTON_TOP_X + (8 * 8) )
#define CANCEL_BUTTON_BOTTOM_Y              ( CANCEL_BUTTON_TOP_Y + (2 * 8) )

// OK�{�^���̈�
#define OK_BUTTON_TOP_X                     ( 26 * 8 )
#define OK_BUTTON_TOP_Y                     ( 21 * 8 )
#define OK_BUTTON_BOTTOM_X                  ( OK_BUTTON_TOP_X + (4 * 8) )
#define OK_BUTTON_BOTTOM_Y                  ( OK_BUTTON_TOP_Y + (2 * 8) )

// ON/OFF�{�^���̈�
#define ON_BUTTON_TOP_X                     ( 8 * 8 )
#define ON_BUTTON_TOP_Y                     ( 8 * 8 )
#define ON_BUTTON_BOTTOM_X                  ( ON_BUTTON_TOP_X + (2 * 8) )
#define ON_BUTTON_BOTTOM_Y                  ( ON_BUTTON_TOP_Y + (2 * 8) )
#define OFF_BUTTON_TOP_X                    ( 18 * 8 )
#define OFF_BUTTON_TOP_Y                    (  8 * 8 )
#define OFF_BUTTON_BOTTOM_X                 ( OFF_BUTTON_TOP_X + (3 * 8) )
#define OFF_BUTTON_BOTTOM_Y                 ( OFF_BUTTON_TOP_Y + (2 * 8) )

// UP/DOWN�{�^���̈�
#define UP_BUTTON_TOP_X                     ( 21 * 8 )
#define UP_BUTTON_TOP_Y                     (  7 * 8 )
#define UP_BUTTON_BOTTOM_X                  ( UP_BUTTON_TOP_X + (2 * 8) )
#define UP_BUTTON_BOTTOM_Y                  ( UP_BUTTON_TOP_Y + (2 * 8) )
#define DOWN_BUTTON_TOP_X                   ( 21 * 8 )
#define DOWN_BUTTON_TOP_Y                   ( 13 * 8 )
#define DOWN_BUTTON_BOTTOM_X                ( DOWN_BUTTON_TOP_X + (2 * 8) )
#define DOWN_BUTTON_BOTTOM_Y                ( DOWN_BUTTON_TOP_Y + (2 * 8) )


// ���ڂ̑���
#define MS_FREESOFTBOX_NUMOF_ELEMENTS          1

// �\�t�g�E�F�A�L�[�{�[�h�̃p�����[�^
#define CHAR_LIST_CHAR_NUM                  120
#define CHAR_LIST_MODE_NUM                  3

// ����L�[�R�[�h
#define EOM_                                (u16)0xe050
#define CODE_BUTTON_TOP_                    (u16)0xe051
#define DEL_BUTTON_                         (u16)0xe051
#define SPACE_BUTTON_                       (u16)0xe052
#define VAR_BUTTON1_                        (u16)0xe053
#define VAR_BUTTON2_                        (u16)0xe054
#define OK_BUTTON_                          (u16)0xe055
#define CANCEL_BUTTON_                      (u16)0xe056
#define CODE_BUTTON_BOTTOM_                 (u16)0xe057

#define CHAR_USCORE     L'�Q'
#define KEY_PER_LINE    11

#define KEY_START       109	//�\�t�g�E�F�A�L�[�̃J�[�\���f�t�H���g�ʒu�̓L�����Z���L�[

#define KEY_OK          0xffff
#define KEY_CANCEL      0xfffe
#define KEY_PREVPAGE    0xfffd
#define KEY_SUCCPAGE    0xfffc
#define KEY_ON          0xfffb
#define KEY_OFF         0xfffa
#define KEY_UP          0xfff9
#define KEY_DOWN        0xfff8
#define MULTI_KEY_UP    0xffe0
#define MULTI_KEY_DOWN  0xffd0
#define MASK_MULTI_KEY  0xfff0

// extern data----------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_birth_scr_data[32 * 32];

// function's prototype-------------------------

static void SetAvailableWirelessInit( void );
static int  SetAvailableWirelessMain( void );

// static variable------------------------------
// �ꎞ�I�ɂ����g��Ȃ�����static�ɂ��Ă���̂�
// �����ł��_�C�G�b�g����������Work�����ɂ���Alloc��Free���܂��傤

// ���j���[�Ŏg�p
static u16    sCursorMenu = 0;

// ����̃��[�h�ł̂ݎg�p
static BOOL   sbAvailableWireless = FALSE;

// const data-----------------------------------

// ++ ���C�����j���[

static const u16 *s_pStrSetting[ MS_FREESOFTBOX_NUMOF_ELEMENTS ];          // ���C�����j���[�p�����e�[�u���ւ̃|�C���^���X�g

static const u16 *const s_pStrSettingElemTbl[ MS_FREESOFTBOX_NUMOF_ELEMENTS ][ LCFG_TWL_LANG_CODE_MAX ] = {

    {
        (const u16 *)L"�����̎g�p",
        (const u16 *)L"Available Wireless",
        (const u16 *)L"Available Wireless(F)",
        (const u16 *)L"Available Wireless(G)",
        (const u16 *)L"Available Wireless(I)",
        (const u16 *)L"Available Wireless(S)",
        (const u16 *)L"Available Wireless(C)",
        (const u16 *)L"Available Wireless(K)",
    },
};

// �\���ʒu
static MenuPos s_settingPos[] = {

    { TRUE,  2 * 8,   6 * 8 },
};

// �\���p�����[�^
static const MenuParam s_settingParam = 
{
    MS_FREESOFTBOX_NUMOF_ELEMENTS,
    TXT_COLOR_BLACK,
    TXT_COLOR_GREEN,
    TXT_COLOR_RED,
    &s_settingPos[0],
    (const u16 **)&s_pStrSetting,
};


//=========================================================
//
// �{�^���̃^�b�`����
//
//=========================================================

// �L�����Z���{�^����pSelectSomethingFunc�̎���
static BOOL SelectCancelFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
                         CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_CANCEL;
    return ret;
}

// OK�{�^����pSelectSomethingFunc�̎���
static BOOL SelectOKFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y,
                         OK_BUTTON_BOTTOM_X, OK_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_OK;
    return ret;
}

// ON/OFF�{�^����pSelectSomethingFunc�̎���
static BOOL SelectONFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( ON_BUTTON_TOP_X, ON_BUTTON_TOP_Y,
                         ON_BUTTON_BOTTOM_X, ON_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_ON;
    return ret;
}
static BOOL SelectOFFFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y,
                         OFF_BUTTON_BOTTOM_X, OFF_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_OFF;
    return ret;
}

// �p�b�h�̃L�[�̒����������o(ReadPad()���Ăяo���Ă��郋�[�v���ŌĂ΂��K�v������)
static u16 DetectPadRepeat( void )
{
    static u8 repcount[12];       // �e�L�[������������Ă���t���[����
    u16       rep = 0;
    int       i;

    for( i=0; i < 12; i++ )     // �S���̃L�[�ɂ���
    {
        if( pad.trg & ((u16)(0x0001 << i)) )     // �����ꂽ��J�E���g���n�߂�
        {
            repcount[i] = 1;
        }
        else if( pad.cont & ((u16)(0x0001 << i)) )
        {
            if( repcount[i] > 25 )  // ������ȏ�̃t���[����������Ă����璷��������Ă����Ɣ���
            {
                rep = (u16)(rep | (u16)(0x0001 << i));
                repcount[i] = 25 - 10;
            }
            else
            {
                repcount[i]++;
            }
        }
        else
        {
            repcount[i] = 0;
        }
    }
    return rep;
}

//=========================================================
//
// ���C�����j���[
//
//=========================================================

// �`��
static void DrawWirelessMenuScene( void )
{
    BOOL  bFlg;

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"WIRELESS" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ARETURN" );
    // ���j���[����
    DrawMenu( sCursorMenu, &s_settingParam );
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾���ĕ\��
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // �t���[�\�t�gBOX��
    bFlg = LCFG_TSD_IsAvailableWireless();
    PutStringUTF16( 25*8, s_settingPos[0].y, TXT_UCOLOR_G0, 
                    (bFlg)?(const u16*)L"ON":(const u16*)L"OFF" );     // �l���s���̂Ƃ��ԐF�ŕ\��
}

// ������
void SetWirelessInit( void )
{
    int  i;

    // NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
    for( i=0; i < MS_FREESOFTBOX_NUMOF_ELEMENTS; i++ )
    {
        s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ LCFG_TSD_GetLanguage() ];
    }

    // BG�f�[�^�̃��[�h����
    GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
    GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));

    DrawWirelessMenuScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// ���j���[����Ă΂�郁�C��
int SetWirelessMain( void )
{
    SelectSomethingFunc func[1]={SelectCancelFunc};
    BOOL        tp_select;
    BOOL        tpCommit = FALSE;
    u16         padrep;
    static u16  commit;

    ReadTP();

    padrep = DetectPadRepeat();     // ���������o

    // ���j���[����̍��ڑI��
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) ){                               // �J�[�\���̈ړ�
        if( ++sCursorMenu >= MS_FREESOFTBOX_NUMOF_ELEMENTS ) {
            sCursorMenu = 0;
        }
    }
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
        if( --sCursorMenu & 0x80 ) {
            sCursorMenu = (u16)(MS_FREESOFTBOX_NUMOF_ELEMENTS - 1);
        }
    }
    tp_select = SelectMenuByTP( &sCursorMenu, &s_settingParam );

    // ����{�^���^�b�`
    tpCommit = SelectSomethingByTP( &commit, func, 1 );

    // ���j���[�ւ̕���
    if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {         // ���j���[���ڂւ̕���
        if( (s_settingParam.pos[sCursorMenu]).enable ) {
            switch( sCursorMenu )
            {
                case 0:
                    SetAvailableWirelessInit();
                    g_pNowProcess = SetAvailableWirelessMain;
                break;
            }
        } // if( (s_settingParam.pos[sCursorMenu]).enable )
    } // if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) )
    else if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_CANCEL)) )
    {
        MachineSettingInit();
        return 0;
    }

    // �ĕ`��
    DrawWirelessMenuScene();
    return 0;
}

//=========================================================
//
// ���C�A���X�ʐM��ON/OFF (ON/OFF�X�C�b�`�؂�ւ�)
//
//=========================================================

// �`�揈��
static void DrawSetAvailableWirelessScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"AVAILABLE WIRELESS" );

    if( sbAvailableWireless )
    {
        PutStringUTF16( ON_BUTTON_TOP_X,  ON_BUTTON_TOP_Y,  TXT_COLOR_GREEN, (const u16*)L"ON" );
        PutStringUTF16( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L"OFF" );
    }
    else
    {
        PutStringUTF16( ON_BUTTON_TOP_X,  ON_BUTTON_TOP_Y,  TXT_UCOLOR_G0,   (const u16*)L"ON" );
        PutStringUTF16( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L"OFF" );
    }
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );
}

// ������
static void SetAvailableWirelessInit( void )
{
    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sbAvailableWireless = LCFG_TSD_IsAvailableWireless();

    DrawSetAvailableWirelessScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetAvailableWirelessMain( void )
{
    SelectSomethingFunc func[4]={SelectCancelFunc, SelectOKFunc, SelectONFunc, SelectOFFFunc };
    u16  commit;
    BOOL tp_touch = FALSE;

    ReadTP();
    
    // �L�[�ɂ��I��
    if( (pad.trg & PAD_KEY_LEFT) || (pad.trg & PAD_KEY_RIGHT) )
    {
        sbAvailableWireless = !sbAvailableWireless;
    }

    // �^�b�`�ɂ��I��
    tp_touch = SelectSomethingByTP( &commit, func, 4 );
    if( tp_touch && (commit == KEY_ON) )
    {
        sbAvailableWireless = TRUE;
    }
    else if( tp_touch && (commit == KEY_OFF) )
    {
        sbAvailableWireless = FALSE;
    }

    // ����
    if( (pad.trg & PAD_BUTTON_A) || (tp_touch && (commit == KEY_OK)) )
    {
        LCFG_TSD_SetFlagAvailableWireless( sbAvailableWireless );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        SetWirelessInit();
        g_pNowProcess = SetWirelessMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tp_touch && (commit == KEY_CANCEL)) )
    {
        SetWirelessInit();                   // �L�����Z���̂Ƃ��Z�b�g���Ȃ�
        g_pNowProcess = SetWirelessMain;
        return 0;
    }
    
    DrawSetAvailableWirelessScene();
    return 0;
}

