/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setParentalControl.c

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
// �ȉ��̃y�A�����^���R���g���[�����̐ݒ�
// ++ ���[�e�B���O������ON/OFF
// ++ ���[�e�B���O�N��
// ++ ���[�e�B���O�c��
// ++ �Ïؔԍ�(4���̐���:ascii�Ŋi�[)
// ++ �閧�̎���ID (������e�����肵�Ă��Ȃ��̂�ID�Ŏw��)
// ++ �閧�̎���ւ̉�(���{���[�W�����̂ݕ����������������ɂȂ�:���{��3--32����/���̑�6--64����)
//
// <��O����>
// ++ �Ïؔԍ���ascii�Ȃ̂ŏ�����Ԃł�ascii�ȊO���i�[����Ă���\��������
//    -> �e����ascii�͈̔͊O�̂Ƃ����j���[���/�ݒ��ʂł� '*' ��\��
//       -- �ݒ��ʂł�4�����ׂĂɐ����Ȓl�����͂����܂�OK�{�^���������Ȃ�(�L�����Z���͉\)
// ++ �閧�̎���ւ̉񓚂͓��{���[�W���������������������������̂�
//    ����������(32����)�ȏ�̕����񂪊i�[����Ă���ꍇ������(���̃��[�W����������{�փ��[�W�����ύX�����Ƃ��Ȃ�)
//    -> �ݒ��ʂł�33�����ڈȍ~��؂�̂Ă�32����(�������肬��)�̕�����Ƃ���
//       -- �ݒ��ʂŌ���{�^���������ꂽ�Ƃ���33�����ڈȍ~��0�N���A���ēo�^����
//       -- �ݒ��ʂŃL�����Z�����ꂽ�Ƃ��͌��̕�����̂܂܂ł���
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

// ���y�[�W�{�^���̈�
#define PREVPAGE_BUTTON_TOP_X               ( 2 * 8 )
#define PREVPAGE_BUTTON_TOP_Y               ( 2 * 8 )
#define PREVPAGE_BUTTON_BOTTOM_X            ( PREVPAGE_BUTTON_TOP_X + (2 * 8) + (9 * 8) )
#define PREVPAGE_BUTTON_BOTTOM_Y            ( PREVPAGE_BUTTON_TOP_Y + (2 * 8) )
#define SUCCPAGE_BUTTON_TOP_X               ( 18 * 8 )
#define SUCCPAGE_BUTTON_TOP_Y               (  2 * 8 )
#define SUCCPAGE_BUTTON_BOTTOM_X            ( SUCCPAGE_BUTTON_TOP_X + (2 * 8) + (9 * 8) )
#define SUCCPAGE_BUTTON_BOTTOM_Y            ( SUCCPAGE_BUTTON_TOP_Y + (2 * 8) )

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
#define UP_BUTTON_TOP_X                     ( 8 * 8 )
#define UP_BUTTON_TOP_Y                     ( 6 * 8 )
#define UP_BUTTON_BOTTOM_X                  ( UP_BUTTON_TOP_X + (2 * 8) + (2 * 8) )
#define UP_BUTTON_BOTTOM_Y                  ( UP_BUTTON_TOP_Y + (2 * 8) )
#define DOWN_BUTTON_TOP_X                   (  8 * 8 )
#define DOWN_BUTTON_TOP_Y                   ( 14 * 8 )
#define DOWN_BUTTON_BOTTOM_X                ( DOWN_BUTTON_TOP_X + (2 * 8) + (4 * 8) )
#define DOWN_BUTTON_BOTTOM_Y                ( DOWN_BUTTON_TOP_Y + (2 * 8) )

// ������UP/DOWN�{�^���̈�
#define MULTI_UP_BUTTON_OFFSET_X            ( 6 * 8 )   // ���[����̃X�y�[�X
#define MULTI_UP_BUTTON_OFFSET_Y            ( 6 * 8 )
#define MULTI_UP_BUTTON_WIDTH_X             ( 2 * 8 )   // �{�^����
#define MULTI_UP_BUTTON_WIDTH_Y             ( 2 * 8 )
#define MULTI_UP_BUTTON_INTERVAL_X          ( 4 * 8 )   // ���E�̃{�^���Ԋu(���ɕ��ׂ�̂�X�����ł悢)
#define MULTI_DOWN_BUTTON_OFFSET_X          (MULTI_UP_BUTTON_OFFSET_X)
#define MULTI_DOWN_BUTTON_OFFSET_Y          (14 * 8 )
#define MULTI_DOWN_BUTTON_WIDTH_X           ( 2 * 8 )
#define MULTI_DOWN_BUTTON_WIDTH_Y           ( 2 * 8 )
#define MULTI_DOWN_BUTTON_INTERVAL_X        (MULTI_UP_BUTTON_INTERVAL_X)

// RESTRICT/NOT RESTRICT�{�^���̈�
#define RESTRICT_BUTTON_TOP_X               (  8 * 8 )
#define RESTRICT_BUTTON_TOP_Y               ( 10 * 8 )
#define RESTRICT_BUTTON_BOTTOM_X            ( RESTRICT_BUTTON_TOP_X + (8 * 8) )
#define RESTRICT_BUTTON_BOTTOM_Y            ( RESTRICT_BUTTON_TOP_Y + (2 * 8) )
#define NOTRESTRICT_BUTTON_TOP_X            (  8 * 8 )
#define NOTRESTRICT_BUTTON_TOP_Y            (  6 * 8 )
#define NOTRESTRICT_BUTTON_BOTTOM_X         ( NOTRESTRICT_BUTTON_TOP_X + (12 * 8) )
#define NOTRESTRICT_BUTTON_BOTTOM_Y         ( NOTRESTRICT_BUTTON_TOP_Y + ( 2 * 8) )


// �y�[�W��
#define MS_PARENTAL_NUMOF_PAGES             3

// ���ڂ̑���
#ifdef BROADON_UI
#define MS_PARENTAL_NUMOF_ELEMENTS          10
#else   // BROADON_UI
#define MS_PARENTAL_NUMOF_ELEMENTS          7
#endif  // BROADON_UI

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
#define KEY_RESTRICT    0xfff7
#define KEY_NOTRESTRICT 0xfff6
#define MULTI_KEY_UP    0xffe0
#define MULTI_KEY_DOWN  0xffd0
#define MASK_MULTI_KEY  0xfff0

// ���s���K�v�ȂƂ��̈�s�ł̕\��������/������s���邩
#define  MS_PARENTAL_ANS_COL            16
#define  MS_PARENTAL_ANS_NUMOF_ROWS     (LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX / MS_PARENTAL_ANS_COL - 1)

// �\�t�g�E�F�A�L�[�{�[�h���g�p���郂�[�h
typedef enum eUseSoftKey
{
    USE_SOFTKEY_COMMON,     // �s�g�p
    USE_SOFTKEY_ANSWER      // �閧�̎���ɑ΂����
}eUseSoftKey;

// extern data----------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_birth_scr_data[32 * 32];

// function's prototype-------------------------

static void SetRatingOgnInit( void );
static int  SetRatingOgnMain( void );
static void SetRatingLockInit( void );
static int  SetRatingLockMain( void );
static void SetRatingAgeInit( void );
static int  SetRatingAgeMain( void );
static void SetSecretQuestionIDInit( void );
static int  SetSecretQuestionIDMain( void );
static void SetAnswerInit( void );
static int  SetAnswerMain( void );
static void SetPasswordInit( void );
static int  SetPasswordMain( void );
static u8   MY_StrLen( const u16 *pStr );
static void SetDrawnPageElement( void );
static void SetBroadOnSettingInit( void );
static int  SetBroadOnSettingMain( void );

// static variable------------------------------
// �ꎞ�I�ɂ����g��Ȃ�����static�ɂ��Ă���̂�
// �����ł��_�C�G�b�g����������Work�����ɂ���Alloc��Free���܂��傤

// ���j���[�Ŏg�p
static u16    sCursorMenu = 0;

// ���p
static int    s_char_mode = 0;      // �\�t�g�E�F�A�L�[�{�[�h�̕������(����/�J�i/�p��)
static u16    s_key_csr = 0;        // ���݂̃L�[�ʒu

// ����̃��[�h�ł̂ݎg�p
static u8     sCurrentPage = 0;             // �y�[�W�ԍ�
static BOOL   sbInitPage = TRUE;            // ���j���[�ɖ߂�Ƃ���1�y�[�W�ڂɖ߂����ǂ���
static BOOL   sbRatingLock = FALSE;         // ���[�e�B���O����
static u8     sRatingAge=0;                 // ���[�e�B���O�N��
static u16    sRatingOgnFirstIndex=0;       // �c�̃��X�g�̃I�t�Z�b�g(�e�[�u���̉��Ԗڂ���c�̃��X�g�Ƃ��Ďg�p���邩)
static u16    sCursorRatingOgn=0;           // �I�t�Z�b�g����̈ʒu(���X�g���牽�Ԗڂ̒c�̂��I������Ă��邩)
static u8     sRegion=0;                    // �{�̂̃��[�W����
static u16    spBufAnswer[ LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 ];    // �閧�̎���ɑ΂����
static u8     sCurrentLenAnswer;
static u8     sBufSizeAnswer;
static u8     sMinLenAnswer;                                                // -- ��������
static u8     sMaxLenAnswer;
static u8     spBufPassword[ LCFG_TWL_PCTL_PASSWORD_LENGTH + 1 ];           // �p�X���[�h
static u16    sCursorPassword=0;
static BOOL   sbValidPassword=FALSE;
static u8     sSecretQuestionID=0;
static BOOL   sbEnableOKButton = FALSE;
static u8     sBroadOnMenu = 0;
static BOOL   sbBroadOnSetting = FALSE;

// const data-----------------------------------

// ++ ���C�����j���[

static const u16 *s_pStrSetting[ MS_PARENTAL_NUMOF_ELEMENTS ];          // ���C�����j���[�p�����e�[�u���ւ̃|�C���^���X�g

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM];      // �\�t�g�E�F�A�L�[�{�[�h�̕����z��

