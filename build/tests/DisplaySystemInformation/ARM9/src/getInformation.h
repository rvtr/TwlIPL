
#ifndef __DISPLAY_INFO_GETINFORMATION__

#include <twl/os/common/sharedFont.h>

#define __DISPLAY_INFO_GETINFORMATION__

// 初回読み込みか否か
static BOOL firstRead;

// NANDアプリ情報
extern s32 gNumContents;
extern OSTitleId *gContentsTitle;
extern u16 *gContentsVersion;

#ifndef VERSION_VIEWER

#define TWL_SYSMENU_VER_STR_LEN			28				// システムメニューバージョン文字列MAX bytes
#define TWL_EULA_URL_LEN				128
#define TWL_NUP_HOSTNAME_LEN			64

#define NUM_FONT_INFO		3	// フォント一つ当たりのメニュー数。名前、サイズ、ハッシュの三項目

typedef struct FontInfo{
	u8 *name;	// お名前
	u32 size;	// 大きさ
	u8 *data;	// データ本体
	u8 *hash;	// データのハッシュ値
	BOOL isHashOK;	// ハッシュ値が既知のものと一致するか
} FontInfo;



// 共有フォント情報
extern FontInfo gFontInfo[ OS_SHARED_FONT_MAX ];

// LCFGデータを読み込むためのバッファ
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