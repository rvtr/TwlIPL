#include <twl.h>
#include <twl/os/common/sharedFont.h>

#include "misc.h"
#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"
#include "util.h"

#define NUM_FONT_HASHDATA	3

const static char* s_strFontHash[] = {
	"07381F6C1FF7421BA309196FA9FC750706B031AD",
	"EF271DAD8B67EDB3DDE2655C6C35288D1334132D",
	"803410DCAF5647134C9B68DA9ADEB562D212DCFC"
};

FontInfo gFontInfo[ OS_SHARED_FONT_MAX ];

void loadFontName( OSSharedFontIndex idx );
void loadFont( OSSharedFontIndex idx );
BOOL checkFontHash( OSSharedFontIndex idx );


void getFontInfo( void )
{
	u32 time;
	int tableSize = 0;
	u8 *tableBuf;
	OSSharedFontIndex fontIdx;
	
	OS_InitSharedFont();

	// フォントテーブルバッファの確保
	tableSize = OS_GetSharedFontTableSize();
	tableBuf = (u8*) Alloc ( (u32)tableSize );
	
	time = OS_GetSharedFontTimestamp();
	OS_TPrintf("SharedFont Time Stamp %08lx\n", time );
	gAllInfo[MENU_FONT][FONT_TIMESTAMP].iValue = (int) time;
	gAllInfo[MENU_FONT][FONT_TIMESTAMP].isNumData = TRUE;
	
	if( ! OS_LoadSharedFontTable( (void*)tableBuf ) )
	{
		OS_TPrintf("Loading shared font table failed.\n");
		return;
	}
	
	for( fontIdx = (OSSharedFontIndex) 0; fontIdx < OS_SHARED_FONT_MAX; fontIdx++ )
	{
		loadFontName( fontIdx );
		
		// サイズ取得
		gFontInfo[fontIdx].size = (u32) OS_GetSharedFontSize( (OSSharedFontIndex)fontIdx );
		
		loadFont( fontIdx );
		
		gFontInfo[fontIdx].isHashOK = checkFontHash( fontIdx );
	}
	
	Free(tableBuf);
	s_numMenu[MENU_FONT] = FONT_INFO + ( NUM_FONT_INFO * OS_SHARED_FONT_MAX );
}

void loadFontName( OSSharedFontIndex idx )
{
	// 名前取得
	const u8* fontName = OS_GetSharedFontName( (OSSharedFontIndex)idx );
	int fontNameLength = STD_StrLen( (char*)fontName );
		
	if( gFontInfo[idx].name == NULL || 
		( sizeof( gFontInfo[idx].name ) <= fontNameLength ))
	{
		// 名前のところにメモリが割り当てられてなかったり、
		// 小さかったりした場合は割り当てなおす。
		
		// NsysのFreeはNULLを渡すと落ちる仕様・・・
		if( gFontInfo[idx].name != NULL )
		{
			Free( gFontInfo[idx].name );
		}
		
		gFontInfo[idx].name = (u8*) Alloc ( sizeof(u8) * (fontNameLength + 1) );
	}
	
	SDK_ASSERT( gFontInfo[idx].name );
	STD_StrLCpy( (char*)gFontInfo[idx].name, (char*)fontName, fontNameLength + 1 );
}

void loadFont( OSSharedFontIndex idx )
{
	// でも別に普段必要なわけじゃないからメモリ確保のためにも
	// ハッシュ値取り終わったら開放するようにするかな？
	if( gFontInfo[idx].data == NULL || 
		( sizeof( gFontInfo[idx].data ) <= gFontInfo[idx].size ))
	{
		// データバッファにメモリが割り当てられてなかったり、
		// 小さかったりした場合は割り当てなおす。
		
		if( gFontInfo[idx].data != NULL )
		{
			Free( gFontInfo[idx].data );
		}
		
		
		gFontInfo[idx].data = (u8*) Alloc ( gFontInfo[idx].size );
	}
	
	SDK_ASSERT( gFontInfo[idx].data );
	OS_LoadSharedFont( idx, (void*)gFontInfo[idx].data );
}	

BOOL checkFontHash( OSSharedFontIndex idx )
{	
	u8 digestBuf[MATH_SHA1_DIGEST_SIZE], cmpBuf[MATH_SHA1_DIGEST_SIZE];
	
	// 実態、検証用それぞれのハッシュを算出
	strToHexa( s_strFontHash[idx], cmpBuf, MATH_SHA1_DIGEST_SIZE );
	MATH_CalcSHA1( digestBuf, gFontInfo[idx].data , gFontInfo[idx].size  );
	
	return MI_CpuComp8( cmpBuf, digestBuf, MATH_SHA1_DIGEST_SIZE ) == 0;
}