/********************************************************************/
/*      main.h                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	メイン定義　ヘッダ
*/

#ifndef	__MAIN_H__
#define	__MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro.h>
#include <fnt.h>
#include "myFunc.h"

// define data--------------------------------------------
#define MSG_VIEW_COUNT							30


// function-----------------------------------------------
extern void SEQ_DispNCD_init( void );
extern int  SEQ_DispNCD( void );

// プログラム起動時に使用
extern void InitDisp( void );
extern void InitIPL2Font( void );
extern void InitIPL2FontBG( void );
extern void ReadKeyPad( void );

#ifdef __cplusplus
}
#endif

#endif  // __MAIN_H__
