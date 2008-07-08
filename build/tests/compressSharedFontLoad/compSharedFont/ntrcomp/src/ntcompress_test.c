/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     ntcompress_test.c

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

#if defined( _DEBUG )

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include "nitroCompLib.h"
#include "multipleCompLib.h"

#define MAX_DATA_SIZE_EX    0x02000000       // 32MB
#define MAX_DATA_SIZE       0x00100000       // 1MB
//#define MAX_DATA_SIZE   0x100            // 256B

#define LOOP_CNT        100
#define LOOP_CNT_EX     4

#if 0
#define TestReport(...)     (void)0
#else
#define TestReport  printf
#endif

#define ASSERT assert

static u8* s_original;

static int TestInit( void );
static void TestLZ8( void );
static void TestRL8( void );
static void TestHuffman( void );

static struct
{
    u64     x;                         // 乱数値
    u64     mul;                       // 乗数
    u64     add;                       // 加算する数
}
sRandContext;

static void InitRand32_(u64 seed)
{
    sRandContext.x   = seed;
    sRandContext.mul = (1566083941LL << 32) + 1812433253LL;
    sRandContext.add = 2531011;
}

static u32 Rand32_(u32 max)
{
    sRandContext.x = sRandContext.mul * sRandContext.x + sRandContext.add;
    
    // 引数maxが定数ならばコンパイラにより最適化される。
    if (max == 0)
    {
        return (u32)(sRandContext.x >> 32);
    }
    else
    {
        return (u32)((((sRandContext.x >> 32) * (max >> 1)) >> 32) + (max >> 1));
    }
}



static int compare( const u8 a[], const u8 b[], int size )
{
    int i;
    int equivalent_flag = 1;
    for ( i = 0; i < size; ++i )
    {
        if ( a[i] != b[i] )
        {
            equivalent_flag = 0;
            break;
        }
    }
    return equivalent_flag;
}


//--------------------------------------------------
// 元データ生成用関数
//--------------------------------------------------

// LZ,Huffmanで圧縮されやすい元データを作成する。
static u32 makeOriginalDataForLZHuff( u32 seed )
{
    u32 fsize;
    u32 i;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    (void)Rand32_( 0 );
    fsize = Rand32_( MAX_DATA_SIZE );
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    for ( i = 0; i < fsize; ++i )
    {
        s_original[ i ] = (u8)(Rand32_(0x100) & 0xF0);
    }
    return fsize;
}


// RL圧縮されやすい元データを作成する。
static u32 makeOriginalDataForRL( u32 seed )
{
    u8 val;
    u32 i;
    u32 fsize;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    (void)Rand32_( 0 );
    fsize = Rand32_( MAX_DATA_SIZE );
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    for ( i = 0; i < fsize; ++i )
    {
        if ( (i % 4) == 0 )
        {
            val = (u8)Rand32_(0x100);
        }
        s_original[ i ] = val;
    }
    return fsize;
}

// 圧縮されにくい元データを作成する。
static u32 makeOriginalDataRand( u32 seed )
{
    u32 i;
    u32 fsize;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    (void)Rand32_( 0 );
    fsize = Rand32_( MAX_DATA_SIZE );
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    for ( i = 0; i < fsize; ++i )
    {
        s_original[ i ] = (u8)Rand32_(0x100);
    }
    return fsize;
}


// すべてが単一値の元データを作成する。
static u32 makeOriginalDataMonoValue( u32 seed )
{
    u32 i;
    u32 fsize;
    u8  val;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    (void)Rand32_( 0 );
    fsize = Rand32_( MAX_DATA_SIZE );
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    val = (u8)Rand32_(0x100);
    for ( i = 0; i < fsize; ++i )
    {
        s_original[ i ] = val;
    }
    return fsize;
}

// LZ,Huffmanで圧縮されやすい元データを作成する。
static u32 makeOriginalDataForLZHuffEx( u32 seed )
{
    u32 i;
    u32 fsize;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    fsize = Rand32_(MAX_DATA_SIZE_EX / 2) + (MAX_DATA_SIZE_EX / 2);
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    for ( i = 0; i < fsize; ++i )
    {
        s_original[ i ] = (u8)(Rand32_(0x100) & 0xF0);
    }
    return fsize;
}


