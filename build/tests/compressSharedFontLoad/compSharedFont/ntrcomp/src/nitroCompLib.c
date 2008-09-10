/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     nitroCompLib.c

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

#include "nitroCompLib.h"

#undef _DEBUG
#ifdef _DEBUG
#define new DEBUG_NEW
#define DEBUG_PRINT
// #define DEBUG_PRINT_DIFFFILT
#define DEBUG_PRINT_RL
// #define DEBUG_PRINT_HUFF
// #define DEBUG_PRINT_LZ
// #define DEBUG_PRINT_DATAMATCH
#endif


#ifdef __cplusplus
#define EXTERN extern "C"
#define STATIC
#else
#define EXTERN
#define STATIC static
#endif


#ifdef DEBUG_PRINT
#define dbg_printf fprintf
#else
#define dbg_printf dummy
#endif

#ifdef DEBUG_PRINT_DIFFFILT
#define dbg_printf_dif fprintf
#else
#define dbg_printf_dif dummy
#endif

#ifdef DEBUG_PRINT_RL
#define dbg_printf_rl fprintf
#else
#define dbg_printf_rl dummy
#endif

#ifdef DEBGU_PRINT_HUFF
#define dbg_printf_huff fprintf
#else
#define dbg_printf_huff dummy
#endif

#ifdef DEBGU_PRINT_LZ
#define dbg_printf_lz fprintf
#else
#define dbg_printf_lz dummy
#endif

#ifdef DEBUG_PRINT_DATAMATCH
#define dbg_printf_match fprintf
#else
#define dbg_printf_match dummy
#endif

void dummy(void *fp, ...)
{
}


//==================================================================================
// グローバル変数宣言
//==================================================================================
static u8 *pCompBuf[2];                // 圧縮処理中に用いるダブルバッファ
static u8 compBufNo = 1;               // 有効なダブルバッファを示す

//==================================================================================
// プロトタイプ宣言
//==================================================================================
static u32 RawWrite(u8 *srcp, u32 size, u8 *dstp);
static u32 DiffFiltWrite(u8 *srcp, u32 size, u8 *dstp, u8 diffBitSize);
static u32 RLCompWrite(u8 *srcp, u32 size, u8 *dstp);
static u32 LZCompWriteEx(u8 *srcp, u32 size, u8 *dstp, u8 lzSearchOffset, BOOL ex_available);
static u32 HuffCompWrite(u8 *srcp, u32 size, u8 *dstp, u8 huffBitSize);

static s32 RawRead     ( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize );
static s32 DiffFiltRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, u8 diffBitSize );
static s32 RLCompRead  ( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize );
static s32 LZCompReadEx( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, BOOL ex_available );
static s32 HuffCompRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, u8 huffBitSize );

/*
//==================================================================================
// DLL用関数
//==================================================================================
EXTERN BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}
*/

//----------------------------------------------------------------------------------
//  圧縮後のデータを置くためのメモリ領域を確保
//    圧縮前のデータの2倍の領域を確保する
//----------------------------------------------------------------------------------
EXTERN u8 * STDCALL nitroCompMalloc(u32 size)
{
    return (u8 *)malloc(size * 3 + 512);
}

//----------------------------------------------------------------------------------
//  圧縮後のデータを置いていたメモリ領域を解放
//----------------------------------------------------------------------------------
EXTERN void STDCALL nitroCompFree(u8 *p)
{
    if (p != NULL)
    {
        free((void *)p);
        p = NULL;
    }
}

//------------------------------------------------------------
//  データの圧縮
//------------------------------------------------------------
EXTERN u32 STDCALL nitroCompress( const u8 *srcp, u32 srcSize, u8 *dstp, char *compList, u8 rawHeaderFlag )
{
    char   *pCompList;                // compListの現在の参照ポイント
    u32     dataSize, nextDataSize;    // 圧縮データのサイズ(バイト単位)
    u8     *pReadBuf;                  // 圧縮データの先頭番地を指すポインタ
    u8      bitSize;                   // 差分フィルタ，ハフマン符号化の適用単位
    char    str[16];
    u16     i, j;
    u8      lzSearchOffset;

//  pCompBuf[0] = (u8 *)malloc(srcSize*2 + 4 + 256*2);     // 最悪のハフマンが size*2 + 4 + 256*2 なので、
//  pCompBuf[1] = (u8 *)malloc(srcSize*2 + 4 + 256*2);     // その他の圧縮やデータヘッダ追加で、不足する可能性あり
    pCompBuf[0] = (u8 *)malloc(srcSize * 3 + 256 * 2);
    pCompBuf[1] = (u8 *)malloc(srcSize * 3 + 256 * 2);
    pReadBuf = pCompBuf[0];
    compBufNo = 1;                     // 重要!!　これをしないと２回目にnitroCompressを呼び出したときにおかしくなる

    // malloc チェック
    if (pCompBuf[0] == NULL || pCompBuf[1] == NULL)
    {
        fprintf(stderr, "Error: Memory is not enough.\n");
        exit(1);
    }

    dataSize = srcSize;

    // NULLヘッダ(圧縮前のデータ用の、擬似ヘッダ) の追加処理
    if (rawHeaderFlag)
    {
        dataSize += 4;
        *(u32 *)pReadBuf = srcSize << 8 | 0;       // データ・ヘッダ
        memcpy(&pReadBuf[4], srcp, srcSize);
    }
    else
    {
        memcpy(pReadBuf, srcp, srcSize);
    }

    pCompList = compList;              // 圧縮順序を格納した配列をポイント

    // 圧縮順序を格納した配列に要素がある限り、ループ
    while (1)
    {
        switch (*pCompList)
        {
        case 'd':
            {
                pCompList++;           // 'd' の次には、8 か 16 
                str[0] = *pCompList;
                if (*pCompList == '1')
                {
                    pCompList++;
                    str[1] = *pCompList;
                    str[2] = '\n';
                }
                bitSize = atoi(str);   // 差分フィルタの適用単位を格納
                str[0] = str[1] = '\n';
                
                dbg_printf(stderr, "nitroCompress  Diff %d\n", bitSize);
                
                if ((bitSize == 16) && (dataSize & 0x01))
                {
                    fprintf(stderr, "16-bit differencial filter must be 2-byte allignment.\n");
                    exit(1);
                }
                nextDataSize = DiffFiltWrite(pReadBuf, dataSize, pCompBuf[compBufNo], bitSize);
            }
            break;
            
        case 'r':
            {
                dbg_printf(stderr, "nitroCompress  RL\n");
                
                nextDataSize = RLCompWrite(pReadBuf, dataSize, pCompBuf[compBufNo]);
            }
            break;
            
        case 'l':
        case 'L':
            {
                BOOL ex_format = (*pCompList == 'L')? TRUE : FALSE;
                
                pCompList++;
                i = 0;
                while (isdigit(*pCompList))
                {
                    str[i] = *pCompList;
                    pCompList++;
                    i++;
                    if (i == 15)
                    {
                        break;
                    }
                }
                str[i] = '\n';
                pCompList--;
                lzSearchOffset = (u8)atoi(str); // 大きな値は切り捨てて丸め
                for (j = 0; j < i; j++)
                {
                    str[j] = '\n';
                }
                dbg_printf(stderr, "nitroCompress  L %d\n", lzSearchOffset);
                
                nextDataSize = LZCompWriteEx(pReadBuf, dataSize, pCompBuf[compBufNo], lzSearchOffset, ex_format);
            }
            break;
            
        case 'h':
            {
                pCompList++;           // 'h' の次には、4 か 8
                str[0] = *pCompList;
                str[1] = '\n';
                bitSize = atoi(str);   // 4 or 8
                str[0] = '\n';
                
                dbg_printf(stderr, "nitroCompress  Huff %d\n", bitSize);
                
                nextDataSize = HuffCompWrite(pReadBuf, dataSize, pCompBuf[compBufNo], bitSize);
            }
            break;
            //-----------------------------------------
            // 圧縮終了 (*CompTypeBufp が NULL)
        default:
            {
                dbg_printf(stderr, "nitroCompress  raw\n");
                
                RawWrite(pReadBuf, dataSize, dstp);
                if (pCompBuf[0] != NULL)
                {
                    free((void *)pCompBuf[0]);
                    pCompBuf[0] = NULL;
                }
                if (pCompBuf[1] != NULL)
                {
                    free(pCompBuf[1]);
                    pCompBuf[1] = NULL;
                }
                return dataSize;
            }
        }
        // もう一周
        pReadBuf = pCompBuf[compBufNo];
        compBufNo ^= 0x01;
        dataSize = nextDataSize;
        pCompList++;
    }
}


