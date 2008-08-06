/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util_menuAppManager.c

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

#ifndef TWL_IPL_MENU_APP_MANAGER_FOR_STG
#include <sysmenu/util_menuAppManager.h>
#endif

#include <stddef.h>

#define MEASURE_BANNER_LOAD_TIME 0

void *(*AMNi_Alloc)( u32 size  ) = NULL;
void  (*AMNi_Free )( void *ptr ) = NULL;

/////////////////////////////////////////////////////////////////////////////////
// private:
static BOOL AMN_isTitleIdValidForLauncher(NAMTitleId id);

static void AMN_initFlags_();
static void AMN_initCardTitleList_();
static void AMN_initNandTitleList_();

static void AMN_lockSubBannerFileBuffer();
static void AMN_unlockSubBannerFileBuffer();

static u32  AMN_getBannerAnimeCRC(const BannerAnime* pAnime);
static BOOL AMN_checkAndReplaceBannerAnime(s32 index);

static void AMNi_updateFreeBoxCount( u8 count );

static vu8     sThreadStarted;
static vu8     sReadCancelFlag; // とりあえず無視。
static vu8     sNandTitleListReady;
static vu8     sCardTitleListLength;

static s32     sNandAllTitleListLength; // error if <= 0
static s32     sNandTitleListLengthForLauncher;

static s32     sNandAppRomHeaderArrayLength;

static TitleProperty     sAllTitlePropertyArray[cAllTitleArrayMax];

enum {
    cEventMask_SubBannerFileBuffer = 0x1
};

static OSEvent         sSubBannerFileBufferEvent;

enum {
    cThreadPriority_Read  = OS_THREAD_LAUNCHER_PRIORITY + 1,
    cThreadPriority_Check = OS_THREAD_LAUNCHER_PRIORITY + 2
};

static OSThread        sReadThread;
static OSThread        sCheckThread;
static u64             sReadThreadStack[1024];
static u64             sCheckThreadStack[2048]; // 16KBytes FSとNAMを使うので少し多めに取る
static OSMessage       sCheckMsgBuf[1];
static OSMessageQueue  sCheckMsgQueue;
static void AMN_procRead();
static void AMN_procReadNandTitleOnly();
static void AMN_procReadNandTitleHeaderOnly();
static void AMN_procCheck();

static char    sBootContentPath[FS_ENTRY_LONGNAME_MAX];
static TWLBannerFile*      spNandBannerFileArray;
static TWLBannerFile*      spEmptyBannerFileBuffer;
    // バナーバッファ、サブバナーバッファは0x20 Bytes alignedにしたい。
    // メモリが厳しいのでサブバナーバッファは1個で使い回す。
static TWLSubBannerFile*   spSubBannerFileBuffer;

static AMNBannerCounter       sBannerCounterArray[cAllTitleArrayMax];
static AMNFrameAnimeData      sFrameAnimeDataArray[cAllTitleArrayMax];
static AMNFrameAnimeData      sEmptyAnimeData;

static ROM_Header_Short    sRomHeaderBuffer;

static ROM_Header_Short    s_AllRomHeaderArray[OS_TITLEIDLIST_MAX];

// see also TwlIPL_RED/build/systemmenu_RED/Launcher/ARM9/src/bannerCounter.h
inline void AMN_BNC_resetCount( AMNBannerCounter *c )
{
    c->count = 0;
    c->control = 0;
}
    
inline void AMN_BNC_setBanner( AMNBannerCounter *c, const TWLBannerFile *b)
{
    c->banner = b;
}
    
inline void AMN_BNC_initCounter( AMNBannerCounter *c, const TWLBannerFile *b)
{
    AMN_BNC_setBanner( c, b );
    AMN_BNC_resetCount( c );
}
    
inline const TWLBannerFile* AMN_BNC_getBanner( AMNBannerCounter *c )
{
    return c->banner;
}

static void AMN_BNC_incrementCount( AMNBannerCounter *c );
static AMNFrameAnimeData AMN_BNC_getFAD( AMNBannerCounter *c );
static AMNFrameAnimeData AMN_BNC_getFADAndIncCount( AMNBannerCounter *c );

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL AMN_isNandTitleListReady() { return sNandTitleListReady; }
s32 AMN_getNandTitleListLengthForLauncher() { return sNandTitleListLengthForLauncher; }
s32 AMN_getCardTitleListLength() { return sCardTitleListLength; }
s32 AMN_getRomHeaderListLength() { return sNandAppRomHeaderArrayLength + cNandTitleIndexStart;}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*!
  コンストラクタ（C++時代の遺物）
  AMN_Init()から呼ばれるだけ
 */
static void AMN_Manager( )
{
    sThreadStarted = FALSE;
    spNandBannerFileArray = NULL;
    spEmptyBannerFileBuffer = NULL;
    spSubBannerFileBuffer = NULL;

    // CARDアプリは1個まで。
    SDK_ASSERT( cCardTitleArrayMax == 1 );
    // NANDアプリは39個まで表示できる。
    SDK_ASSERT( cNandTitleArrayMax == 39 );

#ifndef TWL_IPL_FINAL

    // 言語enumの監視。
    SDK_ASSERT( BANNER_PRIO_JAPANESE == OS_LANGUAGE_JAPANESE );
    SDK_ASSERT( BANNER_PRIO_ENGLISH  == OS_LANGUAGE_ENGLISH  );
    SDK_ASSERT( BANNER_PRIO_FRENCH   == OS_LANGUAGE_FRENCH   );
    SDK_ASSERT( BANNER_PRIO_GERMAN   == OS_LANGUAGE_GERMAN   );
    SDK_ASSERT( BANNER_PRIO_ITALIAN  == OS_LANGUAGE_ITALIAN  );
    SDK_ASSERT( BANNER_PRIO_SPANISH  == OS_LANGUAGE_SPANISH  );
    SDK_ASSERT( BANNER_PRIO_CHINESE  == OS_LANGUAGE_CHINESE  );
    SDK_ASSERT( BANNER_PRIO_KOREAN   == OS_LANGUAGE_HANGUL   );

#endif

    AMN_initFlags_();
}

