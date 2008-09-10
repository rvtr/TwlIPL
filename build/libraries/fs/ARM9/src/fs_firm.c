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

//#define PROFILE_ES    0x02FFCB00

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
    // FS/FATFS�̑S�������N���A
    MI_CpuClearFast((void*)HW_FIRM_FATFS_ARCHNAME_LIST, HW_FIRM_FS_FATFS_ASYNC_REQUEST_END - HW_FIRM_FATFS_ARCHNAME_LIST);
    FATFS_InitFIRM();
    FS_Init( FS_DMA_NOT_USE );
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SetupFATBuffers

  Description:  override weak function

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
void FSi_SetupFATBuffers(void)
{
    FSiFATFSDrive           = (void*)HW_FIRM_FS_FATFS_DRIVE;
    FSiFATFSAsyncRequest    = (void*)HW_FIRM_FS_FATFS_ASYNC_REQUEST;
    FSiTemporaryBuffer      = (void*)HW_FIRM_FS_TEMP_BUFFER;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_OverrideRomArchive

  Description:  override weak function

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FSi_OverrideRomArchive(FSArchive *arc)
{
    (void)arc;
    return FALSE;
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
  Name:         SEA_Decrypt
                SEA_GetCryptoBufferAddr
                SEA_GetCryptoBufferSize

  Description:  stub function

  Arguments:

  Returns:
 *---------------------------------------------------------------------------*/
AESResult SEA_Decrypt(const void* src, u32 srcSize, void* dst);
SDK_WEAK_SYMBOL AESResult SEA_Decrypt(const void* src, u32 srcSize, void* dst)
 __attribute__((never_inline))
{
(void)src;
(void)srcSize;
(void)dst;
    return AES_RESULT_SUCCESS;
}
u32 SEA_GetCryptoBufferAddr(void);
SDK_WEAK_SYMBOL u32 SEA_GetCryptoBufferAddr(void) __attribute__((never_inline)) { return HW_FIRM_ES_BUF; }
u32 SEA_GetCryptoBufferSize(void);
SDK_WEAK_SYMBOL u32 SEA_GetCryptoBufferSize(void) __attribute__((never_inline)) { return HW_FIRM_ES_BUF_SIZE; }

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrl

  Description:  resolve srl filename and store to HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    titleId         title id for srl file

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_ResolveSrl( OSTitleId titleId )
{
    MI_CpuClearFast( (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE );
    if ( ES_ERR_OK != ES_InitLib() ||
         ES_ERR_OK != ES_GetContentPath(titleId, CONTENT_INDEX_BOOT, (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF) ||
         ES_ERR_OK != ES_CloseLib() )
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrlList

  Description:  resolve srl filename with list and store to
                HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    titleIdList     pointer to title id array for srl file
                nums            number of title id

  Returns:      -1 if failed, otherwise succeeded titile id number
 *---------------------------------------------------------------------------*/
int FS_ResolveSrlList( const OSTitleId* titleIdList, u32 nums )
{
    int i;
#ifdef PROFILE_ES
    OSTick tick[8];
    char*ptr=(char*)PROFILE_ES;
    tick[0] = OS_GetTick();
#endif
    MI_CpuClearFast( (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE );
#ifdef PROFILE_ES
    tick[1] = OS_GetTick();
#endif
/*
    ES_InitLib()�ł́ASEA_GetCryptoBuffer[Addr|Size]()�œn����郁�������g�p����B
    (SEA_Decrypt()���Ăяo�����t�@�[���ł͌��ʂ��g��Ȃ�)
    �܂��ACRYPTO���C�u�����o�R�ŁAES_GetContentPath()�ŏ����̂Ɠ����T�C�Y����
    �������̓��I�m�ۂ�����\��������B
*/
    if ( !titleIdList || !nums || ES_ERR_OK != ES_InitLib() )
    {
        return FALSE;
    }
    for ( i = 0; i < nums; i++ )
    {
#ifdef PROFILE_ES
    tick[2+i] = OS_GetTick();
#endif
        if ( ES_ERR_OK == ES_GetContentPath(titleIdList[i], CONTENT_INDEX_BOOT, (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF) )
        {
            break;
        }
    }
#ifdef PROFILE_ES
    tick[3+i] = OS_GetTick();
#endif
    if ( ES_ERR_OK != ES_CloseLib() )
    {
        return FALSE;
    }
#ifdef PROFILE_ES
    tick[4+i] = OS_GetTick();
    tick[5+i] = tick[4+i];
    for(i=0;i<7;i++)
        ptr+=STD_TSPrintf(ptr, "tick[%d]:%d\n",i,(int)OS_TicksToMicroSeconds(tick[i+1]-tick[i]));
#endif
    return (i == nums ? -1 : i);
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
    MI_CpuClearFast( (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE );
    if ( !FS_GetTitleBootContentPathFast((char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, titleId) )
    {
        return FALSE;
    }
    return TRUE;
}


