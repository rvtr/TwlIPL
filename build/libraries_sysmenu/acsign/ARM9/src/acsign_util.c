/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     

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

#include <nitro.h>
#include <sysmenu/acsign.h>
#include <string.h>

#define AUTH_KEY_BUFFER_LEN 128
#define MB_AUTH_SIGN_SIZE				(128)	/* digital sign size */


typedef	struct	MbAuthCode
{
	char		magic_code[2];			// マジックナンバー
	u16		version;			// バージョン
	u8		sign[MB_AUTH_SIGN_SIZE];	// 署名
	u32		serial_number;			// シリアル番号
} MbAuthCode;	// 16byte



static u8 key_buffer[AUTH_KEY_BUFFER_LEN] = {
  0x9E,0xC1,0xCC,0xC0,0x4A,0x6B,0xD0,0xA0,0x6D,0x62,0xED,0x5F,0x15,0x67,0x87,0x12,
  0xE6,0xF4,0x77,0x1F,0xD8,0x5C,0x81,0xCE,0x0C,0xD0,0x22,0x31,0xF5,0x89,0x08,0xF5,
  0xBE,0x04,0xCB,0xC1,0x4F,0x63,0xD9,0x5A,0x98,0xFF,0xEB,0x36,0x0F,0x9C,0x5D,0xAD,
  0x15,0xB9,0x99,0xFB,0xC6,0x86,0x2C,0x0A,0x0C,0xFC,0xE6,0x86,0x03,0x60,0xD4,0x87,
  0x28,0xD5,0x66,0x42,0x9C,0xF7,0x04,0x14,0x4E,0x6F,0x73,0x20,0xC3,0x3E,0x3F,0xF5,
  0x82,0x2E,0x78,0x18,0xD6,0xCD,0xD5,0xC2,0xDC,0xAA,0x1D,0x34,0x91,0xEC,0x99,0xC9,
  0xF7,0xBF,0xBF,0xA0,0x0E,0x1E,0xF0,0x25,0xF8,0x66,0x17,0x54,0x34,0x28,0x2D,0x28,
  0xA3,0xAE,0xF0,0xA9,0xFA,0x3A,0x70,0x56,0xD2,0x34,0xA9,0xC5,0x9E,0x5D,0xF5,0xE1
};


int ACSignDecrpto(void *output_buffer, MBDownloadFileInfo *download_file_info_buf)
{
  return ACSign_Decrypto(output_buffer, 
			 ((MbAuthCode *)(download_file_info_buf->auth_code))->sign, 
			 key_buffer);
}


int ACSignDigest(void *input_buffer, MBDownloadFileInfo *download_file_info_buf)
{
  int ret_code;
  u8 digest_output_buffer[AUTH_BUFFER_LEN];

  ret_code = ACSign_Digest((void *)digest_output_buffer, //  出力領域 20 Bytes
			   (void *)(download_file_info_buf->seg[0].recv_addr),
			   (void *)(download_file_info_buf->seg[1].recv_addr),
			   (int)(download_file_info_buf->seg[1].size),
			   (void *)(download_file_info_buf->seg[2].recv_addr),
			   (int)(download_file_info_buf->seg[2].size),
			   ((MbAuthCode *)(download_file_info_buf)->auth_code)->serial_number 	  //  serial_number
			   );
  if( ret_code == 0 ) 
    {
      return ret_code; // failure
    }
			   
  return  ACSign_Compare(
			 input_buffer,                 //  ACSign_Decryptoの出力
			 (void*)digest_output_buffer    //  ACSign_Digestの出力
			 );
}

#if 0
void*   doAlloc( int size )
{
    OSIntrMode  nOSIntrMode;
    void*       alloc = NULL;

    nOSIntrMode = OS_DisableInterrupts( );
    alloc = OS_Alloc( size );
    (void)OS_RestoreInterrupts( nOSIntrMode );
    return alloc;
}

void    doFree( void* adrs )
{
    OSIntrMode  nOSIntrMode;

    nOSIntrMode = OS_DisableInterrupts( );
    OS_Free( adrs );
    (void)OS_RestoreInterrupts( nOSIntrMode );
}

void*   doMemset( void* adrs, int val, int cnt )
{
    return memset( adrs, val, cnt );
}

void*   doMemcpy( void* dst, void* src, int cnt )
{
    return memcpy( dst, src, cnt );
}

#endif
