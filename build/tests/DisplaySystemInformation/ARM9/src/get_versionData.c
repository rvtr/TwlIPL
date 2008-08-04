
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
// �R���e���c���X�g�����ƂɊe�R���e���c�̃^�C�g��ID�ƃo�[�W�������擾
{
	NAMTitleInfo info;
	int i;

	gNumContents = NAM_GetNumTitles();
	
	if( gNumContents < 0 )
	{
		// NAM���ʖڂ��Ƃ��ANAND�A�N�Z�X�ł��Ȃ����Ƃ��̏ꍇ�͏I��
		return ;
	}	
	
	OS_TPrintf(" numContents: %d\n", gNumContents);	
	
	if( gContentsTitle == NULL )
	{
		// ���񏈗��̎��̓o�b�t�@���m��
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


