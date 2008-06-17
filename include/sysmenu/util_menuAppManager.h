/*!
  @file menuAppManager.h
  @brief アプリマネージャ
 */
#ifndef MENU_APP_MANAGER_H_
#define MENU_APP_MANAGER_H_

#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/banner.h>
#include <twl/lcfg.h>
#include <../build/libraries/os/common/include/application_jump_private.h>
// see also TwlIPL_RED/include/sysmenu/sysmenu_lib/common/sysmenu_api.h
// タイトル情報
typedef struct TitleProperty {			// この情報は、ランチャー時には認証通ってないけど、起動時には認証通すので大丈夫だろう。
	NAMTitleId			titleID;		// タイトルID（TitleID_Hiで起動メディアは判定できる？）
	LauncherBootFlags	flags;			// ブート時のランチャー動作フラグ
	TWLBannerFile		*pBanner;		// バナーへのポインタ（固定長フォーマットなら偽造されても大丈夫だろう。)
}TitleProperty;

typedef struct BannerCounter
{
	u32 control;
	u8 count;
	const TWLBannerFile *banner;
}
BannerCounter;

typedef struct FrameAnimeData{
	const u8 *image;
	const u8 *pltt;
	BOOL vflip;
	BOOL hflip;
}
FrameAnimeData;


// アプリマネージャ

void AMN_Manager( );

void AMN_init( OSArenaId id, OSHeapHandle heap );
void AMN_restart();
void AMN_destroy();

BOOL AMN_isNandTitleListReady();
s32 AMN_getNandTitleListLengthForLauncher();
s32 AMN_getCardTitleListLength();

// 本体設定の場合、アプリマネージャ用indexは飛び飛びになったり、
// ForSetting()が返す値(個数)より大きくなるので
// getNandTitleListLengthForSetting()は用意しない。
// isIndexValidForSetting()で、本体設定用としても有効かどうかを
// 判定してもらう予定。
BOOL        AMN_isIndexValidForSetting(s32 index);
s32         AMN_getIndexByTitleId(NAMTitleId titleId);
NAMTitleId  AMN_getTitleIdByIndex(s32 index);

void            AMN_stepBannerAnimeAll(BOOL restart);
void            AMN_stepBannerAnime(s32 index, BOOL restart);

// see also TwlSDK/include/twl/os/common/format_rom.h
// 0x1BF - TWL expansion flags
typedef union {
    u8      packed;
    struct {
        u8      codec_mode:1;                   // 0:NTR mode, 1:TWL mode       // undeveloped
        u8      otherParentalControls:1;        // 1: Enable OtherParentalControls
        u8      subBannerFile:1;                // 1: Enable subBannerFile
        u8      WiFiConnectionIcon :1;          // 1: WiFiConnectionをランチャーで表示
        u8      DSWirelessIcon :1;              // 1: DSWirelessIconをランチャーで表示
        u8      game_card_on :1;                // 1: NANDアプリでゲームカード電源ON（ノーマルモード）
        u8      enable_nitro_whitelist_signature :1; // 1: NITROホワイトリスト署名有効フラグ
        u8      rsv:1;
    };
} ExpansionFlags;
enum {
    // 0x02f0 - 0x0300 Parental Controls Rating Info
    cParentalControlRatingInfoSize = 0x10
};
typedef struct {
    TitleProperty   prop;
    u8              parental_control_rating_info[ cParentalControlRatingInfoSize ];
    ExpansionFlags  expansionFlags;
} AppInfo;
const AppInfo*  AMN_getAppInfo(s32 index);

const u8*       AMN_getBannerImage(s32 index);
const u8*       AMN_getBannerPltt(s32 index);
BOOL            AMN_getBannerHFlip(s32 index);
BOOL            AMN_getBannerVFlip(s32 index);
FrameAnimeData  AMN_getBannerAnime(s32 index);
const u16*      AMN_getBannerText(s32 index);
const u16*      AMN_getBannerText2(s32 index, OSLanguage language);

BOOL            AMN_isDSWirelessIcon(s32 index);
BOOL            AMN_isWiFiConnectionIcon(s32 index);
BOOL            AMN_isOtherParentalControlsFlag(s32 index);
// info size is cParentalControlRatingInfoSize
const u8*       AMN_getParentalControlRatingInfo(s32 index);

enum {
    // アプリマネージャ用タイトルindexは、0から始まり非負です。
    // 0はCARD固定です。
    // 1から39がNANDになります。
    cAllTitleIndexStart = 0,
    cCardTitleIndexStart = cAllTitleIndexStart,
    cCardTitleArrayMax = 1,
    cNandTitleIndexStart = cCardTitleIndexStart + cCardTitleArrayMax,
    cNandTitleArrayMax = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX,
    cAllTitleArrayMax = cCardTitleArrayMax + cNandTitleArrayMax
};

#endif
