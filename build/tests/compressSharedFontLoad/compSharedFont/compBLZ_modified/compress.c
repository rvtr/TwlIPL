/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compstatic
  File:     compress.c

  Copyright 2003 Nintendo.  All rights reserved.

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
#include "file.h"
#include "compress.h"

static int LZCompressRV(u8 *src_buffer, int src_size, u8 *dst_buffer, int dst_size);
static int FindMatched(u8 *src_buffer, int src_size, u8 *dic_buffer, int dic_size, int *index);
static int HowManyMatched(u8 *src_buffer, u8 *dic_buffer, int max_len);
static int CheckOverwrite(int orig_size, u8 *cmprs_buffer, int cmprs_buffer_size,
                          int *orig_safe, int *cmprs_safe);


/*---------------------------------------------------------------------------*
  Name:         Compress

  Description:  Buffer �̋t���̈��k���s�Ȃ��D
                ���������k�f�[�^�ƓW�J�f�[�^����������Ԃ����L�ł���悤��
                ��������

                buffer       : �툳�k�f�[�^
                buffer_size  : �툳�k�f�[�^�T�C�Y

  Return:       >=0: ���k��̃T�C�Y
                < 0: ���s
 *---------------------------------------------------------------------------*/
int Compress(u8 *buffer_original, int buffer_original_size)
{
    u8     *buffer;
    int     buffer_size;
    int     buffer_start;
    u8     *temp_buffer_original;
    u8     *temp_buffer;
    int     temp_buffer_size;
    int     temp_buffer_start;
    int     compressed_size;
    int     aligned_size;
    int     total_size;
    int     reduced;
    int     i;
    CompFooter *footer;

    // �O����
    if (NULL == (temp_buffer_original = (u8 *)malloc(buffer_original_size)))
    {
        ErrorPrintf("Cannot allocate memory size=%d\n", buffer_original_size);
        return COMPRESS_FATAL_ERROR;
    }

    if ((u32)buffer_original % 4 != 0)
    {
        ErrorPrintf("Top of buffer is not aligned by 4.\n");
        return COMPRESS_FATAL_ERROR;
    }

    buffer = buffer_original;
    buffer_size = buffer_original_size;
    temp_buffer = temp_buffer_original;
    temp_buffer_size = buffer_original_size;

    // ���k���s�Ȃ�
    reduced = LZCompressRV(buffer, buffer_size, temp_buffer, temp_buffer_size);
    if (reduced < 0)
    {
        DebugPrintf("Compressed buffer size exceeds original data size.\n");
        free(temp_buffer_original);
        return COMPRESS_LARGER_ORIGINAL;
    }

    temp_buffer_size -= reduced;
    temp_buffer += reduced;

    DebugPrintf("1: source size = %d  compressed = %d\n", buffer_size, temp_buffer_size);

    // �W�J�s�\�ȏ㏑�����������邩�m�F
    if (!CheckOverwrite
        (buffer_size, temp_buffer, temp_buffer_size, &buffer_start, &temp_buffer_start))
    {
        // �㏑������������Ȃ爳�k�͈͂�ύX����
        buffer += buffer_start;
        buffer_size -= buffer_start;
        temp_buffer += temp_buffer_start;
        temp_buffer_size -= temp_buffer_start;

        DebugPrintf("  !! Shrink back Compressed region to avoid overwriting.\n"
                    "  !! Expand non-compressed region = +%d\n"
                    "2: source size = %d  compressed = %d\n",
                    buffer_start, buffer_size, temp_buffer_size);
    }

    //  PADDING �ƃp�����[�^�̈�������Ă������Ȃ����ǂ�������
    compressed_size = buffer_start + temp_buffer_size;  // header+body
    aligned_size = ROUNDUP4(compressed_size);   //       +padding
    total_size = aligned_size + sizeof(CompFooter);     //       +footer

    if (buffer_original_size <= total_size)
    {
        DebugPrintf("Compressed buffer size exceeds or equals original data size.\n");
        free(temp_buffer_original);
        return COMPRESS_LARGER_ORIGINAL;
    }

    // �f�[�^���e���|�����o�b�t�@���猳�f�[�^�֏㏑������
    CopyBuffer(temp_buffer, buffer, temp_buffer_size);
    free(temp_buffer_original);

    // �T�C�Y�� 4 �̔{���ɂȂ�悤�� PADDING
    //   LZ �̎����㈳�k�̈�̍ŏ��̃o�C�g�l�� 0xff �ɂȂ�Ȃ�(�ŏ��͈��k
    //   �t���O�ł���A�ŏ��̃f�[�^�͈��k�Ȃ��Ŋi�[����邩��)�̂� 0xff ��
    //   ���߂�
    for (i = compressed_size; i < aligned_size; i++)
    {
        buffer_original[i] = 0xff;
    }

    // �T�C�Y�ݒ�
    //    compressBottom �� sizeof(PAD)+sizeof(footer) �Ȃ̂� 1�o�C�g�ŏ\��
    footer = (CompFooter *) (buffer_original + aligned_size);
    footer->bufferTop = total_size - buffer_start;      // ���̒l
    footer->compressBottom = total_size - compressed_size;      // ���̒l
    footer->originalBottom = buffer_original_size - total_size; // ���̒l

    return total_size;
}


