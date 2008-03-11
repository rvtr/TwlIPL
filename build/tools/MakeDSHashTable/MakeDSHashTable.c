/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tools - MakeDSHashTable
  File:     MakeDSHashTable.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include <sysmenu/dht/dht_format.h>
#include <twl/os/common/format_rom.h>
#include <digest/include/sha1.h>
#include <acsign/include/acsign.h>

#define TARGET_DIR  "."
#define OUTPUT_FILE "DSHashTable.bin"

const BOOL DebugMode = TRUE;    // acsignで必要

// データベース署名用秘密鍵
const u8 g_privKey_DER[ 0x261 ] = {
    0x30, 0x82, 0x02, 0x5d, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xc7, 0xf4, 0x1d, 0x27, 0x3f,
    0xe8, 0xae, 0x7f, 0x7c, 0xbc, 0x9a, 0xae, 0x09, 0x8d, 0x19, 0x26, 0x2e, 0x90, 0x04, 0x03, 0x13,
    0x93, 0xbc, 0xb2, 0xe0, 0x8b, 0x1f, 0x85, 0x48, 0xf5, 0xf6, 0x94, 0x69, 0x3e, 0x05, 0x1b, 0x97,
    0x85, 0x44, 0x6d, 0xa3, 0xcd, 0xa8, 0x01, 0xfe, 0xdc, 0x77, 0x5d, 0xd1, 0xb1, 0x36, 0x21, 0xfc,
    0x80, 0xe8, 0xa6, 0x0e, 0xde, 0x59, 0x76, 0xca, 0x96, 0xcc, 0x87, 0x4c, 0xc3, 0x90, 0xc6, 0x3b,
    0xc8, 0x17, 0x9d, 0x2d, 0xac, 0x45, 0xbc, 0xa7, 0x15, 0xb2, 0xe3, 0xd7, 0x76, 0xfa, 0x09, 0x8c,
    0x55, 0x09, 0x22, 0x95, 0x4b, 0xe7, 0xde, 0xc0, 0x82, 0xf2, 0x02, 0x1a, 0x8a, 0x42, 0x38, 0x7f,
    0xbb, 0x31, 0xd6, 0xa8, 0x36, 0xdc, 0x8d, 0x2c, 0x42, 0x56, 0x51, 0xc1, 0xa3, 0x30, 0x21, 0x30,
    0xef, 0x06, 0x72, 0x0c, 0xa6, 0x55, 0xb7, 0x4f, 0x30, 0x35, 0x1b, 0x02, 0x03, 0x01, 0x00, 0x01,
    0x02, 0x81, 0x80, 0x17, 0x47, 0x76, 0x75, 0x71, 0x5d, 0x57, 0xa3, 0x84, 0x14, 0x39, 0x35, 0xf5,
    0xea, 0xb5, 0x78, 0xda, 0x86, 0xed, 0xd0, 0xa5, 0xe0, 0xd7, 0x61, 0x43, 0xff, 0x2e, 0x86, 0x47,
    0xfe, 0x4a, 0xdc, 0xf0, 0x7b, 0xc6, 0x00, 0x76, 0x5d, 0x8f, 0x9f, 0xec, 0x28, 0x2f, 0x5d, 0x3c,
    0x1c, 0xd2, 0xd0, 0xdb, 0x6e, 0xf4, 0x4c, 0x74, 0xa7, 0x55, 0x6e, 0xaf, 0xe0, 0x77, 0x2d, 0xfb,
    0x5c, 0x1f, 0x27, 0x94, 0xa1, 0x64, 0x4d, 0xaa, 0x17, 0x5f, 0x96, 0x55, 0x86, 0x23, 0x94, 0xf1,
    0x46, 0x52, 0x91, 0x49, 0x8e, 0x47, 0x89, 0xf3, 0xa0, 0xe9, 0x71, 0xe4, 0x7f, 0x9a, 0x85, 0xf1,
    0x20, 0xfd, 0x71, 0xf8, 0xc0, 0x97, 0x0d, 0x23, 0x16, 0xb0, 0x80, 0x1c, 0x57, 0xf1, 0xbd, 0x6d,
    0xa0, 0x0f, 0xea, 0x34, 0xed, 0x98, 0xee, 0xf0, 0x38, 0xba, 0x92, 0x6b, 0x51, 0x21, 0xae, 0xf3,
    0x6a, 0x75, 0x91, 0x02, 0x41, 0x00, 0xf0, 0xde, 0xc2, 0x64, 0x7f, 0xdc, 0x84, 0xc5, 0xf6, 0x36,
    0x71, 0xa6, 0x12, 0x07, 0x14, 0x0c, 0xfa, 0x86, 0x4f, 0x2b, 0x37, 0xd6, 0x11, 0xdf, 0xac, 0xe7,
    0xe6, 0xd3, 0x4c, 0x0c, 0xff, 0x70, 0x5f, 0x75, 0x18, 0x5d, 0xd4, 0xee, 0x25, 0xda, 0x66, 0xac,
    0xe1, 0x51, 0x59, 0xe4, 0xc5, 0xcb, 0x6a, 0x87, 0xa6, 0x2e, 0x19, 0xb0, 0xca, 0x44, 0x4c, 0xac,
    0xbf, 0x48, 0x2c, 0xb6, 0x6b, 0xc5, 0x02, 0x41, 0x00, 0xd4, 0x83, 0x68, 0x59, 0x55, 0x37, 0x8b,
    0xab, 0xa1, 0x10, 0xbb, 0xc2, 0xac, 0x91, 0x69, 0x1a, 0x3b, 0x19, 0xd5, 0x11, 0xa0, 0x06, 0x8b,
    0xcd, 0x35, 0xc7, 0x26, 0x19, 0xe4, 0xd3, 0xf0, 0xb9, 0xa6, 0x98, 0xe4, 0xf2, 0xaf, 0x81, 0x08,
    0x44, 0x90, 0xb8, 0x25, 0x54, 0x22, 0x6f, 0xe6, 0x34, 0xfb, 0x84, 0x8f, 0x80, 0x4e, 0x13, 0xee,
    0xe7, 0x6d, 0xda, 0xf4, 0x71, 0x83, 0x75, 0xcb, 0x5f, 0x02, 0x40, 0x2e, 0x3e, 0x36, 0x07, 0xe1,
    0x5c, 0x4e, 0xc9, 0x4a, 0xb9, 0xad, 0x1b, 0xfa, 0xfe, 0x0c, 0xe5, 0x68, 0xfb, 0x7c, 0x1b, 0x89,
    0xfe, 0xb9, 0x33, 0xbe, 0x18, 0x4f, 0x82, 0x65, 0x69, 0x61, 0x69, 0x0b, 0x8a, 0x89, 0x81, 0x0a,
    0x2e, 0x26, 0x6a, 0x45, 0xa8, 0x0f, 0x21, 0xf5, 0x26, 0x2c, 0xab, 0x1e, 0xea, 0xa8, 0x06, 0xd5,
    0xbb, 0xd1, 0x39, 0x47, 0x97, 0x2a, 0xaa, 0x0a, 0x9b, 0x83, 0x19, 0x02, 0x41, 0x00, 0xad, 0x53,
    0x70, 0x2c, 0x4c, 0x40, 0xbd, 0x7e, 0x02, 0x23, 0xef, 0xbf, 0xdb, 0x97, 0xcd, 0xad, 0x43, 0x3e,
    0xd2, 0x75, 0x49, 0x9b, 0x8d, 0x32, 0x96, 0x06, 0x13, 0xa1, 0x52, 0xed, 0x39, 0x8d, 0x71, 0xbf,
    0x8a, 0x1c, 0x15, 0xe0, 0x9b, 0xa9, 0xb0, 0x8f, 0x40, 0x82, 0xda, 0x83, 0x81, 0xb2, 0xfc, 0xe1,
    0x25, 0x59, 0x74, 0x54, 0xf5, 0xa9, 0x74, 0x03, 0xfd, 0x13, 0x08, 0x31, 0x6e, 0xe9, 0x02, 0x41,
    0x00, 0xa9, 0xfa, 0xfc, 0x14, 0x12, 0x56, 0xf5, 0x27, 0xe7, 0xe6, 0x9e, 0x21, 0x21, 0xbc, 0x78,
    0x30, 0xf8, 0xdc, 0x2a, 0xca, 0xbf, 0x5f, 0x2a, 0x48, 0x5b, 0x63, 0xe3, 0x55, 0xde, 0xb8, 0x96,
    0x04, 0x2d, 0xfb, 0xda, 0x88, 0x74, 0x68, 0x7f, 0xb3, 0x69, 0xef, 0xe7, 0x7d, 0x7a, 0x6c, 0x0b,
    0x07, 0x7f, 0x1b, 0x8e, 0xc1, 0xa7, 0x8a, 0xdb, 0x20, 0x6c, 0xea, 0xe9, 0xda, 0x88, 0x14, 0x6e,
    0x95
};

