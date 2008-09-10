/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     multipleCompLib.c

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

#include "multipleCompLib.h"

#undef _DEBUG
#ifdef _DEBUG

#endif

#define LH_CODE_HEADER     (0x40)
#define LRC_CODE_HEADER     (0x50)

#define BLEND_COMP_FLAG   1

#define LH_ENC_OFFSET_WIDTH /* この設定が有効な場合にはoffset値全体ではなくoffsetのbit長を符号化する */

#if !defined(LH_ENC_OFFSET_WIDTH)
  #define LH_OFFSET_BITS         12
  #define LH_OFFSET_TABLE_BITS   LH_OFFSET_BITS
#else // if defined(LH_ENC_OFFSET_WIDTH)
  #define LH_OFFSET_BITS         15
  #define LH_OFFSET_TABLE_BITS   5
#endif

#define LENGTH_BITS      (8 + BLEND_COMP_FLAG)
#define OFFSET_SIZE_MAX (1 << 15)

#define REVERSE_SEARCH

typedef struct
{
    u16 WindowPos;                      // 現在のスライド辞書の先頭位置
    u16 WindowLen;                      // 現在のスライド辞書のサイズ
    
    s16 LZOffsetTable[ OFFSET_SIZE_MAX ];     // オフセットデータのテーブル
  #ifdef REVERSE_SEARCH
    s16 LZRevOffsetTable[ OFFSET_SIZE_MAX ];  // オフセットデータの逆順テーブル
  #endif
    s16 LZByteTable[ 256 ];               // データの先頭テーブル
    s16 LZEndTable [ 256 ];               // データの終端テーブル
    u8  OffsetBits;                       // オフセットを表現する為のビット数
} LZCompressInfo;

static LZCompressInfo  gLZWork;

INLINE u32
RoundUp( u32 value, u32 base )
{
    return (value + (base - 1)) & ~(base - 1);
}

/*---------------------------------------------------------------------------*
  Name:         LZInitTable
  Description:  
  Arguments:    work    
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
LZInitTable( LZCompressInfo* info )
{
    u16     i;
    
    for ( i = 0; i < 256; i++ )
    {
        info->LZByteTable[i] = -1;
        info->LZEndTable[i]  = -1;
    }
    info->WindowPos = 0;
    info->WindowLen = 0;
}

/*---------------------------------------------------------------------------*
  Name:         SlideByte
  Description:  辞書を1バイトスライド
  Arguments:    *srcp   
                work    
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SlideByte( LZCompressInfo* info, const u8 *srcp )
{
    s16     offset;
    u8      in_data = *srcp;
    u16     insert_offset;
    
#if defined( REVERSE_SEARCH ) // 探索順を逆にする(新しいデータを優先)
    s16    *const LZByteTable      = info->LZEndTable;
    s16    *const LZEndTable       = info->LZByteTable;
    s16    *const LZOffsetTable    = info->LZRevOffsetTable;
    s16    *const LZRevOffsetTable = info->LZOffsetTable;
#else
    s16    *const LZByteTable      = info->LZByteTable;
    s16    *const LZEndTable       = info->LZEndTable;
    s16    *const LZOffsetTable    = info->LZOffsetTable;
#endif
    const u16 windowPos = info->WindowPos;
    const u16 windowLen = info->WindowLen;
    const u32 OFFSET_SIZE = (1 << info->OffsetBits);
    
    if ( windowLen == OFFSET_SIZE )
    {
        u8 out_data = *(srcp - OFFSET_SIZE);
        if ((LZByteTable[out_data] = LZOffsetTable[LZByteTable[out_data]]) == -1)
        {
            LZEndTable[out_data] = -1;
        }
        else
        {
        #if defined( REVERSE_SEARCH )
            LZRevOffsetTable[LZByteTable[out_data]] = -1;
        #endif
        }
        insert_offset = windowPos;
    }
    else
    {
        insert_offset = windowLen;
    }
    
    offset = LZEndTable[in_data];
    if (offset == -1)
    {
        LZByteTable[in_data] = insert_offset;
    #if defined( REVERSE_SEARCH )
        LZRevOffsetTable[insert_offset] = -1;
    #endif
    }
    else
    {
    #if defined( REVERSE_SEARCH )
        LZRevOffsetTable[insert_offset] = offset;
    #endif
        LZOffsetTable[offset]   = insert_offset;
    }
    LZEndTable[in_data] = insert_offset;
    LZOffsetTable[insert_offset] = -1;
    
    if (windowLen == OFFSET_SIZE)
    {
        info->WindowPos = (u16)((windowPos + 1) % OFFSET_SIZE);
    }
    else
    {
        info->WindowLen++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         LZSlide
  Description:  
  Arguments:    *srcp   
                n       
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
LZSlide( LZCompressInfo* info, const u8 *srcp, u32 n )
{
    u32     i;

    for (i = 0; i < n; i++)
    {
        SlideByte(info, srcp++);
    }
}

//--------------------------------------------------------
// LZ77圧縮でスライド窓の中から最長一致列を検索します。
//  Arguments:    startp                 データの開始位置を示すポインタ
//                nextp                  検索を開始するデータのポインタ
//                remainSize             残りデータサイズ
//                offset                 一致したオフセットを格納する領域へのポインタ
//  Return   :    一致列が見つかった場合は   TRUE
//                見つからなかった場合は     FALSE
//--------------------------------------------------------
static u16
SearchLZ( const LZCompressInfo* info, const u8 *nextp, u32 remainSize, u16 *offset, u16 minOffset, u32 maxLength )
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     currOffset;
    u16     currLength = 2;
    u16     tmpLength;
    s32     w_offset;
    const s16 * const LZOffsetTable = info->LZOffsetTable;
    const u16 windowPos = info->WindowPos;
    const u16 windowLen = info->WindowLen;
    
    if (remainSize < 3)
    {
        return 0;
    }

    w_offset = info->LZByteTable[ *nextp ];

    while (w_offset != -1)
    {
        if (w_offset < windowPos)
        {
            searchp = nextp - windowPos + w_offset;
        }
        else
        {
            searchp = nextp - windowLen - windowPos + w_offset;
        }
        
        /* 無くても良いが、僅かに高速化する */
        if (*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset = LZOffsetTable[ w_offset ];
            continue;
        }
        
        if (nextp - searchp < minOffset)
        {
            // VRAMは2バイトアクセスなので (VRAMからデータを読み出す場合があるため)、
            // 検索対象データは2バイト前からのデータにしなければならない。
            // 
            // オフセットは12ビットで格納されるため、4096以下
        #if defined( REVERSE_SEARCH )
            w_offset = LZOffsetTable[ w_offset ];
            continue;
        #else
            break;
        #endif
        }
        tmpLength = 3;
        searchHeadp = searchp + 3;
        headp = nextp + 3;
        
        while (((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;
            
            // データ長は8ビットで格納されるため、258以下 (3の下駄をはかせる)
            if (tmpLength == maxLength)
            {
                break;
            }
        }
        if (tmpLength > currLength)
        {
            // 最大長オフセットを更新
            currLength = tmpLength;
            currOffset = (u16)((u32)nextp - (u32)searchp);
            if (currLength == maxLength)
            {
                // 一致長が最大なので、検索を終了する。
                break;
            }
        }
        w_offset = LZOffsetTable[w_offset];
    }
    
    if (currLength < 3)
    {
        return 0;
    }
    *offset = currOffset;
    return currLength;
}

/*---------------------------------------------------------------------------*
  Name:         LZCompWrite

  Description:  

  Arguments:    *srcp   
                size    
                *dstp   
                lzSearchOffset

  Returns:      
 *---------------------------------------------------------------------------*/
