/************************************************************************

タイトル：
  文字、文字列表示ルーチン集

  ************************************************************************/
#include <string.h>
#include "basicdef.h"
#include "MyStrings.h"

/************************************************************************
☆動作モード
 ************************************************************************/
#define USE_SDK_PRINTF  //ＳＤＫのＰｒｉｎｔｆ関係を使用するときに定義

/************************************************************************

☆実体宣言
 ************************************************************************/

//標準のコンソール
CONSOLE_TYPE StdConsole = { { 0, 0},
                            COLOR_BLACK,
                            0, SCREEN_HEIGHT-2,
                            &wText
                           };

CONSOLE_TYPE *Console  = &StdConsole;

/************************************************************************
☆プロトタイプ宣言
 ************************************************************************/
static void    ScrollDown( CONSOLE_TYPE *Console );

/************************************************************************
*************************************************************************
*************************************************************************





☆文字出力関係のユーティリティ





*************************************************************************
*************************************************************************
*************************************************************************/

/************************************************************************

　一文字表示

概要：
  引数の文字コードをテキストＶＲＡＭ上に表示させる
  ( putchar とコンパチかな？）

引数：
  Console   表示コンソール
  Character 文字コード

戻り値：
  文字コード（putcharとなるべく仕様をあわせるため）

  ***********************************************************************/
extern int wcPutchar( CONSOLE_TYPE *Console, int Character ){
    int i;
    u16 *Pointer;
    switch( Character ){

      case '\n':   /* 改行コードの処理 */
        Console->Cursor.y++;
        Console->Cursor.x = 0;
        break;

      case '\t':   /* タブコードの処理（４タブ） */
        Console->Cursor.x +=4 - ( (StdConsole.Cursor.x) % 4 );
        break;

      case '\f':   /* 改ページコードの処理 */
        Pointer = &(Console->Text->Map[0][0]       );
        for( i=0 ; i<SCREEN_HEIGHT*SCREEN_WIDTH ; i++ ) *Pointer++=CLEAR_CHARACTER;
        wcGotoxy( Console, 0, 0 );

        break;

      default:
        /* 文字の表示 */
        Console->Text->Map[ Console->Cursor.y ][ Console->Cursor.x++ ]
                                            = (u16)( (Console->Palette << 12 ) | Character );
    }

    /* 行あふれの処理 */
    if( Console->Cursor.x > SCREEN_WIDTH-1 ){
        Console->Cursor.x = 0;
        Console->Cursor.y++;
    }

    /* 列あふれの処理 */
    if( Console->Cursor.y > Console->ScrollEnd ){
        --Console->Cursor.y;
        ScrollDown( Console );

    }
    return Character;
}

/************************************************************************

　一文字表示（制御コード処理なし）

概要：
  引数の文字コードをテキストＶＲＡＭ上に表示させる
  wcPutcharの制御コード判定なしバージョン

引数：
  Console   表示コンソール
  Character 文字コード

戻り値：
  文字コード（putcharとなるべく仕様をあわせるため）

  ***********************************************************************/
extern int wcPutcharNC( CONSOLE_TYPE *Console, int Character ){

    /* 文字の表示 */
    Console->Text->Map[ Console->Cursor.y ][ Console->Cursor.x++ ]
                                         = (u16)( (Console->Palette << 12) | Character );
    /* 行あふれの処理 */
    if( Console->Cursor.x > SCREEN_WIDTH-1 ){
        Console->Cursor.x = 0;
        Console->Cursor.y++;
    }

    /* 列あふれの処理 */
    if( Console->Cursor.y > Console->ScrollEnd ){
        --Console->Cursor.y;
        ScrollDown( Console );

    }
    return Character;
}

/************************************************************************

　文字列表示（改行なし版）

概要：
  引数の文字列を表示させる（最後に改行コードが入らないので注意）

引数：
  Console   表示コンソール
  String    表示文字列

※ 0x80-0xffの文字コードに対応するため、(u8 *)へのキャストをしています。

  ***********************************************************************/