//===========================================================================
//  圧縮データのコピー
//===========================================================================
static u32 RawWrite(u8 *srcp, u32 size, u8 *dstp)
{
    u32     i;

    dbg_printf(stderr, "RawWrite\tsize=%d\n\n", size);

    size = (size + 0x3) & ~0x3;
    for (i = 0; i < size - 1; i++)
    {
        *dstp = *srcp;
        dstp++;
        srcp++;
    }
    *dstp = *srcp;

    return size;
}


//===========================================================================
//  差分フィルタ
//===========================================================================
static u32 DiffFiltWrite(u8 *srcp, u32 size, u8 *dstp, u8 diffBitSize)
{
    u32     DiffCount;                 // 圧縮データのバイト数
    u32     i;

    u16    *src16p = (u16 *)srcp;
    u16    *dst16p = (u16 *)dstp;

    dbg_printf_dif(stderr, "DiffFiltWrite\tsize=%d\tdiffBitSize=%d\n", size, diffBitSize);

    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | (DIFF_CODE_HEADER | diffBitSize / 8);    // データ・ヘッダ
        DiffCount = 4;
    }
    else
    {
        *(u32 *)dstp = (DIFF_CODE_HEADER | diffBitSize / 8);    // データ・ヘッダ
        *(u32 *)(dstp + 4) = size;
        DiffCount = 8;
    }
    
    if (diffBitSize == 8)
    {
#ifdef DEBUG_PRINT_DIFFFILT
        for (i = 0; i < 16; i++)
        {
            dbg_printf_dif(stderr, "srcp[%d] = %x\n", i, srcp[i]);
        }
#endif
        dstp[DiffCount] = srcp[0];     // 先頭データのみ差分無し
        DiffCount++;
        for (i = 1; i < size; i++, DiffCount++)
        {
            dbg_printf_dif(stderr, "dstp[%x] = srcp[%d]-srcp[%d] = %x - %x = %x\n",
                           DiffCount, i, i - 1, srcp[i], srcp[i - 1], srcp[i] - srcp[i - 1]);

            dstp[DiffCount] = srcp[i] - srcp[i - 1];    // 差分データ格納
        }
    }
    else                               // 16ビットサイズ 
    {
        dst16p[DiffCount / 2] = src16p[0];
        DiffCount += 2;
        for (i = 1; i < size / 2; i++, DiffCount += 2)
        {
            dst16p[DiffCount / 2] = src16p[i] - src16p[i - 1];
        }
    }

    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含めない
    i = 0;
    while ((DiffCount + i) & 0x3)
    {
        dstp[DiffCount + i] = 0;
        i++;
    }

    return DiffCount;
}


//===========================================================================
//  ランレングス符号化 (バイト単位)
//===========================================================================
static u32 RLCompWrite(u8 *srcp, u32 size, u8 *dstp)
{
    u32     RLDstCount;                // 圧縮データのバイト数
    u32     RLSrcCount;                // 圧縮対象データの処理済データ量(バイト単位)
    u8      RLCompFlag;                // ランレングス符号化を行う場合１
    u8      runLength;                 // ランレングス
    u8      rawDataLength;             // ランになっていないデータのレングス
    u32     i;

    u8     *startp;                    // 一回の処理ループにおける、圧縮対象データの先頭をポイント

    dbg_printf_rl(stderr, "RLCompWrite\tsize=%d\n", size);
    
    //  データヘッダ        (サイズは展開後のもの)
    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | RL_CODE_HEADER;  // データ・ヘッダ
        RLDstCount = 4;
    }
    else
    // サイズが24bitに収まらない場合には拡張形式のヘッダとなる
    {
        *(u32 *)dstp = RL_CODE_HEADER;
        *(u32 *)(dstp + 4) = size;
        RLDstCount = 8;
    }
    
    RLSrcCount = 0;
    rawDataLength = 0;
    RLCompFlag = 0;

    while (RLSrcCount < size)
    {
        startp = &srcp[RLSrcCount];    // 圧縮対象データの設定

        for (i = 0; i < 128; i++)      // 7ビットで表現できるデータ量が 0~127
        {
            // 圧縮対象データの末尾に到達
            if (RLSrcCount + rawDataLength >= size)
            {
                rawDataLength = (u8)(size - RLSrcCount);
                break;
            }

            if (RLSrcCount + rawDataLength + 2 < size)
            {
                if (startp[i] == startp[i + 1] && startp[i] == startp[i + 2])
                {
                    RLCompFlag = 1;
                    break;
                }
            }
            rawDataLength++;
        }

        // 符号化しないデータを格納
        // データ長格納バイトの8ビット目が0なら、符号化しないデータ系列
        // データ長は -1 した数になるので、0-127 が 1-128 となる
        if (rawDataLength)
        {
            dstp[RLDstCount++] = rawDataLength - 1;     // "データ長-1" 格納(7ビット)
            for (i = 0; i < rawDataLength; i++)
            {
                dstp[RLDstCount++] = srcp[RLSrcCount++];
            }
            rawDataLength = 0;
        }

        // ランレングス符号化
        if (RLCompFlag)
        {
            runLength = 3;
            for (i = 3; i < 128 + 2; i++)
            {
                // 圧縮用データの末尾に到達
                if (RLSrcCount + runLength >= size)
                {
                    runLength = (u8)(size - RLSrcCount);
                    break;
                }

                // ランが途切れた場合
                if (srcp[RLSrcCount] != srcp[RLSrcCount + runLength])
                {
                    break;
                }
                // ラン継続中
                runLength++;
            }

            // データ長格納バイトの8ビット目が1なら、符号化したデータ系列
            dstp[RLDstCount++] = 0x80 | (runLength - 3);        // ３の下駄をはかせて、3~130を格納
            dstp[RLDstCount++] = srcp[RLSrcCount];
            RLSrcCount += runLength;
            RLCompFlag = 0;
        }
    }

    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含めない
    i = 0;
    while ((RLDstCount + i) & 0x3)
    {
        dstp[RLDstCount + i] = 0;
        i++;
    }

    return RLDstCount;
}


