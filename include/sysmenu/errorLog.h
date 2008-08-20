/*---------------------------------------------------------------------------*
  Project:  TwlIPL - ErrorLog
  File:     errorLog.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
 
 #ifndef __SYSM_ERRORLOG__
 #define __SYSM_ERRORLOG__

#include <twl.h> 
 
#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

/*-- type definition ----------------------------*/
// 日付データとかを除いた、自由フォーマットで書き込めるサイズ
// このサイズを超えた文字列は切り捨てられます
// ERRORLOG_STR_LENGTH = ERRORLOG_BUFSIZE - ERRORLOG_STR_OFFSET - 1
#define ERRORLOG_STR_LENGTH		194

// 既に書き込まれたエラーログを表現するためのエントリ
typedef struct ErrorLogEntry{
	// ランチャからの呼び出しかどうか
	BOOL isBroken;
	BOOL isLauncherError;
	
	// エラーのタイムスタンプ
	int year;
	int month;
	int day;
	char week[4]; // 曜日の3文字表現
	int hour;
	int minute;
	int second;
	
	// u32だけど実態は4byteの文字列
	u32 titleId;
	
	// ---- isLauncherError = TRUEの時のデータ ----
	// エラーコード
	int errorCode;
	
	// ---- isLauncherError = FALSEの時のデータ ----
	char *errorStr;
} ErrorLogEntry;

typedef struct ErrorLogWork{
	// メモリ確保用関数
	void* (*Alloc) ( u32 )  ;
	void (*Free) ( void* )  ;
	// エラーログエントリ保持用変数	
	ErrorLogEntry *entry;
	// エラーログのエントリ数
	int numEntry;
	// エラーログのファイルポインタ
	FSFile file;
} ErrorLogWork;


/*-- function prototype -------------------------*/
extern BOOL ERRORLOG_Printf( const char *fmt, ... );
extern BOOL ERRORLOG_Init( void* (*AllocFunc) (u32) , void (*FreeFunc) (void*)  );
extern void ERRORLOG_End( void );
extern int ERRORLOG_GetNum() ;
extern const ErrorLogEntry* ERRORLOG_Read( int idx );


// for RED Launcher
extern BOOL ERRORLOG_Write( u64 errorCode );

#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
