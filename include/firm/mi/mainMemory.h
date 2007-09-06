/*---------------------------------------------------------------------------*
  Project:  TwlFirm - include - MI
  File:     mainMemory.h

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

#ifndef FIRM_MI_MAINMEMORY_H_
#define FIRM_MI_MAINMEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
//                      メインメモリ CRコマンド
//----------------------------------------------------------------------

// for DS-PSRAM

// CR0
#define MMEM_DCR0_PARTIAL_REFRESH_NONE  0x0003      // パーシャルリフレッシュ無し
#define MMEM_DCR0_BURST_CONTINUOUS      0x001c      // 連続バースト（２２４バイト）
#define MMEM_DCR0_BURST_MODE            0x0000      // バーストモード
#define MMEM_DCR0_PAGE_MODE             0x0020      // ページモード
#define MMEM_DCR0_SB1                   0xffc0      // １固定

// CR1
#define MMEM_DCR1_1ST_R4_W3             0x0002      // １ｓｔＲ／Ｗ ＝ ４／３
#define MMEM_DCR1_BURST_WRITE           0x0000      // バーストライト
#define MMEM_DCR1_CLOCK_TRIGGER_UP      0x0100      // クロック立上り
#define MMEM_DCR1_BURST_LINER           0x0200      // バーストリニア・シーケンス
#define MMEM_DCR1_SB1                   0xe430      // １固定

// CR2
#define MMEM_DCR2_SB1                   0x004003fe  // １固定
#define MMEM_DCR2_CLOCK_TRIGGER_UP      0x00000400  // クロック立上り
#define MMEM_DCR2_BURST_WRITE           0x00000000  // バーストライト
#define MMEM_DCR2_BURST_LINER           0x00001000  // バーストリニア・シーケンス
#define MMEM_DCR2_1ST_R4_W3             0x00004000  // １ｓｔＲ／Ｗ ＝ ４／３
#define MMEM_DCR2_BURST_MODE            0x00000000  // バーストモード
#define MMEM_DCR2_PAGE_MODE             0x00010000  // ページモード
#define MMEM_DCR2_BURST_CONTINUOUS      0x000e0000  // 連続バースト（２２４バイト）
#define MMEM_DCR2_PARTIAL_REFRESH_NONE  0x00300000  // パーシャルリフレッシュ無し

// for TWL-PSRAM

#define MMEM_TCR0                       0xFFFF
#define MMEM_TCR1                       0xFFDF
#define MMEM_TCR2                       0xFFEA


/*---------------------------------------------------------------------------*
  Name:         MIi_InitMainMemCR

  Description:  change main memory into the burst mode

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void  MIi_InitMainMemCR( void );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_MI_MAINMEMORY_H_ */
#endif