/*---------------------------------------------------------------------------*
  Name:         LZCompressRV

  Description:  LZ ���k���s�Ȃ��D�������f�[�^�̌�����爳�k�J�n����
                ���k���ʂ����l�߂ɂȂ�

  Returns:      ���k�f�[�^�̐擪 index
                ���k�f�[�^�� dst_buffer+index ���� dst_buffer+dst_size-1 �܂�
                -1: ���k���s(���k�������ʂ̕����傫���ꍇ)
 *---------------------------------------------------------------------------*/
static int LZCompressRV(u8 *src_buffer, int src_size, u8 *dst_buffer, int dst_size)
{
    int     src_index = src_size;
    int     dst_index = dst_size;
    int     compflag;
    int     compflag_index;
    int     i;

    while (src_index > 0)
    {
        if (dst_index < 1)
            return -1;                 // Buffer Overflow

        // 8bit �̈��k�t���O�̑}���ʒu��\��
        compflag = 0x00;
        compflag_index = --dst_index;

        // �t���O�n��8�r�b�g�f�[�^�Ƃ��Ċi�[����邽�߁A8�񃋁[�v
        for (i = 0; i < 8; i++)
        {
            compflag <<= 1;

            if (src_index > 0)         // src ���c���Ă��邩����
            {
                u8     *dic_buffer;
                int     dic_size;
                u8     *ref_buffer;
                int     ref_size;
                int     index;
                int     len;

                dic_buffer = src_buffer + src_index;
                dic_size = src_size - src_index;
                ref_size = MIN(src_index, LZ_MAX_COPY);
                ref_buffer = dic_buffer - ref_size;

                len = FindMatched(ref_buffer, ref_size,
                                  dic_buffer, MIN(dic_size, LZ_MAX_DIC_LENGTH), &index);

                if (len >= LZ_MIN_COPY)
                {
                    u16     half;

                    // Offset/Len �̋L�^���\���ǂ����m�F
                    if (dst_index < 2)
                        return -1;     // Buffer Overflow

                    // src index �i�߂�
                    src_index -= len;

                    // len >= LZ_MIN_COPY �Ȃ̂ł��̕����Z���l���ߖ񂷂�
                    index -= (LZ_MIN_COPY - 1);
                    len -= (LZ_MIN_COPY - 0);

                    // 16bit �f�[�^�Ƃ��Ă�����
                    half = (u16)((index & (LZ_MAX_INDEX - 1)) | (len << LZ_BIT_INDEX));
                    dst_buffer[--dst_index] = (half >> 8) & 0xff;
                    dst_buffer[--dst_index] = (half >> 0) & 0xff;

                    // flag �Z�b�g
                    compflag |= 0x01;
                }
                else
                {
                    // �l���̂܂܂��L�^���� & src index �i�߂�
                    if (dst_index < 1)
                        return -1;     // Buffer Overflow
                    dst_buffer[--dst_index] = src_buffer[--src_index];
                }
            }
        }
        // ���k�t���O�̕ۑ�
        dst_buffer[compflag_index] = compflag;
    }
    return dst_index;
}


/*---------------------------------------------------------------------------*
  Name:         FindMatched

  Description:  ��v����p�^�[���̌������s�Ȃ��D�������f�[�^�̌������O���ւ�
                ����

                src_buffer[0...src_size-1] �̃p�^�[���� src_buffer �̌������
                dic_buffer[0...dic_size-1] �̃p�^�[���ƍő��v���镔����
                ��������D

  Returns:      ��v�����T�C�Y
 *index ��v�����ʒu
 *---------------------------------------------------------------------------*/