static u32
LZCompWrite_( const u8 *srcp, s32 size, u8 *dstp, u8 lzSearchOffset, u8 offsetBits )
{
    u32     LZDstCount = 0;            // 圧縮データのバイト数
    u8      LZCompFlags;               // 圧縮の有無を示すフラグ系列
    u8     *LZCompFlagsp;              // LZCompFlags を格納するメモリ領域をポイント
    u16     lastOffset;                // 一致データまでのオフセット (その時点での最長一致データ) 
    u16     lastLength;                // 一致データ長 (その時点での最長一致データ)
    u8      i;
    const u32 MAX_LENGTH = 0xFF + 3;

    LZInitTable( &gLZWork );
    gLZWork.OffsetBits = offsetBits;
    
    while ( size > 0 )
    {
        LZCompFlags = 0;
        LZCompFlagsp = dstp++;         // フラグ系列の格納先
        LZDstCount++;

        // フラグ系列が8ビットデータとして格納されるため、8回ループ
        for ( i = 0; i < 8; i++ )
        {
            LZCompFlags <<= 1;         // 初回 (i=0) は特に意味はない
            if (size <= 0)
            {
                // 終端に来た場合はフラグを最後までシフトさせてから終了
                continue;
            }

            if ( (lastLength = SearchLZ(&gLZWork, srcp, size, &lastOffset, lzSearchOffset, MAX_LENGTH)) != 0 )
            {
                // 圧縮可能な場合はフラグを立てる
                LZCompFlags |= 0x1;

                // オフセットは上位4ビットと下位8ビットに分けて格納
                *dstp++ = (u8)(lastLength - 3);
                *dstp++ = (u8)((lastOffset - 1) & 0xff); // リトルエンディアン
                *dstp++ = (u8)((lastOffset - 1) >> 8);
                LZDstCount += 3;
                LZSlide( &gLZWork, srcp, lastLength );
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // 圧縮なし
                LZSlide( &gLZWork, srcp, 1 );
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // 8回ループ終了
        *LZCompFlagsp = LZCompFlags;   // フラグ系列を格納
    }

    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含めない
    i = 0;
    while ((LZDstCount + i) & 0x3)
    {
        *dstp++ = 0;
        i++;
    }
    
    return LZDstCount;
}


typedef struct
{
    u16     No;                        // データNo
    s16     PaNo;                      // 親No 
    u32     Freq;                      // 出現頻度
    s16     ChNo[2];                   // 子No (0: 左側， 1: 右側)
    u16     PaDepth;                   // 親ノードの深さ
    u16     LeafDepth;                 // 葉までの深さ
    u32     HuffCode;                  // ハフマン符号
    u16     Bit;                       // ノードのビットデータ
    u16     HWord;                     // 各中間節点において、その節点をルートとする部分木を HuffTree 格納に必要なメモリ量
}
HuffData;

typedef struct
{
    u8       leftOffsetNeed;            // 左の子節点へのオフセットが必要なら1
    u8       rightOffsetNeed;           // 右の子節点へのオフセットが必要なら1
    u16      leftNodeNo;                // 左の子節点No
    u16      rightNodeNo;               // 右の子節点No
}
HuffTreeCtrlData;

// ハフマンワークバッファ構成
typedef struct
{
    HuffData*         huffTable;            //  huffTable[ 512 ];      12288B
    u16*              huffTree;             //  huffTree[ 256 * 2 ];     512B
    HuffTreeCtrlData* huffTreeCtrl;         //  huffTreeCtrl[ 256 ];    1536B
    u16               huffTreeTop;          //
    u8                bitSize;              //  
    u8                padding_[1];          //  
}
HuffInfo;                       // 計 14340B

static void HuffMakeHuffTree             ( HuffInfo* info, u16 rootNo );
static void HuffMakeSubsetHuffTree       ( HuffInfo* info, u16 huffTreeNo, BOOL rightNodeFlag );
static BOOL HuffRemainingNodeCanSetOffset( HuffInfo* info, u16 costHWord );
static void HuffSetOneNodeOffset         ( HuffInfo* info, u16 huffTreeNo, BOOL rightNodeFlag );
static u16  HuffMakeNode                 ( HuffData* table, u8 bitSize );

static void HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo );
static void HuffAddCodeToTable       ( HuffData* table, u16 nodeNo, u32 paHuffCode );
static u16  HuffAddCountHWordToTable ( HuffData *table, u16 nodeNo );


// ビットストリーム
typedef struct
{
    u8*     dstp;       // 出力先ポインタ
    u32     cnt;        // 出力サイズ
    u32     stream;     // カレントストリームデータ
    u32     stream_len; // ストリームの長さ
}
BitStream;

static void
BitStream_Init( BitStream* context, u8* dstp )
{
    context->dstp       = dstp;
    context->cnt        = 0;
    context->stream     = 0;
    context->stream_len = 0;
}

static void
BitStream_Write( BitStream* context, u32 data, u32 width )
{
    u32 i;
    u32 stream     = context->stream;
    u32 cnt        = context->cnt;
    u32 stream_len = context->stream_len;
    u32 mask       = (1 << width) - 1;
    
    if ( width == 0 )
    {
        return;
    }
    
    stream = (stream << width) | ( data & mask );
    stream_len += width;
    
    for ( i = 0; i < stream_len / 8; i++ )
    {
        context->dstp[ cnt++ ] = (u8)( stream >> ( stream_len - ( i + 1 ) * 8 ) );
    }
    stream_len %= 8;
    
    context->stream     = stream;
    context->cnt        = cnt;
    context->stream_len = stream_len;
}

static void
BitStream_Terminate( BitStream* context, u32 align )
{
    u32 stream     = context->stream;
    u32 cnt        = context->cnt;
    u32 stream_len = context->stream_len;
    
    if ( stream_len > 0 )
    {
        stream <<= 8 - stream_len;
        
        if ( context->stream_len != 0 )
        {
            context->dstp[ cnt++ ] = (u8)( stream );
        }
    }
    
    while ( cnt % align )
    {
        context->dstp[ cnt++ ] = 0;
    }
    context->cnt        = cnt;
    context->stream_len = 0;
}




/*---------------------------------------------------------------------------*
  Name:         HuffInitTable
  Description:  
  Arguments:    info    
                bitSize 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
HuffInitTable( HuffInfo* info, u8 bitSize )
{
    u32 tableSize = (1 << bitSize);
    u32 i;
    
    info->huffTable = (HuffData*)malloc( sizeof(HuffData) * tableSize * 2 );
    info->huffTree  = (u16*)malloc( sizeof(u16) * tableSize * 2 );
    info->huffTreeCtrl = (HuffTreeCtrlData*)malloc( sizeof(HuffTreeCtrlData) * tableSize );
    
    info->huffTreeTop = 1;
    info->bitSize = bitSize;
    
    // huffTableを初期化
    {
        HuffData* table = info->huffTable;
        const HuffData HUFF_TABLE_INIT_DATA = { 0, 0, 0, {-1, -1}, 0, 0, 0, 0, 0 };
        for ( i = 0; i < tableSize * 2; i++ )
        {
            table[ i ]    = HUFF_TABLE_INIT_DATA;
            table[ i ].No = (u16)i;
        }
    }
    
    // huffTree, huffTreeCtrlを初期化
    {
        const HuffTreeCtrlData HUFF_TREE_CTRL_INIT_DATA = { 1, 1, 0, 0 };
        u16*              huffTree     = info->huffTree;
        HuffTreeCtrlData* huffTreeCtrl = info->huffTreeCtrl;
        
        for ( i = 0; i < tableSize; i++ )
        {
            huffTree[ i * 2 ]     = 0;
            huffTree[ i * 2 + 1 ] = 0;
            huffTreeCtrl[ i ]     = HUFF_TREE_CTRL_INIT_DATA;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         LZCountHuffData
  Description:  
  Arguments:    srcp    
                srcSize 
                info8   
                info16  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
LZCountHuffData( const u8* srcp, u32 srcSize, HuffInfo* info8, HuffInfo* info16 )
{
    u32 srcCnt = 0;
    u32 i;
    
    while ( srcCnt < srcSize )
    {
        u8 compFlags = srcp[ srcCnt++ ]; // 圧縮の有無を示すフラグ列
        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // 圧縮されている、length:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // リトルエンディアン
                offset    |= (srcp[ srcCnt++ ] << 8);
                
            #if BLEND_COMP_FLAG
                info8->huffTable[ length | 0x100 ].Freq++;
            #else
                info8->huffTable[ length ].Freq++;
            #endif
            #if !defined(LH_ENC_OFFSET_WIDTH)
                info16->huffTable[ offset ].Freq++;
            #else
                {
                    u32 offset_bit = 0;
                    while ( offset != 0 )
                    {
                        ++offset_bit;
                        offset >>= 1;
                    }
                    info16->huffTable[ offset_bit ].Freq++;
                }
            #endif
            }
            else
            {
                u8 data = srcp[ srcCnt++ ];
                info8->huffTable[ data ].Freq++;
            }
            compFlags <<= 1;
            if ( srcCnt >= srcSize )
            {
                break;
            }
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         ConstructHuffTree
  Description:  
  Arguments:    info    
                bitSize 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
ConstructHuffTree( HuffInfo* info, u8 bitSize )
{
    HuffData* table = info->huffTable;
    u16 rootNo;
    
    // 出現頻度からノードを構築
    rootNo = HuffMakeNode( table, bitSize );
    
    // ハフマンコード生成 (table[i].HuffCode に)
    HuffAddCodeToTable( table, rootNo, 0x00 );        // PaDepthのビット数だけ、HuffCode の下位ビットをマスクしたものがハフマンコード
    
    // 各中間節点において、その節点をルートとする部分木を huffTree 格納に必要なメモリ量の計算
    HuffAddCountHWordToTable( table, rootNo );
    
    HuffMakeHuffTree( info, rootNo );
	info->huffTreeTop--;
}

//-----------------------------------------------------------------------
// ハフマンコード表作成
//-----------------------------------------------------------------------
static void
HuffMakeHuffTree( HuffInfo* info, u16 rootNo )
{
    s16     i;
    s16     costHWord, tmpCostHWord;            // 部分木のコード表を作成しなかった時のコスト 最大値の節点の部分木コード表を作る
    s16     costOffsetNeed, tmpCostOffsetNeed;
    s16     costMaxKey;                         // コスト最小の節点を huffTreeBuf.huffTree から特定するための情報
    BOOL    costMaxRightFlag;
    u16     offsetNeedNum;
    BOOL    tmpRightFlag;
    const u32 MAX_COST = 1 << (info->bitSize - 2);
    
    info->huffTreeTop = 1;
    costOffsetNeed    = 0;
    
    info->huffTreeCtrl[0].leftOffsetNeed = 0; // 使用しない (テーブルサイズとして使用)
    info->huffTreeCtrl[0].rightNodeNo    = rootNo;
    
    while ( 1 )                          // return するまで 
    {
        // オフセットを設定する必要のあるノード数の計算
        offsetNeedNum = 0;
        for ( i = 0; i < info->huffTreeTop; i++ )
        {
            if ( info->huffTreeCtrl[ i ].leftOffsetNeed )
            {
                offsetNeedNum++;
            }
            if ( info->huffTreeCtrl[ i ].rightOffsetNeed )
            {
                offsetNeedNum++;
            }
        }
        
        // 最大コストの節点を検索
        costHWord    = -1;
        costMaxKey   = -1;
        tmpRightFlag =  0;
        
        for ( i = 0; i < info->huffTreeTop; i++ )
        {
            tmpCostOffsetNeed = (u16)( info->huffTreeTop - i );
            
            // 左の子節点のコスト評価
            if ( info->huffTreeCtrl[i].leftOffsetNeed )
            {
                tmpCostHWord = (s16)info->huffTable[ info->huffTreeCtrl[i].leftNodeNo ].HWord;
                
                if ( (u32)(tmpCostHWord + offsetNeedNum) > MAX_COST )
                {
                    goto leftCostEvaluationEnd;
                }
                if ( ! HuffRemainingNodeCanSetOffset( info, (u16)tmpCostHWord ) )
                {
                    goto leftCostEvaluationEnd;
                }
                if ( tmpCostHWord > costHWord )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 0;
                }
                else if ( (tmpCostHWord == costHWord) && (tmpCostOffsetNeed > costOffsetNeed) )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 0;
                }
            }
leftCostEvaluationEnd:{}
            
            if ( info->huffTreeCtrl[i].rightOffsetNeed )
            {
                tmpCostHWord = (s16)info->huffTable[ info->huffTreeCtrl[i].rightNodeNo ].HWord;
                
                if ( (u32)(tmpCostHWord + offsetNeedNum) > MAX_COST )
                {
                    goto rightCostEvaluationEnd;
                }
                if ( ! HuffRemainingNodeCanSetOffset( info, (u16)tmpCostHWord ) )
                {
                    goto rightCostEvaluationEnd;
                }
                if ( tmpCostHWord > costHWord )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 1;
                }
                else if ( (tmpCostHWord == costHWord) && (tmpCostOffsetNeed > costOffsetNeed) )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 1;
                }
            }
rightCostEvaluationEnd:{}
        }
        
        // 部分木をまるまる huffTree に格納
        if ( costMaxKey >= 0 )
        {
            HuffMakeSubsetHuffTree( info, (u16)costMaxKey, costMaxRightFlag);
            goto nextTreeMaking;
        }
        else
        {
            // 必要オフセット最大のノードを検索
            for ( i = 0; i < info->huffTreeTop; i++ )
            {
                u16 tmp = 0;
                tmpRightFlag = 0;
                if ( info->huffTreeCtrl[i].leftOffsetNeed )
                {
                    tmp = info->huffTable[ info->huffTreeCtrl[i].leftNodeNo ].HWord;
                }
                if ( info->huffTreeCtrl[i].rightOffsetNeed )
                {
                    if ( info->huffTable[ info->huffTreeCtrl[i].rightNodeNo ].HWord > tmp )
                    {
                        tmpRightFlag = 1;
                    }
                }
                if ( (tmp != 0) || (tmpRightFlag) )
                {
                    HuffSetOneNodeOffset( info, (u16)i, tmpRightFlag );
                    goto nextTreeMaking;
                }
            }
        }
        return;
nextTreeMaking:{}
    }
}

//-----------------------------------------------------------------------
// 部分木をまるまる huffTree に格納
//-----------------------------------------------------------------------
static void 
HuffMakeSubsetHuffTree( HuffInfo* info, u16 huffTreeNo, BOOL rightNodeFlag )
{
    u16  i;
    
    i = info->huffTreeTop;
    HuffSetOneNodeOffset( info, huffTreeNo, rightNodeFlag );
    
    if ( rightNodeFlag )
    {
        info->huffTreeCtrl[ huffTreeNo ].rightOffsetNeed = 0;
    }
    else
    {
        info->huffTreeCtrl[ huffTreeNo ].leftOffsetNeed = 0;
    }
    
    while ( i < info->huffTreeTop )
    {
        if ( info->huffTreeCtrl[ i ].leftOffsetNeed )
        {
            HuffSetOneNodeOffset( info, i, 0 );
            info->huffTreeCtrl[ i ].leftOffsetNeed = 0;
        }
        if ( info->huffTreeCtrl[ i ].rightOffsetNeed )
        {
            HuffSetOneNodeOffset( info, i, 1 );
            info->huffTreeCtrl[ i ].rightOffsetNeed = 0;
        }
        i++;
    }
}

//-----------------------------------------------------------------------
// 与えられたデータ量の部分木を展開しても huffTree 構築に支障がないか調べる
//-----------------------------------------------------------------------
static BOOL 
HuffRemainingNodeCanSetOffset( HuffInfo* info, u16 costHWord )
{
    u16 i;
    s16 capacity;
    const u32 MAX_COST = 1 << (info->bitSize - 2);
    
    capacity = (s16)( MAX_COST - costHWord );
    
    // オフセット数は i が小さいほど大きいので、ソートせず、i = 0 -> huffTreeTop で計算すればよい
    for ( i = 0; i < info->huffTreeTop; i++ )
    {
        if ( info->huffTreeCtrl[i].leftOffsetNeed )
        {
            if ( (info->huffTreeTop - i) <= capacity )
            {
                capacity--;
            }
            else
            {
                return 0;
            }
        }
        if ( info->huffTreeCtrl[i].rightOffsetNeed )
        {
            if ( (info->huffTreeTop - i) <= capacity )
            {
                capacity--;
            }
            else
            {
                return 0;
            }
        }
    }
    
    return 1;
}


/*---------------------------------------------------------------------------*
  Name:         HuffSetOneNodeOffset
  Description:  1節点分、ハフマンコード表を作成
  Arguments:    *table          ハフマンテーブル
                huffTreeNo      
                rightNodeFlag   右側のノードであるかどうかのフラグ
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
HuffSetOneNodeOffset( HuffInfo* info, u16 huffTreeNo, BOOL rightNodeFlag)
{
    u16 nodeNo;
    u16 offsetData = 0;
    
    HuffData*         huffTable    = info->huffTable;
    u16*              huffTree     = info->huffTree;
    HuffTreeCtrlData* huffTreeCtrl = info->huffTreeCtrl;
    u16               huffTreeTop  = info->huffTreeTop;
    
    if (rightNodeFlag)
    {
        nodeNo = huffTreeCtrl[ huffTreeNo ].rightNodeNo;
        huffTreeCtrl[ huffTreeNo ].rightOffsetNeed = 0;
    }
    else
    {
        nodeNo = huffTreeCtrl[ huffTreeNo ].leftNodeNo;
        huffTreeCtrl [huffTreeNo ].leftOffsetNeed = 0;
    }
    
    // 左の子節点
    if ( huffTable[ huffTable[nodeNo].ChNo[0] ].LeafDepth == 0)
    {
        offsetData |= 0x8000;
        huffTree[ huffTreeTop * 2 + 0 ] = (u16)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftOffsetNeed = 0;   // オフセットは必要なくなる
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];  // オフセットは必要
    }
    
    // 右の子節点
    if ( huffTable[ huffTable[ nodeNo ].ChNo[1] ].LeafDepth == 0 )
    {
        offsetData |= 0x4000;
        huffTree[ huffTreeTop * 2 + 1 ] = (u16)huffTable[nodeNo].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightOffsetNeed = 0;  // オフセットは必要なくなる
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1]; // オフセットは必要
    }
    
    offsetData |= (u16)( huffTreeTop - huffTreeNo - 1 );
    huffTree[ huffTreeNo * 2 + (rightNodeFlag? 1 : 0) ] = offsetData;
    
    info->huffTreeTop++;
}

/*---------------------------------------------------------------------------*
  Name:         HuffMakeNode
  Description:  出現頻度からノードデータを構築
  Arguments:    table   
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u16
HuffMakeNode( HuffData* table, u8 bitSize )
{
    u16       dataNum  = ( 1 << bitSize );
    u16       tableTop = (u16)dataNum; // テーブル作成時の、テーブルトップNo
    
    u32       i;
    s32       leftNo, rightNo;         // 2分木作成時のノードNo
    u16       rootNo;                  // 二分木のルートNo
    
    leftNo  = -1;
    rightNo = -1;
    while ( 1 )
    {
        // Freqの小さい部分木頂点を2つ探す  1つは必ず見つかるはず
        // 子頂点(左)の探索
        for ( i = 0; i < tableTop; i++ )
        {
            if ( ( table[i].Freq == 0 ) ||
                 ( table[i].PaNo != 0 ) )
            {
                continue;
            }
            
            if ( leftNo < 0 )
            {
                leftNo = i;
            }
            else if ( table[i].Freq < table[ leftNo ].Freq )
            {
                leftNo = i;
            }
        }
        
        // 子頂点(右)の探索
        for ( i = 0; i < tableTop; i++ )
        {
            if ( ( table[i].Freq == 0 ) || 
                 ( table[i].PaNo != 0 ) || 
                 ( i == leftNo ) )
            {
                continue;
            }
            
            if ( rightNo < 0 )
            {
                rightNo = i;
            }
            else if ( table[i].Freq < table[ rightNo ].Freq )
            {
                rightNo = i;
            }
        }
        
        // 1つしかなかったら、テーブル作成終了
        if ( rightNo < 0 )
        {
            // 値が一種類しかない存在しない場合には01どちらも同じ値となるノードを１つ作成する
            if ( tableTop == dataNum )
            {
                table[ tableTop ].Freq      = table[ leftNo ].Freq;
                table[ tableTop ].ChNo[0]   = (s16)leftNo;
                table[ tableTop ].ChNo[1]   = (s16)leftNo;
                table[ tableTop ].LeafDepth = 1;
                table[ leftNo   ].PaNo      = (s16)tableTop;
                table[ leftNo   ].Bit       = 0;
                table[ leftNo   ].PaDepth   = 1;
            }
            else
            {
                tableTop--;
            }
            rootNo  = tableTop;
            return rootNo;
        }
        
        // 左部分木と右部分木を統合する頂点作成
        table[ tableTop ].Freq = table[ leftNo ].Freq + table[ rightNo ].Freq;
        table[ tableTop ].ChNo[0] = (s16)leftNo;
        table[ tableTop ].ChNo[1] = (s16)rightNo;
        if ( table[ leftNo ].LeafDepth > table[ rightNo ].LeafDepth )
        {
            table[ tableTop ].LeafDepth = (u16)( table[ leftNo ].LeafDepth + 1 );
        }
        else
        {
            table[ tableTop ].LeafDepth = (u16)( table[ rightNo ].LeafDepth + 1 );
        }
        
        table[ leftNo  ].PaNo = table[ rightNo ].PaNo = (s16)( tableTop );
        table[ leftNo  ].Bit  = 0;
        table[ rightNo ].Bit  = 1;
        
        HuffAddParentDepthToTable( table, (u16)leftNo, (u16)rightNo );
        
        tableTop++;
        leftNo = rightNo = -1;
    }
}


//-----------------------------------------------------------------------
// 2文木作成時に、部分木を統合したときに、部分木の各構成ノードの深さを＋1する
//-----------------------------------------------------------------------
static void
HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo )
{
    table[ leftNo  ].PaDepth++;
    table[ rightNo ].PaDepth++;
    
    if ( table[ leftNo ].LeafDepth != 0 )
    {
        HuffAddParentDepthToTable( table, (u16)table[ leftNo  ].ChNo[0], (u16)table[ leftNo  ].ChNo[1] );
    }
    if ( table[ rightNo ].LeafDepth != 0 )
    {
        HuffAddParentDepthToTable( table, (u16)table[ rightNo ].ChNo[0], (u16)table[ rightNo ].ChNo[1] );
    }
}

//-----------------------------------------------------------------------
// ハフマンコード生成
//-----------------------------------------------------------------------
static void
HuffAddCodeToTable( HuffData* table, u16 nodeNo, u32 paHuffCode )
{
    table[ nodeNo ].HuffCode = (paHuffCode << 1) | table[ nodeNo ].Bit;
    
    if ( table[ nodeNo ].LeafDepth != 0 )
    {
        HuffAddCodeToTable( table, (u16)table[ nodeNo ].ChNo[0], table[ nodeNo ].HuffCode );
        HuffAddCodeToTable( table, (u16)table[ nodeNo ].ChNo[1], table[ nodeNo ].HuffCode );
    }
}


//-----------------------------------------------------------------------
// 中間ノードが huffTree 作成に必要とするデータ量
//-----------------------------------------------------------------------
static u16
HuffAddCountHWordToTable( HuffData *table, u16 nodeNo)
{
    u16      leftHWord, rightHWord;
    
    switch ( table[ nodeNo ].LeafDepth )
    {
    case 0:
        return 0;
    case 1:
        leftHWord = rightHWord = 0;
        break;
    default:
        leftHWord  = HuffAddCountHWordToTable( table, (u16)table[nodeNo].ChNo[0] );
        rightHWord = HuffAddCountHWordToTable( table, (u16)table[nodeNo].ChNo[1] );
        break;
    }
    
    table[ nodeNo ].HWord = (u16)( leftHWord + rightHWord + 1 );
    return (u16)( leftHWord + rightHWord + 1 );
}



/*---------------------------------------------------------------------------*
  Name:         LZMakeHuffTree
  Description:  
  Arguments:    srcp    
                tree8   
                tree16  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
LZMakeHuffTree( const u8* srcp, u32 srcSize, HuffInfo* info8, HuffInfo* info16 )
{
    HuffInitTable( info8,  LENGTH_BITS );
    HuffInitTable( info16, LH_OFFSET_TABLE_BITS );
    
    LZCountHuffData( srcp, srcSize, info8, info16 );
    
    ConstructHuffTree( info8,  LENGTH_BITS );
    ConstructHuffTree( info16, LH_OFFSET_TABLE_BITS );
}


/*---------------------------------------------------------------------------*
  Name:         ExportHuffTree
  Description:  
  Arguments:    dstp    
                info    
                bitSize 
  Returns:      
 *---------------------------------------------------------------------------*/
static u32
ExportHuffTree( u8* dstp, HuffInfo* info, u8 bitSize )
{
    BitStream stream;
    u32 i;
    u8* pSize;
    u32 tblSize;
    
    BitStream_Init( &stream, dstp );
    
    pSize = dstp;
    BitStream_Write( &stream, 0, RoundUp( bitSize, 8 ) );
    
    for ( i = 1; i < (u16)( (info->huffTreeTop + 1) * 2); i++ )
    {
        u16 flags = (u16)( info->huffTree[ i ] & 0xC000 );
        u32 data  = info->huffTree[ i ] | (flags >> (16 - bitSize));
        BitStream_Write( &stream, data, bitSize );
    }
    BitStream_Terminate( &stream, 4 );
    
    // テーブルサイズの1/4をサイズ領域へ保存
    tblSize = (stream.cnt / 4) - 1;
    if ( RoundUp( bitSize, 8 ) == 8 )
    {
        if ( tblSize >= 0x100 )
        {
            fprintf(stderr, "table size is over!\n");
        }
        *pSize = (u8)( tblSize );
    }
    else // RoundUp( bitSize, 8 ) == 16 )
    {
        if ( tblSize >= 0x10000 )
        {
            fprintf(stderr, "table size is over!\n");
        }
        *(u16*)pSize = (u16)( tblSize );
    }
    return stream.cnt;
}


/*---------------------------------------------------------------------------*
  Name:         ConvertHuff
  Description:  
  Arguments:    info    
                data    
                stream  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
ConvertHuff( HuffInfo* info, u16 data, BitStream* stream )
{
    u16 width = info->huffTable[ data ].PaDepth;
    u32 code  = info->huffTable[ data ].HuffCode;
    
    BitStream_Write( stream, code, width );
}


/*---------------------------------------------------------------------------*
  Name:         LZConvertHuffData
  Description:  
  Arguments:    srcp    
                tmpSize 
                dstp    
                info8   
                info16  
  Returns:      
 *---------------------------------------------------------------------------*/
static u32
LZConvertHuffData( const u8* srcp, u32 srcSize, u8* dstp, HuffInfo* info8, HuffInfo* info16 )
{
    u32 srcCnt = 0;
    u32 dstCnt = 0;
    
    BitStream stream;
    
    BitStream_Init( &stream, dstp );
    
    while ( srcCnt < srcSize )
    {
        u32 i;
        u8 compFlags = srcp[ srcCnt++ ]; // 圧縮の有無を示すフラグ列
    #if BLEND_COMP_FLAG
    #else
        BitStream_Write( &stream, compFlags, 8 );
    #endif

        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // 圧縮されている、length:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // リトルエンディアン
                offset |= srcp[ srcCnt++ ] << 8;
                
            #if BLEND_COMP_FLAG
                ConvertHuff( info8,  length | 0x100, &stream );
            #else
                ConvertHuff( info8,  length, &stream );
            #endif
            #if ! defined(LH_ENC_OFFSET_WIDTH)
                ConvertHuff( info16, offset, &stream );
            #else
                {
                    u16 offset_bit = 0;
                    u16 offset_tmp = offset;
                    while ( offset_tmp > 0 )
                    {
                        offset_tmp >>= 1;
                        ++offset_bit;
                    }
                    ConvertHuff( info16, offset_bit, &stream );
                    // offsetが0であることはないので、最上位のビットは省略する
                    BitStream_Write( &stream, offset & ~(1 << (offset_bit - 1)), offset_bit - 1 );
                }
            #endif
            }
            else
            {
                u8 data = srcp[ srcCnt++ ];
                
                ConvertHuff( info8, data, &stream );
            }
            compFlags <<= 1;
            if ( srcCnt >= srcSize )
            {
                break;
            }
        }
    }
    
    BitStream_Terminate( &stream, 4 );
    return stream.cnt;
}

