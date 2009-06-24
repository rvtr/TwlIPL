/************************************************************************

�^�C�g���F
  �����A������\�����[�`���W

  ************************************************************************/
#include <string.h>
#include "basicdef.h"
#include "MyStrings.h"

/************************************************************************
�����샂�[�h
 ************************************************************************/
#define USE_SDK_PRINTF  //�r�c�j�̂o�����������֌W���g�p����Ƃ��ɒ�`

/************************************************************************

�����̐錾
 ************************************************************************/

//�W���̃R���\�[��
CONSOLE_TYPE StdConsole = { { 0, 0},
                            COLOR_BLACK,
                            0, SCREEN_HEIGHT-2,
                            &wText
                           };

CONSOLE_TYPE *Console  = &StdConsole;

/************************************************************************
���v���g�^�C�v�錾
 ************************************************************************/
static void    ScrollDown( CONSOLE_TYPE *Console );

/************************************************************************
*************************************************************************
*************************************************************************





�������o�͊֌W�̃��[�e�B���e�B





*************************************************************************
*************************************************************************
*************************************************************************/

/************************************************************************

�@�ꕶ���\��

�T�v�F
  �����̕����R�[�h���e�L�X�g�u�q�`�l��ɕ\��������
  ( putchar �ƃR���p�`���ȁH�j

�����F
  Console   �\���R���\�[��
  Character �����R�[�h

�߂�l�F
  �����R�[�h�iputchar�ƂȂ�ׂ��d�l�����킹�邽�߁j

  ***********************************************************************/
extern int wcPutchar( CONSOLE_TYPE *Console, int Character ){
    int i;
    u16 *Pointer;
    switch( Character ){

      case '\n':   /* ���s�R�[�h�̏��� */
        Console->Cursor.y++;
        Console->Cursor.x = 0;
        break;

      case '\t':   /* �^�u�R�[�h�̏����i�S�^�u�j */
        Console->Cursor.x +=4 - ( (StdConsole.Cursor.x) % 4 );
        break;

      case '\f':   /* ���y�[�W�R�[�h�̏��� */
        Pointer = &(Console->Text->Map[0][0]       );
        for( i=0 ; i<SCREEN_HEIGHT*SCREEN_WIDTH ; i++ ) *Pointer++=CLEAR_CHARACTER;
        wcGotoxy( Console, 0, 0 );

        break;

      default:
        /* �����̕\�� */
        Console->Text->Map[ Console->Cursor.y ][ Console->Cursor.x++ ]
                                            = (u16)( (Console->Palette << 12 ) | Character );
    }

    /* �s���ӂ�̏��� */
    if( Console->Cursor.x > SCREEN_WIDTH-1 ){
        Console->Cursor.x = 0;
        Console->Cursor.y++;
    }

    /* �񂠂ӂ�̏��� */
    if( Console->Cursor.y > Console->ScrollEnd ){
        --Console->Cursor.y;
        ScrollDown( Console );

    }
    return Character;
}

/************************************************************************

�@�ꕶ���\���i����R�[�h�����Ȃ��j

�T�v�F
  �����̕����R�[�h���e�L�X�g�u�q�`�l��ɕ\��������
  wcPutchar�̐���R�[�h����Ȃ��o�[�W����

�����F
  Console   �\���R���\�[��
  Character �����R�[�h

�߂�l�F
  �����R�[�h�iputchar�ƂȂ�ׂ��d�l�����킹�邽�߁j

  ***********************************************************************/
extern int wcPutcharNC( CONSOLE_TYPE *Console, int Character ){

    /* �����̕\�� */
    Console->Text->Map[ Console->Cursor.y ][ Console->Cursor.x++ ]
                                         = (u16)( (Console->Palette << 12) | Character );
    /* �s���ӂ�̏��� */
    if( Console->Cursor.x > SCREEN_WIDTH-1 ){
        Console->Cursor.x = 0;
        Console->Cursor.y++;
    }

    /* �񂠂ӂ�̏��� */
    if( Console->Cursor.y > Console->ScrollEnd ){
        --Console->Cursor.y;
        ScrollDown( Console );

    }
    return Character;
}

