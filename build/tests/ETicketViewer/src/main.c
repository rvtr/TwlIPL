/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest - Nand-2
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-03#$
  $Rev: 8251 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/
//#include "../../../include/sysmenu/namut.h"

#include <twl.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <twl/nam.h>
#include <twl/aes.h>
#include <twl/os/common/banner.h>
#include <nitro/nvram.h>
#include <nitro/math/rand.h>

#include "application_jump_private.h"
#include "common.h"
#include "screen.h"

#define TITLE_NUM_CUL  				18
#define TITLE_NUM_PAGE 				(TITLE_NUM_CUL * 2)

#define NTR_AREA_READ_NUM	   		7
#define TWL_AREA_READ_NUM			7
#define CARD_READ_SIZE				32
#define CARD_READ_INTERVAL			0x8000

#define FLX_AREA_DATA_Y_BASE		4
#define LTD_AREA_DATA_Y_BASE		13
#define ETICKET_NUM_MAX				10

#define ES_ERR_OK					0

// 表示する対象をユーザーアプリだけにする場合
//#define USER_APP_ONLY

// デバッグ用
//#define DEBUG_MODE

/*---------------------------------------------------------------------------*
    変数 定義
 *---------------------------------------------------------------------------*/
typedef void* (*NAMUTAlloc)(u32 size);
typedef void  (*NAMUTFree)(void* ptr);

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;

// キー入力
static KeyInfo  gKey;

// インストールされている NAND アプリの数
static s32 gNandAppNum;
static s32 gNandInstalledAppNum;
static s32 gNandAllAppNum;

// カーソル位置
static s32 gCurPos = 0;

// eTicketType
typedef enum ETicketType {
	ETICKET_TYPE_COMMON = 0,
	ETICKET_TYPE_PERSONALIZED = 1
}ETicketType;

typedef struct DataStruct
{
	NAMTitleId		id;
	BOOL			commonTicketFlg;
    u32				numTicket;
    ETicketType		tType[ETICKET_NUM_MAX];
    
} DataStruct;

typedef struct {
	u8               pad1[ 12 ];
    u32              deviceId;
	u8				 pad2[ 216 - 16 ];
} ESTicketView;

typedef s32 ESError;

extern ESError ES_GetTicketViews(u64 titleId, ESTicketView* ticketViewList, u32* ticketViewCnt);

static DataStruct gDataList[TITLE_NUM_PAGE * 2];
static DataStruct gInstalledDataList[TITLE_NUM_PAGE];

#ifdef DEBUG_MODE
static MATHRandContext32 context;
#endif

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void DrawScene(DataStruct* list);
static BOOL GetDataStruct(DataStruct* list, DataStruct* Ilist);

static void ConvertTitleIdLo(u8* code, u8* titleid_lo);
static void ConvertGameCode(u8* code, u32 game_code);
static void ConvertInitialCode(u8* code, u32 titleid_lo);

static void* AllocForNAM(u32 size);
static void FreeForNAM(void* ptr);

static s32 GetETicketType(DataStruct* data, ETicketType *pETicketType );
static s32 GetTicketViews(ESTicketView** pptv, u32* pNumTicket, NAMTitleId titleId);

BOOL GetETicketData( void );
void* MyNAMUT_Alloc(u32 size);
void MyNAMUT_Free(void* buffer);

#ifdef USER_APP_ONLY
static void getUserApplication(DataStruct* list);
#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
	u32 count = 0;
    BOOL errorFlg = FALSE;

	InitCommon();
    InitScreen();

    GX_DispOn();
    GXS_DispOn();

    spAllocFunc = AllocForNAM;
	spFreeFunc = FreeForNAM;

	gNandAppNum = 0;
	gNandInstalledAppNum = 0;
	gNandAllAppNum = 0;
    
	FS_Init( FS_DMA_NOT_USE );
    
    NAM_Init(AllocForNAM, FreeForNAM);

	MI_CpuClear8( gDataList, sizeof(gDataList));
	MI_CpuClear8( gInstalledDataList, sizeof(gInstalledDataList));

#ifdef DEBUG_MODE
    MATH_InitRand32( &context, 15 );
