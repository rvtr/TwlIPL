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

	// �t�H���g�e�[�u���o�b�t�@�̊m��
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
		
		// �T�C�Y�擾
		gFontInfo[fontIdx].size = (u32) OS_GetSharedFontSize( (OSSharedFontIndex)fontIdx );
		
		loadFont( fontIdx );
		
		gFontInfo[fontIdx].isHashOK = checkFontHash( fontIdx );
	}
	
	Free(tableBuf);
	s_numMenu[MENU_FONT] = FONT_INFO + ( NUM_FONT_INFO * OS_SHARED_FONT_MAX );
}

void loadFontName( OSSharedFontIndex idx )
{
	// ���O�擾
	const u8* fontName = OS_GetSharedFontName( (OSSharedFontIndex)idx );
	int fontNameLength = STD_StrLen( (char*)fontName );
		
	if( gFontInfo[idx].name == NULL || 
		( sizeof( gFontInfo[idx].name ) <= fontNameLength ))
	{
		// ���O�̂Ƃ���Ƀ����������蓖�Ă��ĂȂ�������A
		// �����������肵���ꍇ�͊��蓖�ĂȂ����B
		
		// Nsys��Free��NULL��n���Ɨ�����d�l�E�E�E
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
	// �ł��ʂɕ��i�K�v�Ȃ킯����Ȃ����烁�����m�ۂ̂��߂ɂ�
	// �n�b�V���l���I�������J������悤�ɂ��邩�ȁH
	if( gFontInfo[idx].data == NULL || 
		( sizeof( gFontInfo[idx].data ) <= gFontInfo[idx].size ))
	{
		// �f�[�^�o�b�t�@�Ƀ����������蓖�Ă��ĂȂ�������A
		// �����������肵���ꍇ�͊��蓖�ĂȂ����B
		
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
	
	// ���ԁA���ؗp���ꂼ��̃n�b�V�����Z�o
	strToHexa( s_strFontHash[idx], cmpBuf, MATH_SHA1_DIGEST_SIZE );
	MATH_CalcSHA1( digestBuf, gFontInfo[idx].data , gFontInfo[idx].size  );
	
	return MI_CpuComp8( cmpBuf, digestBuf, MATH_SHA1_DIGEST_SIZE ) == 0;
}