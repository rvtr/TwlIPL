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
    // FS/FATFSの全メモリクリア
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
    ES_InitLib()では、SEA_GetCryptoBuffer[Addr|Size]()で渡されるメモリを使用する。
    (SEA_Decrypt()も呼び出すがファームでは結果を使わない)
    また、CRYPTOライブラリ経由で、ES_GetContentPath()で消費するのと同じサイズだけ
    メモリの動的確保をする可能性がある。
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


