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


static void ModeSelect(void);          // �e�@/�q�@ �I�����
static void ModeStartParent(void);     // �g�p�����Ⴂ�`�����l�����v�Z���I�������
static void ModeChild(void);           // �q�@ �ʐM���

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
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];

static int send_counter[16];
static int recv_counter[16];

static BOOL gFirstSendAtChild = TRUE;

TEXT_CTRL *tc[NUM_OF_SCREEN];

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
    FX_Init();

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
  Name:         BlockTransferMain

  Description:  �u���b�N�]����Ԓʒm�֐��B

  Arguments:    arg     - �ʒm�� WM �֐��̃R�[���o�b�N�|�C���^

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferMain(void)
{
        // �ʐM��Ԃɂ�菈����U�蕪��
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_IDLE:
            ModeSelect();
            break;
        case WH_SYSSTATE_MEASURECHANNEL:
            ModeStartParent();
            break;
        case WH_SYSSTATE_ERROR:
        case WH_SYSSTATE_CONNECT_FAIL:
            WH_Reset();
            break;
        case WH_SYSSTATE_FATAL:
            break;
        case WH_SYSSTATE_SCANNING:
        case WH_SYSSTATE_BUSY:
            break;
        case WH_SYSSTATE_CONNECTED:
            // �e�@���q�@���ł���ɕ��򂷂�
            switch (WH_GetConnectMode())
            {
            case WH_CONNECTMODE_MP_PARENT:
//                ModeParent();
                break;
            case WH_CONNECTMODE_MP_CHILD:
                ModeChild();
                break;
            }
            break;
        }
}