/*
    自家製scandir
*/
static int alphasort( const struct dirent** a, const struct dirent** b )
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

static int scandir( const char* path, struct dirent ***plist, int (*filter)( const struct dirent* ), int(*compar)( const struct dirent**, const struct dirent** ) )
{
    int nums;
    DIR* dp;
    struct dirent* entry;

    chdir(path);
    if ( !(dp = opendir( "." )) )
    {
        printf("Failed to open the target directory \"%s\".\n", path);
        return -1;
    }
    if ( !plist || !compar )
    {
        printf("plist = %p, compar = %p\n", plist, compar);
        return -1;
    }

    *plist = NULL;
    nums = 0;

    while ( (entry = readdir( dp )) != NULL )
    {
        if (filter && filter(entry) == 0)
        {
            continue;
        }
        *plist = realloc(*plist, ++nums * sizeof(struct dirent*));
        if ( !(*plist) )
        {
            printf("Failed to realloc %d bytes.\n", nums * sizeof(struct dirent*));
            return -1;
        }
        (*plist)[nums-1] = malloc(sizeof(struct dirent));
        memcpy((*plist)[nums-1], entry, sizeof(struct dirent));
    }
    closedir( dp );
    if ( nums && *plist && compar )
    {
        qsort( *plist, nums, sizeof(struct dirent*), (int(*)(const void *, const void *))compar );
    }
    return nums;
}
/*
    個別版HMAC-SHA1
*/
typedef struct HMACSHA1Context
{
    SHA1Context  sha1_ctx;
    u8           key[64];
    u32          keylen;
}
HMACSHA1Context;