/************************************************************************

�@������\���i���s�Ȃ��Łj

�T�v�F
  �����̕������\��������i�Ō�ɉ��s�R�[�h������Ȃ��̂Œ��Ӂj

�����F
  Console   �\���R���\�[��
  String    �\��������

�� 0x80-0xff�̕����R�[�h�ɑΉ����邽�߁A(u8 *)�ւ̃L���X�g�����Ă��܂��B

  ***********************************************************************/
extern void _cPuts( CONSOLE_TYPE *Console, const char *String ){
    int CurrentCharacter;
    while( CurrentCharacter = *(u8 *)String++ ) wcPutchar( Console, CurrentCharacter );
}

/************************************************************************

�@������\��

�T�v�F
  �����̕��������ʂɕ\��������i�Ō�ɉ��s�R�[�h������܂��j

�����F
�@Console   �\���R���\�[��
�@String    �\��������

�߂�l�F
�@��ɂO�@���������Ƃ̃R���p�`�r���e�B��ۂ���

  ***********************************************************************/
extern int wcPuts( CONSOLE_TYPE *Console, const char *String ){
    _cPuts( Console, String );        /* ������̕\�� */
    _cPuts( Console, "\n"   );        /* ���s         */
    return 0;
}

/************************************************************************

  �e�L�X�g��ʂ����ɃX�N���[��������

�����F
  Console   �Y���R���\�[��

  ***********************************************************************/
static void ScrollDown( CONSOLE_TYPE *Console ){
    int i;

    for( i=Console->ScrollStart+1 ; i<=Console->ScrollEnd ; i++ ){
        /* ����ɃR�s�[���� */
        memcpy( Console->Text->Map[i-1], Console->Text->Map[i], SCREEN_WIDTH* sizeof(u16) );
    }

    /* �ŏI�s�̃N���A */
    for( i=0 ; i<SCREEN_WIDTH ; i++ ){
        Console->Text->Map[ Console->ScrollEnd ][i] = CLEAR_CHARACTER;
    }

}

/************************************************************************

  �J�[�\���ʒu�̐ݒ�

�����F
    Console �Y���R���\�[��
    x       �w���W
    y       �x���W

 ************************************************************************/
extern void wcGotoxy( CONSOLE_TYPE *Console, int x, int y ){
    /* �Ƃ肠�������W�̃R�s�[ */
    Console->Cursor.x = x;
    Console->Cursor.y = y;

    /* �͈͐��������邱������Ȃ��ƃI�[�o�[�������N���� */
    if( Console->Cursor.x < 0             ) Console->Cursor.x = 0;
    if( Console->Cursor.x > SCREEN_WIDTH-1  ) Console->Cursor.x = SCREEN_WIDTH-1;
    if( Console->Cursor.y < 0             ) Console->Cursor.y = 0;
    if( Console->Cursor.y > SCREEN_HEIGHT-1 ) Console->Cursor.y = SCREEN_HEIGHT-1;

}
/************************************************************************

  �p���b�g�̐ݒ�

�����F
    Console �Y���R���\�[��
    Palette �ݒ�p���b�g

 ************************************************************************/
extern void wcSetPalette( CONSOLE_TYPE *Console, int Palette ){

    Console->Palette = Palette;

}

/************************************************************************

���o����������

��NitroSDK��OS_Printf���Q�l�ɂ��Ă��܂��B


  �o�������������ǂ�

���W���̃R���\�[���ɕ\������܂��B

�����F
    fmt: �t�H�[�}�b�g�t��������
    ...: �p�����[�^


 ************************************************************************/
extern void wPrintf( const char *fmt , ... ){

    char common_buffer[0x100];

    va_list vlist;

    //�����擾
    va_start( vlist, fmt );

    //vprintf��������
#ifdef USE_SDK_PRINTF
    OS_VSNPrintf( common_buffer, sizeof(common_buffer), fmt, vlist );
#else
    vsnprintf( common_buffer, sizeof(common_buffer), fmt, vlist );
#endif
    _Puts( common_buffer );

    //�����㏈��
    va_end( vlist );

}


/************************************************************************

  ������̍s���̃J�E���g����

�����F
    Str ������

�߂�l�F
    �s��
 ************************************************************************/
extern int wCountLine( const char *Str ){

    int Line;

    if( *Str=='\0' ) return 0;  //�󕶎���̔���

    Line=1;
    while( *Str!='\0' ){
        if( *Str=='\n' ) Line++;
        Str++;
    }

    return Line;
}