/*---------------------------------------------------------------------------*
  Name:         LHCompWrite
  Description:  
  Arguments:    *srcp   
                size    
                *dstp   
  Returns:      
 *---------------------------------------------------------------------------*/
u32
LHCompWrite( const u8 *srcp, s32 srcSize, u8 *dstp )
{
    static HuffInfo sTree8;
    static HuffInfo sTree16;
    
    u32 tmpSize;
    u32 dstSize;
    u8* tmpBuf = (u8*)malloc( srcSize * 3 );
    // まずはsrcpを普通にLZ圧縮
    tmpSize = LZCompWrite_( srcp, srcSize, tmpBuf, 2, LH_OFFSET_BITS );
    
    // offsetとlengthの集計
    LZMakeHuffTree( tmpBuf, tmpSize, &sTree8, &sTree16 );
    
    dstSize = 0;
    
    // ヘッダの書き込み
    if ( srcSize < 0x1000000 && srcSize > 0 )
    {
        *(u32*)dstp = LH_CODE_HEADER | ( srcSize << 8 );
        dstSize = 4;
    }
    else
    {
        *(u32*)dstp     = LH_CODE_HEADER;
        *(u32*)&dstp[4] = srcSize;
        dstSize = 8;
    }
    // ハフマンテーブルを出力
    dstSize += ExportHuffTree( &dstp[ dstSize ], &sTree8,  LENGTH_BITS );
    dstSize += ExportHuffTree( &dstp[ dstSize ], &sTree16, LH_OFFSET_TABLE_BITS );
    
    // 圧縮結果をハフマン符号化しながら出力
    dstSize += LZConvertHuffData( tmpBuf, tmpSize, &dstp[ dstSize ], &sTree8, &sTree16 );
    
    return dstSize;
}



