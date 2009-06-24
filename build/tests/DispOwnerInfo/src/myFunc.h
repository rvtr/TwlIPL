/********************************************************************/
/*      myFunc.h                                                    */
/*          NITRO-IPL                                               */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	標準サブルーチン群　ヘッダ


	$Log: myFunc.h,v $
	Revision 1.1.4.1  2006/02/01 06:29:33  yosiokat
	mf_strcpyの仕様変更。
	
	Revision 1.1  2005/02/08 04:41:23  yosiokat
	新規追加。
	
	Revision 1.2  2004/05/19 08:25:28  yosiokat
	更新ログの追加。
	

*/

#ifndef	__MY_FUNC_H__
#define	__MY_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>
#include "data.h"

// define data ---------------------------------

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


// const data-----------------------------------
extern const u16 myChar[0x2800*8/16];			// キャラクターデータ


// function-------------------------------------
extern void mf_init(void);
extern void mf_KEYPAD_read(void);
extern void mf_KEYPAD_initRapid(void);
extern void mf_KEYPAD_rapid(void);
extern void mf_drawDecimal(u16 pos_x,u16 pos_y,u16 color,const void *valuep,u8 drawLength,u8 size);
extern void mf_drawHex(u16 pos_x,u16 pos_y,u16 color,const void *valuep,u8 drawLength);
extern void mf_drawString(u16 pos_x,u16 pos_y,u16 color,const u8 *strp);
extern void mf_drawString2(u16 pos_x,u16 pos_y,u16 color,const u16 *strp);
extern void mf_clearRect(u16 pos_x,u16 pos_y,u8 height,u8 width);
extern void mf_CSR_init(u16 pos_x,u16 pos_y,u16 add_y);
extern void mf_CSR_moveAndAnime(int nowNum);
extern void mf_CSR_anime(const u16 *csr_charListp);
extern void mf_BLINK_initCounter(void);
extern void mf_BLINK_drawString(u16 pos_x,u16 pos_y,u16 color,const u8 *strp);
extern void mf_waitXframe(u16 frame);
extern void mf_TIME_init(void);
extern void mf_TIME_start(int init_flag);
extern void mf_TIME_stop(void);
extern void mf_TIME_count(void);
extern void mf_TIME_draw(u16 pos_x,u16 pos_y,u16 color);
extern u8*  mf_strcpy(const u8 *str1p,u8 *str2p);
extern u8   mf_strcmp(const u8 *str1p,const u8 *str2p);


#ifdef __cplusplus

#endif

#endif		// __MY_FUNC_H__