extern void _cPuts( CONSOLE_TYPE *Console, const char *String ){
    int CurrentCharacter;
    while( CurrentCharacter = *(u8 *)String++ ) wcPutchar( Console, CurrentCharacter );
}

/************************************************************************

　文字列表示

概要：
  引数の文字列を画面に表示させる（最後に改行コードが入ります）

引数：
　Console   表示コンソール
　String    表示文字列

戻り値：
　常に０　ｐｕｔｓとのコンパチビリティを保つため

  ***********************************************************************/
extern int wcPuts( CONSOLE_TYPE *Console, const char *String ){
    _cPuts( Console, String );        /* 文字列の表示 */
    _cPuts( Console, "\n"   );        /* 改行         */
    return 0;
}

/************************************************************************

  テキスト画面を下にスクロールさせる

引数：
  Console   該当コンソール

  ***********************************************************************/
static void ScrollDown( CONSOLE_TYPE *Console ){
    int i;

    for( i=Console->ScrollStart+1 ; i<=Console->ScrollEnd ; i++ ){
        /* 一列上にコピーする */
        memcpy( Console->Text->Map[i-1], Console->Text->Map[i], SCREEN_WIDTH* sizeof(u16) );
    }

    /* 最終行のクリア */
    for( i=0 ; i<SCREEN_WIDTH ; i++ ){
        Console->Text->Map[ Console->ScrollEnd ][i] = CLEAR_CHARACTER;
    }

}

/************************************************************************

  カーソル位置の設定

引数：
    Console 該当コンソール
    x       Ｘ座標
    y       Ｙ座標

 ************************************************************************/
extern void wcGotoxy( CONSOLE_TYPE *Console, int x, int y ){
    /* とりあえず座標のコピー */
    Console->Cursor.x = x;
    Console->Cursor.y = y;

    /* 範囲制限をするこれをしないとオーバーランを起こす */
    if( Console->Cursor.x < 0             ) Console->Cursor.x = 0;
    if( Console->Cursor.x > SCREEN_WIDTH-1  ) Console->Cursor.x = SCREEN_WIDTH-1;
    if( Console->Cursor.y < 0             ) Console->Cursor.y = 0;
    if( Console->Cursor.y > SCREEN_HEIGHT-1 ) Console->Cursor.y = SCREEN_HEIGHT-1;

}
/************************************************************************

  パレットの設定

引数：
    Console 該当コンソール
    Palette 設定パレット

 ************************************************************************/
extern void wcSetPalette( CONSOLE_TYPE *Console, int Palette ){

    Console->Palette = Palette;

}

/************************************************************************

☆Ｐｒｉｎｔｆ

※NitroSDKのOS_Printfを参考にしています。


  Ｐｒｉｎｔｆもどき

※標準のコンソールに表示されます。

引数：
    fmt: フォーマット付き文字列
    ...: パラメータ


 ************************************************************************/
extern void wPrintf( const char *fmt , ... ){

    char common_buffer[0x100];

    va_list vlist;

    //引数取得
    va_start( vlist, fmt );

    //vprintf相当部分
#ifdef USE_SDK_PRINTF
    OS_VSNPrintf( common_buffer, sizeof(common_buffer), fmt, vlist );
#else
    vsnprintf( common_buffer, sizeof(common_buffer), fmt, vlist );
#endif
    _Puts( common_buffer );

    //引数後処理
    va_end( vlist );

}


/************************************************************************

  文字列の行数のカウントする

引数：
    Str 文字列

戻り値：
    行数
 ************************************************************************/
extern int wCountLine( const char *Str ){

    int Line;

    if( *Str=='\0' ) return 0;  //空文字列の判定

    Line=1;
    while( *Str!='\0' ){
        if( *Str=='\n' ) Line++;
        Str++;
    }

    return Line;
}



