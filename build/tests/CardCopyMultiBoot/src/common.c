/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     common.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
*---------------------------------------------------------------------------*/


#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "wh.h"
#include "common.h"
#include "disp.h"
#include "text.h"
#include "bt.h"

#define     DEFAULT_CHAN            1

extern OSMessageQueue mesgQueue;

static void ModeSelect(void);          // �e�@/�q�@ �I�����
static void ModeStartParent(void);     // �g�p�����Ⴂ�`�����l�����v�Z���I�������

// �f�[�^���M���ɌĂяo�����֐�
void    ParentSendCallback(void);
void    ChildSendCallback(void);

static void VBlankIntr(void);

/*
 * ���̃f���S�̂Ŏg�p���鋤�ʋ@�\.
 */
static u16 padPress;
static u16 padTrig;

// �f�[�^��M���ɌĂяo�����֐�
void ParentReceiveCallback(u16 aid, u16 *data, u16 length);
void ChildReceiveCallback(u16 aid, u16 *data, u16 length);

// �u���b�N�]����Ԓʒm�֐�
void BlockTransferCallback(void *arg);

// �\���p����M�o�b�t�@
static u8 gSendBuf[256] ATTRIBUTE_ALIGN(32);
u8 gRecvBuf[1 + WM_NUM_MAX_CHILD][256] ATTRIBUTE_ALIGN(32);
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];

static u32 send_counter[16];
static u32 recv_counter[16];

TEXT_CTRL *tc[NUM_OF_SCREEN];

static BOOL gFirstSendAtChild;

static BOOL wbt_available = FALSE;
static u16 connected_bitmap = 0;

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  �L�[�̓ǂݍ��ݏ���

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void ReadKey(void)
{
    u16     currData = PAD_Read();

    padTrig = (u16)(~padPress & currData);
    padPress = currData;
}

/*---------------------------------------------------------------------------*
  Name:         GetPressKey

  Description:  �����L�[�擾

  Arguments:    None

  Returns:      ��������Ă���L�[�̃r�b�g�}�b�v
 *---------------------------------------------------------------------------*/
u16 GetPressKey(void)
{
    return padPress;
}


/*---------------------------------------------------------------------------*
  Name:         GetTrigKey

  Description:  �L�[�g���K�擾

  Arguments:    None

  Returns:      �L�[�g���K�̃r�b�g�}�b�v
 *---------------------------------------------------------------------------*/
u16 GetTrigKey(void)
{
    return padTrig;
}


/*---------------------------------------------------------------------------*
  Name:         CommonInit

  Description:  ���ʏ������֐�

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CommonInit(void)
{
    /* OS ������ */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    OS_InitThread();
    FX_Init();
    CARD_Init();

    /* GX ������ */
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    /* V�u�����N�����ݒ� */
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)GX_VBlankIntr(TRUE);

    // �L�[������ǂ�
    ReadKey();
}


