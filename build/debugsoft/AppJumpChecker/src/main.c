/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest - Nand-2
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>

#include "common.h"
#include "screen.h"

#define DMA_NO_FS		1
#define TITLE_NUM_CUL  18
#define TITLE_NUM_PAGE (TITLE_NUM_CUL * 2)

/*---------------------------------------------------------------------------*
    �ϐ� ��`
 *---------------------------------------------------------------------------*/
// �L�[����
static KeyInfo  gKey;

// �C���X�g�[������Ă��� NAND �A�v���̐�
static s32 gNandAppNum;

// �J�[�\���ʒu
static s32 gCurPos = 0;

static BOOL gIsExistCard = FALSE;

// �G���[�\���p�o�b�t�@
static char gErrBuf[33];

// �G���[�\�����Ԍv���p
static OSTick gErrTick = 0;

typedef struct DataStruct
{
	NAMTitleId	id;
	BOOL		normaljmp_flag;
} DataStruct;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void PrintErrMsg(const char* msg);

static BOOL GetDataStruct(DataStruct* list);
static void DrawScene(DataStruct* list);

static void ConvertGameCode(u8* code, u32 game_code);
static void ConvertInitialCode(u8* code, u32 titleid_lo);

static void* AllocForNAM(u32 size);
static void FreeForNAM(void* ptr);

/*---------------------------------------------------------------------------*/

static void PrintErrMsg(const char* msg)
{
	if ( STD_StrLen(msg) > 32 )
	{
		OS_Warning("err msg is too long.");
		return;
	}
	
	STD_StrCpy( gErrBuf, msg );
	gErrTick = OS_GetTick();
}

