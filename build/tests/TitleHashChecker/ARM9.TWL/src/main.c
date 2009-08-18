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
#include <twl.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <twl/nam.h>
#include <twl/os/common/banner.h>
#include <nitro/math/rand.h>
#include <nitro/nvram.h>
#include <twl/lcfg.h>

#include "common.h"
#include "screen.h"
#include "kami_pxi.h"

#define TITLE_SHOW_BASE_Y					5
#define TITLE_MAX_SHOW						0x10
#define TITLE_NUM_PAGE 						300

#define ETICKET_NUM_MAX						10

#define ES_ERR_OK							0

#define CRCPOLY     						0x1021

#define DIGEST_HASH_BLOCK_SIZE_SHA1         (512/8)

#define DMA_NO_FS							1
#define CHECK_APP_NUM						10
#define REGION_NUM							6

#define NAND_FIRM_INFO_OFS					1
#define SHARED_FONT_INFO_OFS				2

#define NAND_BLOCK_BYTE 			       	0x200
#define NAND_FIRM_START_OFFSET    	       	0x200
#define NAND_FIRM_START_OFFSET_IN_FILE     	0x200

#define TITLE_ID_DATA_ONLY_MASK				0x00000008

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

// �J�[�\���ʒu
static s32 gCurPos = 0;

// �I�𒆂̗v�f
static s32 gCurrentElem;

// �y�[�W��
static u32 gCurrentPage;
static u32 gMaxPage;

// Error
static BOOL gErrorFlg;

typedef struct DataStruct
{
	NAMTitleId		id;
	
	u8				ver_major;
	u8				ver_minor;

	u8				DisableDebugFlg;
    
    u8				Sha1_digest[SVC_SHA1_DIGEST_SIZE];
    u16				crc16;
} DataStruct;

static u8 sFontData_Sha1_digest[SVC_SHA1_DIGEST_SIZE];
static u8 sNandFirm_Sha1_digest[SVC_SHA1_DIGEST_SIZE];

static DataStruct gDataList[TITLE_NUM_PAGE * 2];

static u16	crc_table[0x100];

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void DrawScene(DataStruct* list);
BOOL GetDataStruct(DataStruct* list);

static void ConvertTitleIdLo(u8* code, u8* titleid_lo);
static void ConvertGameCode(u8* code, u32 game_code);
static void ConvertInitialCode(u8* code, u32 titleid_lo);

static void* AllocForNAM(u32 size);
static void FreeForNAM(void* ptr);

static BOOL	ProcessTitleHashCheck(void);
static BOOL	GetAppPath(DataStruct* list, char* path_buf);
BOOL CulcuNandAppHash(DataStruct* list, char* full_path);
BOOL CulcuFontDataHash(DataStruct* list);
BOOL CulcuNandFirmHash(DataStruct* list);

u16 newGetCRC(u16  start, u16 *datap, u32  size);
static void inittable(unsigned short *table);