typedef struct
{
    u16 huffTable9 [ (1 << LENGTH_BITS) * 2 ];
    u16 huffTable12[ OFFSET_SIZE_MAX * 2 ];
}
LHContext;


/*---------------------------------------------------------------------------*
  Name:         HuffImportTree
  Description:  
  Arguments:    pTable  
                srcp    
                bitSize 
                srcRemainSize
  Returns:      
 *---------------------------------------------------------------------------*/
static u32
HuffImportTree( u16* pTable, const u8* srcp, u8 bitSize, u32 srcRemainSize )
{
    u32 tableSize;
    u32 idx = 1;
    u32 data = 0;
    u32 bitNum = 0;
    u32 bitMask = (1 << bitSize) - 1;
    u32 srcCnt = 0;
    
    if ( bitSize > 8 )
    {
        tableSize = *(u16*)srcp;
        srcp   += 2;
        srcCnt += 2;
    }
    else
    {
        tableSize = *srcp;
        srcp   += 1;
        srcCnt += 1;
    }
    tableSize = (tableSize + 1) * 4;
    if ( srcRemainSize < tableSize )
    {
        return tableSize;
    }
    
    while ( srcCnt < tableSize )
    {
        while ( bitNum < bitSize )
        {
            data <<= 8;
            data |= *srcp++;
            ++srcCnt;
            bitNum += 8;
        }
		if ( idx < (u32)((1 << bitSize) * 2) )
		{
			pTable[ idx++ ] = (u16)( ( data >> (bitNum - bitSize) ) & bitMask );
		}
        bitNum -= bitSize;
    }

    pTable[ 0 ] = (u16)idx;
    return tableSize;
}


