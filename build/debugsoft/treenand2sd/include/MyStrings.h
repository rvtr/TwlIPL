/*************************************************************************

�ȒP�ȕ�����\������֐��Q

  ************************************************************************/
#include <stdio.h>

#include "basicdef.h"
#include "text.h"

/************************************************************************
  ��d��`��錾/�Q�Ƃ𓯂��w�b�_�Ŏg�p���邽�߂̃v���v���Z�b�T
  ************************************************************************/

/* ���d�C���N���[�h�̉�� */
#ifndef MYSTRINGS_H_INCLUDED
#define MYSTRINGS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************

���g�p����\����

 ************************************************************************/


/* ��ʏo�͂��邽�߂Ɏg�p����R���\�[�� */
typedef struct {
    COORDINATE_TYPE Cursor;        /* ���݂̃J�[�\���ʒu */
    int             Palette;          /* ���ݎg�p����p���b�g  */

    int             ScrollStart;   /* �X�N���[���J�n�E�I���ʒu */
    int             ScrollEnd;

    TEXT_VRAM_TYPE  *Text;         /* �������ݐ� */

} CONSOLE_TYPE;


/************************************************************************

���Q�Ɛ錾
 ************************************************************************/

extern CONSOLE_TYPE StdConsole;
extern CONSOLE_TYPE *Console;



/************************************************************************
���p�u���b�N�Ɠ����Ɏg�p����}�N��
 ************************************************************************/
#define wPuts( str__ )            wcPuts( &StdConsole, str__ )
#define _Puts( str__ )            _cPuts( &StdConsole, str__ )
#define wPutchar( character__ )   wcPutchar( &StdConsole, character__ )
#define wPutcharNC( character__ ) wcPutcharNC( &StdConsole, character__ )
#define wGotoxy( x__, y__ )       wcGotoxy( &StdConsole, x__, y__ )
#define wSetPalette( Palette__ )  wcSetPalette( &StdConsole, Palette__ );


/************************************************************************
���p�u���b�N�֐��̐錾
 ************************************************************************/
extern int     wcPuts      ( CONSOLE_TYPE *Console, const char *str );
extern void    _cPuts      ( CONSOLE_TYPE *Console, const char *str );
extern void    wcGotoxy    ( CONSOLE_TYPE *Console, int x, int y );
extern void    wcSetPalette( CONSOLE_TYPE *Console, int Palette );
extern int     wcPutchar   ( CONSOLE_TYPE *Console, int Character );
extern int     wcPutcharNC ( CONSOLE_TYPE *Console, int Character );
extern void    wcPrintf    ( CONSOLE_TYPE *Console, const char *fmt , ... );
extern void    wPrintf     ( const char *fmt , ... );
extern void    swPrintf    ( const char *fmt , ... );
extern int     wCountLine  ( const char *Str );

#ifdef __cplusplus
}
#endif

/* ���d�C���N���[�h�̉�� */
#endif

