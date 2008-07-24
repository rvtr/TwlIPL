/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - FatalErrorMaker
  File:     fatalErrorMaker.c

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

#include <twl/nam.h>
#include <nitro/nvram/nvram.h>
#include "misc.h"
#include "fatalErrorMaker.h"

#define NORMAL_INFO_PATH "nand:/sys/HWINFO_N.dat"
#define SECURE_INFO_PATH "nand:/sys/HWINFO_S.dat"
#define SHARED_FONT_PATH "nand:/sys/TWLFontTable.dat"
#define WIRELESS_TITLEID 0x0003000F484E4341

#define NOR_BUFSIZE					128
#define NOR_SETTING_ADDRESS_OFFSET	0x20
#define NOR_NANDFLAG_NEGATIVE_OFFSET	( 0x1fe00 - 0x1f380 )
#define NOR_NANDFLAG_MASK	0x10

#define SELECT_MESSAGE	"select data for breaking."
#define SUCCESS_MESSAGE	"breaking data succeeded."
#define FAIL_MESSAGE	"breaking data failed."
#define ALREADY_MESSAGE	"this data is broken already."

#define MENU_LINES 5
#define INFORMATION_LEFT	20
#define INFORMATION_UP		150
#define SELECTION_LEFT		40
#define RESULT_LEFT			190
#define SELECTION_UP		30
#define ALLOW_LEFT			25
#define LINE_OFFSET			20
#define HEADER_LEFT			5
#define HEADER_UP			0

typedef enum FatalErrorTarget {
	BREAK_HW_NORMAL = 0,
	BREAK_HW_SECURE = 1,
	BREAK_FONT = 2,
	BREAK_WIRELESS = 3,
	BREAK_NAND = 4
} FatalErrorTarget;


typedef enum ControlResult {
	CONTROL_RESULT_INITIAL = 0,
	CONTROL_RESULT_SUCCESS = 1,
	CONTROL_RESULT_FAILED = 2,
	CONTROL_RESULT_ALREADY = 3,
	CONTROL_RESULT_MOVE = 4,
	CONTROL_RESULT_NONE = -1
} ControlResult;

ControlResult control( int *line, u8 *flag );
void draw( int select, int result, int flag );
BOOL breakNAND( void );
BOOL breakData( FatalErrorTarget tgt );
static void DumpBinary(u32 offset, void* data, u32 size);
void setParity( u8 *data );

static char* s_strInfo[4];
static char* s_strMenu[ MENU_LINES ];
static char* s_strFilePath[ MENU_LINES ];

void fatalMakerMain( void )
{
	static int selectLine = 0;
	static u8 brokenFlag = 0; // 0: NORMAL, 1: SECURE, 2: FONT, 3: WL, 4: NAND
	ControlResult result ;
		
	result = control( &selectLine, &brokenFlag ); // 入力を呼んで状態を更新
	
	// 画面の描画
	draw( selectLine, result , brokenFlag );
}

void draw( int select, int result, int flag )
{
	static int lastBreakData;
	static int lastStatus = CONTROL_RESULT_INITIAL;
	int i;

	// 何らかの決定動作があった場合はlastBreakDataを更新
	if( result != CONTROL_RESULT_NONE  && result != CONTROL_RESULT_MOVE )
	{
		lastBreakData = select;
		lastStatus = result;
	}

	// 操作があったときはキャンパスクリア
	if( result != CONTROL_RESULT_NONE )
	{
		NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	}

	// 項目の羅列
	for( i = 0; i < MENU_LINES; i++ )
	{
		if( i == select )
		{
			PrintfSJIS( SELECTION_LEFT, SELECTION_UP + (i * LINE_OFFSET), TXT_COLOR_GREEN, s_strMenu[i] );
		}
		else
		{
			PrintfSJIS( SELECTION_LEFT, SELECTION_UP + (i * LINE_OFFSET), TXT_COLOR_BLACK, s_strMenu[i] );
		}
		
		// 破壊したかどうかの表示
		if( flag & (1 << i) )
		{
			PrintfSJIS( RESULT_LEFT, SELECTION_UP + (i * LINE_OFFSET), TXT_COLOR_RED, "BROKEN" );
		}
	}

	PrintfSJIS( HEADER_LEFT, HEADER_UP, TXT_COLOR_RED, "Fatal Error Maker" );	
	PrintfSJIS( INFORMATION_LEFT, INFORMATION_UP, TXT_COLOR_BLACK, s_strInfo[ lastStatus ] );
	PrintfSJIS( ALLOW_LEFT, SELECTION_UP + (select * LINE_OFFSET), TXT_COLOR_BLACK, "→" );

}

