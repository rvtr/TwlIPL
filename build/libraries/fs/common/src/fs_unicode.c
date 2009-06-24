/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_unicode.c

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
#include <firm.h>

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringSjisToUnicode

  Description:  ShiftJIS�������Unicode������ɕϊ��B
                �����p�X�������炩��ASCII�݂̂ł���ꍇ�Ȃ�
                Unicode��ShiftJIS�̑��ݕϊ����ȗ����ł���ꍇ��
                ���̊֐����I�[�o�[���C�h���邱�Ƃɂ����
                STD���C�u�����̕W�������������N�����̂�h�����Ƃ��ł���B

  Arguments:    dst               �ϊ���o�b�t�@.
                                  NULL ���w�肷��Ɗi�[�����͖��������.
                dst_len           �ϊ���o�b�t�@�̍ő啶�������i�[���ēn��,
                                  ���ۂɊi�[���ꂽ���������󂯎��|�C���^.
                                  NULL ��^�����ꍇ�͖��������.
                src               �ϊ����o�b�t�@.
                src_len           �ϊ����ׂ��ő啶�������i�[���ēn��,
                                  ���ۂɕϊ����ꂽ���������󂯎��|�C���^.
                                  ���̎w�����������I�[�̈ʒu���D�悳���.
                                  ���̒l���i�[���ēn���� NULL ��^�����ꍇ��
                                  �I�[�ʒu�܂ł̕��������w�肵���Ƃ݂Ȃ����.
                callback          �ϊ��ł��Ȃ����������ꂽ���ɌĂ΂��R�[���o�b�N.
                                  NULL���w�肵���ꍇ, �ϊ��ł��Ȃ������̈ʒu��
                                  �ϊ��������I������.

  Returns:      �ϊ������̌���.
 *---------------------------------------------------------------------------*/
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
{
#pragma unused(callback)
        STDResult   result = STD_RESULT_SUCCESS;
        int         i;
        int         max = 0x7FFFFFFF;
        if (src_len && (*src_len >= 0))
        {
            max = *src_len;
        }
        if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
        {
            max = *dst_len;
        }
        for (i = 0; i < max; ++i)
        {
            int     c = ((const u8 *)src)[i];
            if (c == 0)
            {
               break;
            }
            else if (c >= 0x80)
            {
               result = STD_RESULT_ERROR;
               break;
            }
            dst[i] = (u16)c;
        }
        if (src_len)
        {
            *src_len = i;
        }
        if (dst_len)
        {
            *dst_len = i;
        }
        return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringUnicodeToSjis

  Description:  Unicode�������ShiftJIS������ɕϊ��B
                �����p�X�������炩��ASCII�݂̂ł���ꍇ�Ȃ�
                Unicode��ShiftJIS�̑��ݕϊ����ȗ����ł���ꍇ��
                ���̊֐����I�[�o�[���C�h���邱�Ƃɂ����
                STD���C�u�����̕W�������������N�����̂�h�����Ƃ��ł���B

  Arguments:    dst               �ϊ���o�b�t�@.
                                  NULL ���w�肷��Ɗi�[�����͖��������.
                dst_len           �ϊ���o�b�t�@�̍ő啶�������i�[���ēn��,
                                  ���ۂɊi�[���ꂽ���������󂯎��|�C���^.
                                  NULL ��^�����ꍇ�͖��������.
                src               �ϊ����o�b�t�@.
                src_len           �ϊ����ׂ��ő啶�������i�[���ēn��,
                                  ���ۂɕϊ����ꂽ���������󂯎��|�C���^.
                                  ���̎w�����������I�[�̈ʒu���D�悳���.
                                  ���̒l���i�[���ēn���� NULL ��^�����ꍇ��
                                  �I�[�ʒu�܂ł̕��������w�肵���Ƃ݂Ȃ����.
                callback          �ϊ��ł��Ȃ����������ꂽ���ɌĂ΂��R�[���o�b�N.
                                  NULL���w�肵���ꍇ, �ϊ��ł��Ȃ������̈ʒu��
                                  �ϊ��������I������.

  Returns:      �ϊ������̌���.
 *---------------------------------------------------------------------------*/
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
{
#pragma unused(callback)
    STDResult   result = STD_RESULT_SUCCESS;
    int         i;
    int         max = 0x7FFFFFFF;
    if (src_len && (*src_len >= 0))
    {
        max = *src_len;
    }
    if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
    {
        max = *dst_len;
    }
    for (i = 0; i < max; ++i)
    {
        int     c = ((const u16 *)src)[i];
        if (c == 0)
        {
           break;
        }
        else if (c >= 0x80)
        {
           result = STD_RESULT_ERROR;
           break;
        }
        dst[i] = (char)c;
    }
    if (src_len)
    {
        *src_len = i;
    }
    if (dst_len)
    {
        *dst_len = i;
    }
    return result;
}
