/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     ntcompress_main.c

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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "nitroCompLib.h"
#include "ntcompress_test.h"
#include "multipleCompLib.h"
#include "rangeCoder.h"

#define FOR_NITRO

#if defined( FOR_NITRO )
  #define INCLUDE_TARGET    "nitro"
  #define TARGET_NAME       "ntrcomp"
  #define NTCOMPRESS_VERSION  "1.1.0"
#else // for revolution
  #define SUPPORT_LH_LRC
  #define INCLUDE_TARGET    "revolution"
  #define TARGET_NAME       "ntcompress"
  #define NTCOMPRESS_VERSION  "2.0.0"
#endif


#define uint    unsigned int
#define ushort  unsigned short
#define ulong   unsigned long
#define uchar   unsigned char


#define ROUNDUP(x,n) (((x) + ((n) - 1)) & (~((n)-1)))
#define ROUNDUP4(x) ROUNDUP((x),4)

#define TEXT_BIG_ENDIAN_FLAG    0x10
#define TEXT_WIDTH_MASK         0x07

//---------------------------------------------------------------------------
// 型定義
//---------------------------------------------------------------------------
// 圧縮タイプ
typedef enum
{
    CONV_TYPE_NON,                     // 変換なし
    CONV_TYPE_DIFF,                    // 差分フィルタ
    CONV_TYPE_RUNLENGTH,               // ランレングス符号化
    CONV_TYPE_LZ77,                    // LZ77圧縮
    CONV_TYPE_HUFFMAN,                 // ハフマン圧縮
    CONV_TYPE_UNCOMPRESS,              // 解凍
    CONV_TYPE_LZ77EX,                  // 拡張LZ77圧縮
    CONV_TYPE_LH,                      // LZ77-ハフマン複合圧縮
    CONV_TYPE_LRC,                     // LZ77-RangeCoder複合圧縮
    CONV_TYPE_RANGECODER               // レンジコーダ
}
t_conv_type;

// 圧縮データ情報構造体
typedef struct
{
    t_conv_type type;                  // 圧縮タイプ
    uint        opt;                   // 圧縮パラメータ
    ulong       src_size;              // 圧縮前データサイズ
    ulong       dest_size;             // 圧縮後データサイズ
    uchar       *src_buf;              // 圧縮前データバッファ
    uchar       *dest_buf;             // 圧縮後データバッファ
    uchar       textout_width;         // テキスト出力するかどうかのフラグ兼サイズ
    uchar       alignment;             // 出力ファイルサイズのアライメント
    uchar       header_flg :1;         // ヘッダ情報を付加するかどうかのフラグ
    uchar       silent_flg :1;         // 標準出力へメッセージを出力するかどうかのフラグ
    uchar       verify_flg :1;         // 圧縮ファイルを展開する際のべりファイをおこなうかどうかのフラグ
}
t_conv_data;

typedef enum
{
    OPT_NONE,        // 無効なオプション
    OPT_VERSION,     // バージョン表示(-v)
    OPT_SILENT,      // 標準出力抑止(-s)
    OPT_LOWER_TEXT,  // 小文字でのテキスト出力(-t)
    OPT_UPPER_TEXT,  // 大文字でのテキスト出力(-T)
    OPT_ALIGN,       // アライメント指定(-A4|8|16|32)
    OPT_HEADER,      // ヘッダ付加(-H)
    OPT_OUTPUT,      // 出力ファイル指定(-o)
    OPT_EXTRACT,     // 解凍(-x)
    OPT_DIFF_FILTER, // 差分フィルタ(-d)
    OPT_RUN_LENGTH,  // ランレングス圧縮(-r)
    OPT_LZ77,        // 旧LZ77圧縮(-l)
    OPT_LZ77EX,      // LZ77圧縮(-lex)
    OPT_LH,          // LH圧縮(-lh)
    OPT_LRC,         // LRC圧縮(-lrc)
    OPT_HUFFMAN,     // ハフマン圧縮(-h4|8)
    
    OPT_RANGECODER, // レンジコーダ圧縮(デバッグ版のみ) (-R)
    OPT_VERIFY,     // ベリファイ      (デバッグ版のみ) (-D)
    OPT_TEST,       // テスト          (デバッグ版のみ) (--TEST)
}
t_opt;




//---------------------------------------------------------------------------
// プロトタイプ宣言
//---------------------------------------------------------------------------
static void  usage( const char *cmd_string );
static void  version_info( void );
static int   parse_opt( int argc, char *argv[], t_conv_data * cnv_dat, char **in_fname,
                        char **out_fname );
