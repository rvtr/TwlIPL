/*---------------------------------------------------------------------------*
  Project:  TwlBrom - GCD - libraries
  File:     gcd_init.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <twl/os.h>
#include <nitro/pxi.h>
#include <firm/os.h>
#include <firm/gcd.h>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#define DBG_PRINTF vlink_dos_printf
#define DBG_CHAR vlink_dos_put_console
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif


//#define FIRM_FORCE_DEBUGGER
#define FIRM_FORCE_FIRMBLOWFISH

#ifndef FIRM_FORCE_DEBUGGER
#define GCD_ENABLE_SCRAMBLE
#endif // FIRM_FORCE_DEBUGGER

static void GCDi_GetExistingSlots( GCDSlot* start, GCDSlot* end );
void GCDi_CallbackForReset(PXIFifoTag tag, u32 data, BOOL err);
void GCDi_CallbackForResetAll(PXIFifoTag tag, u32 data, BOOL err);

static void GCDi_SetPngIntf( GCDSlot slot, u32 pnA_l, u8 pnA_h, u32 pnB_l, u8 pnB_h );
static void GCDi_SendOnlyCardSecureOpCore( GCDSlot slot, GCDRomCtrls *ctrls );
static void GCDi_ReadSecureModeRomCore( GCDSlot slot, void *ramp, s32 size, GCDRomCtrls *ctrls );
static void Encrypt2SetTimer4Secure( GCDSlot slot, GCDRomCtrls *ctrls );

static BOOL IsCtrApp( GCDSlot slot );
static BOOL IsDsApp( GCDSlot slot );

static BOOL GCDi_SecureInitialized[2];


static void GCDi_LoadHeader( GCDSlot slot );
static void GCDi_LoadHeaderAll( void );
static void GCDi_LoadTable( GCDSlot slot );
static void GCDi_LoadTableAsync( GCDSlot slot );
static void GCDi_ReadShortHeader( GCDSlot slot, void *ramp );
static void GCDi_ReadShortHeaderAsync( GCDSlot slot, void *ramp );
static void GCDi_LoadSecure( GCDSlot slot );
static void GCDi_LoadSecure2( GCDSlot slot );
static void GCDi_LoadSecureAll( void );
static void GCDi_LoadSecure2All( void );

static BOOL GCD_Initialized[2];
extern GCDSharedWork GCDi_SharedWork[2];
extern GCDSecureWork GCDi_SecureWork[2];

GCDWork  GCDi_Work[2];



void* GCDi_GetRomHeaderAddr( GCDSlot slot )
{
    return  ((void*)GCDi_HeaderBuf[slot]);
}


// ※SDKにSecondarySlotのカードプロセッサ設定が用意されていないので、とりあえずローカルで用意。
#ifdef SDK_ARM9
static inline void MIi_SetExCardProcessor(MIProcessor proc)
{
    MI_PROCESSOR_ASSERT(proc);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_MPB_MASK) | (proc << REG_MI_EXMEMCNT_MPB_SHIFT));
}
#endif
#ifdef SDK_ARM7
#define REG_MI_EXMEMCNT_MPB_MASK                           0x0400
#define REG_MI_EXMEMCNT_MPB_SHIFT                          10
#endif
static inline MIProcessor MI_GetExCardProcessor(void)
{
    return (MIProcessor)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_MPB_MASK) >> REG_MI_EXMEMCNT_MPB_SHIFT);
}

// ※SDKにSecondarySlotのカードロックが用意されていないので、とりあえずPrimarySlotと共用で。
#ifdef  SDK_ARM9
#define OSi_ASSERT_ID( id )       SDK_ASSERTMSG( id >= OS_MAINP_LOCK_ID_START && id <= OS_MAINP_SYSTEM_LOCK_ID, \
                             "lock ID %d is out of bounds", id )
#else
#define OSi_ASSERT_ID( id )       SDK_ASSERTMSG( id >= OS_SUBP_LOCK_ID_START && id <= OS_SUBP_SYSTEM_LOCK_ID, \
                             "lock ID %d is out of bounds", id )
#endif

static void OSi_AllocateExCardBus(void)
{
#ifdef  SDK_ARM9
    MIi_SetExCardProcessor(MI_PROCESSOR_ARM9);    // Card for MAIN
#endif
}

static void OSi_FreeExCardBus(void)
{
#ifdef  SDK_ARM9
    MIi_SetExCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
#endif
}

#define HW_CARD_B_LOCK_BUF			HW_CTRDG_LOCK_BUF

s32 OS_LockExCard(u16 lockID)
{
    OSi_ASSERT_ID(lockID);

    return OS_LockByWord(lockID, (OSLockWord *)HW_CARD_B_LOCK_BUF, OSi_AllocateExCardBus);
}

s32 OS_UnlockExCard(u16 lockID)
{
    OSi_ASSERT_ID(lockID);

    return OS_UnlockByWord(lockID, (OSLockWord *)HW_CARD_B_LOCK_BUF, OSi_FreeExCardBus);
}


/*---------------------------------------------------------------------------*
  Name:         GCD_Init

  Description:  initialize for game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_Init( void )
{
    GCDSlot slot;
    MIProcessor lastProc = MI_GetCardProcessor();
    MIProcessor lastProcB = MI_GetExCardProcessor();
    u16 id = (u16)OS_GetLockID();

    OS_LockCard( id );
    OS_LockExCard( id );

#ifdef SDK_ARM9
    MIi_SetCardProcessor( MI_PROCESSOR_ARM9 );
    MIi_SetExCardProcessor( MI_PROCESSOR_ARM9 );
#else // SDK_ARM9
    while ( MI_GetCardProcessor() != MI_PROCESSOR_ARM7 )
    {
    }
    while ( MI_GetExCardProcessor() != MI_PROCESSOR_ARM7 )
    {
    }
#endif // SDK_ARM7

    if ( GCD_Initialized[GCD_PRIMARY_SLOT] == FALSE ||
         GCD_Initialized[GCD_SECONDARY_SLOT] == FALSE )
    {
        GCDi_Init( TRUE );
        GCDi_LoadHeaderAll();
        GCDi_LoadSecureAll();
    }

    for (slot=GCD_PRIMARY_SLOT; slot<=GCD_SECONDARY_SLOT; slot++)
    {
        if ( GCD_Initialized[slot] == FALSE )
        {
            if ( GCD_IsExisting( slot ) )
            {
                if ( GCDi_IsDsApp( slot ) )
                {
                    GCDRomHeader *rh = GCDi_GetRomHeaderAddr( slot );

                    if ( rh->l.twlAreaOffset ) // && (sh->nCardID & GCD_ROMID_TWLROM_MASK) )
                    {
                        u32 chat_cycles = OS_MSEC_TO_CPUCYC( 25 ) / GCD_CHATTERING_CYCLES;
#ifdef SDK_ARM9
                        chat_cycles /= 2;
#endif // SDK_ARM9
                        GCD_Reset( slot, chat_cycles );
                        GCDi_LoadHeader( slot );
                        GCDi_LoadSecure2( slot );
                    }

                    GCD_Initialized[slot] = TRUE;
                }
            }
        }
    }

#ifdef SDK_ARM9
    MIi_SetCardProcessor( lastProc );
    MIi_SetExCardProcessor( lastProcB );
#endif // SDK_ARM9

    OS_UnlockExCard( id );
    OS_UnlockCard( id );

    OS_ReleaseLockID( id );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_Init

  Description:  initialize for game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Init( BOOL reset )
{
    static BOOL initialized;

    if ( initialized == FALSE )
    {
        GCDSlot slot;

        if ( reset )
        {
#ifdef SDK_ARM7

            OSIntrMode enabled;
            OSIrqMask lastIE;
            BOOL    lastIME;

            lastIE = OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
            lastIME = OS_EnableIrq();
            enabled = OS_EnableInterrupts();

            //---- setting PXI
            PXI_Init();
            PXI_SetFifoRecvCallback(PXI_FIFO_TAG_GCD, GCDi_CallbackForResetAll);

            while ( !( GCDi_Work[GCD_PRIMARY_SLOT].isPxiDone == TRUE &&
                       GCDi_Work[GCD_SECONDARY_SLOT].isPxiDone == TRUE ) )
            {
            }

            (void)OS_RestoreIrq(lastIME);
            (void)OS_SetIrqMask(lastIE);
            (void)OS_RestoreInterrupts(enabled);

#else // SDK_ARM9

            u32 chat_cycles = OS_MSEC_TO_CPUCYC( 25 ) / GCD_CHATTERING_CYCLES;
            chat_cycles /= 2;
            GCD_ResetAll( chat_cycles );

#endif // SDK_ARM9
        }

        for (slot=GCD_PRIMARY_SLOT; slot<=GCD_SECONDARY_SLOT; slot++)
        {
            GCDWork *gw = &GCDi_Work[slot];
            GCDRomCtrls *ctrls = &gw->ctrls;

            GCDi_GenCtrl( &ctrls->nLoadTableRegs, GCD_RW_READ, 
                       GCD_PAGE_16, 
                       GCD_CKT_240NS, 0, 0x18,
                       FALSE, FALSE, FALSE, FALSE );

            GCDi_GenCtrl( &ctrls->nSendOnlyOpRegs, GCD_RW_READ, 
                       GCD_PAGE_0, 
                       GCD_CKT_240NS, 0x18, 0, 
                       FALSE, FALSE, FALSE, FALSE );

            GCDi_GenCtrl( &ctrls->nIDRegs, GCD_RW_READ, 
                       GCD_PAGE_STAT, 
                       GCD_CKT_240NS, 0x18, 0, 
                       FALSE, FALSE, FALSE, FALSE );

            GCDi_GenCtrl( &ctrls->nReadShortHeaderRegs, GCD_RW_READ, 
                       GCD_PAGE_1, 
                       GCD_CKT_240NS, REG_MI_MCCNT1_L1_MASK, REG_MI_MCCNT1_L2_MASK >> REG_MI_MCCNT1_L2_SHIFT,
                       FALSE, FALSE, FALSE, FALSE );

            GCDi_GenCtrl( &ctrls->nReadRomRegs, GCD_RW_READ, 
                       GCD_PAGE_8, 
                       GCD_CKT_240NS, 0x657, 0x1, 
                       FALSE, FALSE, FALSE, FALSE );

            GCDi_GenCtrl( &ctrls->sSendOnlyOpRegs, GCD_RW_READ, 
                       GCD_PAGE_0, 
                       GCD_CKT_240NS, 0x8f8+0x18, 0, 
                       FALSE, FALSE, FALSE, FALSE ); // enable after PNG ON

            GCDi_GenCtrl( &ctrls->sIDRegs, GCD_RW_READ, 
                       GCD_PAGE_STAT, 
                       GCD_CKT_240NS, 0x8f8+0x18, 0, 
                       FALSE, TRUE, FALSE, TRUE );

            GCDi_GenCtrl( &ctrls->sReadRomRegs, GCD_RW_READ, 
                       GCD_PAGE_8, 
                       GCD_CKT_240NS, 0x8f8, 0x18, 
                       FALSE, TRUE, FALSE, TRUE );

            GCDi_GenCtrl( &ctrls->gIDRegs, GCD_RW_READ, 
                       GCD_PAGE_STAT, 
                       GCD_CKT_150NS, 0, 0x1, 
#ifdef GCD_ENABLE_SCRAMBLE
                       TRUE, TRUE, TRUE, TRUE );
#else // GCD_ENABLE_SCRAMBLE
                       FALSE, FALSE, FALSE, FALSE );
#endif // GCD_ENABLE_SCRAMBLE

            GCDi_GenCtrl( &ctrls->gReadRomRegs, GCD_RW_READ, 
                       GCD_PAGE_1, 
                       GCD_CKT_150NS, 0x657, 0x1, 
#ifdef GCD_ENABLE_SCRAMBLE
                       TRUE, TRUE, TRUE, TRUE );
#else // GCD_ENABLE_SCRAMBLE
                       FALSE, FALSE, FALSE, FALSE );
#endif // GCD_ENABLE_SCRAMBLE

            GCDi_GenCtrl( &ctrls->gWriteRomRegs, GCD_RW_WRITE, 
                       GCD_PAGE_1, 
                       GCD_CKT_150NS, 0x21, 0x1, 
#ifdef GCD_ENABLE_SCRAMBLE
                       TRUE, TRUE, TRUE, TRUE );
#else // GCD_ENABLE_SCRAMBLE
                       FALSE, FALSE, FALSE, FALSE );
#endif // GCD_ENABLE_SCRAMBLE
        }

        initialized = TRUE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GCD_SetPrimarySlot

  Description:  set primary game card slot

  Arguments:    primary slot

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_SetPrimarySlot( u8 slot )
{
    if ( slot <= 1 )
    {
        reg_MI_MC_SWP = (u8)(slot * REG_MI_MC_SWP_E_MASK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCD_GetPrimarySlot

  Description:  get primary game card slot

  Arguments:    None

  Returns:      primary slot
 *---------------------------------------------------------------------------*/
