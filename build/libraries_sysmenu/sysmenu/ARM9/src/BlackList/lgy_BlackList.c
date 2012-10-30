/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_BlackList.cpp

  Copyright (C)2009-2012 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "lgy_BlackList.h"

static void checkBlackListRom( void);


typedef struct lgyBlackCodeList
{
    char** code_list;
    u16   num;
    u16   reserved;
} lgyBlackCodeList;

// 以下、3文字目を揃えたリスト
static char *black_2[] = {
    "A22J", "A22E", "A22P", "A32J", "A32E", "A42J", "A52J", "A62J",
    "A72E", "A62P", "A72J", "Y52E", "A62E", "A82J", "Y22J", "A22K",
    "Y32J", "A72K", "Y42J", "A62K", "Y52J", "Y52K", "Y62J", "A22J"
};

static char *black_3[] = {
    "A23J", "A23E", "A33J", "A43J", "A53J", "A63J", "A63E", "A43E",
    "A43P", "Y23E", "Y73E", "A73J", "A83J", "A93J", "Y23J", "Y33J",
    "A23K", "A43K", "Y43J", "A23J"
};

static char *black_4[] = {
    "A24J", "A24E", "A34J", "A54J",
    "A74P", "Y54J", "Y54E", "Y54P", "Y54X", "A24P", "A44J", "A64J",
    "A84J", "Y34J", "Y54K", "Y64J", "A24J"
};

static char *black_5[] = {
    "A25J", "A35J", "A45J",
    "A55J", "A55E", "A85E", "A45P", "A85J", "A85P", "A95E", "A25J",
    "A65J", "A75J", "A95J", "Y25J", "A85K", "Y35J", "A55K", "Y45J",
    "Y55P", "Y65J", "Y75J"
};

static char *black_6[] = {
    "A26J", "A26E", "A26P", "A36J", "A36E",
    "A36P", "A76E", "Y36E", "Y56E", "Y56X", "Y56P", "A66J", "A56J",
    "A76J", "A96J", "Y26J", "A36K", "Y36J", "A76K", "Y76J", "A26J"
};

static char *black_7[] = {
    "A27J", "A27P", "A37J", "A57J", "A57E", "A57P", "CM7E", "A67J",
    "A77J", "A97J", "Y27J", "A57K", "Y47J", "Y57J", "Y77K", "A27J"
};

static char *black_8[] = {
    "A28J", "A38J", "A48J", "A58E", "A58P", "Y28E", "Y28P", "Y78P",
    "A58J", "A78J", "A98J", "Y28J", "Y28K", "Y48K", "Y68J", "A58K"
};

static char *black_9[] = {
    "A29J", "A29E", "A29P", "A39E", "A49J", "A59E", "A39J", "A79J",
    "A79E", "A79P", "Y49E", "Y49P", "Y79P", "Y79F", "A69J", "A89J",
    "A99J", "A39K", "Y39J", "Y59J", "Y69J", "Y89K"
};

static char *black_A[] = {
    "Y4AP", "Y8AP",
    "Y2AJ", "Y3AJ", "Y4AJ", "Y5AJ", "Y4AK", "Y6AJ", "Y7AJ", "Y8AJ",
    "Y8AE"
};

static char *black_B[] = {
    "Y9BP", "Y9BI", "Y9BF", "Y9BD", "Y2BJ", "Y3BJ", "Y4BJ",
    "Y5BJ", "Y6BJ", "Y8BJ", "Y9BH", "Y9BS"
};

static char *black_C[] = {
    "Y4CE", "Y4CP", "Y2CJ",
    "Y3CJ", "Y4CJ", "Y4CK", "Y6CJ", "Y7CP", "Y7CD", "Y7CF", "Y7CS",
    "Y7CI", "Y7CH"
};

static char *black_D[] = {
    "Y7DE", "Y2DJ", "Y3DJ", "Y5DJ"
};

static char *black_E[] = {
    "Y3EJ", "Y5EE", "Y4EJ", "Y5EJ", "Y6EJ", "Y7EJ", "Y8EE"
};

static char *black_F[] = {
    "Y2FP", "Y6FE", "Y8FE",
    "Y2FJ", "Y3FJ", "Y4FJ", "Y6FJ", "Y7FJ", "Y8FP"
};

