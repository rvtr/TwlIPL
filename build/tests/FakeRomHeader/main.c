/*---------------------------------------------------------------------------*
  Project:  TwlFirmware - tools - mastering
  File:     mastering.c

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>                      // strcasecmp()
#include    <getopt.h>                      // getopt()
#include    <sys/stat.h>                    // stat()
#include    <stdarg.h>
#include    <conio.h>
#include    <twl/types.h>
#include    <twl/os/common/format_rom.h>
#include    <acsign/include/acsign.h>
#include    "revision.h"
#include    "keys.h"

#define	CARD_LATENCY_MASK					0x083f1fff
#define	CARD_MROM_GAME_LATENCY				0x00010017
#define	CARD_MROM_SECURE_HW_LATENCY			0x001808f8
#define	CARD_MROM_SECURE_SW_LATENCY			0x051e
#define	CARD_1TROM_GAME_LATENCY				0x00010657
#define	CARD_1TROM_SECURE_HW_LATENCY		0x081808f8
#define	CARD_1TROM_SECURE_SW_LATENCY		0x0d7e


/*---------------------------------------------------------------------------*


 宣言 & 定数


 *---------------------------------------------------------------------------*/

// -- 内部パラメータ

// CRC (makerom.TWL/misc.cとmakerom.TWL/format_rom_private.hからコピペ)
#define  CRC16_INIT_VALUE    (0xffff)
#define  CALC_CRC16_SIZE     (0x15e)

// ファイルコピー用バッファ
#define  BUFSIZE             (16*1024)
static u8 buf[ BUFSIZE ];

// -- Context

typedef struct _SContext
{
    // オプション
    BOOL bPlatform;
    BOOL bSignFlag;
    BOOL bDevFlag;
    BOOL bVerFlag;
    BOOL bDebugger;
    BOOL bCheckCard;
	int  verNum;
    BOOL bMROM;
    BOOL bNTR;
    
    FILE *ifp;
    FILE *ofp;
}
SContext;

// -- internal function
static BOOL iMain( SContext *pContext );
u16 CalcCRC16(u16 start, u8 *data, int size);
BOOL SignRomHeader( ROM_Header *prh );
BOOL DecryptSignRomHeader( ROM_Header *prh );
static void SetRomSpeedByIndex(ROM_Header * header, tRomSpeedType idx);

/*---------------------------------------------------------------------------*


 グローバル変数


 *---------------------------------------------------------------------------*/

BOOL DebugMode = FALSE;

/*---------------------------------------------------------------------------*


 おおもとの本体


 *---------------------------------------------------------------------------*/

void usage()
{
    printf( "-----------------------------------------------------------------------------\n" );
#ifdef FOR_RED
    printf( "Usage: fakeRomHeader.exe input_file output_file\n" );
    printf( "  input_file  : a ROM data file (generated by makerom.TWL).\n" );
    printf( "  output_file : a destination file.\n" );
#else
    printf( "Usage: RomVersionChanger.exe  input_file  output_file  new_ROM_version\n" );
    printf( "  input_file      : a source ROM file.\n" );
    printf( "  output_file     : a destination ROM file.\n" );
    printf( "  new_ROM_version : a new ROM version.\n" );
#endif
    printf( "\nOption:\n" );
    printf( "-h    : print help only.\n" );
#ifdef FOR_RED
    printf( "-p    : write invalid platform code in a ROM Header.\n" );
    printf( "-s    : negate flag for the signature in a ROM Header.\n" );
    printf( "-d    : negate a new developer encrypt flag, and assert an old one.\n" );
    printf( "-v NUM: change ROM version in a ROM Header.\n" );
    printf( "-m    : [Only NTR limited ROM] Rom speed type replace 1TROM from MROM.\n" );
    printf( "-D    : assert a disable flag of debugger alalysis.\n" );
    printf( "-c    : assert a check(inspection) card flag.\n" );
    printf( "-N    : skip appending signature [using for NTR limited ROM]" );
#endif
    printf( "-f    : force to overwrite a output_file.\n" );
	printf( "-----------------------------------------------------------------------------\n" );
}

