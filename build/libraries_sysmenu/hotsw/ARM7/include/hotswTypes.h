/*---------------------------------------------------------------------------*
  Project:  TwlFirm - GCD - include
  File:     type.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_TYPES_H__
#define __HOTSW_TYPES_H__

#include <twl.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>

#ifdef __cplusplus
extern "C" {
#endif


// Define -------------------------------------------------------------------
//#define HOTSW_NO_MESSAGE                    // Printf抑制スイッチ

#ifdef  HOTSW_NO_MESSAGE
#ifndef SDK_FINALROM
#define OS_TPrintf( ... )                   ((void)0)
#define OS_PutString( ... )                 ((void)0)
#endif
#endif


#define KEY_BUF_SIZE                        3           // Blowfishキーのバッファサイズ
#define HOTSW_DMA_NO                        2           //
#define BOOT_SEGMENT_SIZE                   0x1000      // Boot Segment領域のサイズ

#define PAGE_SIZE                           0x200       // 1ページのサイズ(バイト単位)
#define PAGE_WORD_SIZE                      0x80        // 1ページのサイズ(ワード単位)

#define ONE_SEGMENT_SIZE                    0x1000      // 1Segmentのサイズ(バイト単位)
#define ONE_SEGMENT_WORD_SIZE               0x400       // 1Segmentのサイズ(ワード単位)

#define SECURE_SEGMENT_SIZE                 0x4000      // Secure領域のサイズ

#define ROM_EMULATION_DATA_SIZE             0x20        // ROMエミュレーションデータサイズ

#define PNA_BASE_VALUE                      0x60e8      //
#define PNB_L_VALUE                         0x879b9b05  //
#define PNB_H_VALUE                         0x5c        //

// コントロールレジスタ１ bit関連
#define START_FLG_MASK                      0x80000000
#define READY_FLG_MASK                      0x00800000

#define LATENCY1_SHIFT                      0
#define LATENCY1_MASK                       0x00001fff

#define DS_SHIFT                            13
#define DS_MASK                             0x00002000

#define SE_SHIFT                            14
#define SE_MASK                             0x00004000

#define SCR_SHIFT                           15
#define SCR_MASK                            0x00008000

#define LATENCY2_SHIFT                      16
#define LATENCY2_MASK                       0x003f0000

#define CS_SHIFT                            22
#define CS_MASK                             0x00400000

#define RDY_SHIFT                           23
#define RDY_MASK                            0x00800000

#define PC_SHIFT                            24
#define PC_MASK                             0x07000000

#define CT_SHIFT                            27
#define CT_MASK                             0x08000000

#define TRM_SHIFT                           28
#define TRM_MASK                            0x10000000

#define RESB_SHIFT                          29
#define RESB_MASK                           0x20000000

#define WR_SHIFT                            30
#define WR_MASK                             0x40000000

#define START_SHIFT                         31
#define START_MASK                          0x80000000

#define SCRAMBLE_MASK                       0x1840e000
#define SECURE_COMMAND_SCRAMBLE_MASK		0x00406000 // CS SE DSのマスク

#define AddLatency2ToLatency1(param)\
    ( (((param) &  LATENCY2_MASK)   \
                >> LATENCY2_SHIFT)  \
    +  ((param) &  LATENCY1_MASK)   \
    )

#ifndef DEBUG_USED_CARD_SLOT_B_
// Slot A
#define     SLOT_STATUS_MODE_SELECT_MSK         0x0c
#define     SLOT_STATUS_CDET_MSK                0x01
#define     SLOT_STATUS_MODE_00                 0x00
#define     SLOT_STATUS_MODE_01                 0x04
#define     SLOT_STATUS_MODE_10                 0x08
#define     SLOT_STATUS_MODE_11                 0x0c

#define     reg_HOTSW_MCCMD0                    reg_MI_MCCMD0_A
#define     reg_HOTSW_MCCMD1                    reg_MI_MCCMD1_A

#define     reg_HOTSW_MCCNT0                    reg_MI_MCCNT0_A
#define     reg_HOTSW_MCCNT1                    reg_MI_MCCNT1_A

#define     HOTSW_MCD1                          REG_MCD1_A_ADDR
#define     reg_HOTSW_MCD1                      reg_MI_MCD1_A

#define     reg_HOTSW_MCSCR0                    reg_MI_MCSCR0_A
#define     reg_HOTSW_MCSCR1                    reg_MI_MCSCR1_A
#define     reg_HOTSW_MCSCR2                    reg_MI_MCSCR2_A

#define     HOTSW_IF_CARD_DET                   OS_IE_CARD_A_DET
#define     HOTSW_IF_CARD_IREQ                  OS_IE_CARD_A_IREQ

#else
// Slot B
#define     SLOT_STATUS_MODE_SELECT_MSK         0xc0
#define     SLOT_STATUS_CDET_MSK                0x10
#define     SLOT_STATUS_MODE_00                 0x00
#define     SLOT_STATUS_MODE_01                 0x40
#define     SLOT_STATUS_MODE_10                 0x80
#define     SLOT_STATUS_MODE_11                 0xc0

#define     reg_HOTSW_MCCMD0                    reg_MI_MCCMD0_B
#define     reg_HOTSW_MCCMD1                    reg_MI_MCCMD1_B

#define     reg_HOTSW_MCCNT0                    reg_MI_MCCNT0_B
#define     reg_HOTSW_MCCNT1                    reg_MI_MCCNT1_B

#define     HOTSW_MCD1                          REG_MCD1_B_ADDR
#define     reg_HOTSW_MCD1                      reg_MI_MCD1_B

#define     reg_HOTSW_MCSCR0                    reg_MI_MCSCR0_B
#define     reg_HOTSW_MCSCR1                    reg_MI_MCSCR1_B
#define     reg_HOTSW_MCSCR2                    reg_MI_MCSCR2_B

#define     HOTSW_IF_CARD_DET                   OS_IE_CARD_B_DET
#define     HOTSW_IF_CARD_IREQ                  OS_IE_CARD_B_IREQ

#endif

// Enum ---------------------------------------------------------------------
typedef enum CardTypeEx{
    DS_CARD_TYPE_1 = 0,
    DS_CARD_TYPE_2,
    TWL_CARD,
    ROM_EMULATION
} CardTypeEx;

typedef enum NormalCommandType{
    RD_ID = 0,
    RD_BSEG,
    CHG_MODE
} NormalCommandType;

typedef enum SecureCommandType{
    S_RD_ID = 0,
    S_RD_SEG,
    S_PNG_ON,
    S_PNG_OFF,
    S_CHG_MODE
} SecureCommandType;

typedef enum GameCommandType{
    G_RD_ID = 0,
    G_RD_PAGE
} GameCommandType;

typedef enum CardType{
    CARD_DS_TYPE1 = 0,
    CARD_DS_TYPE2,
    CARD_TWL
} CardType;

typedef enum HotSwState{
	HOTSW_SUCCESS = 0,
    HOTSW_TIME_OUT,
    HOTSW_CARD_LOCK_ERROR,
    HOTSW_CRC_CHECK_ERROR,
    HOTSW_HASH_CHECK_ERROR,
    HOTSW_ID_CHECK_ERROR,
    HOTSW_PULLED_OUT_ERROR,
    HOTSW_DATA_DECRYPT_ERROR,
    HOTSW_UNEXPECTED_ERROR
} HotSwState;

// union ---------------------------------------------------------------------
typedef union
{
    u64             dw;
    u8              b[8];
} GCDCmd64;

// ブートセグメントデータ
typedef union BootSegmentData
{
    ROM_Header          rh;
    u32                 word[BOOT_SEGMENT_SIZE / sizeof(u32)];
} BootSegmentData;

// struct -------------------------------------------------------------------
typedef struct BLOWFISH_CTX{
    u32         P[16 + 2];
    u32         S[4][256];
} BLOWFISH_CTX;

// カードブート時に必要な変数一式をまとめた構造体
typedef struct CardBootData{
    u16                 lockID;

    u32                 vae;
    u32                 vbi;
    u32                 vd;

    u32                 id_nml;
    u32                 id_scr;
    u32                 id_gam;

    u32                 arm9StcSize;
    u32                 arm7StcSize;
    u32                 arm9LtdSize;
    u32                 arm7LtdSize;

    u32                 arm9Stc;
    u32                 arm7Stc;
    u32                 arm9Ltd;
    u32                 arm7Ltd;

    BOOL                twlFlg;
    BOOL                debuggerFlg;

    u32                 romEmuBuf[ROM_EMULATION_DATA_SIZE/sizeof(u32)];
    u32                 keyBuf[KEY_BUF_SIZE];

    CardTypeEx          cardType;
    u32                 secureLatency;
    u32                 gameCommondParam;

    BootSegmentData     *pBootSegBuf;
    u32                 *pSecureSegBuf;

    BLOWFISH_CTX        keyTable;
} CardBootData;

// カード起動用関数
typedef struct CardBootFunction {
    HotSwState (*ReadBootSegment_N)(CardBootData *cbd);
    HotSwState (*ChangeMode_N)(CardBootData *cbd);

    HotSwState (*ReadID_S)(CardBootData *cbd);
    HotSwState (*ReadSegment_S)(CardBootData *cbd);
    HotSwState (*SetPNG_S)(CardBootData *cbd);
    HotSwState (*ChangeMode_S)(CardBootData *cbd);

    HotSwState (*ReadID_G)(CardBootData *cbd);
    HotSwState (*ReadPage_G)(CardBootData *cbd, u32 addr, void* buf, u32 size);
} CardBootFunction;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_TYPES_H__