static const u16 *const s_pStrSettingElemTbl[ MS_PARENTAL_NUMOF_ELEMENTS ][ LCFG_TWL_LANG_CODE_MAX ] = {

    // �y�[�W1
    {
        (const u16 *)L"�y�A�����^���R���g���[��",
        (const u16 *)L"PARENTAL CONTROL",
        (const u16 *)L"PARENTAL CONTROL(F)",
        (const u16 *)L"PARENTAL CONTROL(G)",
        (const u16 *)L"PARENTAL CONTROL(I)",
        (const u16 *)L"PARENTAL CONTROL(S)",
        (const u16 *)L"PARENTAL CONTROL(C)",
        (const u16 *)L"PARENTAL CONTROL(K)",
    },
    {
        (const u16 *)L"���[�e�B���O�Ώ۔N��",
        (const u16 *)L"RATING TARGET AGE",
        (const u16 *)L"RATING TARGET AGE(F)",
        (const u16 *)L"RATING TARGET AGE(G)",
        (const u16 *)L"RATING TARGET AGE(I)",
        (const u16 *)L"RATING TARGET AGE(S)",
        (const u16 *)L"RATING TARGET AGE(C)",
        (const u16 *)L"RATING TARGET AGE(K)",
    },
    {
        (const u16 *)L"���[�e�B���O�c��",
        (const u16 *)L"RATING ORGANIZATION",
        (const u16 *)L"RATING ORGANIZATION(F)",
        (const u16 *)L"RATING ORGANIZATION(G)",
        (const u16 *)L"RATING ORGANIZATION(I)",
        (const u16 *)L"RATING ORGANIZATION(S)",
        (const u16 *)L"RATING ORGANIZATION(C)",
        (const u16 *)L"RATING ORGANIZATION(K)",
    },
//    {
//        (const u16 *)L"Wi-Fi����",
//        (const u16 *)L"Wi-Fi LOCK",
//        (const u16 *)L"Wi-Fi LOCK(F)",
//        (const u16 *)L"Wi-Fi LOCK(G)",
//        (const u16 *)L"Wi-Fi LOCK(I)",
//        (const u16 *)L"Wi-Fi LOCK(S)",
//        (const u16 *)L"Wi-Fi LOCK(C)",
//        (const u16 *)L"Wi-Fi LOCK(K)",
//    },
//    {
//        (const u16 *)L"���ꂿ�����ʐM����",
//        (const u16 *)L"Pass-by LOCK",
//        (const u16 *)L"Pass-by LOCK(F)",
//        (const u16 *)L"Pass-by LOCK(G)",
//        (const u16 *)L"Pass-by LOCK(I)",
//        (const u16 *)L"Pass-by LOCK(S)",
//        (const u16 *)L"Pass-by LOCK(C)",
//        (const u16 *)L"Pass-by LOCK(K)",
//    },
    
    // �y�[�W2
    {
        (const u16 *)L"�Ïؔԍ�",
        (const u16 *)L"PASSWORD",
        (const u16 *)L"PASSWORD(F)",
        (const u16 *)L"PASSWORD(G)",
        (const u16 *)L"PASSWORD(I)",
        (const u16 *)L"PASSWORD(S)",
        (const u16 *)L"PASSWORD(C)",
        (const u16 *)L"PASSWORD(K)",
    },
    {
        (const u16 *)L"�閧�̎���ID",
        (const u16 *)L"SECRET QUESTION ID",
        (const u16 *)L"SECRET QUESTION ID(F)",
        (const u16 *)L"SECRET QUESTION ID(G)",
        (const u16 *)L"SECRET QUESTION ID(I)",
        (const u16 *)L"SECRET QUESTION ID(S)",
        (const u16 *)L"SECRET QUESTION ID(C)",
        (const u16 *)L"SECRET QUESTION ID(K)",
    },
    {
        (const u16 *)L"�閧�̎���ւ̉�",
        (const u16 *)L"ANSWER FOR SECRET QUESTION",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(F)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(G)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(I)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(S)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(C)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(K)",
    },
    
    // �y�[�W3
#ifdef BROADON_UI
    {
        (const u16 *)L"�s�N�g�`���b�g�N��",
        (const u16 *)L"PictoChat Boot",
        (const u16 *)L"PictoChat Boot(F)",
        (const u16 *)L"PictoChat Boot(G)",
        (const u16 *)L"PictoChat Boot(I)",
        (const u16 *)L"PictoChat Boot(S)",
        (const u16 *)L"PictoChat Boot(C)",
        (const u16 *)L"PictoChat Boot(K)",
    },
    {
        (const u16 *)L"DS�_�E�����[�h�N��",
        (const u16 *)L"DSDownload Boot",
        (const u16 *)L"DSDownload Boot(F)",
        (const u16 *)L"DSDownload Boot(G)",
        (const u16 *)L"DSDownload Boot(I)",
        (const u16 *)L"DSDownload Boot(S)",
        (const u16 *)L"DSDownload Boot(C)",
        (const u16 *)L"DSDownload Boot(K)",
    },
    {
        (const u16 *)L"�u���E�U�N��",
        (const u16 *)L"Browser Boot",
        (const u16 *)L"Browser Boot(F)",
        (const u16 *)L"Browser Boot(G)",
        (const u16 *)L"Browser Boot(I)",
        (const u16 *)L"Browser Boot(S)",
        (const u16 *)L"Browser Boot(C)",
        (const u16 *)L"Browser Boot(K)",
    },
    {
        (const u16 *)L"Wii�|�C���g",
        (const u16 *)L"Wii Point",
        (const u16 *)L"Wii Point(F)",
        (const u16 *)L"Wii Point(G)",
        (const u16 *)L"Wii Point(I)",
        (const u16 *)L"Wii Point(S)",
        (const u16 *)L"Wii Point(C)",
        (const u16 *)L"Wii Point(K)",
    },
    {
        (const u16 *)L"�ʐ^����",
        (const u16 *)L"Photo Exchange",
        (const u16 *)L"Photo Exchange(F)",
        (const u16 *)L"Photo Exchange(G)",
        (const u16 *)L"Photo Exchange(I)",
        (const u16 *)L"Photo Exchange(S)",
        (const u16 *)L"Photo Exchange(C)",
        (const u16 *)L"Photo Exchange(K)",
    },
    {
        (const u16 *)L"UGC",
        (const u16 *)L"UGC",
        (const u16 *)L"UGC(F)",
        (const u16 *)L"UGC(G)",
        (const u16 *)L"UGC(I)",
        (const u16 *)L"UGC(S)",
        (const u16 *)L"UGC(C)",
        (const u16 *)L"UGC(K)",
    },
#else  // BROADON_UI
    {
        (const u16 *)L"���̑��̐ݒ�(��)",
        (const u16 *)L"OTHER SETTINGS(None)",
        (const u16 *)L"OTHER SETTINGS(None)(F)",
        (const u16 *)L"OTHER SETTINGS(None)(G)",
        (const u16 *)L"OTHER SETTINGS(None)(I)",
        (const u16 *)L"OTHER SETTINGS(None)(S)",
        (const u16 *)L"OTHER SETTINGS(None)(C)",
        (const u16 *)L"OTHER SETTINGS(None)(K)",
    },
#endif // BROADON_UI
};

// �\���ʒu
static MenuPos s_settingPos[] = {
    // �y�[�W1
    { TRUE,  4 * 8,   6 * 8 },
    { TRUE,  4 * 8,   8 * 8 },
    { TRUE,  4 * 8,  10 * 8 },
//    { TRUE,  4 * 8,  12 * 8 },
//    { TRUE,  4 * 8,  14 * 8 },

    // �y�[�W2
    { TRUE,  3 * 8,   6 * 8 },
    { TRUE,  3 * 8,   8 * 8 },
    { TRUE,  3 * 8,  10 * 8 },

    // �y�[�W3
#ifdef BROADON_UI
    { TRUE,  2 * 8,   6 * 8 },
    { TRUE,  2 * 8,   8 * 8 },
    { TRUE,  2 * 8,  10 * 8 },
    { TRUE,  2 * 8,  12 * 8 },
#else  // BROADON_UI
    { FALSE,  4 * 8,   6 * 8 },
#endif // BROADON_UI
};

// �e�y�[�W�̕\�����ڐ�
static const int sNumOfPageElements[] =
{
    3,
    3,
#ifdef BROADON_UI
    4,
#else  // BROADON_UI
    1,
#endif // BROADON_UI
};

// �\���p�����[�^
static MenuParam s_settingParam = 
{
    3,
    TXT_COLOR_BLACK,
    TXT_COLOR_GREEN,
    TXT_COLOR_RED,
    &s_settingPos[0],
    (const u16 **)&s_pStrSetting,
};

// ++ �\�t�g�E�F�A�L�[�{�[�h

static const u16 *str_button_char[CHAR_LIST_MODE_NUM] = 
{
    L"����",
    L"�J�i",
    L"�p��",
};

static u16 next_char_mode[CHAR_LIST_MODE_NUM-1];

static const u16  str_button_del[]    = L"�ADEL";
static const u16  str_button_space[]  = L"SPACE";
static const u16  str_button_ok[]     = L"OK";
static const u16  str_button_cancel[] = L"CANCEL";

static const u16 *str_button[] = 
{
    (const u16 *)str_button_del,
    (const u16 *)str_button_space,
    NULL,
    NULL,
    (const u16 *)str_button_ok,
    (const u16 *)str_button_cancel,
};

// ++ ���[�e�B���O�c�̂̑I��

// ���[�W�����̕�����
static const u16 *sppRegionCharTable[] =
{
    (const u16*)L"JAPAN",
    (const u16*)L"AMERICA",
    (const u16*)L"EUROPE",
    (const u16*)L"AUSTRALIA",
    (const u16*)L"CHINA",
    (const u16*)L"KOREA",
};

// ���[�e�B���O���X�g�̃I�t�Z�b�g(���X�g�̐擪��UNDEFINED�����邽��)
static const u16 sRatingOgnIndexOffset = 0;     // 1�����X�g�̐擪index

