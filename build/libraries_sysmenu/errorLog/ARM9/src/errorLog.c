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
#define ERRORLOG_WRITE_FORMAT	"#%02u-%02u-%02u[%3s] %02u:%02u:%02u  ErrorCode: %u\n%s \n"ERRORLOG_BAR"\n\0"
#define ERRORLOG_READ_FORMAT	"#%d-%d-%d[%3s] %d:%d:%d  ErrorCode: %u\n%*s \n"ERRORLOG_BAR"\n\0"

#define ERRORLOG_SIZE			( 16 * 1024 )	// ファイルは16KBサイズ固定
#define ERRORLOG_BUFSIZE		128				// 一番長い名前のエラーでも128字以内に収まる
#define ERRORLOG_NUM_ENTRY		( ERRORLOG_SIZE / ERRORLOG_BUFSIZE ) // ログに書き込まれるエントリの最大数



// 内部関数SYSMi_CheckAndCreateDirectoryのエラーチェッカ
typedef enum CheckStatus {
	CHECK_EXIST = 0,
	CHECK_CREATE = 1,
	CHECK_FAILED = 2
} CheckStatus;
	

/*-- global variables ----------------------*/

ErrorLogWork elWork;

/*-- function prototype ----------------------*/
CheckStatus ELi_CheckAndCreateDirectory( const char *path );
CheckStatus ELi_CheckAndCreateFile( const char *path );
int ELi_ReadEntry( void );
BOOL ELi_SetString( char *buf, ErrorLogEntry *entry );
BOOL ELi_addNewEntry( int idx, int errorCode, RTCDate *date, RTCTime *time );
void ELi_WriteLogToBuf( char *buf );
BOOL ELi_WriteLogToFile( char *buf );
void ELi_fillSpace( char *buf, int bufsize );

static char *s_strWeek[7];
static char *s_strError[FATAL_ERROR_MAX];



/*---------------------------------------------------------------------------*
  Name:         EL_Init

  Description:  Errorlogライブラリ用の初期化関数です。
  				

  Arguments:    Alloc:	メモリ確保用の関数です。	
  				Free:	メモリ開放用の関数です。

  Returns:      成功すればTRUEを、失敗すればFALSEを返します。
 *---------------------------------------------------------------------------*/