u8 GCD_GetPrimarySlot( void )
{
    return (u8)((reg_MI_MC_SWP & REG_MI_MC_SWP_E_MASK) >> REG_MI_MC_SWP_E_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         GCDi_SendtoPxi

  Description:  send data via PXI

  Arguments:    data : data to send

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SendtoPxi(u32 data)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_GCD, data, FALSE) != PXI_FIFO_SUCCESS)
    {
        SVC_WaitByLoop(1);
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_CallbackForReset

  Description:  callback to receive data from PXI

  Arguments:    tag  : tag from PXI (unused)
                data : data from PXI
                err  : error bit (unused)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_CallbackForReset(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, err )
    GCDPxiCmd pxi;
    pxi.raw = data;

    if (pxi.e.cmd == GCD_PXI_COMMAND_RESET)
    {
#ifdef SDK_ARM7
        GCD_Reset( (GCDSlot)pxi.e.slot, pxi.e.param );
        //---- send message to ARM9
        GCDi_SendtoPxi( GCD_PXI_COMMAND_RESET );
#endif // SDK_ARM7

        GCDi_Work[pxi.e.slot].isPxiDone = TRUE;
    }
    else
    {
#ifndef SDK_FINALROM
        OS_Panic("illegal GCD pxi command.");
#else
        OS_Panic("");
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_CallbackForResetAll

  Description:  callback to receive data from PXI

  Arguments:    tag  : tag from PXI (unused)
                data : data from PXI
                err  : error bit (unused)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_CallbackForResetAll(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, err )
    GCDPxiCmd pxi;
    pxi.raw = data;

    if (pxi.e.cmd == GCD_PXI_COMMAND_RESET)
    {
#ifdef SDK_ARM7
        GCD_ResetAll( pxi.e.param );
        //---- send message to ARM9
        GCDi_SendtoPxi( GCD_PXI_COMMAND_RESET );
#endif // SDK_ARM7

        GCDi_Work[GCD_PRIMARY_SLOT].isPxiDone = TRUE;
        GCDi_Work[GCD_SECONDARY_SLOT].isPxiDone = TRUE;
    }
    else
    {
#ifndef SDK_FINALROM
        OS_Panic("illegal GCD pxi command.");
#else
        OS_Panic("");
#endif
    }
}


/*---------------------------------------------------------------------------*
  Name:         GCDi_GetExistingSlots

  Description:  get enabled slots

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_GetExistingSlots( GCDSlot* start, GCDSlot* end )
{
    if ( GCD_IsExisting( GCD_PRIMARY_SLOT ) )
    {
        *start = GCD_PRIMARY_SLOT;
        if ( GCD_IsExisting( GCD_SECONDARY_SLOT ) )
        {
            *end = GCD_SECONDARY_SLOT;
        }
        else
        {
            *end = GCD_PRIMARY_SLOT;
        }
    }
    else
    {
        *start = GCD_SECONDARY_SLOT;
        if ( GCD_IsExisting( GCD_SECONDARY_SLOT ) )
        {
            *end = GCD_SECONDARY_SLOT;
        }
        else
        {
            *end = GCD_NO_SLOT;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         GCD_Reset

  Description:  reset game card

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_Reset( GCDSlot slot, u32 chat_cycles )
{
#ifdef SDK_ARM9
#pragma unused( chat_cycles, slot )

    OSIntrMode enabled;
    OSIrqMask lastIE;
    BOOL    lastIME;

    GCDPxiCmd pxi;
    GCDi_Work[GCD_PRIMARY_SLOT].isPxiDone = FALSE;
    GCDi_Work[GCD_SECONDARY_SLOT].isPxiDone = FALSE;

    lastIE = OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
    lastIME = OS_EnableIrq();
    enabled = OS_EnableInterrupts();

    //---- setting PXI
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_GCD, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_GCD, GCDi_CallbackForReset);

    pxi.e.cmd = GCD_PXI_COMMAND_RESET;
    pxi.e.param = chat_cycles;
    pxi.e.slot = (u32)slot;

    //---- send message to ARM7
    GCDi_SendtoPxi( pxi.raw );

    while ( !( GCDi_Work[GCD_PRIMARY_SLOT].isPxiDone == TRUE &&
               GCDi_Work[GCD_SECONDARY_SLOT].isPxiDone == TRUE ) )
    {
    }

    (void)OS_RestoreIrq(lastIME);
    (void)OS_SetIrqMask(lastIE);
    (void)OS_RestoreInterrupts(enabled);

    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_GCD, NULL);

    GCDi_Enable( slot );

#else // SDK_ARM7

    REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );

    reg_MI_MCCHAT = (u16)chat_cycles;

    OS_SpinWait( chat_cycles * GCD_CHATTERING_CYCLES );

    if ( !GCD_IsExisting( slot ) )
    {
        return;
    }

    GCDi_Enable( slot );

    if ( GCD_GetDetectMode( slot ) != 0 )
    {
        GCD_SetDetectMode( slot, 3 );

        while ( GCD_GetDetectMode( slot ) != 0 )
        {
        }
    }

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );

    GCD_SetDetectMode( slot, 1 );

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );

    GCD_SetDetectMode( slot, 2 );

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );

    *cnt1 = REG_MI_MCCNT1_RESB_MASK;  // preset

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );

#endif // SDK_ARM7
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ResetAll

  Description:  reset game cards

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ResetAll( u32 chat_cycles )
{
#ifdef SDK_ARM9
#pragma unused( chat_cycles )

    GCDWork *gw = &GCDi_Work[GCD_PRIMARY_SLOT];
    OSIntrMode enabled;
    OSIrqMask lastIE;
    BOOL    lastIME;

    GCDSlot slot;
    GCDPxiCmd pxi;
    gw->isPxiDone = FALSE;

    lastIE = OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
    lastIME = OS_EnableIrq();
    enabled = OS_EnableInterrupts();

    //---- setting PXI
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_GCD, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_GCD, GCDi_CallbackForResetAll);

    pxi.e.cmd = GCD_PXI_COMMAND_RESET;
    pxi.e.param = chat_cycles;
    pxi.e.slot = (u32)NULL;

    //---- send message to ARM7
    GCDi_SendtoPxi( pxi.raw );

    while (gw->isPxiDone == FALSE)
    {
    }

    (void)OS_RestoreIrq(lastIME);
    (void)OS_SetIrqMask(lastIE);
    (void)OS_RestoreInterrupts(enabled);

    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_GCD, NULL);

    for (slot=GCD_PRIMARY_SLOT; slot<=GCD_SECONDARY_SLOT; slot++)
    {
        GCDi_Enable( slot );
    }

#else // SDK_ARM7

    GCDSlot slot, start, end;

    reg_MI_MCCHAT = (u16)chat_cycles;

    OS_SpinWait( chat_cycles * GCD_CHATTERING_CYCLES );

    GCDi_GetExistingSlots( &start, &end );

    for (slot=start; slot<=end; slot++)
    {
        GCDi_Enable( slot );

        if ( GCD_GetDetectMode( slot ) != 0 )
        {
            GCD_SetDetectMode( slot, 3 );

            while ( GCD_GetDetectMode( slot ) != 0 )
            {
            }
        }
    }

    if ( end != GCD_NO_SLOT )
    {
        OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );
    }

    for (slot=start; slot<=end; slot++)
    {
        GCD_SetDetectMode( slot, 1 );
    }

    if ( end != GCD_NO_SLOT )
    {
        OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );
    }

    for (slot=start; slot<=end; slot++)
    {
        GCD_SetDetectMode( slot, 2 );
    }

    if ( end != GCD_NO_SLOT )
    {
        OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );
    }

    for (slot=start; slot<=end; slot++)
    {
        REGType32v* cnt1 = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );
        *cnt1 = REG_MI_MCCNT1_RESB_MASK;  // preset
    }

    if ( end != GCD_NO_SLOT )
    {
        OS_SpinWait( OS_MSEC_TO_CPUCYC( 50 ) );
    }

#endif // SDK_ARM7
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_LoadSecure

  Description:  load secure area

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/

static BOOL isCtrApp[2];
static BOOL isDsApp[2];


/***************************************************

 ***************************************************/

BOOL GCDi_IsDsApp( GCDSlot slot )
{
    return isDsApp[slot];
}

static const u8 dsNinLogo[] = {
0x24,0xff,0xae,0x51,0x69,0x9a,0xa2,0x21,0x3d,0x84,0x82,0x0a,0x84,0xe4,0x09,0xad,
0x11,0x24,0x8b,0x98,0xc0,0x81,0x7f,0x21,0xa3,0x52,0xbe,0x19,0x93,0x09,0xce,0x20,
0x10,0x46,0x4a,0x4a,0xf8,0x27,0x31,0xec,0x58,0xc7,0xe8,0x33,0x82,0xe3,0xce,0xbf,
0x85,0xf4,0xdf,0x94,0xce,0x4b,0x09,0xc1,0x94,0x56,0x8a,0xc0,0x13,0x72,0xa7,0xfc,
0x9f,0x84,0x4d,0x73,0xa3,0xca,0x9a,0x61,0x58,0x97,0xa3,0x27,0xfc,0x03,0x98,0x76,
0x23,0x1d,0xc7,0x61,0x03,0x04,0xae,0x56,0xbf,0x38,0x84,0x00,0x40,0xa7,0x0e,0xfd,
0xff,0x52,0xfe,0x03,0x6f,0x95,0x30,0xf1,0x97,0xfb,0xc0,0x85,0x60,0xd6,0x80,0x25,
0xa9,0x63,0xbe,0x03,0x01,0x4e,0x38,0xe2,0xf9,0xa2,0x34,0xff,0xbb,0x3e,0x03,0x44,
0x78,0x00,0x90,0xcb,0x88,0x11,0x3a,0x94,0x65,0xc0,0x7c,0x63,0x87,0xf0,0x3c,0xaf,
0xd6,0x25,0xe4,0x8b,0x38,0x0a,0xac,0x72,0x21,0xd4,0xf8,0x07,0x56,0xcf
};

static BOOL IsDsApp( GCDSlot slot )
{
    GCDRomHeader  *rh = GCDi_GetRomHeaderAddr( slot );
    BOOL isDsNinLogo = TRUE;
    int i;

    isDsApp[slot] = FALSE;

    for ( i=0; i<sizeof(dsNinLogo)/2; i++ )
    {
        if ( dsNinLogo[i] != ((u8*)rh->l.nintendoLogo)[i] )
        {
            isDsNinLogo = FALSE;
            break;
        }
    }
    if ( isDsNinLogo )
    {
        isDsApp[slot] = TRUE;
    }

    return isDsApp[slot];
}

/***************************************************

 ***************************************************/
static void SetVAE( GCDSlot slot, GCDCmd64* op )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    op->dw |= ((u64)(sec->va) << GCDOP_N_VAE_SHIFT) & GCDOP_N_VAE_MASK;
}

