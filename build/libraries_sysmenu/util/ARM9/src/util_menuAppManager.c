/*!
  @file menuAppManager.cpp
  @brief アプリマネージャ
 */
#ifndef TWL_IPL_MENU_APP_MANAGER_FOR_STG
#include <sysmenu/util_menuAppManager.h>
#endif

#include <stddef.h>

void *(*AMNi_Alloc)( u32 size  ) = NULL;
void  (*AMNi_Free )( void *ptr ) = NULL;

/////////////////////////////////////////////////////////////////////////////////
// private:
static BOOL AMN_isTitleIdValidForLauncher(NAMTitleId id);

static void AMN_initFlags_();
static void AMN_initCardTitleList_();
static void AMN_initNandTitleList_();
static void AMN_getNandTitleList_();

static void AMN_lockSubBannerFileBuffer();
static void AMN_unlockSubBannerFileBuffer();

static BOOL AMN_checkBannerFile(TWLBannerFile* pBanner);
static u32  AMN_getBannerAnimeCRC(const BannerAnime* pAnime);
static BOOL AMN_checkAndReplaceBannerAnime(s32 index);

static vu8     mThreadStarted;
static vu8     mReadCancelFlag; // とりあえず無視。
static vu8     mNandTitleListReady;
static vu8     mCardTitleListLength;

static s32     mNandAllTitleListLength; // error if <= 0
static s32     mNandTitleListLengthForLauncher;

static AppInfo     mAllAppInfoArray[cAllTitleArrayMax];

enum {
    cEventMask_SubBannerFileBuffer = 0x1
};

static OSEvent         mSubBannerFileBufferEvent;

enum {
    cThreadPriority_Read  = OS_THREAD_LAUNCHER_PRIORITY + 1,
    cThreadPriority_Check = OS_THREAD_LAUNCHER_PRIORITY + 2
};

static OSThread        mReadThread;
static OSThread        mCheckThread;
static u64             mReadThreadStack[1024];
static u64             mCheckThreadStack[2048]; // 16KBytes FSとNAMを使うので少し多めに取る
static OSMessage       mCheckMsgBuf[1];
static OSMessageQueue  mCheckMsgQueue;
static void AMN_procRead();
static void AMN_procCheck();

    // NAM_GetTitleBootContentPath()では
    // 「FS_ENTRY_LONGNAME_MAX 以上」らしいが、
    // TwlIPL_RED/build/libraries_sysmenu/ARM9/src/banner.cの
    // PATH_LENGTHにより…
static char    mBootContentPath[1024];
static TWLBannerFile*      mpNandBannerFileArray;
static TWLBannerFile*      mpEmptyBannerFileBuffer;
    // バナーバッファ、サブバナーバッファは0x20 Bytes alignedにしたい。
    // メモリが厳しいのでサブバナーバッファは1個で使い回す。
static TWLSubBannerFile*   mpSubBannerFileBuffer;

static AMNBannerCounter       mBannerCounterArray[cAllTitleArrayMax];
static AMNFrameAnimeData      mFrameAnimeDataArray[cAllTitleArrayMax];
static AMNFrameAnimeData      mEmptyAnimeData;

static ROM_Header_Short    mRomHeaderBuffer;

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

BOOL AMN_isNandTitleListReady() { return mNandTitleListReady; }
s32 AMN_getNandTitleListLengthForLauncher() { return mNandTitleListLengthForLauncher; }
s32 AMN_getCardTitleListLength() { return mCardTitleListLength; }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*!
  コンストラクタ（C++時代の遺物）
  もうAMN_Init()とまとめて良いかも……
 */