static int   load_file   ( const char *fname, t_conv_data * cnv_dat );
static int   save_file   ( const char *fname, const t_conv_data * cnv_dat );
static int   convert_data( t_conv_data * cnv_dat );
static int   fwrite_text ( const uchar * buf, ulong size, uint width, FILE * fp, const char *name );
static char* cut_fname   ( char *path );
static char* make_output_fname( const char *input_fname, const t_conv_data * cnv_dat );
static int   verify_data ( t_conv_data * cnv_dat );


//---------------------------------------------------------------------------
// エンディアン変換
//---------------------------------------------------------------------------
static ushort
reverse_endian16( ushort x )
{
    return (ushort)( ( (x >> 8) & 0x00FF ) | ( (x << 8) & 0xFF00 ) );
}

static ulong
reverse_endian32( ulong x )
{
    return (ulong)( ( (x >> 24) & 0x000000FF ) |
                    ( (x >>  8) & 0x0000FF00 ) |
                    ( (x <<  8) & 0x00FF0000 ) |
                    ( (x << 24) & 0xFF000000 ) );
}



//---------------------------------------------------------------------------
// メイン関数
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    t_conv_data cnv_dat;
    char   *in_fname, *out_fname;
    
#ifdef _DEBUG
    if ( argc >= 2 && strcmp(argv[1], "-TEST") == 0 )
    {
        ntcompress_test();
        return 0;
    }
#endif

    // データの初期化
    memset(&cnv_dat, 0, sizeof(cnv_dat));

    // コマンドオプションの解析
    if (parse_opt(argc, argv, &cnv_dat, &in_fname, &out_fname) != 0)
    {
        exit(1);
    }

    // 入力ファイルをバッファへ読み込み
    if (load_file(in_fname, &cnv_dat) != 0)
    {
        exit(1);
    }

    // ファイルの圧縮
    if (convert_data(&cnv_dat) != 0)
    {
        exit(1);
    }
    
    // 圧縮データのベリファイ
    if (verify_data( &cnv_dat ) != 0)
    {
        exit(1);
    }
    
    // バッファから出力ファイルへの書き込み
    if (save_file(out_fname, &cnv_dat) != 0)
    {
        exit(1);
    }

    if (!cnv_dat.silent_flg)
    {
        fprintf(stdout, "convert %s(%ldbyte) to %s(%ldbyte)\n", in_fname, cnv_dat.src_size,
                out_fname, cnv_dat.dest_size);
    }

    free(cnv_dat.src_buf);
    if (cnv_dat.dest_buf != cnv_dat.src_buf)
    {
        free(cnv_dat.dest_buf);
    }

    return 0;
}

//---------------------------------------------------------------------------
// USAGE:
//---------------------------------------------------------------------------
static void usage(const char *cmd_string)
{
    fprintf(stderr, "\n");
#if defined( SUPPORT_LH_LRC )
    fprintf(stderr,
            "Usage: %s <-d(4|8)|r|l|lex|h(8|16)|lh|lrc> [-o outputFile] [-A(4|8|16|32)] [-<t|T>[width]] [-s] [-H] [-v] <inputFile> \n"
            "       %s -x [-o outputFile] [-s] <inputFile> \n",
            cmd_string, cmd_string);
#else
    fprintf(stderr,
            "Usage: %s <-d(4|8)|r|l|lex|h(8|16)> [-o outputFile] [-A(4|8|16|32)] [-<t|T>[width]] [-s] [-H] [-v] <inputFile> \n"
            "       %s -x [-o outputFile] [-s] <inputFile> \n",
            cmd_string, cmd_string);
#endif
    fprintf(stderr, "\t-v                   Show version\n");
    fprintf(stderr, "\t-r                   Runlength encode.\n");
    fprintf(stderr, "\t-l                   LZ77 encode(compatible with previous LZ77).\n");
    fprintf(stderr, "\t-lex                 LZ77 encode.\n");
    fprintf(stderr, "\t-h BitSize(4|8)      Huffman   encode.\n");
    fprintf(stderr, "\t-d BitSize(8|16)     Differential filter.\n");
#if defined( SUPPORT_LH_LRC )
    fprintf(stderr, "\t-lh                  LZ and Huffman encode\n");
    fprintf(stderr, "\t-lrc                 LZ and RangeCoder encode\n");
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "\t-A(4|8|16|32)        Align n byte for compressed filesize\n");
    fprintf(stderr, "\t-o outputFile        Specify the output file name.\n");
    fprintf(stderr, "\t-t[TypeWidth(1|2|4)] output C format text(little endian).\n");
    fprintf(stderr, "\t                     We can specify the type(1=u8,2=u16,4=u32).\n");
    fprintf(stderr, "\t-T[TypeWidth(1|2|4)] output C format text(big endian).\n");
    fprintf(stderr, "\t                     We can specify the type(1=u8,2=u16,4=u32).\n");
    fprintf(stderr, "\t-s                   Print no message if you've been successful in the conversion.\n");
    fprintf(stderr, "\t-H                   Raw data header\n");
    fprintf(stderr, "\t-x                   Extract compressed file.\n");
}