ControlResult control( int *line, u8 *flag )
{
	BOOL controlFlag = FALSE; // 何らかの操作があったかどうかをメモる
	
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		// デクリメントしてマイナスになったらループ
		if( --*line < 0 )
		{
			*line = MENU_LINES-1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		// インクリメントしてmaxを超えたらループ
		if( MENU_LINES <= ++*line )
		{
			*line = 0;
		}
	}
	
	if( pad.trg & PAD_BUTTON_A )
	{
		controlFlag = TRUE;
		
		// 選択項目を破壊する
		if( *flag & (1 << *line) )
		{
			// フラグが立ってたら既に破壊済みなので何もしない
			return CONTROL_RESULT_ALREADY;
		}
		
		// データを実際に破壊する
		if( ! breakData( (FatalErrorTarget)*line ) )
		{
			// 破壊に失敗
			return CONTROL_RESULT_FAILED;
		}
		
		// 破壊成功
		*flag |= 1 << *line ;
		return CONTROL_RESULT_SUCCESS;
	}
	
	// 特に何も破壊しない
	return controlFlag ? CONTROL_RESULT_MOVE : CONTROL_RESULT_NONE;
}

BOOL breakData( FatalErrorTarget tgt )
{
	switch( tgt )
	{
		case BREAK_FONT:
		case BREAK_HW_NORMAL:
		case BREAK_HW_SECURE:
		case BREAK_WIRELESS:
			// SharedFont, NormalInfo, SecureInfoについては該当パスきめうちで削除してしまう
			if( ! FS_DeleteFile( s_strFilePath[tgt] ) )
			{
				OS_TPrintf( "FS_DeleteFile Error. file: %s err: %d\n" ,
					s_strFilePath[tgt], FS_GetArchiveResultCode( s_strFilePath[tgt] ) );
				
				return FALSE;
			}

			OS_TPrintf( "delete file succeeded. : %s\n", s_strFilePath[tgt] );
			return TRUE;
			
		case BREAK_NAND:
			return breakNAND() ;
	}

	// illegal argument
	return FALSE;
}

BOOL breakNAND( void )
{
	static u8  buf[NOR_BUFSIZE] ;
	u32 settingAddress;
	NVRAMResult result;
	
	
	// まずIPLヘッダから本体設定のあるオフセットを取得

	// キャッシュを破棄
	DC_InvalidateRange( buf, NOR_BUFSIZE);
	result = NVRAMi_Read( NOR_SETTING_ADDRESS_OFFSET , NOR_BUFSIZE, buf );
	
	if( result != NVRAM_RESULT_SUCCESS )
	{
		// readに失敗
		OS_TPrintf( "nvram_read() failed. errorcode: %d\n", result );
		return FALSE;
	}
	
	OS_TPrintf("reading address... %x %x \n", buf[0], buf[1] );

	// 格納されている値の8倍が設定アドレスの開始アドレス
	settingAddress = ( (u32)(buf[1]) << 8 | (u32)buf[0]) * 8 ;
	OS_TPrintf("settingAddress : %x\n", settingAddress );
	
	DC_InvalidateRange( buf, NOR_BUFSIZE);
	result = NVRAMi_Read( settingAddress - NOR_NANDFLAG_NEGATIVE_OFFSET , NOR_BUFSIZE, buf );

	if( result != NVRAM_RESULT_SUCCESS )
	{
		// readに失敗
		OS_TPrintf( "nvram_read() failed. errorcode: %d\n", result );
		return FALSE;
	}
	
	DumpBinary( settingAddress - NOR_NANDFLAG_NEGATIVE_OFFSET , buf, NOR_BUFSIZE );
			
	OS_TPrintf(" now flag : %x\n", buf[NOR_BUFSIZE-1] );
	buf[ NOR_BUFSIZE - 1 ] |= NOR_NANDFLAG_MASK;
	setParity( &buf[NOR_BUFSIZE-1] );	
	OS_TPrintf(" now flag : %x\n", buf[NOR_BUFSIZE-1] );

	// キャッシュを書き戻して、それからnvramに書き込む
	DC_StoreRange(buf, sizeof(buf));
	result = NVRAMi_Write( settingAddress - NOR_NANDFLAG_NEGATIVE_OFFSET , NOR_BUFSIZE, buf );
	
	// 読み直す
	DC_InvalidateRange( buf, NOR_BUFSIZE);
	result = NVRAMi_Verify( settingAddress - NOR_NANDFLAG_NEGATIVE_OFFSET , NOR_BUFSIZE, buf );

	if( result != NVRAM_RESULT_SUCCESS )
	{
		// writeに失敗
		OS_TPrintf( "nvram_verify() failed. errorcode: %d\n", result );
		return FALSE;
	}
	
	// 書き込み成功
	return TRUE;
}

