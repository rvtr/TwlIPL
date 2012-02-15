/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_VCW.h

  Copyright (C)2009-2011 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#ifndef __LGY_VCW_H__
#define __LGY_VCW_H__

#include <twl.h>
#include <sysmenu.h>

#define UBI_PLAYERINFO_TUTORIALSEENFLAG_SIZE		3
#define UBI_PLAYERINFO_NAME_LENGTH                      12
#define UBI_PLAYERINFO_UNLOCK_LETTERMASK_COUNT		40		//1 for each alphabet letter, including accented letters
#define UBI_PLAYERINFO_UNLOCK_LETTERMASK_SIZE		(UBI_PLAYERINFO_UNLOCK_LETTERMASK_COUNT/4)
#define UBI_PLAYERINFO_DATAITEM_MAXCOUNT		2900
#define UBI_PLAYERINFO_DATAITEM_SAVE_MAXSIZE		(UBI_PLAYERINFO_DATAITEM_MAXCOUNT/2)
#define UBI_PLAYERINFO_THEMEDATAITEM_MAXCOUNT		160
#define UBI_PLAYERINFO_THEMEDATAITEM_SAVE_MAXSIZE	(UBI_PLAYERINFO_THEMEDATAITEM_MAXCOUNT/2)
// defines for MWC2
#define UBI_PLAYERINFO_TOP_SCORE_NUM			4
#define UBI_PLAYERINFO_LAST_GAME_SCORES_NUM		5
#define UBI_PLAYERINFO_SESSION_STARS    		100 // 100 is temp for now
#define UBI_PLAYERINFO_CROSSWORD_COUNT                  20
#define UBI_PLAYERINFO_CROSSWORD_NO_SCORE               0xF
#define UBI_PLAYERINFO_CROSSWORD_MIN_RANK               10

#define GRID_SIZE                                       11

enum E_GameIDs
{	
    UBI_PLAYERINFO_GAMES_MISMATCH_ID = 0,
    UBI_PLAYERINFO_GAMES_NETWORDER_ID, // 1
    UBI_PLAYERINFO_GAMES_QUOTES_ID,    // 2
    UBI_PLAYERINFO_GAMES_IDIOMS_ID,    // 3
    UBI_PLAYERINFO_GAMES_TRACE_ID,     // 4
    UBI_PLAYERINFO_GAMES_RACE_ID,      // 5
    UBI_PLAYERINFO_GAMES_WORDFALL_ID,  // 6
    UBI_PLAYERINFO_GAMES_LADDER_ID,    // 7
    UBI_PLAYERINFO_GAMES_CROSSWORD_ID, // 8
    UBI_PLAYERINFO_GAMES_HIDDENWORD_ID,// 9
    UBI_PLAYERINFO_GAMES_BLUFF_ID,     // 10
    UBI_PLAYERINFO_GAMES_HANGMAN_ID,   // 11
    UBI_PLAYERINFO_GAMES_MUMBLE_ID,    // 12
    UBI_PLAYERINFO_GAMES_UNLOCK_ID,    // 13
    UBI_PLAYERINFO_GAMES_NUM,          // 14
    UBI_PLAYERINFO_GAMES_NONE_ID
};


typedef struct VCW_SavegameHeader
{
    u32 uniqueIdentifier;
    u16 checkSum;
    u8 m_status;
    u8 padTemp;
}VCW_SavegameHeader;

struct ScoreInfo
{
    u16 m_score;  //represents either stars or points, whatever
    u32 m_time;   // could be u16
};

struct ActivePassiveData
{
    u16 m_activeWords;
    u16 m_passiveWords;
};

struct UnlockGameData
{
    u16 m_availableGuesses;
    u16 m_currentQuoteIndex;
    u8 m_letterStateMask[UBI_PLAYERINFO_UNLOCK_LETTERMASK_SIZE];		    
    u8 m_pad[2];					//for 32-bit alignment
};

struct UnlockData
{
    u8 m_lockState;
    u16 m_availableStars;
};

struct HiddenWordData
{
    char gridLetters[11][11];
    struct WordData
    {
        u16 m_posX   : 4;
        u16 m_posY   : 4;
        u16 m_length : 4;
        u16 m_axis   : 4;
    }m_words[12];

// u8 wordsStartPos[12][2];
// u8 wordsLenght[12];
// u8 wordsAxis[12];
    u8 amountOfWords;
    u16 flagWordsFound;	 // The last bit (15) is used to show the presence of a hidden word save game.
    u32 timeElapsed;
    u8 hintsUsed;
};

// Used for dynamically generated crosswords
struct CrosswordData
{
    u8 m_pregenGridId;

    u8 m_helpUsed     : 1;
    u8 m_saveIsValid  : 1;
    u8 m_timerIndex   : 6;

    // Letter displayed in the grid cell, 1st bit is bool for Cheated ( or Not ).
    char m_gridLetters[GRID_SIZE][GRID_SIZE];  // Upper Case Ascii value

    // Add timer data
    u32 m_elapsedTime; // In miliseconds
};

#define MAX_WORD_COUNT    30

struct DynamicCrosswordData
{
    struct Word
    {
        u16 m_startPosX : 4;
        u16 m_startPosY : 4;
        u16 m_axis : 4;
        u16 m_isSmallword : 4;