static void version_info( void )
{
    fprintf(stderr, "%s version %s\n", TARGET_NAME, NTCOMPRESS_VERSION);
}

//---------------------------------------------------------------------------
// パス文字列からファイル名部分のポインタを取得し、拡張子を削る
// @param path パス
// @return ファイル名のポインタ
//---------------------------------------------------------------------------
static char *cut_fname(char *path)
{
    char   *search_tmp;

    if (path == NULL)
    {
        return NULL;
    }

    if ((search_tmp = strrchr(path, '/')) != NULL)
    {
        path = (search_tmp + 1);
    }
    if ((search_tmp = strrchr(path, '\\')) != NULL)
    {
        path = (search_tmp + 1);
    }
    if ((search_tmp = strrchr(path, '.')) != NULL)
    {
        *search_tmp = '\0';
    }
    return path;
}

//---------------------------------------------------------------------------
// 入力ファイル名から出力ファイル名を生成する関数
// @param input_fname 入力ファイル名
// @param cnv_dat 変換情報
// @return 出力ファイル名へのポインタ
//---------------------------------------------------------------------------
static char *make_output_fname(const char *input_fname, const t_conv_data * cnv_dat)
{
    char   *out_fname, *edit_tmp;

    if (input_fname == NULL)
    {
        return NULL;
    }

    // メモリ領域の確保
    if ((out_fname = (char *)malloc(strlen(input_fname) + 0x10)) == NULL)
    {
        fprintf(stderr, "internal err: malloc fail!\n");
        exit(1);
    }

    strcpy(out_fname, input_fname);
    edit_tmp = cut_fname(out_fname);

    // 圧縮方式をファイル名に付加
    switch ( cnv_dat->type )
    {
    case CONV_TYPE_NON:
        strcat(edit_tmp, "_NONE");
        break;
    case CONV_TYPE_DIFF:
        strcat(edit_tmp, "_DIFF");
        break;
    case CONV_TYPE_RUNLENGTH:
        strcat(edit_tmp, "_RL");
        break;
    case CONV_TYPE_LZ77:
    case CONV_TYPE_LZ77EX:
        strcat(edit_tmp, "_LZ");
        break;
    case CONV_TYPE_HUFFMAN:
        strcat(edit_tmp, "_HUFF");
        break;
    case CONV_TYPE_LH:
        strcat(edit_tmp, "_LH");
        break;
    case CONV_TYPE_LRC:
        strcat(edit_tmp, "_LRC");
        break;
    case CONV_TYPE_RANGECODER:
        strcat(edit_tmp, "_RC");
        break;
    case CONV_TYPE_UNCOMPRESS:
        strcat(edit_tmp, "_DECOMP");
    }
    // 拡張子を付加
    if (cnv_dat->textout_width != 0)
    {
        strcat(edit_tmp, ".c");
    }
    else
    {
        strcat(edit_tmp, ".bin");
    }
    return out_fname;
}


//---------------------------------------------------------------------------
// 次のオプションのタイプ判別
// @param  opt 判別したいオプション文字列
// @return オプションのタイプ
//---------------------------------------------------------------------------
t_opt
get_opt_type( const char* opt )
{
    if ( strncmp( opt, "--", 2 ) == 0 )
    {
    }
    else
    {
        // オプション解析
        switch ( opt[1] )
        {
        case 'v':
            return OPT_VERSION;
        case 's':
            return OPT_SILENT;
        case 'D':
            return OPT_VERIFY;
        case 't':
            return OPT_LOWER_TEXT;
        case 'T':
            return OPT_UPPER_TEXT;
        case 'A':
            return OPT_ALIGN;
        case 'H':
            return OPT_HEADER;
        case 'o':
            return OPT_OUTPUT;
        case 'x':
            return OPT_EXTRACT;
        case 'd':
            return OPT_DIFF_FILTER;
        case 'r':
            return OPT_RUN_LENGTH;
        case 'R':
            return OPT_RANGECODER;
        case 'l':
            if ( strcmp(opt, "-lex" ) == 0 )
            {
                return OPT_LZ77EX;
            }
#if defined( SUPPORT_LH_LRC )
            else if ( strcmp(opt, "-lh") == 0 )
            {
                return OPT_LH;
            }
            else if ( strcmp(opt, "-lrc") == 0 )
            {
                return OPT_LRC;
            }
#endif
            else
            {
                return OPT_LZ77;
            }
        case 'h':
            return OPT_HUFFMAN;
        }
    }
    return OPT_NONE;
}