//===========================================================================
//  LZ77圧縮
//===========================================================================

#define LZ_OFFSET_BITS     12
#define LZ_OFFSET_SIZE     (1<<LZ_OFFSET_BITS)

typedef struct
{
    u16 WindowPos;              // 履歴窓の先頭位置
    u16 WindowLen;              // 履歴窓の長さ
    
    s16 LZOffsetTable[LZ_OFFSET_SIZE];    // 履歴窓のオフセットバッファ
    s16 LZByteTable[256];       // キャラクタの最新履歴へのポインタ
    s16 LZEndTable[256];        // キャラクタの最古履歴へのポインタ
} LZCompressInfo;

static u32 SearchLZ( const LZCompressInfo * info, const u8 *nextp, u32 remainSize, u16 *offset, u16 minOffset, u32 maxLength );

static void LZInitTable( LZCompressInfo* info )
{
    u16     i;
    
    for (i = 0; i < 256; i++)
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
static void SlideByte(LZCompressInfo * info, const u8 *srcp)
{
    s16     offset;
    u8      in_data = *srcp;
    u16     insert_offset;
    
    s16    *const LZByteTable   = info->LZByteTable;
    s16    *const LZEndTable    = info->LZEndTable;
    s16    *const LZOffsetTable = info->LZOffsetTable;
    const u16 windowPos = info->WindowPos;
    const u16 windowLen = info->WindowLen;
    
    if (windowLen == LZ_OFFSET_SIZE)
    {
        u8  out_data = *(srcp - LZ_OFFSET_SIZE);
        if ((LZByteTable[out_data] = LZOffsetTable[LZByteTable[out_data]]) == -1)
        {
            LZEndTable[out_data] = -1;
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
    }
    else
    {
        LZOffsetTable[offset] = insert_offset;
    }
    LZEndTable[in_data] = insert_offset;
    LZOffsetTable[insert_offset] = -1;

    if (windowLen == LZ_OFFSET_SIZE)
    {
        info->WindowPos = (u16)((windowPos + 1) % LZ_OFFSET_SIZE);
    }
    else
    {
        info->WindowLen++;
    }
}

static void LZSlide(LZCompressInfo * info, const u8 *srcp, u32 n)
{
    u32     i;

    for (i = 0; i < n; i++)
    {
        SlideByte(info, srcp++);
    }
}


/*---------------------------------------------------------------------------*
  Name:         LZCompWriteEx

  Description:  LZ77圧縮を行なう関数(最大lengthの拡張)

  Arguments:    srcp            圧縮元データへのポインタ
                size            圧縮元データサイズ
                dstp            圧縮先データへのポインタ
                                圧縮元データよりも大きいサイズのバッファが必要です。

  Returns:      圧縮後のデータサイズ。
                圧縮後のデータが圧縮前よりも大きくなる場合には圧縮を中断し0を返します。
 *---------------------------------------------------------------------------*/
static u32 LZCompWriteEx(u8 *srcp, u32 size, u8 *dstp, u8 lzSearchOffset, BOOL ex_available )
{
    static LZCompressInfo sLZInfo;
    
    u32     LZDstCount;                // 圧縮データのバイト数
    u8      LZCompFlags;               // 圧縮の有無を示すフラグ系列
    u8     *LZCompFlagsp;              // LZCompFlags を格納するメモリ領域をポイント
    u16     lastOffset;                // 一致データまでのオフセット (その時点での最長一致データ) 
    u32     lastLength;                // 一致データ長 (その時点での最長一致データ)
    u8      i;
    const u32 MAX_LENGTH = (ex_available)? (0xFFFF + 0xFF + 0xF + 3) : (0xF + 3);
    
    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | LZ_CODE_HEADER | (ex_available? 1 : 0 );  // データ・ヘッダ
        dstp += 4;
        LZDstCount = 4;
    }
    else
    {
        *(u32 *)dstp = LZ_CODE_HEADER | (ex_available? 1 : 0);
        *(u32 *)(dstp + 4) = size;
        dstp += 8;
        LZDstCount = 8;
    }
    LZInitTable( &sLZInfo );
    
    while (size > 0)
    {
        LZCompFlags  = 0;
        LZCompFlagsp = dstp++;         // フラグ系列の格納先
        LZDstCount++;
        
        // フラグ系列が8ビットデータとして格納されるため、8回ループ
        for (i = 0; i < 8; i++)
        {
            LZCompFlags <<= 1;         // 初回 (i=0) は特に意味はない
            if (size <= 0)
            {
                // 終端に来た場合はフラグを最後までシフトさせてから終了
                continue;
            }

            if ( (lastLength = SearchLZ(&sLZInfo, srcp, size, &lastOffset, lzSearchOffset, MAX_LENGTH)) != 0)
            {
                u32 length;
                // 圧縮可能な場合はフラグを立てる
                LZCompFlags |= 0x1;
                
                if ( ex_available )
                {
                    if ( lastLength >= 0xFF + 0xF + 3 )
                    {
                        length  = lastLength - 0xFF - 0xF - 3;
                        *dstp++ = (u8)( 0x10 | (length >> 12) );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 2;
                    }
                    else if ( lastLength >= 0xF + 2 )
                    {
                        length = lastLength - 0xF - 2;
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 1;
                    }
                    else
                    {
                        length = lastLength - 1;
                    }
                }
                else
                {
                    length = lastLength - 3;
                }
                // オフセットは上位4ビットと下位8ビットに分けて格納
                *dstp++ = (u8)( length << 4 | (lastOffset - 1) >> 8 );
                *dstp++ = (u8)( (lastOffset - 1) & 0xFF );
                LZDstCount += 2;
                LZSlide( &sLZInfo, srcp, lastLength);
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // 圧縮なし
                LZSlide(&sLZInfo, srcp, 1);
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


//--------------------------------------------------------
// LZ77圧縮でスライド窓の中から最長一致列を検索します。
//  Arguments:    startp                 データの開始位置を示すポインタ
//                nextp                  検索を開始するデータのポインタ
//                remainSize             残りデータサイズ
//                offset                 一致したオフセットを格納する領域へのポインタ
//  Return   :    一致列が見つかった場合は   TRUE
//                見つからなかった場合は     FALSE
//--------------------------------------------------------
static u32 SearchLZ( const LZCompressInfo * info, const u8 *nextp, u32 remainSize, u16 *offset, u16 minOffset, u32 maxLength )
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     currOffset;
    u32     currLength = 2;
    u32     tmpLength;
    s32     w_offset;
    const s16 * const LZOffsetTable = info->LZOffsetTable;
    const u16 windowPos = info->WindowPos;
    const u16 windowLen = info->WindowLen;

    if (remainSize < 3)
    {
        return 0;
    }

    w_offset = info->LZByteTable[*nextp];

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
            w_offset = LZOffsetTable[w_offset];
            continue;
        }
        
        if (nextp - searchp < minOffset)
        {
            // VRAMは2バイトアクセスなので (VRAMからデータを読み出す場合があるため)、
            // 検索対象データは2バイト前からのデータにしなければならない。
            // 
            // オフセットは12ビットで格納されるため、4096以下
            break;
        }
        tmpLength = 3;
        searchHeadp = searchp + 3;
        headp = nextp + 3;
        
        while (((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;
            
            // 一致長が最大なので、検索を終了する
            if (tmpLength == maxLength)
            {
                break;
            }
        }
        if (tmpLength > currLength)
        {
            // 最大長オフセットを更新
            currLength = tmpLength;
            currOffset = (u16)(nextp - searchp);
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


//===========================================================================
//  ハフマン符号化
//===========================================================================
#define HUFF_END_L  0x80
#define HUFF_END_R  0x40

typedef struct
{
    u16     No;                        // データNo
    s16     PaNo;                      // 親No 
    u32     Freq;                      // 出現頻度
    s16     ChNo[2];                   // 子No (0: 左側， 1: 右側)
    u16     PaDepth;                   // 親ノードの深さ
    u16     LeafDepth;                 // 葉までの深さ
    u32     HuffCode;                  // ハフマン符号
    u8      Bit;                       // ノードのビットデータ
    u16     HWord;                     // 各中間節点において、その節点をルートとする部分木を HuffTree 格納に必要なメモリ量
}
HuffData;

typedef struct
{
    u8      leftOffsetNeed;            // 左の子節点へのオフセットが必要なら1
    u8      rightOffsetNeed;           // 右の子節点へのオフセットが必要なら1
    u16     leftNodeNo;                // 左の子節点No
    u16     rightNodeNo;               // 右の子節点No
}
HuffTreeCtrlData;

// ハフマンワークバッファ構成
typedef struct
{
    HuffData         huffTable[ 512 ];     //  huffTable[ 512 ];      12288B
    u8               huffTree [ 256 * 2 ]; //  huffTree[ 256 * 2 ];     512B
    HuffTreeCtrlData huffTreeCtrl[ 256 ];  //  huffTreeCtrl[ 256 ];    1536B
    u16              huffTreeTop;          //  
    u8               bitSize;              //  
    u8               padding_[1];          //  
}
HuffCompressionInfo;                       // 計 14340B

static void     HuffInitTable( HuffCompressionInfo* info, u8 bitSize );
static void     HuffCountData( HuffData* table, const u8 *srcp, u32 size, u8 bitSize );
static void     HuffConstructTree( HuffCompressionInfo *info, u8 bitSize );
static u32      HuffExportTree( u8* dstp, HuffCompressionInfo* info );
static u32      HuffConvertData( const HuffData *table, const u8* srcp, u8* dstp, u32 srcSize, u8 bitSize );

static void     HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo    );
static void     HuffAddCodeToTable       ( HuffData *table, u16 nodeNo, u32 paHuffCode );
static u16      HuffAddCountHWordToTable ( HuffData *table, u16 nodeNo );

static u16      HuffMakeNode                 ( HuffData* table, u8 bitSize );
static void     HuffMakeHuffTree             ( HuffCompressionInfo* info, u16 rootNo );
static void     HuffMakeSubsetHuffTree       ( HuffCompressionInfo* info, u16 huffTreeNo, BOOL rightNodeFlag );
static BOOL     HuffRemainingNodeCanSetOffset( HuffCompressionInfo* info, u16 costHWord );
static void     HuffSetOneNodeOffset         ( HuffCompressionInfo* info, u16 huffTreeNo, BOOL rightNodeFlag );

HuffCompressionInfo sHuffCompressionInfo;

/*---------------------------------------------------------------------------*
  Name:         HuffCompWrite
  Description:  ハフマン圧縮
  Arguments:    *srcp   
                size    
                *dstp   
                huffBitSize
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u32 HuffCompWrite(u8 *srcp, u32 size, u8 *dstp, u8 huffBitSize)
{
    u32     huffDstCount;              // 圧縮データのバイト数
    u32     offset;
    HuffCompressionInfo* info = &sHuffCompressionInfo;
    
    u16 huffDataNum = 1 << huffBitSize;   // 8->256, 4->16
    
    // テーブル初期化
    HuffInitTable( info, huffBitSize );
    
    // 出現頻度チェック
    HuffCountData( info->huffTable, srcp, size, huffBitSize );
    
    // ハフマン符号テーブル作成
    HuffConstructTree( info, huffBitSize );
    
    // データ・ヘッダ
    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | HUFF_CODE_HEADER | huffBitSize;
        offset = 4;
    }
    else
    {
        *(u32 *)dstp = HUFF_CODE_HEADER | huffBitSize;
        *(u32 *)(dstp + 4) = size;
        offset = 8;
    }
    huffDstCount = offset;
    
    // ハフマンテーブルをバイナリ出力
    huffDstCount += HuffExportTree( &dstp[ huffDstCount ], info );
    
    // ハフマンテーブルによるデータ変換
    huffDstCount += HuffConvertData( info->huffTable, srcp, &dstp[ huffDstCount ], size, huffBitSize );
    
    return huffDstCount;
}




/*---------------------------------------------------------------------------*
  Name:         HuffInitTable
  Description:  ハフマンテーブルの初期化
  Arguments:    table   
                size    
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffInitTable( HuffCompressionInfo* info, u8 bitSize )
{
    u32 tableSize = (1 << bitSize);
    u32 i;
    
    info->huffTreeTop  = 1;
    info->bitSize      = bitSize;
    
    // huffTableを初期化
    {
        HuffData* table = info->huffTable;
        
        const HuffData  HUFF_TABLE_INIT_DATA  = { 0, 0, 0, {-1, -1}, 0, 0, 0, 0, 0 };
        for ( i = 0; i < tableSize * 2U; i++ )
        {
            table[ i ]    = HUFF_TABLE_INIT_DATA;
            table[ i ].No = (u16)i;
        }
    }
    
    // huffTree, huffTreeCtrlを初期化
    {
        const HuffTreeCtrlData HUFF_TREE_CTRL_INIT_DATA = { 1, 1, 0, 0 };
        u8*               huffTree     = info->huffTree;
        HuffTreeCtrlData* huffTreeCtrl = info->huffTreeCtrl;
        
        for ( i = 0; i < 256; i++ )
        {
            huffTree[ i * 2 ]     = 0;
            huffTree[ i * 2 + 1 ] = 0;
            huffTreeCtrl[ i ]     = HUFF_TREE_CTRL_INIT_DATA;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HuffCountData
  Description:  出現頻度のカウント
  Arguments:    table   
                *srcp   
                size    
                bitSize 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffCountData( HuffData* table, const u8 *srcp, u32 size, u8 bitSize )
{
    u32 i;
    u8  tmp;
    
    if ( bitSize == 8 )
    {
        for ( i = 0; i < size; i++ )
        {
            table[ srcp[ i ] ].Freq++; // 8ビット符号化
        }
    }
    else
    {
        for ( i = 0; i < size; i++ )   // 4ビット符号化
        {
            tmp = (srcp[ i ] & 0xf0) >> 4;
            table[ tmp ].Freq++;     // 上位4ビットから先に格納// どっちでもいい
            tmp = srcp[ i ] & 0x0f;
            table[ tmp ].Freq++;     // 問題は符号化のとこ
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HuffConstructTree
  Description:  ハフマンツリーを構築
  Arguments:    *table  
                dataNum 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffConstructTree( HuffCompressionInfo *info, u8 bitSize )
{
    HuffData* table = info->huffTable;
    u16     rootNo;                  // 二分木のルートNo
    
    // 出現頻度からノードを構築
    rootNo = HuffMakeNode( table, bitSize );
    
    // ハフマンコード生成 (table[i].HuffCode に)
    HuffAddCodeToTable( table, rootNo, 0x00 );        // PaDepthのビット数だけ、HuffCode の下位ビットをマスクしたものがハフマンコード
    
    // 各中間節点において、その節点をルートとする部分木を huffTree 格納に必要なメモリ量の計算
    HuffAddCountHWordToTable( table, rootNo );
    
    // sHuffTreeBuf.huffTree 作成
    HuffMakeHuffTree( info, rootNo );
    
    info->huffTree[0] = (u8)( --info->huffTreeTop );
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
                if ( leftNo < 0 )
                {
                    leftNo = 0;
                }
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
static void HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo )
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
static void HuffAddCodeToTable( HuffData* table, u16 nodeNo, u32 paHuffCode )
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
static u16 HuffAddCountHWordToTable( HuffData *table, u16 nodeNo)
{
    u16     leftHWord, rightHWord;
    
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


//-----------------------------------------------------------------------
// ハフマンコード表作成
//-----------------------------------------------------------------------
static void HuffMakeHuffTree( HuffCompressionInfo* info, u16 rootNo )
{
    s16     i;
    s16     costHWord, tmpCostHWord;            // 部分木のコード表を作成しなかった時のコスト 最大値の節点の部分木コード表を作る
    s16     costOffsetNeed, tmpCostOffsetNeed;
    s16     costMaxKey;                         // コスト最小の節点を huffTreeBuf.huffTree から特定するための情報
    BOOL    costMaxRightFlag;
    u16     offsetNeedNum;
    BOOL    tmpRightFlag;
    const u32 MAX_COST = 64;
    
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
            
            if ( info->huffTreeCtrl[i].rightOffsetNeed)
            {
                tmpCostHWord = (s16)info->huffTable[info->huffTreeCtrl[i].rightNodeNo].HWord;
                
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
                if (info->huffTreeCtrl[i].leftOffsetNeed)
                {
                    tmp = info->huffTable[ info->huffTreeCtrl[i].leftNodeNo ].HWord;
                }
                if (info->huffTreeCtrl[i].rightOffsetNeed)
                {
                    if ( info->huffTable[info->huffTreeCtrl[i].rightNodeNo ].HWord > tmp )
                    {
                        tmpRightFlag = 1;
                    }
                }
                if ( (tmp != 0) || (tmpRightFlag) )
                {
                    HuffSetOneNodeOffset( info, (u16)i, tmpRightFlag);
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
static void HuffMakeSubsetHuffTree( HuffCompressionInfo* info, u16 huffTreeNo, BOOL rightNodeFlag )
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
            HuffSetOneNodeOffset( info, i, 0);
            info->huffTreeCtrl[ i ].leftOffsetNeed = 0;
        }
        if ( info->huffTreeCtrl[ i ].rightOffsetNeed )
        {
            HuffSetOneNodeOffset( info, i, 1);
            info->huffTreeCtrl[ i ].rightOffsetNeed = 0;
        }
        i++;
    }
}

//-----------------------------------------------------------------------
// 与えられたデータ量の部分木を展開しても huffTree 構築に支障がないか調べる
//-----------------------------------------------------------------------
static BOOL HuffRemainingNodeCanSetOffset( HuffCompressionInfo* info, u16 costHWord )
{
    u16 i;
    s16 capacity;
    const u32 MAX_COST = 64;
    
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
static void HuffSetOneNodeOffset( HuffCompressionInfo* info, u16 huffTreeNo, BOOL rightNodeFlag)
{
    u16 nodeNo;
    u16 offsetData = 0;
    
    HuffData*         huffTable    = info->huffTable;
    u8*               huffTree     = info->huffTree;
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
        offsetData |= 0x80;
        huffTree[ huffTreeTop * 2 + 0 ] = (u8)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u8)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftOffsetNeed = 0;   // オフセットは必要なくなる
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];  // オフセットは必要
    }
    
    // 右の子節点
    if ( huffTable[ huffTable[ nodeNo ].ChNo[1] ].LeafDepth == 0 )
    {
        offsetData |= 0x40;
        huffTree[ huffTreeTop * 2 + 1 ] = (u8)huffTable[nodeNo].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u8)huffTable[ nodeNo ].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightOffsetNeed = 0;  // オフセットは必要なくなる
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1]; // オフセットは必要
    }
    
    offsetData |= (u16)( huffTreeTop - huffTreeNo - 1 );
    huffTree[ huffTreeNo * 2 + (rightNodeFlag? 1 : 0) ] = (u8)offsetData;
    
    info->huffTreeTop++;
}


/*---------------------------------------------------------------------------*
  Name:         HuffExportTree

  Description:  ハフマンテーブルをバイナリ出力

  Arguments:    dstp    
                info    
                bitSize 

  Returns:      
 *---------------------------------------------------------------------------*/
static u32 HuffExportTree( u8* dstp, HuffCompressionInfo* info )
{
    u32 cnt = 0;
    s32 i;
    
    for ( i = 0; i < (info->huffTreeTop + 1) * 2; i++ )  // ツリーテーブル
    {
        dstp[ cnt++ ] = ((u8*)info->huffTree)[ i ];
    }
    
    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含める (デコーダのアルゴリズムによる)
    while ( cnt & 0x3 )
    {
        if ( cnt & 0x1 )
        {
            info->huffTreeTop++;
            dstp[ 0 ] = dstp[ 0 ] + 1;
        }
        dstp[ cnt++ ] = 0;
    }
    return cnt;
}


/*---------------------------------------------------------------------------*
  Name:         HuffConvertData
  Description:  ハフマンテーブルを元にデータ変換
  Arguments:    *table  
                srcp    
                dstp    
                srcSize 
                bitSize 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u32 HuffConvertData( const HuffData *table, const u8* srcp, u8* dstp, u32 srcSize, u8 bitSize )
{
    u32     i, ii, iii;
    u8      srcTmp;
    u32     bitStream    = 0;
    u32     streamLength = 0;
    u32     dstSize      = 0;
    
    // ハフマン符号化
    for ( i = 0; i < srcSize; i++ )
    {                                  // データ圧縮
        u8 val = srcp[ i ];
        if ( bitSize == 8 )
        {                              // 8ビットハフマン
            bitStream = (bitStream << table[ val ].PaDepth) | table[ val ].HuffCode;
            streamLength += table[ val ].PaDepth;
            for ( ii = 0; ii < streamLength / 8; ii++ )
            {
                dstp[ dstSize++ ] = (u8)(bitStream >> (streamLength - (ii + 1) * 8));
            }
            streamLength %= 8;
        }
        else                           // 4ビットハフマン
        {
            for ( ii = 0; ii < 2; ii++ )
            {
                if ( ii )
                {
                    srcTmp = val >> 4;      // 上位4ビットが後
                }
                else
                {
                    srcTmp = val & 0x0F;    // 下位4ビットが先( デコーダがLittleEndianでアクセスする関係 )
                }
                bitStream = (bitStream << table[ srcTmp ].PaDepth) | table[ srcTmp ].HuffCode;
                streamLength += table[srcTmp].PaDepth;
                for ( iii = 0; iii < streamLength / 8; iii++ )
                {
                    dstp[ dstSize++ ] = (u8)(bitStream >> (streamLength - (iii + 1) * 8));
                }
                streamLength %= 8;
            }
        }
    }
    if ( streamLength != 0 )
    {
        dstp[ dstSize++ ] = (u8)(bitStream << (8 - streamLength));
    }
    
    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含め「る」 
    //   ハフマン符号化だけ特別!　リトルエンディアン変換するため、アラインメント境界データより後にデータが格納される
    while ( dstSize & 0x3 )
    {
        dstp[ dstSize++ ] = 0;
    }
    
    // リトルエンディアン変換
    for ( i = 0; i < dstSize / 4; i++ )
    {
        u8 tmp;
        tmp = dstp[i * 4 + 0];
        dstp[i * 4 + 0] = dstp[i * 4 + 3];
        dstp[i * 4 + 3] = tmp;         // スワップ
        tmp = dstp[i * 4 + 1];
        dstp[i * 4 + 1] = dstp[i * 4 + 2];
        dstp[i * 4 + 2] = tmp;         // スワップ
    }
    return dstSize;
}



/*---------------------------------------------------------------------------*
  Name:         HuffVerifyTable

  Description:  ハフマンテーブルの整合性をチェック

  Arguments:    ハフマンテーブルへのポインタ

  Returns:      正常なテーブルの場合には TRUE
                不正なテーブルの場合には FALSE
 *---------------------------------------------------------------------------*/
static BOOL
HuffVerifyTable( const void* pTable, u8 bit )
{
    enum { FLAGS_ARRAY_NUM = 512 / 8 }; /* 64Byte */
    u8* treep = (u8*)pTable;
    u8* treeStartp = treep + 1;
    u32 treeSize   = *treep;
    u8* treeEndp   = (u8*)pTable + (treeSize + 1) * 2;
    u32 i;
    u8  end_flags[ FLAGS_ARRAY_NUM ];
    u32 idx;
    
    for ( i = 0; i < FLAGS_ARRAY_NUM; i++ )
    {
        end_flags[ i ] = 0;
    }
    
    if ( bit == 4 )
    {
        if ( treeSize >= 0x10 )
        {
            return FALSE;
        }
    }
    
    idx = 1;
    treep = treeStartp;
    while ( treep < treeEndp )
    {
        if ( (end_flags[ idx / 8 ] & (1 << (idx % 8) )) == 0 )
        {
            u32  offset = (u32)( ( (*treep & 0x3F) + 1 ) << 1);
            u8*  nodep  = (u8*)( (((u32)treep >> 1) << 1) + offset );
            
            // 終端のアライメント用データは読み飛ばす
            if ( *treep == 0 && idx >= (treeSize * 2) )
            {
                goto next;
            }
            if ( nodep >= treeEndp )
            {
                return FALSE;
            }
            if ( *treep & 0x80 )
            {
                u32 left = (idx & ~0x1) + offset;
                end_flags[ left / 8 ] |= (u8)( 1 << (left % 8) );
            }
            if ( *treep & 0x40 )
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




//##############################################################################################
//##############################################################################################
// 展開関連の関数は以下
//##############################################################################################
//##############################################################################################

//==================================================================================
// Rawデータ展開
//==================================================================================
static s32 RawRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize )
{
//  memcpy(dstp, srcp, size);
    u32     i;
    
    if ( srcSize < dstSize )
    {
        return -1;
    }
    
    for (i = 0; i < dstSize; i++)
    {
        *dstp = *srcp;
        dstp++;
        srcp++;
    }

    return dstSize;
}

//==================================================================================
// 差分圧縮データ展開
//==================================================================================
static s32 DiffFiltRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, u8 diffBitSize )
{
    s32     DiffCount = 0;             // 展開データのバイト数
    u32     i;

    u16    *src16p = (u16 *)srcp;
    u16    *dst16p = (u16 *)dstp;

    // ソースのバッファオーバーフローチェック
    if ( dstSize > srcSize )
    {
        return -1;
    }

    if (diffBitSize == 8)
    {
#ifdef DEBUG_PRINT_DIFFFILT
        for (i = 0; i < 16; i++)
        {
            dbg_printf_dif(stderr, "srcp[%d] = %x\n", i, srcp[i]);
        }
#endif
        dstp[DiffCount] = srcp[0];     // 先頭データのみ差分無し
        DiffCount++;
        for (i = 1; i < dstSize; i++, DiffCount++)
        {
            dbg_printf_dif(stderr, "dstp[%x] = srcp[%d]+dstp[%d] = %x + %x = %x\n",
                           DiffCount, i, i - 1, srcp[i], dstp[i - 1], srcp[i] - dstp[i - 1]);
            dstp[DiffCount] = srcp[i] + dstp[i - 1];    // 差分データ格納
        }
    }
    else                               // 16ビットサイズ 
    {
        dst16p[DiffCount / 2] = src16p[0];
        DiffCount += 2;
        for (i = 1; i < dstSize / 2; i++, DiffCount += 2)
        {
            dst16p[DiffCount / 2] = src16p[i] + dst16p[i - 1];
        }
    }

    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含めない
    i = 0;
    while ((DiffCount + i) & 0x3)
    {
        dstp[DiffCount + i] = 0;
        i++;
    }

    return DiffCount;
}

//==================================================================================
// ランレングス圧縮データ展開
//==================================================================================
static s32 RLCompRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize )
{
    u32     RLDstCount;                // 展開データのバイト数
    u32     RLSrcCount;                // 展開対象データの処理済データ量(バイト単位)
    u32     i;

    RLSrcCount = 0;
    RLDstCount = 0;
    while ( RLDstCount < dstSize )
    {
        if ( srcp[ RLSrcCount ] & 0x80 )   // 復号処理(ランレングス符号化されている)
        {
            u8 length = (srcp[ RLSrcCount++ ] & 0x7f) + 3; // データ長を格納(３の下駄を履いているので、実際は+3して考える)
            // バッファオーバーランチェック
            if ( RLSrcCount >= srcSize )
            {
                return -1;
            }
            if ( RLDstCount + length > dstSize )
            {
                return -1;
            }
            for ( i = 0; i < length; i++ )
            {
                dstp[ RLDstCount++ ] = srcp[ RLSrcCount ];
            }
            RLSrcCount++;
        }
        else                           // 生データをコピー(ランレングス符号化されていない)
        {
            u8 length = srcp[ RLSrcCount++ ] + 1;        //  (srcp[RLSrcCount] & 0x7f と同じ)
            // バッファオーバーランチェック
            if ( RLSrcCount + length > srcSize )
            {
                return -1;
            }
            if ( RLDstCount + length > dstSize )
            {
                return -1;
            }
            
            for ( i = 0; i < length; i++ )
            {                          //  データ長は -1 されて格納されているため +1
                dstp[ RLDstCount++ ] = srcp[ RLSrcCount++ ];
            }
        }
    }
    
    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含めない
    i = 0;
    while ((RLDstCount + i) & 0x3)
    {
        dstp[RLDstCount + i] = 0;
        i++;
    }

    return RLDstCount;
}

//==================================================================================
// LZ77圧縮データ展開
//==================================================================================
static s32 LZCompReadEx( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, BOOL ex_available)
{
    u32     LZDstCount;                // 展開データのバイト数
    u32     LZSrcCount;                // 展開対象データの処理済データ量(バイト単位)
    u32     i;

    LZSrcCount = 0;
    LZDstCount = 0;
    
    while ( LZDstCount < dstSize )
    {
        u8 compFlags = srcp[LZSrcCount++];  // 圧縮の有無を示すフラグ列
        if ( LZSrcCount > srcSize )
        {
            return -1;
        }
        
        for ( i = 0; i < 8; i++ )
        {
            if (compFlags & 0x80)      // 圧縮されている
            {
                u32 length;                                    // 対象データ長
                u16 offset;                                    // 一致データオフセット - 1 (常に2以上)(下位4ビット，offsetでは11-8ビット目)
                
                length  = srcp[ LZSrcCount ] >> 4;
                
                if ( ex_available )
                {
                    if ( length == 1 )
                    {
                        length =  (srcp[ LZSrcCount ] & 0x0F) << 12;
                        LZSrcCount++;
                        length |= srcp[ LZSrcCount ] << 4;
                        LZSrcCount++;
                        length |= srcp[ LZSrcCount ] >> 4;
                        length += 0xFF + 0xF + 3;
                    }
                    else if ( length == 0 )
                    {
                        length =  (srcp[ LZSrcCount ] & 0x0F) << 4;
                        LZSrcCount++;
                        length |= srcp[ LZSrcCount ] >> 4;
                        length += 0xF + 2;
                    }
                    else
                    {
                        length += 1;
                    }
                }
                else
                {
                    length += 3;
                }
                offset  = (srcp[LZSrcCount] & 0x0F) << 8;  
                LZSrcCount++;
                offset |= srcp[LZSrcCount];
                offset++;
                LZSrcCount++;
                
                // バッファオーバーランをチェック
                if ( LZSrcCount > srcSize )
                {
                    return -1;
                }
                if ( LZDstCount + length > dstSize )
                {
                    return -1;
                }
                if ( LZDstCount < offset )
                {
                    return -1;
                }
                
                // 展開処理
                do
                {
                    dstp[ LZDstCount++ ] = dstp[ LZDstCount - offset ];
                } while ( --length > 0 );
            }
            else                       // 圧縮無し
            {
                dstp[ LZDstCount++ ] = srcp[ LZSrcCount++ ];
                if ( LZSrcCount > srcSize )
                {
                    return -1;
                }
            }
            // サイズに達したら終了
            if ( LZDstCount >= dstSize )
            {
                break;
            }
            compFlags <<= 1;
        }
    }
    
    // 4バイト境界アラインメント
    //   アラインメント用データ0 はデータサイズに含めない
    i = 0;
    while ( (LZDstCount + i) & 0x3 )
    {
        dstp[ LZDstCount + i ] = 0;
        i++;
    }
    return LZDstCount;
}


//==================================================================================
// ハフマン符号化データ展開
//==================================================================================
static s32 HuffCompRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, u8 huffBitSize )
{
    u16     treeSize;                  // huffTreeのサイズ * 2
    u32     HuffSrcCount;              // 展開対象データの処理済データ量(バイト単位)
    u32     HuffDstCount;              // 展開データ
    u32     currentBitStream;
    u8      currentBit;
    u16     treeAddr;
    u8      treeData;
    u8      preTreeData;
    u8      isUpper4bits = 0;

    treeSize     = ((*srcp) + 1) * 2;
    HuffSrcCount = treeSize;           // データの先頭を取得
    HuffDstCount = 0;
    treeAddr     = 1;
    preTreeData  = srcp[ 1 ];
    
    dbg_printf_huff(stderr, "HuffSrcCount = %d\n", HuffSrcCount);
    
    // ハフマンテーブルの整合性チェック
    if ( ! HuffVerifyTable( srcp, huffBitSize ) )
    {
        return -1;
    }
    if ( srcSize < treeSize )
    {
        return -1;
    }
    
    //  展開処理
    while ( HuffDstCount < dstSize )      // return まで
    {
        u16 i;
        
        currentBitStream = srcp[HuffSrcCount++];
        currentBitStream |= srcp[HuffSrcCount++] << 8;
        currentBitStream |= srcp[HuffSrcCount++] << 16;
        currentBitStream |= srcp[HuffSrcCount++] << 24;
        
        if ( HuffSrcCount > srcSize )
        {
            return -1;
        }
        
        for ( i = 0; i < 32; i++ )
        {
            currentBit = (u8)(currentBitStream >> 31);
            currentBitStream <<= 1;
            
            if (((currentBit == 0) && (preTreeData & 0x80)) ||
                ((currentBit == 1) && (preTreeData & 0x40)))
            {
                if (huffBitSize == 8)
                {
                    treeData = srcp[(treeAddr * 2) + currentBit];       // 符号データ
                    dstp[HuffDstCount++] = treeData;
                }
                else if (isUpper4bits)
                {
                    treeData |= (srcp[(treeAddr * 2) + currentBit]) << 4;
                    dstp[HuffDstCount++] = treeData;
                    isUpper4bits = 0;
                }
                else
                {
                    treeData = srcp[(treeAddr * 2) + currentBit];
                    isUpper4bits = 1;
                }
                
                if (HuffDstCount >= dstSize)
                {
                    return HuffDstCount;
                }
                
                treeAddr = 1;
                preTreeData = srcp[ 1 ];
            }
            else
            {
                preTreeData = srcp[(treeAddr * 2) + currentBit];        // オフセット・データ
                treeAddr += (preTreeData & 0x3f) + 1;
            }
        }
    }
    return HuffDstCount;
}


//==================================================================================
// 圧縮ファイルの元ファイルサイズ取得
//==================================================================================
EXTERN u32 STDCALL nitroGetDecompFileSize( const void* srcp )
{
    const u32* p = (const u32*)srcp;
    
    u32 size = *p >> 8;
    if ( size == 0 )
    {
        size = *(p + 1);
    }
    return size;
}


//==================================================================================
// データ展開制御関数       (自動展開のため、最後にrawデータ展開用ヘッダがないと動作しない)
//==================================================================================
EXTERN s32 STDCALL nitroDecompress( const u8 *srcp, u32 srcSize, u8 *dstp, s8 depth )
{
    // rawData      // データ・ヘッダ
    // *(u32 *)pReadBuf = size << 8 | 0;
    //                      [i+3] [i+2] [i+1](サイズ)  |  [0000 0000]
    // DiffFilt
    // *(u32 *)dstp     = size << 8 | 0x80 | diffBitSize/8;
    //                      [i+3] [i+2] [i+1](サイズ)  |  [1000 00XX]
    // RL
    // *(u32 *)dstp     = size << 8 | 0x30;
    //                      [i+3] [i+2] [i+1](サイズ)  |  [0011 0000]
    // LZ77
    // *(u32 *)dstp     = size << 8 | 0x10;
    //                      [i+3] [i+2] [i+1](サイズ)  |  [0001 0000]
    // Huffman
    // *(u32 *)dstp     = size << 8 | 0x20 | huffBitSize;
    //                      [i+3] [i+2] [i+1](サイズ)  |  [0010 XX00]
    u32     header;
    s32     dstSize;
    u32     memSize = srcSize * 3 + 256 * 2;
    u8     *pReadBuf;                  // 圧縮データの先頭番地を指すポインタ
    u8      offset;
    s8      curDepth = 0;
    s8      targetDepth;

    pCompBuf[0] = (u8 *)malloc(memSize);
    pCompBuf[1] = (u8 *)malloc(memSize);
    pReadBuf = pCompBuf[0];

    // malloc チェック
    if (pCompBuf[0] == NULL || pCompBuf[1] == NULL)
    {
        fprintf(stderr, "Error: Memory is not enough.\n");
        exit(1);
    }

    compBufNo = 1;
    memcpy(pReadBuf, srcp, srcSize);

    if (depth < 1)
    {
        targetDepth = -1;
    }
    else
    {
        targetDepth = depth;
    }
    dbg_printf(stderr, "nitroCompress    \t(Compressed   size      is 0x%x)\n", srcSize);

    while (1)
    {
        // targetDepth指定の際の終了条件
        if (curDepth == targetDepth)
        {
            dbg_printf(stderr, "nitroDecompress  Raw \t(Decompressed size will be 0x%x)\n",
                       dstSize);
            dstSize = RawRead(pReadBuf, dstSize, dstp, dstSize);

            if (pCompBuf[0] != NULL)
            {
                free(pCompBuf[0]);
                pCompBuf[0] = NULL;
            }
            if (pCompBuf[1] != NULL)
            {
                free(pCompBuf[1]);
                pCompBuf[1] = NULL;
            }
            return dstSize;
        }

        header  = *(u32 *)pReadBuf;
        dstSize = header >> 8;         // ヘッダを含まないサイズ, 展開関数にもヘッダを含めずに渡す
        offset = 4;
        
        if ( dstSize == 0 )
        {
            dstSize = *(u32 *)(pReadBuf + 4);
            offset  = 8;
        }
        
        if ( memSize < (u32)dstSize )
        {
            memSize = dstSize * 3 + 256 * 2;
            pCompBuf[0] = (u8 *)realloc(pCompBuf[0], memSize);
            pCompBuf[1] = (u8 *)realloc(pCompBuf[1], memSize);
            pReadBuf = pCompBuf[compBufNo ^ 0x1];

        }

        switch (header & CODE_HEADER_MASK)
        {
        case DIFF_CODE_HEADER:
            {
                dbg_printf(stderr, "nitroDecompress  Diff %d \t(Decompressed size will be 0x%x)\n",
                           ((u8)header & 0x03) * 8, dstSize );
                dstSize =
                    DiffFiltRead(&pReadBuf[offset], srcSize - offset, pCompBuf[compBufNo], dstSize,
                                 ((u8)header & 0x03) * 8);
            }
            break;
        case HUFF_CODE_HEADER:
            {
                dbg_printf(stderr, "nitroDecompress  Huff %d \t(Decompressed size will be 0x%x)\n",
                           ((u8)header & 0x0f), dstSize);
                dstSize =
                    HuffCompRead(&pReadBuf[offset], srcSize - offset, pCompBuf[compBufNo], dstSize, (u8)header & 0x0f);
            }
            break;
        case LZ_CODE_HEADER:
            {
                BOOL ex_format = ((header & 0xF) == 0)? FALSE : TRUE;
                
                dbg_printf(stderr, "nitroDecompress  LZ \t(Decompressed size will be 0x%x)\n",
                           dstSize);
                dstSize = LZCompReadEx(&pReadBuf[offset], srcSize - offset, pCompBuf[compBufNo], dstSize, ex_format);
            }
            break;
        case RL_CODE_HEADER:
            {
                dbg_printf(stderr, "nitroDecompress  RL \t(Decompressed size will be 0x%x)\n",
                           dstSize);
                dstSize = RLCompRead(&pReadBuf[offset], srcSize - offset, pCompBuf[compBufNo], dstSize);
            }
            break;
        default:
            {
                dbg_printf(stderr, "nitroDecompress  Raw \t(Decompressed size will be 0x%x)\n",
                           dstSize);

                dstSize = RawRead(&pReadBuf[offset], srcSize - offset, dstp, dstSize);
                if (pCompBuf[0] != NULL)
                {
                    free(pCompBuf[0]);
                    pCompBuf[0] = NULL;
                }
                if (pCompBuf[1] != NULL)
                {
                    free(pCompBuf[1]);
                    pCompBuf[1] = NULL;
                }
                return dstSize;
            }
        }
        
        if ( dstSize < 0 )
        // 不正なファイルで解凍に失敗
        {
            dbg_printf(stderr, "decompress fail\n");
            return -1;
        }
        
        // もう一周
        pReadBuf = pCompBuf[compBufNo];
        compBufNo ^= 0x01;
        srcSize   = dstSize;
        curDepth++;
    }
}

//==================================================================================
// メモリ内容を16進で出力
//==================================================================================
EXTERN void STDCALL debugMemPrint(FILE * fp, u8 *str, u32 size)
{
    u32     i = 0;

    while (str)
    {
        fprintf(fp, "%4lx:\t0x%2x\n", i, *str);
        str++;
        i++;
        if (i >= size)
        {
            break;
        }
    }
}

//==================================================================================
// メモリ内容を2進で出力
//==================================================================================
EXTERN void STDCALL debugMemBitPrint(FILE * fp, u8 *str, u32 size)
{
    u32     i = 0;
    u8      j;

    while (str)
    {
        if (i >= size)
        {
            break;
        }

        fprintf(fp, "%4lx:\t0x%2x\t(binary\t", i, *str);
        for (j = 0; j < 8; j++)
        {
            fprintf(fp, "%d", *str >> (7 - j) & 0x01);
        }
        fprintf(fp, " )\n");
        str++;
        i++;
    }
}

//==================================================================================
// 圧縮前と展開後のデータの比較(正しく展開できていれば、"DATA match"と出力される)
//==================================================================================
EXTERN int STDCALL matchingCheck(u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize)
{
    u32     minSize, i;
    u8      dataMatchFlag = 1;
    u8      sizeMatchFlag;

    sizeMatchFlag = (srcSize == dstSize);
    if (srcSize < dstSize)
    {
        minSize = srcSize;
    }
    else
    {
        minSize = dstSize;
    }

    for (i = 0; i < minSize; i++)
    {
        dbg_printf_match(stderr, "src[%3x], dst[%3x] = %2x , %2x", i, i, srcp[i], dstp[i]);
        if (srcp[i] != dstp[i])
        {
            dataMatchFlag = 0;
            dbg_printf_match(stderr, "\t; mismatch here!");
        }
        dbg_printf_match(stderr, "\n");
    }

    if (sizeMatchFlag)
    {
        fprintf(stderr, "\nSIZE match.\n");
    }
    else
    {
        fprintf(stderr, "\nSIZE mismatch!\n");
    }

    if (dataMatchFlag)
    {
        fprintf(stderr, "DATA match.\n");
    }
    else
    {
        fprintf(stderr, "DATA mismatch!\n");
    }

    if (dataMatchFlag && sizeMatchFlag)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