/*---------------------------------------------------------------------------*
  Name:         InitAllocateSystem

  Description:  ���C����������̃A���[�i�ɂă����������ăV�X�e��������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    // OS_Init�͌Ă΂�Ă���Ƃ����O��
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}




/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  �L�[�g���K�擾

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    DispVBlankFunc();

    //---- ���荞�݃`�F�b�N�t���O
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         MpSendCallback

  Description:  MP�f�[�^���M��ɌĂяo�����֐��B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpSendCallback(BOOL result)
{
#pragma unused( result )
    CARDRomHeader* rh = (void*)CARD_GetRomHeader();
    u32 rh_size = sizeof(CARDRomHeaderNTR);
    u32 limit = rh->rom_size + CARD_ROM_PAGE_SIZE;
    u32 offset = WH_CHILD_SIZE * send_counter[0];

    BgSetMessage(PLTT_YELLOW, " Sending: ROM addr=0x%x", offset);

    if ( offset < limit )
    {
        if ( offset < rh_size )
        {
            u32 rem = rh_size%WH_CHILD_SIZE;
            MI_CpuCopy8( &((u8*)rh)[offset], gSendBuf, WH_CHILD_SIZE );
            if ( offset >= MATH_ROUNDDOWN(rh_size, WH_CHILD_SIZE) )
            {
                MI_CpuFill8( &gSendBuf[rem], 0, WH_CHILD_SIZE - rem );
            }
        }
        else
        if ( offset < CARD_GAME_AREA_OFFSET )
        {
            MI_CpuFill8(gSendBuf, 0, WH_CHILD_SIZE);
        }
        else
        {
            CARD_ReadRom( MI_DMA_NOT_USE, (void*)offset, gSendBuf, WH_CHILD_SIZE );
        }
        gSendBuf[WH_CHILD_SIZE] = FALSE;
        if ( (offset + WH_CHILD_SIZE) >= limit )
        {
            gSendBuf[WH_CHILD_SIZE] = TRUE;
        }
        (void)WH_SendData(gSendBuf, WH_CHILD_MAX_SIZE, NULL);
        send_counter[0]++;
    }
    else
    {
        BgSetMessage(PLTT_RED, " Sent ROM size=0x%x      ", MATH_ROUNDUP(limit, WH_CHILD_SIZE) - CARD_GAME_AREA_OFFSET);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MpReceiveCallback

  Description:  MP�f�[�^��M���ɌĂяo�����֐��B

  Arguments:    aid     - ���M���q�@�� aid( 0 �̏ꍇ�͐e�@����̃f�[�^ )
                data    - ��M�f�[�^�ւ̃|�C���^ (NULL �Őؒf�ʒm)
                length  - ��M�f�[�^�̃T�C�Y

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpReceiveCallback(u16 aid, u16 *data, u16 length)
{
    SDK_MAX_ASSERT(aid, 15);

    // �q�@
    if ( MB_IsMultiBootChild() )
    {
        if (aid == 0)
        {
            if (data && *data == TRUE)
            {
                MpSendCallback( TRUE );
            }
        }

        return;
    }

    // �e�@
    if (data != NULL)
    {
        gRecvFlag[aid] = TRUE;
        // �R�s�[����2�o�C�g�A���C��(4�o�C�g�A���C���łȂ�)
        if (aid == 0)
        {
            // �e�@�����M�����ꍇ
            MI_CpuCopy8(data, &gRecvBuf[aid][0], length);
        }
        else
        {
            static u32 offset = CARD_GAME_AREA_OFFSET;
            offset += WH_CHILD_SIZE;
            MI_CpuCopy8(data, &gRecvBuf[aid][0], length);
            BgSetMessage(PLTT_YELLOW, " Receiving: ROM addr=0x%x", offset);
            if ( gRecvBuf[aid][WH_CHILD_SIZE] == TRUE )
            {
                BgSetMessage(PLTT_RED, " Received ROM size=0x%x   ", offset - CARD_GAME_AREA_OFFSET);
            }
            (void)OS_SendMessage(&mesgQueue, (OSMessage)&gRecvBuf[aid][0], OS_MESSAGE_BLOCK);
        }
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  �e�@/�q�@ �I����ʂł̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    // �J�E���^�N���A
    MI_CpuClear(send_counter, sizeof(send_counter));
    MI_CpuClear(recv_counter, sizeof(recv_counter));

    if (!MB_IsMultiBootChild())
    {
        BgSetMessage(PLTT_YELLOW, " Connect as PARENT");
        //********************************
        (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, DEFAULT_CHAN); // WH_GetMeasureChannel()
        WH_SetReceiver(MpReceiveCallback);
        //********************************
    }
    else
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        BgSetMessage(PLTT_YELLOW, " Connect as CHILD");
        //********************************
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, DEFAULT_CHAN);
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  �e�@ �ʐM��ʂł̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeParent(void)
{
    BgPutString(6, 1, PLTT_RED, "Card Copy Multi Boot");

    BgPrintStr(11, 3, PLTT_YELLOW, "Parent mode");
//    BgPrintStr(4, 3, 0x4, "Send:     %08X", gSendBuf[0]);
    BgPrintStr(4, 5, 0x4, "Receive:");
    {
        s32     i;

        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            if (gRecvFlag[i])
            {
                BgPrintStr(5, (s16)(6 + i), 0x4, "Child%02d: %08X", i, gRecvBuf[i][0]);
            }
            else
            {
                BgPrintStr(5, (s16)(6 + i), 0x7, "No child");
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  �q�@ �ʐM��ʂł̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeChild(void)
{
    BgPutString(6, 1, PLTT_RED, "Card Copy Multi Boot");

    BgPutString(11, 3, PLTT_YELLOW, "Child mode");

    if ( !gFirstSendAtChild )
    {
        BgSetMessage(PLTT_WHITE, " Push A Button to start   ");
    }

    if ( !gFirstSendAtChild ) // (GetTrigKey() & PAD_BUTTON_A) )
    {
        MpSendCallback( TRUE );
        gFirstSendAtChild = TRUE;
    }
}