// RL圧縮されやすい元データを作成する。
static u32 makeOriginalDataForRLEx( u32 seed )
{
    u32 i;
    u32 fsize;
    u8  val;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    fsize = Rand32_(MAX_DATA_SIZE_EX / 2) + (MAX_DATA_SIZE_EX / 2);
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    val;
    for ( i = 0; i < fsize; ++i )
    {
        if ( (i % 4) == 0 )
        {
            val = (u8)Rand32_(0x100);
        }
        s_original[ i ] = val;
    }
    return fsize;
}

// 圧縮されにくい元データを作成する。
static u32 makeOriginalDataRandEx( u32 seed )
{
    u32 fsize;
    u32 i;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    fsize = Rand32_(MAX_DATA_SIZE_EX / 2) + (MAX_DATA_SIZE_EX / 2);
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    for ( i = 0; i < fsize; ++i )
    {
        s_original[ i ] = (u8)Rand32_(0x100);
    }
    return fsize;
}


// すべてが単一値の元データを作成する。
static u32 makeOriginalDataMonoValueEx( u32 seed )
{
    u32 fsize;
    u32 i;
    u8  val;
    
    TestReport("seed: %u\n", seed);
    InitRand32_( seed );
    fsize = Rand32_(MAX_DATA_SIZE_EX / 2) + (MAX_DATA_SIZE_EX / 2);
    if ( fsize <= 4 )
    {
        fsize += 5;
    }
    
    val = (u8)Rand32_(0x100);
    for ( i = 0; i < fsize; ++i )
    {
        s_original[ i ] = val;
    }
    return fsize;
}



typedef u32 (*MakeDataFunc)( u32 seed );
#define WATER_MARK   0xDeadBeaf


static void TestLZ8_template( MakeDataFunc makeDataFunc, u32 exFmt, u32 loop_cnt  )
{
    u32 i;
    for ( i = 0; i < loop_cnt; ++i )
    {
        u32 fsize = makeDataFunc( i );
        
        u8* s_compressed   = nitroCompMalloc( fsize );
        u8* s_uncompressed = (u8*)malloc( fsize + 4 );
        u32 sz;
        
        TestReport("size = %d\n", fsize);
        if ( ! exFmt )
        {
            sz = nitroCompress( s_original, fsize, s_compressed, "l2", 0 );
        }
        else
        {
            sz = nitroCompress( s_original, fsize, s_compressed, "L2", 0 );
        }
        
        if ( sz == 0 )
        {
            TestReport( "compress LZ fail size %d\n", fsize );
        }
        else
        {
            TestReport("compress LZ %d -> %d\n", fsize, sz );
            *(u32*)(&s_uncompressed[ fsize ]) = WATER_MARK;
            nitroDecompress( s_compressed, sz, s_uncompressed, 1 );
            ASSERT( compare( s_original, s_uncompressed, (int)fsize ) );
            // 展開時のオーバーアクセスをチェック
            ASSERT( *(u32*)(&s_uncompressed[ fsize ]) == WATER_MARK );
        }
        
        nitroCompFree( s_compressed );
        free( s_uncompressed );
    }
}

static void TestRL8_template( MakeDataFunc makeDataFunc, u32 loop_cnt )
{
    u32 i;
    for ( i = 0; i < loop_cnt; ++i )
    {
        u32 fsize = makeDataFunc( i );
        
        u8* s_compressed   = nitroCompMalloc( fsize );
        u8* s_uncompressed = (u8*)malloc( fsize + 4 );
        u32 sz;
        
        TestReport("size = %d\n", fsize);
        sz = nitroCompress( s_original, fsize, s_compressed, "r", 0 );
        
        if ( sz == 0 )
        {
            TestReport( "compress RL fail size %d\n", fsize );
        }
        else
        {
            TestReport("compress RL %d -> %d\n", fsize, sz );
            *(u32*)(&s_uncompressed[ fsize ]) = WATER_MARK;
            nitroDecompress( s_compressed, sz, s_uncompressed, 1 );
            ASSERT( compare(s_original, s_uncompressed, (int)fsize) );
            // 展開時のオーバーアクセスをチェック
            ASSERT( *(u32*)(&s_uncompressed[ fsize ]) == WATER_MARK );
        }
        
        nitroCompFree( s_compressed );
        free( s_uncompressed );
    }
}