static void SetVBI( GCDSlot slot, GCDCmd64* op )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    op->dw |= sec->vb;
    sec->vb >>= GCDOP_N_VBI_SHIFT;
}

static void SetVA( GCDSlot slot, GCDCmd64* op )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    op->dw |= ((u64)(sec->va) << GCDOP_S_VA_SHIFT) & GCDOP_S_VA_MASK;
}

static void SetVB( GCDSlot slot, GCDCmd64* op )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    op->dw |= sec->vb & GCDOP_S_VB_MASK;
    sec->vb++;
}

static void SetDummyVC( GCDSlot slot, GCDCmd64* op )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    op->dw |= ((u64)(sec->vc_dummy) << GCDOP_S_VC_SHIFT) & GCDOP_S_VC_MASK;
}

static void SetVD( GCDSlot slot, GCDCmd64* op )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    op->dw |= ((u64)(sec->vd) << GCDOP_S_VD_SHIFT) & GCDOP_S_VD_MASK;
}

static void GCDi_GenVA_VB_VD( GCDSlot slot )
{
    GCDHeader* fh = GCDi_GetRomHeaderAddr( slot );
    GCDSecureWork* sec = &GCDi_SecureWork[slot];

    sec->va = fh->sign.raw[0];
    sec->vb = fh->sign.raw[1];
    sec->vd = fh->sign.raw[2];
    sec->vc_dummy = fh->sign.raw[3];

    EncryptByBlowfish(&sec->blowfishCardTable, &sec->va, &sec->vb);
    EncryptByBlowfish(&sec->blowfishCardTable, &sec->vd, &sec->vc_dummy);
}

