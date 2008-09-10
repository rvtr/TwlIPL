/*---------------------------------------------------------------------------*
  Project:  Sample Test
  File:     test.c

  Copyright 2005 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>

#define	CODE_LENGTH		8

static u32 Crc32Table[0x100];

// Forward references
u32 CalcMasterkey(const u8 *src);

static void Crc32Init(void)
{
  u32 i, j;
  u32 poly = 0xedb88320;
  u32 crc;

  for (i = 0; i < 0x100; i++) {
    crc = i;
    for (j = 8; j > 0; j--) {
      if (crc & 1) {
        crc = (crc >> 1) ^ poly;
      } else {
        crc >>= 1;
      }
    }
    Crc32Table[i] = crc;
  }
}

static u32 CalcCrc32(const u8 *src, u32 len)
{
  u32 crc = 0xffffffff;

  while (len) {
    crc = ((crc >> 8) & 0xffffffff) ^ Crc32Table[(crc ^ *src) & 0xff];
    src++;
    len--;
  }
  return crc;
}

u32 CalcMasterkey(const u8 *src)
{
	static u8 initFlag = 0;
	u32 key;

	if (initFlag == 0) {
		Crc32Init();
		initFlag = 1;
	}

	key = ((CalcCrc32(src, CODE_LENGTH) ^ 0xaaaa) + 5313) % 100000;

	return key;
}

//======================================================================
//
//  マスターキー算出
//
//======================================================================
u32 UTL_CalcPCTLMasterKey( void )
{
	u32 masterKey;
	u32 inquiryCode = UTL_CalcPCTLInquiryCode();
	u8 arg[ 9 ];
	RTCDate date;
	RTC_Init();
	RTC_GetDate( &date );
	STD_TSPrintf( (char *)arg, "%02d%02d%04d", date.month, date.day, inquiryCode % 10000 );
	masterKey = CalcMasterkey( arg );
	OS_TPrintf( "MasterKey : %05d\n", masterKey );
	return masterKey;
}


//======================================================================
//
//  ペアレンタルコントロール問い合わせ番号算出
//
//======================================================================
u32 UTL_CalcPCTLInquiryCode( void )
{
	int i;
	u64 num = 0;
	u8  *p  = (u8 *)&num;
	u8  macAddr[ 6 ];
	u32 inquiryCode;
	OS_GetMacAddress( macAddr );
	for( i = 5; i >= 0; i-- ) {
		*p++ = macAddr[ i ];
	}
	inquiryCode = (u32)( num % 100000000UL );
	OS_TPrintf( "InquiryCode : %08d\n", inquiryCode );
	return inquiryCode;
}