// ���[�e�B���O�c��
static const u16 *sppRatingOgnCharTable[] =
{
    (const u16*)L"CERO",            //LCFG_TWL_PCTL_OGN_CERO        = 0,  // ���{
    (const u16*)L"ESRB",            //LCFG_TWL_PCTL_OGN_ESRB        = 1,  // �A�����J
    (const u16*)L"RESERVED2",       //LCFG_TWL_PCTL_OGN_RESERVED2   = 2,  // 
    (const u16*)L"USK",             //LCFG_TWL_PCTL_OGN_USK         = 3,  // �h�C�c
    (const u16*)L"PEGI GENERAL",    //LCFG_TWL_PCTL_OGN_PEGI_GEN    = 4,  // ���B
    (const u16*)L"RESERVED5",       //LCFG_TWL_PCTL_OGN_RESERVED5   = 5,  // 
    (const u16*)L"PEGI PORTUGAL",   //LCFG_TWL_PCTL_OGN_PEGI_PRT    = 6,  // �|���g�K��
    (const u16*)L"PEGI BBFC",       //LCFG_TWL_PCTL_OGN_PEGI_BBFC   = 7,  // �C�M���X
    (const u16*)L"OFLC",            //LCFG_TWL_PCTL_OGN_OFLC        = 8,  // �I�[�X�g�����A�A�j���[�W�[�����h
    (const u16*)L"GRB",             //LCFG_TWL_PCTL_OGN_GRB         = 9,  // �؍�
};

// �\���ʒu�̃��X�g
static MenuPos spRatingOgnPosTable[] =
{
    { TRUE, 10 * 8,  8 * 8 },   // ���{
    { TRUE, 10 * 8,  8 * 8 },   // �A�����J
    { TRUE, 10 * 8,  8 * 8 },
    { TRUE, 10 * 8, 10 * 8 },
    { TRUE, 10 * 8, 12 * 8 },   // ���B
    { TRUE, 10 * 8, 14 * 8 },
    { TRUE, 10 * 8, 16 * 8 },
    { TRUE, 10 * 8, 18 * 8 },
    { TRUE, 10 * 8,  8 * 8 },   // �I�[�X�g�����A
    { TRUE, 10 * 8,  8 * 8 },   // �؍�
};

// ���[�W�������Ƃ̒c�̐�
static const int spNumOfRatingOgnsTable[] =
{
    1,  // ���{
    1,  // �A�����J
    6,  // ���B
    1,  // �I�[�X�g�����A
    0,  // ����
    1,  // �؍�
};

static const int sIndexFirstOgnTable[] = {
	0,
	1,
	2,
	8,
	0,
	9,
};

// �����ɕ\��������(�I����������)���X�g���R�s�[����
static const u16 *sppRatingOgnCharList[ LCFG_TWL_PCTL_OGN_MAX ];

// �\��/�I���֐��ɓn���p�����[�^
static MenuParam sRatingOgnMenuParam =
{
    1,
    TXT_COLOR_BLACK,
    TXT_COLOR_GREEN,
    TXT_COLOR_RED,
    &spRatingOgnPosTable[0],
    (const u16 **)&sppRatingOgnCharList,
};

// ++ BroadOn�p�̐ݒ荀��
enum
{
    MS_BROADON_PICTOCHAT      = 0,
    MS_BROADON_DS_DOWNLOAD    = 1,
    MS_BROADON_BROWSER        = 2,
    MS_BROADON_WIIPOINT       = 3,
    MS_BROADON_PHOTO_EXCHANGE = 4,
    MS_BROADON_UGC            = 5
};

//=========================================================
//
// �{�^���̃^�b�`����
//
//=========================================================

static void SetSoftKeyboardButton(int mode)
{
    int l;
    int count = 0;
    for(l=0; l<CHAR_LIST_MODE_NUM ;l++)
    {
        if(l != mode){
            str_button[2+count]=str_button_char[l];
            next_char_mode[count] = (u16)l;
            count++;
        }
    }
    s_char_mode = mode;
}

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

// ���y�[�W��pSelectSomethingFunc�̎���
static BOOL SelectPrevPageFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( PREVPAGE_BUTTON_TOP_X, PREVPAGE_BUTTON_TOP_Y,
                         PREVPAGE_BUTTON_BOTTOM_X, PREVPAGE_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_PREVPAGE;
    return ret;
}
static BOOL SelectSuccPageFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( SUCCPAGE_BUTTON_TOP_X, SUCCPAGE_BUTTON_TOP_Y,
                         SUCCPAGE_BUTTON_BOTTOM_X, SUCCPAGE_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_SUCCPAGE;
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
/*
// UP/DOWN�{�^����pSelectSomethingFunc�̎���
static BOOL SelectUPFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( UP_BUTTON_TOP_X, UP_BUTTON_TOP_Y,
                         UP_BUTTON_BOTTOM_X, UP_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_UP;
    return ret;
}
static BOOL SelectDOWNFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( DOWN_BUTTON_TOP_X, DOWN_BUTTON_TOP_Y,
                         DOWN_BUTTON_BOTTOM_X, DOWN_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_DOWN;
    return ret;
}
*/
// UP/DOWN�{�^���̒������ƃg���K�����o����
static BOOL DetectTouchUD( u16 *csr )
{
    BOOL         curr[2]  = {FALSE, FALSE};     // 0:UP/1:DOWN
    static BOOL  prev[2]  = {FALSE, FALSE};     // �g���K���o�̂��߂ɑO�̏�Ԃ��L��������
    BOOL         trg[2]   = {FALSE, FALSE};
    BOOL         rep[2]   = {FALSE, FALSE};     // ������
    static u8    count[2] = {0, 0};             // ���t���[���A���ŉ�����Ă��邩
    BOOL         ret = FALSE;
    u16          i;

    for( i=0; i < 2; i++ )
    {
        switch(i)
        {
            case 0:
                curr[i] = WithinRangeTP( UP_BUTTON_TOP_X, UP_BUTTON_TOP_Y,
                                         UP_BUTTON_BOTTOM_X, UP_BUTTON_BOTTOM_Y, &tpd.disp );
            break;
            case 1:
                curr[i] = WithinRangeTP( DOWN_BUTTON_TOP_X, DOWN_BUTTON_TOP_Y,
                                         DOWN_BUTTON_BOTTOM_X, DOWN_BUTTON_BOTTOM_Y, &tpd.disp );
            break;
            default:
            break;
        }

        // �͂��߂ĉ����ꂽ���ǂ���
        if( !prev[i] && curr[i] )
        {
            trg[i] = TRUE;
        }
        // �������J�E���g
        if( curr[i] )
        {
            if( trg[i] )
            {
                count[i] = 1;
            }
            else if( count[i] > 25 )
            {
                count[i] = 25 - 10;
                rep[i] = TRUE;
            }
            else
            {
                (count[i])++;
            }
        }
        else        // ������Ă��Ȃ��Ƃ�
        {
            count[i] = 0;
        }
        prev[i] = curr[i];        // ��Ԃ��L��
    }

    if(trg[0] || rep[0])
    {
        *csr = KEY_UP;
        ret  = TRUE;
    }
    else if(trg[1] || rep[1])
    {
        *csr = KEY_DOWN;
        ret  = TRUE;
    }
    else
    {
        ret  = FALSE;
    }
    return ret;
}
/*
// UP/DOWN�����{�^����pSelectSomethingFunc�̎���
static BOOL SelectMultiUPDOWNFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    int  topX, topY, bottomX, bottomY;
    int  i;
    for( i=0; i < 4; i++ )      // �����珇��
    {
        // UP�{�^��
        topX    = MULTI_UP_BUTTON_OFFSET_X + (MULTI_UP_BUTTON_WIDTH_X * i) + (MULTI_UP_BUTTON_INTERVAL_X * i);
        bottomX = topX + MULTI_UP_BUTTON_WIDTH_X;
        topY    = MULTI_UP_BUTTON_OFFSET_Y;
        bottomY = topY + MULTI_UP_BUTTON_WIDTH_Y;
        ret = WithinRangeTP( topX, topY, bottomX, bottomY, tgt );
        if(ret)
        {
            *csr = (u16)(MULTI_KEY_UP | i);        // �{�^���̎�� + �ԍ�
            break;
        }

        // DOWN�{�^��
        topX    = MULTI_DOWN_BUTTON_OFFSET_X + (MULTI_DOWN_BUTTON_WIDTH_X * i) + (MULTI_DOWN_BUTTON_INTERVAL_X * i);
        bottomX = topX + MULTI_DOWN_BUTTON_WIDTH_X;
        topY    = MULTI_DOWN_BUTTON_OFFSET_Y;
        bottomY = topY + MULTI_DOWN_BUTTON_WIDTH_Y;
        ret = WithinRangeTP( topX, topY, bottomX, bottomY, tgt );
        if(ret)
        {
            *csr = (u16)(MULTI_KEY_DOWN | i);
            break;
        }
    }
    return ret;
}
*/
// UP/DOWN�����{�^���̒������ƃg���K�����o����
static BOOL DetectTouchMultiUD( u16 *csr )
{
    BOOL         curr[8]  = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};     // 0--3:UP/4--8:DOWN
    static BOOL  prev[8]  = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    BOOL         trg[8]   = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    BOOL         rep[8]   = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    static u8    count[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    BOOL         ret = FALSE;
    int          topX, topY, bottomX, bottomY;
    int          i;

    for( i=0; i < 8; i++ )
    {
        if( i < 4 )
        {
            // UP�{�^��
            topX     = MULTI_UP_BUTTON_OFFSET_X + (MULTI_UP_BUTTON_WIDTH_X * i) + (MULTI_UP_BUTTON_INTERVAL_X * i);
            bottomX  = topX + MULTI_UP_BUTTON_WIDTH_X;
            topY     = MULTI_UP_BUTTON_OFFSET_Y;
            bottomY  = topY + MULTI_UP_BUTTON_WIDTH_Y;
            curr[i] = WithinRangeTP( topX, topY, bottomX, bottomY, &tpd.disp );
        }
        else
        {
            int  pos = i - 4;

            // DOWN�{�^��
            topX     = MULTI_DOWN_BUTTON_OFFSET_X + (MULTI_DOWN_BUTTON_WIDTH_X * pos) + (MULTI_DOWN_BUTTON_INTERVAL_X * pos);
            bottomX  = topX + MULTI_DOWN_BUTTON_WIDTH_X;
            topY     = MULTI_DOWN_BUTTON_OFFSET_Y;
            bottomY  = topY + MULTI_DOWN_BUTTON_WIDTH_Y;
            curr[i] = WithinRangeTP( topX, topY, bottomX, bottomY, &tpd.disp );
        }

        // �͂��߂ĉ����ꂽ���ǂ���
        if( !prev[i] && curr[i] )
        {
            trg[i] = TRUE;
        }
        // �������J�E���g
        if( curr[i] )
        {
            if( trg[i] )
            {
                count[i] = 1;
            }
            else if( count[i] > 25 )
            {
                count[i] = 25 - 10;
                rep[i] = TRUE;
            }
            else
            {
                (count[i])++;
            }
        }
        else        // ������Ă��Ȃ��Ƃ�
        {
            count[i] = 0;
        }
        prev[i] = curr[i];        // ��Ԃ��L��
    }

    ret = FALSE;
    for( i=0; i < 8; i++ )
    {
        if( (i<4) && (trg[i] || rep[i]) )
        {
            *csr = (u16)(MULTI_KEY_UP | i);
            ret  = TRUE;
            break;
        }
        else if( (i>=4) && (trg[i] || rep[i]) )
        {
            *csr = (u16)(MULTI_KEY_DOWN | (i-4));
            ret  = TRUE;
            break;
        }
    }
    return ret;
}

// RESTRICT/NOT RESTRICT�{�^����pSelectSomethingFunc�̎���
static BOOL SelectRESTRICTFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( RESTRICT_BUTTON_TOP_X, RESTRICT_BUTTON_TOP_Y,
                         RESTRICT_BUTTON_BOTTOM_X, RESTRICT_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_RESTRICT;
    return ret;
}
static BOOL SelectNOTRESTRICTFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( NOTRESTRICT_BUTTON_TOP_X, NOTRESTRICT_BUTTON_TOP_Y,
                         NOTRESTRICT_BUTTON_BOTTOM_X, NOTRESTRICT_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_NOTRESTRICT;
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
static void DrawParentalControlMenuScene( void )
{
    u16   utf16Button[2];
    int   x,y,i;
    u16   buf[2] = {0,0};

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PARENTAL CONTROL" );
    PrintfSJIS( 25*8, 0, TXT_COLOR_BLUE, "%d�^3", sCurrentPage+1 );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ARETURN" );
    if( sCurrentPage > 0 )
    {
        utf16Button[0] = 0xE004;
        utf16Button[1] = 0;
        PutStringUTF16( PREVPAGE_BUTTON_TOP_X,     PREVPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)utf16Button );
        PutStringUTF16( PREVPAGE_BUTTON_TOP_X+2*8, PREVPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)L"Prev Page" );
    }
    if( sCurrentPage < (MS_PARENTAL_NUMOF_PAGES-1) )
    {
        utf16Button[0] = 0xE005;
        utf16Button[1] = 0;
        PutStringUTF16( SUCCPAGE_BUTTON_TOP_X,     SUCCPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)L"Next Page" );
        PutStringUTF16( SUCCPAGE_BUTTON_TOP_X+9*8, SUCCPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)utf16Button );
    }
    // ���j���[����
    DrawMenu( sCursorMenu, &s_settingParam );
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾���ĕ\��
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    switch( sCurrentPage )      // �y�[�W�ʂɕ\�����ڂ��ς��
    {
        case 0:
            // �y�A�����^���R���g���[��������ON/OFF
            PutStringUTF16( 23*8, s_settingPos[0].y, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsSetParentalControl() ? L"ON" : L"OFF" );
            // ���[�e�B���O�N��
            PrintfSJIS( 23*8, s_settingPos[1].y, TXT_UCOLOR_G0, "%d", LCFG_TSD_GetPCTLRatingAge() );
            // ���[�e�B���O�c��
            PutStringUTF16( 6*8, 2*8+s_settingPos[2].y, TXT_UCOLOR_G0, 
                            sppRatingOgnCharTable[ LCFG_TSD_GetPCTLRatingOgn() ] );
        break;
        
        case 1:
            // �Ïؔԍ�
            for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
            {
                const char *pPassword = LCFG_TSD_GetPCTLPasswordPtr();
                if( ('0' <= pPassword[i]) && (pPassword[i] <= '9') )
                {
                    PrintfSJIS( 23*8+i*8, s_settingPos[3].y, TXT_UCOLOR_G0, "%c", pPassword[i] );
                }
                else
                {
                    PrintfSJIS( 23*8+i*8, s_settingPos[3].y, TXT_UCOLOR_G0, "*" );
                }
            }
            //PrintfSJIS( 23*8, s_settingPos[3].y, TXT_UCOLOR_G0, "%s", LCFG_TSD_GetPCTLPasswordPtr() );
            // �閧�̎���ID
            PrintfSJIS( 23*8, s_settingPos[4].y, TXT_UCOLOR_G0, "%d", LCFG_TSD_GetPCTLSecretQuestionID() );
            // �閧�̎���ւ̉�
            for( y=0; y < (MS_PARENTAL_ANS_NUMOF_ROWS+1); y++ )             // 1�s���Ƃɕ\��
            {
                const u16 *pAnswer = LCFG_TSD_GetPCTLSecretAnswerPtr();
                for( x=0; x < MS_PARENTAL_ANS_COL; x++ )                    // 1�������\��(���p/�S�p�������Ă����Ԋu�ŕ\������������)
                {
                    buf[0] = pAnswer[MS_PARENTAL_ANS_COL*y + x];
                    buf[1] = 0;
                    PutStringUTF16( 4*8 + 12*x, s_settingPos[5].y + 2*8*y + 2*8, TXT_UCOLOR_G0, buf );
                }
            }
        break;
        
        case 2:
#ifdef BROADON_UI
            PutStringUTF16( 17*8, 6*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictWiiPoint() ? L"Restricted" : L"Not Restricted" );
            PutStringUTF16( 17*8, 8*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictBrowserBoot() ? L"Restricted" : L"Not Restricted" );
            PutStringUTF16( 17*8, 10*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictPictoChatBoot() ? L"Restricted" : L"Not Restricted" );
            PutStringUTF16( 17*8, 12*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictNintendoSpot() ? L"Restricted" : L"Not Restricted" );
#endif // BROADON_UI
        break;
    }
}

