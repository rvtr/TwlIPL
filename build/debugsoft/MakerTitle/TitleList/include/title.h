/*---------------------------------------------------------------------------*
  Project:  TwlSDK_IPL - debugsoft - MakerTitle - TitleList
  File:     title.h

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

// セーブデータチェック対象ロム
static char* GAMECODE_LIST[] = {
     "4KAA",
     "4KBA",
     "4KCA",
     "4KDA",
     "4KEA",
     "4KFA",
     "4KGA",
     "4KHA",
     "4KIA",
     "4KJA",
};

static const u32 TITLE_COUNT = sizeof(GAMECODE_LIST) / sizeof(char*);

// PublicとPrivateの有無
static BOOL DATA_EXIST[TITLE_COUNT][2] = {
    {TRUE, TRUE},   //  A
    {TRUE, TRUE},   //  B
    {TRUE, TRUE},   //  C
    {TRUE, FALSE},  //  D
    {FALSE, TRUE},  //  E
    {FALSE, FALSE}, //  F
    {TRUE, TRUE},   //  G
    {TRUE, FALSE},  //  H
    {FALSE, TRUE},  //  I
    {FALSE, FALSE}, //  J
    
};