static void TestHuffman_template( MakeDataFunc makeDataFunc, u8 encBit, u32 loop_cnt )
{
    u32 i;
    for (i = 0; i < loop_cnt; ++i )
    {
        u32 fsize = makeDataFunc( i );
        u8* s_compressed   = nitroCompMalloc( fsize );
        u8* s_uncompressed = (u8*)malloc( ((fsize + 0x3) & ~0x3) + 4 );
        u32 sz;
        
        TestReport("size = %d\n", fsize);
        sz = nitroCompress( s_original, fsize, s_compressed, (encBit == 4 )? "h4" : "h8", 0 );
         
        if ( sz == 0 )
        {
            TestReport( "compress Huff fail size %d\n", fsize );
        }
        else
        {
            TestReport("compress Huff %d -> %d\n", fsize, sz );
            // ハフマン展開では末尾の4バイトアラインまではオーバーアクセスされ得る
            // コードになっている
            *(u32*)(&s_uncompressed[ (fsize + 0x3) & ~0x3 ]) = WATER_MARK;
            nitroDecompress( s_compressed, sz, s_uncompressed, 1 );
            ASSERT( compare(s_original, s_uncompressed, (int)fsize) );
            // 展開時のオーバーアクセスをチェック
            ASSERT( *(u32*)(&s_uncompressed[ (fsize + 0x3) & ~0x3 ]) == WATER_MARK );
        }
        nitroCompFree( s_compressed );
        free( s_uncompressed );
    }
}


static void TestLH8_template( MakeDataFunc makeDataFunc, u32 exFmt, u32 loop_cnt  )
{
    u32 i;
    for ( i = 0; i < loop_cnt; ++i )
    {
        u32 fsize = makeDataFunc( i );
        
        u8* s_compressed   = nitroCompMalloc( fsize );
        u8* s_uncompressed = (u8*)malloc( fsize + 4 );
        u32 sz;
        
        TestReport("size = %d\n", fsize);
        sz = LHCompWrite( s_original, fsize, s_compressed );
        
        if ( sz == 0 )
        {
            TestReport( "compress LH fail size %d\n", fsize );
        }
        else
        {
            TestReport("compress LH %d -> %d\n", fsize, sz );
            *(u32*)(&s_uncompressed[ fsize ]) = WATER_MARK;
            LHCompRead( s_compressed, sz, s_uncompressed );
            ASSERT( compare(s_original, s_uncompressed, (int)fsize) );
            // 展開時のオーバーアクセスをチェック
            ASSERT( *(u32*)(&s_uncompressed[ fsize ]) == WATER_MARK );
        }
        
        nitroCompFree( s_compressed );
        free( s_uncompressed );
    }
}


static void TestLRC8_template( MakeDataFunc makeDataFunc, u32 exFmt, u32 loop_cnt  )
{
    u32 i;
    for ( i = 0; i < loop_cnt; ++i )
    {
        u32 fsize = makeDataFunc( i );
        
        u8* s_compressed   = nitroCompMalloc( fsize );
        u8* s_uncompressed = (u8*)malloc( fsize + 4 );
        u32 sz;
        
        TestReport("size = %d\n", fsize);
        sz = LRCCompWrite( s_original, fsize, s_compressed );
        
        if ( sz == 0 )
        {
            TestReport( "compress LRC fail size %d\n", fsize );
        }
        else
        {
            TestReport("compress LRC %d -> %d\n", fsize, sz );
            *(u32*)(&s_uncompressed[ fsize ]) = WATER_MARK;
            LRCCompRead( s_compressed, sz, s_uncompressed );
            ASSERT( compare(s_original, s_uncompressed, (int)fsize) );
            // 展開時のオーバーアクセスをチェック
            ASSERT( *(u32*)(&s_uncompressed[ fsize ]) == WATER_MARK );
        }
        
        nitroCompFree( s_compressed );
        free( s_uncompressed );
    }
}


//--------------------------------------------------
// テストメイン
//--------------------------------------------------

static int TestInit( void )
{
    time_t t;
    s_original = (u8*)malloc( MAX_DATA_SIZE_EX + 4 );
    makeOriginalDataRand( (u32)time( &t ) );
    
    return 0;
}


static void TestLZ8_normal( void )
{
    TestLZ8_template( makeOriginalDataForLZHuff, 0, LOOP_CNT );
}
static void TestLZ8_random( void )
{
    TestLZ8_template( makeOriginalDataRand, 0, LOOP_CNT );
}
static void TestLZ8_mono( void )
{
    TestLZ8_template( makeOriginalDataMonoValue, 0, LOOP_CNT );
}