static char *black_G[] = {
    "Y4GP", "Y4GE",
    "Y5GP", "Y7GP", "Y7GE", "Y2GJ", "Y4GK", "Y5GE", "Y7GK"
};

static char *black_H[] = {
    "Y5HE",
    "Y2HJ", "Y2HP", "Y3HK", "Y4HJ", "Y5HJ", "Y5HK", "Y6HJ", "Y7HJ",
    "Y8HJ"
};

static char *black_I[] = {
    "Y4IE", "Y2IJ", "Y3IJ", "Y4IJ", "Y5IJ", "Y6IJ", "Y7IJ"
};

static char *black_J[] = {
    "Y2JE", "Y2JP", "Y5JE", "Y3JJ", "Y2JJ", "Y8JJ"
};

static char *black_K[] = {
    "Y2KJ", "Y3KJ", "Y4KJ", "Y6KJ"
};

static char *black_L[] = {
    "Y2LE", "Y8LP", "Y8LE", "Y3LJ", "Y4LJ", "Y6LJ", "Y8LJ"
};

static char *black_M[] = {
    "Y2ME", "Y2MP", "Y2MJ", "Y3MJ", "Y2MK", "Y4MJ", "Y6MK"
};

static char *black_N[] = {
    "Y2NJ", "Y3NJ", "Y3NK", "AANJ", "Y8NJ"
};

static char *black_O[] = {
    "Y5OE", "Y2OJ", "Y4OJ", "Y5OJ", "Y7OJ", "Y7OK", "Y8OJ"
};

static char *black_P[] = {
    "Y6PE", "Y6PP", "Y3PK", "Y4PJ", "Y4PJ", "Y6PJ", "Y7PJ", "Y6PK"
};

static char *black_Q[] = {
    "Y2QJ", "Y3QJ", "Y4QJ", "Y6QJ"
};

static char *black_R[] = {
    "Y4RE", "Y5RE", "Y4RP", "Y5RX", "Y5RP", "Y6RE", "Y2RJ", "Y3RJ",
    "Y4RJ", "Y4RK", "Y8RJ"
};

static char *black_S[] = {
    "Y4SE", "Y6SE", "Y6SP", "Y7SP", "Y7SE",
    "Y5SE", "Y4SJ", "Y5SJ", "Y6SJ", "Y7SJ"
};

static char *black_T[] = {
    "Y5TE", "Y4TJ", "Y6TJ", "Y8TK"
};

static char *black_U[] = {
    "Y5UE", "Y4UE", "Y4UP", "Y6UP", "Y6UE", "Y2UJ", "Y2UJ",
    "Y4UJ", "Y6UJ", "Y7UJ"
};

static char *black_V[] = {
    "Y5VE", "Y2VJ", "Y3VJ", "Y7VJ"
};

static char *black_W[] = {
    "Y2WE", "Y2WP", "Y2WJ", "Y3WJ", "Y4WJ", "Y5WP"
};

static char *black_X[] = {
    "Y3XJ", "Y7XJ"
};

static char *black_Y[] = {
    "Y4YE", "Y5YE", "Y3YJ", "Y4YJ", "Y4YK", "Y5YJ", "Y6YJ"
};

static char *black_Z[] = {
    "Y6ZS", "Y6ZP",
    "Y6ZI", "Y6ZH", "Y6ZF", "Y6ZD", "Y6ZE", "Y2ZJ", "Y7ZJ"
};