/*!
  起動。
 */
void AMN_init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
    // 元コンストラクタ
    AMN_Manager();
    // この時点でAMNi_AllocとAMNi_FreeがNULLであるべし
    SDK_ASSERT( AMNi_Alloc == NULL && AMNi_Free == NULL );

    AMNi_Alloc = pAlloc;
    AMNi_Free  = pFree;

    spNandBannerFileArray = (TWLBannerFile*)AMNi_Alloc( sizeof(TWLBannerFile) * cNandTitleArrayMax );
    SDK_ASSERT( spNandBannerFileArray );

    spEmptyBannerFileBuffer = (TWLBannerFile*)AMNi_Alloc( sizeof(TWLBannerFile) );
    SDK_ASSERT( spEmptyBannerFileBuffer );

    spSubBannerFileBuffer = (TWLSubBannerFile*)AMNi_Alloc( sizeof(TWLSubBannerFile) );
    SDK_ASSERT( spSubBannerFileBuffer );

    MI_CpuClearFast(spEmptyBannerFileBuffer, sizeof(TWLBannerFile));
    sEmptyAnimeData.image = spEmptyBannerFileBuffer->v1.image;
    sEmptyAnimeData.pltt  = spEmptyBannerFileBuffer->v1.pltt;
    sEmptyAnimeData.hflip = FALSE;
    sEmptyAnimeData.vflip = FALSE;
    
    MI_CpuClearFast(sAllTitlePropertyArray, sizeof(sAllTitlePropertyArray));

#ifndef TWL_IPL_FINAL
    // test

#if 0
    if (0) {
        BOOL ret;

        ret = NAM_CheckTitleLaunchRights(
            0x00030004534d504c // "SMPL" - nandAppSample
//            0x00030015344e4141 // "4NAA" - UIG launcher default
            );
        IPL_PRINT("NAM_CheckTitleLaunchRights()=%d\n", ret);

        SDK_ASSERT( FALSE );
    }

    
    if (0) {
        FSFile file[1];
        BOOL ret;

        FS_InitFile(file);
        ret = FS_OpenFileEx(file, "sdmc:/dummy.bkp", FS_FILEMODE_R);
        IPL_PRINT("FS_OpenFileEx()=%d\n", ret);
        FS_CloseFile(file);

        SDK_ASSERT( FALSE );
        
    }

    
    // test
    if (0) {
        s32 ret = NAM_ImportBkpSave("sdmc:/dummy.bkp");
        IPL_PRINT("NAM_ImportBkpSave()=%d\n", ret);
        SDK_ASSERT( FALSE );
    }

    if (0) {
        // なんか、[ES] generate cert failed -2012
        // になる。
        s32 ret = NAM_ExportBkp(
//            0x0003001534303241, // "402A" - UIG launcher clone
//            0x00030015344e4141, // "4NAA" - UIG launcher default
            0x00030004534d504c, // "SMPL" - nandAppSample
                                "sdmc:/test.bkp");
        IPL_PRINT("NAM_ExportBkp()=%d\n", ret);
    }

    if (0) {
        NAMBkpInfo info;
        static TWLBannerFile banner;
        info.fileSize = 0;
        info.hardwareId = 0;

        s32 ret = NAM_ReadBkpInfo(&info, &banner, sizeof(banner),
                                  "sdmc:/test.bkp");
        IPL_PRINT("NAM_ReadBkpInfo()=%d, fileSize=%d, HWID=%016llx\n",
                  ret, info.fileSize, (u64)info.hardwareId);

        SDK_ASSERT( FALSE );
    }
    
#endif

#endif

    //AMN_restart(); // 自動ロードしてしまうのは、活線挿抜ライブラリでCARDだけ読みたい場合やNAND読み込みタイミングを遅らせたい場合に不都合
}

/*!
  再起動。
 */
void AMN_restart()
{
    // AMNi_AllocおよびAMNi_Freeはセットされているべし
    SDK_ASSERT( AMNi_Alloc != NULL && AMNi_Free != NULL );

    AMN_initFlags_();

    SDK_ASSERT( !sThreadStarted || OS_IsThreadTerminated(&sReadThread) );

    sThreadStarted = TRUE;

    OS_InitEvent(&sSubBannerFileBufferEvent);
    AMN_unlockSubBannerFileBuffer();
    OS_InitMessageQueue(&sCheckMsgQueue, sCheckMsgBuf, sizeof(sCheckMsgBuf) / sizeof(*sCheckMsgBuf));
    OS_CreateThread(&sReadThread, AMN_procRead, NULL,
                    sReadThreadStack + (sizeof(sReadThreadStack) / sizeof(*sReadThreadStack)),
                    sizeof(sReadThreadStack),
                    cThreadPriority_Read);
    OS_CreateThread(&sCheckThread, AMN_procCheck, NULL,
                    sCheckThreadStack + (sizeof(sCheckThreadStack) / sizeof(*sCheckThreadStack)),
                    sizeof(sCheckThreadStack),
                    cThreadPriority_Check);
    // for safety, wakeup sCheckThread first
    OS_WakeupThreadDirect(&sCheckThread);
    OS_WakeupThreadDirect(&sReadThread);
}

/*!
  NANDタイトルだけ取得しながらの再起動。
 */
void AMN_restartWithReadNandTitle()
{
    // AMNi_AllocおよびAMNi_Freeはセットされているべし
    SDK_ASSERT( AMNi_Alloc != NULL && AMNi_Free != NULL );

    AMN_initFlags_();

    SDK_ASSERT( !sThreadStarted || OS_IsThreadTerminated(&sReadThread) );

    sThreadStarted = TRUE;

    OS_InitEvent(&sSubBannerFileBufferEvent);
    AMN_unlockSubBannerFileBuffer();
    OS_InitMessageQueue(&sCheckMsgQueue, sCheckMsgBuf, sizeof(sCheckMsgBuf) / sizeof(*sCheckMsgBuf));
    OS_CreateThread(&sReadThread, AMN_procReadNandTitleOnly, NULL,
                    sReadThreadStack + (sizeof(sReadThreadStack) / sizeof(*sReadThreadStack)),
                    sizeof(sReadThreadStack),
                    cThreadPriority_Read);
    OS_CreateThread(&sCheckThread, AMN_procCheck, NULL,
                    sCheckThreadStack + (sizeof(sCheckThreadStack) / sizeof(*sCheckThreadStack)),
                    sizeof(sCheckThreadStack),
                    cThreadPriority_Check);
    // for safety, wakeup sCheckThread first
    OS_WakeupThreadDirect(&sCheckThread);
    OS_WakeupThreadDirect(&sReadThread);
}