static int FindMatched(u8 *src_buffer, int src_size, u8 *dic_buffer, int dic_size, int *index)
{
    u8     *src_bottom = src_buffer + src_size - 1;
    u8      char_src_bottom = *src_bottom;
    int     n, len, max_len;

    // �Ԓl������
    max_len = 0;

    for (n = 0; n < dic_size; n++)
    {
        // �������̂��߂̃L���b�V��
        if (char_src_bottom == dic_buffer[n])
        {
            len = HowManyMatched(src_bottom, dic_buffer + n, MIN(n + 1, src_size));
            if (max_len < len)
            {
                max_len = len;
                *index = n;
            }
        }
    }

    // �ŏ��T�C�Y�ȏ�Ȃ琬��
    return max_len;
}


/*---------------------------------------------------------------------------*
  Name:         HowManyMatched

  Description:  2�̃p�^�[�����t���ɂǂ��܂ň�v���Ă��邩�𒲍�����

                src_buffer, dic_buffer ��r�p�^�[���̃A�h���X
                                       (���̃A�h���X����t�����֌�������)
                max_len                �ő咲�����钷��

  Returns:      ��v��������
 *---------------------------------------------------------------------------*/
static int HowManyMatched(u8 *src_buffer, u8 *dic_buffer, int max_len)
{
    int     i;

    // �p�^�[����v����(�t��)
    for (i = 0; i < max_len; i++)
    {
        if (*src_buffer != *dic_buffer)
        {
            break;
        }
        src_buffer--;
        dic_buffer--;
    }
    return i;
}


/*---------------------------------------------------------------------------*
  Name:         CheckOverwrite

  Description:  LZ �W�J�œW�J��ƓW�J���𓯂��A�h���X�ɒu�����ꍇ�ɁA�ǂ��܂�
                ����ɓW�J���\�����`�F�b�N����

  Returns:      �Ō�܂œW�J�\�Ȃ� TRUE �r���܂łȂ� FALSE
 *---------------------------------------------------------------------------*/
static int CheckOverwrite(int orig_size, u8 *cmprs_buffer, int cmprs_buffer_size,
                          int *orig_safe, int *cmprs_safe)
{
    int     src = cmprs_buffer_size;
    int     dst = orig_size;
    int     flag;
    int     i;

//#define DETAIL
    while (dst > 0)
    {
        flag = cmprs_buffer[--src];    // ���k�񈳏k�t���O 8 ���[�v��

#ifdef	DETAIL
        DebugPrintf("%08x %08x FLG=0x%02x\n", src, dst, flag);
#endif
        for (i = 0; i < 8; i++)
        {
            if (dst > 0)
            {
                if (flag & 0x80)       // ���k�f�[�^���H
                {
                    u16     half;
                    int     len;

                    // �W�J�����v�Z
                    src -= 2;
                    half = (u16)(cmprs_buffer[src] | (cmprs_buffer[src + 1] << 8));
                    len = ((half >> LZ_BIT_INDEX) & (LZ_MAX_LENGTH - 1)) + LZ_MIN_COPY;
#ifdef	DETAIL
                    DebugPrintf("%08x %08x-%08x LEN=%d\n", src, dst - 1, dst - len, len);
#endif
                    // �\�[�X�f�[�^���㏑�����Ă��܂����`�F�b�N
                    dst -= len;

                    if (dst < 0)
                    {
                        ErrorPrintf("System error in CheckOverwrite???\n");
                        exit(-1);      // Panic!!
                    }

                    if (dst < src)
                    {
                        // �㏑�����Ă��܂��Ȃ爳�k�͌��݂̂Ƃ���܂łŎ~�߂�
                        *orig_safe = dst;
                        *cmprs_safe = src;
                        return FALSE;
                    }
                }
                else
                {
                    // �񈳏k�f�[�^�Ȃ炻�̂܂܃R�s�[�Ȃ̂�
                    // �j��𔺂Ȃ��㏑���͋N����Ȃ�
                    src--;
                    dst--;
#ifdef	DETAIL
                    DebugPrintf("%08x %08x CHR=0x%02x\n", src, dst, cmprs_buffer[src]);
#endif
                }
                flag <<= 1;
            }
        }
    }
    *orig_safe = 0;
    *cmprs_safe = 0;
    return TRUE;
}
