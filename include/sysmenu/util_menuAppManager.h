/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util_menuAppManager.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef MENU_APP_MANAGER_H_
#define MENU_APP_MANAGER_H_

#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/banner.h>
#include <twl/lcfg.h>
#include <../build/libraries/os/common/include/application_jump_private.h>

#include <sysmenu/types.h>

typedef struct AMNBannerCounter
{
	u32 control;
	u8 count;
	const TWLBannerFile *banner;
}
AMNBannerCounter;

typedef struct AMNFrameAnimeData{
	const u8 *image;
	const u8 *pltt;
	BOOL vflip;
	BOOL hflip;
}
AMNFrameAnimeData;


// �A�v���}�l�[�W��

void AMN_init( void *(*pAlloc)(u32), void (*pFree)(void*) );
void AMN_restart();
void AMN_restartWithReadNandTitle();
void AMN_restartWithReadNandTitleHeaderShort();
void AMN_destroy();

BOOL AMN_isNandTitleListReady();
s32 AMN_getNandTitleListLengthForLauncher();
s32 AMN_getCardTitleListLength();
s32 AMN_getRomHeaderListLength();

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

const TitleProperty*  AMN_getTitleProperty(s32 index);
const ROM_Header_Short*  AMN_getTitleRomHeaderShort(s32 index);

const u8*       AMN_getBannerImage(s32 index);
const u8*       AMN_getBannerPltt(s32 index);
BOOL            AMN_getBannerHFlip(s32 index);
BOOL            AMN_getBannerVFlip(s32 index);
AMNFrameAnimeData  AMN_getBannerAnime(s32 index);
const u16*      AMN_getBannerText(s32 index);
const u16*      AMN_getBannerText2(s32 index, OSLanguage language);

BOOL            AMN_isDSWirelessIcon(s32 index);
BOOL            AMN_isWiFiConnectionIcon(s32 index);
BOOL            AMN_isAgreeEULAFlag(s32 index);
// info size is cParentalControlRatingInfoSize
const u8*       AMN_getParentalControlRatingInfo(s32 index);

// ���f�[�^�iTitleProperty�̃��X�g�AHeaderShort���X�g�j�ɃA�N�Z�X�ł��锲�����֐�
TitleProperty* AMN_getTitlePropertyList( void );
ROM_Header_Short* AMN_getRomHeaderList( void );

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
