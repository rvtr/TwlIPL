// 落ち着いたらcmnCameraManager.hへ持っていく。

#ifndef MENU_CAMERA_MANAGER_H_
#define MENU_CAMERA_MANAGER_H_

//#define TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
// まだDSP版JPEGライブラリを使う。
//#define TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
#endif

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#include "const.h"
#endif

namespace cmn { namespace sys { namespace cmr2
{

    // eMode_Texのみ対応。
// カメラモード
enum
{
  eMode_BG = 0 ,
  eMode_Tex ,
  eMode_BGSingle ,
  eMode_Max
};

// 定数
enum
{
  cDefaultShowWidth  = cScreenWidth ,
  cDefaultShowHeight = cScreenHeight ,
  cDefaultShotWidth  = cPhotoWidth ,
  cDefaultShotHeight = cPhotoHeight ,

  cEncodeResultSizeMax = cPhotoWidth * cPhotoHeight,
  cEncodeQuality   = 90,
  cEncodeSampling  = 2,

  cBufferCount = 2 ,

  cCaptureRetryMax = 20,

  cDefaultDmaNo = 1
};

enum
{
    cMsgI2c_Destroy = 0,
    cMsgI2c_AfterInit,
    cMsgI2c_Activate,

//    cMsgI2c_ChangeSizeToShot,
//    cMsgI2c_ChangeSizeToShow,

    cMsgI2c_ChangeLedToOn,
    cMsgI2c_ChangeLedToBlink,

    cMsgI2c_Max
};



enum
{
    // CAMERASelectはビットマスクなので注意。
    cCameraIndex_In = 0,
    cCameraIndex_Out,
    cCameraMax
};

enum
{
    // 通常時でも、起動直後に3回出る。
// CAMERA_RESULT_BUSY type 0 retry 0
// CAMERA_RESULT_BUSY type 0 retry 1
// CAMERA_RESULT_BUSY type 0 retry 2
    

    cI2cRetryMax = 30,
    cI2cRetryIntervalMsec = 100
};

enum
{
    cI2cCommon_Activate = 0,
    cI2cCommon_FrameRate,
    cI2cCommon_ContextSwitch,
    cI2cCommon_LedOn,
    cI2cCommon_LedBlink,

    cI2cCommon_Max
};


struct dtcmWork {
    u32 mShrinkBuf[208*3]; // max(256*2,208*3) * sizeof(u32)
    u64 mYuvBuf[12];
};

typedef struct {
    CAMERAContext   mContext;
    CAMERAFrameRate mFrameRate;
} cameraState;

class Manager
{
public:
    // サイズはinit()で指定する。
    Manager();
    ~Manager();

    void preInit( u32 show_w, u32 show_h, u32 shot_w, u32 shot_h );
    void preInit() {
        preInit( mShowWdt, mShowHgt, mShotWdt, mShotHgt );
    }

    void init(
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
        // no Heap
#else
               sys::Heap* p_heap ,
#endif
               int mode , CAMERASelect sel_cmr , int vram1 , int vram2 ,
               cmn::sys::cmr2::dtcmWork* pDtcmWork ,
               void (*gx_user_proc)(u8* vramAddr, BOOL shot) ,
               u32 show_w, u32 show_h, u32 shot_w, u32 shot_h );

    void init( 
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
        // no Heap
#else
               sys::Heap* p_heap ,
#endif
               int mode , CAMERASelect sel_cmr , int vram1 , int vram2 ,
               cmn::sys::cmr2::dtcmWork* pDtcmWork ,
               void (*gx_user_proc)(u8* vramAddr, BOOL shot) ) {
        init(
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
        // no Heap
#else
              p_heap , 
#endif
              mode, sel_cmr, vram1, vram2,
              pDtcmWork, gx_user_proc,
              mShowWdt, mShowHgt, mShotWdt, mShotHgt );
    }

    BOOL tryDestroy();

    void activate();

    void changeCamera( CAMERASelect sel_cmr ) { mNextCmr = sel_cmr; }
    void changeCamera()
    {
        changeCamera( mNextCmr == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN );
    }