//---------------------------------------------------------------------------
// コマンドパラメータ解析
// @param argc パラメータ数
// @param argv パラメータ配列
// @retval cnv_dat 圧縮情報データ
// @retval in_fname 入力ファイル名
// @retval out_fname 出力ファイル名
// @return 0 ファイルの書き込み成功
//        -1 ファイルの書き込みエラー
//---------------------------------------------------------------------------

static int parse_opt(int argc, char *argv[], t_conv_data * cnv_dat,
                     char **in_fname, char **out_fname)
{
    int     i = 1;
    t_conv_type type;
    int     opt_num;
    char   *cmd_string;
#define SYNTAX_CHECK(exp) do { if (!(exp)) { usage(cmd_string); return -1; } } while(0)

    // コマンド名取得
    cmd_string = cut_fname(argv[0]);
    
    // 状態初期化
    cnv_dat->type = CONV_TYPE_NON;
    cnv_dat->header_flg = 0;
    cnv_dat->textout_width = 0;
    cnv_dat->silent_flg = 0;
    cnv_dat->alignment  = 0;
    cnv_dat->verify_flg = 0;
    *out_fname = NULL;
    *in_fname = NULL;
    
    // オプション解析
    while (i < argc)
    {
        t_opt opt;
        
        if (argv[i][0] != '-')
        {
            SYNTAX_CHECK(*in_fname == NULL);    // ２重読みチェック
            
            // 入力ファイル名取得
            *in_fname = argv[i];
            i++;
            continue;
        }
        
        opt = get_opt_type( argv[i] );
        
        // ハイフン付オプション解析
        switch ( opt )
        {
        //--------------------
        // バージョン表示オプション'v'(引数0)
        case OPT_VERSION:
            SYNTAX_CHECK(argv[i][2] == '\0'); // 文字数チェック
            
            version_info();
            return -1;
            
        //--------------------
        // 標準出力無しオプション's' (引数0)
        case OPT_SILENT:
            SYNTAX_CHECK(argv[i][2] == '\0');   // 文字数チェック
            SYNTAX_CHECK(!cnv_dat->silent_flg); // ２重読みチェック
            
            cnv_dat->silent_flg = 1;
            break;
        
#ifdef _DEBUG
        //--------------------
        // ベリファイオプション'D' (引数0)
        case OPT_VERIFY:
            SYNTAX_CHECK(argv[i][2] == '\0');   // 文字数チェック
            SYNTAX_CHECK(!cnv_dat->verify_flg); // ２重読みチェック
            cnv_dat->verify_flg = 1;
            break;
#endif
        
        //--------------------
        // C言語テキスト出力オプション't' (引数0-1) (リトルエンディアン)
        case OPT_LOWER_TEXT:
            SYNTAX_CHECK(cnv_dat->textout_width == 0);  // ２重読みチェック
            
            if (strlen(argv[i]) == 2)  // パラメータが無い場合はバイト単位で書き込む
            {
                cnv_dat->textout_width = 1;
            }
            else
            {
                cnv_dat->textout_width = atoi(&argv[i][2]);
                SYNTAX_CHECK(cnv_dat->textout_width == 1 || cnv_dat->textout_width == 2
                             || cnv_dat->textout_width == 4);
            }
            break;
            
        //--------------------
        // C言語テキスト出力オプション'T' (引数0-1) (ビッグエンディアン)
        case OPT_UPPER_TEXT:
            SYNTAX_CHECK(cnv_dat->textout_width == 0);  // ２重読みチェック
            
            if (strlen(argv[i]) == 2)  // パラメータが無い場合はバイト単位で書き込む
            {
                cnv_dat->textout_width = 1;
            }
            else
            {
                cnv_dat->textout_width = atoi(&argv[i][2]);
                SYNTAX_CHECK(cnv_dat->textout_width == 1 || cnv_dat->textout_width == 2
                             || cnv_dat->textout_width == 4);
            }
            cnv_dat->textout_width |= TEXT_BIG_ENDIAN_FLAG;
            break;
            
        //--------------------
        // 出力ファイルサイズアライン指定'A4|8|16|32' (引数0)
        case OPT_ALIGN:
            SYNTAX_CHECK(strlen(argv[i]) <= 4 ); // 文字数チェック
            SYNTAX_CHECK( cnv_dat->alignment == 0 ); // ２重読みチェック
            
            cnv_dat->alignment = atoi(&argv[i][2]);
            SYNTAX_CHECK( cnv_dat->alignment == 4 || cnv_dat->alignment == 8 || cnv_dat->alignment == 16 || cnv_dat->alignment == 32 );
            
            break;
            
        //--------------------
        // 展開後データの先頭にデータサイズを付加するオプション'H' (引数0)
        case OPT_HEADER:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // 文字数チェック
            SYNTAX_CHECK(!cnv_dat->header_flg); // ２重読みチェック
            
            cnv_dat->header_flg = 1;
            break;
            
        //--------------------
        // 出力ファイル指定オプション 'o' (引数1)
        case OPT_OUTPUT:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // 文字数チェック
            SYNTAX_CHECK(*out_fname == NULL);   // ２重読みチェック
            SYNTAX_CHECK(i + 1 < argc);         // パラメータチェック
            
            *out_fname = argv[++i];
            break;
        
        //--------------------
        // 解凍指定 'x'(引数0)
        case OPT_EXTRACT:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // 文字数チェック
            
            type = CONV_TYPE_UNCOMPRESS;
            opt_num = 0;
            goto common;
        
        //--------------------
        // 差分フィルタ指定 'd'(引数1)
        case OPT_DIFF_FILTER:
            type = CONV_TYPE_DIFF;
            goto common_opt1;
            
        //--------------------
        // ランレングス圧縮指定'r'(引数0)
        case OPT_RUN_LENGTH:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // 文字数チェック
            
            type = CONV_TYPE_RUNLENGTH;
            opt_num = 0;
            goto common;
            
#ifdef _DEBUG
        //--------------------
        // レンジコーダ圧縮指定'R'(引数0)
        case OPT_RANGECODER:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // 文字数チェック
            
            type = CONV_TYPE_RANGECODER;
            opt_num = 0;
            goto common;
#endif
        //--------------------
        // 旧LZ77圧縮"-l"(引数0)
        case OPT_LZ77:
            SYNTAX_CHECK( (argv[i][2] == '\0') || isdigit(argv[i][2]) );
            type = CONV_TYPE_LZ77;
            opt_num = 0;
            cnv_dat->opt = 2;
            
          #if defined( FOR_NITRO )
            // 次にパラメータがあり、入力ファイル名でなければ読み飛ばす
            if ( i + 2 < argc && argv[i + 1][0] != '-' && 
                 argv[i + 1][1] == '\0' && atoi(&argv[i + 1][0]) != 0 )
            {
                i++;
            }
          #endif
            goto common;
            
        //--------------------
        // LZ77圧縮"-lex"(引数0)
        case OPT_LZ77EX:
            type = CONV_TYPE_LZ77EX;
            opt_num = 0;
            cnv_dat->opt = 2; // 検索開始位置はオプションで指定できない事にする
            goto common;            
            
        //--------------------
        // LH圧縮"-lh"(引数0)
        case OPT_LH:
            type = CONV_TYPE_LH;
            opt_num = 0;
            goto common;
        
        //--------------------
        // LRC圧縮"-lrc"(引数0)
        case OPT_LRC:
            type = CONV_TYPE_LRC;
            opt_num = 0;
            goto common;
        
        //--------------------
        // ハフマン圧縮指定'h'(引数1)
        case OPT_HUFFMAN:
            type = CONV_TYPE_HUFFMAN;
            goto common_opt1;
            
        //--------------------
        // 圧縮方式指定部分の共通処理
        common_opt1:
            if (strlen(argv[i]) == 2)
                // 文字列が終了していれば次の引数がパラメータとなる
            {
                opt_num = 1;
            }
            else
                // 文字列が続いていればその値がパラメータとなる
            {
                cnv_dat->opt = atoi(&argv[i][2]);
                SYNTAX_CHECK(cnv_dat->opt != 0);        // パラメータが1以上の数字であるかのチェック
                opt_num = 0;
            }
            // don't break;
        common:
            SYNTAX_CHECK(cnv_dat->type == CONV_TYPE_NON);       // ２重読みチェック
            
            cnv_dat->type = type;
            
            if (opt_num == 0)
            {
                break;
            }
            
            // 引数がある場合
            SYNTAX_CHECK(i + 1 < argc); // パラメータチェック
            i++;
            SYNTAX_CHECK(argv[i][0] != '-');    // パラメータにハイフンで始まらないかのチェック
            
            cnv_dat->opt = atoi(argv[i]);
            
            SYNTAX_CHECK(cnv_dat->opt != 0);    // パラメータが1以上の数字であるかのチェック
            
            break;
            
        //--------------------
        default:
            usage(cmd_string);
            return -1;
        }
        i++;
    }
    
    SYNTAX_CHECK(*in_fname != NULL);   // 入力ファイルが指定されたかどうかのチェック
    
    // 出力ファイル名の自動生成
    if (*out_fname == NULL)
    {
        *out_fname = make_output_fname(*in_fname, cnv_dat);
    }
    
    if (strcmp(*in_fname, *out_fname) == 0)
    {
        fprintf(stderr, "ERR: output file %s is same as input file\n", *out_fname);
        return -1;
    }
    
    if ( cnv_dat->type == CONV_TYPE_UNCOMPRESS )
    // 解凍オプションが指定された場合のオプションチェック
    {
        SYNTAX_CHECK(cnv_dat->alignment  == 0); // アライメントの指定は不能です
        SYNTAX_CHECK(cnv_dat->header_flg == 0); // ヘッダの追加オプションは不能です
    }
    
    return 0;
#undef SYNTAX_CHECK
}



