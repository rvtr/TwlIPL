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
// �^��`
//---------------------------------------------------------------------------
// ���k�^�C�v
typedef enum
{
    CONV_TYPE_NON,                     // �ϊ��Ȃ�
    CONV_TYPE_DIFF,                    // �����t�B���^
    CONV_TYPE_RUNLENGTH,               // ���������O�X������
    CONV_TYPE_LZ77,                    // LZ77���k
    CONV_TYPE_HUFFMAN,                 // �n�t�}�����k
    CONV_TYPE_UNCOMPRESS,              // ��
    CONV_TYPE_LZ77EX,                  // �g��LZ77���k
    CONV_TYPE_LH,                      // LZ77-�n�t�}���������k
    CONV_TYPE_LRC,                     // LZ77-RangeCoder�������k
    CONV_TYPE_RANGECODER               // �����W�R�[�_
}
t_conv_type;

// ���k�f�[�^���\����
typedef struct
{
    t_conv_type type;                  // ���k�^�C�v
    uint        opt;                   // ���k�p�����[�^
    ulong       src_size;              // ���k�O�f�[�^�T�C�Y
    ulong       dest_size;             // ���k��f�[�^�T�C�Y
    uchar       *src_buf;              // ���k�O�f�[�^�o�b�t�@
    uchar       *dest_buf;             // ���k��f�[�^�o�b�t�@
    uchar       textout_width;         // �e�L�X�g�o�͂��邩�ǂ����̃t���O���T�C�Y
    uchar       alignment;             // �o�̓t�@�C���T�C�Y�̃A���C�����g
    uchar       header_flg :1;         // �w�b�_����t�����邩�ǂ����̃t���O
    uchar       silent_flg :1;         // �W���o�͂փ��b�Z�[�W���o�͂��邩�ǂ����̃t���O
    uchar       verify_flg :1;         // ���k�t�@�C����W�J����ۂׂ̂�t�@�C�������Ȃ����ǂ����̃t���O
}
t_conv_data;

typedef enum
{
    OPT_NONE,        // �����ȃI�v�V����
    OPT_VERSION,     // �o�[�W�����\��(-v)
    OPT_SILENT,      // �W���o�͗}�~(-s)
    OPT_LOWER_TEXT,  // �������ł̃e�L�X�g�o��(-t)
    OPT_UPPER_TEXT,  // �啶���ł̃e�L�X�g�o��(-T)
    OPT_ALIGN,       // �A���C�����g�w��(-A4|8|16|32)
    OPT_HEADER,      // �w�b�_�t��(-H)
    OPT_OUTPUT,      // �o�̓t�@�C���w��(-o)
    OPT_EXTRACT,     // ��(-x)
    OPT_DIFF_FILTER, // �����t�B���^(-d)
    OPT_RUN_LENGTH,  // ���������O�X���k(-r)
    OPT_LZ77,        // ��LZ77���k(-l)
    OPT_LZ77EX,      // LZ77���k(-lex)
    OPT_LH,          // LH���k(-lh)
    OPT_LRC,         // LRC���k(-lrc)
    OPT_HUFFMAN,     // �n�t�}�����k(-h4|8)
    
    OPT_RANGECODER, // �����W�R�[�_���k(�f�o�b�O�ł̂�) (-R)
    OPT_VERIFY,     // �x���t�@�C      (�f�o�b�O�ł̂�) (-D)
    OPT_TEST,       // �e�X�g          (�f�o�b�O�ł̂�) (--TEST)
}
t_opt;