void AMN_Manager( )
{
	u32 loop;
	mThreadStarted = FALSE;
	mpNandBannerFileArray = NULL;
	mpEmptyBannerFileBuffer = NULL;
	mpSubBannerFileBuffer = NULL;

    // CARDアプリは1個まで。
    SDK_ASSERT( cCardTitleArrayMax == 1 );
    // NANDアプリは39個まで表示できる。
    SDK_ASSERT( cNandTitleArrayMax == 39 );

#ifndef TWL_IPL_FINAL
    // 構造体offsetが変わっていないか監視。
    SDK_ASSERT( sizeof(ExpansionFlags) == sizeof(u8) );
    for (loop = 0; loop < 8; loop++) {
        ExpansionFlags flags;
        u32 val;

        flags.packed = (u8)(1 << loop);
        val = 0;
        switch (loop) {
        case 0: val = flags.codec_mode; break;
        case 1: val = flags.otherParentalControls; break;
        case 2: val = flags.subBannerFile; break;
        case 3: val = flags.WiFiConnectionIcon; break;
        case 4: val = flags.DSWirelessIcon; break;
        case 5: val = flags.game_card_on; break;
        case 6: val = flags.enable_nitro_whitelist_signature; break;
        case 7: val = flags.rsv; break;

        default:
            SDK_ASSERT( FALSE );
            break;
        }
        if (!val) {
            OS_TPrintf("ExpansionFlags check failed (loop=%d)\n", loop);
            OS_Halt();
            // TWL_IPL_FINALではスルーするー
            // そもそもTWL_IPL_FINALではこの部分は無いものとして扱われる
        }
    }

    SDK_ASSERT( sizeof(mRomHeaderBuffer.parental_control_rating_info) == cParentalControlRatingInfoSize );

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
    // この時点でAMNi_AllocとAMNi_FreeがNULLであるべし
    SDK_ASSERT( AMNi_Alloc == NULL && AMNi_Free == NULL );

    AMNi_Alloc = pAlloc;
    AMNi_Free  = pFree;

    mpNandBannerFileArray = (TWLBannerFile*)AMNi_Alloc( sizeof(TWLBannerFile) * cNandTitleArrayMax );
    SDK_ASSERT( mpNandBannerFileArray );

    mpEmptyBannerFileBuffer = (TWLBannerFile*)AMNi_Alloc( sizeof(TWLBannerFile) );
    SDK_ASSERT( mpEmptyBannerFileBuffer );

    mpSubBannerFileBuffer = (TWLSubBannerFile*)AMNi_Alloc( sizeof(TWLSubBannerFile) );
    SDK_ASSERT( mpSubBannerFileBuffer );

    MI_CpuClearFast(mpEmptyBannerFileBuffer, sizeof(TWLBannerFile));
    mEmptyAnimeData.image = mpEmptyBannerFileBuffer->v1.image;
    mEmptyAnimeData.pltt  = mpEmptyBannerFileBuffer->v1.pltt;
    mEmptyAnimeData.hflip = FALSE;
    mEmptyAnimeData.vflip = FALSE;

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

    AMN_restart();
}

/*!
  再起動。
 */
void AMN_restart()
{
    // AMNi_AllocおよびAMNi_Freeはセットされているべし
    SDK_ASSERT( AMNi_Alloc != NULL && AMNi_Free != NULL );

    AMN_initFlags_();

    SDK_ASSERT( !mThreadStarted || OS_IsThreadTerminated(&mReadThread) );

    mThreadStarted = TRUE;

    OS_InitEvent(&mSubBannerFileBufferEvent);
    AMN_unlockSubBannerFileBuffer();
    OS_InitMessageQueue(&mCheckMsgQueue, mCheckMsgBuf, sizeof(mCheckMsgBuf) / sizeof(*mCheckMsgBuf));
    OS_CreateThread(&mReadThread, AMN_procRead, NULL,
                    mReadThreadStack + (sizeof(mReadThreadStack) / sizeof(*mReadThreadStack)),
                    sizeof(mReadThreadStack),
                    cThreadPriority_Read);
    OS_CreateThread(&mCheckThread, AMN_procCheck, NULL,
                    mCheckThreadStack + (sizeof(mCheckThreadStack) / sizeof(*mCheckThreadStack)),
                    sizeof(mCheckThreadStack),
                    cThreadPriority_Check);
    // for safety, wakeup mCheckThread first
    OS_WakeupThreadDirect(&mCheckThread);
    OS_WakeupThreadDirect(&mReadThread);
}

