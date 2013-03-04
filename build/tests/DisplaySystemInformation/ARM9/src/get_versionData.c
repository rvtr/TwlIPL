
#include <twl.h>
#include <twl/os/common/sharedFont.h>
#include <twl/nam.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"
#include "misc.h"

void getWirelessVersion( void );
void getContentsVersion( void );
void getSharedFontVersion( void );


void getContentsVersion( void )
// コンテンツリストをもとに各コンテンツのタイトルIDとバージョンを取得
{
	NAMTitleInfo info;
	int i;

	gNumContents = NAM_GetNumTitles();
	
	if( gNumContents < 0 )
	{
		// NAMが駄目だとか、NANDアクセスできないだとかの場合は終了
		return ;
	}	
	
	OS_TPrintf(" numContents: %d\n", gNumContents);	
	
	if( gContentsTitle == NULL )
	{
		// 初回処理の時はバッファを確保
		gContentsTitle = (NAMTitleId*) Alloc( sizeof(NAMTitleId) * gNumContents );
		gContentsVersion = (u16*) Alloc( sizeof(u16) * (u32)gNumContents);
		SDK_ASSERT( gContentsTitle );
		SDK_ASSERT( gContentsVersion );
	}
			
	NAM_GetTitleList( gContentsTitle, (u32)gNumContents);
	SDK_POINTER_ASSERT( gContentsTitle );
	SDK_POINTER_ASSERT( gContentsVersion );

	for( i=0; i<gNumContents; i++ )
	{
		NAM_ReadTitleInfo( &info, gContentsTitle[i] );
//		gContentsTitle[i] = info.titleId;
		gContentsVersion[i] = info.version;
	}
	
	s_numMenu[MENU_VERSION] = gNumContents + VERSIONMENU_SIZE;
}