void* MyNAMUT_Alloc(u32 size);
void MyNAMUT_Free(void* buffer);
static BOOL ReadTWLSettings( void );

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
   	KamiPxiInit();

    FS_Init(FS_DMA_NOT_USE);
    
    InitScreen();

    GX_DispOn();
    GXS_DispOn();

    spAllocFunc = AllocForNAM;
	spFreeFunc = FreeForNAM;

	gNandAppNum = 0;
	gErrorFlg = FALSE;
    
	FS_Init( FS_DMA_NOT_USE );
    
    NAM_Init(AllocForNAM, FreeForNAM);

	MI_CpuClear8( gDataList, sizeof(gDataList));

    ClearScreen();

	ReadTWLSettings();
    
    // hash Check
	ProcessTitleHashCheck();

    while(TRUE)
    {
        // �L�[���͏��擾
		ReadKey(&gKey);

        if (gKey.trg & PAD_KEY_DOWN)
        {
			gCurPos++;
            
            if ( gCurrentPage != gMaxPage )
            {
				if ( gCurPos >= TITLE_MAX_SHOW )
                {
                    gCurrentPage++;
					gCurPos = 0;
                }
            }
            else
            {
                if( (gNandAppNum & 0x0f) == 0 )
                {
					if( gCurPos >= TITLE_MAX_SHOW )
                    {
                    	gCurrentPage = 0;
						gCurPos = 0;
                    }
                }
				else if ( gCurPos >= (gNandAppNum & 0x0f) ) // �o�O
                {
                    gCurrentPage = 0;
					gCurPos = 0;
                }
            }

            OS_TPrintf("�� gCurPos : %x, gCurrentPage : %x, gNandAppNum : %x\n", gCurPos, gCurrentPage, gNandAppNum);
        }
        if (gKey.trg & PAD_KEY_UP)
        {
            if( gCurPos == 0)
            {
            	if ( gCurrentPage == 0 )
            	{
                    gCurrentPage = gMaxPage;
                    gCurPos = ((gNandAppNum & 0x0f) == 0) ? TITLE_MAX_SHOW - 1 : (gNandAppNum & 0x0f) - 1;
            	}
            	else
            	{
                    gCurrentPage--;
					gCurPos = TITLE_MAX_SHOW - 1;
            	}
            }
            else
            {
				gCurPos--;
            }

            OS_TPrintf("�� gCurPos : %x, gCurrentPage : %x, gNandAppNum : %x\n", gCurPos, gCurrentPage, gNandAppNum);
		}
        
        if (gKey.trg & PAD_KEY_LEFT)
        {
            if(gCurrentPage == 0)
            {
				gCurrentPage = gMaxPage;
            }
            else
            {
				gCurrentPage--;
            }

			gCurPos = 0;

            OS_TPrintf("�� gCurPos : %x, gCurrentPage : %x, gNandAppNum : %x\n", gCurPos, gCurrentPage, gNandAppNum);
        }
        if (gKey.trg & PAD_KEY_RIGHT)
        {
			if(gCurrentPage == gMaxPage)
            {
				gCurrentPage = 0;
            }
            else
            {
				gCurrentPage++;
            }

            gCurPos = 0;

            OS_TPrintf("�� gCurPos : %x, gCurrentPage : %x, gNandAppNum : %x\n", gCurPos, gCurrentPage, gNandAppNum);
        }

        // �I�𒆂̗v�f
        gCurrentElem = (s32)((u32)(gCurrentPage << 4) + (u32)gCurPos);
        
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
    0xf0, // ��				0xf1, // ��				0xf2, // ��
  	0xf3, // ��				0xf4, // ��				0xf5, // �s���N
	0xf6, // ���F			0xf7, // �����񂾐�		0xf8, // �����񂾗�
	0xf9, // �����񂾐� 	0xfa, // �����񂾉��F	0xfb, // ��
    0xfc, // ��������		0xfd, // �D�F			0xfe, // �Z���D�F
 *---------------------------------------------------------------------------*/
#define OK_COLOR		((u8)0xf2)
#define NG_COLOR		((u8)0xf1)

#define GAME_CODE_BASE_X		1

static void DrawScene(DataStruct* list)
{
	s32 i;
	u8 init_code[5];
    u8 color;
    u32 start;
    u8* digest;

    DataStruct* p = list;

	if( gErrorFlg )
    {
		PutMainScreen( 10, 12, 0xf1, "--- Error ---");
		PutSubScreen(  10, 12, 0xf1, "--- Error ---");

        return;
    }

	// ����	�ꗗ�\��
	PutMainScreen( 0,  0, 0xf4, "------ Title Hash Checker ------");
    PutMainScreen( 1,  1, 0xff, "<Page %d/%d>", (gCurrentPage+1), (gMaxPage+1));
    PutMainScreen( 1,  3, 0xfa, "GameCode   Version   DebugFlg");
	PutMainScreen( 0,  4, 0xff, "--------------------------------");
    
	// �J�[�\���\��
    if( gCurPos <= TITLE_MAX_SHOW ){
		PutMainScreen( 0, gCurPos+TITLE_SHOW_BASE_Y , 0xf1, ">");
    }

	start = (u32)(gCurrentPage << 4);
	p += start;
    
    for ( i=(s32)start; i < (start + TITLE_MAX_SHOW); i++, p++)
    {
		s32 tmp_i;

		if ( i >= gNandAppNum )
		{
			break;
		}

        tmp_i = (s32)(i & 0xf);
        
        if( i == (gNandAppNum - SHARED_FONT_INFO_OFS) )
        {
    		PutMainScreen( GAME_CODE_BASE_X, TITLE_SHOW_BASE_Y+tmp_i, OK_COLOR, "%2d:Shared Font", (tmp_i+1));
        }
        else if( i == (gNandAppNum - NAND_FIRM_INFO_OFS) )
        {
    		PutMainScreen( GAME_CODE_BASE_X, TITLE_SHOW_BASE_Y+tmp_i, OK_COLOR, "%2d:Nand Firm", (tmp_i+1));
        }
        else
        {
			ConvertInitialCode(init_code, NAM_GetTitleIdLo(p->id));

            color = (list[i].DisableDebugFlg == '1' || list[i].DisableDebugFlg == '-') ? OK_COLOR : NG_COLOR;
            
        	// �Q�[���R�[�h�\��
    		PutMainScreen( GAME_CODE_BASE_X, TITLE_SHOW_BASE_Y+tmp_i, color, "%2d:%s    %d.%d       (%c)",
					   		(tmp_i+1), init_code, list[i].ver_major, list[i].ver_minor, list[i].DisableDebugFlg);
        }
    }
    
    // �����	�ڍו\��
	ConvertInitialCode(init_code, NAM_GetTitleIdLo(list[gCurrentElem].id));

    if( gCurrentElem == (gNandAppNum - SHARED_FONT_INFO_OFS) )
    {
		PutSubScreen(2,   1, 0xf6, "Shared Font Hash Data");
    }
	else if( gCurrentElem == (gNandAppNum - NAND_FIRM_INFO_OFS) )
    {
		PutSubScreen(2,   1, 0xf6, "Nand Firm Hash Data");
    }
    else
    {
    	PutSubScreen(2,   1, 0xf4, "Selected Title : [ %s ]", init_code);
    	PutSubScreen(2,   4, 0xff, "- CRC16 Data -");
	
		PutSubScreen(2,   6, 0xf4, "0x%04x", list[gCurrentElem].crc16);
    }

	digest = (u8 *)list[gCurrentElem].Sha1_digest;
    
    PutSubScreen(2,  10, 0xff, "- SHA1 Digest Data -");

    for( i=0; i<10; i++ ){
		PutSubScreen(2 + (i*3),  12, 0xf4, "%02x ", digest[i]);
        PutSubScreen(2 + (i*3),  14, 0xf4, "%02x ", digest[i+10]);
    }
    
    PutMainScreen( 0, TITLE_MAX_SHOW + TITLE_SHOW_BASE_Y, 0xff, "--------------------------------");
    PutMainScreen( 0, TITLE_MAX_SHOW + TITLE_SHOW_BASE_Y + 1, 0xff, "LR Button     : Mode Change");
    PutMainScreen( 0, TITLE_MAX_SHOW + TITLE_SHOW_BASE_Y + 2, 0xff, "Up Down Key   : Next Application");
}


/*---------------------------------------------------------------------------*
  Name:         ProcessTitleHashCheck

  Description:  
 *---------------------------------------------------------------------------*/
BOOL ProcessTitleHashCheck( void )
{
    u32 i;
    s32 result = TRUE;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

    DataStruct* list;
	
    
    // NAND �ɃC���|�[�g����Ă���NAND �A�v���̐����擾����
    if ( (gNandAppNum = NAM_GetNumTitles()) < 0)
    {
		OS_Panic("NAM_GetNumTitles() failed.");
	}

   	// ���̎擾
	if ( !GetDataStruct(gDataList) )
	{
		OS_Panic("GetDataStruct() failed.");
	}
    
	list = gDataList;
    
    // srl��Hash�l��crc16�����߂�
    for ( i=0; i < gNandAppNum; i++, list++ )
    {
        // �o�b�t�@�̃N���A
		MI_CpuClear8(full_path, sizeof(full_path));

   		// �t�@�C���p�X�̎擾
		if ( !GetAppPath(list, full_path) )
		{
            gErrorFlg = TRUE;
		}

		// Hash, CRC16�̌v�Z
		if ( !CulcuNandAppHash(list, full_path) )
		{
            gErrorFlg = TRUE;
		}
        
		PutMainScreen( 7, 10, 0xf6, "--- Now Loading ---");
        PutMainScreen( 7, 14, 0xf6, " %2d / %2d compleate", i+1, gNandAppNum);
        
        PutSubScreen( 7, 10, 0xf6, "--- Now Loading ---");
        PutSubScreen( 7, 14, 0xf6, " %2d / %2d compleate", i+1, gNandAppNum);
    }

    // Shared�t�H���g��Nand�t�@�[���̒l�p��2��
	gNandAppNum += 2;

    // Shared�t�H���g��Sha1�l�����߂�
	CulcuFontDataHash(gDataList);

    // NandFirm��Sha1�l�����߂�
    CulcuNandFirmHash(gDataList);
    
	OS_PutString("ProcessTitleHashCheck Finish!!\n");
    
    return result;
}


/*---------------------------------------------------------------------------*
  Name:         GetDataStruct

  Description:  TitleID���X�g���擾����֐�
    			NAM_GetTitleList �� NAM_GetInstalledTitleList ���g��
 *---------------------------------------------------------------------------*/
BOOL GetDataStruct(DataStruct* list)
{
	// �^�C�g��ID���X�g�o�b�t�@
	NAMTitleId titleIdList[TITLE_NUM_PAGE];
	s32 i;

    // --- GetTitleList
	if ( NAM_GetTitleList(titleIdList, TITLE_NUM_PAGE) != NAM_OK )
	{
		OS_PutString("NAM_GetTitleList failed.");
		return FALSE;
	}
	
	// �f�[�^���X�g�̍쐬
	for ( i=0; i<TITLE_NUM_PAGE; i++, list++)
	{
		NAMTitleInfo info;
		
		// �������� NAND �A�v���̐��� 1�y�[�W�ɂ������Ȃ��ꍇ�͓r���ŏI������
		if ( i >= gNandAppNum )
		{
			break;
		}
		
        OS_TPrintf("id : 0x%08x\n", titleIdList[i]);
		list->id = titleIdList[i];
		
        // tad�o�[�W�����̎擾
        if( NAM_ReadTitleInfo( &info, list->id ) != NAM_OK )
        {
			OS_TPrintf("[0x%08x] ReadTitleInfo failed...\n", list->id);
		}else {
			list->ver_major = (u8)(info.version >> 8);;
			list->ver_minor = (u8)(info.version & 0xff);
		}
	}

	MI_CpuClear8(titleIdList, sizeof(titleIdList));

	OS_PutString("\n\n");

	return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         GetAppPath

  Description:  

  Arguments:    
	            

  Returns:      
	            
 *---------------------------------------------------------------------------*/
static BOOL	GetAppPath(DataStruct* list, char* path_buf)
{
    FSFile	dir;
    BOOL	ret = TRUE;

    // �f�B���N�g���p�X�̍쐬
    STD_TSNPrintf( path_buf, FS_ENTRY_LONGNAME_MAX,
			   "nand:/title/%08x/%08x/content/", (u32)( list->id >> 32 ), list->id );
    
	// .app�t�@�C����������
    if ( !FS_OpenDirectory(&dir, path_buf, FS_FILEMODE_R | FS_FILEMODE_W) )
    {
    	ret = FALSE;
    	OS_PutString("Error FS_OpenDirectory\n\n");
    }
    else
    {
    	FSDirectoryEntryInfo   info[1];

		// .app ��T���ăt�@�C������ۑ����Ă���
    	while (FS_ReadDirectory(&dir, info))
    	{
        	if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) != 0)
        	{
        	}
        	else
        	{
            	char* pExtension;
          		OS_Printf(" (%d BYTEs)\n", info->filesize);

				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".app"))
					{
                        STD_ConcatenateString( path_buf, info->longname );

                        OS_TPrintf("OK! File Path : %s\n", path_buf);
                        
                        break;
					}
				}
        	}
    	}
        
    	(void)FS_CloseDirectory(&dir);
    }

    return ret;
}


