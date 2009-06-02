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

// �\������Ώۂ����[�U�[�A�v�������ɂ���ꍇ
//#define USER_APP_ONLY

// �f�o�b�O�p
//#define DEBUG_MODE

/*---------------------------------------------------------------------------*
    �ϐ� ��`
 *---------------------------------------------------------------------------*/
typedef void* (*NAMUTAlloc)(u32 size);
typedef void  (*NAMUTFree)(void* ptr);

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;

// �L�[����
static KeyInfo  gKey;

// �C���X�g�[������Ă��� NAND �A�v���̐�
static s32 gNandAppNum;
static s32 gNandInstalledAppNum;
static s32 gNandAllAppNum;

// �J�[�\���ʒu
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

  Description:  ���C���֐�
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
        // �L�[���͏��擾
		ReadKey(&gKey);

        // �J�[�\���̈ړ�
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

        
		// ��ʕ`��
		DrawScene(gDataList);
        
        // �u�u�����N�҂�
        OS_WaitVBlankIntr();
        
        // ��ʃN���A
        ClearScreen();
    }

	// �u�u�����N�҂� �Ō�ɉ�ʂ��X�V���Ă���I��
    OS_WaitVBlankIntr();
    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         DrawScene

  Description:  ��ʕ`��֐�

  �����F
  	0xf3, // ��				0xf4, // ��				0xf5, // �s���N
	0xf6, // ���F			0xf7, // �����񂾐�		0xf8, // �����񂾗�
	0xf9, // �����񂾐� 	0xfa, // �����񂾉��F
 *---------------------------------------------------------------------------*/
static void DrawScene(DataStruct* list)
{
	s32 i;
	u8 init_code[5];
	u8 color;

    DataStruct* p = list;
    
	// ����	�ꗗ�\��
	PutMainScreen( 0,  0, 0xf2, " ------- eTicket Viewer ------- ");
	PutMainScreen( 2,  2, 0xf4, " Game    Ticket  Ticket");
    PutMainScreen( 2,  3, 0xf4, "   Code    Num     Type");
	PutMainScreen( 0,  4, 0xff, " ------------------------------");

	// �J�[�\���\��
    PutMainScreen( 0, gCurPos+5 , 0xf1, ">>");
    
    for ( i=0; i < TITLE_NUM_PAGE; i++, p++)
    {
		// �������� NAND �A�v���̐��� 1�y�[�W�ɂ������Ȃ��ꍇ�͓r���ŏI������
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

    // �����	�ڍו\��
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

  Description:  TitleID���X�g���擾����֐�
    			NAM_GetTitleList �� NAM_GetInstalledTitleList ���g��
 *---------------------------------------------------------------------------*/
static BOOL GetDataStruct(DataStruct* list, DataStruct* Ilist)
{
	// 36���̃^�C�g��ID���X�g�o�b�t�@
	NAMTitleId titleIdList[TITLE_NUM_PAGE];
	s32 i;

    // --- GetTitleList
	if ( NAM_GetTitleList(titleIdList, TITLE_NUM_PAGE) != NAM_OK )
	{
		OS_PutString("NAM_GetTitleList failed.");
		return FALSE;
	}
	
	// �f�[�^���X�g�̍쐬�i1�y�[�W���j
	for ( i=0; i<TITLE_NUM_PAGE; i++, list++)
	{
		// �������� NAND �A�v���̐��� 1�y�[�W�ɂ������Ȃ��ꍇ�͓r���ŏI������
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
	
	// �f�[�^���X�g�̍쐬�i1�y�[�W���j
	for ( i=0; i<TITLE_NUM_PAGE; i++, Ilist++)
	{
		// �������� NAND �A�v���̐��� 1�y�[�W�ɂ������Ȃ��ꍇ�͓r���ŏI������
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

  Description:  �w�肳�ꂽ titleID �� eTicket �^�C�v���擾����

  Arguments:    titleID: common eTicket ���ǂ����𒲂ׂ����^�C�g���� titleID
	            pETicketType : ���ʂ��i�[����ESETicketType�|�C���^

  Returns:      NAM_OK  : �擾����
	            ����ȊO: �擾���s
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
			// eTicket �́A���̂܂܂������͒ǉ��������肦�Ȃ��̂ŁA�v���C���X�g�[�����ꂽ�A�v���ł́A�K��Common eTikcet�����݂���B
			// ����āA�S�Ă� eTicket �̂����A�ЂƂł� deviceId �� 0x00000000 �Ȃ�Acommon eTicket �Ɣ��f�B
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

  Description:  �w�肳�ꂽ�^�C�g���� eTicket ���擾
�@�@�@�@�@�@�@�@��nam_title.c �� GetTicketViews �֐����R�s�y

  Arguments:    pptv       : �擾�������� eTicket ���X�g�̃|�C���^���i�[����|�C���^
	            pNumTicket : �擾�������� eTicket �����i�[����|�C���^
                titleID    : eTicket ���擾�������^�C�g���� titleID

  Returns:      NAM_OK     : �擾����
	            ����ȊO   : �擾���s
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

  Description:  �e�A�v����ETicket�f�[�^���擾����
 *---------------------------------------------------------------------------*/
BOOL GetETicketData( void )
{
    s32 result = TRUE;
    s32 i,j;

    // NAND �ɃC���|�[�g����Ă���NAND �A�v���̐����擾����
    if ( (gNandAppNum = NAM_GetNumTitles()) < 0)
    {
		OS_Panic("NAM_GetNumTitles() failed.");
	}
    // ���̂�����^�C�g����
    if ( (gNandInstalledAppNum = NAM_GetNumInstalledTitles()) < 0)
    {
		OS_Panic("NAM_GetNumInstalledTitles() failed.");
	}
   	// ���̎擾
	if ( !GetDataStruct(gDataList, gInstalledDataList) )
	{
		OS_Panic("GetDataStruct() failed.");
	}

    // NAM_GetTitleList          -- �폜����Ă��邪eTicket�̂ݑ��݂���^�C�g�������X�g�A�b�v����Ȃ�
    // NAM_GetInstalledTitleList -- SRL�͂��邪eTicket���Ȃ��^�C�g�������X�g�A�b�v����Ȃ�
    // ���̂��ߗ��҂��}�[�W����
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
    // ���[�U�[�A�v���������o����
	getUserApplication( gDataList );
#endif

    OS_TPrintf("gNandAllAppNum : %d\n",gNandAllAppNum);
    
    // �A�v����ETicket�f�[�^���擾����
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

  Description:  �S�A�v�����烆�[�U�[�A�v�������𒊏o����֐�
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
    
    // ���[�U�[�A�v�������̃��X�g���R�s�[
	MI_CpuCopy8(buf, list, sizeof(buf));

    // �A�v�������̍X�V
    gNandAllAppNum = count;
}
#endif


/*---------------------------------------------------------------------------*
   UTIL �֐�
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

	// NULL �I�[
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
	
	// NULL �����I�[
	*code = 0x00;
}

static void ConvertInitialCode(u8* code, u32 titleid_lo)
{
	s32 i;
	
	for ( i=0; i<4; i++, code++)
	{
		*code = (u8)(titleid_lo >> (8 * (3-i)));
	}
	
	// NULL�����I�[
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
    // �e�L�X�g�\�����X�V
    UpdateScreen();

    // IRQ �`�F�b�N�t���O47���Z�b�g
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
