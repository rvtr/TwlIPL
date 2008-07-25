
#ifndef __DISPLAY_INFO_GETINFORMATION__
#define __DISPLAY_INFO_GETINFORMATION__

#define TWL_SYSMENU_VER_STR_LEN			28				// �V�X�e�����j���[�o�[�W����������MAX bytes
#define TWL_EULA_URL_LEN				128
#define TWL_NUP_HOSTNAME_LEN			64

// NAND�A�v�����
extern s32 gNumContents;
extern OSTitleId *gContentsTitle;
extern u16 *gContentsVersion;

// LCFG�f�[�^��ǂݍ��ނ��߂̃o�b�t�@
extern u8 *bufLCFG;


void getSysmenuInfo( void );
void getVersions( void );
void getSCFGInfo( void );
void getOwnerInfo( void );
void getHWInfo( void );
void getParentalInfo( void );
void getOtherInfo( void );

#endif