// TWLFIRM_DSCARD_DEV

#ifndef FIRM_FORCE_DEBUGGER

void GCDi_InitCardOpBlowfish( GCDSlot slot )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];
    const BLOWFISH_CTX *blowfishInitTablep = &GCDi_BlowfishInitTableGCDFIRM;
    BLOWFISH_CTX *tableBufp = &sec->blowfishCardTable;

    MI_CpuCopyFast( (void*)blowfishInitTablep, (void *)tableBufp, sizeof(BLOWFISH_CTX) );
    InitBlowfish( tableBufp, &((u8*)GCDi_HeaderBuf[slot])[0xc], 4 );
}

void GCDi_InitSecureParam( GCDSlot slot, BOOL twl_ex )
{
    GCDRomHeaderDS *rh = GCDi_GetRomHeaderAddr( slot );
    u32 code = rh->initialCode;

    if ( twl_ex )
    {
        rh->initialCode = 'ALWT';
    }

#ifdef FIRM_FORCE_FIRMBLOWFISH
    GCDi_InitCardOpBlowfish( slot );
#else // FIRM_FORCE_FIRMBLOWFISH
    GCDi_InitCardOpBlowfishDS( slot );
#endif // FIRM_FORCE_FIRMBLOWFISH

    rh->initialCode = code;

    GCDi_GenVA_VB_VD( slot );     // VA & VB & VD 初期化
}

