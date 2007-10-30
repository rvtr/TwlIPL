/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     misc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	__MISC_H__
#define	__MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <twl.h>
#include "misc.h"

// define data ---------------------------------

// パレットカラー
#define WHITE			(0<<12)
#define RED				(1<<12)
#define	GREEN			(2<<12)
#define	BLUE			(3<<12)
#define	YELLOW			(4<<12)
#define	CYAN			(5<<12)
#define	PURPLE			(6<<12)
#define	LIGHTGREEN		(7<<12)
#define	HIGHLIGHT_Y		(8<<12)
#define	HIGHLIGHT_C		(9<<12)
#define	HIGHLIGHT_W		(10<<12)
#define	HIGHLIGHT_B		(11<<12)

// 時間計測構造体
typedef struct {
  int  enable;
  int  frame;
  int  second;
  int  minute;
  int  hour;
}MyTime;

// キーデータ・ワークエリア構造体
//typedef struct {
//	u16 trg;									// トリガ入力
//	u16 cont;									// ベタ  入力
//}KeyWork;


// global variable------------------------------
extern MyTime		myTime;
//extern KeyWork	pad;
extern int			(*g_pNowProcess)(void);


// const data-----------------------------------
extern const u16 myPlttData[12][16];			// パレットデータ
extern const u16 myChar[0x2800*8/16];			// キャラクターデータ


// function-------------------------------------
extern void mf_init(void);
extern void mf_KEYPAD_read(void);
extern void mf_KEYPAD_initRapid(void);
extern void mf_KEYPAD_rapid(void);
extern void mf_waitXframe(u16 frame);
extern void mf_clearRect(u16 pos_x,u16 pos_y,u8 height,u8 width);
/*
extern void mf_drawUInt(u16 pos_x,u16 pos_y,u16 color,const void *valuep,u8 drawLength,u8 size);
extern void mf_drawHex(u16 pos_x,u16 pos_y,u16 color,const void *valuep,u8 drawLength);
extern void mf_drawString(u16 pos_x,u16 pos_y,u16 color,const u8 *strp);
extern void mf_drawString2(u16 pos_x,u16 pos_y,u16 color,const u16 *strp);
extern void mf_drawRectFrame(u16 pos_x,u16 pos_y,u16 color,u8 height,u8 width);
extern void mf_CSR_init(u16 pos_x,u16 pos_y,u16 add_y);
extern void mf_CSR_moveAndAnime(int nowNum);
extern void mf_CSR_anime(const u16 *csr_charListp);
extern void mf_BLINK_initCounter(void);
extern void mf_BLINK_drawString(u16 pos_x,u16 pos_y,u16 color,const u8 *strp);
extern void mf_TIME_init(void);
extern void mf_TIME_start(int init_flag);
extern void mf_TIME_stop(void);
extern void mf_TIME_count(void);
extern void mf_TIME_draw(u16 pos_x,u16 pos_y,u16 color);
extern void mf_strcpy(const u8 *str1p,u8 *str2p);
extern u8   mf_strcmp(const u8 *str1p,const u8 *str2p);
*/

#ifdef __cplusplus
}
#endif

#endif		// __MISC_H__