/*---------------------------------------------------------------------------*
  Name:         HuffVerifyTable

  Description:  ハフマンテーブルの整合性をチェック

  Arguments:    pTable  ハフマンテーブルへのポインタ
                bit     ハフマン符号のビット数

  Returns:      正常なテーブルの場合には TRUE
                不正なテーブルの場合には FALSE
 *---------------------------------------------------------------------------*/
static BOOL
HuffVerifyTable( const void* pTable, u8 bit )
{
#if !defined(LH_ENC_OFFSET_WIDTH)
    enum { FLAGS_ARRAY_NUM = 8192 / 8 }; /* 1024Byte */
    static u8  end_flags[ FLAGS_ARRAY_NUM ];
#else
    enum { FLAGS_ARRAY_NUM = 1024 / 8 };  /* 128Byte */
    u8  end_flags[ FLAGS_ARRAY_NUM ];
#endif
    u16*  treep = (u16*)pTable;
    u16*  treeStartp = treep + 1;
    u32   treeSize   = *treep;
    u16*  treeEndp   = (u16*)pTable + treeSize;
    u32 i;
    u32 idx;
    const u16 ofs_mask = (u16)( (1 << (bit - 2)) - 1 );
    const u16 l_mask   = (u16)( 1 << (bit - 1) );
    const u16 r_mask   = (u16)( 1 << (bit - 2) );
    
    for ( i = 0; i < FLAGS_ARRAY_NUM; i++ )
    {
        end_flags[ i ] = 0;
    }
    
    if ( treeSize > (1U << (bit + 1)) )
    {
        return FALSE;
    }
    
    idx = 1;
    treep = treeStartp;
    while ( treep < treeEndp )
    {
        if ( (end_flags[ idx / 8 ] & (1 << (idx % 8) )) == 0 )
        {
            u32  offset = (u32)( ( (*treep & ofs_mask) + 1 ) << 1 );
            u16* nodep  = (u16*)((u32)treep & ~0x3) + offset;
            
            // 終端のアライメント用データは読み飛ばす
            if ( *treep == 0 && idx >= treeSize - 4 )
            {
                goto next;
            }
            if ( nodep >= treeEndp )
            {
                return FALSE;
            }
            if ( *treep & l_mask )
            {
                u32 left = (idx & ~0x1) + offset;
                end_flags[ left / 8 ] |= (u8)( 1 << (left % 8) );
            }
            if ( *treep & r_mask )
            {
                u32 right = (idx & ~0x1) + offset + 1;
                end_flags[ right / 8 ] |= (u8)( 1 << (right % 8) );
            }
        }
    next:
        ++idx;
        ++treep;
    }
    return TRUE;
}


typedef struct
{
   const u8* srcp;
   u32       cnt;
   u32       srcSize;
   u32       stream;
   u32       stream_len;
}
BitReader;

static INLINE void
BitReader_Init( BitReader* context, const u8* srcp, u32 srcSize )
{
    context->srcp       = srcp;
    context->cnt        = 0; 
    context->stream     = 0;
    context->stream_len = 0;
    context->srcSize    = srcSize;
}

static INLINE s8
BitReader_Read( BitReader* context )
{
    s8 bit;
    if ( context->stream_len == 0 )
    {
        if ( context->cnt > context->srcSize )
        {
            return -1;
        }
        context->stream     = context->srcp[context->cnt++];
        context->stream_len = 8;
    }
    bit = (s8)( (context->stream >> (context->stream_len - 1)) & 0x1 );
    context->stream_len--;
    return bit;
}

static s32
BitReader_ReadEx( BitReader* context, u8 width )
{
    s32 data;
    
    ASSERT( width <= 24 );
    
    if ( width == 0 )
    {
        return 0;
    }
    
    while ( context->stream_len < width )
    {
        if ( context->cnt > context->srcSize )
        {
            return -1;
        }
        context->stream <<= 8;
        context->stream |= context->srcp[context->cnt++];
        context->stream_len += 8;
    }
    data = (s32)( (context->stream >> (context->stream_len - width)) & ((1 << width) - 1) );
    context->stream_len -= width;
    return data;
}


