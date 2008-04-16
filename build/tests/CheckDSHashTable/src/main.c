/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - CheckDSHashTable
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

#include <twl.h>
#include <sysmenu/dht/dht.h>

/*
    ���̃T���v���̓e�X�g�p�ł���ACARD�֐��̑����FS�֐���
    DHT���C�u��������������Ă���O��ł���
*/

#define ROM_PATH    "/srl"

#define DS_HASH_TABLE_NUMS      3000
#define DS_HASH_TABLE_SIZE      (sizeof(DHTHeader) + DS_HASH_TABLE_NUMS * sizeof(DHTDatabase))

/*
    DHT�t�@�C���ǂݍ��ݐ�
*/
static u8 dht_buffer[DS_HASH_TABLE_SIZE] ATTRIBUTE_ALIGN(32);
static DHTFile *const dht = (DHTFile*)dht_buffer;
/*
    Phase2�p�o�b�t�@
*/
static DHTPhase2Work p2work;

/*
    ���ۂɂ̓A�h���X�Œ�
*/
#include <twl/ltdmain_begin.h>
static ROM_Header_Short rom_header ATTRIBUTE_ALIGN(32); // �g���̂� DHT_DS_HEADER_SIZE ����
static u8 rom_arm9[3*1024*1024] ATTRIBUTE_ALIGN(32);
static u8 rom_arm7[1024*1024] ATTRIBUTE_ALIGN(32);
#include <twl/ltdmain_end.h>

/*
    �f�o�C�X�A�N�Z�XAPI
*/
#if 1
// FS�� (fctx == FSFile*)
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    FSFile* fp = arg;
    s32 result;
    if ( !FS_SeekFile(fp, offset, FS_SEEK_SET) )
    {
        OS_TPrintf("Cannot seek to the offset (%d bytes).\n", offset);
        return FALSE;
    }
    result = FS_ReadFile(fp, dest, length);
    if ( result != length )
    {
        OS_TPrintf("Cannot read the data (%d bytes).\n", length);
        return FALSE;
    }
    return TRUE;
}
#else
#ifdef SDK_ARM9
#define PAGE_SIZE 512
// CARD�� (fctx == dma no)
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    u32 dma = (u32)arg;
    CARD_ReadRom(dma, (void*)offset, dest, (u32)length);
    return TRUE;
}
#else
// HOTSW�� (fctx == CardBootData* cdb)
#include    <hotswTypes.h>
#include    <dsCardCommon.h>
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    HotSwState retval = ReadPageGame((CardBootData*)arg, (u32)offset, dest, (u32)length);
    return (retval == HOTSW_SUCCESS);
}
#endif
#endif

static void VBlankIntr( void )
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*
    ���ؖ{�Ԃ̂����A���ۂɂ̓I�[�o�[�w�b�h�ɂȂ�Ȃ�����
*/
static BOOL CheckValidation(FSFile* fp)
{
    const DHTDatabase *db;
    s32 result;

    // �w�b�_�̓ǂݍ���
    result = FS_ReadFile(fp, &rom_header, DHT_DS_HEADER_SIZE);
    if ( result != DHT_DS_HEADER_SIZE )
    {
        OS_TPrintf("Cannot read ROM header.\n");
        return FALSE;
    }
    // ARM9����
    if ( rom_header.main_size > sizeof(rom_arm9) )
    {
        OS_TPrintf("Too large main size (%d > %d).\n", rom_header.main_size, sizeof(rom_arm9));
        return FALSE;
    }
    if ( !FS_SeekFile(fp, (s32)rom_header.main_rom_offset, FS_SEEK_SET) )
    {
        OS_TPrintf("Cannot seek to ARM9 static.\n");
        return FALSE;
    }
    result = FS_ReadFile(fp, rom_arm9, (s32)rom_header.main_size);
    if ( result != rom_header.main_size )
    {
        OS_TPrintf("Cannot read ARM9 static.\n");
        return FALSE;
    }
    // ARM7����
    if ( rom_header.sub_size > sizeof(rom_arm7) )
    {
        OS_TPrintf("Too large sub size (%d > %d).\n", rom_header.sub_size, sizeof(rom_arm7));
        return FALSE;
    }
    if ( !FS_SeekFile(fp, (s32)rom_header.sub_rom_offset, FS_SEEK_SET) )
    {
        OS_TPrintf("Cannot seek to ARM7 static.\n");
        return FALSE;
    }
    result = FS_ReadFile(fp, rom_arm7, (s32)rom_header.sub_size);
    if ( result != rom_header.sub_size )
    {
        OS_TPrintf("Cannot read ARM7 static.\n");
        return FALSE;
    }

    // �f�[�^�x�[�X�̌���
    db = DHT_GetDatabase(dht, &rom_header);
    if ( !db )
    {
        return FALSE;
    }
//OS_TPrintf("FOUND: 0x%08X: %.4s(%d)\n", db, db->game_code, db->rom_version);
    // �n�b�V���v�Z (1) - �B���\�Ȃ͂�
    if ( !DHT_CheckHashPhase1(db->hash[0], &rom_header, rom_arm9, rom_arm7) )
    {
        return FALSE;
    }
    // �n�b�V���v�Z (2) - �B���͓����
    if ( !DHT_CheckHashPhase2(db->hash[1], &rom_header, &p2work, ReadImage, fp) )
    {
        return FALSE;
    }

    return TRUE;
}

void TwlMain(void)
{
    OS_Init();
    OS_InitTick();

    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // ���荞�݋���----------------------------
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    FS_Init(FS_DMA_NOT_USE);

    // �������[�h
    if ( !DHT_PrepareDatabase(dht) )
    {
        OS_TPanic("Cannot prepare the database.\n");
    }

    // �{��
    {
        FSFile dir;
        FSDirectoryEntryInfo info;
        BOOL result;
        result = FS_OpenDirectory(&dir, ROM_PATH, FS_FILEMODE_R);
        if ( !result )
        {
            OS_TPanic("Cannot open " ROM_PATH ".\n");
        }
        while ( FS_ReadDirectory(&dir, &info) )
        {
            FSFile file;
            OSTick begin;
            char path[FS_ENTRY_LONGNAME_MAX+sizeof(ROM_PATH)+1];
            if ( info.attributes & FS_ATTRIBUTE_IS_DIRECTORY )
            {
                OS_TPrintf("%s is directory.\n", info.longname);
                continue;
            }
            STD_CopyString(path, ROM_PATH "/");
            STD_ConcatenateString(path, info.longname);
            if (!FS_OpenFileEx(&file, path, FS_FILEMODE_R))
            {
                OS_TPrintf("Cannot open %s.\n", path);
                continue;
            }
            OS_TPrintf("\nTrying %s...\n", path);
            begin = OS_GetTick();
            if ( !CheckValidation(&file) )
            {
                OS_TPrintf("Failed. %d msec (includes loading static data).\n", (int)OS_TicksToMilliSeconds(OS_GetTick()-begin));
            }
            else
            {
                OS_TPrintf("Success. %d msec (includes loading static data).\n", (int)OS_TicksToMilliSeconds(OS_GetTick()-begin));
            }
            FS_CloseFile(&file);
        }
        FS_CloseDirectory(&dir);
    }

    OS_TPrintf("\nDone.\n");
    OS_Terminate();
}