void HMACSHA1Init( HMACSHA1Context *ctx, const void *key, u32 keylen )
{
    u8  ipad[64];
    int i;

    if ( ctx == NULL || ( keylen > 0 && key == NULL ) ) // key == NULL && keylen == 0 を認める
        return; // FAILED

    /* 鍵がブロック長よりも長い場合、ハッシュ値を鍵とする. */
    if ( keylen > 64 )
    {
        SHA1Context sha1_ctx;
        SHA1Reset( &sha1_ctx );
        SHA1Input( &sha1_ctx, key, keylen );
        SHA1Result( &sha1_ctx, ctx->key );
        ctx->keylen = 20;
    }
    else
    {
        memcpy( ctx->key, key, keylen );
        ctx->keylen = keylen;
    }
    /* 鍵とipadのXOR */
    for ( i = 0; i < ctx->keylen; i++ )
    {
        ipad[i] = (u8)(ctx->key[i] ^ 0x36);
    }
    /* 鍵のパディング部分とipadのXOR */
    for ( ; i < 64; i++ )
    {
        ipad[i] = 0x00 ^ 0x36;
    }

    /* メッセージとの結合とハッシュ値の計算 */
    SHA1Reset( &ctx->sha1_ctx );
    SHA1Input( &ctx->sha1_ctx, ipad, 64 );
}

void HMACSHA1Update( HMACSHA1Context *ctx, const void *data, u32 len )
{
/*
    if ( ctx == NULL || ( len > 0 && data == NULL ) )
        return; // FAILED
    if ( len == 0 && data == NULL ) // 何もすることはない
        return; // SUCCESS
*/
    if ( ctx == NULL || len == 0 || data == NULL )  // void型なのでまとめる
        return;

    /* メッセージとの結合とハッシュ値の計算 */
    SHA1Input( &ctx->sha1_ctx, data, len );
}

