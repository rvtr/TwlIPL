/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     sound.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-25#$
  $Rev: 1800 $
  $Author: ida $
 *---------------------------------------------------------------------------*/

#include "sound.h"

static BOOL ReadWaveFormat(StreamInfo * strm);
static void ReadStrmData(StreamInfo * strm);
static void SoundAlarmHandler(void *arg);
static void VBlankIntr(void);

static u16 Cont;
static u16 Trg;
static OSMessageQueue msgQ;
static OSMessage msgBuf[1];

static u8 strm_lbuf[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 strm_rbuf[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 strm_tmp[STRM_BUF_PAGESIZE * 2] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
  Name:         PlayStream

  Description:  �X�g���[���Đ����Đ�

  Arguments:    strm - �X�g���[���I�u�W�F�N�g
                filename - �X�g���[���Đ�����t�@�C����

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PlayStream(StreamInfo * strm, const char *filename)
{
    int     timerValue;
    u32     alarmPeriod;

    // �Đ����ł���Β�~����
    if (strm->isPlay)
    {
        u32     tag;
        StopStream(strm);
        tag = SND_GetCurrentCommandTag();
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK | SND_COMMAND_IMMEDIATE);
        SND_WaitForCommandProc(tag);   // ��~��҂�
    }

    // �t�@�C������
    if (FS_IsFile(&strm->file))
        (void)FS_CloseFile(&strm->file);
    if ( ! FS_OpenFile(&strm->file, filename) ) {
        OS_Panic("Error: failed to open file %s\n", filename);
    }
    if (!ReadWaveFormat(strm))
    {
        OS_Panic("Error: failed to read wavefile\n");
    }

    strm->isPlay = TRUE;

    /* �p�����[�^�ݒ� */
    timerValue = SND_TIMER_CLOCK / strm->format.sampleRate;
    alarmPeriod = timerValue * STRM_BUF_PAGESIZE / 32U;
    alarmPeriod /= (strm->format.bitPerSample == 16) ? sizeof(s16) : sizeof(s8);

    // �����X�g���[���f�[�^�ǂݍ���
    (void)FS_SeekFile(&strm->file, (s32)strm->beginPos, FS_SEEK_SET);
    strm->bufPage = 0;
    ReadStrmData(strm);
    ReadStrmData(strm);

    // �`�����l���ƃA���[�����Z�b�g�A�b�v
    SND_SetupChannelPcm(L_CHANNEL,
                        (strm->format.bitPerSample ==
                         16) ? SND_WAVE_FORMAT_PCM16 : SND_WAVE_FORMAT_PCM8, strm_lbuf,
                        SND_CHANNEL_LOOP_REPEAT, 0, STRM_BUF_SIZE / sizeof(u32), 127,
                        SND_CHANNEL_DATASHIFT_NONE, timerValue, 0);
    SND_SetupChannelPcm(R_CHANNEL,
                        (strm->format.bitPerSample ==
                         16) ? SND_WAVE_FORMAT_PCM16 : SND_WAVE_FORMAT_PCM8,
                        (strm->format.channels == 1) ? strm_lbuf : strm_rbuf,
                        SND_CHANNEL_LOOP_REPEAT, 0, STRM_BUF_SIZE / sizeof(u32), 127,
                        SND_CHANNEL_DATASHIFT_NONE, timerValue, 127);
    SND_SetupAlarm(ALARM_NUM, alarmPeriod, alarmPeriod, SoundAlarmHandler, strm);
    SND_StartTimer((1 << L_CHANNEL) | (1 << R_CHANNEL), 0, 1 << ALARM_NUM, 0);
}

/*---------------------------------------------------------------------------*
  Name:         StopStream

  Description:  �X�g���[���Đ����~

  Arguments:    strm - �X�g���[���I�u�W�F�N�g

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StopStream(StreamInfo * strm)
{
    SND_StopTimer((1 << L_CHANNEL) | (1 << R_CHANNEL), 0, 1 << ALARM_NUM, 0);
    if (FS_IsFile(&strm->file))
        (void)FS_CloseFile(&strm->file);
    strm->isPlay = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         StrmThread

  Description:  �X�g���[���X���b�h

  Arguments:    arg - ���[�U�[�f�[�^�i���g�p�j

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StrmThread(void * /*arg */ )
{
    OSMessage message;

    OS_InitMessageQueue(&msgQ, msgBuf, 1);

    while (1)
    {
        (void)OS_ReceiveMessage(&msgQ, &message, OS_MESSAGE_BLOCK);
        (void)ReadStrmData((StreamInfo *) message);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SoundAlarmHandler

  Description:  �A���[���R�[���o�b�N�֐�

  Arguments:    arg - �X�g���[���I�u�W�F�N�g

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SoundAlarmHandler(void *arg)
{
    (void)OS_SendMessage(&msgQ, (OSMessage)arg, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         ReadStrmData

  Description:  �X�g���[���f�[�^�ǂݍ��݊֐�
                �t�@�C������o�b�t�@�̂P�y�[�W���̃X�g���[���f�[�^��ǂݍ���

  Arguments:    strm - �X�g���[���I�u�W�F�N�g

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadStrmData(StreamInfo * strm)
{
    int     i;
    s32     readSize;
    u8     *lbuf, *rbuf;

    // �X�g���[�����I�[�ɒB���Ă���
    if (strm->dataSize <= 0)
    {
        StopStream(strm);
        return;
    }

    // �o�b�t�@�̃y�[�W�ݒ�
    if (strm->bufPage == 0)
    {
        lbuf = strm_lbuf;
        rbuf = strm_rbuf;
        strm->bufPage = 1;
    }
    else
    {
        lbuf = strm_lbuf + STRM_BUF_PAGESIZE;
        rbuf = strm_rbuf + STRM_BUF_PAGESIZE;
        strm->bufPage = 0;
    }

    // �f�[�^�ǂݍ���
    if (strm->format.channels == 1)
    {
        // ���m����
        readSize = FS_ReadFile(&strm->file,
                               strm_tmp,
                               (strm->dataSize <
                                STRM_BUF_PAGESIZE) ? strm->dataSize : STRM_BUF_PAGESIZE);
        if (readSize == -1)
            OS_Panic("read file end\n");

        if (strm->format.bitPerSample == 16)
        {
            // 16bit�f�[�^
            for (i = 0; i < readSize / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = ((s16 *)strm_tmp)[i];
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = 0;  // �X�g���[���̏I�[�ɒB�����ꍇ�A�c����O�Ŗ��߂�
            }
        }
        else
        {
            // 8bit�f�[�^
            for (i = 0; i < readSize / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = (s8)((s16)strm_tmp[i] - 128);
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = 0;
            }
        }
    }
    else
    {
        // �X�e���I
        readSize = FS_ReadFile(&strm->file,
                               strm_tmp,
                               (strm->dataSize <
                                STRM_BUF_PAGESIZE * 2) ? strm->dataSize : STRM_BUF_PAGESIZE * 2);
        if (readSize == -1)
            OS_Panic("read file end\n");

        if (strm->format.bitPerSample == 16)
        {
            // 16bit�f�[�^
            for (i = 0; i < (readSize / 2) / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = ((s16 *)strm_tmp)[2 * i];
                ((s16 *)rbuf)[i] = ((s16 *)strm_tmp)[2 * i + 1];
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = 0;
                ((s16 *)rbuf)[i] = 0;
            }
        }
        else
        {
            // 8bit�f�[�^
            for (i = 0; i < (readSize / 2) / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = (s8)((s16)strm_tmp[2 * i] - 128);
                ((s8 *)rbuf)[i] = (s8)((s16)strm_tmp[2 * i + 1] - 128);
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = 0;
                ((s8 *)rbuf)[i] = 0;
            }
        }
    }

    strm->dataSize -= readSize;

    return;
}


/*---------------------------------------------------------------------------*
  Name:         ReadWaveFormat

  Description:  WAVE�t�H�[�}�b�g�̃f�[�^�̃w�b�_�ƃf�[�^��̐擪�ʒu�A�f�[�^�T�C�Y���擾

  Arguments:    strm - �X�g���[���I�u�W�F�N�g

  Returns:      �ǂݎ��ɐ���������TRUE�A���s������FALSE
 *---------------------------------------------------------------------------*/
static BOOL ReadWaveFormat(StreamInfo * strm)
{
    u32     tag;
    s32     size;
    BOOL    fFmt = FALSE, fData = FALSE;

    (void)FS_SeekFileToBegin(&strm->file);

    (void)FS_ReadFile(&strm->file, &tag, 4);
    if (tag != FOURCC_RIFF)
        return FALSE;

    (void)FS_ReadFile(&strm->file, &size, 4);

    (void)FS_ReadFile(&strm->file, &tag, 4);
    if (tag != FOURCC_WAVE)
        return FALSE;

    while (size > 0)
    {
        s32     chunkSize;
        if (FS_ReadFile(&strm->file, &tag, 4) == -1)
        {
            return FALSE;
        }
        if (FS_ReadFile(&strm->file, &chunkSize, 4) == -1)
        {
            return FALSE;
        }

        switch (tag)
        {
        case FOURCC_fmt:
            if (FS_ReadFile(&strm->file, (u8 *)&strm->format, chunkSize) == -1)
            {
                return FALSE;
            }
            fFmt = TRUE;
            break;
        case FOURCC_data:
            strm->beginPos = FS_GetPosition(&strm->file);
            strm->dataSize = chunkSize;
            (void)FS_SeekFile(&strm->file, chunkSize, FS_SEEK_CUR);
            fData = TRUE;
            break;
        default:
            (void)FS_SeekFile(&strm->file, chunkSize, FS_SEEK_CUR);
            break;
        }
        if (fFmt && fData)
        {
            return TRUE;               // fmt �� data ��ǂݏI�����狭���I��
        }

        size -= chunkSize;
    }

    if (size != 0)
        return FALSE;
    return TRUE;
}

//--------------------------------------------------------------------------------
//    �u�u�����N���荞�ݏ���
//
static void VBlankIntr(void)
{
    // ���荞�݃`�F�b�N�t���O
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