//---------------------------------------------------------------------------
// 入力ファイルの読み込み関数
// 
// @param fname 入力ファイル名
// @retval 圧縮情報構造体へのポインタ
// @return 0 ファイルの読み込み成功
//        -1 ファイルの読み込みエラー
//---------------------------------------------------------------------------

static int load_file(const char *fname, t_conv_data * cnv_dat)
{
    FILE   *in_file;

    // ファイルのオープン
    if ((in_file = fopen(fname, "rb")) == NULL)
    {
        fprintf(stderr, "ERR: could not open file: %s\n", fname);
        return -1;
    }

    // ファイルのサイズ取得
    fseek(in_file, 0, SEEK_END);
    if ((cnv_dat->src_size = ftell(in_file)) < 0)
    {
        fprintf(stderr, "ERR: file error\n");
        return -1;
    }

    // 入力ファイルをバッファへ読み込み
    if ((cnv_dat->src_buf = (uchar *) malloc(ROUNDUP4(cnv_dat->src_size))) == NULL)
    {
        fprintf(stderr, "ERR: memory exhausted\n");
        return -1;
    }

    fseek(in_file, 0, SEEK_SET);
    if (fread(cnv_dat->src_buf, 1, cnv_dat->src_size, in_file) != cnv_dat->src_size)
    {
        fprintf(stderr, "ERR: read error\n");
        return -1;
    }

    fclose(in_file);

    // ４バイトアラインまでを0で埋める
    {
        ulong   i;
        for (i = 0; (cnv_dat->src_size + i) & 3; i++)
        {
            cnv_dat->src_buf[cnv_dat->src_size + i] = 0;
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// 出力ファイルへの書き込み関数
// 
// @param fname 出力ファイル名
// @retval 圧縮情報構造体へのポインタ
// @return 0 ファイルの書き込み成功
//        -1 ファイルの書き込みエラー
//---------------------------------------------------------------------------

static int save_file(const char *fname, const t_conv_data * cnv_dat)
{
    FILE   *out_file;

    // 書き込みデータのチェック
    if (cnv_dat->dest_buf == NULL || cnv_dat->dest_size < 0)
    {
        fprintf(stderr, "ERR: convert error\n");
        return -1;
    }

    // ファイルのオープン
    if ((out_file = fopen(fname, "wb")) == NULL)
    {
        fprintf(stderr, "ERR: could not open file: %s\n", fname);
        return -1;
    }

    // バッファを出力ファイルへ書き込み
    if (cnv_dat->textout_width != 0)
    {
        // テキスト出力
        if ( fwrite_text(cnv_dat->dest_buf, cnv_dat->dest_size, cnv_dat->textout_width, out_file, fname) 
             != cnv_dat->dest_size )
        {
            fprintf(stderr, "ERR: write error\n");
            return -1;
        }
    }
    else
    {
        // バイナリ出力
        if ( fwrite(cnv_dat->dest_buf, 1, cnv_dat->dest_size, out_file) != cnv_dat->dest_size )
        {
            fprintf(stderr, "ERR: write error\n");
            return -1;
        }
    }

    fclose(out_file);

    return 0;
}

//---------------------------------------------------------------------------
// C言語フォーマットでのテキスト出力
// @param buf データバッファ
// @param size データサイズ
// @param fp 出力ファイルポインタ
// @param name 定数ラベル名
// @return 0 ファイルの書き込み成功
//        -1 ファイルの書き込みエラー
//---------------------------------------------------------------------------

static int fwrite_text(const uchar * buf, ulong size, uint width, FILE * fp, const char *name)
{
    char   *const_name_buf;
    char   *const_name_ptr;
    ulong   i;
    ulong   line_num;
    uint    big_endian = width & TEXT_BIG_ENDIAN_FLAG;
    
    width  &= TEXT_WIDTH_MASK;
    
    if (fp == NULL)
    {
        return -1;
    }
    if (buf == NULL)
    {
        return -1;
    }
    if (name == NULL)
    {
        return -1;
    }

    // 定数名の決定
    if ((const_name_buf = (char *)malloc( strlen(name) + 1 )) == NULL)
    {
        fprintf(stderr, "internal err: malloc fail!\n");
        return -1;
    }

    strcpy(const_name_buf, name);
    const_name_ptr = cut_fname(const_name_buf);
    size = (size + width - 1) / width;

    // データの書き込み
    if ( fprintf(fp, "#include <%s/types.h>\n\n", INCLUDE_TARGET) < 0 )
    {
        return -1;
    }
    
    if ( fprintf(fp, "const u%d %s[0x%lX] = {", width * 8, const_name_ptr, size) < 0 )
    {
        return -1;
    }
    
    line_num = (width >= 2) ? 2 : 1;
    
    for ( i = 0; i < size; i++ )
    {
        if ((i * line_num) % 0x10 == 0)
        {
            if (fprintf(fp, "\n") < 0)
            {
                return -1;
            }
        }
        switch ( width )
        {
        case 1:
            {
                if ( fprintf(fp, "0x%02X,", *buf) < 0 )
                {
                    return -1;
                }
                buf++;
            }
            break;
        case 2:
            {
                ushort val = *(ushort*)buf;
                if ( big_endian )
                {
                    val = reverse_endian16( val );
                }
                
                if ( fprintf(fp, "0x%04X,", val) < 0 )
                {
                    return -1;
                }
                buf += 2;
            }
            break;
        case 4:
            {
                ulong val = *(ulong*)buf;
                if ( big_endian )
                {
                    val = reverse_endian32( val );
                }
                
                if ( fprintf(fp, "0x%08lX,", val) < 0 )
                {
                    return -1;
                }
                buf += 4;
            }
            break;
        default:
            return -1;
        }
    }

    if (fprintf(fp, "\n};\n") < 0)
    {
        return -1;
    }

    free(const_name_buf);

    return i * width;
}


//---------------------------------------------------------------------------
// データの変換
// @retval cnv_dat 変換情報データ
// @return 0 変換成功
//        -1 変換失敗
//---------------------------------------------------------------------------

static int convert_data(t_conv_data * cnv_dat)
{
    char    cmd_str[16];

    // コマンドストリングの生成
    switch (cnv_dat->type)
    {
    //---------------------
    // 差分フィルタ
    case CONV_TYPE_DIFF:
        if (cnv_dat->opt != 8 && cnv_dat->opt != 16)
        {
            fprintf(stderr, "ERR: invalid DIFF parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "d%d", cnv_dat->opt);
        break;

    //---------------------
    // ランレングス圧縮
    case CONV_TYPE_RUNLENGTH:
        // ランレングス符号化
        strcpy(cmd_str, "r");
        break;

    //---------------------
    // LZ77圧縮
    case CONV_TYPE_LZ77:
        if (cnv_dat->opt < 2 || cnv_dat->opt > 255)
        {
            fprintf(stderr, "ERR: invalid LZ77 parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "l%d", cnv_dat->opt);
        break;
    
    //---------------------
    // LZ77拡張圧縮
    case CONV_TYPE_LZ77EX:
        if (cnv_dat->opt < 2 || cnv_dat->opt > 255)
        {
            fprintf(stderr, "ERR: invalid LZ77 parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "L%d", cnv_dat->opt);
        break;
    
    //---------------------
    // ハフマン圧縮
    case CONV_TYPE_HUFFMAN:
        if (cnv_dat->opt != 4 && cnv_dat->opt != 8)
        {
            fprintf(stderr, "ERR: invalid HUFFMAN parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "h%d", cnv_dat->opt);
        break;
    
    //---------------------
    // レンジコーダ圧縮
    case CONV_TYPE_RANGECODER:
        cnv_dat->dest_buf  = nitroCompMalloc( cnv_dat->src_size );
        cnv_dat->dest_size = RCACompWrite( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
        goto finish;
    
    //---------------------
    // LZ-ハフマン複合圧縮
    case CONV_TYPE_LH:
        cnv_dat->dest_buf  = nitroCompMalloc( cnv_dat->src_size );
        cnv_dat->dest_size = LHCompWrite( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
        goto finish;
        
    //---------------------
    // LZ-RangeCoder複合圧縮
    case CONV_TYPE_LRC:
        cnv_dat->dest_buf  = nitroCompMalloc( cnv_dat->src_size );
        cnv_dat->dest_size = LRCCompWrite( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
        goto finish;
    
    //---------------------
    // 無圧縮
    case CONV_TYPE_NON:
        {
            cnv_dat->dest_size = cnv_dat->src_size;
            cnv_dat->dest_buf = cnv_dat->src_buf;
            
            // 4バイトアラインチェック
            if (cnv_dat->alignment)
            {
                cnv_dat->dest_size = ROUNDUP(cnv_dat->src_size, cnv_dat->alignment);
            }
            else if (cnv_dat->textout_width >= 2)
            {
                cnv_dat->dest_size = ROUNDUP( cnv_dat->src_size, cnv_dat->textout_width & TEXT_WIDTH_MASK );
            }
        }
        return 0;
    
    //---------------------
    // 解凍
    case CONV_TYPE_UNCOMPRESS:
        {
            s32 ret;
            if ( cnv_dat->src_size == 4 )
            {
                cnv_dat->dest_size = 0;
                cnv_dat->dest_buf  = NULL;
                goto finish;
            }
            cnv_dat->dest_size = nitroGetDecompFileSize( cnv_dat->src_buf );
            cnv_dat->dest_buf  = (void*)malloc( cnv_dat->dest_size + 0x20 ); // アライメント分余分に確保
            memset(cnv_dat->dest_buf, 0, cnv_dat->dest_size + 0x20);
            
            switch ( *cnv_dat->src_buf & 0xF0 )
            {
            case LH_CODE_HEADER:
                ret = LHCompRead( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
                break;
            case LRC_CODE_HEADER:
                ret = LRCCompRead( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
                break;
            case DIFF_CODE_HEADER:
            case LZ_CODE_HEADER:
            case HUFF_CODE_HEADER:
            case RL_CODE_HEADER:
                ret = nitroDecompress( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf, 1 );
                break;
            default:
                ret = -1;
            }
            if ( ret < 0 )
            {
                fprintf(stderr, "ERR: Can't extract this file.\n");
                return -1;
            }
        }
        goto finish;
    
    default:
        fprintf(stderr, "ERR: invalid convert type\n");
        return -1;
    }

    // 圧縮後ファイルのバッファ領域確保
    cnv_dat->dest_buf = nitroCompMalloc( cnv_dat->src_size );

    // データ圧縮
    cnv_dat->dest_size = nitroCompress(cnv_dat->src_buf,
                                       cnv_dat->src_size,
                                       cnv_dat->dest_buf, cmd_str, cnv_dat->header_flg);
    // 4バイトアラインチェック
    if (cnv_dat->alignment)
    {
        cnv_dat->dest_size = ROUNDUP(cnv_dat->dest_size, cnv_dat->alignment);
    }

finish:
    // テキスト形式出力の型によるアラインチェック
    if (cnv_dat->textout_width >= 2)
    {
        cnv_dat->dest_size = ROUNDUP( cnv_dat->dest_size, cnv_dat->textout_width & TEXT_WIDTH_MASK );
    }
    
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         verify_data

  Description:  圧縮データの展開をおこない、元データと一致するかどうか
                確認する。

  Arguments:    cnv_dat 変換情報データ

  Returns:      0 ベリファイOK
                -1 ベリファイNG
 *---------------------------------------------------------------------------*/
static int verify_data( t_conv_data * cnv_dat )
{
    // 圧縮データの展開チェック
    char   *decomp_buf;
    ulong   decomp_size;
    
    if ( ! cnv_dat->verify_flg )
    {
        return 0;
    }
    
    decomp_buf = nitroCompMalloc( cnv_dat->src_size );
    decomp_size = nitroDecompress( cnv_dat->dest_buf, cnv_dat->dest_size, decomp_buf, 1 );
    
    if ( matchingCheck(cnv_dat->src_buf, cnv_dat->src_size, decomp_buf, decomp_size) == 0 )
    {
        fprintf(stderr, "ERR: data convert error\n");
        return -1;
    }
    return 0;
}