        u16 m_wordId;     // Id of the word in dictionary
    };

    u8 m_wordCount;

    // Used for generated grids.
    u16 m_words[MAX_WORD_COUNT];
};

struct VocabularyData
{
    union StateBucket
    {
        struct
        {
            u8 m_state0 : 3;		//word state (item 0)
            u8 m_seen0  : 1;		//seen flag (item 0)
            u8 m_state1 : 3;		//word state (item 1)
            u8 m_seen1  : 1;		//seen flag (item 1)
        };
        u8 m_wholeBucket;
    }StateBucket;

    u8  m_type;
    u8  m_playerDl;
    u8  m_selectionCount[4]; //LT_BasicCount
    u32 m_newActiveCount;
    u32 m_allTimeMaxActiveCount;
    u8  m_lastAccuracy;

    union StateBucket* m_itemStates;
};

struct MainVocabularyData
{
    struct VocabularyData voc;
    union StateBucket m_specificItemStates[UBI_PLAYERINFO_DATAITEM_SAVE_MAXSIZE];	//missing 2 for 32-bit alignment
};

struct ThemeVocabularyData
{
    struct VocabularyData voc;
    union StateBucket m_specificItemStates[UBI_PLAYERINFO_THEMEDATAITEM_SAVE_MAXSIZE];	//missing 2 for 32-bit alignment
};



typedef struct PlayerData
{
    char m_name[UBI_PLAYERINFO_NAME_LENGTH+1]; // 13 (0x08Å`0x0x14)
    u8 m_dataState;                  //  1 (0x15)
    u8 m_musicOff;                       //  1 (0x16)
    u8 m_useWarmups;                     //  1 (0x17)
    u8 m_lefty;                          //  1 (0x18)
    u16 m_numSessions;                   //  2 (0x19Å`0x1A)
    u16 m_seenRealLifeChallenges;        //  2 (0x1BÅ`0x1C)
    f32 m_fReadingTimeMod;               //  4 (0x1DÅ`
    f32 m_fReactionTimeModQuick;         //  4
    f32 m_fReactionTimeModSegment;       //  4

    u8 m_sessionStars;                   //  1
    u8 m_sessionUnlockTry;               //  1
    u8 m_personalProgressSeen;           //  1
    u32 m_gameTracker;                   //  4
    u8 m_hsGamesPlayed;                  //  1
    u8 m_themeViewed;                    //  1
    u8 m_themeUnlocked;                  //  1
    u8 m_gameIntroViewed[UBI_PLAYERINFO_GAMES_NUM]; // 14
    u16 m_gameLastSessionPlayed[UBI_PLAYERINFO_GAMES_NUM];    // 28
    struct ScoreInfo m_topScores[UBI_PLAYERINFO_GAMES_NUM][UBI_PLAYERINFO_TOP_SCORE_NUM]; // 
    struct ScoreInfo m_lastScores[UBI_PLAYERINFO_GAMES_NUM][UBI_PLAYERINFO_LAST_GAME_SCORES_NUM];
    u8  m_crosswordRanks[UBI_PLAYERINFO_CROSSWORD_COUNT];

    struct ActivePassiveData m_activePassiveTotals;
    struct UnlockGameData m_unlockGameData;
    struct UnlockData m_unlockData;

    struct MainVocabularyData m_mainVocabularyData;                         // 
//    struct ThemeVocabularyData m_themeVocabularyData[DICT_THEME_MAX_COUNT]; // 

    struct HiddenWordData m_hiddenWordData;             // 153 ()
    struct CrosswordData  m_crossWordData;              // 133 ()
    struct DynamicCrosswordData m_dynamicCrossWordData; // 65 (Å`0xF37)
}
PlayerData;


typedef struct VCW_BodyForCheck
{
    char m_name[UBI_PLAYERINFO_NAME_LENGTH+1]; // 13 (0x08Å`0x0x14)
    u8   dummy[0xF38 - 8 - 13];
}
VCW_BodyForCheck;
//NN_COMPILER_ASSERT(0xF38 == sizeof(VCW_SavegameHeader) + sizeof(VCW_BodyForCheck));

/**/
BOOL checkVCW( TitleProperty* tp, u32 ggid);


/* TWLÉâÉìÉ`ÉÉÇ…Ç‡à⁄êAÇ∑ÇÈÇÃÇ≈ CÇ≈ÉIÉuÉWÉFÉNÉgÇíËã`Ç∑ÇÈ */
typedef struct VCW
{
    u8* buf;
    u32 ggid;
    VCW_SavegameHeader *header;
    VCW_BodyForCheck   *body;
    u16 S_checksum;
    u16 B_checksum;
}VCW;

//static void VCW_Initialize( VCW* vcw, void* buffer, u32 ggid);
//static bool VCW_IsFactoryOrBroken( VCW* vcw);
//static bool VCW_CheckRange( VCW* vcw);
//static u16  VCW_Modify( VCW* vcw);
//static bool VCW_IsModified( VCW* vcw);
//static void VCW_UpdateChecksum( VCW* vcw);
//static u16  VCW_GetChecksum( VCW* vcw);
//static void VCW_Finalize( VCW* vcw);
/* ÉIÉuÉWÉFÉNÉgÇ±Ç±Ç‹Ç≈ */

#endif