// �\�����ڂ��y�[�W���ƂɃZ�b�g
static void SetDrawnPageElement( void )
{
    int  index;
    int  page;
    int  i;
    
    // �擪�̍��ڂ�index�𒲂ׂ�
    index = 0;
    for( page=0; page < sCurrentPage; page++ )
    {
        index = index + sNumOfPageElements[page];
    }

    //DEBUGPRINT( "current page is %d\n", sCurrentPage );
    //DEBUGPRINT( "index is %d\n", index );
    
    // NITRO�ݒ�f�[�^��language�ɉ��������C�����j���[�\������̐؂�ւ�
    s_settingParam.num = sNumOfPageElements[ sCurrentPage ];
    s_settingParam.pos = &(s_settingPos[ index ]);
    for( i=0; i < sNumOfPageElements[ sCurrentPage ]; i++,index++ )
    {
        s_pStrSetting[ i ] = s_pStrSettingElemTbl[ index ][ LCFG_TSD_GetLanguage() ];
    }
}

// ������
void SetParentalControlInit( void )
{
	// �y�[�W�̏�����
    if( sbInitPage )            // ���ʂ�FALSE���w�肳�ꂽ�Ƃ������y�[�W�����̂܂܂ɂ���
    {
        sCurrentPage = 0;
    }
    sbInitPage = TRUE;
    SetDrawnPageElement();

    // BG�f�[�^�̃��[�h����
    GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
    GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));

    DrawParentalControlMenuScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// ���j���[����Ă΂�郁�C��
int SetParentalControlMain( void )
{
    SelectSomethingFunc func[3]={SelectCancelFunc, SelectPrevPageFunc, SelectSuccPageFunc};
    BOOL        tp_select;
    BOOL        tpCommit = FALSE;
    u16         padrep;
    static u16  commit;

    ReadTP();

    padrep = DetectPadRepeat();     // ���������o

    // �y�[�W���ς���ăJ�[�\�����͂ݏo���Ă�����C��
    if( sCursorMenu >= (u16)sNumOfPageElements[ sCurrentPage ] )
    {
        sCursorMenu = (u16)(sNumOfPageElements[ sCurrentPage ] - 1);
    }

    // ���j���[����̍��ڑI��
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) ){                               // �J�[�\���̈ړ�
        if( ++sCursorMenu >= (u16)sNumOfPageElements[ sCurrentPage ] ) {
            sCursorMenu = 0;
        }
    }
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
        if( --sCursorMenu & 0x80 ) {
            sCursorMenu = (u16)(sNumOfPageElements[ sCurrentPage ] - 1);
        }
    }
    tp_select = SelectMenuByTP( &sCursorMenu, &s_settingParam );

    // ����{�^���^�b�`
    tpCommit = SelectSomethingByTP( &commit, func, 3 );

    // ���j���[�ւ̕���
    if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {         // ���j���[���ڂւ̕���
        if( (s_settingParam.pos[sCursorMenu]).enable ) {
            switch( sCurrentPage )
            {
                case 0:
                    switch( sCursorMenu )
                    {
                        case 0:
                            SetRatingLockInit();
                            g_pNowProcess = SetRatingLockMain;
                        break;
                        case 1:
                            SetRatingAgeInit();
                            g_pNowProcess = SetRatingAgeMain;
                        break;
                        case 2:
                            SetRatingOgnInit();
                            g_pNowProcess = SetRatingOgnMain;
                        break;
                    }
                break;

                case 1:
                    switch( sCursorMenu )
                    {
                        case 0:
                            SetPasswordInit();
                            g_pNowProcess = SetPasswordMain;
                        break;
                        case 1:
                            SetSecretQuestionIDInit();
                            g_pNowProcess = SetSecretQuestionIDMain;
                        break;
                        case 2:
                            SetAnswerInit();
                            g_pNowProcess = SetAnswerMain;
                        break;
                        case 3:
                        break;
                    }
                break;

                case 2:
                    switch( sCursorMenu )
                    {
#ifdef BROADON_UI
                        case 0:
                            sBroadOnMenu = MS_BROADON_WIIPOINT;   // ���ׂ�ON/OFF�ݒ�Ȃ̂Őݒ�֐������ʉ����ăt���O�Őݒ荀�ڐ؂�ւ�
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
                        case 1:
                            sBroadOnMenu = MS_BROADON_BROWSER;
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
                        case 2:
                            sBroadOnMenu = MS_BROADON_PICTOCHAT;
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
                        case 3:
                            sBroadOnMenu = MS_BROADON_NINTENDOSPOT;
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
#else   // BROADON_UI
                        case 0:
                        break;
#endif  // BROADON_UI
                    }
                break;

            } // switch( sCurrentPage )
        } // if( (s_settingParam.pos[sCursorMenu]).enable )
    } // if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) )
    else if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_CANCEL)) )
    {
        MachineSettingInit();
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_L) || (tpCommit && (commit == KEY_PREVPAGE)) )
    {
        if( sCurrentPage > 0 )
        {
            sCurrentPage--;
        }
    }
    else if( (pad.trg & PAD_BUTTON_R) || (tpCommit && (commit == KEY_SUCCPAGE)) )
    {
        if( sCurrentPage < (MS_PARENTAL_NUMOF_PAGES -1) )
        {
            sCurrentPage++;
        }
    }

    // �ĕ`��
    SetDrawnPageElement();
    DrawParentalControlMenuScene();
    return 0;
}