/*!
  NANDタイトルのROM_HEADER_SHORTだけ取得しながらの再起動。
 */
void AMN_restartWithReadNandTitleHeaderShort()
{
    // AMNi_AllocおよびAMNi_Freeはセットされているべし
    SDK_ASSERT( AMNi_Alloc != NULL && AMNi_Free != NULL );

    AMN_initFlags_();

    SDK_ASSERT( !sThreadStarted || OS_IsThreadTerminated(&sReadThread) );

    sThreadStarted = TRUE;

    OS_CreateThread(&sReadThread, AMN_procReadNandTitleHeaderOnly, NULL,
                    sReadThreadStack + (sizeof(sReadThreadStack) / sizeof(*sReadThreadStack)),
                    sizeof(sReadThreadStack),
                    cThreadPriority_Read);
    OS_WakeupThreadDirect(&sReadThread);
}


/*!
  終了。
 */
void AMN_destroy()
{
    sReadCancelFlag = TRUE;
    while ( sThreadStarted && !OS_IsThreadTerminated(&sReadThread) ) {
        OS_Sleep(1);
    }

    if( AMNi_Free != NULL && spSubBannerFileBuffer)
    {
        AMNi_Free( spSubBannerFileBuffer );
    }
    if( AMNi_Free != NULL && spEmptyBannerFileBuffer)
    {
        AMNi_Free( spEmptyBannerFileBuffer );
    }
    if( AMNi_Free != NULL && spNandBannerFileArray)
    {
        AMNi_Free( spNandBannerFileArray );
    }
}

static void AMN_initFlags_()
{
    sReadCancelFlag = FALSE;
    sNandTitleListReady = FALSE;
    sNandAllTitleListLength = 0;
    sNandTitleListLengthForLauncher = 0;
    sCardTitleListLength = 0;
    sNandAppRomHeaderArrayLength = 0;
}

static void AMN_initCardTitleList_()
{
}

