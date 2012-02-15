/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_VCK.h

  Copyright (C)2009-2011 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#ifndef __LGY_VCK_H__
#define __LGY_VCK_H__

#include <twl.h>
#include <sysmenu.h>

enum E_CookingCoachSavegameSlot
{
    CCSS_ProfileSlot,      // 0x0000�`0x0377
    CCSS_ShoppingListSlot, // 0x0378�`0x0BCF
    CCSS_CommentSlot1,     // 0x0BD0�`0x0F03
    CCSS_CommentSlot2,     // 0x0F04�`0x1237
    CCSS_PhotoSlot,        // 0x1238�`0x1F76
    CCSS_SlotCount
};

#define C_PROFILENAME_MAXLENGTH                 11 // Include  \0 character at the end
#define DATASAVE_MAX_INGREDIENTS                 8
#define DATASAVE_MAX_SHOPPINGLIST_CUSTOMITEMS   10
#define DATASAVE_MAX_SHOPPINGLIST_RECIPEITEMS   75
#define MAX_SHOPPING_LIST_RECIPES                6
#define DATASAVE_NUM_CALENDARRECIPES            50
#define DATASAVE_MAX_ROTD                       10
#define DATASAVE_NUM_RECIPECOMMENTS_PER_SLOT     7
#define DATASAVE_RECIPECOMMENT_LENGTH          114
//the favorites are stored in an array of u32 used as a bitfield. each bit represents 
//ONE recipe. Thus, the maximum number of recipes that can be remembered in this array 
//is expressed by DATASAVE_FAVORITE_RECIPES_MAXNUMRECIPES
#define DATASAVE_FAVORITE_RECIPES_NB_U32         10
#define DATASAVE_FAVORITE_RECIPES_MAXNUMRECIPES (8*4*DATASAVE_FAVORITE_RECIPES_NB_U32)
#define QUANTITY_BUFFER_SIZE 20


typedef struct VCK_SlotHeader
{
    u32   m_uniqueIdentifier;
    u16   m_slotCheckSum;
    u8    m_status;
    u8    struct_padding;
}
VCK_SlotHeader;

typedef struct CalendarRecipe
{
    u32   m_uDay;
    u16   m_uRecipeID;
    u8    m_recipeState;
}
CalendarRecipe;

typedef struct RecipeOfTheDay
{
    u32 m_recipeID;
    s32 m_dayProposed;
}
RecipeOfTheDay;


//EntryData is the base class for the two "slots" that will be saved individually.=
//the structs are defined in the base class just in case some of them must be used in 
//both derived classes.
typedef struct VCK_ProfileSlotBody
{
    //TODO : optimize the size of the EntryData structure
    char                            m_playerName[C_PROFILENAME_MAXLENGTH]; // 11(0x08�`0x12)
                                                                           //  1(0x13)
    s32   m_averageCookingTime;                  //  4(0x14�`0x17) [��蓾��l�� -1,0,1,2�̂ǂꂩ]
    s32   m_culinaryLevel;                       //  4(0x18�`0x1B) [��蓾��l�� -1,0,1,2�̂ǂꂩ]
    s32   m_costImportance;                      //  4(0x1C�`0x1F) [��蓾��l�� -1,0,1,2�̂ǂꂩ]
    s32   m_numberCalories;                      //  4(0x20�`0x23) [��蓾��l�� -1,0,1,2�̂ǂꂩ]
    u32                       m_ingredientsExcluded[DATASAVE_MAX_INGREDIENTS]; // 32(0x24�`0x43)
    u32                       m_ingredientsPrefered[DATASAVE_MAX_INGREDIENTS]; // 32(0x44�`0x63)
    CalendarRecipe                  m_calendarRecipes[DATASAVE_NUM_CALENDARRECIPES]; //400(0x64�`0x1F3)
    u16                       m_uNbCalendarRecipes;                            //  2(0x1F4�`0x1F5)

    RecipeOfTheDay                  m_recipeOfTheDay[DATASAVE_MAX_ROTD];             // 80(0x1F8�`0x247)
    u32                       m_ROTD_index;                                    //  4(0x248�`0x24B)

    //---Music
    s32  m_musicStyle;       // (0x24C�`0x24F) ��蓾��l�� -1,0,1,2,3�̂ǂꂩ
    u32  m_musicLocation;    // (0x250�`0x253) ��蓾��l�� 0,1,2�̂ǂ��炩

    //---FAVORITES BIT FIELDS
    u32 m_favoriteRecipes[DATASAVE_FAVORITE_RECIPES_NB_U32];  // 40(0x254)
    u32 m_firstSessionDateInDay;                              //  4(0x27C)
    s32 m_lastSessionDateInDay;                               //  4(0x280)

    u8                        m_uBirthDate_Day;               //  1(0x284) [��蓾��l�� 1�`0x1F]
    u8                        m_uBirthDate_Month;             //  1(0x285) [��蓾��l�� 1�`0x0C]

    // recipe quantity info (300 quantity info saved on 2 bits each)
    u32 m_recipeQuantity[QUANTITY_BUFFER_SIZE];               // 80


    // save kitchen profile
    u8    m_unitsUsed;                           // 1
    u8    m_numServings;                         // 1(0x2D9) ��蓾��l�� 2,4,6,8�̂ǂꂩ

    BOOL        m_hasOven                          :1;  // 0x2DA
    BOOL        m_hasMixer                         :1;
    BOOL        m_hasBBQ                           :1;
    BOOL        m_hasFoodProcessor                 :1;

    // tutorial completed?
    BOOL m_tutorialSearchCompleted                 :1;
    BOOL m_tutorialCriteriaSearchCompleted         :1;    
    BOOL m_tutorialPrecookCompleted                :1;
    BOOL m_tutorialTimelineCompleted               :1;
    BOOL m_tutorialCookbookCompleted               :1;  // 0x2DB
    BOOL m_tutorialShoppingCompleted               :1;
    BOOL m_tutorialManagementCompleted             :1;
    BOOL m_tutorialExtraInfoCompleted              :1;
    BOOL m_tutorialCalendarCompleted               :1;
    BOOL m_searchFirstDeleteCompleted              :1;
    // introduction completed?
    BOOL m_introductionCompleted                   :1;
    BOOL m_tutorialFoodProfileCompleted            :1;
    BOOL m_tutorialFoodProfileFirstDeleteCompleted :1;  // 0x2DC
    BOOL m_tutorialCookbookTimerCompleted          :1;
    
    //---OPTIONS
    //---Voice
    BOOL m_vocalCommandsActivated                  :1;
    BOOL m_narrationActivated                      :1;
    BOOL      m_birthdayRecipeIsLocked             :1;
    //---Welcome
    BOOL m_forceShowBirthday                       :1;
    BOOL m_plannedARecipe                          :1;
    BOOL m_usedFavorites                           :1;
    
    u8 m_welcomeMsgAlreadyDisplayed;  //BIT FIELDS with msg that must be display only once // 0x2DD
                                            //check enum E_WelcomeMsg for bit order.

    u8    m_language;                 // 0x2DE
    u8    padding[0x95];
    //CookingCoach::ShoppingListCustomItem  m_shoppingListCustomItems[DATASAVE_MAX_SHOPPINGLIST_CUSTOMITEMS];
}
VCK_ProfileSlotBody;


// �`�F�b�NAPI�iVCK*�^�C�g���N���O�ɂ�����Ăׂ�OK�j
BOOL checkVCK( TitleProperty* tp);

#endif