//=========================================================
//
// �\�t�g�E�F�A�L�[�{�[�h
//
//=========================================================

// �L�[�̕\��
static void DrawCharKeys( void )
{
	int l;
	u16 code;

	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		int color=TXT_COLOR_BLACK;
		code = char_tbl[s_char_mode][l];
		if (s_key_csr == l) color = TXT_COLOR_GREEN;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
                if( !sbEnableOKButton && (s_key_csr == l) && (code == OK_BUTTON_) )
                {
                    color = TXT_COLOR_RED;              // OK�����Ȃ��Ƃ��͐F��ς���
                }
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, str_button[x] );
			}
			else
			{
				u16 s[2];
				s[0] = code;
				s[1] = 0;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, s );
			}
		}
	}
}

// �ꕶ���폜
static void DeleteACharacter( eUseSoftKey noc )
{
    u16 *buf;
    u8 *length;
    if(noc == USE_SOFTKEY_COMMON)
    {
    }
    else if(noc == USE_SOFTKEY_ANSWER)
    {
        buf = spBufAnswer;
        length = &sCurrentLenAnswer;
    }
    else
    {
        //unknown
        return;
    }

    if(*length > 0) buf[--(*length)] = CHAR_USCORE;
}


// �I�𒆕����L�[�E����L�[�Ō��肵�����̋���
static void PushKeys( u16 code, eUseSoftKey noc )
{
    u16 *buf;
    u8 *length;
    u16 min_length;
    u16 max_length;
    u16 bufsize;
    if(noc == USE_SOFTKEY_COMMON)
    {
        // unused
        return;
    }
    else if(noc == USE_SOFTKEY_ANSWER)
    {
        buf = spBufAnswer;
        length = &sCurrentLenAnswer;
        // �ő咷�͌���ɂ���ĈقȂ�
        min_length = sMinLenAnswer;
        max_length = sMaxLenAnswer;
        bufsize    = sBufSizeAnswer;
    }
    else
    {
        //unknown
        return;
    }

    if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
    {
    	// ����L�[
    	switch(code)
    	{
    		case VAR_BUTTON1_:
    		case VAR_BUTTON2_:
    			SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
    			break;
    		case DEL_BUTTON_:
    			DeleteACharacter(noc);
    			break;
    		case SPACE_BUTTON_:
    			if(*length < max_length) buf[(*length)++] = L'�@';
    			break;
    		case OK_BUTTON_:
                if( *length >= min_length )              // �ŏ��������ɖ����Ȃ��Ƃ���OK�ł��Ȃ�
                {
                    //DEBUGPRINT( "OK: length = %d, min_length = %d, max_length = %d\n", *length, min_length, max_length );

                    // ����ƃo�b�t�@�T�C�Y���قȂ�Ƃ��ɒ��Ԃ̃f�[�^��0�N���A���Ă������ق������S
                    SVC_CpuClear(0, buf + *length, (bufsize - *length) * 2, 16 );     // �������̏���܂łł͂Ȃ��o�b�t�@�̍Ō�܂�0�N���A

                    // �f�o�b�O�p
                    //for( i=0; i < bufsize; i++ )
                    //{
                    //    DEBUGPRINT( "%04x", buf[i] );
                    //    if( ((i+1)%16) == 0 ) DEBUGPRINT( ",\n" );
                    //    else if( (i+1) == bufsize ) DEBUGPRINT( "\n" );
                    //    else DEBUGPRINT( "," );
                    //}

                    if(noc == USE_SOFTKEY_COMMON)
                    {
                    }
                    else if(noc == USE_SOFTKEY_ANSWER)
                    {
                        LCFG_TSD_SetPCTLSecretAnswer( buf );
                    }
                    // ::::::::::::::::::::::::::::::::::::::::::::::
                    // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
                    // ::::::::::::::::::::::::::::::::::::::::::::::
                    if( !MY_WriteTWLSettings() )
                    {
                        OS_TPrintf( "TWL settings write failed.\n" );
                    }
                    sbInitPage = FALSE;
                    SetParentalControlInit();
                    g_pNowProcess = SetParentalControlMain;
                }
                break;
                case CANCEL_BUTTON_:
                    sbInitPage = FALSE;                         // �y�[�W�����̂܂܂ɂ��Ă���
                    SetParentalControlInit();
                    g_pNowProcess = SetParentalControlMain;
                break;
			default:// unknown code
				break;
		}
	}
	else
	{
		// ���ʃL�[
		if(*length < max_length) buf[(*length)++] = code;
	}
}

