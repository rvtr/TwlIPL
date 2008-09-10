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
#include "romSpec.h"

#ifdef __cplusplus
extern "C" {
#endif


static inline void HOTSW_WaitCardCtrl(void)
{
	while( reg_MI_MCCNT1_A & REG_MI_MCCNT1_START_MASK ){}
}

// Define -------------------------------------------------------------------
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
#define	HOTSW_POLLING_CTRL_BUFFER_NUM		4

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

#define SCRAMBLE_MASK                       0x1040e000 // スクランブル関係のフラグマスク
#define SECURE_COMMAND_SCRAMBLE_MASK		0x00406000 // CS SE DSのマスク

// --- Page Count
#define PAGE_0     			  				0x0UL << PC_SHIFT
#define PAGE_1     			  				0x1UL << PC_SHIFT
#define PAGE_2    	 		  				0x2UL << PC_SHIFT
#define PAGE_4     			  				0x3UL << PC_SHIFT
#define PAGE_8     			  				0x4UL << PC_SHIFT
#define PAGE_16    			  				0x5UL << PC_SHIFT
#define PAGE_32    			  				0x6UL << PC_SHIFT
#define PAGE_STAT				  			0x7UL << PC_SHIFT


// union ---------------------------------------------------------------------
typedef union
{
    u64             dw;
    u8              b[8];
} Cmd64;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __HOTSW_TYPES_H__