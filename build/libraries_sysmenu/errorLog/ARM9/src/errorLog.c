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
//


#include <twl.h>
#include <nitro/fs.h>
#include <string.h>
#include <sysmenu.h>
#include <sysmenu/errorLog.h>


#define ERRORLOG_DIRECTORYPATH	"nand:/sys/log"
#define ERRORLOG_FILEPATH		"nand:/sys/log/sysmenu.log"
#define ERRORLOG_BAR			"======================"
#define ERRORLOG_WRITE_FORMAT	"#%02lu-%02lu-%02lu[%3s] %02lu:%02lu:%02lu  ErrorCode: %llu\n%s \n"ERRORLOG_BAR"\n\0"
#define ERRORLOG_READ_FORMAT	"#%2lu-%2lu-%2lu[%3s] %2lu:%2lu:%2lu  ErrorCode: %llu\n%*s \n"ERRORLOG_BAR"\n\0"

#define ERRORLOG_SIZE			( 16 * 1024 )	// ファイルは16KBサイズ固定
#define ERRORLOG_BUFSIZE		128				// 一番長い名前のエラーでも128字以内に収まる
#define ERRORLOG_NUM_ENTRY		( ERRORLOG_SIZE / ERRORLOG_BUFSIZE ) // ログに書き込まれるエントリの最大数



// 内部関数SYSMi_CheckAndCreateDirectoryのエラーチェッカ
typedef enum CheckStatus {
	CHECK_EXIST = 0,
	CHECK_CREATE = 1,
	CHECK_FAILED = 2
} CheckStatus;
	
// 既に書き込まれたエラーログを表現するためのエントリ
typedef struct ErrorLogEntry{
	// エラーのタイムスタンプ
	u32 year;
	u32 month;
	u32 day;
	char week[4]; // 曜日の3文字表現
	u32 hour;
	u32 minute;
	u32 second;
	// エラーコード
	u64 errorCode;
} ErrorLogEntry;

// ログエラーのエントリを持つ

/*-- function prototype ----------------------*/
CheckStatus ELi_CheckAndCreateDirectory( const char *path );
CheckStatus ELi_CheckAndCreateFile( FSFile *file, const char *path );
int ELi_ReadEntry( FSFile *file, ErrorLogEntry *entry );
BOOL ELi_SetString( char *buf, ErrorLogEntry *entry );
BOOL ELi_WriteLog( FSFile *file ,ErrorLogEntry *entry, int num, u64 err );
void ELi_fillSpace( char *buf, int bufsize );

static char *s_strWeek[7];
static char *s_strError[FATAL_ERROR_MAX];

/*---------------------------------------------------------------------------*
  Name:         EL_WriteErrorLog

  Description:  nand:/sys/log/sysmenu.logにエラーログを出力します。
  

  Arguments:    errorCode:	発生したエラーのエラーコード

  Returns:      書き込みに成功したときはTRUEを、失敗したときはFALSEを返します。
 *---------------------------------------------------------------------------*/
