// crc_whole.h の実装

#include "stdafx.h"
#include "common.h"
#include "crc_whole.h"
#include <cstring>
#include <cstdio>

#define CRCPOLY     0x1021
#define BUFLEN      0x1000

namespace MasterEditorTWL
{
	u16	crc_table[0x100];

	/*----------------------------------------------------------------------------
	  関数名
		inittable - initialize table

	  形式
		static void inittable(unsigned short *table);

	  解説
		CRC算出を高速化するための参照テーブルを作成する。
	----------------------------------------------------------------------------*/
	static void inittable(unsigned short *table)
	{
		unsigned short  i, j, r;

		for(i = 0; i < 0x100; i++) {
			r = i << 8;
			for(j = 0; j < 8; j++) {
				if(r & 0x8000U)
					r = (r << 1) ^ CRCPOLY;
				else
					r <<= 1;
			}
			*table++ = r;
		}
	}

	/*============================================================================*
	   CRC計算
	   製品技術部のCRCテーブルを使ったCRC計算関数

	   偶数バイトと奇数バイトを入れ替えてから計算する。
	 *============================================================================*/
	static u16 newGetCRC
		( 
		u16  start,    // CRC初期値（累積値） 
		u16 *datap,    // データの先頭を指すポインタ、中島さんのと違い、u8
		u32  size      // バイト単位でのデータサイズ
		)
	{
		u32 i;
		u16 crc;
		u8* byte;

		crc  = start;
		byte = (u8 *)datap;

		for (i=0; i<size; i+=2)
		{
			u8 byte0, byte1;

			byte0 = *byte;  byte++;
			byte1 = *byte;  byte++;

			crc = (crc << 8) ^ crc_table[(crc >> 8) ^ byte1];
			crc = (crc << 8) ^ crc_table[(crc >> 8) ^ byte0];
		}

		return crc;
	}

	/*---------------------------------------------------------------------------*

	 Name:        getWholeCRCInFp

	 Description: srlファイル全体のCRCを算出

	 Arguments:   [in]  srlファイルのファイルポインタ
				  [out] CRC格納先

	 Return:      成功ならTRUE.

	 *---------------------------------------------------------------------------*/
	BOOL getWholeCRCInFp( FILE *fp, u16 *pCRC )
	{
		u32   file_size;
		u16   crc;
		u32   i;
		u16   data[ 0x1000 / 2 ]; // 64Kバイト(512Kビット)

		if( !fp )
		{
			return FALSE;
		}

		// ファイルサイズチェック
		fseek( fp, 0, SEEK_END );
		file_size = ftell( fp );
		
		// CRCテーブル初期化（製品技術部のコード）
		inittable(crc_table);

		// ファイル全体のCRC をとる
		fseek( fp, 0, SEEK_SET );
		crc = 0;
		for ( i=0; i<file_size; i+=sizeof(data) )
		{
			fread( (void*)data, sizeof(data), 1, fp );
			crc = newGetCRC( crc, data, sizeof(data) );
		}
		*pCRC = crc;

		return TRUE;
	}

	/*---------------------------------------------------------------------------*

	 Name:        getWholeCRCInFile

	 Description: srlファイル全体のCRCを算出

	 Arguments:   [in]  srlファイル名(呼び出し前に閉じておくこと)
				  [out] CRC格納先

	 Return:      None.

	 *---------------------------------------------------------------------------*/
	BOOL getWholeCRCInFile( System::String ^filename, u16 *pCRC )
	{
		FILE* fp;
		const char *pchFilename = 
			(const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi( filename ).ToPointer();

		fopen_s( &fp, pchFilename, "rb" );
		if ( fp == NULL )
		{
			return FALSE;
		}

		getWholeCRCInFp( fp, pCRC );

		fclose( fp );
		return TRUE;
	}

} // end of namespace MasterEditorTWL
