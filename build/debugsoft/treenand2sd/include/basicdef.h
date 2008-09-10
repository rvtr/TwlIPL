/**************************************************************************

  ��{��`�w�b�_�t�@�C��

  �����̃\�[�X�t�@�C���Ŏg�p�����{�I�ȏ����L�q���Ă��܂��B


 *************************************************************************/


/* ���d�C���N���[�h�̉�� */
#ifndef __BASICDEF_H__
#define __BASICDEF_H__

/**************************************************************************
 �W���ŃC���N���[�h����w�b�_�t�@�C��
 *************************************************************************/
#include <nitro.h>
#include <stdio.h>  // NULL���g�p���邽��

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
  #define�n�v���v���Z�b�T���� �L�q�̈�
 *************************************************************************/

//�u����
#define VERTICAL_CYCLE          16743     //�P�ʃ�S


//��ʊ֌W�̒�`
#define SCREEN_WIDTH    32             /* �R���\�[���Ŏg�p����e�L�X�g�q�`�l�̗� */
#define SCREEN_HEIGHT   24             /* �R���\�[���Ŏg�p����e�L�X�g�q�`�l�̍s�� */

#define VIRTUAL_SCREEN_WIDTH  32     /* ���z��ʂ̃T�C�Y */
#define VIRTUAL_SCREEN_HEIGHT 32

#define CHARACTER_SIDE_LENGTH   8       /* �L�����N�^�̈�ӂ̒��� */

#define SCREEN_WIDTH_DOT    256
#define SCREEN_HEIGHT_DOT   192

/**************************************************************************
    �ėp�}�N����`
 *************************************************************************/

//�z��̗v�f�������߂�  'Code Complete ��@���Ł@P379 ������p
#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

/**************************************************************************
    �^��`
 *************************************************************************/


#if 0
/* �u�[���^�̐ݒ� */
#undef FALSE
#undef TRUE
typedef enum { FALSE=0, TRUE=1 } BOOL;
#endif

/* �����^���W�\���� */
typedef struct {    /* ���W���w�肷��\����   */
    int x;         /*  �w���W                */
    int y;        /*   �x���W               */
} COORDINATE_TYPE;

/* �e�F�̐��� */
typedef enum {
    RAW_COLOR_BLACK     = 0x0000,
    RAW_COLOR_RED       = 0x001f,
    RAW_COLOR_GREEN     = 0x03e0,
    RAW_COLOR_YELLOW    = 0x03ff,
    RAW_COLOR_BLUE      = 0x7c00,
    RAW_COLOR_MAGENTA   = 0x7c1f,
    RAW_COLOR_CYAN      = 0x7fe0,
    RAW_COLOR_WHITE     = 0x7fff,

    RAW_COLOR_GRAY1     = 0x1004,
    RAW_COLOR_GRAY2     = 0x2108,
    RAW_COLOR_GRAY3     = 0x318c,
    RAW_COLOR_GRAY4     = 0x4210,
    RAW_COLOR_GRAY5     = 0x5294,
    RAW_COLOR_GRAY6     = 0x6318

} RAW_COLOR;

/* �p���b�g�ݒ� */
typedef enum {
    COLOR_TRANSLUCENT   = 0,
    COLOR_RED           = 1,
    COLOR_GREEN         = 2,
    COLOR_YELLOW        = 3,
    COLOR_BLUE          = 4,
    COLOR_MAGENTA       = 5,
    COLOR_CYAN          = 6,
    COLOR_WHITE         = 7,
    COLOR_BLACK         = 8,
    COLOR_GRAY1         = 9,
    COLOR_GRAY2         =10,
    COLOR_GRAY3         =11,
    COLOR_GRAY4         =12,
    COLOR_GRAY5         =13,
    COLOR_GRAY6         =14


} COLOR_TYPE;

#define COLOR_GRAY COLOR_GRAY5

#ifdef __cplusplus
}
#endif

/* ���d�C���N���[�h�̉�� */
#endif





