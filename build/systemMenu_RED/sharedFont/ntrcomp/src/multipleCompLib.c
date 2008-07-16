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

#define LH_ENC_OFFSET_WIDTH /* ���̐ݒ肪�L���ȏꍇ�ɂ�offset�l�S�̂ł͂Ȃ�offset��bit���𕄍������� */

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
    u16 WindowPos;                      // ���݂̃X���C�h�����̐擪�ʒu
    u16 WindowLen;                      // ���݂̃X���C�h�����̃T�C�Y
    
    s16 LZOffsetTable[ OFFSET_SIZE_MAX ];     // �I�t�Z�b�g�f�[�^�̃e�[�u��
  #ifdef REVERSE_SEARCH
    s16 LZRevOffsetTable[ OFFSET_SIZE_MAX ];  // �I�t�Z�b�g�f�[�^�̋t���e�[�u��
  #endif
    s16 LZByteTable[ 256 ];               // �f�[�^�̐擪�e�[�u��
    s16 LZEndTable [ 256 ];               // �f�[�^�̏I�[�e�[�u��
    u8  OffsetBits;                       // �I�t�Z�b�g��\������ׂ̃r�b�g��
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
  Description:  ������1�o�C�g�X���C�h
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
    
#if defined( REVERSE_SEARCH ) // �T�������t�ɂ���(�V�����f�[�^��D��)
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
// LZ77���k�ŃX���C�h���̒�����Œ���v����������܂��B
//  Arguments:    startp                 �f�[�^�̊J�n�ʒu�������|�C���^
//                nextp                  �������J�n����f�[�^�̃|�C���^
//                remainSize             �c��f�[�^�T�C�Y
//                offset                 ��v�����I�t�Z�b�g���i�[����̈�ւ̃|�C���^
//  Return   :    ��v�񂪌��������ꍇ��   TRUE
//                ������Ȃ������ꍇ��     FALSE
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
        
        /* �����Ă��ǂ����A�͂��ɍ��������� */
        if (*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset = LZOffsetTable[ w_offset ];
            continue;
        }
        
        if (nextp - searchp < minOffset)
        {
            // VRAM��2�o�C�g�A�N�Z�X�Ȃ̂� (VRAM����f�[�^��ǂݏo���ꍇ�����邽��)�A
            // �����Ώۃf�[�^��2�o�C�g�O����̃f�[�^�ɂ��Ȃ���΂Ȃ�Ȃ��B
            // 
            // �I�t�Z�b�g��12�r�b�g�Ŋi�[����邽�߁A4096�ȉ�
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
            
            // �f�[�^����8�r�b�g�Ŋi�[����邽�߁A258�ȉ� (3�̉��ʂ��͂�����)
            if (tmpLength == maxLength)
            {
                break;
            }
        }
        if (tmpLength > currLength)
        {
            // �ő咷�I�t�Z�b�g���X�V
            currLength = tmpLength;
            currOffset = (u16)((u32)nextp - (u32)searchp);
            if (currLength == maxLength)
            {
                // ��v�����ő�Ȃ̂ŁA�������I������B
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
    u32     LZDstCount = 0;            // ���k�f�[�^�̃o�C�g��
    u8      LZCompFlags;               // ���k�̗L���������t���O�n��
    u8     *LZCompFlagsp;              // LZCompFlags ���i�[���郁�����̈���|�C���g
    u16     lastOffset;                // ��v�f�[�^�܂ł̃I�t�Z�b�g (���̎��_�ł̍Œ���v�f�[�^) 
    u16     lastLength;                // ��v�f�[�^�� (���̎��_�ł̍Œ���v�f�[�^)
    u8      i;
    const u32 MAX_LENGTH = 0xFF + 3;

    LZInitTable( &gLZWork );
    gLZWork.OffsetBits = offsetBits;
    
    while ( size > 0 )
    {
        LZCompFlags = 0;
        LZCompFlagsp = dstp++;         // �t���O�n��̊i�[��
        LZDstCount++;

        // �t���O�n��8�r�b�g�f�[�^�Ƃ��Ċi�[����邽�߁A8�񃋁[�v
        for ( i = 0; i < 8; i++ )
        {
            LZCompFlags <<= 1;         // ���� (i=0) �͓��ɈӖ��͂Ȃ�
            if (size <= 0)
            {
                // �I�[�ɗ����ꍇ�̓t���O���Ō�܂ŃV�t�g�����Ă���I��
                continue;
            }

            if ( (lastLength = SearchLZ(&gLZWork, srcp, size, &lastOffset, lzSearchOffset, MAX_LENGTH)) != 0 )
            {
                // ���k�\�ȏꍇ�̓t���O�𗧂Ă�
                LZCompFlags |= 0x1;

                // �I�t�Z�b�g�͏��4�r�b�g�Ɖ���8�r�b�g�ɕ����Ċi�[
                *dstp++ = (u8)(lastLength - 3);
                *dstp++ = (u8)((lastOffset - 1) & 0xff); // ���g���G���f�B�A��
                *dstp++ = (u8)((lastOffset - 1) >> 8);
                LZDstCount += 3;
                LZSlide( &gLZWork, srcp, lastLength );
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // ���k�Ȃ�
                LZSlide( &gLZWork, srcp, 1 );
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // 8�񃋁[�v�I��
        *LZCompFlagsp = LZCompFlags;   // �t���O�n����i�[
    }

    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂Ȃ�
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
    u16     No;                        // �f�[�^No
    s16     PaNo;                      // �eNo 
    u32     Freq;                      // �o���p�x
    s16     ChNo[2];                   // �qNo (0: �����C 1: �E��)
    u16     PaDepth;                   // �e�m�[�h�̐[��
    u16     LeafDepth;                 // �t�܂ł̐[��
    u32     HuffCode;                  // �n�t�}������
    u16     Bit;                       // �m�[�h�̃r�b�g�f�[�^
    u16     HWord;                     // �e���Ԑߓ_�ɂ����āA���̐ߓ_�����[�g�Ƃ��镔���؂� HuffTree �i�[�ɕK�v�ȃ�������
}
HuffData;

typedef struct
{
    u8       leftOffsetNeed;            // ���̎q�ߓ_�ւ̃I�t�Z�b�g���K�v�Ȃ�1
    u8       rightOffsetNeed;           // �E�̎q�ߓ_�ւ̃I�t�Z�b�g���K�v�Ȃ�1
    u16      leftNodeNo;                // ���̎q�ߓ_No
    u16      rightNodeNo;               // �E�̎q�ߓ_No
}
HuffTreeCtrlData;

// �n�t�}�����[�N�o�b�t�@�\��
typedef struct
{
    HuffData*         huffTable;            //  huffTable[ 512 ];      12288B
    u16*              huffTree;             //  huffTree[ 256 * 2 ];     512B
    HuffTreeCtrlData* huffTreeCtrl;         //  huffTreeCtrl[ 256 ];    1536B
    u16               huffTreeTop;          //
    u8                bitSize;              //  
    u8                padding_[1];          //  
}
HuffInfo;                       // �v 14340B

static void HuffMakeHuffTree             ( HuffInfo* info, u16 rootNo );
static void HuffMakeSubsetHuffTree       ( HuffInfo* info, u16 huffTreeNo, BOOL rightNodeFlag );
static BOOL HuffRemainingNodeCanSetOffset( HuffInfo* info, u16 costHWord );
static void HuffSetOneNodeOffset         ( HuffInfo* info, u16 huffTreeNo, BOOL rightNodeFlag );
static u16  HuffMakeNode                 ( HuffData* table, u8 bitSize );

static void HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo );
static void HuffAddCodeToTable       ( HuffData* table, u16 nodeNo, u32 paHuffCode );
static u16  HuffAddCountHWordToTable ( HuffData *table, u16 nodeNo );


// �r�b�g�X�g���[��
typedef struct
{
    u8*     dstp;       // �o�͐�|�C���^
    u32     cnt;        // �o�̓T�C�Y
    u32     stream;     // �J�����g�X�g���[���f�[�^
    u32     stream_len; // �X�g���[���̒���
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
    
    // huffTable��������
    {
        HuffData* table = info->huffTable;
        const HuffData HUFF_TABLE_INIT_DATA = { 0, 0, 0, {-1, -1}, 0, 0, 0, 0, 0 };
        for ( i = 0; i < tableSize * 2; i++ )
        {
            table[ i ]    = HUFF_TABLE_INIT_DATA;
            table[ i ].No = (u16)i;
        }
    }
    
    // huffTree, huffTreeCtrl��������
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
        u8 compFlags = srcp[ srcCnt++ ]; // ���k�̗L���������t���O��
        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // ���k����Ă���Alength:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // ���g���G���f�B�A��
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
    
    // �o���p�x����m�[�h���\�z
    rootNo = HuffMakeNode( table, bitSize );
    
    // �n�t�}���R�[�h���� (table[i].HuffCode ��)
    HuffAddCodeToTable( table, rootNo, 0x00 );        // PaDepth�̃r�b�g�������AHuffCode �̉��ʃr�b�g���}�X�N�������̂��n�t�}���R�[�h
    
    // �e���Ԑߓ_�ɂ����āA���̐ߓ_�����[�g�Ƃ��镔���؂� huffTree �i�[�ɕK�v�ȃ������ʂ̌v�Z
    HuffAddCountHWordToTable( table, rootNo );
    
    HuffMakeHuffTree( info, rootNo );
	info->huffTreeTop--;
}

//-----------------------------------------------------------------------
// �n�t�}���R�[�h�\�쐬
//-----------------------------------------------------------------------
static void
HuffMakeHuffTree( HuffInfo* info, u16 rootNo )
{
    s16     i;
    s16     costHWord, tmpCostHWord;            // �����؂̃R�[�h�\���쐬���Ȃ��������̃R�X�g �ő�l�̐ߓ_�̕����؃R�[�h�\�����
    s16     costOffsetNeed, tmpCostOffsetNeed;
    s16     costMaxKey;                         // �R�X�g�ŏ��̐ߓ_�� huffTreeBuf.huffTree ������肷�邽�߂̏��
    BOOL    costMaxRightFlag;
    u16     offsetNeedNum;
    BOOL    tmpRightFlag;
    const u32 MAX_COST = 1 << (info->bitSize - 2);
    
    info->huffTreeTop = 1;
    costOffsetNeed    = 0;
    
    info->huffTreeCtrl[0].leftOffsetNeed = 0; // �g�p���Ȃ� (�e�[�u���T�C�Y�Ƃ��Ďg�p)
    info->huffTreeCtrl[0].rightNodeNo    = rootNo;
    
    while ( 1 )                          // return ����܂� 
    {
        // �I�t�Z�b�g��ݒ肷��K�v�̂���m�[�h���̌v�Z
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
        
        // �ő�R�X�g�̐ߓ_������
        costHWord    = -1;
        costMaxKey   = -1;
        tmpRightFlag =  0;
        
        for ( i = 0; i < info->huffTreeTop; i++ )
        {
            tmpCostOffsetNeed = (u16)( info->huffTreeTop - i );
            
            // ���̎q�ߓ_�̃R�X�g�]��
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
        
        // �����؂��܂�܂� huffTree �Ɋi�[
        if ( costMaxKey >= 0 )
        {
            HuffMakeSubsetHuffTree( info, (u16)costMaxKey, costMaxRightFlag);
            goto nextTreeMaking;
        }
        else
        {
            // �K�v�I�t�Z�b�g�ő�̃m�[�h������
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
// �����؂��܂�܂� huffTree �Ɋi�[
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
// �^����ꂽ�f�[�^�ʂ̕����؂�W�J���Ă� huffTree �\�z�Ɏx�Ⴊ�Ȃ������ׂ�
//-----------------------------------------------------------------------
static BOOL 
HuffRemainingNodeCanSetOffset( HuffInfo* info, u16 costHWord )
{
    u16 i;
    s16 capacity;
    const u32 MAX_COST = 1 << (info->bitSize - 2);
    
    capacity = (s16)( MAX_COST - costHWord );
    
    // �I�t�Z�b�g���� i ���������قǑ傫���̂ŁA�\�[�g�����Ai = 0 -> huffTreeTop �Ōv�Z����΂悢
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
  Description:  1�ߓ_���A�n�t�}���R�[�h�\���쐬
  Arguments:    *table          �n�t�}���e�[�u��
                huffTreeNo      
                rightNodeFlag   �E���̃m�[�h�ł��邩�ǂ����̃t���O
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
    
    // ���̎q�ߓ_
    if ( huffTable[ huffTable[nodeNo].ChNo[0] ].LeafDepth == 0)
    {
        offsetData |= 0x8000;
        huffTree[ huffTreeTop * 2 + 0 ] = (u16)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftOffsetNeed = 0;   // �I�t�Z�b�g�͕K�v�Ȃ��Ȃ�
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];  // �I�t�Z�b�g�͕K�v
    }
    
    // �E�̎q�ߓ_
    if ( huffTable[ huffTable[ nodeNo ].ChNo[1] ].LeafDepth == 0 )
    {
        offsetData |= 0x4000;
        huffTree[ huffTreeTop * 2 + 1 ] = (u16)huffTable[nodeNo].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightOffsetNeed = 0;  // �I�t�Z�b�g�͕K�v�Ȃ��Ȃ�
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1]; // �I�t�Z�b�g�͕K�v
    }
    
    offsetData |= (u16)( huffTreeTop - huffTreeNo - 1 );
    huffTree[ huffTreeNo * 2 + (rightNodeFlag? 1 : 0) ] = offsetData;
    
    info->huffTreeTop++;
}

/*---------------------------------------------------------------------------*
  Name:         HuffMakeNode
  Description:  �o���p�x����m�[�h�f�[�^���\�z
  Arguments:    table   
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u16
HuffMakeNode( HuffData* table, u8 bitSize )
{
    u16       dataNum  = ( 1 << bitSize );
    u16       tableTop = (u16)dataNum; // �e�[�u���쐬���́A�e�[�u���g�b�vNo
    
    u32       i;
    s32       leftNo, rightNo;         // 2���؍쐬���̃m�[�hNo
    u16       rootNo;                  // �񕪖؂̃��[�gNo
    
    leftNo  = -1;
    rightNo = -1;
    while ( 1 )
    {
        // Freq�̏����������ؒ��_��2�T��  1�͕K��������͂�
        // �q���_(��)�̒T��
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
        
        // �q���_(�E)�̒T��
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
        
        // 1�����Ȃ�������A�e�[�u���쐬�I��
        if ( rightNo < 0 )
        {
            // �l�����ނ����Ȃ����݂��Ȃ��ꍇ�ɂ�01�ǂ���������l�ƂȂ�m�[�h���P�쐬����
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
        
        // �������؂ƉE�����؂𓝍����钸�_�쐬
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
// 2���؍쐬���ɁA�����؂𓝍������Ƃ��ɁA�����؂̊e�\���m�[�h�̐[�����{1����
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
// �n�t�}���R�[�h����
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
// ���ԃm�[�h�� huffTree �쐬�ɕK�v�Ƃ���f�[�^��
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
    
    // �e�[�u���T�C�Y��1/4���T�C�Y�̈�֕ۑ�
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
        u8 compFlags = srcp[ srcCnt++ ]; // ���k�̗L���������t���O��
    #if BLEND_COMP_FLAG
    #else
        BitStream_Write( &stream, compFlags, 8 );
    #endif

        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // ���k����Ă���Alength:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // ���g���G���f�B�A��
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
                    // offset��0�ł��邱�Ƃ͂Ȃ��̂ŁA�ŏ�ʂ̃r�b�g�͏ȗ�����
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
    // �܂���srcp�𕁒ʂ�LZ���k
    tmpSize = LZCompWrite_( srcp, srcSize, tmpBuf, 2, LH_OFFSET_BITS );
    
    // offset��length�̏W�v
    LZMakeHuffTree( tmpBuf, tmpSize, &sTree8, &sTree16 );
    
    dstSize = 0;
    
    // �w�b�_�̏�������
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
    // �n�t�}���e�[�u�����o��
    dstSize += ExportHuffTree( &dstp[ dstSize ], &sTree8,  LENGTH_BITS );
    dstSize += ExportHuffTree( &dstp[ dstSize ], &sTree16, LH_OFFSET_TABLE_BITS );
    
    // ���k���ʂ��n�t�}�����������Ȃ���o��
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

  Description:  �n�t�}���e�[�u���̐��������`�F�b�N

  Arguments:    pTable  �n�t�}���e�[�u���ւ̃|�C���^
                bit     �n�t�}�������̃r�b�g��

  Returns:      ����ȃe�[�u���̏ꍇ�ɂ� TRUE
                �s���ȃe�[�u���̏ꍇ�ɂ� FALSE
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
            
            // �I�[�̃A���C�����g�p�f�[�^�͓ǂݔ�΂�
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
    
    // �w�b�_�̓ǂݍ���
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
    
    // �n�t�}���e�[�u����ǂ�
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
        // �񈳏k�f�[�^
        {
            dstp[dstCnt++] = (u8)val;
        }
        else
        // ���k�f�[�^
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
            
            // �o�b�t�@�I�[�o�[�������`�F�b�N
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
// LRC���k/�W�J
// 
//==============================================================================

#define LRC_ADAPTIVE            // �K���^�����W�R�[�_���g�p���邩�ǂ���
#define RC_MAX_RANGE    0x80000000
#define RC_UNIT_BITS    8       // 1�o�C�g�P�ʂŏo��
// #define LRC_ENC_OFFSET_WIDTH // NOTE: ���̃I�v�V�����ł͐���ɓ���ł��Ȃ��B
                                //   ���m�ɂ́A�W�J���ɐ���bit�f�[�^�������W�R�[�h�Ƃ��Đ��
                                //   �ǂ܂�Ă��܂��̂ŁA������ɂ͎��̃f�[�^�����o���Ȃ��B

#if defined( LRC_ADAPTIVE )
  #define TABLE8_ADAPTIVE       TRUE
#else
  #define TABLE8_ADAPTIVE       FALSE
#endif

#if defined( LRC_ENC_OFFSET_WIDTH )
  #define TABLE16_ADAPTIVE        FALSE // bitLen���g�p����ꍇ�ɂ́A���E�e�[�u�����������̂�ofs�͐ÓIRC��OK
  #define LRC_OFFSET_BITS         15
  #define LRC_OFFSET_TABLE_BITS   5
#else
  #define TABLE16_ADAPTIVE        TRUE  // bitLen���g�p���Ȃ��ꍇ�ɂ́A���E�e�[�u�����傫���Ȃ�̂�ofs�͓��IRC���g�p
  #define LRC_OFFSET_BITS         12
  #define LRC_OFFSET_TABLE_BITS   LRC_OFFSET_BITS
#endif

// �����W�R�[�_�p�\����
typedef struct
{
    u32 *freq;          // �o���p�x�e�[�u��  (1 << bitSize) * sizeof(u32) Byte
    u32 *low_cnt;       // LOW���E�l�e�[�u�� (1 << bitSize) * sizeof(u32) Byte
    u32 total;          // �g�[�^��          4 Byte
    u8  bitSize;        // �r�b�g�T�C�Y      1 Byte
    u8  padding_[1];    //
}
RCCompressionInfo;

// �����W�R�[�_��ԍ\����
typedef struct
{
    u32     low;
    u32     range;
    u32     code;       // �W�J���̂ݎg�p
    u8      carry;      // ���k���̂ݎg�p
    u32     carry_cnt;  // ���k���̂ݎg�p
}
RCState;

/*---------------------------------------------------------------------------*
  Name:         RCInitState_

  Description:  RC��Ԃ̏������������Ȃ��܂��B

  Arguments:    state   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCInitState_( RCState* state )
{
    // �J�nRange��0x80000000�Ȃ̂ŁA���񂢂��Ȃ茅�グ���������邱�Ƃ͂Ȃ�
    state->low   = 0;
    state->range = RC_MAX_RANGE;
    state->code  = 0;
    state->carry = 0;
    state->carry_cnt = 0;
}


/*---------------------------------------------------------------------------*
  Name:         RCInitInfo_

  Description:  �ÓI�����W�R�[�_�̃e�[�u��������
                ���ׂĂ̏o���p�x��0�ŏ��������܂��B

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

  Description:  �ÓIRC�e�[�u�����o�͂��܂��B

  Arguments:    dstp
                info

  Returns:      �o�̓f�[�^�T�C�Y
 *---------------------------------------------------------------------------*/
static u32
RCExportTable_( u8* dstp, RCCompressionInfo* info )
{
    u32 tableSize = (1 << info->bitSize);
    u32 cnt = 0;
    u32 i;
    
    // �p�x�e�[�u���̏o��(16bit���g���G���f�B�A��)
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
    
    // �p�x�e�[�u���̃C���|�[�g(16bit���g���G���f�B�A��)
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

  Description:  RC�e�[�u���̐��K��

  Arguments:    info    

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RCNormalizeTable_( RCCompressionInfo* info )
{
    u32 tableSize = (1 << info->bitSize);
    u32 i;
    
    // �g�[�^���̃J�E���g
    info->total = 0;
    for ( i = 0; i < tableSize; i++ )
    {
        info->total += info->freq[ i ];
    }
    
    // ���K��
    // 0x10000�֐��K��
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
        // �ł��o���p�x�̍����l���덷�����ɗ��p
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
    
    // low_cnt�̌v�Z
    info->low_cnt[ 0 ] = 0;
    for ( i = 1; i < tableSize; i++ )
    {
        info->low_cnt[ i ] = info->low_cnt[ i - 1 ] + info->freq[ i - 1 ];
    }
}


/*---------------------------------------------------------------------------*
  Name:         RCCountData_

  Description:  LZ���k�����f�[�^����ÓI�����W�R�[�_�p�̃e�[�u�����쐬���܂��B

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
        u8 compFlags = srcp[ srcCnt++ ]; // ���k�̗L���������t���O��
        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // ���k����Ă���Alength:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // ���g���G���f�B�A��
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

  Description:  �K���^�����W�R�[�_�̃e�[�u��������
                ���ׂĂ̏o���p�x��1�ŏ��������܂��B

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

  Description:  �K���^�����W�R�[�_�̕p�x�e�[�u�����X�V���܂��B

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
    
    // �g�[�^�����ő�l���z�����ꍇ�ɂ́A�č\������B
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
        // �o���p�x�e�[�u�����X�V
        RCAAddCount_( info, data );
    }
    
    // ���オ�肪��������ꍇ�̏���
    if ( prevLow > state->low )
    {
        // �L�����[���P����グ
        state->carry++;
        // �L�����[��(�L�����[�J�E���^ - 1)��0x00���o�͂��܂��B
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
    
    // Range�̏��1�o�C�g����ɂȂ����猅�グ
    while ( state->range < MIN_RANGE )
    {
        u8 candidate = (u8)( state->low >> 24 );
        // ���̃L�����[��0xFF�̏ꍇ�͍X�Ɍ��グ�����蓾��̂�carry���o�͂�����carry_cnt�������₷
        if ( candidate == 0xFF )
        {
            state->carry_cnt++;
        }
        else
        // ���̃L�����[��0xFF�ł͂Ȃ��ꍇ��carry���o�͂���
        {
            // carry��(carry_cnt - 1)����0xFF���o�͂���
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
            // �V����carry�ɒu������
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
    // carry��(carry_cnt - 1)����0xFF���o�͂���
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
    // low�Ɏc�����f�[�^�������o��
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
        u8 compFlags = srcp[ srcCnt++ ]; // ���k�̗L���������t���O��
        
        for ( i = 0; i < 8; i++ )
        {
            if ( compFlags & 0x80 ) // ���k����Ă���Alength:8, offset:16
            {
                u8  length = srcp[ srcCnt++ ];
                u16 offset = srcp[ srcCnt++ ]; // ���g���G���f�B�A��
                offset |= srcp[ srcCnt++ ] << 8;
                
                // length | 0x100�������W�R�[�_�Ɋ|����
                ConvertRC( info8,  length | 0x100, &stream, &rcState, TABLE8_ADAPTIVE );
            
            #if !defined( LRC_ENC_OFFSET_WIDTH ) // �e�[�u���T�C�Y���傫���̂ŋ����I�ɓK���^
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
                    // offset��0�ł��邱�Ƃ͂Ȃ��̂ŁA�ŏ�ʂ̃r�b�g�͏ȗ�����
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
    // carry��low�Ɏc�����f�[�^��f���o��
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
    // �܂���srcp�𕁒ʂ�LZ���k
    tmpSize = LZCompWrite_( srcp, srcSize, tmpBuf, 2, LRC_OFFSET_BITS );
    
    // �e�[�u��������
    RCInitInfo_( &sRCInfo8,  LENGTH_BITS );
    RCInitInfo_( &sRCInfo16, LRC_OFFSET_TABLE_BITS );
    
    dstSize = 0;
    
    // �w�b�_�̏�������
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
    
    // �ÓI�����W�R�[�_�̕p�x�\���쐬�������Ă���(�g�p���邩�ǂ����̓I�v�V��������)
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
    
    // ���k���ʂ������W�R�[�_���������Ȃ���o��
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
    // TODO: �Ƃ肠�������`�T���A�񕪒T���ɂ���ׂ�
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
    // �񕪒T��
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
    
    // �o���p�x�e�[�u�����X�V
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
    
    // �w�b�_�̓ǂݍ���
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
    
    // RC�p�x�e�[�u����ǂ�
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
        // �Œ�ł�����̃R�[�h����4Byte�͕K�v
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
        // �񈳏k�f�[�^
        {
            dstp[ dstCnt++ ] = (u8)val;
        }
        else
        // ���k�f�[�^
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
            
            // �o�b�t�@�I�[�o�[�������`�F�b�N
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