BOOL EL_Init( void* (*AllocFunc) (u32) , void (*FreeFunc) (void*)  )
{
	SDK_POINTER_ASSERT(AllocFunc);
    SDK_POINTER_ASSERT(FreeFunc);

	elWork.Alloc = AllocFunc;
	elWork.Free = FreeFunc;
	
	// ログ読み出し用のバッファを確保
	elWork.entry = (ErrorLogEntry*) elWork.Alloc ( sizeof (ErrorLogEntry) * ERRORLOG_NUM_ENTRY );
	
	if( !FS_IsAvailable() )
	{
		// FSがInitされてなかったらInitする
		FS_Init( FS_DMA_NOT_USE );
	}

	FS_InitFile( &elWork.file );	
	
	// ファイルの存在確認
	if( ELi_CheckAndCreateDirectory( ERRORLOG_DIRECTORYPATH ) == CHECK_FAILED )
	{
		return FALSE;
	}
	
	switch ( ELi_CheckAndCreateFile( ERRORLOG_FILEPATH ) )
	{
		case CHECK_FAILED:
			return FALSE;
			break;
			
		case CHECK_EXIST:
			// 既にログファイルが存在していたら、そこからログを読み出す
			elWork.numEntry = ELi_ReadEntry();
			break;
			
		case CHECK_CREATE:
			// 新規にファイルが作られたなら何もしなくていい
			break;
	}

	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         EL_End

  Description:  Errorlogライブラリの終了処理を行います。
  				再度ELライブラリを利用するためにはEL_Initを呼ぶ必要があります。
  				
  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void EL_End( void )
{
	elWork.Free( elWork.entry );

	if( !FS_CloseFile( &elWork.file ) )
	{
		OS_TPrintf("EL Error: FS_CloseFile() failed.\n" );
	}
}
	
/*---------------------------------------------------------------------------*
  Name:         EL_WriteErrorLog

  Description:  nand:/sys/log/sysmenu.logにエラーログを出力します。
  

  Arguments:    errorCode:	発生したエラーのエラーコード

  Returns:      書き込みに成功したときはTRUEを、失敗したときはFALSEを返します。
 *---------------------------------------------------------------------------*/
BOOL EL_WriteErrorLog( u64 errorCode )
{
	int bufBeginPoint = 0; 	// リングバッファの開始点
	int numEntry = 0;
	int counter = 0;

	RTCDate date;
	RTCTime time;
	RTCResult rtcRes;
	
	char *writeBuf;
	
	writeBuf = (char*) elWork.Alloc( ERRORLOG_SIZE );

	// 新しいログエントリを書き込むためのRTC
	if( ( rtcRes = RTC_GetDateTime( &date, &time )) != RTC_RESULT_SUCCESS)
	{
		OS_TPrintf("EL Error: RTC getDateTime() Failed!  Status:%d\n", rtcRes);
		return FALSE;
	}

	
	for(counter = 0; counter < FATAL_ERROR_MAX; counter++ )
	{
		if( ( errorCode >> counter ) & 0x1LL )
		{
			// 末尾のビットが立っていたらエントリに入れてバッファ開始点を進める
			ELi_addNewEntry( elWork.numEntry % ERRORLOG_NUM_ENTRY , counter , &date, &time );
			elWork.numEntry++;
		}
	}
	
	
	// まずエントリをもとにバッファに書き込む
	ELi_WriteLogToBuf( writeBuf );
	
	// 最終的にファイルを書き込む
	if( !ELi_WriteLogToFile( writeBuf ) )
	{
		return FALSE;
	}
	
	elWork.Free( writeBuf );
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         EL_getErrorLogNum

  Description:  読み出したエントリ数を取得します。

  Arguments:    なし。

  Returns:      ログファイルから読み出したエントリ数を返します。
 *---------------------------------------------------------------------------*/

int EL_getErrorLogNum()
{
	return elWork.numEntry;
}

	
/*---------------------------------------------------------------------------*
  Name:         EL_getErrorLog

  Description:  指定したナンバのエラーログエントリへのポインタを取得します。

  Arguments:    idx:	エントリ番号の指定

  Returns:      idx番目のエントリへのポインタです。
 *---------------------------------------------------------------------------*/

const ErrorLogEntry* EL_getErrorLog( int idx )
{
	return &elWork.entry[idx];
}

/*---------------------------------------------------------------------------*
  Name:         ELi_addNewEntry

  Description:  エラーコードとRTCデータをエラーログのエントリに追加します。

  Arguments:    int:		エントリを挿入するインデクス
  				errorCode:	エラーコード
  				date:		日付データ
  				time:		時刻データ

  Returns:      FATAL_ERROR_MAXを超えるエラーコードが渡された場合はFALSEを、
  				それ以外のときはTRUEを返します。
 *---------------------------------------------------------------------------*/


BOOL ELi_addNewEntry( int idx, int errorCode, RTCDate *date, RTCTime *time )
{

	if( errorCode >= FATAL_ERROR_MAX )
	{
		// イリーガルなエラーコード
		OS_TPrintf("EL Error: Illigal error code (%d)\n", errorCode);
		return FALSE;
	}
	
	elWork.entry[idx].year = (int)date->year;
	elWork.entry[idx].month = (int)date->month;
	elWork.entry[idx].day = (int)date->day;
	STD_CopyLStringZeroFill( elWork.entry[idx].week, s_strWeek[ date->week ], 4 );
	elWork.entry[idx].hour = (int)time->hour;
	elWork.entry[idx].minute = (int)time->minute;
	elWork.entry[idx].second = (int)time->second;
	elWork.entry[idx].errorCode = (int)errorCode;
	
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

  Arguments:    path:		チェックを行うファイルのパス

  Returns:      ファイルが存在した場合はCHECK_EXISTを、
  				存在しておらず作成した場合はCHECK_CREATEを、
  				ファイル作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/

CheckStatus ELi_CheckAndCreateFile( const char *path )
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
  Name:         ELi_ReadEntry

  Description:  ログファイルに書き込まれた過去のエントリを読み出す

  Arguments:    なし。

  Returns:      読み出したエントリの数
 *---------------------------------------------------------------------------*/

int ELi_ReadEntry( void )
{
	char buf[ERRORLOG_BUFSIZE+1];
	int numEntry = 0;
	int readSize = 0;
	
	buf[ERRORLOG_BUFSIZE] = '\0';
	
	FS_SeekFileToBegin( &elWork.file );
	readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );

	// エントリの頭には必ず'#'が書き込まれているのでそれで判定	
	while( readSize == ERRORLOG_BUFSIZE && buf[0] == '#')
	{
		// 決められたファイルフォーマットからエントリに読み込む
		STD_TSScanf( buf, ERRORLOG_READ_FORMAT, 
					&(elWork.entry[numEntry].year) ,
					&(elWork.entry[numEntry].month) ,
					&(elWork.entry[numEntry].day) ,
					&(elWork.entry[numEntry].week) ,
					&(elWork.entry[numEntry].hour) ,
					&(elWork.entry[numEntry].minute) ,
					&(elWork.entry[numEntry].second) ,
					&(elWork.entry[numEntry].errorCode)  );

		numEntry++;
		readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );
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
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ELi_WriteLogToBuf

  Description:  受け取ったエントリの中身をバッファへ書き出します。

  Arguments:    buf:		16KB長バッファへのポインタ
  				entry:		エラー内容のエントリの配列ポインタ
  				numEntry:	合計エントリ数

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void ELi_WriteLogToBuf( char *buf )
{
	// エントリを書き出す開始点
	int entryIdx = elWork.numEntry <= ERRORLOG_NUM_ENTRY ? 0 : elWork.numEntry % ERRORLOG_NUM_ENTRY ;
	int counter;
	int counterMax = elWork.numEntry <= ERRORLOG_NUM_ENTRY ? elWork.numEntry : ERRORLOG_NUM_ENTRY ;
	
	// ファイルの頭に戻って書き込みなおす
	FS_SeekFileToBegin( &elWork.file );
	
	for( counter = 0; counter < counterMax ; counter++ )
	{
		// bufに一エントリずつ文字列化して詰めていく
		ELi_SetString( &buf[ counter * ERRORLOG_BUFSIZE ], &(elWork.entry[ (entryIdx + counter) % ERRORLOG_NUM_ENTRY ]) );
		
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
  Name:         ELi_WriteLogToFile

  Description:  受け取ったエントリの中身をログファイルに書き出します。

  Arguments:    buf:	書き込み内容の格納されたバッファへのポインタ

  Returns:      成功した場合はTRUE、失敗した場合はFALSEが返ります。
 *---------------------------------------------------------------------------*/

BOOL ELi_WriteLogToFile( char *buf )
{
	FSResult res;
	
	FS_SeekFileToBegin( &elWork.file );
	if( FS_WriteFile( &elWork.file, buf, ERRORLOG_SIZE ) != ERRORLOG_SIZE )
	{
		OS_TPrintf("EL Error: FS_WriteFile() failed.\n");
		return FALSE;
	}
	
	if( ( res = FS_FlushFile( &elWork.file )) != FS_RESULT_SUCCESS )
	{
		OS_TPrintf("EL Error: FS_FlushFile() failed. FSResult: %d\n", res);
		return FALSE;
	}
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ELi_fillSpace

  Description:  受け取ったエントリの余りの部分を0で埋めます。

  Arguments:    buf:		書き込み内容の格納されたバッファへのポインタ
  				bufsize:	エントリ全体のサイズ

  Returns:      なし。
 *---------------------------------------------------------------------------*/

void ELi_fillSpace( char *buf, int bufsize )
{
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
	"FATAL_ERROR_LOAD_UNFINISHED",
	"FATAL_ERROR_LOAD_OPENFILE_FAILED",
	"FATAL_ERROR_LOAD_MEMALLOC_FAILED",
	"FATAL_ERROR_LOAD_SEEKFILE_FAILED",
	"FATAL_ERROR_LOAD_READHEADER_FAILED",
	"FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39",
	"FATAL_ERROR_LOAD_READDLSIGN_FAILED",
	"FATAL_ERROR_LOAD_RELOCATEINFO_FAILED",
	"FATAL_ERROR_LOAD_READMODULE_FAILED"

};