    // 「内(外)カメラに切り替えました」旨のメッセージを表示する場合、
    // 厳密に今の瞬間のカメラではなく、
    // 最終的にどちらのカメラが有効になるかを返せば良いと思うので
    // mNextCmrを返す。
    CAMERASelect getCurrentCamera() const { return mNextCmr; }
    // でも、切り替えにタイムラグがあるので、切り替え中かどうかを
    // 知る方法も用意する。
    // mCaptureSkipCountは、他の要因(例えばカメラ解像度切り替え中)でも
    // 0でなくなる。その場合にこの関数が一瞬0でなくなるが、
    // IN/OUT切り替え中の判定のために呼び出す限りは問題ないはず。
    BOOL isCameraChanging() { return ((mSelCmr != mNextCmr) || mCaptureSkipCount); }

    void startCapture(CAMERASelect sel_cmr);
    void startCapture() {
        startCapture(mNextCmr);
    }
    void stopCapture()  { mNextCaptureFlag = FALSE; }
    BOOL isCaptureBusy() { return (mCaptureFlag || mNextCaptureFlag); }
    BOOL isSaveBusy() { return mEncodeBusyFlag; }
    BOOL shoot(BOOL keepCapture = FALSE);
    BOOL tryPrepareToSleep();

//   void setShutterSnd( const u8* p_wav , u32 size , BOOL stereo );
// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
//   void startShutterSnd();
//   void finishShutterSnd();
//   BOOL isFinishedShutterSnd() const;
// #endif

    virtual void procVBlankIntr();
    virtual void procCameraIntr();
    virtual u16* procPreCapture();
    virtual void procPostCapture();
    virtual void procPostConvert(BOOL shot);
    virtual void procPostShot();
    virtual u16* allocCapBuf();
    virtual void freeCapBufByPtr(const u16* pBuf);
    BOOL isShotDone() { return mShotDone; }
    void clearShotDone() { mShotDone = FALSE; }
    void setTexSwap() { mTexSwap = TRUE; }
//    int getTexBank() { return mTexBank; }
    void setOneShotFlag(BOOL flag) { mOneShotFlag = (u8)flag; }
    BOOL getOneShotFlag() { return mOneShotFlag; }
    virtual BOOL convert(BOOL shot);

    void setCaptureRgbFlag(BOOL useRgb);
    BOOL isFirstPictureReady() { return mFirstPictureReady; }

#ifndef TWL_IPL_FINAL
public:
    vu32 mDebugPort;
#endif

private:

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
        // no Heap
#else
    sys::Heap* mpHeap;
#endif

    u8  mLedKeepFlag;
    u8  mOneShotFlag;
    u8  mCaptureRgbFlag;

    vu8 mEncodeBusyFlag;

    vu8 mPreInitFlag;
    vu8 mInitFlag;
    vu8 mPreInitBusyFlag;
    vu8 mInitBusyFlag;

    u32 mEncodeOption;

    static vu8 mCaptureFlag;
    static vu8 mNextCaptureFlag;
    static vu8 mShotFlag;
    static vu8 mNextShotFlag;

    vu8 mShotDone;
    vu8 mTexSwap;
    vu8 mI2cBusy;
#if SDK_VERSION_DATE < 20080630
    vu8 mIntrPositiveEdge;
#else
    // intr edgeはCAMERAライブラリがハンドリングする。
#endif
    vu8 mFirstPictureReady;

    vu32 mShotErr;

    u32  mCaptureSkipCount;
    u32  mCaptureRetryCount;

    u8 mIsCapBufUsed[2];
    u16* mpCapBuf[2];
    vu16* mpCaptureBuf;
    vu16* mpConvBuf;
    vu16* mpNextConvBuf;
    vu16* mpShotBuf;
    dtcmWork* mpDtcmWork;

    volatile int mTexBank;
    volatile int mNextTexBank;

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
    struct encodeWork {
        u16 y[cPhotoWidth * cPhotoHeight];
        u16 cbcr[(cPhotoWidth * cPhotoHeight / 4) * 2];
    } *mpEncodeWork;
#else
    u8* mpEncodeWork;
#endif

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    u8* mpEncodeResult;
#endif

  int mTarget;

