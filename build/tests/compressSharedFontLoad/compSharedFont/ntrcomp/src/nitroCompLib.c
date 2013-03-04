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
// �O���[�o���ϐ��錾
//==================================================================================
static u8 *pCompBuf[2];                // ���k�������ɗp����_�u���o�b�t�@
static u8 compBufNo = 1;               // �L���ȃ_�u���o�b�t�@������

//==================================================================================
// �v���g�^�C�v�錾
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
// DLL�p�֐�
//==================================================================================
EXTERN BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}
*/

//----------------------------------------------------------------------------------
//  ���k��̃f�[�^��u�����߂̃������̈���m��
//    ���k�O�̃f�[�^��2�{�̗̈���m�ۂ���
//----------------------------------------------------------------------------------
EXTERN u8 * STDCALL nitroCompMalloc(u32 size)
{
    return (u8 *)malloc(size * 3 + 512);
}

//----------------------------------------------------------------------------------
//  ���k��̃f�[�^��u���Ă����������̈�����
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
//  �f�[�^�̈��k
//------------------------------------------------------------
EXTERN u32 STDCALL nitroCompress( const u8 *srcp, u32 srcSize, u8 *dstp, char *compList, u8 rawHeaderFlag )
{
    char   *pCompList;                // compList�̌��݂̎Q�ƃ|�C���g
    u32     dataSize, nextDataSize;    // ���k�f�[�^�̃T�C�Y(�o�C�g�P��)
    u8     *pReadBuf;                  // ���k�f�[�^�̐擪�Ԓn���w���|�C���^
    u8      bitSize;                   // �����t�B���^�C�n�t�}���������̓K�p�P��
    char    str[16];
    u16     i, j;
    u8      lzSearchOffset;

//  pCompBuf[0] = (u8 *)malloc(srcSize*2 + 4 + 256*2);     // �ň��̃n�t�}���� size*2 + 4 + 256*2 �Ȃ̂ŁA
//  pCompBuf[1] = (u8 *)malloc(srcSize*2 + 4 + 256*2);     // ���̑��̈��k��f�[�^�w�b�_�ǉ��ŁA�s������\������
    pCompBuf[0] = (u8 *)malloc(srcSize * 3 + 256 * 2);
    pCompBuf[1] = (u8 *)malloc(srcSize * 3 + 256 * 2);
    pReadBuf = pCompBuf[0];
    compBufNo = 1;                     // �d�v!!�@��������Ȃ��ƂQ��ڂ�nitroCompress���Ăяo�����Ƃ��ɂ��������Ȃ�

    // malloc �`�F�b�N
    if (pCompBuf[0] == NULL || pCompBuf[1] == NULL)
    {
        fprintf(stderr, "Error: Memory is not enough.\n");
        exit(1);
    }

    dataSize = srcSize;

    // NULL�w�b�_(���k�O�̃f�[�^�p�́A�[���w�b�_) �̒ǉ�����
    if (rawHeaderFlag)
    {
        dataSize += 4;
        *(u32 *)pReadBuf = srcSize << 8 | 0;       // �f�[�^�E�w�b�_
        memcpy(&pReadBuf[4], srcp, srcSize);
    }
    else
    {
        memcpy(pReadBuf, srcp, srcSize);
    }

    pCompList = compList;              // ���k�������i�[�����z����|�C���g

    // ���k�������i�[�����z��ɗv�f���������A���[�v
    while (1)
    {
        switch (*pCompList)
        {
        case 'd':
            {
                pCompList++;           // 'd' �̎��ɂ́A8 �� 16 
                str[0] = *pCompList;
                if (*pCompList == '1')
                {
                    pCompList++;
                    str[1] = *pCompList;
                    str[2] = '\n';
                }
                bitSize = atoi(str);   // �����t�B���^�̓K�p�P�ʂ��i�[
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
                lzSearchOffset = (u8)atoi(str); // �傫�Ȓl�͐؂�̂ĂĊۂ�
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
                pCompList++;           // 'h' �̎��ɂ́A4 �� 8
                str[0] = *pCompList;
                str[1] = '\n';
                bitSize = atoi(str);   // 4 or 8
                str[0] = '\n';
                
                dbg_printf(stderr, "nitroCompress  Huff %d\n", bitSize);
                
                nextDataSize = HuffCompWrite(pReadBuf, dataSize, pCompBuf[compBufNo], bitSize);
            }
            break;
            //-----------------------------------------
            // ���k�I�� (*CompTypeBufp �� NULL)
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
        // �������
        pReadBuf = pCompBuf[compBufNo];
        compBufNo ^= 0x01;
        dataSize = nextDataSize;
        pCompList++;
    }
}


//===========================================================================
//  ���k�f�[�^�̃R�s�[
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
//  �����t�B���^
//===========================================================================
static u32 DiffFiltWrite(u8 *srcp, u32 size, u8 *dstp, u8 diffBitSize)
{
    u32     DiffCount;                 // ���k�f�[�^�̃o�C�g��
    u32     i;

    u16    *src16p = (u16 *)srcp;
    u16    *dst16p = (u16 *)dstp;

    dbg_printf_dif(stderr, "DiffFiltWrite\tsize=%d\tdiffBitSize=%d\n", size, diffBitSize);

    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | (DIFF_CODE_HEADER | diffBitSize / 8);    // �f�[�^�E�w�b�_
        DiffCount = 4;
    }
    else
    {
        *(u32 *)dstp = (DIFF_CODE_HEADER | diffBitSize / 8);    // �f�[�^�E�w�b�_
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
        dstp[DiffCount] = srcp[0];     // �擪�f�[�^�̂ݍ�������
        DiffCount++;
        for (i = 1; i < size; i++, DiffCount++)
        {
            dbg_printf_dif(stderr, "dstp[%x] = srcp[%d]-srcp[%d] = %x - %x = %x\n",
                           DiffCount, i, i - 1, srcp[i], srcp[i - 1], srcp[i] - srcp[i - 1]);

            dstp[DiffCount] = srcp[i] - srcp[i - 1];    // �����f�[�^�i�[
        }
    }
    else                               // 16�r�b�g�T�C�Y 
    {
        dst16p[DiffCount / 2] = src16p[0];
        DiffCount += 2;
        for (i = 1; i < size / 2; i++, DiffCount += 2)
        {
            dst16p[DiffCount / 2] = src16p[i] - src16p[i - 1];
        }
    }

    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂Ȃ�
    i = 0;
    while ((DiffCount + i) & 0x3)
    {
        dstp[DiffCount + i] = 0;
        i++;
    }

    return DiffCount;
}


