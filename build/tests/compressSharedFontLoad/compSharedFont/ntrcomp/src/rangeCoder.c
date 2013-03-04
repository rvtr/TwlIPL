/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     rangeCoder.c

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

#include "rangeCoder.h"

typedef struct
{
    u32 freq[ 0x100 ];
    u32 low_cnt[ 0x100 ];
    u32 total;
}
RCCompressionInfo;

// レンジコーダ状態構造体
typedef struct
{
    u32     low;
    u32     range;
    u32     code;       // 展開時のみ使用
    u8      carry;      // 圧縮時のみ使用
    u32     carry_cnt;  // 圧縮時のみ使用
}
RCState;


#define MAX_RANGE  0x80000000

/*---------------------------------------------------------------------------*
  Name:         RCInitInfo_
  Description:  
  Arguments:    info    
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCInitInfo_( RCCompressionInfo* info, BOOL adaptive )
{
    u32 i;
    
    if ( adaptive )
    // 適応型レンジコーダ
    {
        for ( i = 0; i < 0x100; i++ )
        {
            info->freq[ i ]    = 1;
            info->low_cnt[ i ] = i;
        }
        info->total = 0x100;
    }
    else
    // 静的レンジコーダ
    {
        for ( i = 0; i < 0x100; i++ )
        {
            info->freq[ i ]    = 0;
            info->low_cnt[ i ] = 0;
        }
        info->total = 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         RCCount_
  Description:  
  Arguments:    info    
                srcp    
                size    
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCCount_( RCCompressionInfo* info, const u8* srcp, u32 size )
{
    u32 srcCnt = 0;
    u32 i;
    
    while ( srcCnt < size )
    {
        info->freq[ srcp[ srcCnt ] ]++;
        info->total++;
        srcCnt++;
    }
    
    // 0x10000へ正規化
    #define NORMAL_FREQ  0x10000
    {
        f32 rate = (f32)NORMAL_FREQ / info->total;
        u32 max_i    = 0;
        u32 max_freq = 0;
        
        info->total = 0;
        for ( i = 0; i < 0x100; i++ )
        {
            u32 orig = info->freq[ i ];
            info->freq[ i ] = (u32)(rate * info->freq[ i ] + 0.5f);
            if ( orig != 0 && info->freq[ i ] == 0 )
            {
                info->freq[ i ] = 1;
            }
            
            info->total += info->freq[ i ];
            if ( info->freq[ i ] >= max_freq )
            {
                max_i    = i;
                max_freq = info->freq[ i ];
            }
        }
        if ( info->total > NORMAL_FREQ )
        {
            info->freq[ max_i ] -= (info->total - NORMAL_FREQ);
        }
        else
        {
            info->freq[ max_i ] += (NORMAL_FREQ - info->total);
        }
        info->total = NORMAL_FREQ;
    }
    #undef NORMAL_FREQ
    
    info->low_cnt[ 0 ] = 0;
    
    for ( i = 1; i < 0x100; i++ )
    {
        info->low_cnt[ i ] = info->low_cnt[ i - 1 ] + info->freq[ i - 1 ];
    }
}



/*---------------------------------------------------------------------------*
  Name:         RCAAddCount_

  Description:  適応型レンジコーダの頻度テーブルを更新します。

  Arguments:    info    
                val     

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCAAddCount_( RCCompressionInfo* info, u8 val )
{
    u32 i;
    
    info->freq[ val ]++;
    info->total++;
    for ( i = val + 1; i < 0x100; i++ )
    {
        info->low_cnt[ i ]++;
    }
    
    // トータルが最大値を越えた場合には、再構成する。
    if ( info->total >= 0x00010000 )
    {
        if ( info->freq[ 0 ] > 1 )
        {
            info->freq[ 0 ] = info->freq[ 0 ] / 2;
        }
        info->low_cnt[ 0 ] = 0;
        info->total = info->freq[ 0 ];
        
        for ( i = 1; i < 0x100; i++ )
        {
            if ( info->freq[ i ] > 1 )
            {
                info->freq[ i ] >>= 1;
            }
            info->low_cnt[ i ] = info->low_cnt[ i - 1 ] + info->freq[ i - 1 ];
            info->total += info->freq[ i ];
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         RCCompWrite

  Description:  レンジコーダの圧縮(テーブル型)

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
s32
RCCompWrite_( const u8* srcp, u32 size, u8* dstp, BOOL adaptive )
{
#define MIN_RANGE 0x01000000
    static RCCompressionInfo sInfo;
    u32 srcCnt = 0;
    u32 dstCnt = 0;
    u32 low, range;
    u8  carry;
    u32 carry_cnt;
    u32 i;
    
    // ワークの初期化
    RCInitInfo_( &sInfo, adaptive );
    
    if ( ! adaptive )
    {
        // 出現頻度を計算
        RCCount_( &sInfo, srcp, size );
    }
    
    // 開始Rangeが0x80000000なので、初回いきなり桁上げが発生することはない
    low   = 0;
    range = MAX_RANGE;
    carry = 0;
    carry_cnt = 0;
    
    // ヘッダ出力
    dstp[ dstCnt++ ] = LRC_CODE_HEADER;
    if ( size > 0x1000000 )
    {
        dstp[ dstCnt++ ] = 0;
        dstp[ dstCnt++ ] = 0;
        dstp[ dstCnt++ ] = 0;
    }
    dstp[ dstCnt++ ] = (u8)( size ); 
    dstp[ dstCnt++ ] = (u8)( size >> 8  );
    dstp[ dstCnt++ ] = (u8)( size >> 16 );
    if ( size > 0x1000000 )
    {
        dstp[ dstCnt++ ] = (u8)( size >> 24 );
    }
    
    if ( ! adaptive )
    {
        // 頻度テーブルの出力(16bitリトルエンディアン)
        for ( i = 0; i < 0x100; i++ )
        {
            dstp[ dstCnt++ ] = (u8)( sInfo.freq[ i ] );
            dstp[ dstCnt++ ] = (u8)( sInfo.freq[ i ] >> 8 );
        }
    }
    
    // コードの出力
    while ( srcCnt < size )
    {
        u8  val  = srcp[ srcCnt++ ];
        u32 temp = range / sInfo.total;
        u32 prevLow = low;
        low   = low + sInfo.low_cnt[ val ] * temp;
        range = sInfo.freq[ val ] * temp;
        
        if ( adaptive )
        {
            // 出現頻度テーブルを更新
            RCAAddCount_( &sInfo, val );
        }
        
        // 桁上がりが発生する場合の処理
        if ( prevLow > low )
        {
            // キャリーを１繰上げ
            ++carry;
            // キャリーと(キャリーカウンタ - 1)分の0x00を出力します。
            if ( carry_cnt > 1 )
            {
                dstp[ dstCnt++ ] = carry;
                --carry_cnt;
                carry = 0x00;
            }
            while ( carry_cnt > 1 )
            {
                dstp[ dstCnt++ ] = 0x00;
                --carry_cnt;
            }
        }
        
        // Rangeの上位1バイトが空になったら桁上げ
        while ( range < MIN_RANGE )
        {
            u8 candidate = (u8)( low >> 24 );
            
            // 次のキャリーが0xFFの場合は更に桁上げがあり得るのでcarryを出力せずにcarry_cntだけ増やす
            if ( candidate == 0xFF )
            {
                ++carry_cnt;
            }
            else
            // 次のキャリーが0xFFではない場合はcarryを出力する
            {
                // carryと(carry_cnt - 1)分の0xFFを出力する
                if ( carry_cnt > 0 )
                {
                    dstp[ dstCnt++ ] = carry;
                    --carry_cnt;
                }
                while ( carry_cnt > 0 )
                {
                    dstp[ dstCnt++ ] = 0xFF;
                    --carry_cnt;
                }
                // 新しいcarryに置き換え
                carry = candidate;
                carry_cnt = 1;
            }
            low   <<= 8;
            range <<= 8;
        }
    }
    
    if ( carry_cnt > 0 )
    {
        dstp[ dstCnt++ ] = carry;
        --carry_cnt;
    }
    while ( carry_cnt > 0 )
    {
        dstp[ dstCnt++ ] = 0xFF;
        --carry_cnt;
    }
    dstp[ dstCnt++ ] = (u8)( (low >> 24) & 0xFF );
    dstp[ dstCnt++ ] = (u8)( (low >> 16) & 0xFF );
    dstp[ dstCnt++ ] = (u8)( (low >> 8 ) & 0xFF );
    dstp[ dstCnt++ ] = (u8)( (low >> 0 ) & 0xFF );
    
    return dstCnt;
#undef MIN_RANGE
}


/*---------------------------------------------------------------------------*
  Name:         RCACompWrite

  Description:  レンジコーダの圧縮(適応型)

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
s32
RCCompWrite( const u8* srcp, u32 size, u8* dstp )
{
    return RCCompWrite_( srcp, size, dstp, FALSE );
}

s32
RCACompWrite( const u8* srcp, u32 size, u8* dstp )
{
    return RCCompWrite_( srcp, size, dstp, TRUE );
}

static u8
SearchRC_( RCCompressionInfo* info, u32 code, u32 range, u32 low )
{
#define TABLE_SIZE  0x100
    u32 codeVal = code - low;
    u32 i;
    u32 temp = range / info->total;
    u32 tempVal = codeVal / temp;
    
#if 0
    // TODO: とりあえず線形探索、二分探索にするべき
    for ( i = 0; i < TABLE_SIZE - 1; i++ )
    {
        if ( info->low_cnt[ i + 1 ] > tempVal )
        {
            while ( info->freq[ i ] == 0 )
            {
                --i;
            }
            return (u8)i;
        }
    }
    return TABLE_SIZE - 1;
#else
    // 二分探索
    u32 left  = 0;
    u32 right = TABLE_SIZE - 1;
    
    while ( left < right )
    {
        i = (left + right) / 2;
        
        if ( info->low_cnt[ i ] > tempVal )
        {
            right = i;
        }
        else
        {
            left = i + 1;
        }
    }
    
    i = left;
    while ( info->low_cnt[ i ] > tempVal )
    {
        --i;
    }
    return (u8)i;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         RCCompRead

  Description:  レンジコーダ展開(テーブル型)

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
s32
RCCompRead_( const u8* srcp, u32 srcSize, u8* dstp, BOOL adaptive )
{
#define MIN_RANGE 0x01000000
    static RCCompressionInfo sInfo;
    
    u32 dstSize = *(u32*)srcp >> 8;
    u32 dstCnt  = 0;
    u32 srcCnt  = 0;
    u32 i;
    u32 code, range, low;
    
    if ( srcSize < 4 )
    {
        return -1;
    }
    
    srcCnt = 4;
    if ( dstSize == 0 )
    {
        dstSize = *(u32*)&srcp[ srcCnt ];
        srcCnt += 4;
        if ( srcSize < 8 )
        {
            return -1;
        }
    }
    
    if ( srcSize < sizeof(u16) * 0x100 + srcCnt )
    {
        return -1;
    }
    
    // ワークの初期化
    if ( adaptive )
    {
        RCInitInfo_( &sInfo, TRUE );
    }
    else
    {
        sInfo.total = 0;
        for ( i = 0; i < 0x100; i++ )
        {
            sInfo.freq[ i ] = *(u16*)&srcp[ srcCnt ];
            srcCnt += 2;
            sInfo.total += sInfo.freq[ i ];
        }
        sInfo.low_cnt[ 0 ] = 0;
        for ( i = 1; i < 0x100; i++ )
        {
            sInfo.low_cnt[ i ] = sInfo.low_cnt[ i - 1 ] + sInfo.freq[ i - 1 ];
        }
    }
    
    code   = (u32)((srcp[ srcCnt ] << 24) | (srcp[ srcCnt + 1 ] << 16) | 
                   (srcp[ srcCnt + 2 ] << 8) | (srcp[ srcCnt + 3 ]));
    range  = MAX_RANGE;
    low    = 0;
    srcCnt += 4;
    
    while ( dstCnt < dstSize )
    {
        u8 val = SearchRC_( &sInfo, code, range, low );
        
        dstp[ dstCnt++ ] = val;
        
        {
            u32 tmp;
            tmp   = range / sInfo.total;
            low   = low + sInfo.low_cnt[ val ] * tmp;
            range = sInfo.freq[ val ] * tmp; 
        }
        
        if ( adaptive )
        {
            // 出現頻度テーブルを更新
            RCAAddCount_( &sInfo, val );
        }
        
        while ( range < MIN_RANGE )
        {
            code  <<= 8;
            code += srcp[ srcCnt++ ];
            range <<= 8;
            low   <<= 8;
        }
    }
    
    return dstSize;
#undef MIN_RANGE
}


/*---------------------------------------------------------------------------*
  Name:         RCACompRead

  Description:  レンジコーダ展開(適応型)

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
s32
RCCompRead( const u8* srcp, u32 srcSize, u8* dstp )
{
    return RCCompRead_( srcp, srcSize, dstp, FALSE );
}

s32
RCACompRead( const u8* srcp, u32 srcSize, u8* dstp )
{
    return RCCompRead_( srcp, srcSize, dstp, TRUE );
}