int main(int argc, char *argv[])
{
    SContext        context;
    char           *pSrc, *pDst;
    int             opt;
    extern char    *optarg;
    extern int      optind, opterr;
    struct stat     st;
    BOOL            bResult;
    BOOL            bForceOverwrite = FALSE;

    printf( "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
#ifdef FOR_RED
    printf( "         FakeRomHeader [%s-%s]\n", SDK_REVISION, IPL_REVISION );
#else
    printf( "         RomVersionChanger [%s-%s]\n", SDK_REVISION, IPL_REVISION );
#endif
    printf( "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );

    // context の初期化
    memset( &context, 0, sizeof(SContext) );

    // オプション
#ifdef FOR_RED
    while( (opt = getopt(argc, argv, "hpsdmv:DcfN")) >= 0 )
#endif
    while( (opt = getopt(argc, argv, "hf")) >= 0 )
    {
        switch( opt )
        {
            case 'h':
                usage();
                return 0;
            break;

#ifdef FOR_RED
            case 'p':
                context.bPlatform = TRUE;
            break;

            case 's':
                context.bSignFlag = TRUE;
            break;
            
            case 'd':
                context.bDevFlag = TRUE;
            break;

            case 'v':
                context.bVerFlag = TRUE;
                context.verNum   = strtol(optarg, NULL, 16);
            break;

            case 'm':
                context.bMROM = TRUE;
            break;
            
            case 'D':
                context.bDebugger = TRUE;
            break;
            
            case 'c':
                context.bCheckCard = TRUE;
            break;

            case 'N':
                context.bNTR = TRUE;
            break;
#endif
            case 'f':
                bForceOverwrite = TRUE;
            break;
            
            default:            // オプション引数が指定されていないときにも実行される
                usage();
                fprintf( stdout, "error: illegal option\n" );
                exit(1);
            break;
        }
    }
    argc = argc - optind;
    argv = argv + optind;

    // 引数処理
#ifdef FOR_RED
    if( argc != 2 )
    {
        usage();
        printf( "error arguments\n" );
        exit(1);
    }
    else
    {
        pSrc = argv[0];
        pDst = argv[1];
    }
#else
    if( argc != 3 )     // 一般版はROMバージョンを引数として受け取る
    {
        usage();
        printf( "error arguments\n" );
        exit(1);
    }
    else
    {
        pSrc = argv[0];
        pDst = argv[1];
        context.bVerFlag = TRUE;
        context.verNum   = strtol(argv[2], NULL, 16);
    }
#endif

    // ROMバージョンが1バイトで収まるかチェック
    if( context.bVerFlag )
    {
        if( (context.verNum < 0) || (255 < context.verNum) )
        {
            printf("Error: ROM version (arg 3) must be within [0, 0xFF].\n");
            return -1;
        }
    }

    printf( "input_file:  %s\n", pSrc );
    printf( "output_file: %s\n", pDst );
    printf( "\n" );

    // 入力ファイルと出力ファイル名が同じならダメ
    if( (strlen(pSrc) == strlen(pDst)) && (memcmp(pSrc, pDst, strlen(pSrc)) == 0) )
    {
        printf( "The name of output_file must be different from the one of input_file\n" );
        exit(1);
    }

    // 出力ファイルが存在するとき上書きするかどうかを確認する
    if( !bForceOverwrite && stat( pDst, &st )==0 )
    {
        char  ans;
        printf( "The %s already exists. Overwrite it? (y or n) ", pDst );
        //fscanf( stdin, "%c", &ans );
        ans = (char)getch();    // リターンキーなし
        printf( "%c\n", ans );  // getch では入力が表示されないので自分で表示する必要あり
        printf( "\n" );
        if( !(ans=='y') && !(ans=='Y') )
        {
            printf( "mastering is aborted.\n" );
            exit(1);
        }
    }

    // ファイルを開く
    context.ifp = fopen(pSrc, "rb");
    context.ofp = fopen(pDst, "wb+");
    if( !(context.ifp) || !(context.ofp) )
    {
        printf( "fopen failed.\n" );
        bResult = FALSE;
        goto FINALIZE;
    }

    // 本処理
    bResult = iMain( &context );

// 終了処理
FINALIZE:

    if( !bResult )
    {
        printf( "\n" );
        printf( "error.\n" );
    }
    else
    {
        printf( "\n" );
        printf( "succeeded.\n" );
    }
    if( context.ifp ) fclose(context.ifp);  // 資源の解放
    if( context.ofp ) fclose(context.ofp);

    if( (!bResult) && (stat( pDst, &st ) == 0) )
    {
        unlink( pDst );                 // 出力ファイルを削除する
    }
    return ((bResult)?0:1);
}

/*---------------------------------------------------------------------------*

 ROMヘッダの偽装

 *---------------------------------------------------------------------------*/

static BOOL iMain( SContext *pContext )
{
    ROM_Header rh;
    int        size;
    
    // ROMヘッダをリード
    fseek( pContext->ifp, 0, SEEK_SET );
    if( sizeof(ROM_Header) != fread( &rh, 1, sizeof(ROM_Header), pContext->ifp ) )
    {
        printf( "fread failed in reading ROM Header\n" );
        return FALSE;
    }

#ifndef FOR_RED
    // 署名の検証
    if( !DecryptSignRomHeader(&rh) )
    {
        printf( "Decrypt Sign:     Failed\n" );
        return FALSE;
    }
    printf( "Decrypt Sign:     Succeeded\n" );

    // NTRのときエラー(よくありそうなので仰々しく出力)
    if( rh.s.platform_code == PLATFORM_CODE_NTR )
    {
        printf("\n");
        printf("******************** Error *******************\n");
        printf("*                                              *\n");
        printf("*  NTR Limited tytle is not supported.         *\n");
        printf("*                                              *\n");
        printf("************************************************\n");
        printf("\n");
        return FALSE;
    }

    // Hybridのとき警告
    if( rh.s.platform_code == PLATFORM_CODE_TWL_HYBLID)
    {
        printf("\n");
        printf("******************** Warning *******************\n");
        printf("*                                              *\n");
        printf("*  Platform is TWL/NTR Hybrid.                 *\n");
        printf("*                                              *\n");
        printf("*  Clone-boot application is not supported.    *\n");
        printf("*                                              *\n");
        printf("************************************************\n");
        printf("\n");
    }
    printf("Initial Code:     %c%c%c%c\n", 
            rh.s.game_code[0], rh.s.game_code[1], rh.s.game_code[2], rh.s.game_code[3]);
#endif //#ifndef FOR_RED

    // ROMヘッダをいじる
    {
        if( pContext->bPlatform )
        {
            printf( "Platform Code:    0x%02x -> 0x01\n", rh.s.platform_code );
            rh.s.platform_code = 0x01;
        }
        
        if( pContext->bSignFlag )
        {
            printf( "Enable Signature: 0x%02x -> 0x00\n", rh.s.enable_signature );
            rh.s.enable_signature = 0x0;
        }
        
        if( pContext->bDevFlag )
        {
            printf( "Old Dev. Encrypt: 0x%02x -> 0x01x\n", rh.s.developer_encrypt_old );
            printf( "Oew Dev. Encrypt: 0x%02x -> 0x00x\n", rh.s.exFlags.developer_encrypt );
            rh.s.developer_encrypt_old     = 0x1;
            rh.s.exFlags.developer_encrypt = 0x0;
        }
		
        if( pContext->bVerFlag )
        {
            u8  old = rh.s.rom_version;
			rh.s.rom_version = (u8)(pContext->verNum & 0xFF);
            printf( "ROM version:      0x%02x -> 0x%02X\n", old, rh.s.rom_version );
		}
		
		if( pContext->bMROM )
		{
            if( rh.s.platform_code != 0x00 )
            {
                printf( "ERROR: This option is only for NTR Limited." );
                return FALSE;
            }
            //if( (rh.s.game_cmd_param & CARD_LATENCY_MASK) != CARD_MROM_GAME_LATENCY )
            //{
            //    printf( "ERROR: This option is only for MROM Setting" );
            //    return FALSE;
            //}
            SetRomSpeedByIndex( &rh, ONETROM );
            printf( "ROM Speed Type: **** -> 1TROM\n" );
        }
        
        if( pContext->bDebugger )
        {
            if( rh.s.disable_debug == 0 )
            {
                rh.s.disable_debug = 0x1;
                printf( "Debugger analysis: Enable -> Disable\n" );
            }
            else
            {
                printf( "Debugger analysis: Originally Disable\n" );
            }
        }
        
        if( pContext->bCheckCard )
        {
            if( rh.s.inspect_card == 0 )
            {
                rh.s.inspect_card = 0x1;
                printf( "Check Card Flag:  Disable -> Enable\n" );
            }
            else
            {
                printf( "Check Card Flag:  Originally Enable\n" );
            }
        }
    }
    
    // ヘッダCRC計算
    printf("Header CRC:       0x%04X -> ", rh.s.header_crc16);
    rh.s.header_crc16 = CalcCRC16( CRC16_INIT_VALUE, (u8*)&rh, CALC_CRC16_SIZE );
    printf("0x%04X\n", rh.s.header_crc16);
    
    // 再署名
    if( !(pContext->bMROM) && !(pContext->bNTR) )        // NTR専用オプションのときは署名しない
    {
        if( !SignRomHeader( &rh ) )
        {
            printf( "Encrypt Sign:     Failed\n" );
            printf("\n*** Error: Failed to encrypt the sigunature. ***\n\n");
            return FALSE;
        }
        printf( "Encrypt Sign:     Succeeded\n" );
    }
    else
    {
        printf( "Encrypt Sign:     Skip\n" );
    }

    // ファイルをコピる
    fseek( pContext->ifp, 0, SEEK_END );
    size = ftell( pContext->ifp );
    fseek( pContext->ifp, 0, SEEK_SET );
    fseek( pContext->ofp, 0, SEEK_SET );
    while( size > 0 )
    {
        int len = (BUFSIZE < size)?BUFSIZE:size;
        
        if( len != fread( buf, 1, len, pContext->ifp ) )
        {
            printf( "fread failed in a copying file\n" );
            return FALSE;
        }
        if( len != fwrite( buf, 1, len, pContext->ofp ) )
        {
            printf( "fwrite failed in a copying file\n" );
            return FALSE;
        }
        
        size -= len;
    }
    
    // ROMヘッダを上書き
    fseek( pContext->ofp, 0, SEEK_SET );
    if( sizeof(ROM_Header) != fwrite( &rh, 1, sizeof(ROM_Header), pContext->ofp ) )
    {
        printf( "fwrite failed in a copying file\n" );
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*

 CRCの計算

 *---------------------------------------------------------------------------*/

static u16 crc16_table[16] = {
    0x0000, 0xCC01, 0xD801, 0x1400,
    0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401,
    0x5000, 0x9C01, 0x8801, 0x4400
};

u16 CalcCRC16(u16 start, u8 *data, int size)
{
    u16     r1;
    u16     total = start;

    while (size-- > 0)
    {
        // 下位4bit
        r1 = crc16_table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ crc16_table[*data & 0xf];

        // 上位4bit
        r1 = crc16_table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ crc16_table[(*data >> 4) & 0xf];

        data++;
    }
    return total;
}


/*---------------------------------------------------------------------------*

 ROMヘッダの署名

 *---------------------------------------------------------------------------*/

BOOL SignRomHeader( ROM_Header *prh )
{
	SignatureData   signSrc;						// 署名のもととなるダイジェスト値
	u8              signDst[ RSA_KEY_LENGTH ];		// 署名の格納先Tmpバッファ
	u8              decryptBlock[ RSA_KEY_LENGTH ];	// 署名を解除後ブロックバッファ
	BOOL            result;
	int             pos;

	// ROMヘッダのダイジェストを算出(先頭から証明書領域の直前までが対象)
	ACSign_DigestUnit(
		signSrc.digest,
		prh,
		(u32)&(prh->certificate) - (u32)prh		// this->pRomHeader はマネージヒープ上にあるので実アドレスを取得できない
	);

#ifdef KEY_USER
    printf( "Signature Key:    USER\n" );
#endif
#ifdef KEY_SYSTEM
    printf( "Signature Key:    SYSTEM\n" );
#endif
#ifdef KEY_SECURE
    printf( "Signature Key:    SECURE\n" );
#endif
#ifdef KEY_LAUNCHER
    printf( "Signature Key:    LAUNCHER\n" );
#endif

	// ダイジェストに署名をかける
	result = ACSign_Encrypto( signDst, g_devPrivKey_DER, &signSrc, sizeof(SignatureData) ); 
	if( !result )
	{
        printf( "\n*** Error: failed to ACSign_Encrypto. ***\n\n" );
		return FALSE;
	}

	// 署名を解除してダイジェストと一致するかベリファイする
	result = ACSign_Decrypto( decryptBlock, g_devPubKey_DER, signDst, RSA_KEY_LENGTH );
	for( pos=0; pos < RSA_KEY_LENGTH; pos++ )
	{
		if( decryptBlock[pos] == 0x0 )			// 解除後ブロックから実データをサーチ
			break;
	}
	if( !result || (memcmp( &signSrc, &(decryptBlock[pos+1]), sizeof(SignatureData) ) != 0) )
	{
        printf( "\n*** Error: failed to verify the signature. ***\n\n" );
		return FALSE;
	}

	// ROMヘッダに署名を上書き
	memcpy( prh->signature, signDst, RSA_KEY_LENGTH );

	return TRUE;
} // ECSrlResult RCSrl::signRomHeader(void)

// 署名を外す
BOOL DecryptSignRomHeader( ROM_Header *prh )
{
	u8     original[ RSA_KEY_LENGTH ];	// 署名外した後のデータ格納先
	s32    pos = 0;						// ブロックの先頭アドレス
	u8     digest[ DIGEST_SIZE_SHA1 ];	// ROMヘッダのダイジェスト

	// <データの流れ>
	// (1) 公開鍵で復号した結果(ブロック)をローカル変数(original)に格納
	// (2) ブロックから余分な部分を取り除いて引数(pDst)にコピー

    // 鍵選択
#ifdef KEY_USER
    printf( "Decryption Key:   USER\n" );
#endif
#ifdef KEY_SYSTEM
    printf( "Decryption Key:   SYSTEM\n" );
#endif
#ifdef KEY_SECURE
    printf( "Decryption Key:   SECURE\n" );
#endif
#ifdef KEY_LAUNCHER
    printf( "Decryption Key:   LAUNCHER\n" );
#endif

	// 署名の解除 = 公開鍵で復号
	if( !ACSign_Decrypto( original, g_devPubKey_DER, prh->signature, RSA_KEY_LENGTH ) )
	{
        printf( "\n*** Error: failed to decrypt the signature. ***\n\n" );
		return FALSE;
	}
	// 署名前データを復号後ブロックからゲット
	for( pos=0; pos < (RSA_KEY_LENGTH-2); pos++ )   // 本来ブロックの先頭は0x00だが復号化の内部処理によって消える仕様
	{
		// 暗号ブロック形式 = 0x00, BlockType, Padding, 0x00, 実データ
		if( original[pos] == 0x00 )                               // 実データの直前の0x00をサーチ
		{
			break;
		}
	}
	// ベリファイ
	// ROMヘッダのダイジェストを算出(先頭から証明書領域の直前までが対象)
	ACSign_DigestUnit( digest,	prh, (u32)&(prh->certificate) - (u32)prh );
	if( memcmp( &(original[pos+1]), digest, DIGEST_SIZE_SHA1 ) != 0 )
	{
        printf( "\n*** Error: failed to verify the signature. ***\n\n" );
		return FALSE;
	}
	return TRUE;
}


/*---------------------------------------------------------------------------*

 ROM Speed Typeの設定

 *---------------------------------------------------------------------------*/

static ROM_LT romSpeedTypeLatencyTable[] = {
    {CARD_1TROM_GAME_LATENCY, CARD_1TROM_SECURE_HW_LATENCY, CARD_1TROM_SECURE_SW_LATENCY},
    {CARD_MROM_GAME_LATENCY, CARD_MROM_SECURE_HW_LATENCY, CARD_MROM_SECURE_SW_LATENCY},
};

static void SetRomSpeedByIndex(ROM_Header * header, tRomSpeedType idx)
{
    ROM_LT *l = &romSpeedTypeLatencyTable[idx];

    header->s.game_cmd_param &= ~CARD_LATENCY_MASK;
    header->s.game_cmd_param |= l->game_latency;
    header->s.secure_cmd_param &= ~CARD_LATENCY_MASK;
    header->s.secure_cmd_param |= l->secure_hw_latency;
    header->s.secure_cmd_latency = l->secure_sw_latency;
}


// end of file