/*---------------------------------------------------------------------------*
  Name:         CulcuNandAppHash

  Description:  

  Arguments:    

  Returns:      

 *---------------------------------------------------------------------------*/
#define READ_SIZE	0x1000 // MasterEditor�ł�CRC�v�Z�ɂ��킹�邽�߂��̒l���g��

BOOL	CulcuNandAppHash(DataStruct* list, char* full_path)
{
    FSFile  file;
    BOOL    open_is_ok;
    BOOL	seek_is_ok;
	BOOL    read_is_ok;
	u32 	file_size;
    u32		read_size = 0;
    u32		data_size = 0;
    u32		i;
    u16		crc = 0;
    u8* 	pTempBuf;
    SVCSHA1Context hash;
    
    FSResult result;
    
	// CRC16�v�Z�p�e�[�u���̏�����
    MI_CpuClear8(crc_table, sizeof(crc_table));
    inittable(crc_table);

    // �n�b�V��������
    SVC_SHA1Init( &hash );

    // FS������
    FS_InitFile(&file);

    // �t�@�C���I�[�v��
    open_is_ok = FS_OpenFile(&file, full_path);
	if (!open_is_ok)
	{
		OS_Warning("Failure! FS_OpenFile");
		return FALSE;
	}

    // �t�@�C�����擾
    file_size  = FS_GetFileLength(&file);

    // �t�@�C�����傫�����̂����邩��א؂�œǂ�
    for(i=1;;i++){
        // �o�b�t�@�m��
    	pTempBuf = spAllocFunc( READ_SIZE );
    	if (pTempBuf == NULL)
		{
			FS_CloseFile(&file);
        	OS_Warning("Failure! Alloc Buffer");
			return FALSE;		
		}

        // �ǂݍ��ރT�C�Y�����߂�
		if( (READ_SIZE * i) > file_size )
        {
			data_size = file_size % READ_SIZE;
        }
        else
        {
			data_size = READ_SIZE;
        }
        
		// �t�@�C�����[�h
		read_is_ok = FS_ReadFile( &file, pTempBuf, READ_SIZE ); // �{���Ȃ�΂����� data_size���ǂ߂΂悢���A
        														// MasterEditor��CRC�v�Z�ɂ��킹�邽�߁AREAD_SIZE���ǂݍ���
        result = FS_GetResultCode(&file);
		if (!read_is_ok)
		{
			FS_CloseFile(&file);
			spFreeFunc(pTempBuf);
        	OS_Warning("Failure! Read File (Error Code : %d)", result);
			return FALSE;
		}

        // ���ł�DisableDebugFlg�̏�Ԃ����Ă���
		if(i == 1)
        {
            if( (u32)( list->id >> 32 ) & TITLE_ID_DATA_ONLY_MASK )
            {
				list->DisableDebugFlg = '-';
            }
            else
            {
            	if(((ROM_Header *)pTempBuf)->s.disable_debug)
            	{
					list->DisableDebugFlg = '1';
            	}
            	else
            	{
					list->DisableDebugFlg = '0';
            	}
            }
        }
        
        // �ǂ񂾃T�C�Y���X�V
        read_size += data_size;

    	// CRC16�v�Z
		crc = newGetCRC(crc, (u16 *)pTempBuf, READ_SIZE); 	// �{���Ȃ�΂����� data_size���̌v�Z������΂悢���A
															// MasterEditor��CRC�v�Z�ɂ��킹�邽�߁AREAD_SIZE���v�Z����
		// Hash�l UpDate
		SVC_SHA1Update( &hash, pTempBuf, data_size );
		
        // �t�@�C�����S���ǂ߂���while���ʂ���
        if( read_size >= file_size )
        {
			break;
        }
        else
        {
			// �t�@�C���V�[�N
			seek_is_ok = FS_SeekFile( &file, (s32)(READ_SIZE * i), FS_SEEK_SET );
            result = FS_GetResultCode(&file);
            if(!seek_is_ok)
            {
				FS_CloseFile(&file);
				spFreeFunc(pTempBuf);
        		OS_Warning("Failure! Seek File [read size : %d bytes] (Error Code : %d)", read_size, result);
				return FALSE;
            }
        }
        spFreeFunc( pTempBuf );
    }

    if(file_size != read_size)
    {
		OS_PutString("   Read Size Error!!!\n");
    }

    OS_PutString("\n\n");
    
    // Hash�l�Z�o
	SVC_SHA1GetHash( &hash, list->Sha1_digest );

	list->crc16 = crc;
    
	// �t�@�C���N���[�Y
	FS_CloseFile(&file);

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         CulcuFontDataHash

  Description:  

  Arguments:    

  Returns:      

 *---------------------------------------------------------------------------*/
BOOL CulcuFontDataHash(DataStruct* list)
{
    FSFile  file;
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	u32 	file_size;
    u8* 	pTempBuf;
    SVCSHA1Context hash;
    
    FSResult result;

    // �P�c����2�ڂ�SharedFont�p�̃f�[�^�ɂ���
	DataStruct *data = &list[gNandAppNum - SHARED_FONT_INFO_OFS];
    
    // �n�b�V��������
    SVC_SHA1Init( &hash );

    // FS������
    FS_InitFile(&file);

    // �t�@�C���I�[�v��
    open_is_ok = FS_OpenFile(&file, "nand:/sys/TWLFontTable.dat");
	if (!open_is_ok)
	{
		OS_Warning("Failure! FS_OpenFile");
		return FALSE;
	}

    // �t�@�C�����擾
    file_size  = FS_GetFileLength(&file);

    // �o�b�t�@�m��
   	pTempBuf = spAllocFunc( file_size );
   	if (pTempBuf == NULL)
	{
		FS_CloseFile(&file);
       	OS_Warning("Failure! Alloc Buffer");
		return FALSE;		
	}
        
	// �t�@�C�����[�h
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
    result = FS_GetResultCode(&file);
	if (!read_is_ok)
	{
		FS_CloseFile(&file);
		spFreeFunc(pTempBuf);
    	OS_Warning("Failure! Read File (Error Code : %d)", result);
		return FALSE;
	}

	// Hash�l UpDate
	SVC_CalcSHA1( data->Sha1_digest, pTempBuf, file_size );

    // �o�b�t�@�̊J��
    spFreeFunc( pTempBuf );
    
	// �t�@�C���N���[�Y
	FS_CloseFile(&file);

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         CulcuNandFirmHash

  Description:  

  Arguments:    

  Returns:      

 *---------------------------------------------------------------------------*/
#define NAND_BLOCK_BYTE 			    	0x200
#define NAND_HEADER_SIZE					0x800
#define NAND_FIRM_REV_SIZE					0xa
#define NAND_FIRM_START_OFFSET    	    	0x200
#define TEMP_BUFFER_SIZE					0x100000
#define READ_NAND_FIRM_SIZE					(TEMP_BUFFER_SIZE - NAND_FIRM_START_OFFSET)

BOOL CulcuNandFirmHash(DataStruct* list)
{
	u8* pTempBuf;
    u32 read_block;
    u32 culcu_size;
    u32 alloc_size;
    BOOL ret = TRUE;

	u8* p;
    
    // �P�c����1�ڂ�SharedFont�p�̃f�[�^�ɂ���
	DataStruct *data = &list[gNandAppNum - NAND_FIRM_INFO_OFS];

	// �o�b�t�@�m��
	// �������݂��u���b�N�P��(512byte)�ł��邱�Ƃ��l����512�A���C�����g���m��
	alloc_size = MATH_ROUNDUP(TEMP_BUFFER_SIZE, 512);
	pTempBuf = spAllocFunc( alloc_size ); //pTempBuf = spAllocFunc( READ_NAND_FIRM_SIZE );
	if (pTempBuf == NULL)
	{
        OS_Warning("Failure! Alloc Buffer");
		return FALSE;
	}
	MI_CpuClear8( pTempBuf, TEMP_BUFFER_SIZE ); //MI_CpuClear8( pTempBuf, READ_NAND_FIRM_SIZE );

    // ------ Nvram���烊�[�h ------
	DC_FlushRange(pTempBuf, sizeof(NORHeaderDS));
    
	if (NVRAMi_Read(0, sizeof(NORHeaderDS), pTempBuf) != NVRAM_RESULT_SUCCESS)
	{
        OS_Warning("Fail NVRAMi_Read()!");
	}
	DC_FlushRange(pTempBuf, sizeof(NORHeaderDS));

    
	// ------ NAND���烊�[�h ------
	DC_FlushRange( pTempBuf, TEMP_BUFFER_SIZE );
    
    read_block = READ_NAND_FIRM_SIZE / NAND_BLOCK_BYTE + (READ_NAND_FIRM_SIZE % NAND_BLOCK_BYTE != 0);
	if (kamiNandRead(NAND_FIRM_START_OFFSET/NAND_BLOCK_BYTE, pTempBuf+NAND_FIRM_START_OFFSET, read_block ) == KAMI_RESULT_SEND_ERROR)
	{
	    OS_Warning("Failure! Read NandFirm");
	}
	DC_FlushRange( pTempBuf, TEMP_BUFFER_SIZE );

    
    // ------ Hash�̌v�Z ------
	// Hash�v�Z����T�C�Y�����߂�
    culcu_size = MATH_ROUNDUP((*(u32 *)(pTempBuf+0x22c)) + (*(u32 *)(pTempBuf+0x23c)) , 256) + NAND_HEADER_SIZE + NAND_FIRM_REV_SIZE;
    OS_TPrintf("NandFirm main Size : 0x%08x\n", (*(u32 *)(pTempBuf+0x22c)));
    OS_TPrintf("NandFirm  sub Size : 0x%08x\n", (*(u32 *)(pTempBuf+0x23c)));
    OS_TPrintf("NandFirm      Size : 0x%08x\n", culcu_size);

	p = (pTempBuf + (culcu_size - NAND_FIRM_REV_SIZE));
    
	OS_TPrintf("Nand Firm rev      : %c%c%c%c %c%c%c%c\n\n"
               ,*p,*(p+1),*(p+2),*(p+3),*(p+5),*(p+6),*(p+7),*(p+8));
    
	// Hash�l UpDate
    if( culcu_size <= READ_NAND_FIRM_SIZE )
    {
        SVC_CalcSHA1( data->Sha1_digest, pTempBuf, culcu_size );
    }
    else
    {
		OS_TPrintf("Culcu Size is illegal...\n");
        ret = FALSE;
    }
	// ���������
	spFreeFunc(pTempBuf);

    return ret;
}


/*---------------------------------------------------------------------------*
  Name:         newGetCRC

  Description:	CRC�v�Z
   				���i�Z�p����CRC�e�[�u�����g����CRC�v�Z�֐�

				�����o�C�g�Ɗ�o�C�g�����ւ��Ă���v�Z����B
 *---------------------------------------------------------------------------*/
u16 newGetCRC
	( 
	u16  start,    // CRC�����l�i�ݐϒl�j 
	u16 *datap,    // �f�[�^�̐擪���w���|�C���^�A
	u32  size      // �o�C�g�P�ʂł̃f�[�^�T�C�Y
	)
{
	u32 i;
	u16 crc;
	u8* byte;

	crc  = start;
	byte = (u8 *)datap;

	for (i=0; i<size; i+=2)
	{
		u8 byte0, byte1;

		byte0 = *byte;  byte++;
		byte1 = *byte;  byte++;

		crc = (u16)((crc << 8) ^ crc_table[(crc >> 8) ^ byte1]);
		crc = (u16)((crc << 8) ^ crc_table[(crc >> 8) ^ byte0]);
	}

	return crc;
}


/*----------------------------------------------------------------------------
  �֐���
    inittable - initialize table

  �`��
    static void inittable(unsigned short *table);

  ���
    CRC�Z�o�����������邽�߂̎Q�ƃe�[�u�����쐬����B
----------------------------------------------------------------------------*/
static void inittable(unsigned short *table)
{
    unsigned short  i, j, r;

    for(i = 0; i < 0x100; i++) {
        r = (unsigned short)(i << 8);
        for(j = 0; j < 8; j++) {
            if(r & 0x8000U)
                r = (unsigned short)((r << 1) ^ CRCPOLY);
            else
                r <<= 1;
        }
        *table++ = r;
    }
}


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


// TWL�ݒ�f�[�^�̃��[�h
static BOOL ReadTWLSettings( void )
{
    u8 *pBuffer = OS_AllocFromMain( LCFG_READ_TEMP );
    BOOL result;
    if( pBuffer ) {
        result = LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ] )pBuffer );
		// Read�Ɏ��s�����ꍇ�t�@�C���̃��J�o�����������݂�
		if (!result)
		{
			OS_TPrintf( "TSD read failed. Retry onece more.\n" );
	        result = LCFG_RecoveryTWLSettings();
		}
        OS_FreeToMain( pBuffer );
    }
    if( result ) {
        OS_TPrintf( "TSD read succeeded.\n" );
    }else {
        OS_TPrintf( "TSD read failed.\n" );
    }

	return result;
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