/*!
  終了。
 */
void AMN_destroy()
{
    mReadCancelFlag = TRUE;
    while ( mThreadStarted && !OS_IsThreadTerminated(&mReadThread) ) {
        OS_Sleep(1);
    }

	if( AMNi_Free != NULL && mpSubBannerFileBuffer)
	{
		AMNi_Free( mpSubBannerFileBuffer );
	}
	if( AMNi_Free != NULL && mpEmptyBannerFileBuffer)
	{
		AMNi_Free( mpEmptyBannerFileBuffer );
	}
	if( AMNi_Free != NULL && mpNandBannerFileArray)
	{
		AMNi_Free( mpNandBannerFileArray );
	}
}

void AMN_initFlags_()
{
    mReadCancelFlag = FALSE;
    mNandTitleListReady = FALSE;
    mNandAllTitleListLength = 0;
    mNandTitleListLengthForLauncher = 0;
    mCardTitleListLength = 0;
}

void AMN_initCardTitleList_()
{
}

/*!
  see also SYSM_InitNandTitleList() and SYSM_GetNandTitleList()
  インポートされているすべてのNANDアプリを列挙したリストの準備
 */
void AMN_initNandTitleList_()
{
    s32 ret;
    s32 l;
    NAMTitleId* pNandAllTitleIDList = NULL;
    AppInfo* pApp;

    // インポートされているタイトルの取得
    mNandAllTitleListLength = NAM_GetNumTitles();
    OS_TPrintf("NAM_GetNumTitles()=%d\n", mNandAllTitleListLength);
    // 少なくともランチャーと無線ファームがリストアップされるはず。
    // だが、一応0は通す。
    SDK_ASSERT( mNandAllTitleListLength >= 0 );

    if (mNandAllTitleListLength > 0) {
        pNandAllTitleIDList = (NAMTitleId*)AMNi_Alloc( sizeof(NAMTitleId) * mNandAllTitleListLength );
        SDK_ASSERT( pNandAllTitleIDList );

        if (!pNandAllTitleIDList) {
            // error
            mNandAllTitleListLength = 0;
        } else {
            ret = NAM_GetTitleList(pNandAllTitleIDList, (u32)mNandAllTitleListLength);
            OS_TPrintf("NAM_GetTitleList()=%d\n", ret);
            SDK_ASSERT( ret == NAM_OK );

            if (ret != NAM_OK) {
                // error
                mNandAllTitleListLength = 0;
            }
        }
    }

    // メモリの確保と解放を同じ関数内で行いたいので、
    // 取得したタイトルがローンチ対象かどうかをチェック
    // を、ここで行う。
    // ただしバナーはまだ読まない。まずはローンチ対象タイトルの絞り込み。

    pApp = &mAllAppInfoArray[cNandTitleIndexStart];
    for (l = 0; l < mNandAllTitleListLength; l++) {
        if (AMN_isTitleIdValidForLauncher(pNandAllTitleIDList[l])) {
            pApp->prop.titleID = pNandAllTitleIDList[l];
            // この時点でpBannerが指すバッファは不定値だが問題なか。
            pApp->prop.pBanner = &mpNandBannerFileArray[mNandTitleListLengthForLauncher];
            pApp->prop.flags.isValid = TRUE;
            pApp->prop.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
            pApp++;
            mNandTitleListLengthForLauncher++;
    // 本体設定の場合、アプリマネージャ用indexは飛び飛びになったり、ForSetting()が返す値(個数)より大きくなるので
    // getNandTitleListLengthForSetting()は用意しない。
            if (mNandTitleListLengthForLauncher >= cNandTitleArrayMax) {
                break;
            }
        }
    }

    if (pNandAllTitleIDList) {
        AMNi_Free( pNandAllTitleIDList);
    }
}

/*!
  see also SYSM_GetNandTitleList()
 */