//===========================================================================
//  ���������O�X������ (�o�C�g�P��)
//===========================================================================
static u32 RLCompWrite(u8 *srcp, u32 size, u8 *dstp)
{
    u32     RLDstCount;                // ���k�f�[�^�̃o�C�g��
    u32     RLSrcCount;                // ���k�Ώۃf�[�^�̏����σf�[�^��(�o�C�g�P��)
    u8      RLCompFlag;                // ���������O�X���������s���ꍇ�P
    u8      runLength;                 // ���������O�X
    u8      rawDataLength;             // �����ɂȂ��Ă��Ȃ��f�[�^�̃����O�X
    u32     i;

    u8     *startp;                    // ���̏������[�v�ɂ�����A���k�Ώۃf�[�^�̐擪���|�C���g

    dbg_printf_rl(stderr, "RLCompWrite\tsize=%d\n", size);
    
    //  �f�[�^�w�b�_        (�T�C�Y�͓W�J��̂���)
    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | RL_CODE_HEADER;  // �f�[�^�E�w�b�_
        RLDstCount = 4;
    }
    else
    // �T�C�Y��24bit�Ɏ��܂�Ȃ��ꍇ�ɂ͊g���`���̃w�b�_�ƂȂ�
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
        startp = &srcp[RLSrcCount];    // ���k�Ώۃf�[�^�̐ݒ�

        for (i = 0; i < 128; i++)      // 7�r�b�g�ŕ\���ł���f�[�^�ʂ� 0~127
        {
            // ���k�Ώۃf�[�^�̖����ɓ��B
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

        // ���������Ȃ��f�[�^���i�[
        // �f�[�^���i�[�o�C�g��8�r�b�g�ڂ�0�Ȃ�A���������Ȃ��f�[�^�n��
        // �f�[�^���� -1 �������ɂȂ�̂ŁA0-127 �� 1-128 �ƂȂ�
        if (rawDataLength)
        {
            dstp[RLDstCount++] = rawDataLength - 1;     // "�f�[�^��-1" �i�[(7�r�b�g)
            for (i = 0; i < rawDataLength; i++)
            {
                dstp[RLDstCount++] = srcp[RLSrcCount++];
            }
            rawDataLength = 0;
        }

        // ���������O�X������
        if (RLCompFlag)
        {
            runLength = 3;
            for (i = 3; i < 128 + 2; i++)
            {
                // ���k�p�f�[�^�̖����ɓ��B
                if (RLSrcCount + runLength >= size)
                {
                    runLength = (u8)(size - RLSrcCount);
                    break;
                }

                // �������r�؂ꂽ�ꍇ
                if (srcp[RLSrcCount] != srcp[RLSrcCount + runLength])
                {
                    break;
                }
                // �����p����
                runLength++;
            }

            // �f�[�^���i�[�o�C�g��8�r�b�g�ڂ�1�Ȃ�A�����������f�[�^�n��
            dstp[RLDstCount++] = 0x80 | (runLength - 3);        // �R�̉��ʂ��͂����āA3~130���i�[
            dstp[RLDstCount++] = srcp[RLSrcCount];
            RLSrcCount += runLength;
            RLCompFlag = 0;
        }
    }

    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂Ȃ�
    i = 0;
    while ((RLDstCount + i) & 0x3)
    {
        dstp[RLDstCount + i] = 0;
        i++;
    }

    return RLDstCount;
}