/*---------------------------------------------------------------------------*
  Name:         LHCompRead
  Description:  
  Arguments:    srcp    
                size    
                *dstp   
  Returns:      
 *---------------------------------------------------------------------------*/
s32
LHCompRead( const u8* srcp, u32 srcSize, u8* dstp )
{
    static LHContext sContext;
    u32 dstSize;
    u32 srcCnt = 0;
    u32 dstCnt = 0;
    BitReader stream;
    
    if ( srcSize < 4 )
    {
        return -1;
    }
    
    // ヘッダの読み込み
    dstSize = *(u32*)srcp >> 8;
    srcCnt = 4;
    if ( dstSize == 0 )
    {
        if ( srcSize < 8 )
        {
            return -1;
        }
        dstSize = *(u32*)(srcp + 4);
        srcCnt += 4;
    }
    
    // ハフマンテーブルを読む
    srcCnt += HuffImportTree( sContext.huffTable9,  &srcp[srcCnt], LENGTH_BITS, srcSize - srcCnt );
    if ( srcCnt >= srcSize || (!HuffVerifyTable( sContext.huffTable9, LENGTH_BITS )) )
    {
        return -1;
    }
    
    srcCnt += HuffImportTree( sContext.huffTable12, &srcp[srcCnt], LH_OFFSET_TABLE_BITS, srcSize - srcCnt );
    
    if ( srcCnt >= srcSize || (!HuffVerifyTable( sContext.huffTable12, LH_OFFSET_TABLE_BITS )) )
    {
        return -1;
    }
    
    BitReader_Init( &stream, &srcp[srcCnt], srcSize - srcCnt );
    
    while ( dstCnt < dstSize )
    {
        u16* nodep = sContext.huffTable9 + 1;
        u16  val;
        do
        {
            s8  bit    = BitReader_Read( &stream );
            u32 offset = (((*nodep & 0x7F) + 1) << 1) + bit;
            
            if ( bit < 0 )
            {
                return -1;
            }
            
            if ( *nodep & (0x100 >> bit) )
            {
                nodep = (u16*)((u32)nodep & ~0x3);
                val  = *(nodep + offset);
                break;
            }
            else
            {
                nodep = (u16*)((u32)nodep & ~0x3);
                nodep += offset;
            }
        } while ( 1 );
        
        if ( val < 0x100 )
        // 非圧縮データ
        {
            dstp[dstCnt++] = (u8)val;
        }
        else
        // 圧縮データ
        {
        #if !defined(LH_ENC_OFFSET_WIDTH)
          #define OFFSET_MASK    0x3FF
          #define LEAF_FLAG      0x800
        #else
          #define OFFSET_MASK    0x07
          #define LEAF_FLAG      0x10
            u16 offset_bit;
        #endif
            u16 length = (val & 0xFF) + 3;
            u16* nodep = sContext.huffTable12 + 1;
            do
            {
                s8  bit    = BitReader_Read( &stream );
                u32 offset = (((*nodep & OFFSET_MASK) + 1) << 1) + bit;
                
                if ( bit < 0 )
                {
                    return -1;
                }
                
                if ( *nodep & (LEAF_FLAG >> bit) )
                {
                    nodep = (u16*)((u32)nodep & ~0x3);
                    val  = *(nodep + offset);
                    break;
                }
                else
                {
                    nodep = (u16*)((u32)nodep & ~0x3);
                    nodep += offset;
                }
            } while ( 1 );
            
        #if defined(LH_ENC_OFFSET_WIDTH)
            offset_bit = val;
            val = 0;
            if ( offset_bit > 0 )
            {
                val = 1;
                while ( --offset_bit > 0 )
                {
                    val <<= 1;
                    val |= BitReader_Read( &stream );
                }
            }
        #endif
            val += 1;
            
            // バッファオーバーランをチェック
            if ( dstCnt + length > dstSize )
            {
                return -1;
            }
            if ( dstCnt < val )
            {
                return -1;
            }
            if ( srcCnt + stream.cnt > srcSize )
            {
                return -1;
            }
            
            while ( length-- > 0 )
            {
                dstp[dstCnt] = dstp[dstCnt - val];
                ++dstCnt;
            }
        #undef OFFSET_MASK
        #undef LEAF_FLAG
        }
    }
    return dstCnt;


}


//==============================================================================
// 
// LRC圧縮/展開
// 
//==============================================================================

#define LRC_ADAPTIVE            // 適応型レンジコーダを使用するかどうか
#define RC_MAX_RANGE    0x80000000
#define RC_UNIT_BITS    8       // 1バイト単位で出力
// #define LRC_ENC_OFFSET_WIDTH // NOTE: このオプションでは正常に動作できない。
                                //   正確には、展開時に生のbitデータがレンジコードとして先に
                                //   読まれてしまうので、復元後には次のデータが取り出せない。

#if defined( LRC_ADAPTIVE )
  #define TABLE8_ADAPTIVE       TRUE
#else
  #define TABLE8_ADAPTIVE       FALSE
#endif

#if defined( LRC_ENC_OFFSET_WIDTH )
  #define TABLE16_ADAPTIVE        FALSE // bitLenを使用する場合には、境界テーブルが小さいのでofsは静的RCでOK
  #define LRC_OFFSET_BITS         15
  #define LRC_OFFSET_TABLE_BITS   5
#else
  #define TABLE16_ADAPTIVE        TRUE  // bitLenを使用しない場合には、境界テーブルが大きくなるのでofsは動的RCを使用
  #define LRC_OFFSET_BITS         12
  #define LRC_OFFSET_TABLE_BITS   LRC_OFFSET_BITS
#endif

// レンジコーダ用構造体
typedef struct
{
    u32 *freq;          // 出現頻度テーブル  (1 << bitSize) * sizeof(u32) Byte
    u32 *low_cnt;       // LOW境界値テーブル (1 << bitSize) * sizeof(u32) Byte
    u32 total;          // トータル          4 Byte
    u8  bitSize;        // ビットサイズ      1 Byte
    u8  padding_[1];    //
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

/*---------------------------------------------------------------------------*
  Name:         RCInitState_

  Description:  RC状態の初期化をおこないます。

  Arguments:    state   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCInitState_( RCState* state )
{
    // 開始Rangeが0x80000000なので、初回いきなり桁上げが発生することはない
    state->low   = 0;
    state->range = RC_MAX_RANGE;
    state->code  = 0;
    state->carry = 0;
    state->carry_cnt = 0;
}


/*---------------------------------------------------------------------------*
  Name:         RCInitInfo_

  Description:  静的レンジコーダのテーブル初期化
                すべての出現頻度を0で初期化します。

  Arguments:    info    

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCInitInfo_( RCCompressionInfo* info, u8 bitSize )
{
    u32 tableSize = (1 << bitSize);
    u32 i;
    
    info->bitSize = bitSize;
    info->freq    = (u32*)malloc( sizeof(u32) * tableSize );
    info->low_cnt = (u32*)malloc( sizeof(u32) * tableSize );
    
    for ( i = 0; i < tableSize; i++ )
    {
        info->freq[ i ]    = 0;
        info->low_cnt[ i ] = 0;
    }
    info->total = 0;
}

/*---------------------------------------------------------------------------*
  Name:         RCExportTable_

  Description:  静的RCテーブルを出力します。

  Arguments:    dstp
                info

  Returns:      出力データサイズ
 *---------------------------------------------------------------------------*/
static u32
RCExportTable_( u8* dstp, RCCompressionInfo* info )
{
    u32 tableSize = (1 << info->bitSize);
    u32 cnt = 0;
    u32 i;
    
    // 頻度テーブルの出力(16bitリトルエンディアン)
    for ( i = 0; i < tableSize; i++ )
    {
        dstp[ cnt++ ] = (u8)( info->freq[ i ] );
        dstp[ cnt++ ] = (u8)( info->freq[ i ] >> 8 );
    }
    return cnt;
}

/*---------------------------------------------------------------------------*
  Name:         RCImportTable_

  Description:  

  Arguments:    info    
                srcp    
                srcRemainSize

  Returns:      
 *---------------------------------------------------------------------------*/
static u32
RCImportTable_( RCCompressionInfo* info, const u8* srcp, u32 srcRemainSize )
{
    u32 tableSize = (1 << info->bitSize);
    u32 cnt = 0;
    u32 i;
    
    if ( srcRemainSize < tableSize * sizeof(u16) )
    {
        return srcRemainSize;
    }
    
    // 頻度テーブルのインポート(16bitリトルエンディアン)
    for ( i = 0; i < tableSize; i++ )
    {
        info->freq[ i ] = srcp[ cnt ] | ( srcp[ cnt + 1 ] << 8 );
        cnt += 2;
    }
    info->low_cnt[ 0 ] = 0;
    info->total = info->freq[ 0 ];
    for ( i = 1; i < tableSize; i++ )
    {
        info->low_cnt[ i ] = info->low_cnt[ i - 1 ] + info->freq[ i - 1 ];
        info->total += info->freq[ i ];
    }
    return cnt;
}

/*---------------------------------------------------------------------------*
  Name:         RCNormalizeTable_

  Description:  RCテーブルの正規化

  Arguments:    info    

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCNormalizeTable_( RCCompressionInfo* info )
{
    u32 tableSize = (1 << info->bitSize);
    u32 i;
    
    // トータルのカウント
    info->total = 0;
    for ( i = 0; i < tableSize; i++ )
    {
        info->total += info->freq[ i ];
    }
    
    // 正規化
    // 0x10000へ正規化
    #define NORMAL_FREQ  0x10000
    {
        f32 rate = (f32)NORMAL_FREQ / info->total;
        u32 max_i    = 0;
        u32 max_freq = 0;
        
        info->total = 0;
        for ( i = 0; i < tableSize; i++ )
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
        // 最も出現頻度の高い値を誤差調整に利用
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
    
    // low_cntの計算
    info->low_cnt[ 0 ] = 0;
    for ( i = 1; i < tableSize; i++ )
    {
        info->low_cnt[ i ] = info->low_cnt[ i - 1 ] + info->freq[ i - 1 ];
    }
}


/*---------------------------------------------------------------------------*
  Name:         RCCountData_

  Description:  LZ圧縮したデータから静的レンジコーダ用のテーブルを作成します。

  Arguments:    srcp    
                srcSize 
                info8   
                info16  

  Returns:      
 *---------------------------------------------------------------------------*/
static void
RCCountData_( const u8* srcp, u32 srcSize, RCCompressionInfo* info8, RCCompressionInfo* info16 )
{
    u32 srcCnt = 0;
    u32 i;
    
    while ( srcCnt < srcSize )
    {
        u8 compFlags = srcp[ srcCnt++ ]; // 圧縮の有無を示すフラグ列
        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // 圧縮されている、length:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // リトルエンディアン
                offset    |= (srcp[ srcCnt++ ] << 8);
                
            #if BLEND_COMP_FLAG
                info8->freq[ length | 0x100 ]++;
            #else
                info8->freq[ length ]++;
            #endif
            #if !defined( LRC_ENC_OFFSET_WIDTH )
                info16->freq[ offset ]++;
            #else
                {
                    u32 offset_bit = 0;
                    while ( offset != 0 )
                    {
                        ++offset_bit;
                        offset >>= 1;
                    }
                    info16->freq[ offset_bit ]++;
                }
            #endif
            }
            else
            {
                u8 data = srcp[ srcCnt++ ];
                info8->freq[ data ]++;
            }
            compFlags <<= 1;
            if ( srcCnt >= srcSize )
            {
                break;
            }
        }
    }
    
    RCNormalizeTable_( info8  );
    RCNormalizeTable_( info16 );
}