static void TestLZ8EX_normal( void )
{
    TestLZ8_template( makeOriginalDataForLZHuff, 1, LOOP_CNT );
}
static void TestLZ8EX_random( void )
{
    TestLZ8_template( makeOriginalDataRand, 1, LOOP_CNT );
}
static void TestLZ8EX_mono( void )
{
    TestLZ8_template( makeOriginalDataMonoValue, 1, LOOP_CNT );
}


static void TestRL8_normal( void )
{
    TestRL8_template( makeOriginalDataForRL, LOOP_CNT );
}
static void TestRL8_random( void )
{
    TestRL8_template( makeOriginalDataRand, LOOP_CNT );
}
static void TestRL8_mono( void )
{
    TestRL8_template( makeOriginalDataMonoValue, LOOP_CNT );
}

static void TestHuffman8_normal( void )
{
    TestHuffman_template( makeOriginalDataForLZHuff, 8, LOOP_CNT );
}
static void TestHuffman8_random( void )
{
    TestHuffman_template( makeOriginalDataRand, 8, LOOP_CNT );
}
static void TestHuffman8_mono( void )
{
    TestHuffman_template( makeOriginalDataMonoValue, 8, LOOP_CNT );
}

static void TestHuffman4_normal( void )
{
    TestHuffman_template( makeOriginalDataForLZHuff, 4, LOOP_CNT );
}
static void TestHuffman4_random( void )
{
    TestHuffman_template( makeOriginalDataRand, 4, LOOP_CNT );
}
static void TestHuffman4_mono( void )
{
    TestHuffman_template( makeOriginalDataMonoValue, 4, LOOP_CNT );
}

static void TestLZ_ex(void)
{
    TestLZ8_template( makeOriginalDataForLZHuffEx, 0, LOOP_CNT_EX );
}

static void TestLZEX_ex(void)
{
    TestLZ8_template( makeOriginalDataForLZHuffEx, 1, LOOP_CNT_EX );
}

static void TestRL_ex(void)
{
    TestRL8_template( makeOriginalDataForRLEx, LOOP_CNT_EX );
}

static void TestHuffman8_ex(void)
{
    TestHuffman_template( makeOriginalDataForLZHuffEx, 8, LOOP_CNT_EX );
}

static void TestHuffman4_ex(void)
{
    TestHuffman_template( makeOriginalDataForLZHuffEx, 4, LOOP_CNT_EX );
}

static void TestLH_normal(void)
{
    TestLH8_template( makeOriginalDataForLZHuff, 0, LOOP_CNT );
}

static void TestLH_random(void)
{
    TestLH8_template( makeOriginalDataRand, 0, LOOP_CNT );
}

static void TestLH_mono(void)
{
    TestLH8_template( makeOriginalDataMonoValue, 0, LOOP_CNT );
}

static void TestLRC_normal(void)
{
    TestLRC8_template( makeOriginalDataForLZHuff, 0, LOOP_CNT );
}

static void TestLRC_random(void)
{
    TestLRC8_template( makeOriginalDataRand, 0, LOOP_CNT );
}

static void TestLRC_mono(void)
{
    TestLRC8_template( makeOriginalDataMonoValue, 0, LOOP_CNT );
}

#endif  /* defined( _DEBUG ) */





/*---------------------------------------------------------------------------*
  Name:         ntcompress_test

  Description:  テストメイン

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ntcompress_test( void )
{
#if defined( _DEBUG )
    TestInit();
    
    TestLH_normal();
    TestLH_random();
    TestLH_mono  ();
    TestLRC_normal();
    TestLRC_random();
    TestLRC_mono  ();
    
    TestLZ8EX_normal();
    TestLZ8EX_random();
    TestLZ8EX_mono  ();
    TestLZ8_normal();
    TestLZ8_random();
    TestLZ8_mono  ();
    TestRL8_normal();
    TestRL8_random();
    TestRL8_mono  ();
    TestHuffman8_normal();
    TestHuffman8_random();
    TestHuffman8_mono  ();
    TestHuffman4_normal();
    TestHuffman4_random();
    TestHuffman4_mono  ();
    TestLZ_ex();
    TestLZEX_ex();
    TestRL_ex();
    TestHuffman8_ex();
    TestHuffman4_ex();

#endif
}