static lgyBlackCodeList black_code_list[43] = {
    { NULL,    0}, // '0'
    { NULL,    0}, // '1'
    { black_2, sizeof(black_2)/sizeof(char*)}, // '2'
    { black_3, sizeof(black_3)/sizeof(char*)}, // '3'
    { black_4, sizeof(black_4)/sizeof(char*)}, // '4'
    { black_5, sizeof(black_5)/sizeof(char*)}, // '5'
    { black_6, sizeof(black_6)/sizeof(char*)}, // '6'
    { black_7, sizeof(black_7)/sizeof(char*)}, // '7'
    { black_8, sizeof(black_8)/sizeof(char*)}, // '8'
    { black_9, sizeof(black_9)/sizeof(char*)}, // '9'
    { NULL,    0}, // ':'
    { NULL,    0}, // ';'
    { NULL,    0}, // '<'
    { NULL,    0}, // '='
    { NULL,    0}, // '>'
    { NULL,    0}, // '?'
    { NULL,    0}, // '@'
    { black_A, sizeof(black_A)/sizeof(char*)}, // 'A'
    { black_B, sizeof(black_B)/sizeof(char*)}, // 'B'
    { black_C, sizeof(black_C)/sizeof(char*)}, // 'C'
    { black_D, sizeof(black_D)/sizeof(char*)}, // 'D'
    { black_E, sizeof(black_E)/sizeof(char*)}, // 'E'
    { black_F, sizeof(black_F)/sizeof(char*)}, // 'F'
    { black_G, sizeof(black_G)/sizeof(char*)}, // 'G'
    { black_H, sizeof(black_H)/sizeof(char*)}, // 'H'
    { black_I, sizeof(black_I)/sizeof(char*)}, // 'I'
    { black_J, sizeof(black_J)/sizeof(char*)}, // 'J'
    { black_K, sizeof(black_K)/sizeof(char*)}, // 'K'
    { black_L, sizeof(black_L)/sizeof(char*)}, // 'L'
    { black_M, sizeof(black_M)/sizeof(char*)}, // 'M'
    { black_N, sizeof(black_N)/sizeof(char*)}, // 'N'
    { black_O, sizeof(black_O)/sizeof(char*)}, // 'O'
    { black_P, sizeof(black_P)/sizeof(char*)}, // 'P'
    { black_Q, sizeof(black_Q)/sizeof(char*)}, // 'Q'
    { black_R, sizeof(black_R)/sizeof(char*)}, // 'R'
    { black_S, sizeof(black_S)/sizeof(char*)}, // 'S'
    { black_T, sizeof(black_T)/sizeof(char*)}, // 'T'
    { black_U, sizeof(black_U)/sizeof(char*)}, // 'U'
    { black_V, sizeof(black_V)/sizeof(char*)}, // 'V'
    { black_W, sizeof(black_W)/sizeof(char*)}, // 'W'
    { black_X, sizeof(black_X)/sizeof(char*)}, // 'X'
    { black_Y, sizeof(black_Y)/sizeof(char*)}, // 'Y'
    { black_Z, sizeof(black_Z)/sizeof(char*)}  // 'Z'
};



BOOL CheckBlackList( TitleProperty *rhs)
{
    u16 list_num;
    u8 second_character_value;
    char test[5];

    {
        char* n = (char*)&rhs->titleID;
        test[0] = n[3];
        test[1] = n[2];
        test[2] = n[1];
        test[3] = n[0];
        test[4] = '\0';
    }

/*
    {   // テスト用
        int total = 0;
        for( int i=0; i<43; i++)
        {
            total += black_code_list[i].num;
            if( !black_code_list[i].code_list)
            {
                OS_TPrintf( "[%d] : NULL, %d\n", i, black_code_list[i].num);
            }
            else
            {
                OS_TPrintf( "[%d] : 0x%x, %d\n", i, black_code_list[i].code_list, black_code_list[i].num);
            }
        }
        OS_TPrintf( "black list total : %d\n", total);
    }
*/

    second_character_value = (u8)(test[2]); // 3文字目
    if( (second_character_value < '0')||(second_character_value > 'Z'))
    {
        OS_TPrintf(" This game has no backup device...\n");
        return TRUE;
    }

    {
        u8     second_character_index = second_character_value - '0';
        u16    max                    = black_code_list[second_character_index].num;
        char** small_list             = black_code_list[second_character_index].code_list;
        for( list_num = 0; list_num < max; list_num++)
        {
            OS_TPrintf("small list:%s\n", small_list[list_num]);
            if( 0 == STD_CompareNString( &test[0], small_list[list_num], 4))
            {
                OS_TPrintf("%s is on the BlackList.\n", small_list[list_num]);
                checkBlackListRom();
                return FALSE;
            }
        }
    }
    OS_TPrintf(" This game is not on the BlackList.\n");
    return TRUE;
}

/*
  ダミーリードによってベリファイによる起動ブロックを装ってみるかも
 */
static void checkBlackListRom( void)
{
}