void TwlMain(void)
{
	DataStruct dataList[TITLE_NUM_PAGE];

	InitCommon();
    InitScreen();

    GX_DispOn();
    GXS_DispOn();

	FS_Init(DMA_NO_FS);
    NAM_Init(AllocForNAM, FreeForNAM);

    ClearScreen();

    // NAND �ɃC���|�[�g����Ă���NAND �A�v���̐����擾����
    if ( (gNandAppNum = NAM_GetNumTitles()) < 0)
    {
		OS_Panic("NAM_GetNumTitles() failed.");
	}

	

    // �L�[���͏��擾�̋�Ăяo��(IPL �ł� A �{�^�������΍�)
    ReadKey(&gKey);
    
   	// ���̎擾
	if ( !GetDataStruct(dataList) )
	{
		PrintErrMsg("Failed to get dataList.");
	}

    while(TRUE)
    {
        // �L�[���͏��擾
		ReadKey(&gKey);
        
        // �J�[�\���̈ړ�
        if (gKey.trg & PAD_KEY_DOWN)
        {
			gCurPos++;
			if ( gCurPos >= ( gNandAppNum < TITLE_NUM_PAGE ? gNandAppNum : TITLE_NUM_PAGE) )
			{
				gCurPos = 0;
			}
        }
        if (gKey.trg & PAD_KEY_UP)
        {
			if ( gCurPos == 0)
			{
				if ( gNandAppNum < TITLE_NUM_PAGE )
				{
					gCurPos = gNandAppNum - 1;
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
		
		// ��̈ړ�
		if ( (gKey.trg & PAD_KEY_RIGHT || gKey.trg & PAD_KEY_LEFT) && gNandAppNum >= TITLE_NUM_CUL )
		{
			if ( gCurPos > TITLE_NUM_CUL )
			{
				if ( gCurPos - TITLE_NUM_CUL < gNandAppNum )
				{
					gCurPos -= TITLE_NUM_CUL;
				}
			}
			else
			{
				if ( gCurPos + TITLE_NUM_CUL < gNandAppNum )
				{
					gCurPos += TITLE_NUM_CUL;
				}
			}
		}
		
		// �A�v���W�����v�̎��s
		if (gKey.trg & PAD_BUTTON_A)
		{
			OS_DoApplicationJump(dataList[gCurPos].id, OS_APP_JUMP_NORMAL);
			// �������͂����ȍ~�͎��s����Ȃ�
			PrintErrMsg("Failed to App Jump.");
		}
		
		// �J�[�h�A�v���ւ̃A�v���W�����v���s
		if (gKey.trg & PAD_BUTTON_Y)
		{
			CARDRomHeader* rh;
			u64 titleId;
			rh = (CARDRomHeader*)CARD_GetRomHeader();
			
			if ( rh->game_code != 0)
			{
				titleId = (0x00030000 << 32) | rh->game_code;
				OS_DoApplicationJump( titleId, OS_APP_JUMP_NORMAL);
			}

			PrintErrMsg("Failed to App Jump.");
		}
		
		// �G���[�\������
		if ( OS_TicksToSeconds(OS_GetTick() - gErrTick) > 3 )
		{
			PrintErrMsg(" ");
		}
		
		// ��ʕ`��
		DrawScene(dataList);
        
        // �u�u�����N�҂�
        OS_WaitVBlankIntr();
        
        // ��ʃN���A
        ClearScreen();
    }

	// �u�u�����N�҂� �Ō�ɉ�ʂ��X�V���Ă���I��
    OS_WaitVBlankIntr();
    OS_Terminate();
}

static BOOL GetDataStruct(DataStruct* list)
{
	// 36���̃^�C�g��ID���X�g�o�b�t�@
	NAMTitleId titleIdList[TITLE_NUM_PAGE];
	s32 i;
	FSFile fp;
	
	if ( NAM_GetTitleList(titleIdList, TITLE_NUM_PAGE) != NAM_OK )
	{
		PrintErrMsg("NAM_GetTitleList failed.");
		return FALSE;
	}
	
	// �f�[�^���X�g�̍쐬�i1�y�[�W���j
	for ( i=0; i<TITLE_NUM_PAGE; i++, list++)
	{
		ROM_Header_Short rh;
		char pathbuf[FS_ENTRY_LONGNAME_MAX + 32];
		
		// �������� NAND �A�v���̐��� 1�y�[�W�ɂ������Ȃ��ꍇ�͓r���ŏI������
		if ( i >= gNandAppNum )
		{
			break;
		}
		
	// TitleID �̊i�[
		list->id = titleIdList[i];
		
	// �m�[�}���W�����v�������̂������邩��\���t���O�̎擾
		// TitleID �����āA�^�C�g����ʂ��f�[�^�^�C�g���Ȃ��
		// �������� ROM�w�b�_�����݂��Ȃ��A�A�v���W�����v������K�v���Ȃ�
		if ( !NAM_IsDataTitle(list->id) )
		{
			// �A�v���{�̂ւ̃p�X���擾
			if ( NAM_GetTitleBootContentPath(pathbuf, list->id) != NAM_OK )
			{
				PrintErrMsg("GetContentPath failed.");
				return FALSE;
			}
		
			// �A�v���̃t�@�C���I�[�v��
			FS_InitFile(&fp);
			if ( !FS_OpenFileEx(&fp, pathbuf, FS_FILEMODE_R) )
			{
				// ���s���̓G���[�R�[�h�o�͂ŏI��
				PrintErrMsg("FS_OpenFileEx failed.");
				return FALSE;
			}
			
			if ( -1 == FS_ReadFile(&fp, &rh, sizeof(ROM_Header_Short) ))
			{
				PrintErrMsg("FS_ReadFile failed.");
				return FALSE;
			}
			
			list->normaljmp_flag = rh.permit_landing_normal_jump;
			
			FS_CloseFile(&fp);
		}
		else
		{
			list->normaljmp_flag = FALSE;
		}
	}
	
	PrintErrMsg(" ");
	return TRUE;
}

static void DrawScene(DataStruct* list)
{
	s32 i;
	u8 init_code[5];
	
	// ����
	PutMainScreen( 0,  1, 0xff, " ------ App Jump Checker ------ ");
	
	PutMainScreen( 0,  3, 0xff, "  total app : %d", gNandAppNum);
	
	PutMainScreen( 0,  5, 0xff, " ------------------------------ ");
	
	PutMainScreen( 0,  7, 0xff, "   A : try App Jump (to NAND)"   );
	PutMainScreen( 0,  8, 0xff, "   Y : try App Jump (to CARD)"   );
	PutMainScreen( 0, 10, 0xff, "   UP ,DOWN ,LEFT ,RIGHT KEY :"  );
	PutMainScreen( 0, 11, 0xff, "                move * (cursor) ");
	PutMainScreen( 0, 13, 0xff, " ------------------------------ ");
	
	PutMainScreen( 0, 15, 0xf1, "%s", gErrBuf);
	
	// �����
	PutSubScreen(  0, 0, 0xf4, " NAND ( max 36 )");
	PutSubScreen(  0, 1, 0xff, "--- ID ---------------- ID -------");
	
	
		// ����ʂ�1�s���A�J�[�\���p�X�y�[�X�A�C�j�V�����R�[�h�A�m�[�}���W�����v�ۃt���O��\��
	for ( i=0; i < TITLE_NUM_PAGE; i++, list++)
	{
		// �������� NAND �A�v���̐��� 1�y�[�W�ɂ������Ȃ��ꍇ�͓r���ŏI������
		if ( i >= gNandAppNum )
		{
			break;
		}
		
		ConvertInitialCode(init_code, NAM_GetTitleIdLo(list->id));
		
		if ( list->normaljmp_flag )
		{
			PutSubScreen( (i >= TITLE_NUM_CUL ? 18 : 0), 2 + (i >= TITLE_NUM_CUL ? i-18 : i), 0xf2, "   %s : o", init_code );
		}
		else
		{
			PutSubScreen( (i >= TITLE_NUM_CUL ? 18 : 0), 2 + (i >= TITLE_NUM_CUL ? i-18 : i), 0xff, "   %s : -", init_code );
		}
	}
	
		// �J�[�h�A�v��
	PutSubScreen(  0, 2 + TITLE_NUM_CUL + 1, 0xf4, " CARD");
	{
		CARDRomHeader* rh;	
		rh = (CARDRomHeader*)CARD_GetRomHeader();
		
		if ( rh->game_code != 0 )
		{
			ConvertGameCode(init_code, rh->game_code);
			if ( rh->reserved_A[8] & 0x80 )
			{
				PutSubScreen( 0, 2 + TITLE_NUM_CUL + 2, 0xf2, "   %s : o", init_code );
			}
			else
			{
				PutSubScreen( 0, 2 + TITLE_NUM_CUL + 2, 0xff, "   %s : -", init_code );
			}
		}
		else
		{
			PutSubScreen( 0, 2 + TITLE_NUM_CUL + 2, 0xfe, "   not exist");
		}
	}
	
	
	// �J�[�\���`��
	PutSubScreen( 1 + (gCurPos >= TITLE_NUM_CUL ? 18 : 0), 2 + (gCurPos >= TITLE_NUM_CUL ? gCurPos - 18 : gCurPos), 0xf4, "*");
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

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  �u�u�����N�����݃n���h���B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    // �e�L�X�g�\�����X�V
    UpdateScreen();

    // IRQ �`�F�b�N�t���O47���Z�b�g
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
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

static void FreeForNAM(void* ptr)
{
	OS_FreeToMain(ptr);
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