void HMACSHA1GetHash( HMACSHA1Context *ctx, u8* md )
{
    u8 opad[64];
    u8 temp[20];
    int i;

    if ( ctx == NULL || md == NULL )
        return; // FAILED

    /* メッセージとの結合とハッシュ値の計算 */
    SHA1Result( &ctx->sha1_ctx, temp );

    /* 鍵とopadのXOR */
    for ( i = 0; i < ctx->keylen; i++ )
    {
        opad[i] = (u8)(ctx->key[i] ^ 0x5c);
    }
    /* 鍵のパディング部分とopadのXOR */
    for ( ; i < 64; i++ )
    {
        opad[i] = 0x00 ^ 0x5c;
    }
    /* ハッシュ値との結合とハッシュ値の計算 */
    SHA1Reset( &ctx->sha1_ctx );
    SHA1Input( &ctx->sha1_ctx, opad, 64 );
    SHA1Input( &ctx->sha1_ctx, temp, 20 );
    SHA1Result( &ctx->sha1_ctx, md );
}

/*
    シーク済みfpからsize分をHMAC-SHA1計算
*/
#define BUFSIZE 0x1000
static BOOL FileHMACSHA1( HMACSHA1Context *ctx, FILE* fp, int size )
{
    while ( size > 0 )
    {
        int len = size < BUFSIZE ? size : BUFSIZE;
        u8  buffer[BUFSIZE];
        if ( fread(buffer, len, 1, fp) < 1 )
        {
            return FALSE;
        }
        HMACSHA1Update(ctx, buffer, len);
        size -= len;
    }
    return TRUE;
}

/*
    ROMファイルっぽいものかどうかの判定
*/
static int srl_filter( const struct dirent* dir )
{
    struct stat buf;

    if ( strlen( dir->d_name ) > 11 )
    {
        return 0;
    }
    if ( stat(dir->d_name, &buf) )
    {
        printf("Cannot stat \"%s\".\n", dir->d_name);
        return 0;
    }
    if ( S_ISDIR(buf.st_mode) )
    {
        return 0;
    }
    if ( buf.st_size < sizeof(ROM_Header) )
    {
        return 0;
    }

    return 1;
}
/*
    ROMヘッダhpがまともそうか判定する
*/
static BOOL check_code( const ROM_Header_Short* hp )
{
    if ( !isprint(hp->game_code[0]) || !isprint(hp->game_code[1]) || !isprint(hp->game_code[2]) || !isprint(hp->game_code[3])
      || !isprint(hp->maker_code[0]) || !isprint(hp->maker_code[1]) )
    {
        return FALSE;
    }
    return TRUE;
}
/*
    ROMヘッダhpからオーバーレイのサイズを計算する
*/
static int get_overlay_nums( const ROM_Header_Short* hp )
{
    return hp->main_ovt_size / sizeof(struct ROM_OVT);
}
/*
    ROMヘッダhpからオーバーレイのサイズを計算する
*/
static BOOL get_overlay_info( FILE* fp, int offset, int no, int* pOff, int* pLen )
{
    struct ROM_FAT fat;
    if ( !fp )
    {
        return FALSE;
    }
    if ( fseek( fp, offset + no * sizeof(ROM_FAT), SEEK_SET ) )
    {
        printf("Cannot seek No.%d of FAT\n", no);
        return FALSE;
    }
    if ( fread( &fat, sizeof(struct ROM_FAT), 1, fp ) < 1 )
    {
        printf("Cannot read No.%d of FAT\n", no);
        return FALSE;
    }
    if ( pOff )
    {
        *pOff = fat.top.offset;
    }
    if ( pLen )
    {
        *pLen = fat.bottom.offset - fat.top.offset;
    }
    return TRUE;
}