/*---------------------------------------------------------------------------*
  Name:         BlockTransferCallback

  Description:  �u���b�N�]����Ԓʒm�֐��B

  Arguments:    arg     - �ʒm�� WM �֐��̃R�[���o�b�N�|�C���^

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferCallback(void *arg)
{
    int connectMode = WH_GetConnectMode();
    
    switch (((WMCallback*)arg)->apiid)
    {
    case WM_APIID_START_MP:
        {                              /* MP �X�e�[�g�J�n */
           WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                if (connectMode == WH_CONNECTMODE_MP_PARENT)
                {
                    ParentSendCallback();
                }
                else if (connectMode == WH_CONNECTMODE_MP_CHILD)
                {
                    WBT_SetOwnAid(WH_GetCurrentAid());
//                    mfprintf(tc[2], "aid = %d\n", WH_GetCurrentAid());
                    bt_start();
                    ChildSendCallback();
                }
                break;
            }
        }
        break;
    case WM_APIID_SET_MP_DATA:
        {                              /* �P���� MP �ʐM���� */
            if (connectMode == WH_CONNECTMODE_MP_PARENT)
            {
                if (connected_bitmap != 0)
                {
                    ParentSendCallback();
                }
            }
            else if (connectMode == WH_CONNECTMODE_MP_CHILD)
            {
                ChildSendCallback();
            }
        }
        break;
    case WM_APIID_START_PARENT:
        {                              /* �V�K�̎q�@�ڑ� */
            WMStartParentCallback *cb = (WMStartParentCallback *)arg;
            if (connectMode == WH_CONNECTMODE_MP_PARENT)
            {
                switch (cb->state)
                {
                case WM_STATECODE_CONNECTED:
                    if (connected_bitmap == 0)
                    {
                        ParentSendCallback();
                    }
                    connected_bitmap |= (1 << cb->aid);
                    break;
                case WM_STATECODE_DISCONNECTED:
                    connected_bitmap &= ~(1 << cb->aid);
                    break;
                }
            }
        }
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ChildSendCallback

  Description:  �q�@�Ƃ��Đe�@����̃f�[�^��M���ɌĂяo�����֐��B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChildSendCallback(void)
{
    const u16 size = (u16)WBT_MpChildSendHook(gSendBuf, WC_CHILD_DATA_SIZE_MAX);
    send_counter[0]++;
    (void)WH_SendData(gSendBuf, size, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         ParentSendCallback

  Description:  �e�@�Ƃ��Ďq�@�ւ̃f�[�^���M���ɌĂяo�����֐��B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentSendCallback(void)
{
    const u16 size = (u16)WBT_MpParentSendHook(gSendBuf, WC_PARENT_DATA_SIZE_MAX);
    send_counter[0]++;
    (void)WH_SendData(gSendBuf, size, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         ParentReceiveCallback

  Description:  �e�@�Ƃ��Ďq�@����̃f�[�^��M���ɌĂяo�����֐��B

  Arguments:    aid     - ���M���q�@�� aid
                data    - ��M�f�[�^�ւ̃|�C���^ (NULL �Őؒf�ʒm)
                length  - ��M�f�[�^�̃T�C�Y

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentReceiveCallback(u16 aid, u16 *data, u16 length)
{
    BgSetMessage(PLTT_YELLOW, " Receive: p=0x%x, len=0x%x", data, length);

	recv_counter[aid]++;
    if (data != NULL)
    {
        gRecvFlag[aid] = TRUE;
        // �R�s�[����2�o�C�g�A���C��(4�o�C�g�A���C���łȂ�)
//        recv_counter[aid]++;
        WBT_MpParentRecvHook((u8 *)data, length, aid);
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ChildReceiveCallback

  Description:  �q�@�Ƃ��Đe�@����̃f�[�^��M���ɌĂяo�����֐��B

  Arguments:    aid     - ���M���e�@�� aid (��� 0)
                data    - ��M�f�[�^�ւ̃|�C���^ (NULL �Őؒf�ʒm)
                length  - ��M�f�[�^�̃T�C�Y

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChildReceiveCallback(u16 aid, u16 *data, u16 length)
{
    BgSetMessage(PLTT_YELLOW, " Receive: p=0x%x, len=0x%x", data, length);

    (void)aid;
    recv_counter[0]++;
    if (data != NULL)
    {
        gRecvFlag[0] = TRUE;
        // �R�s�[����2�o�C�g�A���C��(4�o�C�g�A���C���łȂ�)
        WBT_MpChildRecvHook((u8 *)data, length);
    }
    else
    {
        gRecvFlag[0] = FALSE;
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
    
    gFirstSendAtChild = TRUE;
    
    if (wbt_available)
    {
        bt_stop();
        WBT_End();
        wbt_available = FALSE;
    }

    if (!MB_IsMultiBootChild())
    {
        BgSetMessage(PLTT_YELLOW, " Connect as PARENT");
        //********************************
        WBT_InitParent(BT_PARENT_PACKET_SIZE, BT_CHILD_PACKET_SIZE, bt_callback);
        WH_SetReceiver(ParentReceiveCallback);
        bt_register_blocks();
        (void)WH_StartMeasureChannel();
        wbt_available = TRUE;
        //********************************
    }
    else
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        BgSetMessage(PLTT_YELLOW, " Connect as CHILD");
        //********************************
        WBT_InitChild(bt_callback);
        WH_SetReceiver(ChildReceiveCallback);
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, 0);
        wbt_available = TRUE;
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeStartParent

  Description:  �g�p���̒Ⴂ�`�����l�����v�Z���I�����Ƃ��̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeStartParent(void)
{
    (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, WH_GetMeasureChannel());
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  �q�@ �ʐM��ʂł̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeChild(void)
{
    if (gFirstSendAtChild)
    {
        // 1��ڂ̃f�[�^���M
        ChildSendCallback();
        gFirstSendAtChild = FALSE;
    }

#if 0
    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
    else if (gKey.trg & PAD_BUTTON_Y)
    {
        bt_start();
    }
    else if (gKey.trg & PAD_BUTTON_X)
    {
        bt_stop();
    }
#endif
}

