/*---------------------------------------------------------------------------*
  Project:  TwlFirm - GCD - include
  File:     ngcd.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef FIRM_GCD_GCD_MISC_H_
#define FIRM_GCD_GCD_MISC_H_

#include <firm/format/gcdfirm.h>
#include "./gcd.h"


#ifdef __cplusplus
extern "C" {
#endif

#define PXI_FIFO_TAG_GCD	PXI_FIFO_TAG_USER_1

#define reg_MI_MC_DET									   (*(REGType8v *) REG_MC_OFFSET)
#define reg_MI_MC_SWP								       (*(REGType8v *) ( REG_MC_OFFSET + 1 ) )

#define REG_MCCHAT_OFFSET                                  0x4012
#define REG_MCCHAT_ADDR                                    (HW_REG_BASE + REG_MCCHAT_OFFSET)
#define reg_MI_MCCHAT                                      (*( REGType16v *) REG_MCCHAT_ADDR)

/* MCSCRA_L */

#define REG_MCSCRA_L_OFFSET                                0x1b0
#define REG_MCSCRA_L_ADDR                                  (HW_REG_BASE + REG_MCSCRA_L_OFFSET)
#define reg_MI_MCSCRA_L                                    (*( REGType32v *) REG_MCSCRA_L_ADDR)

/* MCSCRB_L */

#define REG_MCSCRB_L_OFFSET                                0x1b4
#define REG_MCSCRB_L_ADDR                                  (HW_REG_BASE + REG_MCSCRB_L_OFFSET)
#define reg_MI_MCSCRB_L                                    (*( REGType32v *) REG_MCSCRB_L_ADDR)

/* MCSCRA_H */

#define REG_MCSCRA_H_OFFSET                                0x1b8
#define REG_MCSCRA_H_ADDR                                  (HW_REG_BASE + REG_MCSCRA_H_OFFSET)
#define reg_MI_MCSCRA_H                                    (*( REGType8v *) REG_MCSCRA_H_ADDR)

/* MCSCRB_H */

#define REG_MCSCRB_H_OFFSET                                0x1ba
#define REG_MCSCRB_H_ADDR                                  (HW_REG_BASE + REG_MCSCRB_H_OFFSET)
#define reg_MI_MCSCRB_H                                    (*( REGType8v *) REG_MCSCRB_H_ADDR)


#define REG_MI_MCCNT1_A_CSC_SHIFT                          22
#define REG_MI_MCCNT1_A_CSC_SIZE                           1
#define REG_MI_MCCNT1_A_CSC_MASK                           0x00400000

#define REG_MI_MC_SWP_E_SHIFT                              7
#define REG_MI_MC_SWP_E_SIZE                               1
#define REG_MI_MC_SWP_E_MASK                               0x80



typedef enum
{
    GCD_PAGE_0             = 0x0UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_1             = 0x1UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_2             = 0x2UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_4             = 0x3UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_8             = 0x4UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_16            = 0x5UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_32            = 0x6UL << REG_MI_MCCNT1_A_PC_SHIFT,
    GCD_PAGE_STAT          = 0x7UL << REG_MI_MCCNT1_A_PC_SHIFT
}
GCDPageCount;

typedef enum
{
    GCD_CKT_150NS          = 0x0UL << REG_MI_MCCNT1_A_CT_SHIFT,
    GCD_CKT_240NS          = 0x1UL << REG_MI_MCCNT1_A_CT_SHIFT
}
GCDClockType;

typedef enum
{
    GCD_RW_READ            = 0x0UL << REG_MI_MCCNT1_WR_SHIFT,
    GCD_RW_WRITE           = 0x1UL << REG_MI_MCCNT1_WR_SHIFT
}
GCDRw;

typedef enum
{
    GCD_RESET_LO           = 0x0UL << REG_MI_MCCNT1_A_RESB_SHIFT,
    GCD_RESET_HI           = 0x1UL << REG_MI_MCCNT1_A_RESB_SHIFT
}
GCDReset;

typedef enum
{
#ifdef SDK_ARM9
    GCD_LTCK_DISABLE       = 0x0UL << REG_MI_MCCNT1_A_TRM_SHIFT,
    GCD_LTCK_ENABLE        = 0x1UL << REG_MI_MCCNT1_A_TRM_SHIFT
#else
    GCD_LTCK_DISABLE       = 0x0UL << REG_MI_MCCNT1_A_RTM_SHIFT,
    GCD_LTCK_ENABLE        = 0x1UL << REG_MI_MCCNT1_A_RTM_SHIFT
#endif
}
GCDLtClkEnable;

typedef struct
{
    u32 ctrl;
    u8  master;
    u8  spi;
}
NGCDCtrlRegs;

// PXI�ł̒ʐM�v���g�R���֘A��`
#define GCD_PXI_COMMAND_MASK                0x0000003f  // �J�n���[�h�̃R�}���h��
#define GCD_PXI_COMMAND_SHIFT               0
#define GCD_PXI_COMMAND_SIZE                6
#define GCD_PXI_COMMAND_PARAM_MASK          0x03ffffc0  // �J�n���[�h�̃p�����[�^��
#define GCD_PXI_COMMAND_PARAM_SHIFT         6
#define GCD_PXI_COMMAND_PARAM_SIZE          20

typedef union
{
    struct
    {
        u32 cmd:GCD_PXI_COMMAND_SIZE;
        u32 param:GCD_PXI_COMMAND_PARAM_SIZE-1;
        u32 slot:1;
    }
    e;
    u32 raw;
}
GCDPxiCmd;

// PXI�o�R�Ŕ��s����閽��
#define GCD_PXI_COMMAND_RESET               0x01
#define GCD_PXI_COMMAND_LOADED              0x02