static void AMNi_getAndAddNandTitleData( NAMTitleId titleID, BOOL readShowData )
{
    TitleProperty* pTitleProp;
    // header情報の取得

#if (MEASURE_BANNER_LOAD_TIME == 1)
    OSTick start;
#endif

    FSFile  file[1];
    BOOL bSuccess;
    char path[FS_ENTRY_LONGNAME_MAX];
    s32 readLen;
    u32 offset;
    
    s32 rhArrayLen = cNandTitleIndexStart + sNandAppRomHeaderArrayLength;
    
    // もう入らない
    if (rhArrayLen >= OS_TITLEIDLIST_MAX) {
        return;
    }
    
    FS_InitFile(file);
    
#if (MEASURE_BANNER_LOAD_TIME == 1)
    start = OS_GetTick();
#endif

    if( (titleID & TITLE_ID_DATA_ONLY_FLAG_MASK) == 0 )
    {
        // DataOnlyでない場合
        readLen = NAM_GetTitleBootContentPathFast( path, titleID );
        
#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "NAM_GetTitleBootContentPath : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif
        
        // ファイルパスを取得
        if(readLen != NAM_OK){
            // error
            SDK_ASSERT( FALSE );
            return;
        }
        
#if (MEASURE_BANNER_LOAD_TIME == 1)
        start = OS_GetTick();
#endif

        // ファイルオープン
        bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
        if( ! bSuccess )
        {
            // error
            SDK_ASSERT( FALSE );
            return;
        }
        
#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "OpenFileEX : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
        start = OS_GetTick();
#endif
        
        readLen = FS_ReadFile(file, &s_AllRomHeaderArray[rhArrayLen], sizeof(ROM_Header_Short));
        if( readLen != sizeof(ROM_Header_Short) )
        {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            return;
        }
        
        offset = s_AllRomHeaderArray[rhArrayLen].banner_offset;

#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "FS_ReadFile header : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif
    }
    else
    {
        // DataOnlyの場合
        int l;
        NAMTitleInfo naminfo;
        // この関数で得られる情報は無検証なので改ざんの可能性があるが、メーカーコードのみの判定なので、速度を優先する。(2008.06.20吉岡)
        // （Fastつけないと一回300msぐらいかかる）
        NAM_ReadTitleInfoFast( &naminfo, titleID );
        MI_CpuClear8(&s_AllRomHeaderArray[rhArrayLen], sizeof(ROM_Header_Short) );
        OS_TPrintf( "0x%0.16llx dataonly-title : makercode = ", titleID );
        for(l=0;l<MAKER_CODE_MAX;l++)
        {
            s_AllRomHeaderArray[rhArrayLen].maker_code[l] = ((char *)&naminfo.companyCode)[l];
            OS_TPrintf( "0x%0.2x ", s_AllRomHeaderArray[rhArrayLen].maker_code[l] );
        }
        OS_TPrintf( "\n" );
        s_AllRomHeaderArray[rhArrayLen].titleID = titleID;
        
        sNandAppRomHeaderArrayLength++;
        return;
    }

    if(s_AllRomHeaderArray[rhArrayLen].platform_code & PLATFORM_CODE_FLAG_TWL){
        // ROMヘッダのリージョンチェックとCRCチェックを行い、不正なアプリは無視する。
        if( !UTL_CheckAppRegion( s_AllRomHeaderArray[rhArrayLen].card_region_bitmap ) ) {
            OS_TPrintf( "Region Check NG : %llx\n", titleID );
            FS_CloseFile(file);
            return;
        }
    }
    if( !UTL_CheckAppCRC16( &s_AllRomHeaderArray[rhArrayLen] ) ) {
        OS_TPrintf( "CRC16  Check NG : %llx\n", titleID );
        FS_CloseFile(file);
        return;
    }

    sNandAppRomHeaderArrayLength++;

    // もうランチャー表示用情報は数がオーバーしてるか、表示用情報を読まない設定
    if ( ( sNandTitleListLengthForLauncher >= cNandTitleArrayMax ) || !readShowData ) {
        FS_CloseFile(file);
        return;
    }
    
    // 以下は表示するアプリのみ（現在リストにあるデータ数がNandタイトル数の最大を超えていない場合）の処理
    // タイトルリストへの追加
    pTitleProp = &sAllTitlePropertyArray[cNandTitleIndexStart + sNandTitleListLengthForLauncher];
    pTitleProp->titleID = titleID;
    pTitleProp->pBanner = &spNandBannerFileArray[sNandTitleListLengthForLauncher];
    pTitleProp->flags.isValid = TRUE;
    pTitleProp->flags.bootType = LAUNCHER_BOOTTYPE_NAND;

    // バナーの取得
    // バナーが存在する場合のみリード
    if( offset ) {
        
#if (MEASURE_BANNER_LOAD_TIME == 1)
        start = OS_GetTick();
#endif

        bSuccess = FS_SeekFile(file, (s32)offset, FS_SEEK_SET);
        if( ! bSuccess )
        {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            return;
        }

#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "FS_SeekFile banner: %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
        start = OS_GetTick();
#endif
        
        readLen = FS_ReadFile( file, pTitleProp->pBanner, (s32)sizeof(TWLBannerFile) );
        if( readLen != (s32)sizeof(TWLBannerFile) )
        {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            return;
        }

#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "FS_ReadFile banner: %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
        start = OS_GetTick();
#endif

        // バナーチェックリクエスト送信。
        OS_SendMessage(&sCheckMsgQueue, (OSMessage)(pTitleProp->pBanner), OS_MESSAGE_BLOCK);

#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "check banner: %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif

    }else {
        // バナーが存在しない場合はバッファクリア
        MI_CpuClearFast( pTitleProp->pBanner, sizeof(TWLBannerFile) );
    }
    
#if (MEASURE_BANNER_LOAD_TIME == 1)
    start = OS_GetTick();
#endif

    FS_CloseFile(file);
    
#if (MEASURE_BANNER_LOAD_TIME == 1)
    OS_TPrintf( "close file : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
    start = OS_GetTick();
#endif
    
    // サブバナーファイルを読み込んでみる
    if(s_AllRomHeaderArray[rhArrayLen].exFlags.availableSubBannerFile &&
       NAM_OK == NAM_GetTitleBannerFilePath( path, titleID ))
    {

#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "NAM_GetTitleBannerFilePath : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
        start = OS_GetTick();
#endif
        if (FS_OpenFileEx(file, path, FS_FILEMODE_R)) {
            // サブバナーバッファ確保。
            AMN_lockSubBannerFileBuffer();
            readLen = FS_ReadFile(file, spSubBannerFileBuffer, sizeof(*spSubBannerFileBuffer));
            FS_CloseFile(file);
            if (readLen == sizeof(TWLSubBannerFile)) {
                // 読み込みには成功したので正当性チェック
                // サブバナーチェックリクエスト送信。
                OS_SendMessage(&sCheckMsgQueue, (OSMessage)(cNandTitleIndexStart + sNandTitleListLengthForLauncher), OS_MESSAGE_BLOCK);
                // 解放はCHECKスレッド側で行う。
            } else {
                OS_TPrintf("subbanner read failed (%d)\n", readLen);
                // 解放。
                AMN_unlockSubBannerFileBuffer();
            }
        }

#if (MEASURE_BANNER_LOAD_TIME == 1)
        OS_TPrintf( "open-read-close-check subbanner : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif
    }
    
    sNandTitleListLengthForLauncher++;
}

/*!
  see also SYSM_InitNandTitleList() and SYSM_GetNandTitleList()
  インポートされているすべてのNANDアプリを列挙したリストの準備
 */
static void AMN_initNandTitleList_()
{
    s32 ret;
    s32 l;
    NAMTitleId* pNandAllTitleIDList = NULL;
    u8 count_valid_app_for_launcher = 0;

    // インポートされているタイトルの取得
    sNandAllTitleListLength = NAM_GetNumTitles();
    OS_TPrintf("NAM_GetNumTitles()=%d\n", sNandAllTitleListLength);
    // 少なくともランチャーと無線ファームがリストアップされるはず。
    // だが、一応0は通す。
    SDK_ASSERT( sNandAllTitleListLength >= 0 );

    if (sNandAllTitleListLength > 0) {
        pNandAllTitleIDList = (NAMTitleId*)AMNi_Alloc( sizeof(NAMTitleId) * sNandAllTitleListLength );
        SDK_ASSERT( pNandAllTitleIDList );

        if (!pNandAllTitleIDList) {
            // error
            sNandAllTitleListLength = 0;
        } else {
            ret = NAM_GetTitleList(pNandAllTitleIDList, (u32)sNandAllTitleListLength);
            OS_TPrintf("NAM_GetTitleList()=%d\n", ret);
            SDK_ASSERT( ret == NAM_OK );

            if (ret != NAM_OK) {
                // error
                sNandAllTitleListLength = 0;
            }
        }
    }

    // メモリの確保と解放を同じ関数内で行いたいので、
    // 取得したタイトルがローンチ対象かどうかをチェック
    // を、ここで行う。
    // すべてのタイトルのヘッダを読み込むので、絞込みの必要はない

    for (l = 0; l < sNandAllTitleListLength; l++) {
        if (AMN_isTitleIdValidForLauncher(pNandAllTitleIDList[l])) {
            AMNi_getAndAddNandTitleData( pNandAllTitleIDList[l], TRUE );
    // 本体設定の場合、アプリマネージャ用indexは飛び飛びになったり、ForSetting()が返す値(個数)より大きくなるので
    // getNandTitleListLengthForSetting()は用意しない。
    
            // FreeboxCount更新のためのカウンタ
            count_valid_app_for_launcher++;
        }
    }

    // FreeboxCount更新
    AMNi_updateFreeBoxCount( count_valid_app_for_launcher );
    

    // ローンチ対象でないタイトルの情報の取得
    for (l = 0; l < sNandAllTitleListLength; l++) {
        if (!AMN_isTitleIdValidForLauncher(pNandAllTitleIDList[l])) {
            AMNi_getAndAddNandTitleData( pNandAllTitleIDList[l], FALSE );
        }
    }

    if (pNandAllTitleIDList) {
        AMNi_Free( pNandAllTitleIDList );
    }
}

// インポートされているすべてのタイトルのROM_HEADER_SHORTのみ読み出す
static void AMN_initNandTitleRomHeaderShortList_()
{
    s32 ret;
    s32 l;
    NAMTitleId* pNandAllTitleIDList = NULL;
    u8 count_valid_app_for_launcher = 0;

    // インポートされているタイトルの取得
    sNandAllTitleListLength = NAM_GetNumTitles();
    OS_TPrintf("NAM_GetNumTitles()=%d\n", sNandAllTitleListLength);
    // 少なくともランチャーと無線ファームがリストアップされるはず。
    // だが、一応0は通す。
    SDK_ASSERT( sNandAllTitleListLength >= 0 );

    if (sNandAllTitleListLength > 0) {
        pNandAllTitleIDList = (NAMTitleId*)AMNi_Alloc( sizeof(NAMTitleId) * sNandAllTitleListLength );
        SDK_ASSERT( pNandAllTitleIDList );

        if (!pNandAllTitleIDList) {
            // error
            sNandAllTitleListLength = 0;
        } else {
            ret = NAM_GetTitleList(pNandAllTitleIDList, (u32)sNandAllTitleListLength);
            OS_TPrintf("NAM_GetTitleList()=%d\n", ret);
            SDK_ASSERT( ret == NAM_OK );

            if (ret != NAM_OK) {
                // error
                sNandAllTitleListLength = 0;
            }
        }
    }

    // タイトルの情報(ヘッダのみ)の取得
    for (l = 0; l < sNandAllTitleListLength; l++) {
        AMNi_getAndAddNandTitleData( pNandAllTitleIDList[l], FALSE );
        if (AMN_isTitleIdValidForLauncher(pNandAllTitleIDList[l])) {
            // FreeboxCount更新のためのカウンタ
            count_valid_app_for_launcher++;
        }
    }
    
    // FreeboxCount更新
    AMNi_updateFreeBoxCount( count_valid_app_for_launcher );

    if (pNandAllTitleIDList) {
        AMNi_Free( pNandAllTitleIDList );
    }
}

// 生データ（TitlePropertyのリスト、HeaderShortリスト）にアクセスできる抜け道関数
TitleProperty* AMN_getTitlePropertyList( void )
{
    return sAllTitlePropertyArray;
}

ROM_Header_Short* AMN_getRomHeaderList( void )
{
    return s_AllRomHeaderArray;
}

static void AMN_procRead()
{
    MI_CpuClearFast(sAllTitlePropertyArray, sizeof(sAllTitlePropertyArray));

    AMN_initCardTitleList_();
    AMN_initNandTitleList_();

    OS_SendMessage(&sCheckMsgQueue, NULL, OS_MESSAGE_BLOCK);
    OS_JoinThread(&sCheckThread);

    AMN_stepBannerAnimeAll( TRUE );

    // for GX DMA
    DC_StoreAll();

    if (!sReadCancelFlag) {
        sNandTitleListReady = TRUE;
    }
}

// Nandタイトルだけ取得、Card部分はno care
static void AMN_procReadNandTitleOnly()
{
    MI_CpuClearFast(&sAllTitlePropertyArray[cNandTitleIndexStart], sizeof(TitleProperty) * ( cAllTitleArrayMax - cNandTitleIndexStart ) );

    AMN_initNandTitleList_();

    OS_SendMessage(&sCheckMsgQueue, NULL, OS_MESSAGE_BLOCK);
    OS_JoinThread(&sCheckThread);

    AMN_stepBannerAnimeAll( TRUE );

    // for GX DMA
    DC_StoreAll();

    if (!sReadCancelFlag) {
        sNandTitleListReady = TRUE;
    }
}

// NandタイトルのROM_Header_Shortだけ取得、Card部分はno care
static void AMN_procReadNandTitleHeaderOnly()
{
    AMN_initNandTitleRomHeaderShortList_();

    // for GX DMA
    DC_StoreAll();

    if (!sReadCancelFlag) {
        sNandTitleListReady = TRUE;
    }
}


static void AMN_procCheck()
{
    OSMessage msg;
    s32 index;
    TWLBannerFile* pBanner;

    while (TRUE) {
        OS_ReceiveMessage(&sCheckMsgQueue, &msg, OS_MESSAGE_BLOCK);
//        IPL_PRINT("procCheck: msg=0x%x\n", msg);
        if (!msg) {
            break;
        }
        index = (s32)msg;
        // 0はスレッド終了を意味するのでindexとしては不適切。
        // また、CARDなのでサブバナーはありえない。
        SDK_ASSERT( cNandTitleIndexStart > 0 );
        if (cNandTitleIndexStart <= index && index < (cNandTitleIndexStart + cNandTitleArrayMax)) {
            // サブバナー
            (void)AMN_checkAndReplaceBannerAnime(index);
            AMN_unlockSubBannerFileBuffer();
        } else {
            pBanner = (TWLBannerFile*)msg;
            if (!AMN_checkBannerFile(pBanner)) {
                // 正当性チェック失敗の場合はバッファクリア
                MI_CpuClearFast(pBanner, sizeof(*pBanner));
                OS_TPrintf("check NG! msg=0x%x propAry=0x%x\n", pBanner, sAllTitlePropertyArray);
            }
        }
    }
}

static void AMN_lockSubBannerFileBuffer()
{
    OS_WaitEventEx_And(&sSubBannerFileBufferEvent,
                       cEventMask_SubBannerFileBuffer,
                       cEventMask_SubBannerFileBuffer);
}

static void AMN_unlockSubBannerFileBuffer()
{
    OS_SignalEvent(&sSubBannerFileBufferEvent,
                   cEventMask_SubBannerFileBuffer);
}

const TitleProperty* AMN_getTitleProperty(s32 index)
{
    const TitleProperty* pTitleProp;

    if (cAllTitleIndexStart <= index && index < cAllTitleArrayMax) {
        pTitleProp = &sAllTitlePropertyArray[index];
        if (pTitleProp->flags.isValid) {
            return pTitleProp;
        }
    }

    return NULL;
}

const ROM_Header_Short*  AMN_getTitleRomHeaderShort(s32 index)
{
    const ROM_Header_Short* pRhs;

    if (cAllTitleIndexStart <= index && index < cAllTitleArrayMax) {
        pRhs = &s_AllRomHeaderArray[index];
        return pRhs;
    }

    return NULL;
}

BOOL AMN_isDSWirelessIcon(s32 index)
{
    const ROM_Header_Short* pRhs = AMN_getTitleRomHeaderShort(index);

    if (pRhs) {
        return pRhs->exFlags.DSWirelessIcon;
    }

    return FALSE;
}

BOOL AMN_isWiFiConnectionIcon(s32 index)
{
    const ROM_Header_Short* pRhs = AMN_getTitleRomHeaderShort(index);

    if (pRhs) {
        return pRhs->exFlags.WiFiConnectionIcon;
    }

    return FALSE;
}

BOOL AMN_isAgreeEULAFlag(s32 index)
{
    const ROM_Header_Short* pRhs = AMN_getTitleRomHeaderShort(index);

    if (pRhs) {
        return pRhs->exFlags.agree_EULA;
    }

    return FALSE;
}

const u8* AMN_getParentalControlRatingInfo(s32 index)
{
    const ROM_Header_Short* pRhs = AMN_getTitleRomHeaderShort(index);

    if (pRhs) {
        return pRhs->parental_control_rating_info;
    }

    return FALSE;
}

// タイトルIDが本体設定用としては無効であっても、
// 対応するindexを返します。
s32 AMN_getIndexByTitleId(NAMTitleId titleId)
{
    const TitleProperty* pTitleProp;
    s32 index;

    // 一応、TitleID == 0 のアプリがあるかもしれない？ので、
    // NANDから優先的に探す。
    // CARDはTITLE_ID_MEDIA_MASKが1になるはず。

    // NAND
    index = cNandTitleIndexStart + sNandTitleListLengthForLauncher - 1;
    while (index >= cNandTitleIndexStart) {
        pTitleProp = AMN_getTitleProperty(index);
        if (pTitleProp &&
            pTitleProp->titleID == titleId) {
            return index;
        }
        index--;
    }

    // CARD
    index = sCardTitleListLength - 1;
    while (index >= cCardTitleIndexStart) {
        pTitleProp = AMN_getTitleProperty(index);
        if (pTitleProp &&
            pTitleProp->titleID == titleId) {
            return index;
        }
        index--;
    }

    return -1;
}

// indexが本体設定用としては無効であっても、
// 対応するタイトルIDを返します。
NAMTitleId  AMN_getTitleIdByIndex(s32 index)
{
    const TitleProperty* pTitleProp = AMN_getTitleProperty(index);

    if (pTitleProp) {
        return pTitleProp->titleID;
    }

    return (NAMTitleId)0;
}

void AMN_stepBannerAnimeAll(BOOL restart)
{
    s32 index;

    for (index = cAllTitleIndexStart; index < cAllTitleArrayMax; index++) {
        AMN_stepBannerAnime(index, restart);
    }
}

void AMN_stepBannerAnime(s32 index, BOOL restart)
{
    const TWLBannerFile* pBanner;
    AMNBannerCounter* pBNC;

    if (cAllTitleIndexStart <= index && index < cAllTitleArrayMax) {
        pBNC = &sBannerCounterArray[index];
        if (restart) {
            pBanner = sAllTitlePropertyArray[index].pBanner;
            if (!pBanner) {
                pBanner = spEmptyBannerFileBuffer;
            }
            AMN_BNC_initCounter(pBNC, pBanner);
        } else {
            AMN_BNC_incrementCount(pBNC);
        }
        sFrameAnimeDataArray[index] = AMN_BNC_getFAD(pBNC);
    }
}

const u8* AMN_getBannerImage(s32 index)
{
    AMNFrameAnimeData ret = AMN_getBannerAnime(index);

    return ret.image;
}

const u8* AMN_getBannerPltt(s32 index)
{
    AMNFrameAnimeData ret = AMN_getBannerAnime(index);

    return ret.pltt;
}

BOOL AMN_getBannerHFlip(s32 index)
{
    AMNFrameAnimeData ret = AMN_getBannerAnime(index);

    return ret.hflip;
}

BOOL AMN_getBannerVFlip(s32 index)
{
    AMNFrameAnimeData ret = AMN_getBannerAnime(index);

    return ret.vflip;
}

AMNFrameAnimeData AMN_getBannerAnime(s32 index)
{
    if (cAllTitleIndexStart <= index && index < cAllTitleArrayMax) {
        return sFrameAnimeDataArray[index];
    }

    return sEmptyAnimeData;
}

const u16* AMN_getBannerText2(s32 index, OSLanguage language)
{
    const TitleProperty* pTitleProp = AMN_getTitleProperty(index);

    if (language >= OS_LANGUAGE_CODE_MAX) {
        language = OS_LANGUAGE_ENGLISH;
    }

    if (pTitleProp) {
        if (language < BANNER_LANG_NUM_V1) {
            return pTitleProp->pBanner->v1.gameName[language];
        }
        language = (OSLanguage)(language - BANNER_LANG_NUM_V1);
        if (language < BANNER_LANG_NUM_V2) {
            return pTitleProp->pBanner->v2.gameName[language];
        }
        language = (OSLanguage)(language - BANNER_LANG_NUM_V2);
        return pTitleProp->pBanner->v3.gameName[language];
    }

    return spEmptyBannerFileBuffer->v1.gameName[language];
}

const u16* AMN_getBannerText(s32 index)
{
    OSOwnerInfo info;

    // とりあえず。
    // 正式には本体設定のLCFGラッパーから取らないとダメ？
    OS_GetOwnerInfo(&info);
    return AMN_getBannerText2(index, (OSLanguage)info.language);
}

// see also SYSMi_CheckBannerFile()
// バナーデータの正誤チェック
BOOL AMN_checkBannerFile(TWLBannerFile* pBanner)
{
typedef struct BannerCheckParam {
    u8      *pSrc;
    u32     size;
}BannerCheckParam;

    int i;
    BOOL retval = TRUE;
    u16 calc_crc = 0xffff;
    u16 *pHeaderCRC = (u16 *)&pBanner->h.crc16_v1;
    BannerCheckParam bannerCheckList[ BANNER_VER_NTR_MAX ];
    BannerCheckParam *pChk = &bannerCheckList[ 0 ];
    
    // NTR互換部分は標準でチェック
    bannerCheckList[ 0 ].pSrc = (u8 *)&( pBanner->v1 );
    bannerCheckList[ 0 ].size = sizeof( BannerFileV1 );
    bannerCheckList[ 1 ].pSrc = (u8 *)&( pBanner->v2 );
    bannerCheckList[ 1 ].size = sizeof( BannerFileV2 );
    bannerCheckList[ 2 ].pSrc = (u8 *)&( pBanner->v3 );
    bannerCheckList[ 2 ].size = sizeof( BannerFileV3 );
    
    for( i = 0; i < BANNER_VER_NTR_MAX; i++ ) {
        if( i < pBanner->h.version ) {
            calc_crc = SVC_GetCRC16( calc_crc, pChk->pSrc, pChk->size );
            if( calc_crc != *pHeaderCRC++ ) {
                retval = FALSE;
                break;
            }
        }else {
            MI_CpuClear16( pChk->pSrc, pChk->size );
        }
        pChk++;
    }
    
    // TWLバナーなら、バナーアニメ部もチェック
    if( pBanner->h.platform == BANNER_PLATFORM_TWL ) {
            if (pBanner->h.crc16_anime != AMN_getBannerAnimeCRC(&pBanner->anime)) {
                retval = FALSE;
            }
    }
    return retval;
}

// アニメ部CRC算出。
static u32 AMN_getBannerAnimeCRC(const BannerAnime* pAnime)
{
    return SVC_GetCRC16(0xffff, pAnime, sizeof(*pAnime));
}

static BOOL AMN_checkAndReplaceBannerAnime(s32 index)
{
    BOOL retval = FALSE;
    TWLBannerFile* pBanner;

    // 範囲外のindexを無視。
    if (cNandTitleIndexStart <= index && index < (cNandTitleIndexStart + sNandTitleListLengthForLauncher)) {
        if (spSubBannerFileBuffer->h.crc16_anime == AMN_getBannerAnimeCRC(&spSubBannerFileBuffer->anime)) {
            // 成功したのでコピーする
            pBanner = &spNandBannerFileArray[index-cNandTitleIndexStart];
            // h.crc16_animeしかチェックしていないので、それとanimeだけコピー
            pBanner->h.crc16_anime = spSubBannerFileBuffer->h.crc16_anime;
            // MI_CpuCopy()の方が遅くなった…
            pBanner->anime = spSubBannerFileBuffer->anime;
            retval = TRUE;
        }
    }

//    IPL_PRINT("checkAndReplaceBannerAnime(%d)=%d\n", index, retval);

    return retval;
}

// see also TwlIPL_RED/build/systemmenu_RED/Launcher/ARM9/src/bannerCounter.c
void AMN_BNC_incrementCount( AMNBannerCounter *c )
{
    // TWLのみカウントインクリメント
    if( c->banner->h.platform == BANNER_PLATFORM_TWL )
    {
        if( c->banner->anime.control[0].frameCount == 0 )
        {
            // アニメに終端しか存在しない
            //OS_TPrintf( "BNC_incrementCount:Only a Terminator!\n" );
            return;
        }
        
        c->count++;
        if( c->count >= c->banner->anime.control[c->control].frameCount )
        {
            // カウント値がコントロールのフレームカウントを超えたので次のコントロールへ
            c->control++;
            c->count = 0;

            //ループ及び停止の処理
            if( c->control >= BANNER_ANIME_CONTROL_INFO_NUM )
            {
                // コントロールが限界を超えたら無条件でループ
                AMN_BNC_resetCount( c );
            }
            else if( c->banner->anime.control[c->control].frameCount == 0 )
            {
                // コントロールのフレームカウントが0なら終端到達
                if( c->banner->anime.control[c->control].animeType == 0 )
                {
                    // アニメタイプ0ならループ
                    AMN_BNC_resetCount( c );
                }
                else if( c->banner->anime.control[c->control].animeType == 1 )
                {
                    // アニメタイプ1なら停止（一つ前のコントロールに戻す）
                    c->control--;
                }
            }
        }
    }
}

AMNFrameAnimeData AMN_BNC_getFAD( AMNBannerCounter *c )
{
    AMNFrameAnimeData ret;
    if( c->banner->h.platform == BANNER_PLATFORM_NTR )
    {
        ret.image = c->banner->v1.image;
        ret.pltt = c->banner->v1.pltt;
        ret.hflip = FALSE;
        ret.vflip = FALSE;
    }
    else
    {
        if( c->banner->anime.control[0].frameCount == 0 )
        {
            // アニメに終端しか存在しない
            //OS_TPrintf( "BNC_getFAD:Only a Terminator!\n" );
            ret.image = c->banner->v1.image;
            ret.pltt = c->banner->v1.pltt;
            ret.hflip = FALSE;
            ret.vflip = FALSE;
            return ret;
        }
        // コントロールデータを読んで、現在のフレームに該当するデータを返す
        ret.image = c->banner->anime.image[ c->banner->anime.control[c->control].normal.cellNo ];
        ret.pltt = c->banner->anime.pltt[ c->banner->anime.control[c->control].normal.plttNo ];
        ret.hflip = c->banner->anime.control[c->control].normal.flipType & 0x1;
        ret.vflip = (c->banner->anime.control[c->control].normal.flipType & 0x2) >> 1;
    }
    return ret;
}

AMNFrameAnimeData AMN_BNC_getFADAndIncCount( AMNBannerCounter *c )
{
    AMNFrameAnimeData ret = AMN_BNC_getFAD( c );
    AMN_BNC_incrementCount( c );
    return ret;
}

BOOL AMN_isTitleIdValidForLauncher(NAMTitleId id)
{
    // "Not Launch"でない　かつ　"Data Only"でない　なら有効なタイトルとしてリストに追加
    return ((id & ( TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK )) == 0);
}

BOOL AMN_isIndexValidForSetting(s32 index)
{
    const TitleProperty* pTitleProp = AMN_getTitleProperty(index);

    if (pTitleProp) {
        return NAM_IsUserTitle(pTitleProp->titleID);
    }

    return FALSE;
}

static void AMNi_updateFreeBoxCount( u8 count )
{
    u32 installed;
    u8 free;
    BOOL changed = FALSE;
    u8 *pBuffer;
    BOOL retval = TRUE;
    
    installed = LCFG_TSD_GetInstalledSoftBoxCount();
    free = LCFG_TSD_GetFreeSoftBoxCount();
    
    if( count > LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX )
    {
        count = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX;
    }
    
    OS_TPrintf( "AMNi_updateFreeBoxCount : Install:%d, Free:%d, LCFG_Install:%d, LCFG_Free:%d, MAX:%d\n",
                 count, LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - count, installed, free, LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX );
    
    if( (u8)installed != count )
    {
        LCFG_TSD_SetInstalledSoftBoxCount( ( u8 )count );
        changed = TRUE;
    }
    
    if( LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - count != free )
    {
        LCFG_TSD_SetFreeSoftBoxCount( (u8)( LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - count ) );
        changed = TRUE;
    }

    if(changed)
    {
        // LCFGライブラリの静的変数の値をNANDに反映
        pBuffer = AMNi_Alloc( LCFG_WRITE_TEMP );
        if (!pBuffer) {
            OS_TPrintf( "AMNi_updateFreeBoxCount : AMNi_Alloc failed.\n" );
            return;
        }
        retval = LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
        AMNi_Free( pBuffer );
        if (!retval) {
            OS_TPrintf( "AMNi_updateFreeBoxCount : LCFG_WriteTWLSettings failed.\n" );
            return;
        }
        OS_TPrintf( "AMNi_updateFreeBoxCount : FreeBoxCount is updated.\n" );
    }else
    {
        OS_TPrintf( "AMNi_updateFreeBoxCount : FreeBoxCount is not updated.\n" );
    }
}

/*--------------------------------------------------------------------------------------------------------

// アプリケーションに埋め込むテストコードのサンプル
// 実験用にヒープを拡張メインメモリから確保しているので注意
// 事前にOS_EnableMainExArena、OS_Init（OS_InitArena）、およびNAM_Initを呼んでおく必要あり
static OSHeapHandle os_heap_handle;
static void *alloc_tmp(u32 size)
{
    return OS_AllocFromHeap( OS_ARENA_MAINEX, os_heap_handle, size);
}
static void free_tmp( void *ptr )
{
    OS_FreeToHeap( OS_ARENA_MAINEX, os_heap_handle, ptr );
}
static void test2()
{

    s32 length;
    s32 idx;
    void *nstart;
    
    nstart = OS_InitAlloc( OS_ARENA_MAINEX, OS_GetMainExArenaLo(), OS_GetMainExArenaHi(), 1 );
    OS_SetMainExArenaLo( nstart );
    
    os_heap_handle = OS_CreateHeap( OS_ARENA_MAINEX, OS_GetMainExArenaLo(), OS_GetMainExArenaHi() );
    
    AMN_init( alloc_tmp, free_tmp );

    // 起動直後、自動的にバナーデータ読み込みを行います。
    // メインスレッドでは、とりあえず完了まで待ちます。
    while (!AMN_isNandTitleListReady()) {
        OS_Sleep(1);
    }

    // タイトル一覧をOS_Printfします。
    // cmn::util::U16toSjisは、STDライブラリの関数を呼び出してUnicodeからShiftJISへ変換します。
    length = AMN_getNandTitleListLengthForLauncher();
    for (idx = 0; idx < length; idx++) {
        s32 i;
        char dst[256];
        int dstlen = 256;
        i = cNandTitleIndexStart + idx;
        MI_CpuClearFast(dst,256);
        STD_ConvertStringUnicodeToSjis(dst, &dstlen, AMN_getBannerText(i), NULL, NULL);
        OS_TPrintf("title idx=%2d, titleId=%016llx exp=%02x forStg=%d bannerText=%s\n",
                  i, (u64)AMN_getTitleIdByIndex(i),
                  AMN_getTitleProperty(i)->expansionFlags,
                  AMN_isIndexValidForSetting(i),
                  dst);
    }

// タイトルIDからの逆引き
    {
        static const NAMTitleId table[] = {
            0x00030015344e4141, // "4NAA" - UIG launcher default
            0x0003001534303141, // "401A" - UIG launcher clone
            0x0003001534303241, // "402A" - UIG launcher clone
            0x00030017484e4141, // "HNAA" - RED launcher default
            0x00030015484e4241, // "HNBA" - RED MachineSettings
            0x0000000000000001, // nonexistent
            0 // stopper
        };
        u32 i;

        i = 0;
        while (table[i]) {
            OS_TPrintf("index by titleId(%016llx)=%d\n", (u64)table[i], AMN_getIndexByTitleId(table[i]));
            i++;
        }
    }

// バナーイメージデータ
    {
        s32 index;
        const u8* pImage;

        index = 3; //app::Manager::cNandTitleIndexStart;
        pImage = AMN_getBannerImage(index);
        OS_TPrintf("banner image for index(%02d)=%02x %02x %02x %02x %02x %02x %02x %02x \n",
                  index,
                  pImage[0], pImage[1], pImage[2], pImage[3],
                  pImage[4], pImage[5], pImage[6], pImage[7]);
    }

}

--------------------------------------------------------------------------------------------------------*/
