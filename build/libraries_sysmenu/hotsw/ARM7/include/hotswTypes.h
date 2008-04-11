/*---------------------------------------------------------------------------*
  Project:  TwlIPL - HOTSW - include
  File:     type.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

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
#include <firm/gcd/blowfish.h>
#include "romSpec.h"

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

// --- NewDMA Ch
#define HOTSW_NDMA_NO                       2

// --- Blowfish
#define KEY_BUF_SIZE                        3

// --- Boot Segment
#define BOOT_SEGMENT_SIZE                   0x1000

// --- Secure Segment
#define SECURE_SEGMENT_START				0x4000
#define SECURE_SEGMENT_SIZE                 0x4000
#define SECURE_SEGMENT_END	   				(SECURE_SEGMENT_START + SECURE_SEGMENT_SIZE)

// --- TWL Card
#define TWLCARD_BORDER_OFFSET				0x80000

// --- Page / Segment (Byte)
#define PAGE_SIZE                           0x200
#define ONE_SEGMENT_SIZE                    0x1000

// --- Page / Segment (Word)
#define PAGE_WORD_SIZE                      0x80
#define ONE_SEGMENT_WORD_SIZE               0x400

// --- Rom Emulation
#define ROM_EMULATION_DATA_SIZE             0x20

// --- Thread
#define	HOTSW_THREAD_STACK_SIZE				(1024 * 2)
#define	HOTSW_DMA_MSG_NUM					8
#define	HOTSW_INSERT_MSG_NUM				16
#define	HOTSW_PULLED_MSG_NUM				16
#define	HOTSW_CTRL_MSG_NUM					8
#define HOTSW_MSG_BUFFER_NUM				(HOTSW_INSERT_MSG_NUM + HOTSW_PULLED_MSG_NUM + HOTSW_CTRL_MSG_NUM)

// --- Card Control Reg 0
#define HOTSW_E2PROM_CTRL_MASK		  		0x00ff

// --- Card Control Reg 1
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

#define SCRAMBLE_MASK                       0x1840e000 // スクランブル関係のフラグマスク
#define SECURE_COMMAND_SCRAMBLE_MASK		0x00406000 // CS SE DSのマスク

// --- Page Count
#define HOTSW_PAGE_0     			  		0x0UL << PC_SHIFT
#define HOTSW_PAGE_1     			  		0x1UL << PC_SHIFT
#define HOTSW_PAGE_2    	 		  		0x2UL << PC_SHIFT
#define HOTSW_PAGE_4     			  		0x3UL << PC_SHIFT
#define HOTSW_PAGE_8     			  		0x4UL << PC_SHIFT
#define HOTSW_PAGE_16    			  		0x5UL << PC_SHIFT
#define HOTSW_PAGE_32    			  		0x6UL << PC_SHIFT
#define HOTSW_PAGE_STAT				  		0x7UL << PC_SHIFT

// --- Latency
#define AddLatency2ToLatency1(param)\
    ( (((param) &  LATENCY2_MASK)   \
                >> LATENCY2_SHIFT)  \
    +  ((param) &  LATENCY1_MASK)   \
    )

// --- Register Define
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

typedef enum RomMode{
    HOTSW_ROM_MODE_NULL = 0,
    HOTSW_ROM_MODE_NORMAL,
    HOTSW_ROM_MODE_SECURE,
    HOTSW_ROM_MODE_GAME,

    HOTSW_ROM_MODE_NUM
} RomMode;

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
    HOTSW_BUFFER_OVERRUN_ERROR,
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
/*typedef struct BLOWFISH_CTX{
    u32         P[16 + 2];
    u32         S[4][256];
} BLOWFISH_CTX;*/

// カードブート時に必要な変数一式をまとめた構造体
typedef struct CardBootData{
    u32                 vae;
    u32                 vbi;
    u32                 vd;

    u32                 id_nml;
    u32                 id_scr;
    u32                 id_scr2;
    u32                 id_gam;

    u32                 arm9Stc;
    u32                 arm7Stc;
    u32                 arm9Ltd;
    u32                 arm7Ltd;

    BOOL                twlFlg;
    BOOL                debuggerFlg;
	BOOL				illegalCardFlg;
    BOOL				isLoadTypeTwl;
    
    u32                 romStatus;

    u32                 keyBuf[KEY_BUF_SIZE];
    u32					keyBuf2[KEY_BUF_SIZE];

    CardTypeEx          cardType;
   	ModeType			modeType;
    u32                 secureLatency;
    u32                 gameCommondParam;

    BootSegmentData     *pBootSegBuf;
    u32                 *pSecureSegBuf;
	u32                 *pSecure2SegBuf;
    
    BLOWFISH_CTX        keyTable;
} CardBootData;

// スレッド・メッセージ関係をまとめた構造体
typedef struct CardThreadData{
    u64  				hotswStack[HOTSW_THREAD_STACK_SIZE / sizeof(u64)];
	u64  				monitorStack[HOTSW_THREAD_STACK_SIZE / sizeof(u64)];
	OSThread 			hotswThread;
	OSThread			monitorThread;

	u32 				idx_insert;
    u32					idx_pulledOut;
    u32					idx_ctrl;
    u32					idx_dma;

   	OSMessage			hotswDmaMsg[HOTSW_DMA_MSG_NUM];
    HotSwMessage		hotswInsertMsg[HOTSW_INSERT_MSG_NUM];
    HotSwMessage		hotswPulledOutMsg[HOTSW_PULLED_MSG_NUM];
	HotSwMessage		hotswPxiMsg[HOTSW_CTRL_MSG_NUM];

    OSMessageQueue   	hotswQueue;
    OSMessageQueue   	hotswDmaQueue;

    OSMessage			hotswMsgBuffer[HOTSW_MSG_BUFFER_NUM];
	OSMessage			hotswDmaMsgBuffer[HOTSW_DMA_MSG_NUM];
} CardThreadData;

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