/*---------------------------------------------------------------------------*
  Name:         GCD_Init

  Description:  initialize for game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_Init( void );

/*---------------------------------------------------------------------------*
  Name:         GCDi_Init

  Description:  initialize for game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Init( BOOL reset );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRomID

  Description:  read rom ID
                sync version

  Arguments:    None

  Returns:      rom ID
 *---------------------------------------------------------------------------*/
u32 GCD_ReadRomID( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRom

  Description:  read rom data
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRomAsync

  Description:  read rom data
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadNormalModeRom

  Description:  read rom data on normal mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadNormalModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadGameModeRom

  Description:  read rom data on game mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadGameModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadRomPreCore

  Description:  read rom data
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL GCD_ReadRomPreCore( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadGameModeRomAsync

  Description:  read rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadGameModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_WriteGameModeRom

  Description:  write rom data on game mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_WriteGameModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_WriteGameModeRomAsync

  Description:  write rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_WriteGameModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_Enable

  Description:  Enable game card master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Enable( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_Disable

  Description:  disable game card master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Disable( GCDSlot slot );

//================================================================================
//       WAIT/STOP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         GCD_IsExisting

  Description:  get whether cartridge exists

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
static inline BOOL GCD_IsExisting( GCDSlot slot )
{
    s32 ofs = (GCD_GetPrimarySlot() ^ slot) * 4;
    s32 r = ~reg_MI_MC_DET & (REG_MI_MC_SL1_CDET_MASK << ofs);
    return r >> (REG_MI_MC_SL1_CDET_SHIFT + ofs);
}

/*---------------------------------------------------------------------------*
  Name:         GCD_SetDetectMode

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void GCD_SetDetectMode( GCDSlot slot, u32 mode )
{
    s32 ofs = (GCD_GetPrimarySlot() ^ slot) * 4;
    s32 others = reg_MI_MC_DET & ~(REG_MI_MC_SL1_MODE_MASK << ofs);
    reg_MI_MC_DET = (u8)((mode << (REG_MI_MC_SL1_MODE_SHIFT + ofs)) | others);
}

/*---------------------------------------------------------------------------*
  Name:         GCD_GetDetectMode

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline u8 GCD_GetDetectMode( GCDSlot slot )
{
    s32 ofs = (GCD_GetPrimarySlot() ^ slot) * 4;
    return  (u8)((reg_MI_MC_DET & (REG_MI_MC_SL1_MODE_MASK << ofs))
                               >> (REG_MI_MC_SL1_MODE_SHIFT + ofs));
}

/*---------------------------------------------------------------------------*
  Name:         GCD_SetChatCounter

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void GCD_SetChatCounter( u16 value )
{
    reg_MI_MCCHAT = value;
}

/*---------------------------------------------------------------------------*
  Name:         GCD_IsBusy

  Description:  check whether game card is busy or not

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL GCD_IsBusy( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCD_IsDataReady

  Description:  check whether data is ready or not

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL GCD_IsDataReady( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCD_Stop

  Description:  stop game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_Stop( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCD_WaitRomAsync

  Description:  wait for game card async access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_WaitRomAsync( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_SelectRegAddr

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void* GCDi_SelectRegAddr( GCDSlot slot, u32 base )
{
    u32 addr = base;

    if ( slot == GCD_SECONDARY_SLOT )
    {
        addr += REG_MCCNT0_B_ADDR - REG_MCCNT0_ADDR;
    }

    return (void*)addr;
}

/*---------------------------------------------------------------------------*
  Name:         GCDi_SelectIrqMask

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline u32 GCDi_SelectIrqMask( GCDSlot slot, u32 base_mask )
{
    u32 mask = base_mask;

    if ( slot == GCD_SECONDARY_SLOT )
    {
        switch ( mask )
        {
            case OS_IE_CARD_DATA:
                mask = OS_IE_CARD_B_DATA;
                break;
            case OS_IE_CARD_IREQ:
                mask = OS_IE_CARD_B_IREQ;
                break;
            case OS_IE_CARD_A_DET:
                mask = OS_IE_CARD_B_DET;
                break;
        }
    }

    return mask;
}


// internal

u32 GCD_ReadNormalModeID( GCDSlot slot );
u32 GCD_ReadSecureModeID( GCDSlot slot );
u32 GCD_ReadGameModeID( GCDSlot slot );

void GCDi_SetOp( GCDSlot slot, GCDCmd64* op );
void GCDi_SetCtrl( GCDSlot slot, GCDCtrlRegs* regs );
void GCDi_GenCtrl( GCDCtrlRegs* regs,
                        GCDRw rw,
                        GCDPageCount pcount,
                        GCDClockType ckt, u32 lt1, u32 lt2,
                        BOOL cpn, BOOL dpn, BOOL csc, BOOL dsc );
s32 GCDi_GetOneShotSizeFromCtrl( u32 ctrl );
void GCD_SendOnlyCardOpCore( GCDSlot slot, GCDRomCtrls *ctrls );
u32 GCDi_ReadRomIDCore( GCDSlot slot, GCDRomCtrls *ctrls );
void GCD_ReadRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls );
void GCD_WaitRomAsyncCore( GCDSlot slot );

void GCDi_WaitCtrl( GCDSlot slot );
void GCDi_WaitData( GCDSlot slot );
void GCDi_WaitDma( GCDSlot slot, u32 dmaNo );
void GCDi_WaitInterrupt( GCDSlot slot );

void GCDi_SendtoPxi(u32 data);

#ifdef __cplusplus
} /* extern "C" */

#endif

/* FIRM_GCD_GCD_MISC_H_ */
#endif