BOOL EL_WriteErrorLog( u64 errorCode )
{
	FSFile file;
	ErrorLogEntry entry[ERRORLOG_NUM_ENTRY];
	int numEntry = 0;
	
	FS_InitFile( &file );	

	if( errorCode >= FATAL_ERROR_MAX )
	{
		// イリーガルなエラーコード
		OS_TPrintf("EL Error: Illigal error code (%d)\n", errorCode);
		return FALSE;
	}
	
	if( !FS_IsAvailable() )
	{
		// FSがInitされてなかったらInitする
		FS_Init( FS_DMA_NOT_USE );
	}
	
	if( ELi_CheckAndCreateDirectory( ERRORLOG_DIRECTORYPATH ) == CHECK_FAILED )
	{
		return FALSE;
	}
	
	switch ( ELi_CheckAndCreateFile( &file, ERRORLOG_FILEPATH ) )
	{
		case CHECK_FAILED:
			return FALSE;
			break;
			
		case CHECK_EXIST:
			// 既にログファイルが存在していたら、そこからログを読み出す
			numEntry = ELi_ReadEntry( &file, entry );
			break;
			
		case CHECK_CREATE:
			// 新規にファイルが作られたなら何もしなくていい
			break;
	}
	
	
	if( !ELi_WriteLog( &file, entry, numEntry+1 , errorCode ) )
	{
		return FALSE;
	}
		
	return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         ELi_CheckAndCreateDirectory

  Description:  この関数は該当ディレクトリが存在していれば何もしません。
				該当ディレクトリが存在していなかった場合は
				ディレクトリを作成します。

  Arguments:    path:		チェックを行うディレクトリのパス

  Returns:      ディレクトリが存在した場合はCHECK_EXISTを、
  				存在しておらず作成した場合はCHECK_CREATEを、
  				ディレクトリ作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/

CheckStatus ELi_CheckAndCreateDirectory( const char *path )
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
	if( ! FS_CreateDirectory( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateDirectory() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// ディレクトリ作成に失敗
		return CHECK_FAILED;
	}

	// ディレクトリ作成に成功
	return CHECK_CREATE;
}

/*---------------------------------------------------------------------------*
  Name:         ELi_CheckAndCreateFile

  Description:  この関数は該当ファイルが存在していれば何もしません。
				該当ファイルが存在していなかった場合はファイルを作成します。

  Arguments:    file:		FSFile構造体へのポインタ
  				path:		チェックを行うファイルのパス

  Returns:      ファイルが存在した場合はCHECK_EXISTを、
  				存在しておらず作成した場合はCHECK_CREATEを、
  				ファイル作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/

CheckStatus ELi_CheckAndCreateFile( FSFile *file, const char *path )
{

	if( FS_OpenFileEx( file, path, FS_FILEMODE_RWL ) )
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
	if( !FS_OpenFileEx( file, path, FS_FILEMODE_RW ) )
	{
		// 作成したファイルをopenできなかった場合
		OS_TPrintf("EL Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return CHECK_FAILED;
	}
	
	if( FS_SetFileLength( file, ERRORLOG_SIZE ) != FS_RESULT_SUCCESS )
	{
		// 作成したファイルのサイズを設定できなかった
		OS_TPrintf("EL Error: FS_SetFileLength() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		FS_CloseFile( file );
		return CHECK_FAILED; 
	}

	// サイズ変更が終わったら、念のためファイルサイズ変更不可なRWLモードで開きなおしておく
	FS_CloseFile( file );

	if( !FS_OpenFileEx( file, path, FS_FILEMODE_RWL ) )
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
			FS_WriteFile( file, nullbuf, 1024);
		}
	}
	
	return CHECK_CREATE;
	
}



/*---------------------------------------------------------------------------*
  Name:         ELi_ReadEntry

  Description:  ログファイルに書き込まれた過去のエントリを読み出す

  Arguments:    file:		ログファイルのFSFile構造体
  				entry:		エントリを書き込むバッファへのポインタ

  Returns:      読み出したエントリの数
 *---------------------------------------------------------------------------*/

int ELi_ReadEntry( FSFile *file, ErrorLogEntry *entry )
{
	char buf[ERRORLOG_BUFSIZE+1];
	int numEntry = 0;
	
	buf[ERRORLOG_BUFSIZE] = '\0';
	
	FS_SeekFileToBegin( file );
	FS_ReadFile( file, buf, ERRORLOG_BUFSIZE );

	// エントリの頭には必ず'#'が書き込まれているのでそれで判定	
	while( buf[0] == '#' )
	{
		// 決められたファイルフォーマットからエントリに読み込む
		STD_TSScanf( buf, ERRORLOG_READ_FORMAT, 
					&(entry[numEntry].year) ,
					&(entry[numEntry].month) ,
					&(entry[numEntry].day) ,
					&(entry[numEntry].week) ,
					&(entry[numEntry].hour) ,
					&(entry[numEntry].minute) ,
					&(entry[numEntry].second) ,
					&(entry[numEntry].errorCode)  );

		numEntry++;

		FS_ReadFile( file, buf, ERRORLOG_BUFSIZE );

	}
	
	return numEntry;
}



/*---------------------------------------------------------------------------*
  Name:         ELi_SetString

  Description:  バッファに書き込むべきログデータをセットします。

  Arguments:    buf:	文字列をセットするバッファへのポインタ
  				entry:	エラー内容のエントリ

  Returns:      成功した場合はTRUE、失敗した場合はFALSEが返ります。
 *---------------------------------------------------------------------------*/

BOOL ELi_SetString( char *buf, ErrorLogEntry *entry )
{
	STD_TSNPrintf(buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT, 
					entry->year, entry->month, entry->day, entry->week,
					entry->hour, entry->minute, entry->second,
					entry->errorCode, s_strError[entry->errorCode] );
	
	// 余りをスペースで埋めて、改行で終端する
	ELi_fillSpace( buf, ERRORLOG_BUFSIZE );
	buf[ ERRORLOG_BUFSIZE-1 ] = '\n';
	
	//OS_TPrintf("set String...\n%s", buf );
				
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ELi_WriteLog

  Description:  受け取ったエントリの中身をログファイルに書き出します。

  Arguments:    file:	ログファイルのFSFile構造体へのポインタ
  				entry:	エラー内容のエントリの配列ポインタ
  				num:	entryに含まれるエントリの数
  				err:	今回発生したエラーのエラーコード

  Returns:      成功した場合はTRUE、失敗した場合はFALSEが返ります。
 *---------------------------------------------------------------------------*/

BOOL ELi_WriteLog( FSFile *file ,ErrorLogEntry *entry, int num, u64 err )
{
	RTCDate date;
	RTCTime time;
	RTCResult rtcRes;
	
	int entryIdx = 0;
	char buf[ERRORLOG_BUFSIZE];

	// ファイル数チェック
	if( num >= ERRORLOG_NUM_ENTRY ) 
	{
		entryIdx++;
	}
	
	// ファイルの頭に戻って書き込みなおす
	FS_SeekFileToBegin( file );
	
	for( ; entryIdx < num - 1; entryIdx++ )
	{
		ELi_SetString( buf, &entry[entryIdx] );
		
		if( FS_WriteFile( file, buf, (s32)ERRORLOG_BUFSIZE ) == -1 )
		{
			OS_TPrintf("EL Error: FS_WriteFile() failed. entry: %d\n", entryIdx );
			return FALSE;
		}
	}
	
	// 最後の一つは自前でRTCを取得して書き込む
	if( ( rtcRes = RTC_GetDateTime( &date, &time )) != RTC_RESULT_SUCCESS)
	{
		OS_TPrintf("EL Error: RTC getDateTime() Failed!  Status:%d\n", rtcRes);
		return FALSE;
	}
	
	snprintf( buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT, 
				date.year, date.month, date.day, s_strWeek[ date.week ], 
				time.hour, time.minute, time.second,
				err, s_strError[ err ] );
	
	// エントリ一つのサイズをきっちり128バイト固定にするために余白分を空白で埋めて
	// \0で終端する
	ELi_fillSpace( buf, ERRORLOG_BUFSIZE );
	buf[ERRORLOG_BUFSIZE-1] = '\0';
	
	// 最後のエントリをファイルに書き出す
	if( FS_WriteFile( file, buf, (s32)strlen(buf) ) == -1 )
	{
		OS_TPrintf("EL Error: FS_WriteFile() failed.\n" );
		return FALSE;
	}

	// ファイルの余りを0埋めする
	// open modeがサイズ固定なのでファイル終端を気にせず書き込む
	MI_CpuClear8( buf, ERRORLOG_BUFSIZE );
	while ( FS_WriteFile( file, buf, (s32) ERRORLOG_BUFSIZE ) == ERRORLOG_BUFSIZE ) {};

	
	if( !FS_CloseFile( file ) )
	{
		OS_TPrintf("EL Error: FS_CloseFile() failed.\n" );
		return FALSE;
	}
	
	
	return TRUE;
}

void ELi_fillSpace( char *buf, int bufsize )
{
	// エントリの末尾にスペースを入れて
	// 一つのエントリがちょうど128バイトになるように辻褄を合わせる
	u32 length = strlen( buf );
	MI_CpuFill8( &buf[length], ' ', bufsize - length );	
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

static char *s_strError[] = {
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
	"FATAL_ERROR_LOAD_UNFINISHED"
};