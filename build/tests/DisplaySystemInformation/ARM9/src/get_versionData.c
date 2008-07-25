
#include <twl.h>
#include <twl/os/common/sharedFont.h>
#include <twl/nam.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"
#include "misc.h"


#define WL_TITLEID 0x0003000F484E4341

void getWirelessVersion( void );
void getContentsVersion( void );
void getSharedFontVersion( void );


void getVersions( void )
{
	
	getWirelessVersion();
	getContentsVersion();
	getSharedFontVersion();

}

void getWirelessVersion( void )
// �����t�@�[���E�F�A�̃o�[�W�������擾����
{
	FSFile file;
	char filePath[NAM_PATH_LEN+1];
	char filebuf[2];	// FW�o�[�W������Major, Minor���ꂼ��1�o�C�g����
	int res;

	FS_InitFile( &file );	
	NAM_GetTitleBootContentPath( filePath , WL_TITLEID); // �����t�@�[���̃t�@�C���p�X���擾
	OS_TPrintf("wireless firm path: %s\n", filePath ) ;
	res = FS_OpenFileEx( &file, filePath, FS_FILEMODE_R );
	
	// �o�[�W�������̓ǂݎ��
	FS_SeekFile( &file, 0xA0, FS_SEEK_SET ); // �t�@�C����0xA0����2�o�C�g���o�[�W�������
	res = FS_ReadFile( &file, filebuf, 2 );
	SDK_ASSERT( res == 2 );
	
	snprintf( gAllInfo[MENU_VERSION][VERSION_WIRELESS].str.sjis, DISPINFO_BUFSIZE-1, "%d.%d", filebuf[0], filebuf[1] );
	gAllInfo[MENU_VERSION][VERSION_WIRELESS].iValue = filebuf[0] *100 + filebuf[1];

}

void getSharedFontVersion( void )
{
	u32 time;
	
	OS_InitSharedFont();
	time = OS_GetSharedFontTimestamp();
	OS_TPrintf("SharedFont Time Stamp %08lx\n", time );
	gAllInfo[MENU_VERSION][VERSION_FONT].iValue = (int) time;
	gAllInfo[MENU_VERSION][VERSION_FONT].isNumData = TRUE;
}

void getContentsVersion( void )
// �R���e���c���X�g�����ƂɊe�R���e���c�̃^�C�g��ID�ƃo�[�W�������擾
{
	NAMTitleInfo info;
	int i;

	gNumContents = NAM_GetNumTitles();
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


