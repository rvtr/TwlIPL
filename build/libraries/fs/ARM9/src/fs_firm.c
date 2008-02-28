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

// bootContent を表す特殊な contentIndex
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

  Description:  対象のタイトルの TMD ファイルをメモリに読み込みます。
                処理が正常に行われた場合に得られるバッファへのポインタは
                不要になったら NAMi_Free で解放する必要があります。

  Arguments:    pTmd:       タイトルメタデータの読み込み先
                titleId:    対象のタイトルの TitleID。

  Returns:      処理が正常に行われたなら TRUEを返します。
                そうでなければ FALSE を返します。
 *---------------------------------------------------------------------------*/
static BOOL LoadTMD(ESTitleMeta* pTmd, OSTitleId titleId)
{
    char path[64];
    FSFile f;
    BOOL bSuccess;
    u32 fileSize;
    s32 readSize;
    s32 readResult;

    // TMD のパスを生成
    STD_TSPrintf(path, PATH_FORMAT_TMD, (u32)(titleId >> 32), (u32)titleId);

    FS_InitFile(&f);
    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
        // ファイルが開けなかった
        return FALSE;
    }

    fileSize = FS_GetFileLength(&f);

    // ファイルサイズをチェック 1
    // 固定部分サイズ <= fileSize <= 固定部分サイズ + 可変長部分最大サイズ
    if( (fileSize < sizeof(IOSCSigRsa2048) + sizeof(ESTitleMetaHeader))
     || (sizeof(ESTitleMeta) < fileSize) )
    {
        // ファイルサイズが異常
        FS_CloseFile(&f);
        return FALSE;
    }

    readSize = (s32)fileSize;
    readResult = FS_ReadFile(&f, pTmd, readSize);
    FS_CloseFile(&f);

    if( readResult != readSize )
    {
        // ファイルからの読み込みに失敗
        return FALSE;
    }

    // ファイルサイズをチェック 2
    // 可変長部分を正しく考慮
    if( fileSize != sizeof(IOSCSigRsa2048)
                  + sizeof(ESTitleMetaHeader)
                  + sizeof(ESContentMeta) * MI_SwapEndian16(pTmd->head.numContents) )
    {
        // ファイルサイズが異常
        return FALSE;
    }

    // タイトル ID の一致をチェック
    if( titleId != MI_SwapEndian64(pTmd->head.titleId) )
    {
        // タイトル ID が一致しない
        return FALSE;
    }

    return TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         FS_GetTitleBootContentPathFast

  Description:  NAND にインストールされているアプリの実行ファイルのパスを
                取得します。
                取得する情報の正当性を検証しないため高速ですが、
                情報が改竄されている可能性があることに注意しなければなりません。

  Arguments:    buf:        パスを格納するバッファへのポインタ。
                            FS_ENTRY_LONGNAME_MAX 以上のサイズが必要です。
                titleId:    パスを取得するアプリの Title ID。

  Returns:      正常に処理が行われたなら TRUE を返します。
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

    // 生の TMD は BigEndian
    bootContentIndex = MI_SwapEndian16(tmd.head.bootIndex);
    numContents      = MI_SwapEndian16(tmd.head.numContents);

    // bootContentIndex に一致するコンテンツを探す
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

    // コンテンツのパスを生成
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


