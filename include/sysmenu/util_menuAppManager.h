/*!
  @file menuAppManager.h
  @brief �A�v���}�l�[�W��
 */
#ifndef MENU_APP_MANAGER_H_
#define MENU_APP_MANAGER_H_

#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/banner.h>
#include <twl/lcfg.h>
#include <../build/libraries/os/common/include/application_jump_private.h>
// see also TwlIPL_RED/include/sysmenu/sysmenu_lib/common/sysmenu_api.h
// �^�C�g�����
typedef struct TitleProperty {			// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	NAMTitleId			titleID;		// �^�C�g��ID�iTitleID_Hi�ŋN�����f�B�A�͔���ł���H�j
	LauncherBootFlags	flags;			// �u�[�g���̃����`���[����t���O
	TWLBannerFile		*pBanner;		// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
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


// �A�v���}�l�[�W��

void AMN_Manager( );

void AMN_init( OSArenaId id, OSHeapHandle heap );
void AMN_restart();
void AMN_destroy();

BOOL AMN_isNandTitleListReady();
s32 AMN_getNandTitleListLengthForLauncher();
s32 AMN_getCardTitleListLength();

// �{�̐ݒ�̏ꍇ�A�A�v���}�l�[�W���pindex�͔�є�тɂȂ�����A
// ForSetting()���Ԃ��l(��)���傫���Ȃ�̂�
// getNandTitleListLengthForSetting()�͗p�ӂ��Ȃ��B
// isIndexValidForSetting()�ŁA�{�̐ݒ�p�Ƃ��Ă��L�����ǂ�����
// ���肵�Ă��炤�\��B
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
        u8      WiFiConnectionIcon :1;          // 1: WiFiConnection�������`���[�ŕ\��
        u8      DSWirelessIcon :1;              // 1: DSWirelessIcon�������`���[�ŕ\��
        u8      game_card_on :1;                // 1: NAND�A�v���ŃQ�[���J�[�h�d��ON�i�m�[�}�����[�h�j
        u8      enable_nitro_whitelist_signature :1; // 1: NITRO�z���C�g���X�g�����L���t���O
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
    // �A�v���}�l�[�W���p�^�C�g��index�́A0����n�܂�񕉂ł��B
    // 0��CARD�Œ�ł��B
    // 1����39��NAND�ɂȂ�܂��B
    cAllTitleIndexStart = 0,
    cCardTitleIndexStart = cAllTitleIndexStart,
    cCardTitleArrayMax = 1,
    cNandTitleIndexStart = cCardTitleIndexStart + cCardTitleArrayMax,
    cNandTitleArrayMax = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX,
    cAllTitleArrayMax = cCardTitleArrayMax + cNandTitleArrayMax
};

#endif