/*---------------------------------------------------------------------------*
  Name:         RCAInitInfo_

  Description:  適応型レンジコーダのテーブル初期化
                すべての出現頻度を1で初期化します。

  Arguments:    info    

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCAInitInfo_( RCCompressionInfo* info, u8 bitSize )
{
    u32 tableSize = (1 << bitSize);
    u32 i;
    
    info->bitSize = bitSize;
    info->freq    = (u32*)malloc( sizeof(u32) * tableSize );
    info->low_cnt = (u32*)malloc( sizeof(u32) * tableSize );
    
    for ( i = 0; i < tableSize; i++ )
    {
        info->freq[ i ]    = 1;
        info->low_cnt[ i ] = i;
    }
    info->total = tableSize;
}


/*---------------------------------------------------------------------------*
  Name:         RCAAddCount_

  Description:  適応型レンジコーダの頻度テーブルを更新します。

  Arguments:    info    
                val     

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCAAddCount_( RCCompressionInfo* info, u16 val )
{
    u32 i;
    u32 tableSize = (1 << info->bitSize);
    
    info->freq[ val ]++;
    info->total++;
    for ( i = val + 1; i < tableSize; i++ )
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
        
        for ( i = 1; i < tableSize; i++ )
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
  Name:         ConvertRC
  Description:  
  Arguments:    info    
                data    
                stream  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
ConvertRC( RCCompressionInfo* info, u16 data, BitStream* stream, RCState* state, BOOL adaptive )
{
#define MIN_RANGE 0x01000000
    u32 temp    = state->range / info->total;
    u32 prevLow = state->low;
    
    state->low   += info->low_cnt[ data ] * temp;
    state->range  = info->freq[ data ] * temp;
    
    if ( adaptive )
    {
        // 出現頻度テーブルを更新
        RCAAddCount_( info, data );
    }
    
    // 桁上がりが発生する場合の処理
    if ( prevLow > state->low )
    {
        // キャリーを１くり上げ
        state->carry++;
        // キャリーと(キャリーカウンタ - 1)個の0x00を出力します。
        if ( state->carry_cnt > 1 )
        {
            BitStream_Write( stream, state->carry, RC_UNIT_BITS );
            state->carry_cnt--;
            state->carry = 0x00;
        }
        while ( state->carry_cnt > 1 )
        {
            BitStream_Write( stream, 0x00, RC_UNIT_BITS );
            state->carry_cnt--;
        }
    }
    
    // Rangeの上位1バイトが空になったら桁上げ
    while ( state->range < MIN_RANGE )
    {
        u8 candidate = (u8)( state->low >> 24 );
        // 次のキャリーが0xFFの場合は更に桁上げがあり得るのでcarryを出力せずにcarry_cntだけ増やす
        if ( candidate == 0xFF )
        {
            state->carry_cnt++;
        }
        else
        // 次のキャリーが0xFFではない場合はcarryを出力する
        {
            // carryと(carry_cnt - 1)分の0xFFを出力する
            if ( state->carry_cnt > 0 )
            {
                BitStream_Write( stream, state->carry, RC_UNIT_BITS );
                state->carry_cnt--;
            }
            while ( state->carry_cnt > 0 )
            {
                BitStream_Write( stream, 0xFF, RC_UNIT_BITS );
                state->carry_cnt--;
            }
            // 新しいcarryに置き換え
            state->carry = candidate;
            state->carry_cnt = 1;
        }
        state->low   <<= 8;
        state->range <<= 8;
    }
#undef MIN_RANGE
}

/*---------------------------------------------------------------------------*
  Name:         FinalizeRC_
  Description:  
  Arguments:    srcp    
                tmpSize 
                dstp    
                info8   
                info16  
  Returns:      
 *---------------------------------------------------------------------------*/
static void
FinalizeRC_( BitStream* stream, RCState* state )
{
    // carryと(carry_cnt - 1)分の0xFFを出力する
    if ( state->carry_cnt > 0 )
    {
        BitStream_Write( stream, state->carry, RC_UNIT_BITS );
        state->carry_cnt--;
    }
    while ( state->carry_cnt > 0 )
    {
        BitStream_Write( stream, 0xFF, RC_UNIT_BITS );
        state->carry_cnt--;
    }
    // lowに残ったデータを書き出す
    BitStream_Write( stream, state->low >> 24, RC_UNIT_BITS );
    BitStream_Write( stream, state->low >> 16, RC_UNIT_BITS );
    BitStream_Write( stream, state->low >>  8, RC_UNIT_BITS );
    BitStream_Write( stream, state->low      , RC_UNIT_BITS );
}


/*---------------------------------------------------------------------------*
  Name:         LZConvertDataRC
  Description:  
  Arguments:    srcp    
                tmpSize 
                dstp    
                info8   
                info16  
  Returns:      
 *---------------------------------------------------------------------------*/
