/*---------------------------------------------------------------------------*
  Project:  TwlIPL - ErrorLog
  File:     errorLog.c

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

// Fatal error発生時に"nand:/sys/log/sysmenu.log"にログを吐くためのライブラリ


#include <twl.h>
#include <nitro/fs.h>
#include <string.h>
#include <sysmenu.h>
#include <sysmenu/errorLog.h>


#define ERRORLOG_DIRECTORYPATH	"nand:/sys/log"
#define ERRORLOG_FILEPATH		"nand:/sys/log/sysmenu.log"

#define ERRORLOG_HEADER_SIZE		6
#define ERRORLOG_WRITE_FORMAT_RED1	"\n\n#RED %02u-%02u-%02u[%3s] %02u:%02u:%02u\n"
#define ERRORLOG_WRITE_FORMAT_RED2	"title: %04s ErrorCode: %u\n%s"
#define ERRORLOG_WRITE_FORMAT1		"\n\n#FFT %02u-%02u-%02u[%3s] %02u:%02u:%02u\n"
#define ERRORLOG_WRITE_FORMAT2		"title: %04s\n%s"
#define ERRORLOG_READ_FORMAT_RED1	"\n\n#RED %d-%d-%d[%3s] %d:%d:%d\n"
#define ERRORLOG_READ_FORMAT_RED2	"title: %4s ErrorCode: %u\n%*s"
#define ERRORLOG_READ_FORMAT1		"\n\n#FFT %d-%d-%d[%3s] %d:%d:%d\n"
#define ERRORLOG_READ_FORMAT2		"title: %4s\n%s"

#define ERRORLOG_WRITE_FORMAT		ERRORLOG_WRITE_FORMAT1 ERRORLOG_WRITE_FORMAT2
#define ERRORLOG_WRITE_FORMAT_RED	ERRORLOG_WRITE_FORMAT_RED1 ERRORLOG_WRITE_FORMAT_RED2
#define ERRORLOG_READ_FORMAT		ERRORLOG_READ_FORMAT1 ERRORLOG_READ_FORMAT2
#define ERRORLOG_READ_FORMAT_RED	ERRORLOG_READ_FORMAT_RED1 ERRORLOG_READ_FORMAT_RED2

#define ERRORLOG_NUM_ARGS			9

#define ERRORLOG_STR_OFFSET		42



// 内部関数SYSMi_CheckAndCreateDirectoryのエラーチェッカ
typedef enum CheckStatus {
	CHECK_EXIST = 0,
	CHECK_CREATE = 1,
	CHECK_FAILED = 2
} CheckStatus;
	

/*-- global variables ----------------------*/

ErrorLogWork elWork;
static BOOL isInitialized = FALSE;

/*-- function prototype ----------------------*/
CheckStatus ERRORLOGi_CheckAndCreateDirectory( const char *path );
CheckStatus ERRORLOGi_CheckAndCreateFile( const char *path );
int ERRORLOGi_ReadEntry( void );
BOOL ERRORLOGi_SetString( char *buf, ErrorLogEntry *entry );
BOOL ERRORLOGi_addNewEntryRED( int idx, RTCDate *date, RTCTime *time, int errorCode );
void ERRORLOGi_addNewEntry( int idx, RTCDate *date, RTCTime *time, const char *fmt, va_list arglist );
void ERRORLOGi_WriteLogToBuf( char *buf );
BOOL ERRORLOGi_WriteLogToFile( char *buf );
void ERRORLOGi_fillSpace( char *buf, int bufsize );
BOOL ERRORLOGi_WriteCommon( BOOL isLauncherError, u64 errorCode, const char *fmt, va_list arglist );
u32 ERRORLOGi_getTitleId( void );

static char *s_strWeek[7];
static char *s_strError[FATAL_ERROR_MAX];


/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Init

  Description:  Errorlogライブラリ用の初期化関数です。
  				

  Arguments:    Alloc:	メモリ確保用の関数です。	
  				Free:	メモリ開放用の関数です。

  Returns:      成功すればTRUEを、失敗すればFALSEを返します。
 *---------------------------------------------------------------------------*/