  u32 mShowWdt;
  u32 mShowHgt;
  u32 mShotWdt;
  u32 mShotHgt;
  int mShowTransferLines;
  int mShotTransferLines;

  int mMode;

  u32 mGxDma;

  int mVramType[ 2 ];
  u8* mVramAddr[ 2 ];

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
    const u8* mpShutterSnd;
    u32 mShutterSndSize;
    BOOL mShutterStereo;
#endif

    MINDmaConfig    mDmaConfig;

    // 手元の実験ではmpTaskが使えないので…
    OSThread        mLowerThread;
    OSThread        mI2cThread;
    u64             mLowerThreadStack[2048]; // JPEGを使うので、念のため16KBで。
    u64             mI2cThreadStack[1024];
    OSMessage       mI2cMsgBuf[1];
    OSMessageQueue  mI2cMsgQueue;
    enum {
        cEventMaskLower_Destroy                         = 1 << 0,
        cEventMaskLower_EnableIntrAndPrepareBuffer      = 1 << 1,
        cEventMaskLower_DisableIntrAndDestroyBuffer     = 1 << 2,
        cEventMaskLower_Convert                         = 1 << 3,
        cEventMaskLower_Shot                            = 1 << 4,
        cEventMaskLower_NextConvertAndShot              = 1 << 5,
        cEventMaskLower_Activate                        = 1 << 6,

        cEventMaskLower_All = 0xffffffff
    };
    OSEvent         mLowerEvent;
    void procLower();
    void procI2c();
    static void procLower_(void* p_arg) {
        reinterpret_cast<Manager*>(p_arg)->procLower();
    }
    static void procI2c_(void* p_arg) {
        reinterpret_cast<Manager*>(p_arg)->procI2c();
    }

    static OSMessage       mI2cAsyncMsgBuf[1];
    static OSMessageQueue  mI2cAsyncMsgQueue;
    CAMERAResult i2cCommon(u32 type, CAMERASelect sel_cmr);
    CAMERAResult i2cSetupOthers();
    CAMERAResult i2cActivate(CAMERASelect sel_cmr);
    CAMERAResult i2cFrameRate(CAMERASelect sel_cmr);
    CAMERAResult i2cContextSwitch(CAMERASelect sel_cmr);
    CAMERAResult i2cLed(BOOL isBlink);
    static void i2cCommonAsyncCallback(CAMERAResult result, void *arg);
    static void i2cActivateCallback(CAMERAResult result, void *arg);
    static void i2cFrameRateCallback(CAMERAResult result, void *arg);
    static void i2cContextSwitchCallback(CAMERAResult result, void *arg);
    static void i2cLedCallback(CAMERAResult result, void *arg);

    CAMERASelect indexToCameraSelect(u32 i) { return (CAMERASelect)(CAMERA_SELECT_IN << i); }
    static BOOL isOutCamera(CAMERASelect sel_cmr) { return (sel_cmr == CAMERA_SELECT_OUT); }

    static vu8 mCurLedBlink;
    static volatile CAMERASelect mSelCmr;
    static volatile CAMERASelect mNextCmr;

    static volatile cameraState mCmrCurState[cCameraMax];
    static volatile cameraState mCmrNextState[cCameraMax];

    void prepareBuffer();
    void destroyBuffer();
    void useNextConvBuf();
    void encodeAndSave();
    void setInterrupt(BOOL enable);

    void (*mpGxUserProc)(u8* vramAddr, BOOL shot);
    BOOL writeJpegToSD(const u8* src, u32 size);
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
    BOOL writeJpegToNand(const u8* src, u32 size);
#endif

    // if non-static, CW claims:
    //   Error: 'pDtcm' could not be assigned to a register
    //   Error: internal compiler error (report to ...)
    //          while executing in file 'PCodeAssembly.c' line: 468
    static void Shrink640Yuv422To208Rgb555( const void* src, void* dst, u32 height, void* pDtcm );
    static void Shrink640Yuv422To256Rgb555( const void* src, void* dst, u32 height, void* pDtcm );
    static void Convert8bitYuv422To16bitYuv420( const u32* src, u16* pYCbCrBuf, u32 width, u32 height, void* pDtcm_ );

};

}}}

#endif