#endif
    
	(void) GetETicketData();
    
    ClearScreen();
    
    while(TRUE)
    {
        // キー入力情報取得
		ReadKey(&gKey);

        // カーソルの移動
        if (gKey.trg & PAD_KEY_DOWN)
        {
			gCurPos++;
			if ( gCurPos >= ( gNandAllAppNum < TITLE_NUM_PAGE ? gNandAllAppNum : TITLE_NUM_PAGE) )
			{
				gCurPos = 0;
			}
        }
        if (gKey.trg & PAD_KEY_UP)
        {
            if( gNandAllAppNum )
            {
				if ( gCurPos == 0 )
				{
					if ( gNandAllAppNum < TITLE_NUM_PAGE )
					{
						gCurPos = gNandAllAppNum - 1;
					}
					else
					{
						gCurPos = TITLE_NUM_PAGE - 1;
					}
				}
				else
				{
					gCurPos--;
				}
            }
		}

        
		// 画面描画
		DrawScene(gDataList);
        
        // Ｖブランク待ち
        OS_WaitVBlankIntr();
        
        // 画面クリア
        ClearScreen();
    }

	// Ｖブランク待ち 最後に画面を更新してから終了
    OS_WaitVBlankIntr();
    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         DrawScene

  Description:  画面描画関数

  メモ：
  	0xf3, // 青				0xf4, // 黄				0xf5, // ピンク
	0xf6, // 水色			0xf7, // くすんだ赤		0xf8, // くすんだ緑
	0xf9, // くすんだ青 	0xfa, // くすんだ黄色
 *---------------------------------------------------------------------------*/