// PadDetectOnKey��SelectSomethingByTP�Ŏg��SelectSomethingFunc�̎���
static BOOL SelectSoftwareKeyFunc( u16 *csr, TPData *tgt )
{
	// �܂��͌��ƂȂ���W�i�J�[�\���P�ʁj���擾
	int csrx;
	int csry;
	int csrxy;
	int a;
	int b;
	NNSG2dTextRect rect;
	u16 code;
	BOOL ret;
	
	csrx = tgt->x - CLIST_LT_X;
	csrx = csrx - (CLIST_SEGMENT_INTERVAL*(csrx/(CLIST_MARGIN*CLIST_KEY_PER_SEGMENT+CLIST_SEGMENT_INTERVAL)));
	csrx = csrx / CLIST_MARGIN;
	csry = (tgt->y - CLIST_LT_Y) / CLIST_MARGIN;
	if(csrx < 0 ) return FALSE;

	if ( csrx >= KEY_PER_LINE ) csrx = KEY_PER_LINE - 1;
	csrxy = csrx + csry * KEY_PER_LINE;

	if ( csrxy < 0 || csrxy >= CHAR_LIST_CHAR_NUM) return FALSE;// ���炩�ɂ͂ݏo����

	// �����W�̃L�[�R�[�h�擾
	code = char_tbl[s_char_mode][csrxy];
	if(code == EOM_) return FALSE;
	
	// �����W�̗̈�擾
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		int x = code - CODE_BUTTON_TOP_;
		rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str_button[x] );
	}
	else
	{
		u16 s[2];
		s[0] = code;
		s[1] = 0;
		// rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, s );
		// ���������Ⴉ�Ȃ蔻�肪�������c�c�M���M���܂łƂ��Ă݂�
		rect.width = CLIST_MARGIN;
		rect.height = CLIST_MARGIN;
	}
	a = CLIST_LT_X + CLIST_MARGIN*(csrxy%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((csrxy%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT);
	b = CLIST_LT_Y + CLIST_MARGIN*(csrxy/KEY_PER_LINE);
	
	// �����W�̗̈�Ƀ^�b�`���W���܂܂�Ă��邩�`�F�b�N
	ret = WithinRangeTP( a, b, a+rect.width, b+rect.height, tgt );
	
	if(ret)
	{
		*csr = (u16)csrxy;
	}
	return ret;
}

// �\�t�g�E�F�A�L�[��ł̃L�[�p�b�h�y�у^�b�`�p�b�h����
// ���ReadTP���Ă������ƁB
static void PadDetectOnKey( eUseSoftKey noc )
{
	SelectSomethingFunc func[1];
	BOOL tp_select = FALSE;
    u16  padrep;

    // ���������o
    padrep = DetectPadRepeat();

	//--------------------------------------
	//  �L�[���͏���
	//--------------------------------------
	if( (pad.trg & PAD_KEY_RIGHT) || (padrep & PAD_KEY_RIGHT) ){									// �J�[�\���̈ړ�
		do
		{
			if(s_key_csr%KEY_PER_LINE != KEY_PER_LINE-1) s_key_csr++;
			else s_key_csr -= KEY_PER_LINE-1;
			if( s_key_csr == CHAR_LIST_CHAR_NUM ) s_key_csr -= s_key_csr%KEY_PER_LINE;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( (pad.trg & PAD_KEY_LEFT) || (padrep & PAD_KEY_LEFT) ){
		do
		{
			if(s_key_csr%KEY_PER_LINE != 0) s_key_csr--;
			else s_key_csr += KEY_PER_LINE-1;
			if( s_key_csr & 0x8000 ) s_key_csr = KEY_PER_LINE-1;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) ){									// �J�[�\���̈ړ�
		do
		{
			s_key_csr += KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE*(s_key_csr/KEY_PER_LINE);
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
		do
		{
			if( s_key_csr < KEY_PER_LINE ) s_key_csr += (CHAR_LIST_CHAR_NUM/KEY_PER_LINE)*KEY_PER_LINE;
			else s_key_csr -= KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	
	func[0] = (SelectSomethingFunc)SelectSoftwareKeyFunc;
	tp_select = SelectSomethingByTP(&s_key_csr, func, 1 );
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// �L�[�������ꂽ
		PushKeys( char_tbl[s_char_mode][s_key_csr], noc );
	}else if( pad.trg & PAD_BUTTON_B ) {
		DeleteACharacter(noc);
	}
}

/*
// �����Ă���Ԑ��������X�s�[�h�ŕω�����悤�ȏ���
static void Birthday_AutoNumScrollByTP( void )
{
	static u16 first_csr = 0xffff;
	u16 temp_csr;
	static int same_count = 0;
	
	if( tpd.disp.touch )
	{
		BOOL t = SelectBirthdayFunc( &temp_csr, &tpd.disp );
		if( t )
		{
			if(same_count == 0) // count start
			{
				first_csr = temp_csr;
				same_count = 1;
			}else if(first_csr == temp_csr)
			{
				if( same_count == 1 || (same_count > 29 && same_count%10==0))
				{
					switch(temp_csr)
					{
						case 0:
							s_birth_csr = TRUE;
							s_temp_birthday.month++;
							break;
						case 1:
							s_birth_csr = FALSE;
							s_temp_birthday.day++;
							break;
						case 2:
							s_birth_csr = TRUE;
							s_temp_birthday.month--;
							break;
						case 3:
							s_birth_csr = FALSE;
							s_temp_birthday.day--;
							break;
						default:
							break;
					}
				}
				same_count++;
			}
		}
	}else // touch==0
	{
		same_count = 0;
		first_csr = 0xffff;
	}
}
*/

//=========================================================
//
// ���[�e�B���O�c�̂̐ݒ� (���X�g����̑I��)
//
//=========================================================

// �`�揈��
static void DrawSetRatingOgnScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"RATING ORGANIZATION" );

    PutStringUTF16(  4*8, 4*8, TXT_UCOLOR_G0, (const u16 *)L"Region: " );
    PutStringUTF16( 12*8, 4*8, TXT_UCOLOR_G0, sppRegionCharTable[ sRegion ] );
    PutStringUTF16(  4*8, 6*8, TXT_UCOLOR_G0, (const u16 *)L"Organization List: " );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );

    DrawMenu( sCursorRatingOgn, &sRatingOgnMenuParam );
}

// ������
static void SetRatingOgnInit( void )
{
    int               i;
    LCFGTWLRatingOgn  ogn;

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    ogn     = LCFG_TSD_GetPCTLRatingOgn();
    sRegion = LCFG_THW_GetRegion();

    // ���[�W�����ɂ��킹�đI��������c�̃��X�g������(���ʂ̂ق�����������)
    sRatingOgnFirstIndex = (u16)sIndexFirstOgnTable[ sRegion ];
    DEBUGPRINT( "ogn index = %d\n", sRatingOgnFirstIndex );
    sRatingOgnMenuParam.num = spNumOfRatingOgnsTable[ sRegion ];                            // ���X�g�̃����o��
    sRatingOgnMenuParam.pos = &(spRatingOgnPosTable[ sRatingOgnFirstIndex ]);               // �\���ʒu
    for( i=0; i < spNumOfRatingOgnsTable[ sRegion ]; i++ )
    {
        sppRatingOgnCharList[i] = sppRatingOgnCharTable[ (int)sRatingOgnFirstIndex + i ];   // �\��������
    }

    // �J�[�\���ʒu������
    if( (ogn < sRatingOgnFirstIndex) || ((sRatingOgnFirstIndex + spNumOfRatingOgnsTable[ sRegion ] - 1) < ogn) )
    {
        sCursorRatingOgn = 0;       // �͂ݏo�Ă���Ƃ��͕␳
    }
    else
    {
        sCursorRatingOgn = (u16)(ogn - sRatingOgnFirstIndex);
    }

    DrawSetRatingOgnScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetRatingOgnMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    u16  commit;
    BOOL tpSelect = FALSE;
    BOOL tpCommit = FALSE;

    ReadTP();
    
    // �I��
    if( pad.trg & PAD_KEY_DOWN ){                               // �J�[�\���̈ړ�
        if( ++sCursorRatingOgn >= (u16)spNumOfRatingOgnsTable[ sRegion ] ) {
            sCursorRatingOgn = 0;
        }
    }
    if( pad.trg & PAD_KEY_UP ){
        if( --sCursorRatingOgn & 0x8000 ) {
            sCursorRatingOgn = (u16)(spNumOfRatingOgnsTable[ sRegion ] - 1);
        }
    }
    tpSelect = SelectMenuByTP( &sCursorRatingOgn, &sRatingOgnMenuParam );

    // ����{�^���`�F�b�N
    tpCommit = SelectSomethingByTP( &commit, func, 2 );

    // ����
    if( (pad.trg & PAD_BUTTON_A) || (tpCommit && (commit == KEY_OK)) )
    {
        LCFG_TSD_SetPCTLRatingOgn( (LCFGTWLRatingOgn)(sRatingOgnFirstIndex + sCursorRatingOgn) );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_CANCEL)) )
    {
        SetParentalControlInit();                   // �L�����Z���̂Ƃ��Z�b�g���Ȃ�
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    DrawSetRatingOgnScene();
    return 0;
}

//=========================================================
//
// ���[�e�B���O������ON/OFF (ON/OFF�X�C�b�`�؂�ւ�)
//
//=========================================================

// �`�揈��
static void DrawSetRatingLockScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"RATING LOCK" );

    if( sbRatingLock )
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
static void SetRatingLockInit( void )
{
    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sbRatingLock = LCFG_TSD_IsSetParentalControl();

    DrawSetRatingLockScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetRatingLockMain( void )
{
    SelectSomethingFunc func[4]={SelectCancelFunc, SelectOKFunc, SelectONFunc, SelectOFFFunc };
    u16  commit;
    BOOL tp_touch = FALSE;

    ReadTP();
    
    // �L�[�ɂ��I��
    if( (pad.trg & PAD_KEY_LEFT) || (pad.trg & PAD_KEY_RIGHT) )
    {
        sbRatingLock = !sbRatingLock;
    }

    // �^�b�`�ɂ��I��
    tp_touch = SelectSomethingByTP( &commit, func, 4 );
    if( tp_touch && (commit == KEY_ON) )
    {
        sbRatingLock = TRUE;
    }
    else if( tp_touch && (commit == KEY_OFF) )
    {
        sbRatingLock = FALSE;
    }

    // ����
    if( (pad.trg & PAD_BUTTON_A) || (tp_touch && (commit == KEY_OK)) )
    {
        LCFG_TSD_SetFlagParentalControl( sbRatingLock );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tp_touch && (commit == KEY_CANCEL)) )
    {
        SetParentalControlInit();                   // �L�����Z���̂Ƃ��Z�b�g���Ȃ�
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    DrawSetRatingLockScene();
    return 0;
}

//=========================================================
//
// ���[�e�B���O�N��̑I�� (���l�I��)
//
//=========================================================

// �`�揈��
static void DrawSetRatingAgeScene( void )
{
    u16 iconUp[2]   = {0xE01B, 0};
    u16 iconDown[2] = {0xE01C, 0};
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"RATING TARGET AGE" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );
    PutStringUTF16( UP_BUTTON_TOP_X,     UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconUp );
    PutStringUTF16( UP_BUTTON_TOP_X+2*8, UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"UP" );
    PutStringUTF16( DOWN_BUTTON_TOP_X,     DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconDown );
    PutStringUTF16( DOWN_BUTTON_TOP_X+2*8, DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"DOWN" );
    PutStringUTF16( 8*8, 10*8, TXT_UCOLOR_G0, (const u16 *)L"Age" );
    PrintfSJIS( 12*8, 10*8, TXT_COLOR_GREEN, "%d", sRatingAge );
}

// ������
static void SetRatingAgeInit( void )
{
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sRatingAge = LCFG_TSD_GetPCTLRatingAge();

    DrawSetRatingAgeScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetRatingAgeMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    BOOL tpCommit = FALSE;
    BOOL tpUD = FALSE;
    u16  csrCommit;
    u16  csrUD;
    u16  padrep;

    ReadTP();

    // TP�`�F�b�N
    tpCommit = SelectSomethingByTP( &csrCommit, func, 2 );
    tpUD = DetectTouchUD( &csrUD );

    padrep = DetectPadRepeat();    // �L�[�̒��������o

    // �ύX
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) || (tpUD && (csrUD == KEY_UP)) )
    {
        if( (--sRatingAge) > LCFG_TWL_PCTL_RATING_AGE_MAX )
        {
            sRatingAge = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) || (tpUD && (csrUD == KEY_DOWN)) )
    {
        if( (++sRatingAge) & 0x80 )
        {
            sRatingAge = LCFG_TWL_PCTL_RATING_AGE_MAX;
        }
    }

    // ����
    if( pad.trg & PAD_BUTTON_A || (tpCommit && (csrCommit == KEY_OK)) )
    {
        LCFG_TSD_SetPCTLRatingAge( sRatingAge );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    // �ĕ`��
    DrawSetRatingAgeScene();
    return 0;
}

//=========================================================
//
// �閧�̎���ID�̑I�� (���l�I��)
//
//=========================================================

// �`�揈��
static void DrawSetSecretQuestionIDScene( void )
{
    u16 iconUp[2]   = {0xE01B, 0};
    u16 iconDown[2] = {0xE01C, 0};
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"SECRET QUESTION ID" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );
    PutStringUTF16( UP_BUTTON_TOP_X,     UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconUp );
    PutStringUTF16( UP_BUTTON_TOP_X+2*8, UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"UP" );
    PutStringUTF16( DOWN_BUTTON_TOP_X,     DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconDown );
    PutStringUTF16( DOWN_BUTTON_TOP_X+2*8, DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"DOWN" );
    PutStringUTF16( 8*8, 10*8, TXT_UCOLOR_G0, (const u16 *)L"No." );
    PrintfSJIS( 12*8, 10*8, TXT_COLOR_GREEN, "%d", sSecretQuestionID );
}