//===========================================================================
//  LZ77���k
//===========================================================================

#define LZ_OFFSET_BITS     12
#define LZ_OFFSET_SIZE     (1<<LZ_OFFSET_BITS)

typedef struct
{
    u16 WindowPos;              // ���𑋂̐擪�ʒu
    u16 WindowLen;              // ���𑋂̒���
    
    s16 LZOffsetTable[LZ_OFFSET_SIZE];    // ���𑋂̃I�t�Z�b�g�o�b�t�@
    s16 LZByteTable[256];       // �L�����N�^�̍ŐV�����ւ̃|�C���^
    s16 LZEndTable[256];        // �L�����N�^�̍Ō×����ւ̃|�C���^
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
  Description:  ������1�o�C�g�X���C�h
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

  Description:  LZ77���k���s�Ȃ��֐�(�ő�length�̊g��)

  Arguments:    srcp            ���k���f�[�^�ւ̃|�C���^
                size            ���k���f�[�^�T�C�Y
                dstp            ���k��f�[�^�ւ̃|�C���^
                                ���k���f�[�^�����傫���T�C�Y�̃o�b�t�@���K�v�ł��B

  Returns:      ���k��̃f�[�^�T�C�Y�B
                ���k��̃f�[�^�����k�O�����傫���Ȃ�ꍇ�ɂ͈��k�𒆒f��0��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
static u32 LZCompWriteEx(u8 *srcp, u32 size, u8 *dstp, u8 lzSearchOffset, BOOL ex_available )
{
    static LZCompressInfo sLZInfo;
    
    u32     LZDstCount;                // ���k�f�[�^�̃o�C�g��
    u8      LZCompFlags;               // ���k�̗L���������t���O�n��
    u8     *LZCompFlagsp;              // LZCompFlags ���i�[���郁�����̈���|�C���g
    u16     lastOffset;                // ��v�f�[�^�܂ł̃I�t�Z�b�g (���̎��_�ł̍Œ���v�f�[�^) 
    u32     lastLength;                // ��v�f�[�^�� (���̎��_�ł̍Œ���v�f�[�^)
    u8      i;
    const u32 MAX_LENGTH = (ex_available)? (0xFFFF + 0xFF + 0xF + 3) : (0xF + 3);
    
    if ( size < 0x1000000 && size > 0 )
    {
        *(u32 *)dstp = size << 8 | LZ_CODE_HEADER | (ex_available? 1 : 0 );  // �f�[�^�E�w�b�_
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
        LZCompFlagsp = dstp++;         // �t���O�n��̊i�[��
        LZDstCount++;
        
        // �t���O�n��8�r�b�g�f�[�^�Ƃ��Ċi�[����邽�߁A8�񃋁[�v
        for (i = 0; i < 8; i++)
        {
            LZCompFlags <<= 1;         // ���� (i=0) �͓��ɈӖ��͂Ȃ�
            if (size <= 0)
            {
                // �I�[�ɗ����ꍇ�̓t���O���Ō�܂ŃV�t�g�����Ă���I��
                continue;
            }

            if ( (lastLength = SearchLZ(&sLZInfo, srcp, size, &lastOffset, lzSearchOffset, MAX_LENGTH)) != 0)
            {
                u32 length;
                // ���k�\�ȏꍇ�̓t���O�𗧂Ă�
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
                // �I�t�Z�b�g�͏��4�r�b�g�Ɖ���8�r�b�g�ɕ����Ċi�[
                *dstp++ = (u8)( length << 4 | (lastOffset - 1) >> 8 );
                *dstp++ = (u8)( (lastOffset - 1) & 0xFF );
                LZDstCount += 2;
                LZSlide( &sLZInfo, srcp, lastLength);
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // ���k�Ȃ�
                LZSlide(&sLZInfo, srcp, 1);
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


//--------------------------------------------------------
// LZ77���k�ŃX���C�h���̒�����Œ���v����������܂��B
//  Arguments:    startp                 �f�[�^�̊J�n�ʒu�������|�C���^
//                nextp                  �������J�n����f�[�^�̃|�C���^
//                remainSize             �c��f�[�^�T�C�Y
//                offset                 ��v�����I�t�Z�b�g���i�[����̈�ւ̃|�C���^
//  Return   :    ��v�񂪌��������ꍇ��   TRUE
//                ������Ȃ������ꍇ��     FALSE
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
        
        /* �����Ă��ǂ����A�͂��ɍ��������� */
        if (*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset = LZOffsetTable[w_offset];
            continue;
        }
        
        if (nextp - searchp < minOffset)
        {
            // VRAM��2�o�C�g�A�N�Z�X�Ȃ̂� (VRAM����f�[�^��ǂݏo���ꍇ�����邽��)�A
            // �����Ώۃf�[�^��2�o�C�g�O����̃f�[�^�ɂ��Ȃ���΂Ȃ�Ȃ��B
            // 
            // �I�t�Z�b�g��12�r�b�g�Ŋi�[����邽�߁A4096�ȉ�
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
            
            // ��v�����ő�Ȃ̂ŁA�������I������
            if (tmpLength == maxLength)
            {
                break;
            }
        }
        if (tmpLength > currLength)
        {
            // �ő咷�I�t�Z�b�g���X�V
            currLength = tmpLength;
            currOffset = (u16)(nextp - searchp);
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


//===========================================================================
//  �n�t�}��������
//===========================================================================
#define HUFF_END_L  0x80
#define HUFF_END_R  0x40

typedef struct
{
    u16     No;                        // �f�[�^No
    s16     PaNo;                      // �eNo 
    u32     Freq;                      // �o���p�x
    s16     ChNo[2];                   // �qNo (0: �����C 1: �E��)
    u16     PaDepth;                   // �e�m�[�h�̐[��
    u16     LeafDepth;                 // �t�܂ł̐[��
    u32     HuffCode;                  // �n�t�}������
    u8      Bit;                       // �m�[�h�̃r�b�g�f�[�^
    u16     HWord;                     // �e���Ԑߓ_�ɂ����āA���̐ߓ_�����[�g�Ƃ��镔���؂� HuffTree �i�[�ɕK�v�ȃ�������
}
HuffData;

typedef struct
{
    u8      leftOffsetNeed;            // ���̎q�ߓ_�ւ̃I�t�Z�b�g���K�v�Ȃ�1
    u8      rightOffsetNeed;           // �E�̎q�ߓ_�ւ̃I�t�Z�b�g���K�v�Ȃ�1
    u16     leftNodeNo;                // ���̎q�ߓ_No
    u16     rightNodeNo;               // �E�̎q�ߓ_No
}
HuffTreeCtrlData;

// �n�t�}�����[�N�o�b�t�@�\��
typedef struct
{
    HuffData         huffTable[ 512 ];     //  huffTable[ 512 ];      12288B
    u8               huffTree [ 256 * 2 ]; //  huffTree[ 256 * 2 ];     512B
    HuffTreeCtrlData huffTreeCtrl[ 256 ];  //  huffTreeCtrl[ 256 ];    1536B
    u16              huffTreeTop;          //  
    u8               bitSize;              //  
    u8               padding_[1];          //  
}
HuffCompressionInfo;                       // �v 14340B

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
  Description:  �n�t�}�����k
  Arguments:    *srcp   
                size    
                *dstp   
                huffBitSize
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u32 HuffCompWrite(u8 *srcp, u32 size, u8 *dstp, u8 huffBitSize)
{
    u32     huffDstCount;              // ���k�f�[�^�̃o�C�g��
    u32     offset;
    HuffCompressionInfo* info = &sHuffCompressionInfo;
    
    u16 huffDataNum = 1 << huffBitSize;   // 8->256, 4->16
    
    // �e�[�u��������
    HuffInitTable( info, huffBitSize );
    
    // �o���p�x�`�F�b�N
    HuffCountData( info->huffTable, srcp, size, huffBitSize );
    
    // �n�t�}�������e�[�u���쐬
    HuffConstructTree( info, huffBitSize );
    
    // �f�[�^�E�w�b�_
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
    
    // �n�t�}���e�[�u�����o�C�i���o��
    huffDstCount += HuffExportTree( &dstp[ huffDstCount ], info );
    
    // �n�t�}���e�[�u���ɂ��f�[�^�ϊ�
    huffDstCount += HuffConvertData( info->huffTable, srcp, &dstp[ huffDstCount ], size, huffBitSize );
    
    return huffDstCount;
}




/*---------------------------------------------------------------------------*
  Name:         HuffInitTable
  Description:  �n�t�}���e�[�u���̏�����
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
    
    // huffTable��������
    {
        HuffData* table = info->huffTable;
        
        const HuffData  HUFF_TABLE_INIT_DATA  = { 0, 0, 0, {-1, -1}, 0, 0, 0, 0, 0 };
        for ( i = 0; i < tableSize * 2U; i++ )
        {
            table[ i ]    = HUFF_TABLE_INIT_DATA;
            table[ i ].No = (u16)i;
        }
    }
    
    // huffTree, huffTreeCtrl��������
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
  Description:  �o���p�x�̃J�E���g
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
            table[ srcp[ i ] ].Freq++; // 8�r�b�g������
        }
    }
    else
    {
        for ( i = 0; i < size; i++ )   // 4�r�b�g������
        {
            tmp = (srcp[ i ] & 0xf0) >> 4;
            table[ tmp ].Freq++;     // ���4�r�b�g�����Ɋi�[// �ǂ����ł�����
            tmp = srcp[ i ] & 0x0f;
            table[ tmp ].Freq++;     // ���͕������̂Ƃ�
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HuffConstructTree
  Description:  �n�t�}���c���[���\�z
  Arguments:    *table  
                dataNum 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffConstructTree( HuffCompressionInfo *info, u8 bitSize )
{
    HuffData* table = info->huffTable;
    u16     rootNo;                  // �񕪖؂̃��[�gNo
    
    // �o���p�x����m�[�h���\�z
    rootNo = HuffMakeNode( table, bitSize );
    
    // �n�t�}���R�[�h���� (table[i].HuffCode ��)
    HuffAddCodeToTable( table, rootNo, 0x00 );        // PaDepth�̃r�b�g�������AHuffCode �̉��ʃr�b�g���}�X�N�������̂��n�t�}���R�[�h
    
    // �e���Ԑߓ_�ɂ����āA���̐ߓ_�����[�g�Ƃ��镔���؂� huffTree �i�[�ɕK�v�ȃ������ʂ̌v�Z
    HuffAddCountHWordToTable( table, rootNo );
    
    // sHuffTreeBuf.huffTree �쐬
    HuffMakeHuffTree( info, rootNo );
    
    info->huffTree[0] = (u8)( --info->huffTreeTop );
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
// �n�t�}���R�[�h����
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
// ���ԃm�[�h�� huffTree �쐬�ɕK�v�Ƃ���f�[�^��
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
// �n�t�}���R�[�h�\�쐬
//-----------------------------------------------------------------------
static void HuffMakeHuffTree( HuffCompressionInfo* info, u16 rootNo )
{
    s16     i;
    s16     costHWord, tmpCostHWord;            // �����؂̃R�[�h�\���쐬���Ȃ��������̃R�X�g �ő�l�̐ߓ_�̕����؃R�[�h�\�����
    s16     costOffsetNeed, tmpCostOffsetNeed;
    s16     costMaxKey;                         // �R�X�g�ŏ��̐ߓ_�� huffTreeBuf.huffTree ������肷�邽�߂̏��
    BOOL    costMaxRightFlag;
    u16     offsetNeedNum;
    BOOL    tmpRightFlag;
    const u32 MAX_COST = 64;
    
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
// �����؂��܂�܂� huffTree �Ɋi�[
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
// �^����ꂽ�f�[�^�ʂ̕����؂�W�J���Ă� huffTree �\�z�Ɏx�Ⴊ�Ȃ������ׂ�
//-----------------------------------------------------------------------
static BOOL HuffRemainingNodeCanSetOffset( HuffCompressionInfo* info, u16 costHWord )
{
    u16 i;
    s16 capacity;
    const u32 MAX_COST = 64;
    
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
    
    // ���̎q�ߓ_
    if ( huffTable[ huffTable[nodeNo].ChNo[0] ].LeafDepth == 0)
    {
        offsetData |= 0x80;
        huffTree[ huffTreeTop * 2 + 0 ] = (u8)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u8)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftOffsetNeed = 0;   // �I�t�Z�b�g�͕K�v�Ȃ��Ȃ�
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];  // �I�t�Z�b�g�͕K�v
    }
    
    // �E�̎q�ߓ_
    if ( huffTable[ huffTable[ nodeNo ].ChNo[1] ].LeafDepth == 0 )
    {
        offsetData |= 0x40;
        huffTree[ huffTreeTop * 2 + 1 ] = (u8)huffTable[nodeNo].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u8)huffTable[ nodeNo ].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightOffsetNeed = 0;  // �I�t�Z�b�g�͕K�v�Ȃ��Ȃ�
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1]; // �I�t�Z�b�g�͕K�v
    }
    
    offsetData |= (u16)( huffTreeTop - huffTreeNo - 1 );
    huffTree[ huffTreeNo * 2 + (rightNodeFlag? 1 : 0) ] = (u8)offsetData;
    
    info->huffTreeTop++;
}


/*---------------------------------------------------------------------------*
  Name:         HuffExportTree

  Description:  �n�t�}���e�[�u�����o�C�i���o��

  Arguments:    dstp    
                info    
                bitSize 

  Returns:      
 *---------------------------------------------------------------------------*/
static u32 HuffExportTree( u8* dstp, HuffCompressionInfo* info )
{
    u32 cnt = 0;
    s32 i;
    
    for ( i = 0; i < (info->huffTreeTop + 1) * 2; i++ )  // �c���[�e�[�u��
    {
        dstp[ cnt++ ] = ((u8*)info->huffTree)[ i ];
    }
    
    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂� (�f�R�[�_�̃A���S���Y���ɂ��)
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
  Description:  �n�t�}���e�[�u�������Ƀf�[�^�ϊ�
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
    
    // �n�t�}��������
    for ( i = 0; i < srcSize; i++ )
    {                                  // �f�[�^���k
        u8 val = srcp[ i ];
        if ( bitSize == 8 )
        {                              // 8�r�b�g�n�t�}��
            bitStream = (bitStream << table[ val ].PaDepth) | table[ val ].HuffCode;
            streamLength += table[ val ].PaDepth;
            for ( ii = 0; ii < streamLength / 8; ii++ )
            {
                dstp[ dstSize++ ] = (u8)(bitStream >> (streamLength - (ii + 1) * 8));
            }
            streamLength %= 8;
        }
        else                           // 4�r�b�g�n�t�}��
        {
            for ( ii = 0; ii < 2; ii++ )
            {
                if ( ii )
                {
                    srcTmp = val >> 4;      // ���4�r�b�g����
                }
                else
                {
                    srcTmp = val & 0x0F;    // ����4�r�b�g����( �f�R�[�_��LittleEndian�ŃA�N�Z�X����֌W )
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
    
    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߁u��v 
    //   �n�t�}����������������!�@���g���G���f�B�A���ϊ����邽�߁A�A���C�������g���E�f�[�^����Ƀf�[�^���i�[�����
    while ( dstSize & 0x3 )
    {
        dstp[ dstSize++ ] = 0;
    }
    
    // ���g���G���f�B�A���ϊ�
    for ( i = 0; i < dstSize / 4; i++ )
    {
        u8 tmp;
        tmp = dstp[i * 4 + 0];
        dstp[i * 4 + 0] = dstp[i * 4 + 3];
        dstp[i * 4 + 3] = tmp;         // �X���b�v
        tmp = dstp[i * 4 + 1];
        dstp[i * 4 + 1] = dstp[i * 4 + 2];
        dstp[i * 4 + 2] = tmp;         // �X���b�v
    }
    return dstSize;
}



/*---------------------------------------------------------------------------*
  Name:         HuffVerifyTable

  Description:  �n�t�}���e�[�u���̐��������`�F�b�N

  Arguments:    �n�t�}���e�[�u���ւ̃|�C���^

  Returns:      ����ȃe�[�u���̏ꍇ�ɂ� TRUE
                �s���ȃe�[�u���̏ꍇ�ɂ� FALSE
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
            
            // �I�[�̃A���C�����g�p�f�[�^�͓ǂݔ�΂�
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
// �W�J�֘A�̊֐��͈ȉ�
//##############################################################################################
//##############################################################################################

//==================================================================================
// Raw�f�[�^�W�J
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
// �������k�f�[�^�W�J
//==================================================================================
static s32 DiffFiltRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, u8 diffBitSize )
{
    s32     DiffCount = 0;             // �W�J�f�[�^�̃o�C�g��
    u32     i;

    u16    *src16p = (u16 *)srcp;
    u16    *dst16p = (u16 *)dstp;

    // �\�[�X�̃o�b�t�@�I�[�o�[�t���[�`�F�b�N
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
        dstp[DiffCount] = srcp[0];     // �擪�f�[�^�̂ݍ�������
        DiffCount++;
        for (i = 1; i < dstSize; i++, DiffCount++)
        {
            dbg_printf_dif(stderr, "dstp[%x] = srcp[%d]+dstp[%d] = %x + %x = %x\n",
                           DiffCount, i, i - 1, srcp[i], dstp[i - 1], srcp[i] - dstp[i - 1]);
            dstp[DiffCount] = srcp[i] + dstp[i - 1];    // �����f�[�^�i�[
        }
    }
    else                               // 16�r�b�g�T�C�Y 
    {
        dst16p[DiffCount / 2] = src16p[0];
        DiffCount += 2;
        for (i = 1; i < dstSize / 2; i++, DiffCount += 2)
        {
            dst16p[DiffCount / 2] = src16p[i] + dst16p[i - 1];
        }
    }

    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂Ȃ�
    i = 0;
    while ((DiffCount + i) & 0x3)
    {
        dstp[DiffCount + i] = 0;
        i++;
    }

    return DiffCount;
}

//==================================================================================
// ���������O�X���k�f�[�^�W�J
//==================================================================================
static s32 RLCompRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize )
{
    u32     RLDstCount;                // �W�J�f�[�^�̃o�C�g��
    u32     RLSrcCount;                // �W�J�Ώۃf�[�^�̏����σf�[�^��(�o�C�g�P��)
    u32     i;

    RLSrcCount = 0;
    RLDstCount = 0;
    while ( RLDstCount < dstSize )
    {
        if ( srcp[ RLSrcCount ] & 0x80 )   // ��������(���������O�X����������Ă���)
        {
            u8 length = (srcp[ RLSrcCount++ ] & 0x7f) + 3; // �f�[�^�����i�[(�R�̉��ʂ𗚂��Ă���̂ŁA���ۂ�+3���čl����)
            // �o�b�t�@�I�[�o�[�����`�F�b�N
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
        else                           // ���f�[�^���R�s�[(���������O�X����������Ă��Ȃ�)
        {
            u8 length = srcp[ RLSrcCount++ ] + 1;        //  (srcp[RLSrcCount] & 0x7f �Ɠ���)
            // �o�b�t�@�I�[�o�[�����`�F�b�N
            if ( RLSrcCount + length > srcSize )
            {
                return -1;
            }
            if ( RLDstCount + length > dstSize )
            {
                return -1;
            }
            
            for ( i = 0; i < length; i++ )
            {                          //  �f�[�^���� -1 ����Ċi�[����Ă��邽�� +1
                dstp[ RLDstCount++ ] = srcp[ RLSrcCount++ ];
            }
        }
    }
    
    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂Ȃ�
    i = 0;
    while ((RLDstCount + i) & 0x3)
    {
        dstp[RLDstCount + i] = 0;
        i++;
    }

    return RLDstCount;
}

//==================================================================================
// LZ77���k�f�[�^�W�J
//==================================================================================
static s32 LZCompReadEx( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, BOOL ex_available)
{
    u32     LZDstCount;                // �W�J�f�[�^�̃o�C�g��
    u32     LZSrcCount;                // �W�J�Ώۃf�[�^�̏����σf�[�^��(�o�C�g�P��)
    u32     i;

    LZSrcCount = 0;
    LZDstCount = 0;
    
    while ( LZDstCount < dstSize )
    {
        u8 compFlags = srcp[LZSrcCount++];  // ���k�̗L���������t���O��
        if ( LZSrcCount > srcSize )
        {
            return -1;
        }
        
        for ( i = 0; i < 8; i++ )
        {
            if (compFlags & 0x80)      // ���k����Ă���
            {
                u32 length;                                    // �Ώۃf�[�^��
                u16 offset;                                    // ��v�f�[�^�I�t�Z�b�g - 1 (���2�ȏ�)(����4�r�b�g�Coffset�ł�11-8�r�b�g��)
                
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
                
                // �o�b�t�@�I�[�o�[�������`�F�b�N
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
                
                // �W�J����
                do
                {
                    dstp[ LZDstCount++ ] = dstp[ LZDstCount - offset ];
                } while ( --length > 0 );
            }
            else                       // ���k����
            {
                dstp[ LZDstCount++ ] = srcp[ LZSrcCount++ ];
                if ( LZSrcCount > srcSize )
                {
                    return -1;
                }
            }
            // �T�C�Y�ɒB������I��
            if ( LZDstCount >= dstSize )
            {
                break;
            }
            compFlags <<= 1;
        }
    }
    
    // 4�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂Ȃ�
    i = 0;
    while ( (LZDstCount + i) & 0x3 )
    {
        dstp[ LZDstCount + i ] = 0;
        i++;
    }
    return LZDstCount;
}


//==================================================================================
// �n�t�}���������f�[�^�W�J
//==================================================================================
static s32 HuffCompRead( const u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize, u8 huffBitSize )
{
    u16     treeSize;                  // huffTree�̃T�C�Y * 2
    u32     HuffSrcCount;              // �W�J�Ώۃf�[�^�̏����σf�[�^��(�o�C�g�P��)
    u32     HuffDstCount;              // �W�J�f�[�^
    u32     currentBitStream;
    u8      currentBit;
    u16     treeAddr;
    u8      treeData;
    u8      preTreeData;
    u8      isUpper4bits = 0;

    treeSize     = ((*srcp) + 1) * 2;
    HuffSrcCount = treeSize;           // �f�[�^�̐擪���擾
    HuffDstCount = 0;
    treeAddr     = 1;
    preTreeData  = srcp[ 1 ];
    
    dbg_printf_huff(stderr, "HuffSrcCount = %d\n", HuffSrcCount);
    
    // �n�t�}���e�[�u���̐������`�F�b�N
    if ( ! HuffVerifyTable( srcp, huffBitSize ) )
    {
        return -1;
    }
    if ( srcSize < treeSize )
    {
        return -1;
    }
    
    //  �W�J����
    while ( HuffDstCount < dstSize )      // return �܂�
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
                    treeData = srcp[(treeAddr * 2) + currentBit];       // �����f�[�^
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
                preTreeData = srcp[(treeAddr * 2) + currentBit];        // �I�t�Z�b�g�E�f�[�^
                treeAddr += (preTreeData & 0x3f) + 1;
            }
        }
    }
    return HuffDstCount;
}


//==================================================================================
// ���k�t�@�C���̌��t�@�C���T�C�Y�擾
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
// �f�[�^�W�J����֐�       (�����W�J�̂��߁A�Ō��raw�f�[�^�W�J�p�w�b�_���Ȃ��Ɠ��삵�Ȃ�)
//==================================================================================
EXTERN s32 STDCALL nitroDecompress( const u8 *srcp, u32 srcSize, u8 *dstp, s8 depth )
{
    // rawData      // �f�[�^�E�w�b�_
    // *(u32 *)pReadBuf = size << 8 | 0;
    //                      [i+3] [i+2] [i+1](�T�C�Y)  |  [0000 0000]
    // DiffFilt
    // *(u32 *)dstp     = size << 8 | 0x80 | diffBitSize/8;
    //                      [i+3] [i+2] [i+1](�T�C�Y)  |  [1000 00XX]
    // RL
    // *(u32 *)dstp     = size << 8 | 0x30;
    //                      [i+3] [i+2] [i+1](�T�C�Y)  |  [0011 0000]
    // LZ77
    // *(u32 *)dstp     = size << 8 | 0x10;
    //                      [i+3] [i+2] [i+1](�T�C�Y)  |  [0001 0000]
    // Huffman
    // *(u32 *)dstp     = size << 8 | 0x20 | huffBitSize;
    //                      [i+3] [i+2] [i+1](�T�C�Y)  |  [0010 XX00]
    u32     header;
    s32     dstSize;
    u32     memSize = srcSize * 3 + 256 * 2;
    u8     *pReadBuf;                  // ���k�f�[�^�̐擪�Ԓn���w���|�C���^
    u8      offset;
    s8      curDepth = 0;
    s8      targetDepth;

    pCompBuf[0] = (u8 *)malloc(memSize);
    pCompBuf[1] = (u8 *)malloc(memSize);
    pReadBuf = pCompBuf[0];

    // malloc �`�F�b�N
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
        // targetDepth�w��̍ۂ̏I������
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
        dstSize = header >> 8;         // �w�b�_���܂܂Ȃ��T�C�Y, �W�J�֐��ɂ��w�b�_���܂߂��ɓn��
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
        // �s���ȃt�@�C���ŉ𓀂Ɏ��s
        {
            dbg_printf(stderr, "decompress fail\n");
            return -1;
        }
        
        // �������
        pReadBuf = pCompBuf[compBufNo];
        compBufNo ^= 0x01;
        srcSize   = dstSize;
        curDepth++;
    }
}

//==================================================================================
// ���������e��16�i�ŏo��
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
// ���������e��2�i�ŏo��
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
// ���k�O�ƓW�J��̃f�[�^�̔�r(�������W�J�ł��Ă���΁A"DATA match"�Əo�͂����)
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


