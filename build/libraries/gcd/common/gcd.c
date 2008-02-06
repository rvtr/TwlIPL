/*---------------------------------------------------------------------------*
  Project:  TwlBrom - GCD - libraries
  File:     gcd.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <firm/gcd.h>
#include <firm/mi.h>

// use this functions for DS mode only

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#define DBG_PRINTF vlink_dos_printf
#define DBG_CHAR vlink_dos_put_console
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif


static BOOL GCDi_ReadRomPreCore( GCDSlot slot, u32 romp, void *ramp, s32 size );
static void GCDi_ReadGameModeRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls );
static void GCDi_WriteGameModeRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls );

static void GCD_DmaRecvRomAsync( GCDSlot slot, u32 dmaNo, void *dest );
static void GCD_DmaSendRomAsync( GCDSlot slot, u32 dmaNo, void *src );


u32 GCDi_HeaderBuf[2][ GCD_ROM_HEADER_SIZE/sizeof(u32) ];
u32 GCDi_SecureAreaBuf[2][ GCD_SECURE_AREA_SIZE/sizeof(u32) ];
u32 GCDi_Secure2AreaBuf[2][ GCD_SECURE_AREA_SIZE/sizeof(u32) ];

GCDSharedWork GCDi_SharedWork[2];
GCDSecureWork GCDi_SecureWork[2];


/*---------------------------------------------------------------------------*
  Name:         GCDi_Enable

  Description:  enable game card master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Enable( GCDSlot slot )
{
    REGType16v* cnt0 = GCDi_SelectRegAddr( slot, REG_MCCNT0_ADDR );

    OSIntrMode enabled = OS_DisableInterrupts();

    GCDi_WaitCtrl( slot );

    *cnt0 |= REG_MI_MCCNT0_A_E_MASK | REG_MI_MCCNT0_A_I_MASK;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_Disable

  Description:  disable game card master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Disable( GCDSlot slot )
{
    REGType16v* cnt0 = GCDi_SelectRegAddr( slot, REG_MCCNT0_ADDR );

    OSIntrMode enabled = OS_DisableInterrupts();

    GCDi_WaitCtrl( slot );

    *cnt0 &= ~(REG_MI_MCCNT0_A_E_MASK | REG_MI_MCCNT0_A_I_MASK);

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_SelectRom

  Description:  select game card rom

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SelectRom( GCDSlot slot )
{
    REGType16v* cnt0 = GCDi_SelectRegAddr( slot, REG_MCCNT0_ADDR );

    OSIntrMode enabled = OS_DisableInterrupts();

    *cnt0 &= ~REG_MI_MCCNT0_A_SEL_MASK;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_SelectSpi

  Description:  select game card spi

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SelectSpi( GCDSlot slot )
{
    REGType16v* cnt0 = GCDi_SelectRegAddr( slot, REG_MCCNT0_ADDR );

    OSIntrMode enabled = OS_DisableInterrupts();

    *cnt0 |= REG_MI_MCCNT0_A_SEL_MASK;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_GenCtrl

  Description:  generate game card control

  Arguments:    op : command

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_GenCtrl( GCDCtrlRegs* regs,
                        GCDRw rw,
                        GCDPageCount pcount,
                        GCDClockType ckt, u32 lt1, u32 lt2,
                        BOOL cpn, BOOL dpn, BOOL csc, BOOL dsc )
{
    regs->latency = ((lt1 << REG_MI_MCCNT1_A_L1_SHIFT) & REG_MI_MCCNT1_A_L1_MASK)
                  | ((lt2 << REG_MI_MCCNT1_A_L2_SHIFT) & REG_MI_MCCNT1_A_L2_MASK);

    regs->ctrl = REG_MI_MCCNT1_A_START_MASK | REG_MI_MCCNT1_A_RESB_MASK
               | rw
               | pcount
               | ckt
               | ((lt1 << REG_MI_MCCNT1_A_L1_SHIFT) & REG_MI_MCCNT1_A_L1_MASK)
               | ((lt2 << REG_MI_MCCNT1_A_L2_SHIFT) & REG_MI_MCCNT1_A_L2_MASK)
               | ((cpn || dpn) ? (u32)REG_MI_MCCNT1_A_SE_MASK : 0)
               | (csc ? REG_MI_MCCNT1_A_CSC_MASK : 0)
               | (dsc ? REG_MI_MCCNT1_A_DS_MASK : 0);

#if 1
    regs->scramble = (cpn || dpn) ? (u32)REG_MI_MCCNT1_A_SE_MASK : 0
                   | csc ? REG_MI_MCCNT1_A_CSC_MASK : 0
                   | dsc ? REG_MI_MCCNT1_A_DS_MASK : 0;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_SetCtrl

  Description:  set game card control

  Arguments:    op : command

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SetCtrl( GCDSlot slot, GCDCtrlRegs* regs )
{
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    *cnt1 = regs->ctrl | regs->latency | regs->scramble;
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_SetOp

  Description:  set game card command

  Arguments:    op : command

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SetOp( GCDSlot slot, GCDCmd64* op )
{
    GCDi_SelectRom( slot );

    {
        REGType8v* cmd = GCDi_SelectRegAddr( slot, REG_MCCMD0_ADDR );

        cmd[0] = op->b[7];
        cmd[1] = op->b[6];
        cmd[2] = op->b[5];
        cmd[3] = op->b[4];
        cmd[4] = op->b[3];
        cmd[5] = op->b[2];
        cmd[6] = op->b[1];
        cmd[7] = op->b[0];
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCD_SendOnlyCardOpCore

  Description:  send only rom command
                sync version

  Arguments:    ctrls  : rom control

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_SendOnlyCardOpCore( GCDSlot slot, GCDRomCtrls *ctrls )
{
    GCDi_WaitCtrl( slot );

    GCDi_SetOp( slot, &ctrls->gcdOp );

    GCDi_SetCtrl( slot, ctrls->gcdRegs );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRomID

  Description:  read rom ID
                sync version

  Arguments:    None

  Returns:      rom ID
 *---------------------------------------------------------------------------*/