void GCDi_InitPngIntf( GCDSlot slot )
{
    GCDSecureWork* sec = &GCDi_SecureWork[slot];

#define PNA_INDEX           (((u32)GCDi_GetRomHeaderAddr( slot )) + 0x12)  // PNAテーブル・インデックス
static u32 default_pnB_l  = 0x879b9b05;
static u8  default_pnB_h = 0x5c;
static u8  default_pnA_l_1 = 0x60;
static u8  default_pnA_l_0_table[8] = { 0xe8, 0x4d, 0x5a, 0xb1, 0x17, 0x8f };

    u32 pnA_l;
    u8  pnA_h;

    pnA_l = sec->vd <<15
          | default_pnA_l_0_table[(*(vu16 *)PNA_INDEX & 0x0700) >>8] | default_pnA_l_1 <<8;
    pnA_h = (u8)((sec->vd >>(32-15)) & 0x7f);

    GCDi_SetPngIntf( slot, pnA_l, pnA_h, default_pnB_l, default_pnB_h );
}

#endif // FIRM_FORCE_DEBUGGER

/***************************************************

 ***************************************************/
static void GCDi_SetPngIntf( GCDSlot slot, u32 pnA_l, u8 pnA_h, u32 pnB_l, u8 pnB_h )
{
    REGType32v* cnt1   = GCDi_SelectRegAddr( slot, REG_MCCNT1_ADDR );
    REGType32v* scra_l = GCDi_SelectRegAddr( slot, REG_MCSCRA_L_ADDR );
    REGType8v*  scra_h = GCDi_SelectRegAddr( slot, REG_MCSCRA_H_ADDR );
    REGType32v* scrb_l = GCDi_SelectRegAddr( slot, REG_MCSCRB_L_ADDR );
    REGType8v*  scrb_h = GCDi_SelectRegAddr( slot, REG_MCSCRB_H_ADDR );

    // ＰＮ初期値設定

    GCDi_WaitCtrl( slot );

    *scra_l = pnA_l;
    *scra_h = pnA_h;

    *scrb_l = pnB_l;
    *scrb_h = pnB_h; 

    *cnt1 = REG_MI_MCCNT1_A_SCR_MASK | REG_MI_MCCNT1_A_SE_MASK | REG_MI_MCCNT1_A_DS_MASK | REG_MI_MCCNT1_A_RESB_MASK;

    GCDi_WaitCtrl( slot );

    // データスクランブル設定
    GCDi_Work[slot].ctrls.sSendOnlyOpRegs.scramble |= REG_MI_MCCNT1_A_SE_MASK | REG_MI_MCCNT1_A_DS_MASK;
}


void GCDi_DecryptObjectFile( GCDSlot slot )
{
    if ( GCDi_IsDsApp( slot ) )
    {
    }
}

/***************************************************

 ***************************************************/
static void ReadCardAsync4Secure_1( GCDSlot slot, void *romp )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

    if ( GCDi_SharedWork[slot].nCardID & GCD_ROMID_1TROM_MASK )
    {
        // 3Dメモリ専用設定
        {
            ctrls->gcdRegs->ctrl &= ~REG_MI_MCCNT1_PC_MASK;
            ctrls->gcdRegs->ctrl |= GCD_PAGE_1;
        }
    }
    ctrls->gcdOp.dw = GCDOP_S_OP_RD_SEGMENT     // コマンド設定
                    | (((u64)romp/GCD_SECURE_SEGMENT_SIZE) << GCDOP_S_VC_SHIFT);

    gw->oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );
    SetVA( slot, &ctrls->gcdOp );
    Encrypt2SetTimer4Secure( slot, ctrls );

    if ( (GCDi_SharedWork[slot].nCardID & GCD_ROMID_1TROM_MASK) == 0 )
    {
        ctrls->ltckReq = TRUE;
    }
}

/***************************************************

 ***************************************************/
static void ReadCardAsync4Secure_2( GCDSlot slot )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;
    GCDSecureWork* sec = &GCDi_SecureWork[slot];

    /* CardTimerIntr4Secure(void)のかわり */
    s32 oneShotSize;
    s32 offset;

    // セグメント境界にて
    // 3Dメモリは1セグメントを8ページに分割して読む
    do {
        oneShotSize = gw->oneShotSize;
        offset = sec->segmentOffset + sec->blockOffset;

        GCDi_ReadSecureModeRomCore( slot, gw->ramp + offset, oneShotSize, ctrls );

        gw->restSize -= oneShotSize;
        sec->blockOffset += oneShotSize;
    }
    while( gw->restSize % GCD_SECURE_SEGMENT_SIZE );
}

/***************************************************

 ***************************************************/
u8 GCDi_SegmentIndexTable4Secure[] = 
{
    (0<<6)|(1<<4)|(3<<2)|(2<<0), (0<<6)|(2<<4)|(1<<2)|(3<<0), (0<<6)|(2<<4)|(3<<2)|(1<<0), (0<<6)|(3<<4)|(1<<2)|(2<<0),
    (1<<6)|(0<<4)|(2<<2)|(3<<0), (1<<6)|(2<<4)|(0<<2)|(3<<0), (1<<6)|(3<<4)|(0<<2)|(2<<0), (1<<6)|(3<<4)|(2<<2)|(0<<0),
    (2<<6)|(1<<4)|(0<<2)|(3<<0), (2<<6)|(1<<4)|(3<<2)|(0<<0), (2<<6)|(3<<4)|(0<<2)|(1<<0), (2<<6)|(3<<4)|(1<<2)|(0<<0),
    (3<<6)|(0<<4)|(1<<2)|(2<<0), (3<<6)|(0<<4)|(2<<2)|(1<<0), (3<<6)|(1<<4)|(2<<2)|(0<<0), (3<<6)|(2<<4)|(0<<2)|(1<<0),
};

/*
        DCB     4_0132, 4_0213, 4_0231, 4_0312  ; - 4_0123, 4_0321
        DCB     4_1023, 4_1203, 4_1302, 4_1320  ; - 4_1032, 4_1230
        DCB     4_2103, 4_2130, 4_2301, 4_2310  ; - 4_2013, 4_2031
        DCB     4_3012, 4_3021, 4_3120, 4_3201  ; - 4_3102, 4_3210
*/