/*
    filenameを解析して結果をfoutに書き出す
*/
static BOOL output_hash( FILE* fout, const char* filename )
{
    FILE* fin = fopen(filename, "rb");
    ROM_Header_Short header;
    DHTDatabase db;
    HMACSHA1Context ctx;
    const u8 key[] = DHT_HMAC_KEY;
    int nums;

    if ( !fin )
    {
        printf("Cannot open the target file \"%s\".\n", filename);
        return FALSE;
    }
    // まずヘッダを読む
    if ( fread( &header, sizeof(ROM_Header_Short), 1, fin ) < 1 )
    {
        fseek( fin, 0, SEEK_SET );
        printf("Cannot read the target file \"%s\". %d bytes only.\n", filename, fread( &header, 1, sizeof(ROM_Header_Short), fin ));
        fclose(fin);
        return FALSE;
    }
    // DSのROMか簡易チェック
    if ( !check_code( &header ) )
    {
        fclose(fin);
        return FALSE;
    }

    // db clear
    memset(&db, 0, sizeof(db));
    memcpy(db.game_code, header.game_code, sizeof(db.game_code));
    db.rom_version = header.rom_version;

    // ハッシュ準備 (その１)
    HMACSHA1Init( &ctx, key, sizeof(key) );

    // ヘッダのハッシュ計算
    HMACSHA1Update( &ctx, (u8*)&header, DHT_DS_HEADER_SIZE );

    // ARM9の読み込み
    fseek(fin, header.main_rom_offset, SEEK_SET);
    if ( !FileHMACSHA1( &ctx, fin, header.main_size ) )
    {
        printf("Cannot read the ARM9 Static for %.4s(%x).\n", header.game_code, header.rom_version);
        fclose(fin);
        return FALSE;
    }

    // ARM7の読み込み
    fseek(fin, header.sub_rom_offset, SEEK_SET);
    if ( !FileHMACSHA1( &ctx, fin, header.sub_size ) )
    {
        printf("Cannot read the ARM7 Static for %.4s(%x).\n", header.game_code, header.rom_version);
        fclose(fin);
        return FALSE;
    }

    // ハッシュ計算 (その１)
    HMACSHA1GetHash( &ctx, db.hash[0] );

    // オーバーレイ編
    nums = get_overlay_nums(&header);
    if (nums)
    {
        int i;
        int total_sectors;

        // ハッシュ準備 (その２)
        HMACSHA1Init( &ctx, key, sizeof(key) );

        // OVTの読み込み
        fseek(fin, (long)header.main_ovt_offset, SEEK_SET);
        if ( !FileHMACSHA1( &ctx, fin, header.main_ovt_size ) )
        {
            printf("Cannot read the OVT for %.4s(%x).\n", header.game_code, header.rom_version);
            fclose(fin);
            return FALSE;
        }

        // FAT(オーバーレイ部分)の読み込み
        fseek(fin, (long)header.fat_offset, SEEK_SET);
        if ( !FileHMACSHA1( &ctx, fin, nums * sizeof(ROM_FAT) ) )
        {
            printf("Cannot read the part of FAT for %.4s(%x).\n", header.game_code, header.rom_version);
            fclose(fin);
            return FALSE;
        }

        // 各オーバーレイの読み込み
        total_sectors = 0;
        for (i = 0; i < nums; i++)
        {
            int max_sectors = (DHT_OVERLAY_MAX/512 - total_sectors) / (nums-i);
            int offset;
            int length;
            if ( !get_overlay_info( fin, (long)header.fat_offset, i, &offset, &length ) )
            {
                printf("Cannot get overlay info No.%d for %.4s(%x).\n", i, header.game_code, header.rom_version);
                fclose(fin);
                return FALSE;
            }
            length = (length + 511) / 512;  // bytes -> sectors
            if ( length > max_sectors )
            {
                length = max_sectors;
            }
            fseek(fin, offset, SEEK_SET);
            if ( !FileHMACSHA1( &ctx, fin, length*512 ) )
            {
                printf("Cannot read the overlay No.%d for %.4s(%x).\n", i, header.game_code, header.rom_version);
                fclose(fin);
                return FALSE;
            }
            total_sectors += length;
            if (total_sectors > DHT_OVERLAY_MAX/512)
            {
                printf("PROGRAM ERROR!!\n");
                exit(1);
            }
        }

        // ハッシュ計算 (その２)
        HMACSHA1GetHash( &ctx, db.hash[1] );
    }
    else
    {
        memset(db.hash[1], 0, 20);
    }
    fclose(fin);

    // 書き込み
    if ( fwrite(&db, sizeof(db), 1, fout) < 1 )
    {
        printf("Cannot write the DHTDatabase for %.4s(%x).\n", header.game_code, header.rom_version);
        return FALSE;
    }

    return TRUE;
}
/*
    ヘッダをfoutに書き出す
*/
static BOOL output_header( FILE* fp, int nums )
{
    DHTHeader header;
    header.magic_code = DHT_MAGIC_CODE;
    header.nums = nums;
    if (nums)
    {
        SHA1Context ctx;
        u8 hash[20];
        u8 sign[128];
        int size = sizeof(DHTDatabase) * nums;

        printf("Total hash size: %d bytes.\n", nums * sizeof(DHTDatabase));
        if ( SHA1Reset( &ctx ) )
        {
            printf("Cannot reset SHA1 context.\n");
            return FALSE;
        }
        SHA1Input(&ctx, (u8*)&header.nums, sizeof(header.nums));
        fseek(fp, sizeof(DHTHeader), SEEK_SET);
        while (size > 0)
        {
            int len = size < BUFSIZE ? size : BUFSIZE;
            u8  buffer[BUFSIZE];
            if ( fread(buffer, len, 1, fp) < 1 )
            {
                printf("Cannot calc SHA1 the while file.\n");
                return FALSE;
            }
            SHA1Input(&ctx, buffer, len);
            size -= len;
        }
        if ( SHA1Result( &ctx, hash ) )
        {
            printf("Cannot get SHA1 result for the whole file.\n");
            return FALSE;
        }
        if( !ACSign_Encrypto( sign, g_privKey_DER, hash, sizeof(hash) ) )
        {
            printf("Cannot get signature.\n");
            return FALSE;
        }
        memcpy(header.sign, sign, 128);
    }
    else
    {
        memset(header.sign, 0, 128);
    }
    rewind(fp);
    if (fwrite(&header, sizeof(header), 1, fp) < 1)
    {
        printf("Cannot write the header. %d bytes only.\n", fwrite( &header, 1, sizeof(header), fp ));
        return FALSE;
    }
    return TRUE;
}
/*
    CSVファイルを作成する
*/
static BOOL make_hash( const char* path )
{
    struct dirent **namelist;
    int file_nums;
    int i;
    FILE* fp;

    if ( !path )
    {
        return FALSE;
    }

    // 出力ファイルの作成
    fp = fopen( OUTPUT_FILE, "w+b" );
    if ( !fp )
    {
        printf("Cannot open the output file \"./%s\".\n", OUTPUT_FILE);
        return FALSE;
    }
    printf("Ouptput file: ./%s\n", OUTPUT_FILE);

    // ファイルリストの取得
    file_nums = scandir( path, &namelist, srl_filter, alphasort );
    if ( file_nums <= 0 )
    {
        printf("No target file.\n");
        fclose(fp);
        return FALSE;
    }
    printf("Found %d files.\n", file_nums);

    // 仮ヘッダ作成
    output_header( fp, 0 );

    // ファイル解析＆出力
    for ( i = 0; i < file_nums; i++ )
    {
        output_hash( fp, namelist[i]->d_name );
        free( namelist[i] );
        if ( i % 500 == 0) printf("\n");
        if ( i %  10 == 0) printf(".");
    }
    free( namelist );
    printf("\n\n");

    // 本ヘッダ作成
    output_header( fp, file_nums );

    fclose( fp );
    return TRUE;
}

int main( int argc, char** argv )
{
    printf("\n");

    if (argc == 2 && argv[1] )
    {
        make_hash(argv[1]);
    }
    else
    {
        make_hash(TARGET_DIR);
    }

    printf("\nDone.\n");

    return 0;
}