u32 GCD_ReadRomID( GCDSlot slot )
{
    return  GCD_ReadGameModeID( slot );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadGameModeID

  Description:  read rom ID on game mode
                sync version

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 GCD_ReadGameModeID( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;
    u32 romID = 0;
    u16 id = (u16)OS_GetLockID();

    if ( !GCD_IsExisting( slot ) )
    {
        return  romID;
    }

    if ( slot == GCD_PRIMARY_SLOT )
    {
        OS_LockCard( id );
    }
    else
    {
        OS_LockExCard( id );
    }

    ctrls->gcdRegs = &ctrls->gIDRegs;
    ctrls->gcdOp.dw = GCDOP_G_OP_RD_ROM_ID;

    romID = GCDi_ReadRomIDCore( slot, ctrls );

    if ( slot == GCD_PRIMARY_SLOT )
    {
        OS_UnlockCard( id );
    }
    else
    {
        OS_UnlockExCard( id );
    }
    OS_ReleaseLockID( id );

    return  romID;
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadRomIDCore

  Description:  read rom ID
                sync version

  Arguments:    ctrls  : rom control

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 GCDi_ReadRomIDCore( GCDSlot slot, GCDRomCtrls *ctrls )
{
    REGType32v* data1 = GCDi_SelectRegAddr( slot, REG_MCD1_ADDR );

    GCDi_WaitCtrl( slot );

    GCDi_SetOp( slot, &ctrls->gcdOp );

    GCDi_SetCtrl( slot, ctrls->gcdRegs );
    GCDi_WaitData( slot );

    return  *data1;
}


/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRom

  Description:  read rom data
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadRom( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDWork *gw = &GCDi_Work[slot];
    u16 id;

    if ( !GCD_IsExisting( slot ) )
    {
        return;
    }

    id = (u16)OS_GetLockID();
    if ( slot == GCD_PRIMARY_SLOT )
    {
        OS_LockCard( id );
    }
    else
    {
        OS_LockExCard( id );
    }

    if ( GCDi_ReadRomPreCore( slot, romp, ramp, size ) )
    {
        GCD_ReadGameModeRom( slot, gw->romp, gw->ramp, gw->restSize );
    }

    if ( slot == GCD_PRIMARY_SLOT )
    {
        OS_UnlockCard( id );
    }
    else
    {
        OS_UnlockExCard( id );
    }
    OS_ReleaseLockID( id );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRomAsync

  Description:  read rom data
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDWork *gw = &GCDi_Work[slot];

    if ( GCDi_ReadRomPreCore( slot, romp, ramp, size ) )
    {
        GCD_ReadGameModeRomAsync( slot, gw->romp, gw->ramp, gw->restSize );
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadRomPreCore

  Description:  read rom data
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL GCDi_ReadRomPreCore( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    BOOL game_area = GCDi_ReadRomCommonPreCore( slot, romp, ramp, size );
    {
        GCDWork *gw = &GCDi_Work[slot];
        romp = gw->romp;
        ramp = gw->ramp;
        size = gw->restSize;
    }

    return game_area;
}

BOOL GCDi_ReadRomCommonPreCore( GCDSlot slot, u32 romp, void *ram, s32 size )
{
    GCDRomHeader  *rh = GCDi_GetRomHeaderAddr( slot );
    u32 romEndp = romp + size;
    u32 secure2 = (u32)rh->l.twlAreaOffset * GCD_TWL_AREA_ALIGN + GCD_SECURE2_AREA_OFFSET;
    u32 game2 = secure2 + GCD_SECURE2_AREA_SIZE;
    u8 *ramp = ram;

    // header
    if ( romp < GCD_ROM_HEADER_SIZE
      && size > 0 )
    {
        s32 otherSize = (s32)romEndp - GCD_ROM_HEADER_SIZE;
        s32 hdrSize = size;
        if ( otherSize > 0 )
        {
            hdrSize -= otherSize;
        }
        MI_CpuCopyFast( &GCDi_HeaderBuf[slot][romp/4], ramp, (u32)hdrSize );
        romp += hdrSize;
        ramp += hdrSize;
        size -= hdrSize;
    }

    // pad
    if ( romp >= GCD_ROM_HEADER_SIZE && romp < GCD_SECURE_AREA_OFFSET
      && size > 0 )
    {
        s32 otherSize = (s32)romEndp - (GCD_SECURE_AREA_OFFSET - GCD_ROM_HEADER_SIZE);
        s32 padSize = size;
        if ( otherSize > 0 )
        {
            padSize -= otherSize;
        }
        MI_CpuClearFast( ramp, (u32)padSize );
        romp += padSize;
        ramp += padSize;
        size -= padSize;
    }

    //secure
    if ( romp >= GCD_SECURE_AREA_OFFSET && romp < GCD_GAME_AREA_OFFSET
      && size > 0 )
    {
        s32 gameSize = (s32)romEndp - GCD_GAME_AREA_OFFSET;
        s32 secureSize = size;
        if ( gameSize > 0 )
        {
            secureSize -= gameSize;
        }
        MI_CpuCopyFast( &GCDi_SecureAreaBuf[slot][(romp - GCD_SECURE_AREA_OFFSET)/4], ramp, (u32)secureSize );
        romp += secureSize;
        ramp += secureSize;
        size -= secureSize;
    }

    //secure2
    if ( rh->l.twlAreaOffset
      && romp >= secure2 && romp < game2
      && size > 0 )
    {
        s32 game2Size = (s32)(romEndp - game2);
        s32 secure2Size = size;
        if ( game2Size > 0 )
        {
            secure2Size -= game2Size;
        }
        MI_CpuCopyFast( &GCDi_Secure2AreaBuf[slot][(romp - secure2)/4], ramp, (u32)secure2Size );
        romp += secure2Size;
        ramp += secure2Size;
        size -= secure2Size;
    }

    GCD_WaitRomAsync( slot );

    {
        GCDWork *gw = &GCDi_Work[slot];
        gw->romp = romp;
        gw->ramp = ramp;
        gw->restSize = size;
    }

    // game
    if ( romp >= GCD_GAME_AREA_OFFSET
      && size > 0 )
    {
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadNormalModeRom

  Description:  read rom data on normal mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadNormalModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;
    s32 restSize = size;
    s32 oneShotSize;

    ctrls->gcdRegs = &ctrls->nReadRomRegs;
    oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );

    if ( GCDi_SharedWork[slot].nCardID & GCD_ROMID_1TROM_MASK )
    {
        // 3Dメモリ専用設定
        {
            ctrls->gcdRegs->ctrl &= ~REG_MI_MCCNT1_A_PC_MASK;
            ctrls->gcdRegs->ctrl |= GCD_PAGE_1;
        }
    }

    while ( restSize > 0 )                                  // ブロック分割読み込み
    {
        GCDi_ReadNormalModeRomCore( slot, romp, ramp, oneShotSize, ctrls );

                 romp += oneShotSize;
        *(u8 **)&ramp += oneShotSize;
        restSize      -= oneShotSize;
    }

    GCDi_WaitDma( slot, ctrls->dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadNormalModeRomCore

  Description:  read rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadNormalModeRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
    ctrls->gcdOp.dw = GCDOP_N_OP_RD_PAGE
                     | ((u64 )(romp/GCD_ROM_PAGE_SIZE) << GCDOP_N_RD_PAGE_ADDR_SHIFT);

    GCDi_ReadRomCore( slot, ramp, size, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadGameModeRom

  Description:  read rom data on game mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadGameModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;
    s32 restSize = size;
    s32 oneShotSize;

    ctrls->gcdRegs = &ctrls->gReadRomRegs;
    oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );

    while ( restSize > 0 )                                  // ブロック分割読み込み
    {
        GCDi_ReadGameModeRomCore( slot, romp, ramp, oneShotSize, ctrls );

                 romp += oneShotSize;
        *(u8 **)&ramp += oneShotSize;
        restSize      -= oneShotSize;
    }

    GCDi_WaitDma( slot, ctrls->dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadGameModeRomCore

  Description:  read rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_ReadGameModeRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
    ctrls->gcdOp.dw = GCDOP_G_OP_RD_PAGE
                     | ((u64 )(romp/GCD_ROM_PAGE_SIZE) << GCDOP_G_RD_PAGE_ADDR_SHIFT);

    GCDi_ReadRomCore( slot, ramp, size, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadRomCore

  Description:  read rom data on game mode

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadRomCore( GCDSlot slot, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
    u32 dmaNo = ctrls->dmaNo;
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );
    REGType32v* data1 = GCDi_SelectRegAddr( slot, REG_MCD1_ADDR );

    if ( !GCD_IsExisting( slot ) )
    {
        return;
    }

    GCDi_WaitDma( slot, dmaNo );

    GCDi_SetOp( slot, &ctrls->gcdOp );                         // コマンド設定

    if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
    {
        GCD_DmaRecvRomAsync( slot, dmaNo, ramp );
        GCDi_SetCtrl( slot, ctrls->gcdRegs );                  // コントロール設定 & DMAスタート
    }
    else
    {
        void *ramEndp;
        u32 ctrlTmp;

        GCDi_SetCtrl( slot, ctrls->gcdRegs );                   // コントロール設定

        ramEndp = (u8 *)ramp + size;                            // 格納終了アドレス算出

        do {                                                    // CPU読み込み
            ctrlTmp = *cnt1;

            if (ctrlTmp & REG_MI_MCCNT1_A_RDY_MASK) {
                u32 dataTmp = *data1;

                if (ramp < ramEndp)
                {
                    *((vu32 *)ramp) = dataTmp;                  // 指定サイズまで格納（後続データは読み捨て）
                    (*(vu32 **)&ramp)++;
                }
            }
        }
        while (ctrlTmp & REG_MI_MCCNT1_A_START_MASK);
    }
}


/*---------------------------------------------------------------------------*
  Name:         GCD_WriteGameModeRom

  Description:  write rom data on game mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_WriteGameModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;
    s32 restSize = size;
    s32 oneShotSize;

    ctrls->gcdRegs = &ctrls->gWriteRomRegs;

    oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );

    while ( restSize > 0 )                                  // ブロック分割読み込み
    {
        GCDi_WriteGameModeRomCore( slot, romp, ramp, oneShotSize, ctrls );

                 romp += oneShotSize;
        *(u8 **)&ramp += oneShotSize;
        restSize      -= oneShotSize;
    }

    GCDi_WaitDma( slot, ctrls->dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_WriteGameModeRomCore

  Description:  write rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_WriteGameModeRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
    ctrls->gcdOp.dw = GCDOP_G_OP_WR_PAGE
                     | ((u64 )(romp/GCD_ROM_PAGE_SIZE) << GCDOP_G_RD_PAGE_ADDR_SHIFT);

    GCDi_WriteRomCore( slot, ramp, size, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_WriteRomCore

  Description:  write rom data on game mode

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WriteRomCore( GCDSlot slot, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
    u32 dmaNo = ctrls->dmaNo;
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );
    REGType32v* data1 = GCDi_SelectRegAddr( slot, REG_MCD1_ADDR );

//    OS_TPrintf( "\nwrite size = %#x\n\n", size );

    GCDi_WaitDma( slot, dmaNo );

    GCDi_SetOp( slot, &ctrls->gcdOp );                     // コマンド設定

    if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
    {
        GCD_DmaSendRomAsync( slot, dmaNo, ramp );
        GCDi_SetCtrl( slot, ctrls->gcdRegs );              // コントロール設定 & DMAスタート
    }
    else
    {
        void *ramEndp;
        u32 ctrlTmp;

        GCDi_SetCtrl( slot, ctrls->gcdRegs );                   // コントロール設定

        ramEndp = (u8 *)ramp + size;                            // 格納終了アドレス算出

        do {                                                    // CPU読み込み
            ctrlTmp = *cnt1;

            if (ctrlTmp & REG_MI_MCCNT1_A_RDY_MASK) {
                u32 dataTmp = 0;

                if (ramp < ramEndp)
                {
                    dataTmp = *((vu32 *)ramp);
                    (*(vu32 **)&ramp)++;
                }
                *data1 = dataTmp;
            }
        }
        while (ctrlTmp & REG_MI_MCCNT1_A_START_MASK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCD_DmaRecvRomAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCD_DmaRecvRomAsync( GCDSlot slot, u32 dmaNo, void *dest )
{
    const void* data1 = GCDi_SelectRegAddr( slot, REG_MCD1_ADDR );
    u32 timing = MI_NDMA_TIMING_CARD_A;
    u32 blockSize = MI_NDMA_BWORD_1;
    u32 interval = 1;
    u32 prescale = MI_NDMA_INTERVAL_PS_1;

    if ( slot == GCD_SECONDARY_SLOT )
    {
        timing = MI_NDMA_TIMING_CARD_B;
    }

    MIi_ExDmaRecvAsyncCore(dmaNo, data1, dest, 4, 4,
                blockSize, interval, prescale,
                MI_NDMA_CONTINUOUS_ON, MI_NDMA_SRC_RELOAD_ENABLE, MI_NDMA_DEST_RELOAD_DISABLE,
                timing);
}

/*---------------------------------------------------------------------------*
  Name:         GCD_DmaSendRomAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCD_DmaSendRomAsync( GCDSlot slot, u32 dmaNo, void *src )
{
    const void* data1 = GCDi_SelectRegAddr( slot, REG_MCD1_ADDR );
    u32 timing = MI_NDMA_TIMING_CARD_A;
    u32 blockSize = MI_NDMA_BWORD_1;
    u32 interval = 1;
    u32 prescale = MI_NDMA_INTERVAL_PS_1;

    if ( slot == GCD_SECONDARY_SLOT )
    {
        timing = MI_NDMA_TIMING_CARD_B;
    }

    MIi_ExDmaSendAsyncCore(dmaNo, src, (void*)REG_MCD1_ADDR, 4, 4,
                blockSize, interval, prescale,
                MI_NDMA_CONTINUOUS_ON, MI_NDMA_SRC_RELOAD_DISABLE, MI_NDMA_DEST_RELOAD_ENABLE,
                timing);
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_GetOneShotSizeFromCtrl

  Description:  get block size

  Arguments:    ctrl  : rom control

  Returns:      None
 *---------------------------------------------------------------------------*/
s32 GCDi_GetOneShotSizeFromCtrl( u32 ctrl )
{
    s32 oneShotSize = 0;
    u32 pages = (ctrl & REG_MI_MCCNT1_A_PC_MASK) >> REG_MI_MCCNT1_A_PC_SHIFT;
    s32 pageSize = 512;

    oneShotSize = pageSize << (pages - 1);

    return  oneShotSize;
}


/*---------------------------------------------------------------------------*
  Name:         GCD_ReadNormalModeRomAsync

  Description:  read rom data on normal mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadNormalModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

    ctrls->gcdRegs = &ctrls->nReadRomRegs;

    gw->funcp = GCDi_ReadNormalModeRomCore;

    GCD_SetInterrupt( slot, romp, ramp, size, gw );

    gw->funcp( slot, gw->romp, gw->ramp, gw->oneShotSize, ctrls );
}


/*---------------------------------------------------------------------------*
  Name:         GCD_ReadGameModeRomAsync

  Description:  read rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadGameModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

    ctrls->gcdRegs = &ctrls->gReadRomRegs;

    gw->funcp = GCDi_ReadGameModeRomCore;

    GCD_SetInterrupt( slot, romp, ramp, size, gw );

    gw->funcp( slot, gw->romp, gw->ramp, gw->oneShotSize, ctrls );
}


/*---------------------------------------------------------------------------*
  Name:         GCD_WriteGameModeRomAsync

  Description:  write rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_WriteGameModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

    ctrls->gcdRegs = &ctrls->gWriteRomRegs;

    gw->funcp = GCDi_WriteGameModeRomCore;

    GCD_SetInterrupt( slot, romp, ramp, size, gw );

    gw->funcp( slot, gw->romp, gw->ramp, gw->oneShotSize, ctrls );
}


/*---------------------------------------------------------------------------*
  Name:         GCD_SetInterrupt

  Description:  set interrupt

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_SetInterrupt( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDWork *gw )
{
    GCDRomCtrls *ctrls = &gw->ctrls;
    u32 dmaNo = ctrls->dmaNo;
    u32 mask = GCDi_SelectIrqMask( slot, OS_IE_CARD_DATA );
    OSIrqFunction intr = GCDi_InterruptHandlerPRIME;

    gw->romp = romp;
    gw->ramp = ramp;
    gw->restSize  = size;
    gw->oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );

    if ( dmaNo < MI_EXDMA_CH_MIN || MI_EXDMA_CH_MAX < dmaNo )
    {
        ctrls->lastDmaNo = ctrls->dmaNo;
        ctrls->dmaNo = GCD_DEFAULT_DMA_A_NO;
        if ( slot == GCD_SECONDARY_SLOT )
        {
            ctrls->dmaNo = GCD_DEFAULT_DMA_B_NO;
        }
    }

    GCDi_WaitDma( slot, dmaNo );

    (void)OS_DisableInterrupts();

    if ( slot )
    {
        intr = GCDi_InterruptHandlerSECOND;
    }

    OS_SetIrqFunction( mask, intr );

    gw->isAsync = TRUE;

    gw->intrDone = FALSE;

    reg_OS_IF  = mask;
    reg_OS_IE |= mask;

    (void)OS_EnableInterrupts();
    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_InterruptHandler

  Description:  interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_InterruptHandlerCommon( GCDSlot slot )
{
    GCDWork *gw = &GCDi_Work[slot];
    s32  oneShotSize = gw->oneShotSize;

    // read block

    gw->romp += oneShotSize;
    gw->ramp += oneShotSize;
    gw->restSize -= oneShotSize;

    if (gw->restSize > 0) {
        GCDRomCtrls *ctrls = &gw->ctrls;
        gw->oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );
        gw->funcp( slot, gw->romp, gw->ramp, oneShotSize, &gw->ctrls );
    } else {
        gw->intrDone = TRUE;                            // カード割込処理完了の通知
    }
}

void GCDi_InterruptHandlerPRIME( void )
{
    GCDi_InterruptHandlerCommon( GCD_PRIMARY_SLOT );
}

void GCDi_InterruptHandlerSECOND( void )
{
    GCDi_InterruptHandlerCommon( GCD_SECONDARY_SLOT );
}


//================================================================================
//       WAIT/STOP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         GCD_IsBusy

  Description:  check whether game card is busy or not

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL GCD_IsBusy( GCDSlot slot )
{
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    return (BOOL)((*cnt1 & REG_MI_MCCNT1_A_START_MASK) >> REG_MI_MCCNT1_A_START_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         GCD_IsDataReady

  Description:  check whether data is ready or not

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL GCD_IsDataReady( GCDSlot slot )
{
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    return (BOOL)((*cnt1 & REG_MI_MCCNT1_A_RDY_MASK) >> REG_MI_MCCNT1_A_RDY_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         GCD_Stop

  Description:  stop game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_Stop( GCDSlot slot )
{
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    OSIntrMode enabled = OS_DisableInterrupts();

    *cnt1 &= ~REG_MI_MCCNT1_A_START_MASK;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitCtrl

  Description:  wait while game card is busy

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitCtrl( GCDSlot slot )
{
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    while (*cnt1 & REG_MI_MCCNT1_A_START_MASK)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitData

  Description:  wait until data is ready

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitData( GCDSlot slot )
{
    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    while ( !(*cnt1 & REG_MI_MCCNT1_A_RDY_MASK) )
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitDma

  Description:  wait for stopping game card DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitDma( GCDSlot slot, u32 dmaNo )
{
    GCDi_WaitCtrl( slot );

    if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
    {
        MIi_StopExDma( dmaNo );
    }
}


/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitInterrupt

  Description:  wait for game card interrupt

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitInterrupt( GCDSlot slot )
{
    GCDWork *gw = &GCDi_Work[slot];

    if ( gw->isAsync )
    {
        while ( !gw->intrDone )
        {
        }
    }
    gw->intrDone = FALSE;
    gw->isAsync = FALSE;

    GCDi_WaitDma( slot, gw->ctrls.dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_WaitRomAsync

  Description:  wait for game card async access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_WaitRomAsync( GCDSlot slot )
{
    GCDi_WaitInterrupt( slot );

    {
        GCDWork *gw = &GCDi_Work[slot];
        GCDRomCtrls *ctrls = &gw->ctrls;
        u32 mask = GCDi_SelectIrqMask( slot, OS_IE_CARD_DATA );

        OSIntrMode enabled = OS_DisableInterrupts();

        ctrls->dmaNo = ctrls->lastDmaNo;

#define IOP_DISABLE_OS_TIMER
#ifdef IOP_DISABLE_OS_TIMER
        reg_OS_IE &= ~REG_OS_IE_T1_MASK;
#endif // IOP_DISABLE_OS_TIMER

        reg_OS_IE &= ~mask;       // disable card interrupt

        (void)OS_RestoreInterrupts( enabled );
    }
}


