/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_firm.c

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
#include <firm.h>
#include <es.h>

// bootContent ��\������� contentIndex
#define CONTENT_INDEX_BOOT          0xFFFF

#define PATH_FORMAT_TMD         "nand:/title/%08x/%08x/content/title.tmd"
#define PATH_FORMAT_CONTENT     "nand:/title/%08x/%08x/content/%08x.app"

/*---------------------------------------------------------------------------*
  Name:         FS_InitFIRM

  Description:  initialize FS/FATFS for firm

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FS_InitFIRM( void )
{
    MI_CpuClearFast( (void*)HW_FIRM_FS_TEMP_BUFFER, HW_FIRM_FS_TEMP_BUFFER_SIZE );
    FSiTemporaryBuffer = (void*)HW_FIRM_FS_TEMP_BUFFER;
    FATFS_InitFIRM();
    FS_Init( FS_DMA_NOT_USE );
}

/*---------------------------------------------------------------------------*
  Name:         LoadTMD

  Description:  �Ώۂ̃^�C�g���� TMD �t�@�C�����������ɓǂݍ��݂܂��B
                ����������ɍs��ꂽ�ꍇ�ɓ�����o�b�t�@�ւ̃|�C���^��
                �s�v�ɂȂ����� NAMi_Free �ŉ������K�v������܂��B

  Arguments:    pTmd:       �^�C�g�����^�f�[�^�̓ǂݍ��ݐ�
                titleId:    �Ώۂ̃^�C�g���� TitleID�B

  Returns:      ����������ɍs��ꂽ�Ȃ� TRUE��Ԃ��܂��B
                �����łȂ���� FALSE ��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
static BOOL LoadTMD(ESTitleMeta* pTmd, OSTitleId titleId)
{
    char path[64];
    FSFile f;
    BOOL bSuccess;
    u32 fileSize;
    s32 readSize;
    s32 readResult;

    // TMD �̃p�X�𐶐�
    STD_TSPrintf(path, PATH_FORMAT_TMD, (u32)(titleId >> 32), (u32)titleId);

    FS_InitFile(&f);
    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
        // �t�@�C�����J���Ȃ�����
        return FALSE;
    }

    fileSize = FS_GetFileLength(&f);

    // �t�@�C���T�C�Y���`�F�b�N 1
    // �Œ蕔���T�C�Y <= fileSize <= �Œ蕔���T�C�Y + �ϒ������ő�T�C�Y
    if( (fileSize < sizeof(IOSCSigRsa2048) + sizeof(ESTitleMetaHeader))
     || (sizeof(ESTitleMeta) < fileSize) )
    {
        // �t�@�C���T�C�Y���ُ�
        FS_CloseFile(&f);
        return FALSE;
    }

    readSize = (s32)fileSize;
    readResult = FS_ReadFile(&f, pTmd, readSize);
    FS_CloseFile(&f);

    if( readResult != readSize )
    {
        // �t�@�C������̓ǂݍ��݂Ɏ��s
        return FALSE;
    }

    // �t�@�C���T�C�Y���`�F�b�N 2
    // �ϒ������𐳂����l��
    if( fileSize != sizeof(IOSCSigRsa2048)
                  + sizeof(ESTitleMetaHeader)
                  + sizeof(ESContentMeta) * MI_SwapEndian16(pTmd->head.numContents) )
    {
        // �t�@�C���T�C�Y���ُ�
        return FALSE;
    }

    // �^�C�g�� ID �̈�v���`�F�b�N
    if( titleId != MI_SwapEndian64(pTmd->head.titleId) )
    {
        // �^�C�g�� ID ����v���Ȃ�
        return FALSE;
    }

    return TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         FS_GetTitleBootContentPathFast

  Description:  NAND �ɃC���X�g�[������Ă���A�v���̎��s�t�@�C���̃p�X��
                �擾���܂��B
                �擾������̐����������؂��Ȃ����ߍ����ł����A
                ��񂪉�₂���Ă���\�������邱�Ƃɒ��ӂ��Ȃ���΂Ȃ�܂���B

  Arguments:    buf:        �p�X���i�[����o�b�t�@�ւ̃|�C���^�B
                            FS_ENTRY_LONGNAME_MAX �ȏ�̃T�C�Y���K�v�ł��B
                titleId:    �p�X���擾����A�v���� Title ID�B

  Returns:      ����ɏ������s��ꂽ�Ȃ� TRUE ��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
BOOL FS_GetTitleBootContentPathFast(char* buf, OSTitleId titleId)
{
    ESTitleMeta tmd;
    u32 bootContentId;
    int bootContentIndex;
    int numContents;
    int i;

    SDK_POINTER_ASSERT(buf);



    if( !LoadTMD(&tmd, titleId) )
    {
        return FALSE;
    }

    // ���� TMD �� BigEndian
    bootContentIndex = MI_SwapEndian16(tmd.head.bootIndex);
    numContents      = MI_SwapEndian16(tmd.head.numContents);

    // bootContentIndex �Ɉ�v����R���e���c��T��
    for( i = 0; i < numContents; ++i )
    {
        const ESContentMeta* pContent = &tmd.contents[i];

        if( MI_SwapEndian16(pContent->index) == bootContentIndex )
        {
            bootContentId = MI_SwapEndian32(pContent->cid);
            break;
        }
    }

    if( i >= numContents )
    {
        return FALSE;
    }

    // �R���e���c�̃p�X�𐶐�
    STD_TSPrintf(buf, PATH_FORMAT_CONTENT, (u32)(titleId >> 32), (u32)titleId, bootContentId);

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrl

  Description:  resolve srl filename and store to HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    titleId         title id for srl file

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_ResolveSrl( OSTitleId titleId )
{
    if ( ES_ERR_OK != ES_InitLib() ||
         ES_ERR_OK != ES_GetContentPath(titleId, CONTENT_INDEX_BOOT, (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF) ||
         ES_ERR_OK != ES_CloseLib() )
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrlUnsecured

  Description:  resolve srl filename and store to HW_TWL_FS_BOOT_SRL_PATH_BUF
                without almost security check

  Arguments:    titleId         title id for srl file

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_ResolveSrlUnsecured( OSTitleId titleId )
{
    if ( !FS_GetTitleBootContentPathFast((char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, titleId) )
    {
        return FALSE;
    }
    return TRUE;
}


