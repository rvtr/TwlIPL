#include <twl.h>
#include <twl/nam.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"

#define WL_TITLEID 0x0003000F484E4341

#define WL_FW_LOADSIZE			0x10
#define WL_FW_LOAD_OFFSET		0xa0
#define WL_FW_VERSION_LO_IDX	0x1
#define WL_FW_VERSION_HI_IDX	0x0
#define WL_FW_VERSION_SIZE		0x2
#define WL_NUM_FW_IDX			0x2
#define WL_NUM_FW_SIZE			0x2
#define WL_FW_TYPE_IDX			0xc
#define WL_FW_TYPE_SIZE			0x4


void getWLInfo( void )
{
	FSFile file;
	char filePath[NAM_PATH_LEN+1];
	char filebuf[WL_FW_LOADSIZE];
	int res;

	FS_InitFile( &file );	
	NAM_GetTitleBootContentPath( filePath , WL_TITLEID); // �����t�@�[���̃t�@�C���p�X���擾
	OS_TPrintf("wireless firm path: %s\n", filePath ) ;
	
	if( FS_OpenFileEx( &file, filePath, FS_FILEMODE_R ) )
	{
		/*
		// �o�[�W�������̓ǂݎ��
		FS_SeekFile( &file, WL_FW_VERSION_OFFSET, FS_SEEK_SET );
		res = FS_ReadFile( &file, filebuf, WL_FW_VERSION_SIZE );
		
		if( res == WL_FW_VERSION_SIZE )
		{
			snprintf( gAllInfo[MENU_WL][WL_VERSION].str.sjis, DISPINFO_BUFSIZE-1, "%d.%d", filebuf[0], filebuf[1] );
			gAllInfo[MENU_WL][WL_VERSION].iValue = filebuf[0] *100 + filebuf[1];
		}
		
		// �t�@�[���E�F�A�̐��̎擾
		FS_SeekFile( &file, WL_NUM_FW, FS_SEEK_SET );
		res = FS_ReadFile( &file, filebuf, 2 );
		
		if( res == WL_NUM_FW_SIZE )
		{
			gAllInfo[MENU_WL][WL_NUM_FW].iValue = filebuf[0];
			gAllInfo[MENU_WL][WL_NUM_FW].isNumData = TRUE;
		}
		
		// �t�@�[���^�C�v�̎擾
		FS_SeekFile( &file, WL_FW_TYPE_OFFSET, FS_SEEK_SET );
		res = FS_ReadFile( &file, filebuf, WL_FW_TYPE_SIZE );
		
		if( res == WL_FW_TYPE_SIZE )
		{
			int value = (int) MI_LoadLE32( filebuf );
			gAllInfo[MENU_WL][WL_FW_TYPE].iValue = value;
			gAllInfo[MENU_WL][WL_FW_TYPE].str.sjis = s_strWLFWType[ value ];	
		}*/
		
		FS_SeekFile( &file, WL_FW_LOAD_OFFSET , FS_SEEK_SET);
		res = FS_ReadFile( &file, filebuf, WL_FW_LOADSIZE);
		
		if( res == WL_FW_LOADSIZE )
		{
			int value;
			
			snprintf( gAllInfo[MENU_WL][WL_VERSION].str.sjis, DISPINFO_BUFSIZE-1, "%d.%d",
						 filebuf[WL_FW_VERSION_HI_IDX], filebuf[WL_FW_VERSION_LO_IDX] );
						 
			gAllInfo[MENU_WL][WL_NUM_FW].iValue = (int) MI_LoadLE8( &filebuf[WL_NUM_FW_IDX] );
			gAllInfo[MENU_WL][WL_NUM_FW].isNumData = TRUE;
			
			value = (int) MI_LoadLE32( &filebuf[WL_FW_TYPE_IDX] );
			gAllInfo[MENU_WL][WL_FW_TYPE].iValue = value;
			gAllInfo[MENU_WL][WL_FW_TYPE].str.sjis = s_strWLFWType[ value ];	
		}
	}
	else
	{
		// �ǂݍ��߂Ȃ�������Ƃ肠����N/A���ē���Ă���
		STD_StrLCpy( gAllInfo[MENU_WL][WL_VERSION].str.sjis, s_strNA, DISPINFO_BUFSIZE );
	}

}