// ������
static void SetSecretQuestionIDInit( void )
{
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sSecretQuestionID = LCFG_TSD_GetPCTLSecretQuestionID();

    DrawSetSecretQuestionIDScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetSecretQuestionIDMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    BOOL tpCommit = FALSE;
    BOOL tpUD     = FALSE;
    u16  csrCommit;
    u16  csrUD;
    u16  rep;

    ReadTP();

    // TP�`�F�b�N
    tpCommit = SelectSomethingByTP( &csrCommit, func, 2 );
    tpUD     = DetectTouchUD( &csrUD );

    rep = DetectPadRepeat();

    // �ύX
    if( (pad.trg & PAD_KEY_UP) || (rep & PAD_KEY_UP) || (tpUD && (csrUD == KEY_UP)) )
    {
        if( (--sSecretQuestionID) > LCFG_TWL_PCTL_SECRET_QUESTION_ID_MAX )
        {
            sSecretQuestionID = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (rep & PAD_KEY_DOWN) || (tpUD && (csrUD == KEY_DOWN)) )
    {
        if( (++sSecretQuestionID) & 0x80 )
        {
            sSecretQuestionID = LCFG_TWL_PCTL_SECRET_QUESTION_ID_MAX;
        }
    }

    // ����
    if( pad.trg & PAD_BUTTON_A || (tpCommit && (csrCommit == KEY_OK)) )
    {
        LCFG_TSD_SetPCTLSecretQuestionID( sSecretQuestionID );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    // �ĕ`��
    DrawSetSecretQuestionIDScene();
    return 0;
}

//=========================================================
//
// �閧�̎���ւ̉� (�\�t�g�E�F�A�L�[�{�[�h����)
//
//=========================================================

// �`�揈��
static void DrawSetAnswerScene( void )
{
    //u16 tempbuf[MS_PARENTAL_ANS_COL*2+2];      // ���s�� + NULL������
    int row;
    int color;
    int i;
    u16 buf[2] = {0,0};     // 1�������\������

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"ANSWER FOR A SECRET QUESTION" );

    // ���������ŏ����������Z���Ƃ�OK�����Ȃ�
    if( sCurrentLenAnswer < sMinLenAnswer )
    {
        color            = TXT_UCOLOR_RED;  // �\���F���ς���
        sbEnableOKButton = FALSE;
    }
    else
    {
        color            = TXT_UCOLOR_G0;
        sbEnableOKButton = TRUE;
    }
    // ���s�ڂ���\�����邩����
    if( sCurrentLenAnswer <= (MS_PARENTAL_ANS_COL*2) )
    {
        row = 0;
    }
    else
    {
        row = 2;
    }
    for( i=0; i < MS_PARENTAL_ANS_COL; i++ )                    // 1�������\��(���Ԋu�ŕ\������������)
    {
        buf[0] = spBufAnswer[MS_PARENTAL_ANS_COL*row + i];
        buf[1] = 0;
        PutStringUTF16( 4*8 + 12*i, 15, color, buf );           // 1�s��
        buf[0] = spBufAnswer[MS_PARENTAL_ANS_COL*(row+1) + i];
        PutStringUTF16( 4*8 + 12*i, 31, color, buf );           // 2�s��
    }

    DrawCharKeys();
}

// ������
static void SetAnswerInit( void )
{
    //int i=0;

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // �R�����g�p�e���|�����o�b�t�@�̏�����
    LCFG_TSD_GetPCTLSecretAnswer( spBufAnswer );
    sCurrentLenAnswer = LCFG_TSD_GetPCTLSecretAnswerLength();

    // ���{�ł��������������ς��
    sRegion = LCFG_THW_GetRegion();                             // ���{�ł������͂̎d�l���ς��
    if( sRegion == OS_TWL_REGION_JAPAN )
    {
        SetSoftKeyboardButton(0);       // ���{�ꂪ�ݒ肳��Ă���Ƃ������f�t�H���g�̃L�[�{�[�h��ς���
        sMinLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MIN >> 1;
        sMaxLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX >> 1;
    }
    else
    {
        SetSoftKeyboardButton(2);
        sMinLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MIN;     // 6
        sMaxLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX;     // 64
    }
    s_key_csr = KEY_START;
    sBufSizeAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1;    // 64 + 1 = 65

    // ������̗�O����
    if( sCurrentLenAnswer > sMaxLenAnswer )        // �����32�����Ȃ̂ɂ���ȏ�̂Ƃ�/�����64�����ł�65�����ڂ�0�łȂ��Ƃ�
    {
        sCurrentLenAnswer = sMaxLenAnswer;         // �����񒷂�؂�l�߂ď���ȍ~����舵���Ȃ��悤�ɂ���
        SVC_CpuClear( 0, &spBufAnswer[ sMaxLenAnswer ], (sBufSizeAnswer - sMaxLenAnswer) * 2, 16 );     // ����ȍ~�����ׂ�0�N���A
    }

    //DEBUGPRINT( "-- for debug\n" );
    //for( i=0; i < sBufSizeAnswer; i++ )
    //{
    //    DEBUGPRINT( "%04x", spBufAnswer[i] );
    //    if( ((i+1)%16) == 0 ) DEBUGPRINT( ",\n" );
    //    else if( (i+1) == sBufSizeAnswer ) DEBUGPRINT( "\n" );
    //    else DEBUGPRINT( "," );
    //}
    //DEBUGPRINT( "currentlen = %d, maxlen = %d, clear size = %d\n", sCurrentLenAnswer, sMaxLenAnswer, (sBufSizeAnswer - sMaxLenAnswer) );

    // �\��������̐���
    if( sCurrentLenAnswer < LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX )
    {
        SVC_CpuClear( CHAR_USCORE, &spBufAnswer[ sCurrentLenAnswer ],                           // "_"���߂�
                      (LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX - sCurrentLenAnswer) * 2, 16);
    }
    spBufAnswer[ LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX ] = 0;  // �I�[����

    DrawSetAnswerScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetAnswerMain( void )
{
    ReadTP();
    PadDetectOnKey(USE_SOFTKEY_ANSWER);

    DrawSetAnswerScene();

    return 0;
}


// UTF16�̕����񒷎Z�o
static u8 MY_StrLen( const u16 *pStr )
{
	u8 len = 0;
	while( *pStr++ ) {
		++len;
		if( len == 255 ) {
			break;
		}
	}
	return len;
}

//=========================================================
//
// �p�X���[�h (�������l�I��)
//
//=========================================================

// �`�揈��
static void DrawSetPasswordScene( void )
{
    u16  iconUp[2]   = {0xE01B, 0};
    u16  iconDown[2] = {0xE01C, 0};
    int  posUpX, posUpY, posDownX, posDownY;
    int  i;

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PASSWORD" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, (sbValidPassword)?TXT_UCOLOR_G0:TXT_UCOLOR_RED, (const u16 *)L"�@OK" );   // �s���ȂƂ�OK�����Ȃ�

    for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
    {
        posUpX   = MULTI_UP_BUTTON_OFFSET_X + (MULTI_UP_BUTTON_WIDTH_X*i) + (MULTI_UP_BUTTON_INTERVAL_X*i);
        posUpY   = MULTI_UP_BUTTON_OFFSET_Y;
        posDownX = MULTI_DOWN_BUTTON_OFFSET_X + (MULTI_DOWN_BUTTON_WIDTH_X*i) + (MULTI_DOWN_BUTTON_INTERVAL_X*i);
        posDownY = MULTI_DOWN_BUTTON_OFFSET_Y;
        PutStringUTF16( posUpX,   posUpY,   TXT_UCOLOR_G0, (const u16 *)iconUp );
        PutStringUTF16( posDownX, posDownY, TXT_UCOLOR_G0, (const u16 *)iconDown );
        if( (spBufPassword[i] < 0) || (9 < spBufPassword[i]) )
        {
            PrintfSJIS( posUpX, posUpY+4*8,(sCursorPassword == i)?TXT_COLOR_GREEN:TXT_COLOR_BLACK, "*" );   // ��O����
        }
        else
        {
            PrintfSJIS( posUpX, posUpY+4*8,(sCursorPassword == i)?TXT_COLOR_GREEN:TXT_COLOR_BLACK, "%d", spBufPassword[i] );
        }
    }
}

// ������
static void SetPasswordInit( void )
{
    int i;

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    LCFG_TSD_GetPCTLPassword( spBufPassword );
    sbValidPassword = TRUE;
    for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
    {
        if( ('0' <= spBufPassword[i]) && (spBufPassword[i] <= '9' ) )
        {
            spBufPassword[i] = (u8)(spBufPassword[i] - '0');  // �v�Z���₷���悤�ɕ����R�[�h���琔�l�ɒ����Ă���
        }
        else
        {
            spBufPassword[i] = 0xFF;                          // �s��
            sbValidPassword  = FALSE;
        }
    }
    sCursorPassword = 0;                            // �J�[�\���͐ݒ�̂��тɐ擪�ɂȂ�

    DrawSetPasswordScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetPasswordMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    BOOL tpCommit = FALSE;
    BOOL tpUD     = FALSE;
    u16  csrCommit;
    u16  csrUD;
    int  i;
    u16  padrep;

    ReadTP();

    padrep = DetectPadRepeat();

    // ���E�L�[�Ō����ړ�
    if( pad.trg & PAD_KEY_RIGHT )
    {
        if( ++sCursorPassword > (LCFG_TWL_PCTL_PASSWORD_LENGTH - 1) )        // ���ݑI������Ă��錅�̒l��ύX
        {
            sCursorPassword = 0;
        }
    }
    if( pad.trg & PAD_KEY_LEFT )
    {
        if( --sCursorPassword & 0x80 )
        {
            sCursorPassword = (LCFG_TWL_PCTL_PASSWORD_LENGTH - 1);
        }
    }

    // �\���L�[�Ŋe���̒l��ύX
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) )
    {
        if( --(spBufPassword[sCursorPassword]) > 9 )        // ���ݑI������Ă��錅�̒l��ύX
        {
            spBufPassword[sCursorPassword] = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) )
    {
        if( ++(spBufPassword[sCursorPassword]) & 0x80 )
        {
            (spBufPassword[sCursorPassword]) = 9;
        }
    }

    // TP�`�F�b�N
    tpCommit = SelectSomethingByTP( &csrCommit, func, 2 );
    tpUD     = DetectTouchMultiUD( &csrUD );

    // �^�b�`�p�l���Ŋe���̒l��ύX
    if( tpUD && ((csrUD & MASK_MULTI_KEY) == MULTI_KEY_UP) )
    {
        sCursorPassword = (u16)(csrUD & ~MASK_MULTI_KEY);        // �������
        if( --(spBufPassword[sCursorPassword]) > 9 )
        {
            spBufPassword[sCursorPassword] = 0;
        }
    }
    if( tpUD && ((csrUD & MASK_MULTI_KEY) == MULTI_KEY_DOWN) )
    {
        sCursorPassword = (u16)(csrUD & ~MASK_MULTI_KEY);
        if( ++(spBufPassword[sCursorPassword]) & 0x80 )
        {
            (spBufPassword[sCursorPassword]) = 9;
        }
    }

    // ���ׂĂ̒l���������ǂ����`�F�b�N
    sbValidPassword = TRUE;
    for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
    {
        if( !((0 <= spBufPassword[i])&&(spBufPassword[i] <= 9)) )
        {
            sbValidPassword = FALSE;
            break;
        }
    }

    // ����
    if( ((pad.trg & PAD_BUTTON_A) || (tpCommit && (csrCommit == KEY_OK))) && sbValidPassword )     // ��������Ȃ��Ƃ��̓R�~�b�g�s��
    {
        for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
        {
            spBufPassword[i] = (u8)(spBufPassword[i] + '0');          // �����R�[�h�ɒ����Ă���
        }
        LCFG_TSD_SetPCTLPassword( spBufPassword );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    // �ĕ`��
    DrawSetPasswordScene();
    return 0;
}

//=========================================================
//
// BroadOn�p�ݒ� (������ON/OFF�X�C�b�`�؂�ւ�)
//
//=========================================================

// �`�揈��
static void DrawSetBroadOnSettingScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    switch( sBroadOnMenu )          // �����̐ݒ��ʂ��t���O�Ő؂�ւ���
    {
        case MS_BROADON_PICTOCHAT:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PICTOCHAT BOOT" );
        break;
        case MS_BROADON_DS_DOWNLOAD:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"DS DOWNLOAD BOOT" );
        break;
        case MS_BROADON_BROWSER:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BROWSER BOOT" );
        break;
        case MS_BROADON_WIIPOINT:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"WII POINT USING" );
        break;
        case MS_BROADON_PHOTO_EXCHANGE:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PHOTO EXCHANGE USING" );
        break;
        case MS_BROADON_UGC:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"UGC USING" );
        break;
    }

    if( sbBroadOnSetting )
    {
        PutStringUTF16( NOTRESTRICT_BUTTON_TOP_X, NOTRESTRICT_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L"Not Restrict" );
        PutStringUTF16( RESTRICT_BUTTON_TOP_X,   RESTRICT_BUTTON_TOP_Y,     TXT_COLOR_GREEN, (const u16*)L"Restrict" );
    }
    else
    {
        PutStringUTF16( NOTRESTRICT_BUTTON_TOP_X, NOTRESTRICT_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L"Not Restrict" );
        PutStringUTF16( RESTRICT_BUTTON_TOP_X,   RESTRICT_BUTTON_TOP_Y,     TXT_UCOLOR_G0,   (const u16*)L"Restrict" );
    }
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�ACANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"�@OK" );
}