static u32
LZConvertDataRC( const u8* srcp, u32 srcSize, u8* dstp, RCCompressionInfo* info8, RCCompressionInfo* info16 )
{
    u32 srcCnt = 0;
    u32 dstCnt = 0;
    
    BitStream stream;
    RCState   rcState;
    
    RCInitState_( &rcState );
    
    BitStream_Init( &stream, dstp );
    
    while ( srcCnt < srcSize )
    {
        u32 i;
        u8 compFlags = srcp[ srcCnt++ ]; // 圧縮の有無を示すフラグ列
        
        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // 圧縮されている、length:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // リトルエンディアン
                offset |= srcp[ srcCnt++ ] << 8;
                
                // length | 0x100をレンジコーダに掛ける
                ConvertRC( info8,  length | 0x100, &stream, &rcState, TABLE8_ADAPTIVE );
            
            #if !defined( LRC_ENC_OFFSET_WIDTH ) // テーブルサイズが大きいので強制的に適応型
                ConvertRC( info16, offset, &stream, &rcState, TABLE16_ADAPTIVE );
            #else
                {
                    u16 offset_bit = 0;
                    u16 offset_tmp = offset;
                    while ( offset_tmp > 0 )
                    {
                        offset_tmp >>= 1;
                        ++offset_bit;
                    }
                    ConvertRC( info16, offset_bit, &stream, &rcState, TABLE16_ADAPTIVE );
                    // offsetが0であることはないので、最上位のビットは省略する
                    BitStream_Write( &stream, offset & ~(1 << (offset_bit - 1)), offset_bit - 1 );
                }
            #endif
            }
            else
            {
                u8 data = srcp[ srcCnt++ ];
                
                ConvertRC( info8, data, &stream, &rcState, TABLE8_ADAPTIVE );
            }
            compFlags <<= 1;
            if ( srcCnt >= srcSize )
            {
                break;
            }
        }
    }
    // carryとlowに残ったデータを吐き出す
    FinalizeRC_( &stream, &rcState );
    
    BitStream_Terminate( &stream, 4 );
    return stream.cnt;
}


/*---------------------------------------------------------------------------*
  Name:         LRCCompWrite

  Description:  

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
u32 LRCCompWrite( const u8* srcp, u32 srcSize, u8* dstp )
{
    RCCompressionInfo sRCInfo8;
    RCCompressionInfo sRCInfo16;
    
    u32 tmpSize;
    u32 dstSize;
    u8* tmpBuf = (u8*)malloc( srcSize * 3 );
    // まずはsrcpを普通にLZ圧縮
    tmpSize = LZCompWrite_( srcp, srcSize, tmpBuf, 2, LRC_OFFSET_BITS );
    
    // テーブル初期化
    RCInitInfo_( &sRCInfo8,  LENGTH_BITS );
    RCInitInfo_( &sRCInfo16, LRC_OFFSET_TABLE_BITS );
    
    dstSize = 0;
    
    // ヘッダの書き込み
    if ( srcSize < 0x1000000 && srcSize > 0 )
    {
        *(u32*)dstp = LRC_CODE_HEADER | ( srcSize << 8 );
        dstSize = 4;
    }
    else
    {
        *(u32*)dstp     = LRC_CODE_HEADER;
        *(u32*)&dstp[4] = srcSize;
        dstSize = 8;
    }
    
    // 静的レンジコーダの頻度表を作成だけしておく(使用するかどうかはオプション次第)
    RCCountData_( tmpBuf, tmpSize, &sRCInfo8, &sRCInfo16 );
    
#if TABLE8_ADAPTIVE
    RCAInitInfo_( &sRCInfo8,  LENGTH_BITS );
#else // if (! TABLE8_ADAPTIVE )
    dstSize += RCExportTable_( &dstp[ dstSize ], &sRCInfo8 );
#endif
    
#if TABLE16_ADAPTIVE
    RCAInitInfo_( &sRCInfo16, LRC_OFFSET_TABLE_BITS );
#else // if (! TABLE16_ADAPTIVE )
    dstSize += RCExportTable_( &dstp[ dstSize ], &sRCInfo16 );
#endif
    
    // 圧縮結果をレンジコーダ符号化しながら出力
    dstSize += LZConvertDataRC( tmpBuf, tmpSize, &dstp[ dstSize ], &sRCInfo8, &sRCInfo16 );
    
    return dstSize;
}



/*---------------------------------------------------------------------------*
  Name:         SearchRC_

  Description:  

  Arguments:    info    
                code    
                range   
                low     

  Returns:      
 *---------------------------------------------------------------------------*/
static u16
SearchRC_( RCCompressionInfo* info, u32 code, u32 range, u32 low )
{
    u32 tableSize = (1 << info->bitSize);
    u32 codeVal = code - low;
    u32 i;
    u32 temp = range / info->total;
    u32 tempVal = codeVal / temp;
    
#if 0
    // TODO: とりあえず線形探索、二分探索にするべき
    for ( i = 0; i < tableSize - 1; i++ )
    {
        if ( info->low_cnt[ i + 1 ] > tempVal )
        {
            while ( info->freq[ i ] == 0 )
            {
                --i;
            }
            return (u16)i;
        }
    }
    return (u16)( (1 << info->bitSize) - 1 );
#else
    // 二分探索
    u32 left  = 0;
    u32 right = tableSize - 1;
    
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
    return (u16)i;
    
#endif
}



static u16
RCGetDate_( BitReader* stream, RCCompressionInfo* info, RCState* state, BOOL adaptive )
{
#define MIN_RANGE 0x01000000
    u16 val = SearchRC_( info, state->code, state->range, state->low );
    
    {
        u32 tmp;
        tmp          =  state->range / info->total;
        state->low   += info->low_cnt[ val ] * tmp;
        state->range =  info->freq[ val ] * tmp;
    }
    
    // 出現頻度テーブルを更新
    if ( adaptive )
    {
        RCAAddCount_( info, val );
    }
    while ( state->range < MIN_RANGE )
    {
        state->code  <<= 8;
        state->code += BitReader_ReadEx( stream, 8 );
        state->range <<= 8;
        state->low   <<= 8;
    }
    return val;
#undef MIN_RANGE
}


/*---------------------------------------------------------------------------*
  Name:         LRCCompRead

  Description:  

  Arguments:    srcp    
                size    
                dstp    

  Returns:      
 *---------------------------------------------------------------------------*/
s32 LRCCompRead( const u8* srcp, u32 srcSize, u8* dstp )
{
    RCCompressionInfo sRCInfo8;
    RCCompressionInfo sRCInfo16;
    RCState rcState;
    
    u32 dstSize;
    u32 srcCnt = 0;
    u32 dstCnt = 0;
    BitReader stream;
    
    if ( srcSize < 4 )
    {
        return -1;
    }
    
    // ヘッダの読み込み
    dstSize = *(u32*)srcp >> 8;
    srcCnt = 4;
    if ( dstSize == 0 )
    {
        if ( srcSize < 8 )
        {
            return -1;
        }
        dstSize = *(u32*)(srcp + 4);
        srcCnt += 4;
    }
    
    // RC頻度テーブルを読む
#if TABLE8_ADAPTIVE
    RCAInitInfo_( &sRCInfo8, LENGTH_BITS );
#else
    RCInitInfo_( &sRCInfo8,  LENGTH_BITS );
    srcCnt += RCImportTable_( &sRCInfo8, &srcp[ srcCnt ], srcSize - srcCnt );
#endif
#if TABLE16_ADAPTIVE
    RCAInitInfo_( &sRCInfo16, LRC_OFFSET_TABLE_BITS );
#else
    RCInitInfo_( &sRCInfo16, LRC_OFFSET_TABLE_BITS );
    srcCnt += RCImportTable_( &sRCInfo16, &srcp[ srcCnt ], srcSize - srcCnt );
#endif
    
    BitReader_Init( &stream, &srcp[ srcCnt ], srcSize - srcCnt );
    
    if ( srcSize - srcCnt <= 4 )
    {
        // 最低でも初回のコード分の4Byteは必要
        return -1;
    }
    
    RCInitState_( &rcState );
    
    rcState.code = (u32)( (BitReader_ReadEx( &stream, 8 ) << 24) |
                          (BitReader_ReadEx( &stream, 8 ) << 16) |
                          (BitReader_ReadEx( &stream, 8 ) <<  8) |
                          (BitReader_ReadEx( &stream, 8 )      ) );
    
    while ( dstCnt < dstSize )
    {
        u16 val = (u16)( RCGetDate_( &stream, &sRCInfo8, &rcState, TABLE8_ADAPTIVE ) );
        
        if ( val < 0x100 )
        // 非圧縮データ
        {
            dstp[ dstCnt++ ] = (u8)val;
        }
        else
        // 圧縮データ
        {
            u16 length = (val & 0xFF) + 3;
            val = (u16)( RCGetDate_( &stream, &sRCInfo16, &rcState, TABLE16_ADAPTIVE ) );
            
        #if defined( LRC_ENC_OFFSET_WIDTH )
            {
                u16 offset_bit = val;
                val = 0;
                if ( offset_bit > 0 )
                {
                    val = 1;
                    while ( --offset_bit > 0 )
                    {
                        val <<= 1;
                        val |= BitReader_Read( &stream );
                    }
                }
            }
        #endif
            val += 1;
            
            // バッファオーバーランをチェック
            if ( dstCnt + length > dstSize )
            {
                return -1;
            }
            if ( dstCnt < val )
            {
                return -1;
            }
            if ( srcCnt + stream.cnt > srcSize )
            {
                return -1;
            }

            while ( length-- > 0 )
            {
                dstp[ dstCnt ] = dstp[ dstCnt - val ];
                ++dstCnt;
            }
        }
    }
    
    return dstCnt;
}