BOOL ERRORLOG_Init( void* (*AllocFunc) (u32) , void (*FreeFunc) (void*)  )
{
	if( isInitialized )
	{
		return FALSE ;
	}
	
	SDK_POINTER_ASSERT(AllocFunc);
    SDK_POINTER_ASSERT(FreeFunc);

	elWork.Alloc = AllocFunc;
	elWork.Free = FreeFunc;
	
	OS_InitMutex( &elWork.mutex );
	OS_LockMutex( &elWork.mutex );

	// ログ読み出し用のバッファを確保
	if( elWork.entry == NULL )
	{
		elWork.entry = (ErrorLogEntry*) elWork.Alloc ( sizeof (ErrorLogEntry) * ERRORLOG_NUM_ENTRY );
		MI_CpuClear8( elWork.entry, sizeof (ErrorLogEntry) * ERRORLOG_NUM_ENTRY);
	}
	
	SDK_ASSERT( elWork.entry );
	
	if( !FS_IsAvailable() )
	{
		// FSがInitされてなかったらInitする
		FS_Init( FS_DMA_NOT_USE );
	}

	FS_InitFile( &elWork.file );	
	
	// ファイルの存在確認
	if( ERRORLOGi_CheckAndCreateDirectory( ERRORLOG_DIRECTORYPATH ) == CHECK_FAILED )
	{
		OS_UnlockMutex( &elWork.mutex );
		return FALSE;
	}
	
	// ファイルの存在を確認、ついでに中でオープンしてエントリの読み込み
	switch ( ERRORLOGi_CheckAndCreateFile( ERRORLOG_FILEPATH ) )
	{
		case CHECK_FAILED:
			OS_UnlockMutex( &elWork.mutex );
			return FALSE;
			break;
			
		case CHECK_EXIST:
			// 既にログファイルが存在していたら、そこからログを読み出す
			elWork.numEntry = ERRORLOGi_ReadEntry();
			break;
			
		case CHECK_CREATE:
			// 新規にファイルが作られたなら何もしなくていい
			break;
	}

	FS_CloseFile( &elWork.file );
	OS_UnlockMutex( &elWork.mutex );
	isInitialized = TRUE;

	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_End

  Description:  Errorlogライブラリの終了処理を行います。
  				再度ELライブラリを利用するためにはErrorLog_Initを呼ぶ必要があります。
  				
  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void ERRORLOG_End( void )
{
	int idx;

	for( idx = 0; idx < ERRORLOG_NUM_ENTRY ; idx++ )
	{
		if( elWork.entry[idx].errorStr != NULL )
		{
			elWork.Free( elWork.entry[idx].errorStr );
		}
	}
	
	elWork.Free( elWork.entry );

	isInitialized = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Printf

  Description:  nand:/sys/log/sysmenu.logにエラーログをフリーな形式で書き込みます。
  
  Arguments:    :	printfに準拠

  Returns:      書き込みに成功したときはTRUEを、失敗したときはFALSEを返します。
 *---------------------------------------------------------------------------*/
BOOL ERRORLOG_Printf( const char *fmt, ... )
{
	BOOL result = TRUE;
	va_list arglist;
	
	if( !isInitialized )
	{
		return FALSE;
	}
	
	OS_LockMutex( &elWork.mutex );
	va_start( arglist, fmt );
	result = ERRORLOGi_WriteCommon( FALSE, 0, fmt, arglist );
	va_end( arglist );
	OS_UnlockMutex( &elWork.mutex );

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Write

  Description:  nand:/sys/log/sysmenu.logにエラーログを出力します。
  
  Arguments:    errorCode:	発生したエラーのエラーコード

  Returns:      書き込みに成功したときはTRUEを、失敗したときはFALSEを返します。
 *---------------------------------------------------------------------------*/
BOOL ERRORLOG_Write( u64 errorCode )
{
	BOOL res;
	
	if( !isInitialized )
	{
		return FALSE;
	}
	
	OS_LockMutex( &elWork.mutex );
	res = ERRORLOGi_WriteCommon( TRUE, errorCode, NULL, NULL );
	OS_UnlockMutex( &elWork.mutex );
	
	return res;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_WriteCommon

  Description:  
  
  Arguments:	isLauncherError: ERRORLOG_Writeから呼ばれた場合はTRUE、
  								 ERRORLOG_Printfから呼ばれた場合はFALSEとなります。
  				errorCode:	isLauncherErrorがTRUEな時に書き込まれるエラーコード
  				fmt, ...:		isLauncherErrorがFALSEな時に書き込まれる文字列と引数

  Returns:      書き込みに成功したときはTRUEを、失敗したときはFALSEを返します。
 *---------------------------------------------------------------------------*/
 
BOOL ERRORLOGi_WriteCommon( BOOL isLauncherError, u64 errorCode, const char *fmt, va_list arglist )
{
	int bufBeginPoint = 0; 	// リングバッファの開始点
	int numEntry = 0;
	int counter = 0;

	RTCDate date;
	RTCTime time;
	RTCResult rtcRes;
	
	char *writeBuf;
	
	writeBuf = (char*) elWork.Alloc( ERRORLOG_SIZE );
	SDK_ASSERT( writeBuf );

	// 新しいログエントリを書き込むためのRTC
	if( ( rtcRes = RTC_GetDateTime( &date, &time )) != RTC_RESULT_SUCCESS )
	{
		elWork.Free( writeBuf );
		OS_TPrintf("EL Error: RTC getDateTime() Failed!  Status:%d\n", rtcRes);
		return FALSE;
	}

	if( isLauncherError )
	{
		for(counter = 0; counter < FATAL_ERROR_MAX; counter++ )
		{
			if( ( errorCode >> counter ) & 0x1LL )
			{
				// 末尾のビットが立っていたらエントリに入れてバッファ開始点を進める
				ERRORLOGi_addNewEntryRED( elWork.numEntry % ERRORLOG_NUM_ENTRY , &date, &time, counter );
				elWork.numEntry++;
			}
		}
	}
	else
	{
		ERRORLOGi_addNewEntry( elWork.numEntry % ERRORLOG_NUM_ENTRY, &date, &time, fmt, arglist );
		elWork.numEntry++;
	}
	
	// まずエントリをもとにバッファに書き込む
	ERRORLOGi_WriteLogToBuf( writeBuf );
	
	// 最終的にファイルを書き込む
	if( !ERRORLOGi_WriteLogToFile( writeBuf ) )
	{
		elWork.Free( writeBuf );
		return FALSE;
	}
	
	elWork.Free( writeBuf );
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_GetNum

  Description:  読み出したエントリ数を取得します。

  Arguments:    なし。

  Returns:      ログファイルから読み出したエントリ数を返します。
 *---------------------------------------------------------------------------*/

int ERRORLOG_GetNum()
{
	return elWork.numEntry < ERRORLOG_NUM_ENTRY ? elWork.numEntry : ERRORLOG_NUM_ENTRY;
}

	
/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Read

  Description:  指定したナンバのエラーログエントリへのポインタを取得します。

  Arguments:    idx:	エントリ番号の指定

  Returns:      idx番目のエントリへのポインタです。
 *---------------------------------------------------------------------------*/

const ErrorLogEntry* ERRORLOG_Read( int idx )
{
	if( idx >= 0 && idx < ERRORLOG_NUM_ENTRY )
	{
		return &elWork.entry[idx];
	}
	else
	{
		return NULL;
	}
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_addNewEntryRED

  Description:  エラーコードとRTCデータをエラーログのエントリに追加します。

  Arguments:    int:		エントリを挿入するインデクス
  				date:		日付データ
  				time:		時刻データ
  				errorCode:	エラーコード

  Returns:      FATAL_ERROR_MAXを超えるエラーコードが渡された場合はFALSEを、
  				それ以外のときはTRUEを返します。
 *---------------------------------------------------------------------------*/


BOOL ERRORLOGi_addNewEntryRED( int idx, RTCDate *date, RTCTime *time, int errorCode )
{

	if( errorCode >= FATAL_ERROR_MAX )
	{
		// イリーガルなエラーコード
		OS_TPrintf("EL Error: Illigal error code (%d)\n", errorCode);
		return FALSE;
	}

	elWork.entry[idx].isLauncherError = TRUE;
	elWork.entry[idx].isBroken = FALSE;
	elWork.entry[idx].year = (int)date->year;
	elWork.entry[idx].month = (int)date->month;
	elWork.entry[idx].day = (int)date->day;
	STD_CopyLStringZeroFill( elWork.entry[idx].week, s_strWeek[ date->week ], 4 );
	elWork.entry[idx].hour = (int)time->hour;
	elWork.entry[idx].minute = (int)time->minute;
	elWork.entry[idx].second = (int)time->second;
	elWork.entry[idx].errorCode = (int)errorCode;
	elWork.entry[idx].titleId = ERRORLOGi_getTitleId();
	
	if( elWork.entry[idx].errorStr != NULL )
	{
		// 上書きされる前がUIG側のエラーだった場合はstr用のバッファを開放しておく
		elWork.Free( elWork.entry[idx].errorStr );
	}
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_addNewEntry

  Description:  自由な文字列とRTCデータをエラーログエントリに追加します。

  Arguments:    int:		エントリを挿入するインデクス
  				date:		日付データ
  				time:		時刻データ
  				fmt:		エラーログに書き込まれる文字列のフォーマット
  				arglist:	fmtに対する引数
  				

  Returns:      なし。
 *---------------------------------------------------------------------------*/


void ERRORLOGi_addNewEntry( int idx, RTCDate *date, RTCTime *time, const char *fmt, va_list arglist )
{

	elWork.entry[idx].isLauncherError = FALSE;
	elWork.entry[idx].isBroken = FALSE;
	elWork.entry[idx].year = (int)date->year;
	elWork.entry[idx].month = (int)date->month;
	elWork.entry[idx].day = (int)date->day;
	STD_CopyLStringZeroFill( elWork.entry[idx].week, s_strWeek[ date->week ], 4 );
	elWork.entry[idx].hour = (int)time->hour;
	elWork.entry[idx].minute = (int)time->minute;
	elWork.entry[idx].second = (int)time->second;
	elWork.entry[idx].titleId = ERRORLOGi_getTitleId();
	
	if( elWork.entry[idx].errorStr == NULL )
	{
		// str用の領域があたってなければ確保する
		elWork.entry[idx].errorStr = elWork.Alloc( ERRORLOG_BUFSIZE+1 );
		MI_CpuClear8( elWork.entry[idx].errorStr, ERRORLOG_BUFSIZE+1 );	
	}
	
	STD_TVSNPrintf( elWork.entry[idx].errorStr, ERRORLOG_BUFSIZE, fmt, arglist );
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_CheckAndCreateDirectory

  Description:  この関数は該当ディレクトリが存在していれば何もしません。
				該当ディレクトリが存在していなかった場合は
				ディレクトリを作成します。

  Arguments:    path:		チェックを行うディレクトリのパス

  Returns:      ディレクトリが存在した場合はCHECK_EXISTを、
  				存在しておらず作成した場合はCHECK_CREATEを、
  				ディレクトリ作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/

CheckStatus ERRORLOGi_CheckAndCreateDirectory( const char *path )
{
	FSFile dir;

	FS_InitFile( &dir );
	
	if( FS_OpenDirectory( &dir, path, FS_FILEMODE_RW ) )
	{
		// ディレクトリが存在していたらそのままCloseして戻る
		FS_CloseDirectory( &dir );
		return CHECK_EXIST;
	}

	// ディレクトリが存在しないのでディレクトリを作成
	if( ! FS_CreateDirectoryAuto( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateDirectory() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// ディレクトリ作成に失敗
		return CHECK_FAILED;
	}

	// ディレクトリ作成に成功
	return CHECK_CREATE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_CheckAndCreateFile

  Description:  この関数は該当ファイルが存在していれば何もしません。
				該当ファイルが存在していなかった場合はファイルを作成します。

  Arguments:    path:		チェックを行うファイルのパス

  Returns:      ファイルが存在した場合はCHECK_EXISTを、
  				存在しておらず作成した場合はCHECK_CREATEを、
  				ファイル作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/

CheckStatus ERRORLOGi_CheckAndCreateFile( const char *path )
{

	if( FS_OpenFileEx( &elWork.file, path, FS_FILEMODE_RWL ) )
	{
		return CHECK_EXIST;
	}

	// ファイルが存在しないので作成する	
	if( !FS_CreateFile( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateFile() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// ファイル作成に失敗
		return CHECK_FAILED;
	}


	// ファイル作成に成功
	if( !FS_OpenFileEx( &elWork.file, path, FS_FILEMODE_RW ) )
	{
		// 作成したファイルをopenできなかった場合
		OS_TPrintf("EL Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return CHECK_FAILED;
	}
	
	if( FS_SetFileLength( &elWork.file, ERRORLOG_SIZE ) != FS_RESULT_SUCCESS )
	{
		// 作成したファイルのサイズを設定できなかった
		OS_TPrintf("EL Error: FS_SetFileLength() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		FS_CloseFile( &elWork.file );
		return CHECK_FAILED; 
	}

	// サイズ変更が終わったら、念のためファイルサイズ変更不可なRWLモードで開きなおしておく
	FS_CloseFile( &elWork.file );

	if( !FS_OpenFileEx( &elWork.file, path, FS_FILEMODE_RWL ) )
	{
		OS_TPrintf("EL Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return CHECK_FAILED;
	}

	
	// ファイルのゼロ埋めを行う
	{
		char nullbuf[1024];
		int i;

		MI_CpuClear8( nullbuf, 1024);
		
		for(i = 0; i < 16; i++)
		{
			FS_WriteFile( &elWork.file, nullbuf, 1024);
		}
	}
	
	return CHECK_CREATE;
}



/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_ReadEntry

  Description:  ログファイルに書き込まれた過去のエントリを読み出す

  Arguments:    なし。

  Returns:      読み出したエントリの数
 *---------------------------------------------------------------------------*/

int ERRORLOGi_ReadEntry( void )
{
	char buf[ERRORLOG_BUFSIZE+1];
	char titlebuf[5];
	int numEntry = 0;
	int readSize = 0;
	
	buf[ERRORLOG_BUFSIZE] = '\0';
	
	FS_SeekFileToBegin( &elWork.file );
	readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );

	// エントリの頭には必ず'#'が書き込まれているのでそれで判定	
	while( readSize == ERRORLOG_BUFSIZE && numEntry < ERRORLOG_NUM_ENTRY)
	{
		int numArgs = 0;
		
		// 決められたファイルフォーマットからエントリに読み込む
		if( ! STD_StrNCmp( "\n\n#RED", buf, ERRORLOG_HEADER_SIZE ) )
		{
			// ランチャから書き込まれたエラーの場合

			
			elWork.entry[numEntry].isLauncherError = TRUE;
			elWork.entry[numEntry].isBroken = FALSE;
			numArgs = STD_TSScanf( buf, ERRORLOG_READ_FORMAT_RED, 
						&(elWork.entry[numEntry].year) ,
						&(elWork.entry[numEntry].month) ,
						&(elWork.entry[numEntry].day) ,
						&(elWork.entry[numEntry].week) ,
						&(elWork.entry[numEntry].hour) ,
						&(elWork.entry[numEntry].minute) ,
						&(elWork.entry[numEntry].second) ,
						titlebuf ,
						&(elWork.entry[numEntry].errorCode)  );
			
			elWork.entry[numEntry].titleId = MI_LoadLE32( titlebuf );
			
		}
		else if( !STD_StrNCmp( "\n\n#FFT", buf, ERRORLOG_HEADER_SIZE ) )
		{
			// フリーフォーマットで書き込まれたエラーの場合
			if( elWork.entry[numEntry].errorStr == NULL )
			{
				elWork.entry[numEntry].errorStr = elWork.Alloc( ERRORLOG_BUFSIZE+1 );
			}
			
			elWork.entry[numEntry].isLauncherError = FALSE;
			elWork.entry[numEntry].isBroken = FALSE;
			numArgs = STD_TSScanf( buf, ERRORLOG_READ_FORMAT, 
						&(elWork.entry[numEntry].year) ,
						&(elWork.entry[numEntry].month) ,
						&(elWork.entry[numEntry].day) ,
						&(elWork.entry[numEntry].week) ,
						&(elWork.entry[numEntry].hour) ,
						&(elWork.entry[numEntry].minute) ,
						&(elWork.entry[numEntry].second) ,
						titlebuf ,
						elWork.entry[numEntry].errorStr ); // 最後の一つは引数数のつじつまを合わせるため
			
			elWork.entry[numEntry].titleId = MI_LoadLE32( titlebuf );
			STD_CopyLStringZeroFill( elWork.entry[numEntry].errorStr, &buf[ERRORLOG_STR_OFFSET], ERRORLOG_STR_LENGTH + 1);

		}
		
		if ( numArgs != ERRORLOG_NUM_ARGS )
		{
			char cmpBuf[ERRORLOG_BUFSIZE+1];
			MI_CpuClear8( cmpBuf, ERRORLOG_BUFSIZE+1 );
			if( ! MI_CpuComp8( cmpBuf, buf, ERRORLOG_BUFSIZE+1 ) )
			{
				// 全部ヌル文字だったらそのエントリは書き込まれていないだけ
				readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );
				continue;
			}
			
			// エラーログが壊れていて解析できなかった場合の処理
			// もしくは古いログで上記の接頭辞がないログの場合の処理
			if( elWork.entry[numEntry].errorStr == NULL )
			{
				elWork.entry[numEntry].errorStr = elWork.Alloc( ERRORLOG_BUFSIZE+1 );
			}
			
			elWork.entry[numEntry].isBroken = TRUE;
			STD_CopyLStringZeroFill( elWork.entry[numEntry].errorStr, buf, ERRORLOG_BUFSIZE+1 );
			
		}

		numEntry++;
		readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );
	}
	
	return numEntry;
}



/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_SetString

  Description:  バッファに書き込むべきログデータをセットします。

  Arguments:    buf:	文字列をセットするバッファへのポインタ
  				entry:	エラー内容のエントリ

  Returns:      成功した場合はTRUE、失敗した場合はFALSEが返ります。
 *---------------------------------------------------------------------------*/

BOOL ERRORLOGi_SetString( char *buf, ErrorLogEntry *entry )
{
	char titlebuf[5];
	
	if( entry->isBroken )
	{
		STD_CopyLString( buf, entry->errorStr, ERRORLOG_BUFSIZE );
		buf[ ERRORLOG_BUFSIZE-1 ] = '\n';
		return TRUE;
	}
	
	// 壊れてない場合はtitleIDを書き込む必要があるので、u32を文字列化する
	STD_CopyLStringZeroFill( titlebuf, (char*)&entry->titleId, 5);
	
	if ( entry->isLauncherError )
	{
		STD_TSNPrintf(buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT_RED, 
						entry->year, entry->month, entry->day, entry->week,
						entry->hour, entry->minute, entry->second,
						titlebuf, entry->errorCode,
						s_strError[entry->errorCode] ? s_strError[entry->errorCode] : "" );
	}
	else
	{
		STD_TSNPrintf(buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT,
						entry->year, entry->month, entry->day, entry->week,
						entry->hour, entry->minute, entry->second,
						titlebuf, entry->errorStr );
	}
					    
	// 余りをスペースで埋めて、改行で終端する
	ERRORLOGi_fillSpace( buf, ERRORLOG_BUFSIZE );
	buf[ ERRORLOG_BUFSIZE-1 ] = '\n';
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_WriteLogToBuf

  Description:  受け取ったエントリの中身をバッファへ書き出します。

  Arguments:    buf:		16KB長バッファへのポインタ

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void ERRORLOGi_WriteLogToBuf( char *buf )
{
	// エントリを書き出す開始点
	int entryIdx = elWork.numEntry <= ERRORLOG_NUM_ENTRY ? 0 : elWork.numEntry % ERRORLOG_NUM_ENTRY ;
	int counter;
	int counterMax = elWork.numEntry <= ERRORLOG_NUM_ENTRY ? elWork.numEntry : ERRORLOG_NUM_ENTRY ;
	
	for( counter = 0; counter < counterMax ; counter++ )
	{
		// bufに一エントリずつ文字列化して詰めていく
		ERRORLOGi_SetString( &buf[ counter * ERRORLOG_BUFSIZE ], &(elWork.entry[ (entryIdx + counter) % ERRORLOG_NUM_ENTRY ]) );
		
		if( counter == counterMax-1 )
		{
			// 最後のエントリは改行を入れずにヌル文字で終端
			buf[ (counter+1) * ERRORLOG_BUFSIZE - 1] = '\0';
		}
	}

	// バッファのあまり部分をゼロ埋めする
	MI_CpuClear8( &buf[ counter * ERRORLOG_BUFSIZE], (u32) ((ERRORLOG_NUM_ENTRY - counter) * ERRORLOG_BUFSIZE ) );
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_WriteLogToFile

  Description:  受け取ったエントリの中身をログファイルに書き出します。

  Arguments:    buf:	書き込み内容の格納されたバッファへのポインタ

  Returns:      成功した場合はTRUE、失敗した場合はFALSEが返ります。
 *---------------------------------------------------------------------------*/

BOOL ERRORLOGi_WriteLogToFile( char *buf )
{
	// ファイルの頭に戻って書き込みなおす
	FS_OpenFileEx( &elWork.file, ERRORLOG_FILEPATH, FS_FILEMODE_W );
	FS_SeekFileToBegin( &elWork.file );
	
	if( FS_WriteFile( &elWork.file, buf, ERRORLOG_SIZE ) != ERRORLOG_SIZE )
	{
		OS_TPrintf("EL Error: FS_WriteFile() failed.\n");
		FS_CloseFile( &elWork.file );
		return FALSE;
	}
	
	if( ! FS_CloseFile( &elWork.file ) )
	{
		OS_TPrintf("EL Error: FS_CloseFile() in WriteLogToFile() failed.");
		return FALSE;
	}
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_fillSpace

  Description:  受け取ったエントリの余りの部分をスペースで埋めます。

  Arguments:    buf:		書き込み内容の格納されたバッファへのポインタ
  				bufsize:	エントリ全体のサイズ

  Returns:      なし。
 *---------------------------------------------------------------------------*/

void ERRORLOGi_fillSpace( char *buf, int bufsize )
{
	u32 length = strlen( buf );
	MI_CpuFill8( &buf[length], ' ', bufsize - length );	
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_getTitleId

  Description:  起動しているアプリのtitleIdを取得

  Arguments:    

  Returns:      起動しているアプリのtitleID_Lo
 *---------------------------------------------------------------------------*/

u32 ERRORLOGi_getTitleId( void )
{
	return MI_LoadBE32( (void*)(HW_TWL_ROM_HEADER_BUF + 0x230) );

}

FSFile ERRORLOGi_getLogFilePt( void )
{
	return elWork.file;
}

static char *s_strWeek[] = {
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT"
};

static char *s_strError[ FATAL_ERROR_MAX ] = {
	"FATAL_ERROR_UNDEFINED",
	"FATAL_ERROR_NAND",
	"FATAL_ERROR_HWINFO_NORMAL",
	"FATAL_ERROR_HWINFO_SECURE",
	"FATAL_ERROR_TWLSETTINGS",
	"FATAL_ERROR_SHARED_FONT",
	"FATAL_ERROR_WLANFIRM_AUTH",
	"FATAL_ERROR_WLANFIRM_LOAD",
	"FATAL_ERROR_TITLE_LOAD_FAILED",
	"FATAL_ERROR_TITLE_POINTER_ERROR",
	"FATAL_ERROR_AUTHENTICATE_FAILED",
	"FATAL_ERROR_ENTRY_ADDRESS_ERROR",
	"FATAL_ERROR_TITLE_BOOTTYPE_ERROR",
	"FATAL_ERROR_SIGN_DECRYPTION_FAILED",
	"FATAL_ERROR_SIGN_COMPARE_FAILED",
	"FATAL_ERROR_HEADER_HASH_CALC_FAILED",
	"FATAL_ERROR_TITLEID_COMPARE_FAILED",
	"FATAL_ERROR_VALID_SIGN_FLAG_OFF",
	"FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED",
	"FATAL_ERROR_MODULE_HASH_CHECK_FAILED",
	"FATAL_ERROR_MODULE_HASH_CALC_FAILED",
	"FATAL_ERROR_MEDIA_CHECK_FAILED",
	"FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED",
	"FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED",
	"FATAL_ERROR_DL_HASH_CALC_FAILED",
	"FATAL_ERROR_DL_SIGN_COMPARE_FAILED",
	"FATAL_ERROR_WHITELIST_INITDB_FAILED",
	"FATAL_ERROR_WHITELIST_NOTFOUND",
	"FATAL_ERROR_DHT_PHASE1_FAILED",
	"FATAL_ERROR_DHT_PHASE2_FAILED",
	"FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF",
	"FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN",
	"FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN",
	"FATAL_ERROR_PLATFORM_UNKNOWN",
	"FATAL_ERROR_LOAD_UNFINISHED",
	"FATAL_ERROR_LOAD_OPENFILE_FAILED",
	"FATAL_ERROR_LOAD_MEMALLOC_FAILED",
	"FATAL_ERROR_LOAD_SEEKFILE_FAILED",
	"FATAL_ERROR_LOAD_READHEADER_FAILED",
	"FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39",
	"FATAL_ERROR_LOAD_READDLSIGN_FAILED",
	"FATAL_ERROR_LOAD_RELOCATEINFO_FAILED",
	"FATAL_ERROR_LOAD_READMODULE_FAILED",
    "FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED",
    "FATAL_ERROR_SYSMENU_VERSION",
    "FATAL_ERROR_DHT_PHASE1_CALC_FAILED",
    "FATAL_ERROR_LOAD_UNKNOWN_BOOTTYPE",
    "FATAL_ERROR_LOAD_AUTH_HEADER_FAILED",
    "FATAL_ERROR_LOAD_NEVER_STARTED",
};