// ������
static void SetBroadOnSettingInit( void )
{
    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // ���炩����TWL�ݒ�f�[�^�t�@�C������ǂݍ��ݍς݂̐ݒ���擾
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    switch( sBroadOnMenu )
    {
        case MS_BROADON_PICTOCHAT:
            sbBroadOnSetting = LCFG_TSD_IsRestrictPictoChatBoot();
        break;
        case MS_BROADON_DS_DOWNLOAD:
            sbBroadOnSetting = LCFG_TSD_IsRestrictDSDownloadBoot();
        break;
        case MS_BROADON_BROWSER:
            sbBroadOnSetting = LCFG_TSD_IsRestrictBrowserBoot();
        break;
        case MS_BROADON_WIIPOINT:
            sbBroadOnSetting = LCFG_TSD_IsRestrictWiiPoint();
        break;
        case MS_BROADON_PHOTO_EXCHANGE:
            sbBroadOnSetting = LCFG_TSD_IsRestrictPhotoExchange();
        break;
        case MS_BROADON_UGC:
            sbBroadOnSetting = LCFG_TSD_IsRestrictUGC();
        break;
    }

    DrawSetBroadOnSettingScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// �\���v���Z�X�Ƃ��ČĂяo����郁�C��
static int SetBroadOnSettingMain( void )
{
    SelectSomethingFunc func[4]={SelectCancelFunc, SelectOKFunc, SelectRESTRICTFunc, SelectNOTRESTRICTFunc };
    u16  commit;
    BOOL tp_touch = FALSE;

    ReadTP();
    
    // �L�[�ɂ��I��
    if( (pad.trg & PAD_KEY_DOWN) || (pad.trg & PAD_KEY_UP) )
    {
        sbBroadOnSetting = !sbBroadOnSetting;
    }

    // �^�b�`�ɂ��I��
    tp_touch = SelectSomethingByTP( &commit, func, 4 );
    if( tp_touch && (commit == KEY_RESTRICT) )
    {
        sbBroadOnSetting = TRUE;
    }
    else if( tp_touch && (commit == KEY_NOTRESTRICT) )
    {
        sbBroadOnSetting = FALSE;
    }

    // ����
    if( (pad.trg & PAD_BUTTON_A) || (tp_touch && (commit == KEY_OK)) )
    {
        switch( sBroadOnMenu )
        {
            case MS_BROADON_PICTOCHAT:
                LCFG_TSD_SetRestrictPictoChatBoot( sbBroadOnSetting );
            break;
            case MS_BROADON_DS_DOWNLOAD:
                LCFG_TSD_SetRestrictDSDownloadBoot( sbBroadOnSetting );
            break;
            case MS_BROADON_BROWSER:
                LCFG_TSD_SetRestrictBrowserBoot( sbBroadOnSetting );
            break;
            case MS_BROADON_WIIPOINT:
                LCFG_TSD_SetRestrictWiiPoint( sbBroadOnSetting );
            break;
            case MS_BROADON_PHOTO_EXCHANGE:
                LCFG_TSD_SetRestrictPhotoExchange( sbBroadOnSetting );
            break;
            case MS_BROADON_UGC:
                LCFG_TSD_SetRestrictUGC( sbBroadOnSetting );
            break;
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL�ݒ�f�[�^�t�@�C���ւ̏�������
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tp_touch && (commit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();                   // �L�����Z���̂Ƃ��Z�b�g���Ȃ�
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    DrawSetBroadOnSettingScene();
    return 0;
}


//======================================================
// �\�t�g�E�F�A�L�[�{�[�h�p�L�����e�[�u��
//======================================================

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM] = {
	{	// �Ђ炪��
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	DEL_BUTTON_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	SPACE_BUTTON_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	EOM_,
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	VAR_BUTTON1_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	EOM_,	L'��',	L'��',	VAR_BUTTON2_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	EOM_,
		
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	EOM_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'��',	L'��',	L'��',	L'��',	OK_BUTTON_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	L'�A',	L'�B',	L'�I',	L'�H',	EOM_,
		
		L'�u',	L'�v',	L'�`',	EOM_,	L'�[',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// �J�^�J�i
		L'�A',	L'�C',	L'�E',	L'�G',	L'�I',
		L'�J',	L'�L',	L'�N',	L'�P',	L'�R',	DEL_BUTTON_,
		L'�T',	L'�V',	L'�X',	L'�Z',	L'�\',
		L'�^',	L'�`',	L'�c',	L'�e',	L'�g',	SPACE_BUTTON_,
		L'�i',	L'�j',	L'�k',	L'�l',	L'�m',
		L'�n',	L'�q',	L'�t',	L'�w',	L'�z',	EOM_,
		
		L'�}',	L'�~',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	VAR_BUTTON1_,
		L'��',	L'��',	L'��',	L'��',	L'��',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	VAR_BUTTON2_,
		L'�@',	L'�B',	L'�D',	L'�F',	L'�H',
		L'��',	EOM_,	L'��',	EOM_,	L'��',	EOM_,
		
		L'�K',	L'�M',	L'�O',	L'�Q',	L'�S',
		L'�U',	L'�W',	L'�Y',	L'�[',	L'�]',	EOM_,
		L'�_',	L'�a',	L'�d',	L'�f',	L'�h',
		L'�o',	L'�r',	L'�u',	L'�x',	L'�{',	OK_BUTTON_,
		L'�p',	L'�s',	L'�v',	L'�y',	L'�|',
		L'�b',	L'�A',	L'�B',	L'�I',	L'�[',	EOM_,
		
		L'�u',	L'�v',	L'�`',	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// �p��
        // 'A'���珇��UTF-16�����R�[�h(���g���G���f�B�A��)�ő�����Ă���
		0x0041,	0x0042,	0x0043,	0x0044,	0x0045,
		0x0046,	0x0047,	0x0048,	0x0049,	0x004a,	DEL_BUTTON_,
		0x004b,	0x004c,	0x004d,	0x004e,	0x004f,
		0x0050,	0x0051,	0x0052,	0x0053,	0x0054,	SPACE_BUTTON_,
		0x0055,	0x0056,	0x0057,	0x0058,	0x0059,
		0x005a,	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		// 'a'���珇
		0x0061,	0x0062,	0x0063,	0x0064,	0x0065,
		0x0066,	0x0067,	0x0068,	0x0069,	0x006a,	VAR_BUTTON1_,
		0x006b,	0x006c,	0x006d,	0x006e,	0x006f,
		0x0070,	0x0071,	0x0072,	0x0073,	0x0074,	VAR_BUTTON2_,
		0x0075,	0x0076,	0x0077,	0x0078,	0x0079,
		0x007a,	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		// '0'���珇
		0x0030,	0x0031,	0x0032,	0x0033,	0x0034,
		0x0035,	0x0036,	0x0037,	0x0038,	0x0039,	EOM_,
        // ���ꕶ��
        // '!'           '&'             '/'
		0x0021,	EOM_,	0x0026,	EOM_,	0x002f,
        // ','           '.'             '-'
		0x002c,	EOM_,	0x002e,	EOM_,	0x002d,	OK_BUTTON_,
        // '''           '"'
		0x0027,	EOM_,	0x2033,	EOM_,	EOM_,
        // '@'           '('             ')'
		0x0040,	EOM_,	0x0028,	EOM_,	0x0029,	EOM_,
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
};