static void DrawScene(DataStruct* list)
{
	s32 i;
	u8 init_code[5];
	u8 color;

    DataStruct* p = list;
    
	// 上画面	一覧表示
	PutMainScreen( 0,  0, 0xf2, " ------- eTicket Viewer ------- ");
	PutMainScreen( 2,  2, 0xf4, " Game    Ticket  Ticket");
    PutMainScreen( 2,  3, 0xf4, "   Code    Num     Type");
	PutMainScreen( 0,  4, 0xff, " ------------------------------");

	// カーソル表示
    PutMainScreen( 0, gCurPos+5 , 0xf1, ">>");
    
    for ( i=0; i < TITLE_NUM_PAGE; i++, p++)
    {
		// そもそも NAND アプリの数が 1ページにも満たない場合は途中で終了する
		if ( i >= gNandAllAppNum )
		{
			break;
		}

		ConvertInitialCode(init_code, NAM_GetTitleIdLo(p->id));
        
    	color = p->commonTicketFlg ? (u8)0xf9 : (u8)0xfa;
    
    	PutMainScreen( 3, 5 +i, color, "%s", init_code);
    	PutMainScreen(11, 5 +i, color, "%d", p->numTicket);

    	if(p->commonTicketFlg)
    	{
			PutMainScreen(19, 5 +i, color, "common");
    	}
    	else
    	{
			PutMainScreen(19, 5 +i, color, "personalized");
    	}
    }

    // 下画面	詳細表示
	ConvertInitialCode(init_code, NAM_GetTitleIdLo(list[gCurPos].id));
    PutSubScreen(3,   2, 0xf6, "Selected Title : [ %s ]", init_code);
    PutSubScreen(3,   4, 0xff, "- Ticket List -");

    for( i=0; i < list[gCurPos].numTicket; i++){
        if(i > 15)
        {
			break;
        }
        
        PutSubScreen(5, 6+i, 0xf4, "Ticket%d : ", (i+1));

        if(list[gCurPos].tType[i] == ETICKET_TYPE_COMMON)
        {
			PutSubScreen(15, 6+i, 0xf9, "COMMON");
        }
        else
        {
			PutSubScreen(15, 6+i, 0xfa, "PERSONALIZED");
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         GetDataStruct

  Description:  TitleIDリストを取得する関数
    			NAM_GetTitleList と NAM_GetInstalledTitleList を使う
 *---------------------------------------------------------------------------*/
static BOOL GetDataStruct(DataStruct* list, DataStruct* Ilist)
{
	// 36個分のタイトルIDリストバッファ
	NAMTitleId titleIdList[TITLE_NUM_PAGE];
	s32 i;

    // --- GetTitleList
	if ( NAM_GetTitleList(titleIdList, TITLE_NUM_PAGE) != NAM_OK )
	{
		OS_PutString("NAM_GetTitleList failed.");
		return FALSE;
	}
	
	// データリストの作成（1ページ分）
	for ( i=0; i<TITLE_NUM_PAGE; i++, list++)
	{
		// そもそも NAND アプリの数が 1ページにも満たない場合は途中で終了する
		if ( i >= gNandAppNum )
		{
			break;
		}

        OS_TPrintf("id : 0x%08x\n", titleIdList[i]);
		list->id = titleIdList[i];
	}

	MI_CpuClear8(titleIdList, sizeof(titleIdList));

	OS_PutString("\n\n");
    
    // --- GetInstalledTitleList
	if ( NAM_GetInstalledTitleList(titleIdList, TITLE_NUM_PAGE) != NAM_OK )
	{
		OS_PutString("NAM_GetInstalledTitleList failed.");
		return FALSE;
	}
	
	// データリストの作成（1ページ分）
	for ( i=0; i<TITLE_NUM_PAGE; i++, Ilist++)
	{
		// そもそも NAND アプリの数が 1ページにも満たない場合は途中で終了する
		if ( i >= gNandInstalledAppNum )
		{
			break;
		}

        OS_TPrintf("id : 0x%08x\n", titleIdList[i]);
		Ilist->id = titleIdList[i];
	}

	return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         GetETicketType

  Description:  指定された titleID の eTicket タイプを取得する

  Arguments:    titleID: common eTicket かどうかを調べたいタイトルの titleID
	            pETicketType : 結果を格納するESETicketTypeポインタ

  Returns:      NAM_OK  : 取得成功
	            それ以外: 取得失敗
 *---------------------------------------------------------------------------*/
static s32 GetETicketType(DataStruct* data, ETicketType *pETicketType )
{
    s32 result;
	ESTicketView* ptv;
    u32 numTicket;

#ifdef DEBUG_MODE
    u32 i;
#endif
	
	*pETicketType = ETICKET_TYPE_PERSONALIZED;
	
    result = GetTicketViews(&ptv, &numTicket, data->id);

#ifndef DEBUG_MODE
	data->numTicket = numTicket;
#else
	data->numTicket = MATH_Rand32( &context, 10 );
#endif
    
    if( result == NAM_OK )
    {
        if( numTicket > 0 )
        {
            int i;
			// eTicket は、そのままもしくは追加しかありえないので、プリインストールされたアプリでは、必ずCommon eTikcetが存在する。
			// よって、全ての eTicket のうち、ひとつでも deviceId が 0x00000000 なら、common eTicket と判断。
            for( i = 0; i < numTicket; i++ )
            {
				if( ptv[i].deviceId == 0x00000000 )
                {
					*pETicketType = ETICKET_TYPE_COMMON;
                    data->tType[i] = ETICKET_TYPE_COMMON;
				}
                else
                {
					data->tType[i] = ETICKET_TYPE_PERSONALIZED;
                }
			}
		}
        MyNAMUT_Free(ptv);
	}

#ifdef DEBUG_MODE
	for( i=0; i<data->numTicket; i++)
    {
        if( MATH_Rand32( &context, 10 ) % 5 )
        {
			data->tType[i] = ETICKET_TYPE_COMMON;
        }
        else
        {
			data->tType[i] = ETICKET_TYPE_PERSONALIZED;
        }
    }
#endif
    
	return result;
}


/*---------------------------------------------------------------------------*
  Name:         GetTicketViews

  Description:  指定されたタイトルの eTicket を取得
　　　　　　　　※nam_title.c の GetTicketViews 関数をコピペ

  Arguments:    pptv       : 取得成功時に eTicket リストのポインタを格納するポインタ
	            pNumTicket : 取得成功時に eTicket 数を格納するポインタ
                titleID    : eTicket を取得したいタイトルの titleID

  Returns:      NAM_OK     : 取得成功
	            それ以外   : 取得失敗
 *---------------------------------------------------------------------------*/
static s32 GetTicketViews(ESTicketView** pptv, u32* pNumTicket, NAMTitleId titleId)
{
    s32 result;
    u32 numTicket;
    ESTicketView* ptv = NULL;

    result = ES_GetTicketViews(titleId, NULL, &numTicket);

    if( result != ES_ERR_OK )
    {
        return result;
    }

    if( numTicket != 0 )
    {
        ptv = MyNAMUT_Alloc(sizeof(ESTicketView) * numTicket);

        if( ptv == NULL )
        {
            return NAM_NO_MEMORY;
        }

        result = ES_GetTicketViews(titleId, ptv, &numTicket);
    }

    if( result == ES_ERR_OK )
    {
        *pptv = ptv;
        *pNumTicket = numTicket;
    }
    else
    {
        MyNAMUT_Free(ptv);
    }

    return result;
}


/*---------------------------------------------------------------------------*
  Name:         GetETicketData

  Description:  各アプリのETicketデータを取得する
 *---------------------------------------------------------------------------*/
BOOL GetETicketData( void )
{
    s32 result = TRUE;
    s32 i,j;

    // NAND にインポートされているNAND アプリの数を取得する
    if ( (gNandAppNum = NAM_GetNumTitles()) < 0)
    {
		OS_Panic("NAM_GetNumTitles() failed.");
	}
    // 実体があるタイトル数
    if ( (gNandInstalledAppNum = NAM_GetNumInstalledTitles()) < 0)
    {
		OS_Panic("NAM_GetNumInstalledTitles() failed.");
	}
   	// 情報の取得
	if ( !GetDataStruct(gDataList, gInstalledDataList) )
	{
		OS_Panic("GetDataStruct() failed.");
	}

    // NAM_GetTitleList          -- 削除されているがeTicketのみ存在するタイトルがリストアップされない
    // NAM_GetInstalledTitleList -- SRLはあるがeTicketがないタイトルがリストアップされない
    // そのため両者をマージする
    gNandAllAppNum = gNandAppNum;
    for (i=0; i<gNandInstalledAppNum; i++)
    {
        BOOL find = FALSE;
        for (j=0; j<gNandAppNum; j++)
        {
            if (gInstalledDataList[i].id == gDataList[j].id) 
            {
                find = TRUE;
                break;
            }
        }
        if (find == FALSE)
        {
            MI_CpuCopy8(&gInstalledDataList[i], &gDataList[gNandAllAppNum], sizeof(DataStruct));
            gNandAllAppNum++;
        }
    }

#ifdef USER_APP_ONLY
    // ユーザーアプリだけ抽出する
	getUserApplication( gDataList );
#endif

    OS_TPrintf("gNandAllAppNum : %d\n",gNandAllAppNum);
    
    // アプリのETicketデータを取得する
    for (i=0; i<gNandAllAppNum; i++)
    {
		ETicketType eTicketType = ETICKET_TYPE_PERSONALIZED; // default
		
		if( GetETicketType( &gDataList[i], &eTicketType ) != NAM_OK )
		{
			result = FALSE;
		}
		else
		{
			u32 numTicket = 0;

            (void)ES_GetTicketViews( gDataList[i].id, NULL, &numTicket);

            gDataList[i].commonTicketFlg = (eTicketType == ETICKET_TYPE_COMMON) ? TRUE : FALSE;
		}
    }
    
    return result;
}



#ifdef USER_APP_ONLY
/*---------------------------------------------------------------------------*
  Name:         getUserApplication

  Description:  全アプリからユーザーアプリだけを抽出する関数
 *---------------------------------------------------------------------------*/
static void getUserApplication(DataStruct* list)
{
    u32 i;
    s32 count = 0;

    DataStruct* p = list;
    DataStruct buf[TITLE_NUM_PAGE * 2];

    for( i=0; i<gNandAllAppNum; i++, p++ )
    {
        if(!(p->id & TITLE_ID_APP_TYPE_MASK))
        {
			MI_CpuCopy8(p, &buf[count], sizeof(DataStruct));
            count++;
        }
    }
    
    // ユーザーアプリだけのリストをコピー
	MI_CpuCopy8(buf, list, sizeof(buf));

    // アプリ総数の更新
    gNandAllAppNum = count;
}
#endif


/*---------------------------------------------------------------------------*
   UTIL 関数
 *---------------------------------------------------------------------------*/
static void ConvertTitleIdLo(u8* code, u8* titleid_lo)
{
	u8 tmp[5];
	s32 i;
	
	for ( i=3; i>=0; i--, titleid_lo++ )
	{
		tmp[i] = *titleid_lo;
		*code = tmp[i];
	}

	// NULL 終端
	*code = 0x00;
}

static void ConvertGameCode(u8* code, u32 game_code)
{
	u8 tmp[5];
	s32 i;
	
	ConvertInitialCode(tmp, game_code);
	
	for ( i=3; i>=0; i--, code++)
	{
		*code = tmp[i];
	}
	
	// NULL 文字終端
	*code = 0x00;
}

static void ConvertInitialCode(u8* code, u32 titleid_lo)
{
	s32 i;
	
	for ( i=0; i<4; i++, code++)
	{
		*code = (u8)(titleid_lo >> (8 * (3-i)));
	}
	
	// NULL文字終端
	*code = 0x00;
}


void* MyNAMUT_Alloc(u32 size)
{
    const u32 allocSize = MATH_ROUNDUP32(size);
    SDK_ASSERTMSG( spAllocFunc != NULL, "NAMUT_Init should be called previously.\n");
    return spAllocFunc(allocSize);
}


static void* AllocForNAM(u32 size)
{
	void* ptr;
	ptr = OS_AllocFromMain(size);
	
	if (ptr == NULL)
	{
		OS_Panic("alloc failed.");
	}
	
	return ptr;
}


void MyNAMUT_Free(void* buffer)
{
    SDK_ASSERTMSG( spFreeFunc != NULL, "NAMUT_Init should be called previously.\n");
    if (buffer)
    {
        spFreeFunc(buffer);
    }
}


static void FreeForNAM(void* ptr)
{
	OS_FreeToMain(ptr);
}


void VBlankIntr(void)
{
    // テキスト表示を更新
    UpdateScreen();

    // IRQ チェックフラグ47をセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
