/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makegcdfirm
  File:     compress.c

  Copyright 2007 Nintendo.   All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>                 // atoi()
#include    <string.h>                 // strcmp()
#include    <ctype.h>                  // isprint()
#include    <unistd.h>                 // chdir()
#include    <tchar.h>
#include    <limits.h>                 // UCHAR_MAX
#include    <time.h>
#include    <sys/stat.h>               // stat()
#include    "elf.h"
#include    "misc.h"
#include    "defval.h"
#include    "format_rom.h"
#include    "format_nlist.h"
#include    "makegcdfirm.h"

//#define ADD_HEADER

#define DIFF_CODE_HEADER        (0x80)
#define RL_CODE_HEADER          (0x30)
#define LZ_CODE_HEADER          (0x10)
#define HUFF_CODE_HEADER        (0x20)
#define CODE_HEADER_MASK        (0xF0)

//===========================================================================
//  LZ77���k
//===========================================================================
static u8 SearchLZ(const u8 *nextp, u32 remainSize, u16 *offset);

static u16 windowPos;
static u16 windowLen;

static s16 LZOffsetTable[4096];
static s16 LZByteTable[256];
static s16 LZEndTable[256];


static void LZInitTable(void)
{
    u16     i;

    for (i = 0; i < 256; i++)
    {
        LZByteTable[i] = -1;
        LZEndTable[i] = -1;
    }
    windowPos = 0;
    windowLen = 0;
}

static void SlideByte(const u8 *srcp)
{
    s16     offset;
    u8      in_data = *srcp;
    u16     insert_offset;

    if (windowLen == 4096)
    {
        u8      out_data = *(srcp - 4096);
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

    if (windowLen == 4096)
    {
        windowPos = (u16)((windowPos + 1) % 0x1000);
    }
    else
    {
        windowLen++;
    }
}

static void LZSlide(const u8 *srcp, u32 n)
{
    u32     i;

    for (i = 0; i < n; i++)
    {
        SlideByte(srcp++);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_CompressLZ

  Description:  LZ77���k���s�Ȃ��֐�

  Arguments:    srcp            ���k���f�[�^�ւ̃|�C���^
                size            ���k���f�[�^�T�C�Y
                dstp            ���k��f�[�^�ւ̃|�C���^
                                ���k���f�[�^�����傫���T�C�Y�̃o�b�t�@���K�v�ł��B

  Returns:      ���k��̃f�[�^�T�C�Y�B
                ���k��̃f�[�^�����k�O�����傫���Ȃ�ꍇ�ɂ͈��k�𒆒f��0��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
u32 LZCompWrite(u8 *srcp, u32 size, u8 *dstp, int boundary)
{
    u32     LZDstCount;                // ���k�f�[�^�̃o�C�g��
    u8      LZCompFlags;               // ���k�̗L���������t���O�n��
    u8     *LZCompFlagsp;              // LZCompFlags ���i�[���郁�����̈���|�C���g
    u16     lastOffset;                // ��v�f�[�^�܂ł̃I�t�Z�b�g (���̎��_�ł̍Œ���v�f�[�^) 
    u8      lastLength;                // ��v�f�[�^�� (���̎��_�ł̍Œ���v�f�[�^)
    u8      i;
    u32     dstMax;

#ifdef ADD_HEADER
    *(u32 *)dstp = size << 8 | LZ_CODE_HEADER;  // �f�[�^�E�w�b�_
    dstp += 4;
#endif
    LZDstCount = 4;
    dstMax = size;
    LZInitTable();

    while (size > 0)
    {
        LZCompFlags = 0;
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

            if ((lastLength = SearchLZ(srcp, size, &lastOffset)))
            {
                // ���k�\�ȏꍇ�̓t���O�𗧂Ă�
                LZCompFlags |= 0x1;

                // �I�t�Z�b�g�͏��4�r�b�g�Ɖ���8�r�b�g�ɕ����Ċi�[
                *dstp++ = (u8)((lastLength - 3) << 4 | (lastOffset - 1) >> 8);
                *dstp++ = (u8)((lastOffset - 1) & 0xff);
                LZDstCount += 2;
                LZSlide(srcp, lastLength);
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // ���k�Ȃ�
                LZSlide(srcp, 1);
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // 8�񃋁[�v�I��
        *LZCompFlagsp = LZCompFlags;   // �t���O�n����i�[
    }

    // 16�o�C�g���E�A���C�������g
    //   �A���C�������g�p�f�[�^0 �̓f�[�^�T�C�Y�Ɋ܂߂�
    i = 0;
    while (LZDstCount & (boundary - 1))
//    while ((LZDstCount + i) & 0x3)
    {
        *dstp++ = 0;
        LZDstCount++;
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
static u8 SearchLZ(const u8 *nextp, u32 remainSize, u16 *offset)
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     maxOffset;
    u8      maxLength = 2;
    u8      tmpLength;
    s32     w_offset;

    if (remainSize < 3)
    {
        return 0;
    }

    w_offset = LZByteTable[*nextp];

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

        if (nextp - searchp < 2)
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

            // �f�[�^����4�r�b�g�Ŋi�[����邽�߁A18�ȉ� (3�̉��ʂ��͂�����)
            if (tmpLength == (0xF + 3))
            {
                break;
            }
        }
        if (tmpLength > maxLength)
        {
            // �ő咷�I�t�Z�b�g���X�V
            maxLength = tmpLength;
            maxOffset = (u16)(nextp - searchp);
            if (maxLength == (0xF + 3))
            {
                // ��v�����ő�Ȃ̂ŁA�������I������B
                break;
            }
        }
        w_offset = LZOffsetTable[w_offset];
    }

    if (maxLength < 3)
    {
        return 0;
    }
    *offset = maxOffset;
    return maxLength;
}