//---------------------------------------------------------------------------
// �v���g�^�C�v�錾
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
// �G���f�B�A���ϊ�
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
// ���C���֐�
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

    // �f�[�^�̏�����
    memset(&cnv_dat, 0, sizeof(cnv_dat));

    // �R�}���h�I�v�V�����̉��
    if (parse_opt(argc, argv, &cnv_dat, &in_fname, &out_fname) != 0)
    {
        exit(1);
    }

    // ���̓t�@�C�����o�b�t�@�֓ǂݍ���
    if (load_file(in_fname, &cnv_dat) != 0)
    {
        exit(1);
    }

    // �t�@�C���̈��k
    if (convert_data(&cnv_dat) != 0)
    {
        exit(1);
    }
    
    // ���k�f�[�^�̃x���t�@�C
    if (verify_data( &cnv_dat ) != 0)
    {
        exit(1);
    }
    
    // �o�b�t�@����o�̓t�@�C���ւ̏�������
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
// �p�X�����񂩂�t�@�C���������̃|�C���^���擾���A�g���q�����
// @param path �p�X
// @return �t�@�C�����̃|�C���^
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
// ���̓t�@�C��������o�̓t�@�C�����𐶐�����֐�
// @param input_fname ���̓t�@�C����
// @param cnv_dat �ϊ����
// @return �o�̓t�@�C�����ւ̃|�C���^
//---------------------------------------------------------------------------
static char *make_output_fname(const char *input_fname, const t_conv_data * cnv_dat)
{
    char   *out_fname, *edit_tmp;

    if (input_fname == NULL)
    {
        return NULL;
    }

    // �������̈�̊m��
    if ((out_fname = (char *)malloc(strlen(input_fname) + 0x10)) == NULL)
    {
        fprintf(stderr, "internal err: malloc fail!\n");
        exit(1);
    }

    strcpy(out_fname, input_fname);
    edit_tmp = cut_fname(out_fname);

    // ���k�������t�@�C�����ɕt��
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
    // �g���q��t��
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
// ���̃I�v�V�����̃^�C�v����
// @param  opt ���ʂ������I�v�V����������
// @return �I�v�V�����̃^�C�v
//---------------------------------------------------------------------------
t_opt
get_opt_type( const char* opt )
{
    if ( strncmp( opt, "--", 2 ) == 0 )
    {
    }
    else
    {
        // �I�v�V�������
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
// �R�}���h�p�����[�^���
// @param argc �p�����[�^��
// @param argv �p�����[�^�z��
// @retval cnv_dat ���k���f�[�^
// @retval in_fname ���̓t�@�C����
// @retval out_fname �o�̓t�@�C����
// @return 0 �t�@�C���̏������ݐ���
//        -1 �t�@�C���̏������݃G���[
//---------------------------------------------------------------------------

static int parse_opt(int argc, char *argv[], t_conv_data * cnv_dat,
                     char **in_fname, char **out_fname)
{
    int     i = 1;
    t_conv_type type;
    int     opt_num;
    char   *cmd_string;
#define SYNTAX_CHECK(exp) do { if (!(exp)) { usage(cmd_string); return -1; } } while(0)

    // �R�}���h���擾
    cmd_string = cut_fname(argv[0]);
    
    // ��ԏ�����
    cnv_dat->type = CONV_TYPE_NON;
    cnv_dat->header_flg = 0;
    cnv_dat->textout_width = 0;
    cnv_dat->silent_flg = 0;
    cnv_dat->alignment  = 0;
    cnv_dat->verify_flg = 0;
    *out_fname = NULL;
    *in_fname = NULL;
    
    // �I�v�V�������
    while (i < argc)
    {
        t_opt opt;
        
        if (argv[i][0] != '-')
        {
            SYNTAX_CHECK(*in_fname == NULL);    // �Q�d�ǂ݃`�F�b�N
            
            // ���̓t�@�C�����擾
            *in_fname = argv[i];
            i++;
            continue;
        }
        
        opt = get_opt_type( argv[i] );
        
        // �n�C�t���t�I�v�V�������
        switch ( opt )
        {
        //--------------------
        // �o�[�W�����\���I�v�V����'v'(����0)
        case OPT_VERSION:
            SYNTAX_CHECK(argv[i][2] == '\0'); // �������`�F�b�N
            
            version_info();
            return -1;
            
        //--------------------
        // �W���o�͖����I�v�V����'s' (����0)
        case OPT_SILENT:
            SYNTAX_CHECK(argv[i][2] == '\0');   // �������`�F�b�N
            SYNTAX_CHECK(!cnv_dat->silent_flg); // �Q�d�ǂ݃`�F�b�N
            
            cnv_dat->silent_flg = 1;
            break;
        
#ifdef _DEBUG
        //--------------------
        // �x���t�@�C�I�v�V����'D' (����0)
        case OPT_VERIFY:
            SYNTAX_CHECK(argv[i][2] == '\0');   // �������`�F�b�N
            SYNTAX_CHECK(!cnv_dat->verify_flg); // �Q�d�ǂ݃`�F�b�N
            cnv_dat->verify_flg = 1;
            break;
#endif
        
        //--------------------
        // C����e�L�X�g�o�̓I�v�V����'t' (����0-1) (���g���G���f�B�A��)
        case OPT_LOWER_TEXT:
            SYNTAX_CHECK(cnv_dat->textout_width == 0);  // �Q�d�ǂ݃`�F�b�N
            
            if (strlen(argv[i]) == 2)  // �p�����[�^�������ꍇ�̓o�C�g�P�ʂŏ�������
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
        // C����e�L�X�g�o�̓I�v�V����'T' (����0-1) (�r�b�O�G���f�B�A��)
        case OPT_UPPER_TEXT:
            SYNTAX_CHECK(cnv_dat->textout_width == 0);  // �Q�d�ǂ݃`�F�b�N
            
            if (strlen(argv[i]) == 2)  // �p�����[�^�������ꍇ�̓o�C�g�P�ʂŏ�������
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
        // �o�̓t�@�C���T�C�Y�A���C���w��'A4|8|16|32' (����0)
        case OPT_ALIGN:
            SYNTAX_CHECK(strlen(argv[i]) <= 4 ); // �������`�F�b�N
            SYNTAX_CHECK( cnv_dat->alignment == 0 ); // �Q�d�ǂ݃`�F�b�N
            
            cnv_dat->alignment = atoi(&argv[i][2]);
            SYNTAX_CHECK( cnv_dat->alignment == 4 || cnv_dat->alignment == 8 || cnv_dat->alignment == 16 || cnv_dat->alignment == 32 );
            
            break;
            
        //--------------------
        // �W�J��f�[�^�̐擪�Ƀf�[�^�T�C�Y��t������I�v�V����'H' (����0)
        case OPT_HEADER:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // �������`�F�b�N
            SYNTAX_CHECK(!cnv_dat->header_flg); // �Q�d�ǂ݃`�F�b�N
            
            cnv_dat->header_flg = 1;
            break;
            
        //--------------------
        // �o�̓t�@�C���w��I�v�V���� 'o' (����1)
        case OPT_OUTPUT:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // �������`�F�b�N
            SYNTAX_CHECK(*out_fname == NULL);   // �Q�d�ǂ݃`�F�b�N
            SYNTAX_CHECK(i + 1 < argc);         // �p�����[�^�`�F�b�N
            
            *out_fname = argv[++i];
            break;
        
        //--------------------
        // �𓀎w�� 'x'(����0)
        case OPT_EXTRACT:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // �������`�F�b�N
            
            type = CONV_TYPE_UNCOMPRESS;
            opt_num = 0;
            goto common;
        
        //--------------------
        // �����t�B���^�w�� 'd'(����1)
        case OPT_DIFF_FILTER:
            type = CONV_TYPE_DIFF;
            goto common_opt1;
            
        //--------------------
        // ���������O�X���k�w��'r'(����0)
        case OPT_RUN_LENGTH:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // �������`�F�b�N
            
            type = CONV_TYPE_RUNLENGTH;
            opt_num = 0;
            goto common;
            
#ifdef _DEBUG
        //--------------------
        // �����W�R�[�_���k�w��'R'(����0)
        case OPT_RANGECODER:
            SYNTAX_CHECK(strlen(argv[i]) == 2); // �������`�F�b�N
            
            type = CONV_TYPE_RANGECODER;
            opt_num = 0;
            goto common;
#endif
        //--------------------
        // ��LZ77���k"-l"(����0)
        case OPT_LZ77:
            SYNTAX_CHECK( (argv[i][2] == '\0') || isdigit(argv[i][2]) );
            type = CONV_TYPE_LZ77;
            opt_num = 0;
            cnv_dat->opt = 2;
            
          #if defined( FOR_NITRO )
            // ���Ƀp�����[�^������A���̓t�@�C�����łȂ���Γǂݔ�΂�
            if ( i + 2 < argc && argv[i + 1][0] != '-' && 
                 argv[i + 1][1] == '\0' && atoi(&argv[i + 1][0]) != 0 )
            {
                i++;
            }
          #endif
            goto common;
            
        //--------------------
        // LZ77���k"-lex"(����0)
        case OPT_LZ77EX:
            type = CONV_TYPE_LZ77EX;
            opt_num = 0;
            cnv_dat->opt = 2; // �����J�n�ʒu�̓I�v�V�����Ŏw��ł��Ȃ����ɂ���
            goto common;            
            
        //--------------------
        // LH���k"-lh"(����0)
        case OPT_LH:
            type = CONV_TYPE_LH;
            opt_num = 0;
            goto common;
        
        //--------------------
        // LRC���k"-lrc"(����0)
        case OPT_LRC:
            type = CONV_TYPE_LRC;
            opt_num = 0;
            goto common;
        
        //--------------------
        // �n�t�}�����k�w��'h'(����1)
        case OPT_HUFFMAN:
            type = CONV_TYPE_HUFFMAN;
            goto common_opt1;
            
        //--------------------
        // ���k�����w�蕔���̋��ʏ���
        common_opt1:
            if (strlen(argv[i]) == 2)
                // �����񂪏I�����Ă���Ύ��̈������p�����[�^�ƂȂ�
            {
                opt_num = 1;
            }
            else
                // �����񂪑����Ă���΂��̒l���p�����[�^�ƂȂ�
            {
                cnv_dat->opt = atoi(&argv[i][2]);
                SYNTAX_CHECK(cnv_dat->opt != 0);        // �p�����[�^��1�ȏ�̐����ł��邩�̃`�F�b�N
                opt_num = 0;
            }
            // don't break;
        common:
            SYNTAX_CHECK(cnv_dat->type == CONV_TYPE_NON);       // �Q�d�ǂ݃`�F�b�N
            
            cnv_dat->type = type;
            
            if (opt_num == 0)
            {
                break;
            }
            
            // ����������ꍇ
            SYNTAX_CHECK(i + 1 < argc); // �p�����[�^�`�F�b�N
            i++;
            SYNTAX_CHECK(argv[i][0] != '-');    // �p�����[�^�Ƀn�C�t���Ŏn�܂�Ȃ����̃`�F�b�N
            
            cnv_dat->opt = atoi(argv[i]);
            
            SYNTAX_CHECK(cnv_dat->opt != 0);    // �p�����[�^��1�ȏ�̐����ł��邩�̃`�F�b�N
            
            break;
            
        //--------------------
        default:
            usage(cmd_string);
            return -1;
        }
        i++;
    }
    
    SYNTAX_CHECK(*in_fname != NULL);   // ���̓t�@�C�����w�肳�ꂽ���ǂ����̃`�F�b�N
    
    // �o�̓t�@�C�����̎�������
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
    // �𓀃I�v�V�������w�肳�ꂽ�ꍇ�̃I�v�V�����`�F�b�N
    {
        SYNTAX_CHECK(cnv_dat->alignment  == 0); // �A���C�����g�̎w��͕s�\�ł�
        SYNTAX_CHECK(cnv_dat->header_flg == 0); // �w�b�_�̒ǉ��I�v�V�����͕s�\�ł�
    }
    
    return 0;
#undef SYNTAX_CHECK
}



//---------------------------------------------------------------------------
// ���̓t�@�C���̓ǂݍ��݊֐�
// 
// @param fname ���̓t�@�C����
// @retval ���k���\���̂ւ̃|�C���^
// @return 0 �t�@�C���̓ǂݍ��ݐ���
//        -1 �t�@�C���̓ǂݍ��݃G���[
//---------------------------------------------------------------------------

static int load_file(const char *fname, t_conv_data * cnv_dat)
{
    FILE   *in_file;

    // �t�@�C���̃I�[�v��
    if ((in_file = fopen(fname, "rb")) == NULL)
    {
        fprintf(stderr, "ERR: could not open file: %s\n", fname);
        return -1;
    }

    // �t�@�C���̃T�C�Y�擾
    fseek(in_file, 0, SEEK_END);
    if ((cnv_dat->src_size = ftell(in_file)) < 0)
    {
        fprintf(stderr, "ERR: file error\n");
        return -1;
    }

    // ���̓t�@�C�����o�b�t�@�֓ǂݍ���
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

    // �S�o�C�g�A���C���܂ł�0�Ŗ��߂�
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
// �o�̓t�@�C���ւ̏������݊֐�
// 
// @param fname �o�̓t�@�C����
// @retval ���k���\���̂ւ̃|�C���^
// @return 0 �t�@�C���̏������ݐ���
//        -1 �t�@�C���̏������݃G���[
//---------------------------------------------------------------------------

static int save_file(const char *fname, const t_conv_data * cnv_dat)
{
    FILE   *out_file;

    // �������݃f�[�^�̃`�F�b�N
    if (cnv_dat->dest_buf == NULL || cnv_dat->dest_size < 0)
    {
        fprintf(stderr, "ERR: convert error\n");
        return -1;
    }

    // �t�@�C���̃I�[�v��
    if ((out_file = fopen(fname, "wb")) == NULL)
    {
        fprintf(stderr, "ERR: could not open file: %s\n", fname);
        return -1;
    }

    // �o�b�t�@���o�̓t�@�C���֏�������
    if (cnv_dat->textout_width != 0)
    {
        // �e�L�X�g�o��
        if ( fwrite_text(cnv_dat->dest_buf, cnv_dat->dest_size, cnv_dat->textout_width, out_file, fname) 
             != cnv_dat->dest_size )
        {
            fprintf(stderr, "ERR: write error\n");
            return -1;
        }
    }
    else
    {
        // �o�C�i���o��
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
// C����t�H�[�}�b�g�ł̃e�L�X�g�o��
// @param buf �f�[�^�o�b�t�@
// @param size �f�[�^�T�C�Y
// @param fp �o�̓t�@�C���|�C���^
// @param name �萔���x����
// @return 0 �t�@�C���̏������ݐ���
//        -1 �t�@�C���̏������݃G���[
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

    // �萔���̌���
    if ((const_name_buf = (char *)malloc( strlen(name) + 1 )) == NULL)
    {
        fprintf(stderr, "internal err: malloc fail!\n");
        return -1;
    }

    strcpy(const_name_buf, name);
    const_name_ptr = cut_fname(const_name_buf);
    size = (size + width - 1) / width;

    // �f�[�^�̏�������
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
// �f�[�^�̕ϊ�
// @retval cnv_dat �ϊ����f�[�^
// @return 0 �ϊ�����
//        -1 �ϊ����s
//---------------------------------------------------------------------------

static int convert_data(t_conv_data * cnv_dat)
{
    char    cmd_str[16];

    // �R�}���h�X�g�����O�̐���
    switch (cnv_dat->type)
    {
    //---------------------
    // �����t�B���^
    case CONV_TYPE_DIFF:
        if (cnv_dat->opt != 8 && cnv_dat->opt != 16)
        {
            fprintf(stderr, "ERR: invalid DIFF parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "d%d", cnv_dat->opt);
        break;

    //---------------------
    // ���������O�X���k
    case CONV_TYPE_RUNLENGTH:
        // ���������O�X������
        strcpy(cmd_str, "r");
        break;

    //---------------------
    // LZ77���k
    case CONV_TYPE_LZ77:
        if (cnv_dat->opt < 2 || cnv_dat->opt > 255)
        {
            fprintf(stderr, "ERR: invalid LZ77 parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "l%d", cnv_dat->opt);
        break;
    
    //---------------------
    // LZ77�g�����k
    case CONV_TYPE_LZ77EX:
        if (cnv_dat->opt < 2 || cnv_dat->opt > 255)
        {
            fprintf(stderr, "ERR: invalid LZ77 parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "L%d", cnv_dat->opt);
        break;
    
    //---------------------
    // �n�t�}�����k
    case CONV_TYPE_HUFFMAN:
        if (cnv_dat->opt != 4 && cnv_dat->opt != 8)
        {
            fprintf(stderr, "ERR: invalid HUFFMAN parameter %d\n", cnv_dat->opt);
            return -1;
        };
        sprintf(cmd_str, "h%d", cnv_dat->opt);
        break;
    
    //---------------------
    // �����W�R�[�_���k
    case CONV_TYPE_RANGECODER:
        cnv_dat->dest_buf  = nitroCompMalloc( cnv_dat->src_size );
        cnv_dat->dest_size = RCACompWrite( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
        goto finish;
    
    //---------------------
    // LZ-�n�t�}���������k
    case CONV_TYPE_LH:
        cnv_dat->dest_buf  = nitroCompMalloc( cnv_dat->src_size );
        cnv_dat->dest_size = LHCompWrite( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
        goto finish;
        
    //---------------------
    // LZ-RangeCoder�������k
    case CONV_TYPE_LRC:
        cnv_dat->dest_buf  = nitroCompMalloc( cnv_dat->src_size );
        cnv_dat->dest_size = LRCCompWrite( cnv_dat->src_buf, cnv_dat->src_size, cnv_dat->dest_buf );
        goto finish;
    
    //---------------------
    // �����k
    case CONV_TYPE_NON:
        {
            cnv_dat->dest_size = cnv_dat->src_size;
            cnv_dat->dest_buf = cnv_dat->src_buf;
            
            // 4�o�C�g�A���C���`�F�b�N
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
    // ��
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
            cnv_dat->dest_buf  = (void*)malloc( cnv_dat->dest_size + 0x20 ); // �A���C�����g���]���Ɋm��
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

    // ���k��t�@�C���̃o�b�t�@�̈�m��
    cnv_dat->dest_buf = nitroCompMalloc( cnv_dat->src_size );

    // �f�[�^���k
    cnv_dat->dest_size = nitroCompress(cnv_dat->src_buf,
                                       cnv_dat->src_size,
                                       cnv_dat->dest_buf, cmd_str, cnv_dat->header_flg);
    // 4�o�C�g�A���C���`�F�b�N
    if (cnv_dat->alignment)
    {
        cnv_dat->dest_size = ROUNDUP(cnv_dat->dest_size, cnv_dat->alignment);
    }

finish:
    // �e�L�X�g�`���o�͂̌^�ɂ��A���C���`�F�b�N
    if (cnv_dat->textout_width >= 2)
    {
        cnv_dat->dest_size = ROUNDUP( cnv_dat->dest_size, cnv_dat->textout_width & TEXT_WIDTH_MASK );
    }
    
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         verify_data

  Description:  ���k�f�[�^�̓W�J�������Ȃ��A���f�[�^�ƈ�v���邩�ǂ���
                �m�F����B

  Arguments:    cnv_dat �ϊ����f�[�^

  Returns:      0 �x���t�@�COK
                -1 �x���t�@�CNG
 *---------------------------------------------------------------------------*/
static int verify_data( t_conv_data * cnv_dat )
{
    // ���k�f�[�^�̓W�J�`�F�b�N
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