void GCDi_ReadCardSegmentAsync4Secure( GCDSlot slot, BOOL twl_ex )
{
  GCDSecureWork* sec = &GCDi_SecureWork[slot];
  s16 *blockOffsetp = &sec->blockOffset;
  s16 *segmentOffsetp = &sec->segmentOffset;
  s16 *tblShiftp = &sec->segmentTblShift;
  s32 diffSegmentNo;
  s32 offset;

    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

//  GCDi_WaitCtrl();

  GCDi_SecureWork[slot].segmentTblp = &(((u8 *)GCDi_SegmentIndexTable4Secure)[GCDi_SecureWork[slot].vd >>28]);  // セグメントテーブル設定
  GCDi_SecureWork[slot].numSecureSegment = GCD_SECURE_AREA_SIZE / GCD_SECURE_SEGMENT_SIZE;  // SECUREセグメント数 1segment 4096

  ctrls->gcdRegs = &ctrls->sReadRomRegs;
  gw->romp = GCD_SECURE_AREA_OFFSET;
  if ( !twl_ex )
  {
    gw->ramp = (void*)GCDi_SecureAreaBuf[slot];
  }
  else
  {
    gw->ramp = (void*)GCDi_Secure2AreaBuf[slot];
  }
  gw->restSize  = sizeof( GCDi_SecureAreaBuf[slot] );
  gw->oneShotSize = GCDi_GetOneShotSizeFromCtrl( ctrls->gcdRegs->ctrl );

  sec->secureSize = gw->restSize;

  do {
    // SECURE領域を超えないように調整
    while ((diffSegmentNo = ((*sec->segmentTblp) >>*tblShiftp) & 0x3) >= sec->numSecureSegment)
      *tblShiftp += 2;

    *segmentOffsetp = (s16)(GCD_SECURE_SEGMENT_SIZE * diffSegmentNo);  // セグメントオフセット セット
    *blockOffsetp = 0;  // ブロックオフセット クリア

    offset = *segmentOffsetp + *blockOffsetp;

    ReadCardAsync4Secure_1( slot, (u8*)gw->romp + offset );
    ReadCardAsync4Secure_2( slot );

    *tblShiftp += 2;
    *tblShiftp &= 0x7;
  }
  while( gw->restSize > 0 );
}


/*---------------------------------------------------------------------------*
  Name:         GCD_SendOnlyCardOpCore

  Description:  send only rom command
                sync version

  Arguments:    ctrls  : rom control

  Returns:      None
 *---------------------------------------------------------------------------*/

#define AddLatency2ToLatency1(param)                                \
    ( (((param) &  CARD_LATENCY2_CYCLES_MASK)                       \
                >> CARD_LATENCY2_CYCLES_SHIFT)                      \
    +  ((param) & ~CARD_LATENCY2_CYCLES_MASK)                       \
    )


static void Encrypt2SetTimer4Secure( GCDSlot slot, GCDRomCtrls *ctrls )
{
  GCDSecureWork* sec = &GCDi_SecureWork[slot];
  GCDSharedWork* sh = &GCDi_SharedWork[slot];

  SetVB( slot, &ctrls->gcdOp ); // VB セット

  //GCDi_SecureWork.paramBak = *paramp; // 暗号化前のバップアップ
  // my_memcopy((void *)paramp, (void *)&(GCDi_SecureWork.paramBak),sizeof(CardCtrlParam)); // 暗号化前のバップアップ

  // コマンド暗号
  EncryptByBlowfish( &sec->blowfishCardTable, &((u32*)&ctrls->gcdOp)[1], (u32*)&ctrls->gcdOp );

  // 3Dメモリ専用プリコマンド発行
  if ( sh->nCardID & GCD_ROMID_1TROM_MASK ) {
    GCDCtrlRegs *preRegs = ctrls->gcdRegs;
    ctrls->gcdRegs = &ctrls->sSendOnlyOpRegs;
    GCD_SendOnlyCardOpCore( slot, ctrls );
    ctrls->gcdRegs = preRegs;

    OS_SpinWait( OS_MSEC_TO_CPUCYC(30) );  /* wait 27 milli sec. for 3D-MEMROY? */
  }
}


static void GCDi_SendOnlyCardSecureOpCore( GCDSlot slot, GCDRomCtrls *ctrls )
{
  GCDCtrlRegs *regs = ctrls->gcdRegs;

  // レイテンシ２をレイテンシ１へ加算
#if 0
  paramp->cardCnt = AddLatency2ToLatency1(GetCardCnt4Secure())
    | CARD_READ_MODE | CARD_0_PAGE
    | CARD_START | CARD_RESET_HI;
#endif

  SetDummyVC( slot, &ctrls->gcdOp );
  Encrypt2SetTimer4Secure( slot, ctrls );

  if ( (GCDi_SharedWork[slot].nCardID & GCD_ROMID_1TROM_MASK) == 0 ) {
      ctrls->gcdRegs->ctrl |= GCD_LTCK_ENABLE; //  マスクROM専用設定： レイテンシ期間にクロック供給
//      ctrls->ltckReq = TRUE;
  }
  GCD_SendOnlyCardOpCore( slot, ctrls );
}


/***************************************************

 ***************************************************/
void GCDi_ChangeIntoSecureMode( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->nSendOnlyOpRegs;
    ctrls->gcdOp.dw = GCDOP_N_OP_CHG_MODE;      // コマンド設定
    SetVAE( slot, &ctrls->gcdOp );
    SetVBI( slot, &ctrls->gcdOp );

    GCD_SendOnlyCardOpCore( slot, ctrls );
}

void GCDi_ChangeIntoSecure2Mode( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->nSendOnlyOpRegs;
    ctrls->gcdOp.dw = GCDOP_N_OP_CHG2_MODE;      // コマンド設定
    SetVAE( slot, &ctrls->gcdOp );
    SetVBI( slot, &ctrls->gcdOp );

    GCD_SendOnlyCardOpCore( slot, ctrls );
}


void GCDi_SendPngON( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->sSendOnlyOpRegs;
    ctrls->gcdOp.dw = GCDOP_S_OP_PNG_ON;        // コマンド設定
    SetVD( slot, &ctrls->gcdOp );

    GCDi_SendOnlyCardSecureOpCore( slot, ctrls );
}

void GCDi_SendPngOFF( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->sSendOnlyOpRegs;
    ctrls->gcdOp.dw = GCDOP_S_OP_PNG_OFF;       // コマンド設定
    SetVA( slot, &ctrls->gcdOp );

    GCDi_SendOnlyCardSecureOpCore( slot, ctrls );
}