void AMN_getNandTitleList_()
{
    s32 loop;
    s32 index;
    FSFile file[1];
    s32 readLen;
    s32 offset;
    const u32 expansionFlagsOffset = 0x1bf;
    AppInfo* pApp;

    // errorの場合は、メインバナーはfatal。サブバナーは無視、でよい？

    loop = 0;
    // while (!mReadCancelFlag) になる？
    while (loop < mNandTitleListLengthForLauncher) {
		s32 wantLen;
        // 順序リストから、次に取得するタイトルの
        // マネージャ用indexを取り出す。
        // まだ。

        index = cNandTitleIndexStart + loop;
        // remove const cast
        pApp = (AppInfo*)AMN_getAppInfo(index);
        SDK_ASSERT( pApp );

	// ファイルパスを取得
        readLen = NAM_GetTitleBootContentPathFast( mBootContentPath, pApp->prop.titleID );
        if (readLen != NAM_OK) {
            // error
            SDK_ASSERT( FALSE );
            goto next;
        }

	// ファイルオープン
        FS_InitFile(file);
        if (!FS_OpenFileEx(file, mBootContentPath, FS_FILEMODE_R)) {
            // error
            SDK_ASSERT( FALSE );
            goto next;
        }

#if 1
        // 1 seek - faster
        // ROMヘッダのバナーデータオフセットを読み込む
        wantLen = offsetof(ROM_Header_Short, parental_control_rating_info)
                    + sizeof(mRomHeaderBuffer.parental_control_rating_info)
                    - offsetof(ROM_Header_Short, banner_offset);
        if (!FS_SeekFile(file, offsetof(ROM_Header_Short, banner_offset), FS_SEEK_SET)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        readLen = FS_ReadFile(file, &mRomHeaderBuffer.banner_offset, wantLen);
        if (readLen != wantLen) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        offset = (s32)mRomHeaderBuffer.banner_offset;
        pApp->expansionFlags = *(ExpansionFlags *)(((u32)&mRomHeaderBuffer) + expansionFlagsOffset);
        MI_CpuCopy(mRomHeaderBuffer.parental_control_rating_info,
                   pApp->parental_control_rating_info,
                   sizeof(pApp->parental_control_rating_info));
#else
        // 3 seeks
        // ROMヘッダのバナーデータオフセットを読み込む
        if (!FS_SeekFile(file, 0x68, FS_SEEK_SET)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        readLen = FS_ReadFile(file, &offset, sizeof(offset));
        if (readLen != sizeof(offset)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        // TWL expansion flagsを読み込む。
        if (!FS_SeekFile(file, 0x1bf, FS_SEEK_SET)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        readLen = FS_ReadFile(file, &pApp->expansionFlags, sizeof(pApp->expansionFlags));
        if (readLen != sizeof(pApp->expansionFlags)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        // Parental Controls Rating Infoを読み込む。
        if (!FS_SeekFile(file, 0x2f0, FS_SEEK_SET)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
        readLen = FS_ReadFile(file, pApp->parental_control_rating_info, sizeof(pApp->parental_control_rating_info));
        if (readLen != sizeof(pApp->parental_control_rating_info)) {
            // error
            SDK_ASSERT( FALSE );
            FS_CloseFile(file);
            goto next;
        }
#endif

	// バナーが存在する場合のみリード
        if (offset) {
            if (!FS_SeekFile(file, offset, FS_SEEK_SET)) {
                // error
                SDK_ASSERT( FALSE );
                FS_CloseFile(file);
                goto next;
            }
            readLen = FS_ReadFile(file, pApp->prop.pBanner, sizeof(*pApp->prop.pBanner));
            if (readLen != sizeof(TWLBannerFile)) {
                // error
                SDK_ASSERT( FALSE );
                FS_CloseFile(file);
                goto next;
            }
#if 1
            // バナーチェックリクエスト送信。
            OS_SendMessage(&mCheckMsgQueue, (OSMessage)(pApp->prop.pBanner), OS_MESSAGE_BLOCK);
#else
            if (!checkBannerFile(pApp->prop.pBanner)) {
                // 正当性チェック失敗の場合はバッファクリア
                MI_CpuClearFast(pApp->prop.pBanner, sizeof(*pApp->prop.pBanner));
                IPL_PRINT("check NG! index=%d\n", index);
            }
#endif
        } else {
            // バナーが存在しない場合はバッファクリア
            MI_CpuClearFast(pApp->prop.pBanner, sizeof(*pApp->prop.pBanner));
        }
        FS_CloseFile(file);

        // サブバナーファイルを読み込んでみる
        if (pApp->expansionFlags.subBannerFile &&
            NAM_OK == NAM_GetTitleBannerFilePath(mBootContentPath, pApp->prop.titleID)) {
            FS_InitFile(file);
            if (FS_OpenFileEx(file, mBootContentPath, FS_FILEMODE_R)) {
                // サブバナーバッファ確保。
                AMN_lockSubBannerFileBuffer();
                readLen = FS_ReadFile(file, mpSubBannerFileBuffer, sizeof(*mpSubBannerFileBuffer));
                FS_CloseFile(file);
                if (readLen == sizeof(TWLSubBannerFile)) {
                    // 読み込みには成功したので正当性チェック
#if 1
                    // サブバナーチェックリクエスト送信。
                    OS_SendMessage(&mCheckMsgQueue, (OSMessage)index, OS_MESSAGE_BLOCK);
                    // 解放はCHECKスレッド側で行う。
#else
                    (void)checkAndReplaceBannerAnime(index);
                    // 解放。
                    unlockSubBannerFileBuffer();
#endif
                } else {
                    OS_TPrintf("subbanner read failed (%d)\n", readLen);
                    // 解放。
                    AMN_unlockSubBannerFileBuffer();
                }
            }
        }

next:

        loop++;
        continue;
    }
}

void AMN_procRead()
{
    MI_CpuClearFast(mAllAppInfoArray, sizeof(mAllAppInfoArray));

    AMN_initCardTitleList_();
    AMN_initNandTitleList_();
    if (mNandAllTitleListLength > 0) {
        AMN_getNandTitleList_();
    }

    OS_SendMessage(&mCheckMsgQueue, NULL, OS_MESSAGE_BLOCK);
    OS_JoinThread(&mCheckThread);

    AMN_stepBannerAnimeAll( TRUE );

    // for GX DMA
    DC_StoreAll();

    if (!mReadCancelFlag) {
        mNandTitleListReady = TRUE;
    }
}


void AMN_procCheck()
{
    OSMessage msg;
    s32 index;
    TWLBannerFile* pBanner;

    while (TRUE) {
        OS_ReceiveMessage(&mCheckMsgQueue, &msg, OS_MESSAGE_BLOCK);
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
                OS_TPrintf("check NG! msg=0x%x propAry=0x%x\n", pBanner, mAllAppInfoArray);
            }
        }
    }
}

void AMN_lockSubBannerFileBuffer()
{
    OS_WaitEventEx_And(&mSubBannerFileBufferEvent,
                       cEventMask_SubBannerFileBuffer,
                       cEventMask_SubBannerFileBuffer);
}

void AMN_unlockSubBannerFileBuffer()
{
    OS_SignalEvent(&mSubBannerFileBufferEvent,
                   cEventMask_SubBannerFileBuffer);
}

const AppInfo* AMN_getAppInfo(s32 index)
{
    const AppInfo* pApp;

    if (cAllTitleIndexStart <= index && index < cAllTitleArrayMax) {
        pApp = &mAllAppInfoArray[index];
        if (pApp->prop.flags.isValid) {
            return pApp;
        }
    }

    return NULL;
}

BOOL AMN_isDSWirelessIcon(s32 index)
{
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (pApp) {
        return pApp->expansionFlags.DSWirelessIcon;
    }

    return FALSE;
}

BOOL AMN_isWiFiConnectionIcon(s32 index)
{
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (pApp) {
        return pApp->expansionFlags.WiFiConnectionIcon;
    }

    return FALSE;
}

BOOL AMN_isOtherParentalControlsFlag(s32 index)
{
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (pApp) {
        return pApp->expansionFlags.otherParentalControls;
    }

    return FALSE;
}

const u8* AMN_getParentalControlRatingInfo(s32 index)
{
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (pApp) {
        return pApp->parental_control_rating_info;
    }

    return FALSE;
}

// タイトルIDが本体設定用としては無効であっても、
// 対応するindexを返します。
s32 AMN_getIndexByTitleId(NAMTitleId titleId)
{
    const AppInfo* pApp;
    s32 index;

    // 一応、TitleID == 0 のアプリがあるかもしれない？ので、
    // NANDから優先的に探す。
    // CARDはTITLE_ID_MEDIA_MASKが1になるはず。

    // NAND
    index = cNandTitleIndexStart + mNandTitleListLengthForLauncher - 1;
    while (index >= cNandTitleIndexStart) {
        pApp = AMN_getAppInfo(index);
        if (pApp &&
            pApp->prop.titleID == titleId) {
            return index;
        }
        index--;
    }

    // CARD
    index = mCardTitleListLength - 1;
    while (index >= cCardTitleIndexStart) {
        pApp = AMN_getAppInfo(index);
        if (pApp &&
            pApp->prop.titleID == titleId) {
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
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (pApp) {
        return pApp->prop.titleID;
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
        pBNC = &mBannerCounterArray[index];
        if (restart) {
            pBanner = mAllAppInfoArray[index].prop.pBanner;
            if (!pBanner) {
                pBanner = mpEmptyBannerFileBuffer;
            }
            AMN_BNC_initCounter(pBNC, pBanner);
        } else {
            AMN_BNC_incrementCount(pBNC);
        }
        mFrameAnimeDataArray[index] = AMN_BNC_getFAD(pBNC);
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
        return mFrameAnimeDataArray[index];
    }

    return mEmptyAnimeData;
}

const u16* AMN_getBannerText2(s32 index, OSLanguage language)
{
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (language >= OS_LANGUAGE_CODE_MAX) {
        language = OS_LANGUAGE_ENGLISH;
    }

    if (pApp) {
        if (language < BANNER_LANG_NUM_V1) {
            return pApp->prop.pBanner->v1.gameName[language];
        }
        language = (OSLanguage)(language - BANNER_LANG_NUM_V1);
        if (language < BANNER_LANG_NUM_V2) {
            return pApp->prop.pBanner->v2.gameName[language];
        }
        language = (OSLanguage)(language - BANNER_LANG_NUM_V2);
        return pApp->prop.pBanner->v3.gameName[language];
    }

    return mpEmptyBannerFileBuffer->v1.gameName[language];
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
	u8		*pSrc;
	u32		size;
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
u32 AMN_getBannerAnimeCRC(const BannerAnime* pAnime)
{
    return SVC_GetCRC16(0xffff, pAnime, sizeof(*pAnime));
}

BOOL AMN_checkAndReplaceBannerAnime(s32 index)
{
    BOOL retval = FALSE;
    TWLBannerFile* pBanner;

    // 範囲外のindexを無視。
    if (cNandTitleIndexStart <= index && index < (cNandTitleIndexStart + mNandTitleListLengthForLauncher)) {
        if (mpSubBannerFileBuffer->h.crc16_anime == AMN_getBannerAnimeCRC(&mpSubBannerFileBuffer->anime)) {
            // 成功したのでコピーする
            pBanner = &mpNandBannerFileArray[index];
            // h.crc16_animeしかチェックしていないので、それとanimeだけコピー
            pBanner->h.crc16_anime = mpSubBannerFileBuffer->h.crc16_anime;
            // MI_CpuCopy()の方が遅くなった…
            pBanner->anime = mpSubBannerFileBuffer->anime;
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
    const AppInfo* pApp = AMN_getAppInfo(index);

    if (pApp) {
        return NAM_IsUserTitle(pApp->prop.titleID);
    }

    return FALSE;
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
	
	AMN_Manager();
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
	              AMN_getAppInfo(i)->expansionFlags,
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