void fatalMakerInit( void )
{
	static char filePath[NAM_PATH_LEN+1];
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	// 無線ファームのパスを取得する
	NAM_GetTitleBootContentPath( filePath , WIRELESS_TITLEID); // 
	s_strFilePath[ BREAK_WIRELESS ] = filePath;
	OS_TPrintf( "wireless path : %s\n", filePath );
}

static char* s_strInfo[] = {
	SELECT_MESSAGE,
	SUCCESS_MESSAGE,
	FAIL_MESSAGE,
	ALREADY_MESSAGE
};

static char* s_strMenu[] = {
	"HW Normal Info",
	"HW Secure Info",
	"Shared Font",
	"Wireless Firmware",
	"NAND"
};

static char* s_strFilePath[] = {
	NORMAL_INFO_PATH,
	SECURE_INFO_PATH,
	SHARED_FONT_PATH,
	NULL,	// 無線ファームのパスはあとからNAM経由で入れる
	NULL
};


/*---------------------------------------------------------------------------*
  Name:         DumpBinary
  Description:  バイナリデータを 16 進数表記でダンプ出力する。
  Arguments:    offset  -   先頭アドレスを指定。
                data    -   ダンプ表示するデータへのポインタ。
                size    -   ダンプ表示するデータ量をバイト単位で指定。
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
DumpBinary(u32 offset, void* data, u32 size)
{
    s32     i;

    if (size > 0)
    {
        if ((offset % 16) != 0)
        {
            OS_TPrintf("%08x:", (u32)(offset & ~0xf));
            for (i = 0; i < (offset % 16); i ++)
            {
                OS_TPrintf("   ");
                if ((i % 16) == 7)
                {
                    OS_TPrintf(" -");
                }
            }
        }
        for (i = 0; i < size; i ++)
        {
            if (((offset + i) % 16) == 0)
            {
                OS_TPrintf("%08x:", offset + i);
            }
            OS_TPrintf(" %02x", ((u8*)data)[i]);
            if (((offset + i) % 16) == 7)
            {
                OS_TPrintf(" -");
            }
            else if (((offset + i) % 16) == 15)
            {
                OS_TPrintf("\n");
            }
        }
        if (((offset + i  - 1) % 16) != 15)
        {
            OS_TPrintf("\n");
        }
    }
}

void setParity( u8 *data )
{
    u8 i, parity;

    parity = 1;           /* 通常と逆（1が奇数個で0になる）*/
    for( i=0; i<7; i++) { /* 0-7（最上位ビットがparity） */
        parity += ((*data >> i) & 0x01);
    }
    /* パリティビットを上書き */
    *data &= (u8)(~(0x80));
    *data |= ((parity << 7) & 0x80);
}