void GCDi_ChangeIntoGameMode( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->sSendOnlyOpRegs;
    ctrls->gcdOp.dw = GCDOP_S_OP_CHG_MODE;      // コマンド設定
    SetVA( slot, &ctrls->gcdOp );

    GCDi_SendOnlyCardSecureOpCore( slot, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_SendOnlyCardNormalOpCore

  Description:  send only rom command
                sync version

  Arguments:    ctrls  : rom control

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCD_SendOnlyCardNormalOpCore( GCDSlot slot, GCDRomCtrls *ctrls )
{
	ctrls->gcdRegs = &ctrls->nSendOnlyOpRegs;

	GCD_SendOnlyCardOpCore( slot, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCD_SendOnlyCardSecureOpCore

  Description:  send only rom command
                sync version

  Arguments:    ctrls  : rom control

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCD_SendOnlyCardSecureOpCore( GCDSlot slot, GCDRomCtrls *ctrls )
{
	ctrls->gcdRegs = &ctrls->sSendOnlyOpRegs;

    if ( ctrls->ltckReq )
    {
        ctrls->gcdRegs->ctrl |= GCD_LTCK_ENABLE; //  マスクROM専用設定： レイテンシ期間にクロック供給
        ctrls->ltckReq = FALSE;
    }

	GCD_SendOnlyCardOpCore( slot, ctrls );
}

/***************************************************

 ***************************************************/
static u32 GCDi_ReadSecureModeIDCore( GCDSlot slot, GCDRomCtrls *ctrls )
{
	ctrls->gcdRegs = &ctrls->sIDRegs;

    if ( ctrls->ltckReq )
    {
        ctrls->gcdRegs->ctrl |= GCD_LTCK_ENABLE; //  マスクROM専用設定： レイテンシ期間にクロック供給
        ctrls->ltckReq = FALSE;
    }

    return GCDi_ReadRomIDCore( slot, ctrls );
}

static void GCDi_ReadSecureModeRomCore( GCDSlot slot, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
	ctrls->gcdRegs = &ctrls->sReadRomRegs;

    if ( ctrls->ltckReq )
    {
        ctrls->gcdRegs->ctrl |= GCD_LTCK_ENABLE; //  マスクROM専用設定： レイテンシ期間にクロック供給
        ctrls->ltckReq = FALSE;
    }

    GCDi_ReadRomCore( slot, ramp, size, ctrls );
}


/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadSecureModeID

  Description:  read rom ID on secure mode
                sync version

  Arguments:    None

  Returns:      rom ID
 *---------------------------------------------------------------------------*/
u32 GCDi_ReadSecureModeID( GCDSlot slot )
{
    GCDSharedWork* sh = &GCDi_SharedWork[slot];

    if ( GCDi_IsSecureInitialized( slot ) == FALSE )
    {
        GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

        ctrls->gcdRegs = &ctrls->sIDRegs;
        ctrls->gcdOp.dw = GCDOP_S_OP_RD_ROM_ID;     // コマンド設定
        SetVA( slot, &ctrls->gcdOp );
        SetDummyVC( slot, &ctrls->gcdOp );
        Encrypt2SetTimer4Secure( slot, ctrls );

        if ( (sh->nCardID & GCD_ROMID_1TROM_MASK) == 0 ) {
            ctrls->ltckReq = TRUE;
        }
        sh->sCardID = GCDi_ReadSecureModeIDCore( slot, ctrls );
        DBG_PRINTF("%s %d CardID Secure 0x%08x\n",__FUNCTION__,__LINE__, sh->sCardID);
    }

    return  sh->sCardID;
}

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadNormalModeID

  Description:  read rom ID on normal mode
                sync version

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 GCD_ReadNormalModeID( GCDSlot slot )
{
    GCDSharedWork* sh = &GCDi_SharedWork[slot];

    if ( GCDi_IsSecureInitialized( slot ) == FALSE )
    {
        GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

        ctrls->gcdRegs = &ctrls->nIDRegs;
        ctrls->gcdOp.dw = GCDOP_N_OP_RD_ROM_ID;		// コマンド設定

        return  GCDi_ReadRomIDCore( slot, ctrls );
    } else {
        return  sh->nCardID;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GCDi_LoadHeader

  Description:  load header

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_LoadHeader( GCDSlot slot )
{
    if ( GCDi_IsSecureInitialized( slot ) == FALSE )
    {
        GCDSharedWork* sh = &GCDi_SharedWork[slot];

        // load table from dev flash into ASIC
        GCDi_LoadTable( slot );

        // read ROM header
        GCD_ReadNormalModeRom( slot, 0, GCDi_HeaderBuf[slot], sizeof( GCDi_HeaderBuf[slot] )  );

        if ( ! IsDsApp( slot ) )
        {
            MI_CpuFillFast( GCDi_HeaderBuf[slot], 0,  sizeof( GCDi_HeaderBuf[slot] ) );
        }

        // read ROM-ID
        sh->nCardID = GCD_ReadNormalModeID( slot );
        DBG_PRINTF("%s %d CardID Normal 0x%08x\n",__FUNCTION__,__LINE__, sh->nCardID);
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_LoadHeaderAll

  Description:  load header

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_LoadHeaderAll( void )
{
    GCDSlot slot, start, end;

    GCDi_GetExistingSlots( &start, &end );

    if ( GCDi_IsSecureInitialized( GCD_PRIMARY_SLOT ) == FALSE )
    {
        for (slot=start; slot<=end; slot++)
        {
            // load table from dev flash into ASIC
            GCDi_LoadTableAsync( slot );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCD_WaitRomAsync( slot );

            // read short ROM header
            GCDi_ReadShortHeaderAsync( slot, GCDi_HeaderBuf[slot] );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCD_WaitRomAsync( slot );

            if ( ! IsDsApp( slot ) )
            {
                MI_CpuFillFast( GCDi_HeaderBuf[slot], 0,  sizeof( GCDi_HeaderBuf[slot] ) );
            }
        }

        for (slot=start; slot<=end; slot++)
        {
            // read ROM-ID
            GCDSharedWork* sh = &GCDi_SharedWork[slot];
            sh->nCardID = GCD_ReadNormalModeID( slot );
            DBG_PRINTF("%s %d CardID Normal 0x%08x\n",__FUNCTION__,__LINE__, sh->nCardID);

            // read ROM header
            GCD_ReadNormalModeRomAsync( slot, 0, GCDi_HeaderBuf[slot], sizeof( GCDi_HeaderBuf[slot] )  );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCD_WaitRomAsync( slot );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_LoadTable

  Description:  load table from dev flash into ASIC
                sync version

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_LoadTable( GCDSlot slot )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->nLoadTableRegs;
    ctrls->gcdOp.dw = GCDOP_N_OP_LD_TABLE;

    GCDi_ReadRomCore( slot, NULL, 0, ctrls );
}

static void GCDi_LoadTableCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls )
{
#pragma unused( romp )

    GCDi_ReadRomCore( slot, ramp, size, ctrls );
}

static void GCDi_LoadTableAsync( GCDSlot slot )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

    ctrls->gcdRegs = &ctrls->nLoadTableRegs;
    ctrls->gcdOp.dw = GCDOP_N_OP_LD_TABLE;

    gw->funcp = GCDi_LoadTableCore;

    GCD_SetInterrupt( slot, 0, GCDi_Secure2AreaBuf[slot], GCD_LOAD_TABLE_SIZE, gw );

    gw->funcp( slot, gw->romp, gw->ramp, gw->oneShotSize, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadShortHeader

  Description:  load table from dev flash into ASIC
                sync version

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_ReadShortHeader( GCDSlot slot, void *ramp )
{
    GCDRomCtrls *ctrls = &GCDi_Work[slot].ctrls;

    ctrls->gcdRegs = &ctrls->nReadShortHeaderRegs;

    GCDi_ReadNormalModeRomCore( slot, 0, ramp, GCD_ROM_PAGE_SIZE, ctrls );
}

static void GCDi_ReadShortHeaderAsync( GCDSlot slot, void *ramp )
{
    GCDWork *gw = &GCDi_Work[slot];
    GCDRomCtrls *ctrls = &gw->ctrls;

    ctrls->gcdRegs = &ctrls->nReadShortHeaderRegs;

    gw->funcp = GCDi_ReadNormalModeRomCore;

    GCD_SetInterrupt( slot, 0, ramp, GCD_ROM_PAGE_SIZE, gw );

    gw->funcp( slot, gw->romp, gw->ramp, gw->oneShotSize, ctrls );
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_LoadSecure

  Description:  load secure area

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void GCDi_LoadSecureCore( GCDSlot slot, BOOL twl_ex )
{
    if ( GCDi_IsSecureInitialized( slot ) == FALSE )
    {
        GCDRomHeader *rh = GCDi_GetRomHeaderAddr( slot );

#ifndef FIRM_FORCE_DEBUGGER

        GCDi_InitSecureParam( slot, twl_ex );

#endif // FIRM_FORCE_DEBUGGER

        if ( !twl_ex )
        {
            GCDi_ChangeIntoSecureMode( slot );
        }
        else
        {
            GCDi_ChangeIntoSecure2Mode( slot );
        }

#ifdef FIRM_FORCE_DEBUGGER

        if ( !twl_ex )
        {
            GCD_ReadNormalModeRom( slot, GCD_SECURE_AREA_OFFSET, GCDi_SecureAreaBuf[slot], sizeof( GCDi_SecureAreaBuf[slot] )  );
        }
        else if ( rh->l.twlAreaOffset )
        {
            GCD_ReadNormalModeRom( slot, (u32)rh->l.twlAreaOffset * GCD_TWL_AREA_ALIGN + GCD_SECURE2_AREA_OFFSET, 
                                                                 GCDi_Secure2AreaBuf[slot], sizeof( GCDi_Secure2AreaBuf[slot] )  );
        }

#else // FIRM_FORCE_DEBUGGER

        GCDi_SendPngON( slot );
        GCDi_InitPngIntf( slot );

        GCDi_ReadSecureModeID( slot );

        GCDi_ReadCardSegmentAsync4Secure( slot, twl_ex );

#ifndef GCD_ENABLE_SCRAMBLE
        GCDi_SendPngOFF( slot );
#endif // GCD_ENABLE_SCRAMBLE

#endif // FIRM_FORCE_DEBUGGER

        GCDi_ChangeIntoGameMode( slot );

        // 暗号化オブジェクトの復号
        if ( !twl_ex )
        {
#ifndef FIRM_FORCE_FIRMBLOWFISH
            GCDi_DecryptObjectFileDS( slot );
#endif // FIRM_FORCE_FIRMBLOWFISH
        }

        if ( twl_ex || !rh->l.twlAreaOffset )
        {
            GCDi_SecureInitialized[slot] = TRUE;
        }
    }
}

static void GCDi_LoadSecure( GCDSlot slot )
{
    GCDi_LoadSecureCore( slot, FALSE );

}

static void GCDi_LoadSecure2( GCDSlot slot )
{
    GCDi_LoadSecureCore( slot, TRUE );

}

static void GCDi_LoadSecureAllCore( BOOL twl_ex )
{
    GCDSlot slot, start, end;

    GCDi_GetExistingSlots( &start, &end );

    if ( GCDi_IsSecureInitialized( GCD_PRIMARY_SLOT ) == FALSE )
    {

#ifndef FIRM_FORCE_DEBUGGER

        for (slot=start; slot<=end; slot++)
        {
            GCDi_InitSecureParam( slot, twl_ex );
        }

#endif // FIRM_FORCE_DEBUGGER

        if ( !twl_ex )
        {
            for (slot=start; slot<=end; slot++)
            {
                GCDi_ChangeIntoSecureMode( slot );
            }
        }
        else
        {
            for (slot=start; slot<=end; slot++)
            {
                GCDi_ChangeIntoSecure2Mode( slot );
            }
        }

#ifdef FIRM_FORCE_DEBUGGER

        for (slot=start; slot<=end; slot++)
        {
            GCDRomHeader  *rh = GCDi_GetRomHeaderAddr( slot );
            GCD_ReadNormalModeRomAsync( slot, GCD_SECURE_AREA_OFFSET, GCDi_SecureAreaBuf[slot], sizeof( GCDi_SecureAreaBuf[slot] )  );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCDRomHeader  *rh = GCDi_GetRomHeaderAddr( slot );
            GCD_WaitRomAsync( slot );

            if ( rh->l.twlAreaOffset )
            {
                GCD_ReadNormalModeRomAsync( slot, (u32)rh->l.twlAreaOffset * GCD_TWL_AREA_ALIGN + GCD_SECURE2_AREA_OFFSET, 
                                                           GCDi_Secure2AreaBuf[slot], sizeof( GCDi_Secure2AreaBuf[slot] )  );
            }
        }

        for (slot=start; slot<=end; slot++)
        {
            GCD_WaitRomAsync( slot );
        }

#else // FIRM_FORCE_DEBUGGER

        for (slot=start; slot<=end; slot++)
        {
            GCDi_SendPngON( slot );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCDi_InitPngIntf( slot );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCDi_ReadSecureModeID( slot );
        }

        for (slot=start; slot<=end; slot++)
        {
            GCDi_ReadCardSegmentAsync4Secure( slot, twl_ex );
        }

#ifndef GCD_ENABLE_SCRAMBLE
        for (slot=start; slot<=end; slot++)
        {
            GCDi_SendPngOFF( slot );
        }
#endif // GCD_ENABLE_SCRAMBLE

#endif // FIRM_FORCE_DEBUGGER

        for (slot=start; slot<=end; slot++)
        {
            GCDi_ChangeIntoGameMode( slot );
        }

        // 暗号化オブジェクトの復号
        for (slot=start; slot<=end; slot++)
        {
            GCDRomHeader  *rh = GCDi_GetRomHeaderAddr( slot );

            if ( !twl_ex )
            {
#ifndef FIRM_FORCE_FIRMBLOWFISH
                GCDi_DecryptObjectFileDS( slot );
#endif // FIRM_FORCE_FIRMBLOWFISH
            }

            if ( twl_ex || !rh->l.twlAreaOffset )
            {
                GCDi_SecureInitialized[slot] = TRUE;
            }
        }
    }
}

static void GCDi_LoadSecureAll( void )
{
    GCDi_LoadSecureAllCore( FALSE );

}

static void GCDi_LoadSecure2All( void )
{
    GCDi_LoadSecureAllCore( TRUE );

}

//
BOOL GCDi_IsSecureInitialized( GCDSlot slot )
{
    return GCDi_SecureInitialized[slot];
}

