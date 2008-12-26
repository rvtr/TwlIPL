
#ifndef __DISPLAY_INFO_GETINFORMATION__

#include <twl/os/common/sharedFont.h>

#define __DISPLAY_INFO_GETINFORMATION__

// ����ǂݍ��݂��ۂ�
static BOOL firstRead;

// NAND�A�v�����
extern s32 gNumContents;
extern OSTitleId *gContentsTitle;
extern u16 *gContentsVersion;

#ifndef VERSION_VIEWER

#define TWL_SYSMENU_VER_STR_LEN			28				// �V�X�e�����j���[�o�[�W����������MAX bytes
#define TWL_EULA_URL_LEN				128
#define TWL_NUP_HOSTNAME_LEN			64

#define NUM_FONT_INFO		3	// �t�H���g�������̃��j���[���B���O�A�T�C�Y�A�n�b�V���̎O����

typedef struct FontInfo{
	u8 *name;	// �����O
	u32 size;	// �傫��
	u8 *data;	// �f�[�^�{��
	u8 *hash;	// �f�[�^�̃n�b�V���l
	BOOL isHashOK;	// �n�b�V���l�����m�̂��̂ƈ�v���邩
} FontInfo;



// ���L�t�H���g���
extern FontInfo gFontInfo[ OS_SHARED_FONT_MAX ];

// LCFG�f�[�^��ǂݍ��ނ��߂̃o�b�t�@
extern u8 *bufLCFG;

#endif // ifndef VERSION_VIEWER

void getVersions( void );
void getContentsVersion( void );

#ifndef VERSION_VIEWER
void getSysmenuInfo( void );
void getSCFGInfo( void );
void getOwnerInfo( void );
void getHWInfo( void );
void getParentalInfo( void );
void getOtherInfo( void );
void getSecureUserInfo( void );
void getFontInfo( void );
void getWLInfo( void );
void getWhiteListInfo( void );

#endif // ifndef VERSION_VIEWER

#endif // ifdef __DISPLAY_INFO_GETINFORMATION__