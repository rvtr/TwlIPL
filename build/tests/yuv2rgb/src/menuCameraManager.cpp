// TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT�ɂ���
// �����`���[�ł͏�ɖ���`�ł��B
// ���̎茳�ōs���Ă���J���������ɓ������������J���v���W�F�N�g��
// �\�[�X�����p���邽�߁A������֑g�ݍ��܂�鎞�̂ݒ�`����܂��B

// ������������cmnCameraManager.cpp�֎����Ă����B

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
#include "menu.h"
//#include "cmnJpegManager.h"
#include "cmnFile.h"
//#include "cmnImageSampling.h"
#include "menuImageSampling.h"
#endif


#include <twl.h>
#include <twl/camera.h>
#include "menuCameraManager.h"
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
#include <twl/dsp.h>
#include <twl/dsp/ARM9/dsp_jpeg_enc.h>
#else
#include <twl/armjpeg/ARM9/jpegenc.h>
#include <twl/armjpeg/ARM9/exifenc.h>
#endif

// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
// #else
// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
// #include "menuDspManager.h"
// #endif
// #endif

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
extern "C" {
// build/libraries/dsp/ARM9.TWL/src/dsp_jpeg_enc.c
extern u16 *DSPFrame_y; 
extern u16 *DSPFrame_cb; 
extern u16 *DSPFrame_cr;
} /* extern "C" */
#endif

#if SDK_VERSION_DATE < 20080630
#else
/*!
  �J����Vsync�R�[���o�b�N
 */
static void proc_camera_vsync_callback( CAMERAResult result )
{
#pragma unused( result )
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    {
        extern cmn::sys::cmr2::Manager* mpCmrMgr2;
        mpCmrMgr2->procCameraIntr();
    }
#else
    MenuSys()->getCmrMgr2()->procCameraIntr();
#endif
}
#endif

namespace cmn { namespace sys { namespace cmr2
{
    vu8 Manager::mCaptureFlag;
    vu8 Manager::mNextCaptureFlag;
    vu8 Manager::mShotFlag;
    vu8 Manager::mNextShotFlag;
    vu8 Manager::mCurLedBlink;
    volatile CAMERASelect Manager::mSelCmr;
    volatile CAMERASelect Manager::mNextCmr;
    volatile cameraState Manager::mCmrCurState[cCameraMax];
    volatile cameraState Manager::mCmrNextState[cCameraMax];
    OSMessage       Manager::mI2cAsyncMsgBuf[1];
    OSMessageQueue  Manager::mI2cAsyncMsgQueue;

/*!
  �R���X�g���N�^
 */
Manager::Manager() :
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    // no Heap
#else
    mpHeap( NULL ),
#endif
    mShowWdt( cDefaultShowWidth ),
    mShowHgt( cDefaultShowHeight ),
    mShotWdt( cDefaultShotWidth ),
    mShotHgt( cDefaultShotHeight ),
    mLedKeepFlag( FALSE ),
    mOneShotFlag( FALSE ),
    mEncodeBusyFlag( FALSE ),
    mPreInitFlag( FALSE ),
    mInitFlag( FALSE ),
    mpCaptureBuf( NULL ),
    mpConvBuf( NULL ),
    mpNextConvBuf( NULL ),
    mpShotBuf( NULL ),
    mI2cBusy( FALSE ),
    mTexSwap( FALSE ),
    mTexBank( GX_VRAM_TEX_NONE ),
    mNextTexBank( GX_VRAM_TEX_NONE ),
    mpEncodeWork(NULL)
{
    u32 i;

    // manager default value
    for (i = 0; i < cCameraMax; i++) {
        mCmrNextState[i].mContext   = CAMERA_CONTEXT_B;
        // �t���[�����[�g���A�J�������ɕێ����Ă���B
        mCmrNextState[i].mFrameRate = CAMERA_FRAME_RATE_20;
    }

    for (i = 0; i < cBufferCount; i++) {
        mIsCapBufUsed[i] = FALSE;
        mpCapBuf[i] = NULL;
    }

    mSelCmr = mNextCmr = CAMERA_SELECT_NONE;
    mCaptureFlag = mNextCaptureFlag = mShotFlag = mNextShotFlag = FALSE;
    mCurLedBlink = FALSE;

    // �������͂��̃^�C�~���O�Ŋm���ɍs���Ă����B
    OS_InitEvent(&mLowerEvent);

    setCaptureRgbFlag( FALSE );

// TwlSDK/build/libraries/mi/common.TWL/src/mi_ndma.c:
// #define CAMERA_DMA_BLOCK_SIZE      MI_NDMA_BWORD_16
// #define CAMERA_DMA_INTERVAL        2
// #define CAMERA_DMA_PRESCALER       MI_NDMA_INTERVAL_PS_1
    MI_GetNDmaConfig(cDefaultDmaNo, &mDmaConfig);
    mDmaConfig.intervalTimer = 2; // CAMERA_DMA_INTERVAL
    mDmaConfig.prescaler = MI_NDMA_INTERVAL_PS_1; // CAMERA_DMA_PRESCALER;
    // blockWord��16����jackal (hyena)�g�p���̏����������������A
    // yuv2rgb�����ɂ����āA32�Ȃ�64�Ƃ̍��͊������Ȃ������B
    // �����`���[�֑g�ݍ���ł݂�ƁA32���64�̕����������������Ȃ������B
    // 64�ɂ���B
    mDmaConfig.blockWord = MI_NDMA_BWORD_16; // MI_NDMA_BWORD_64; // CAMERA_DMA_BLOCK_SIZE;

}


/*!
  �f�X�g���N�^
 */
Manager::~Manager()
{
    while (!tryDestroy()) {
        OS_Sleep(1);
    }
}


/*!
  �J������s�������B
  �̊��I�ȃJ�����N�����Ԃ�Z�k���邽�߁A�{�������̎�O
  (�q�[�v��K�v�Ƃ��Ȃ�����)�܂ōs���B
  �J�����̓X�^���o�C�ɁB
  �Ă΂Ȃ��Ă��ǂ��B
 */
void Manager::preInit( u32 show_w, u32 show_h, u32 shot_w, u32 shot_h )
{
    // preInit()�͕����ӏ�����Ă΂�Ă��ǂ��悤�ɂ��Ă����B
    {
        OSIntrMode enabled = OS_DisableInterrupts();

        if (mPreInitFlag) {
            OS_RestoreInterrupts(enabled);
            return;
        }
        mPreInitBusyFlag = TRUE;
        mPreInitFlag = TRUE;
        OS_RestoreInterrupts(enabled);
    }

    mShowWdt = show_w;
    mShowHgt = show_h;
    mShotWdt = shot_w;
    mShotHgt = shot_h;

//    setShutterSnd(NULL, 0, FALSE);

    // ���O�̏I���v�����c���Ă��邩������Ȃ��B
    // �������v���ȊO�A�C�x���g�̓N���A���Ă����B
    OS_ClearEvent(&mLowerEvent, (u32)(cEventMaskLower_All & ~cEventMaskLower_EnableIntrAndPrepareBuffer));

    OS_InitMessageQueue(&mI2cMsgQueue, mI2cMsgBuf, sizeof(mI2cMsgBuf) / sizeof(*mI2cMsgBuf));
    OS_InitMessageQueue(&mI2cAsyncMsgQueue, mI2cAsyncMsgBuf, sizeof(mI2cAsyncMsgBuf) / sizeof(*mI2cAsyncMsgBuf));
    OS_CreateThread(&mLowerThread, procLower_, this,
                    mLowerThreadStack + (sizeof(mLowerThreadStack) / sizeof(*mLowerThreadStack)),
                    sizeof(mLowerThreadStack),
                    menu::sys::Object::cThreadPriority_CameraLowerLo);
    OS_CreateThread(&mI2cThread, procI2c_, this,
                    mI2cThreadStack + (sizeof(mI2cThreadStack) / sizeof(*mI2cThreadStack)),
                    sizeof(mI2cThreadStack),
                    menu::sys::Object::cThreadPriority_CameraI2c);
    OS_WakeupThreadDirect(&mLowerThread);
    OS_WakeupThreadDirect(&mI2cThread);
}


/*!
  �J�������C�u�����̏�����
  @param p_heap �q�[�v
  @param mode �\�����[�h
  @param sel_cmr �I�����ꂽ�J����
  @param vram1 �_�u���o�b�t�@��VRAM1
  @param vram2 �_�u���o�b�t�@��VRAM2
  @param show_w �\������ (�Ƃ肠����208�̂�)
  @param show_h �\������ (�Ƃ肠����156�̂�)
  @param shot_w �B�e���� (�Ƃ肠����640�̂�)
  @param shot_h �B�e���� (�Ƃ肠����480�̂�)
 */
void Manager::init(
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    // no Heap
#else
                    sys::Heap* p_heap ,
#endif
                    int mode , CAMERASelect sel_cmr , int vram1 , int vram2 ,
                    cmn::sys::cmr2::dtcmWork* pDtcmWork ,
                    void (*gx_user_proc)(u8* vramAddr, BOOL shot) ,
                    u32 show_w , u32 show_h , u32 shot_w , u32 shot_h )
{
    if (mInitFlag) {
        return;
    }
    mInitBusyFlag = TRUE;
    mInitFlag = TRUE;

    preInit(show_w, show_h, shot_w, shot_h);

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    // no Heap
#else
    mpHeap = p_heap;
#endif

    mpDtcmWork = pDtcmWork;
    mTarget = 0;
    mpCaptureBuf = NULL;
    mpConvBuf = NULL;
    mpNextConvBuf = NULL;
    mpShotBuf = NULL;
    // �ŏ��ɏ����������̂�vram1�ŁA���łɏ����ς݂̂��̂�vram2
    // �Ƃ���B
    mTexBank = GXVRamTex(vram2);
    mNextTexBank = 0;
    mCaptureFlag = FALSE;
    mShotFlag = FALSE;
    mNextCaptureFlag = FALSE;
    mNextShotFlag = FALSE;
    mShotDone = FALSE;
    mShotErr = 0;
    mpGxUserProc = gx_user_proc;
#ifndef TWL_IPL_FINAL
    mDebugPort = 0;
#endif

    mTexSwap = FALSE;
    mFirstPictureReady = FALSE;

    mCaptureSkipCount = 0;
    mCaptureRetryCount = 0;

  mVramType[ 0 ] = vram1;
  mVramType[ 1 ] = vram2;

  // vram2 == 0 �Ȃ�V���O���o�b�t�@�ɂ���΂����̂ł́H
  for ( int i = 0 ; i < 2; ++ i )
  {
    switch ( mVramType[ i ] )
    {
    case GX_VRAM_A: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_A ); break;
    case GX_VRAM_B: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_B ); break;
    case GX_VRAM_C: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_C ); break;
    case GX_VRAM_D: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_D ); break;
    case GX_VRAM_E: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_E ); break;
    case GX_VRAM_F: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_F ); break;
    case GX_VRAM_G: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_G ); break;
    case GX_VRAM_H: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_H ); break;
    case GX_VRAM_I: mVramAddr[ i ] = reinterpret_cast< u8* > ( HW_LCDC_VRAM_I ); break;
    default: IPL_ASSERT( 0 ); break;
    }
  }

    mMode = mode;
    mNextCmr = sel_cmr;

    mGxDma = GX_GetDefaultDMA();
    // GX��DMA���g��Ȃ� (��DMA�̓J����DMA�̎ז��ɂȂ�)
    // �ŁA���߂��́H
    GX_SetDefaultDMA(GX_DMA_NOT_USE);

    OS_SignalEvent(&mLowerEvent,
                   cEventMaskLower_EnableIntrAndPrepareBuffer);
}


void Manager::procLower()
{
    u32 event;
    BOOL initDone = FALSE;
    CAMERAResult camResult;
    u32 i;
    BOOL shot;

    while (TRUE) {
        if (!initDone) {
#if SDK_VERSION_DATE < 20080630
            // CAMERA���C�u���������������Ă��Ȃ��Ƃ����O��Ȃ̂ŁA
            // setInterrupt()�͎g��Ȃ��B
            OS_DisableIrqMask(OS_IE_CAMERA);
#else
            // OS_IE_CAMERA��CAMERA���C�u�������n���h�����O����B
#endif
            // �d���̂Œ�D��x�ŁB
            camResult = CAMERA_Init();
            if (camResult != CAMERA_RESULT_SUCCESS) {
                // �Ƃ肠����
                IPL_HALT("CAMERA_Init() error %d\n", camResult);
                OS_TPanic("");
                // NEVER REACH HERE
            }
            initDone = TRUE;

            // SDK default value
            mSelCmr = CAMERA_SELECT_NONE;
            for (i = 0; i < cCameraMax; i++) {
                mCmrCurState[i].mContext   = CAMERA_CONTEXT_A;
                mCmrCurState[i].mFrameRate = CAMERA_FRAME_RATE_15;
            }
            mCurLedBlink = FALSE;
            // I2C�e��ݒ胊�N�G�X�g���M�B
            // ���D��x�X���b�h��Async�����B
            OS_SendMessage(&mI2cMsgQueue, (OSMessage)cMsgI2c_AfterInit, OS_MESSAGE_BLOCK);
        }

        event = OS_WaitEventEx_Or(&mLowerEvent, cEventMaskLower_All, cEventMaskLower_All);
//        IPL_PRINT("event=0x%02x\n", event);

        if (event & cEventMaskLower_Destroy) {
            break;
        }
        if (event & cEventMaskLower_EnableIntrAndPrepareBuffer) {
            // �J�������荞�݂�L���ɂ���B
            setInterrupt(TRUE);
            prepareBuffer();
            // �������͏I������B
            mInitBusyFlag = FALSE;
        }
        if (event & cEventMaskLower_Activate) {
            if (mSelCmr == CAMERA_SELECT_NONE && mNextCmr != CAMERA_SELECT_NONE) {
                // CAMERA_SELECT_NONE��ݒ莞�́ACAM_VSYNC�͔������Ȃ��̂�
                // kick����B
                // �J�����N��������t���[�����̊Ԃ͉摜�����肵�Ȃ��̂ŃL���v�`����~�B
                mCaptureSkipCount = 1;
                OS_SendMessage(&mI2cMsgQueue, (OSMessage)cMsgI2c_Activate, OS_MESSAGE_BLOCK);
            }
        }
        shot = FALSE;
        if ((event & cEventMaskLower_Convert) || mpNextConvBuf) {
            shot = convert((BOOL)(event & cEventMaskLower_Shot));
        }
        if (shot) {
            // �ϊ����Ȃ��ŎB�e�݂̂Ƃ��A
            // �B�e���ɃL���v�`�����p�����邱�Ƃɂ́A�܂��Ή����Ă��Ȃ��B
            mEncodeBusyFlag = TRUE;
            encodeAndSave();
            mEncodeBusyFlag = FALSE;
        }
        if (event & cEventMaskLower_DisableIntrAndDestroyBuffer) {
            // ���̃C�x���g�͘A����������\��������B
            if (mInitFlag) {
                // �J�������荞�݂𖳌��ɂ���B
                setInterrupt(FALSE);
                destroyBuffer();
                MI_StopNDma(cDefaultDmaNo);
                // �ď��������K�v�B
                mInitFlag = FALSE;
            }
        }
    }

    // I2C�X���b�h���I�� (BLOCK)
    OS_SendMessage(&mI2cMsgQueue, (OSMessage)cMsgI2c_Destroy, OS_MESSAGE_BLOCK);
    OS_JoinThread(&mI2cThread);

    CAMERA_End();

    GX_SetDefaultDMA(mGxDma);
}


void Manager::procI2c()
{
    OSMessage msg;
    CAMERAResult camResult;

    while (TRUE) {
        OS_ReceiveMessage(&mI2cMsgQueue, &msg, OS_MESSAGE_BLOCK);
//        IPL_PRINT("I2c msg=%d\n", msg);
        if (!msg) {
            break;
        }

        mI2cBusy = TRUE;

        switch ((u32)msg) {
        case cMsgI2c_AfterInit:
            CAMERA_SetOutputFormat(mCaptureRgbFlag ? CAMERA_OUTPUT_RGB : CAMERA_OUTPUT_YUV);
            CAMERA_SetTrimming(FALSE);
            // Trimming��FALSE�Ȃ̂�TrimmingParams�͖����B
            mShowTransferLines = (int)CAMERA_GET_MAX_LINES(mShowWdt);
            mShotTransferLines = (int)CAMERA_GET_MAX_LINES(mShotWdt);
            // ���͉𑜓x�؂�ւ����ł��Ȃ��̂ŁAmShowWdt����Ȃ���mShotWdt�ŁB
            CAMERA_SetTransferLines(mShotTransferLines);

            camResult = i2cSetupOthers();
            // ���ʂ͂Ƃ������A��s�������͏I������B
            mPreInitBusyFlag = FALSE;
                if (camResult != CAMERA_RESULT_SUCCESS) {
                    // �܂��B
                IPL_HALT("not yet %d\n", camResult);
            // TWL_IPL_FINAL�ł̓X���[����[
                OS_TPanic("");
                }

            break;

        case cMsgI2c_Activate:
            camResult = i2cActivate(mNextCmr);
                if (camResult != CAMERA_RESULT_SUCCESS) {
                    // �܂��B
                IPL_HALT("not yet %d\n", camResult);
            // TWL_IPL_FINAL�ł̓X���[����[
                OS_TPanic("");
                }
            
            break;

        case cMsgI2c_ChangeLedToOn:
        case cMsgI2c_ChangeLedToBlink:
            camResult = i2cLed( ((u32)msg == cMsgI2c_ChangeLedToBlink) );
                if (camResult != CAMERA_RESULT_SUCCESS) {
                    // �܂��B
                IPL_HALT("not yet %d\n", camResult);
            // TWL_IPL_FINAL�ł̓X���[����[
                OS_TPanic("");
                }
            
            break;

        default:
            IPL_HALT("unknown msg %d\n", msg);
            // TWL_IPL_FINAL�ł̓X���[����[
            break;
        }

        mI2cBusy = FALSE;
    }
}


// ���D��x�X���b�h����̂݌Ă΂�邱�Ƃ�z�肵�Ă���̂Ŕr�����䂵�Ȃ��B
CAMERAResult Manager::i2cCommon(u32 type, CAMERASelect sel_cmr)
{
    union {
        CAMERAResult result;
        OSMessage    msg;
    } u = {
        (CAMERAResult)(-1) // CAMERAResult�͔񕉂Ȃ̂ŁA���̒l���Ԃ���������~�X�B
    };
    const volatile cameraState* pNextState        = &mCmrNextState[isOutCamera(sel_cmr)];
    u32 retryCount = 0;

    // cI2cCommon_Activate�̂݁ACAMERA_SELECT_NONE���󂯕t����B
    // �ǂ�ȂƂ����ACAMERA_SELECT_BOTH�͎󂯕t���Ȃ��B
    if (sel_cmr == CAMERA_SELECT_NONE &&
        type != cI2cCommon_Activate) {
        IPL_HALT("error: CAMERA_SELECT_NONE for type %d\n", type);
        return (CAMERAResult)(-2);
    }

    while (TRUE) {
        switch (type) {
        case cI2cCommon_Activate:
            u.result = CAMERA_I2CActivateAsync(sel_cmr, i2cActivateCallback, (void*)sel_cmr);
            break;

        case cI2cCommon_FrameRate:
            u.result = CAMERA_I2CFrameRateAsync(sel_cmr, pNextState->mFrameRate, i2cFrameRateCallback, (void*)sel_cmr);
            break;

        case cI2cCommon_ContextSwitch:
            u.result = CAMERA_I2CContextSwitchAsync(sel_cmr, pNextState->mContext, i2cContextSwitchCallback, (void*)sel_cmr);
            break;

        // I2C����Ȃ�����
        case cI2cCommon_LedOn:
        case cI2cCommon_LedBlink:
            u.result = CAMERA_SetLEDAsync( (type == cI2cCommon_LedBlink), i2cLedCallback, (void*)(type == cI2cCommon_LedBlink));
            break;

        default:
            IPL_HALT("unknown type(1) %d sel_cmr %d\n", type, sel_cmr);
            // TWL_IPL_FINAL�ł̓X���[����[
            break;
        }

        if (u.result == CAMERA_RESULT_BUSY) {
            IPL_PRINT("CAMERA_RESULT_BUSY type %d retry %d\n", type, retryCount);
            retryCount++;
            if (retryCount < cI2cRetryMax) {
                OS_Sleep(cI2cRetryIntervalMsec);
                continue;
            }
        }
        break;
    }

    if (u.result == CAMERA_RESULT_SUCCESS) {
        OS_ReceiveMessage(&mI2cAsyncMsgQueue, &u.msg, OS_MESSAGE_BLOCK);
        if (u.result != CAMERA_RESULT_SUCCESS) {
            IPL_HALT("I2cAsyncCommon error: result %d for type %d sel_cmr %d\n", u.result, type, sel_cmr);
                // TWL_IPL_FINAL�ł̓X���[����[
        }
    } else {
            IPL_HALT("I2cAsyncCommon error: result %d for type %d sel_cmr %d\n", u.result, type, sel_cmr);
                // TWL_IPL_FINAL�ł̓X���[����[
    }

    return u.result;
}


// �����̃J�����ɂ��āA�R���e�L�X�g�؂�ւ��A�����
// �R���e�L�X�g�ŕێ�����Ȃ��ݒ���s���B
// (mNextCmr == CAMERA_SELECT_NONE)�̏ꍇ�ɂ̂݃X�^���o�C�ɂ���B
// ����ȊO�ł̓J�����̖����I�ȋN���͂��Ȃ��B
CAMERAResult Manager::i2cSetupOthers()
{
    CAMERAResult result = CAMERA_RESULT_SUCCESS;
    CAMERASelect sel_cmr;
    u32 i;

    if (mNextCmr == CAMERA_SELECT_NONE) {
        // ��ɃX�^���o�C�ɂ���B
        result = i2cActivate(mNextCmr);
        // �X�^���o�C�ɂ��Ă��A���̐ݒ�𑱂���B
    }

    for (i = 0; i < cCameraMax; i++) {
        sel_cmr = indexToCameraSelect(i);

        if (result == CAMERA_RESULT_SUCCESS) {
            // �ǂ���̃R���e�L�X�g�֐؂�ւ���H
            if (mCmrNextState[i].mContext == CAMERA_CONTEXT_A) {
                // �t���[�����[�g�͌ォ��ύX(�オ�邩������Ȃ�)
                result = i2cContextSwitch(sel_cmr);
                if (result == CAMERA_RESULT_SUCCESS) {
                    result = i2cFrameRate(sel_cmr);
                }
            } else {
                // �t���[�����[�g�͐�ɕύX(�����邩������Ȃ�)
                result = i2cFrameRate(sel_cmr);
                if (result == CAMERA_RESULT_SUCCESS) {
                    result = i2cContextSwitch(sel_cmr);
                }
            }
        }
    }

    return result;
}


#if SDK_VERSION_DATE < 20080630
void Manager::setInterrupt(BOOL enable)
{
    OS_DisableIrqMask(OS_IE_CAMERA);

    CAMERA_SetMasterInterrupt(enable);
    if (enable) {
        // �����ŃJ�������荞�݂͋֎~�ɂȂ��Ă���̂�
        // mIntrPositiveEdge�̑�������荞�݋֎~�ɂ͂��Ȃ��B
        mIntrPositiveEdge = FALSE;
        // almost begin of vblank
        CAMERA_SetVsyncInterrupt(CAMERA_INTR_VSYNC_NEGATIVE_EDGE);
        CAMERA_SetBufferErrorInterrupt(FALSE);
        CAMERA_ClearBuffer();
        OS_ResetRequestIrqMask(OS_IE_CAMERA);
        OS_EnableIrqMask(OS_IE_CAMERA);
    }
}
#else
// OS_IE_CAMERA��CAMERA���C�u�������n���h�����O����B
void Manager::setInterrupt(BOOL enable)
{
    CAMERA_SetVsyncCallback( NULL );
    if (enable) {
        CAMERA_ClearBuffer();
        CAMERA_SetVsyncCallback( proc_camera_vsync_callback );
    }
}
#endif


CAMERAResult Manager::i2cActivate(CAMERASelect sel_cmr)
{
    return i2cCommon(cI2cCommon_Activate, sel_cmr);
}

CAMERAResult Manager::i2cFrameRate(CAMERASelect sel_cmr)
{
    return i2cCommon(cI2cCommon_FrameRate, sel_cmr);
}

CAMERAResult Manager::i2cContextSwitch(CAMERASelect sel_cmr)
{
    return i2cCommon(cI2cCommon_ContextSwitch, sel_cmr);
}

CAMERAResult Manager::i2cLed(BOOL isBlink)
{
    return i2cCommon(isBlink ? cI2cCommon_LedBlink : cI2cCommon_LedOn, CAMERA_SELECT_OUT);
}

void Manager::i2cCommonAsyncCallback(CAMERAResult result, void *arg)
{
#ifdef TWL_IPL_FINAL
#pragma unused(arg)
#endif
    // ���̃R�[���o�b�N���Ă΂��O�Ƀ��b�Z�[�W�o�b�t�@�͋��
    // �Ȃ��Ă���͂��B
    if (!OS_SendMessage(&mI2cAsyncMsgQueue, (OSMessage)result, OS_MESSAGE_NOBLOCK)) {
        IPL_HALT("mI2cAsyncMsgQueue full! arg %d\n", arg);
        // TWL_IPL_FINAL�ł̓X���[����[
    }
}

void Manager::i2cActivateCallback(CAMERAResult result, void *arg)
{
    if (result == CAMERA_RESULT_SUCCESS) {
        mSelCmr = (CAMERASelect)arg;
    }
    i2cCommonAsyncCallback(result, arg);
}

void Manager::i2cFrameRateCallback(CAMERAResult result, void *arg)
{
    if (result == CAMERA_RESULT_SUCCESS) {
        mCmrCurState[isOutCamera((CAMERASelect)arg)].mFrameRate = 
        mCmrNextState[isOutCamera((CAMERASelect)arg)].mFrameRate;
    }
    i2cCommonAsyncCallback(result, arg);
}

void Manager::i2cContextSwitchCallback(CAMERAResult result, void *arg)
{
    if (result == CAMERA_RESULT_SUCCESS) {
        mCmrCurState[isOutCamera((CAMERASelect)arg)].mContext =
        mCmrNextState[isOutCamera((CAMERASelect)arg)].mContext;
    }
    i2cCommonAsyncCallback(result, arg);
}

void Manager::i2cLedCallback(CAMERAResult result, void *arg)
{
    if (result == CAMERA_RESULT_SUCCESS) {
        // �P����(u8)arg�Ƃ���ƁA
        // Warning: integral type is not large enough to hold pointer
        // �ƌ�����B
        mCurLedBlink = (u8)((BOOL)arg);
    }
    i2cCommonAsyncCallback(result, arg);
}


void Manager::prepareBuffer()
{
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
#ifdef TWL_IPL_USE_RED_IPL
    MenuSys()->getRedIplMgr()->UseScratchHeaps(TRUE);
#endif
#endif

    for (u32 i = 0; i < cBufferCount; i++) {
        mpCapBuf[i] =
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
                      new u16 [ cPhotoWidth * cPhotoHeight ];
#else
#ifdef TWL_IPL_USE_RED_IPL
                      (u16*)MenuSys()->getRedIplMgr()->allocCameraCaptureScratchBuffer(cPhotoWidth * cPhotoHeight * sizeof(u16));
#else
                      new( mpHeap ) u16 [ cPhotoWidth * cPhotoHeight ];
#endif
#endif
        IPL_ASSERT( mpCapBuf[i] );
        IPL_PRINT("mpCapBuf[%d]=%x\n", i, mpCapBuf[i]);
    }

    mpEncodeWork =
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
                   new encodeWork;
#else
                   new u8 [ SSP_JpegEncoderGetBufferSize(cPhotoWidth, cPhotoHeight, cEncodeSampling, mEncodeOption) ];
#endif
#else
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
#ifdef TWL_IPL_USE_RED_IPL
                   (encodeWork*)MenuSys()->getRedIplMgr()->allocJpegEncodeScratchBuffer(sizeof(encodeWork));
#else
                   new ( mpHeap ) encodeWork;
#endif
#else
#ifdef TWL_IPL_USE_RED_IPL
                   (u8*)MenuSys()->getRedIplMgr()->allocJpegEncodeScratchBuffer(SSP_JpegEncoderGetBufferSize(cPhotoWidth, cPhotoHeight, cEncodeSampling, mEncodeOption));
#else
                   new ( mpHeap ) u8 [ SSP_JpegEncoderGetBufferSize(cPhotoWidth, cPhotoHeight, cEncodeSampling, mEncodeOption) ];
#endif
#endif
#endif
    IPL_ASSERT( mpEncodeWork );

    IPL_PRINT("mpEncodeWork=%x\n", mpEncodeWork);

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    mpEncodeResult = new u8 [ cEncodeResultSizeMax ]; IPL_ASSERT( mpEncodeResult );
#endif

    // invalidate capture buffers
    DC_FlushAll();
}

void Manager::destroyBuffer()
{
    // �g�p���ł������ɔj������B
    IPL_ASSERT( !mpCaptureBuf );
    IPL_ASSERT( !mpConvBuf );
    IPL_ASSERT( !mpNextConvBuf );
    IPL_ASSERT( !mpShotBuf );

    mpCaptureBuf = NULL;
    mpConvBuf = NULL;
    mpNextConvBuf = NULL;
    mpShotBuf = NULL;

    {
        OSIntrMode enabled = OS_DisableInterrupts();

        for (u32 i = 0; i < cBufferCount; i++) {
            IPL_ASSERT( !mIsCapBufUsed[i] );
            mIsCapBufUsed[i] = FALSE;
            if (mpCapBuf[i]) {
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
                delete [] mpCapBuf[i];
#else
#ifdef TWL_IPL_USE_RED_IPL
                MenuSys()->getRedIplMgr()->freeCameraCaptureScratchBuffer(mpCapBuf[i]);
#else
                delete [] mpCapBuf[i];
#endif
#endif
                mpCapBuf[i] = NULL;
            }
        }

        if (mpEncodeWork) {
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
            delete mpEncodeWork;
#else
            delete [] mpEncodeWork;
#endif
#else
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
#ifdef TWL_IPL_USE_RED_IPL
            MenuSys()->getRedIplMgr()->freeJpegEncodeScratchBuffer(mpEncodeWork);
#else
            delete mpEncodeWork;
#endif
#else
#ifdef TWL_IPL_USE_RED_IPL
            MenuSys()->getRedIplMgr()->freeJpegEncodeScratchBuffer(mpEncodeWork);
#else
            delete [] mpEncodeWork;
#endif
#endif
#endif
            mpEncodeWork = NULL;
        }

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
        if (mpEncodeResult) {
            delete [] mpEncodeResult;
            mpEncodeResult = NULL;
        }
#endif

        OS_RestoreInterrupts(enabled);
    }

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
#ifdef TWL_IPL_USE_RED_IPL
    MenuSys()->getRedIplMgr()->UseScratchHeaps(FALSE);
#endif
#endif

}

// (mpNextConvBuf != NULL) �̏ꍇ�ɂ̂݌Ă΂��B
void Manager::useNextConvBuf()
{
    OSIntrMode enabled = OS_DisableInterrupts();

    if (mpConvBuf) {
        freeCapBufByPtr((u16*)mpConvBuf);
    }
    mpConvBuf = mpNextConvBuf;
    mpNextConvBuf = NULL;
    OS_RestoreInterrupts(enabled);

    // invalidate capture buffers
    DC_FlushAll();

    IPL_PRINT("using NextConvBuf\n");
}

BOOL Manager::convert(BOOL shot)
{
            // �ϊ����Ȃ��ŎB�e�݂̂Ƃ��A
            // �B�e���ɃL���v�`�����p�����邱�Ƃɂ́A�܂��Ή����Ă��Ȃ��B
    if (!mpConvBuf && !mpNextConvBuf) {
        IPL_HALT("(!mpConvBuf && !mpNextConvBuf)\n");
        // TWL_IPL_FINAL�ł̓X���[����[

        return FALSE;
    }

    while (mTexSwap ||
           mTexBank == mVramType[mTarget]) {
        // �ϊ����ʏo�͐��VRAM��LCDC�֊��蓖�Ă��Ȃ��̂ŁA
        // ����sleep�B
        // �܂��X���b�hEND���N�G�X�g�̃`�F�b�N�����Ă��Ȃ��̂ŁA
        // �I�����ɂ͂܂�Ȃ��悤���ӁB
        OS_Sleep(1);
    }
    mNextTexBank = mVramType[mTarget];
    GX_SetBankForLCDC( mNextTexBank );

    if (mpNextConvBuf) {
        // �B�e�f�[�^�����݁B
        // mpConvBuf�͔j������B
        useNextConvBuf();
        shot = TRUE;
    }

    // �L���v�`���ς݃f�[�^��invalidate�͍ς�ł���B
retry:
    Shrink640Yuv422To256Rgb555(const_cast<u16*>(mpConvBuf), mVramAddr[mTarget], mShotHgt, mpDtcmWork->mShrinkBuf);

    // �����ł�mpNextConvBuf���`�F�b�N����B
    if (mpNextConvBuf) {
        // �B�e�f�[�^�����݁B
        // mpConvBuf�͔j������B
        useNextConvBuf();
        shot = TRUE;
        goto retry;
    }

    // �X���b�h�D��x��ʏ��荂������B
    OS_SetThreadPriority(OS_GetCurrentThread(), menu::sys::Object::cThreadPriority_CameraLowerHi);
    {
        OSIntrMode enabled = OS_DisableInterrupts();

        procPostConvert(shot);

        OS_RestoreInterrupts(enabled);
    }

    if (mMode == eMode_BG) {
        mTexSwap = TRUE;
    } else {
        // eMode_Tex
        if (mpGxUserProc) {
            mpGxUserProc(mVramAddr[mTarget], shot);
        }
    }

    mTarget ^= 1;

    OS_SetThreadPriority(OS_GetCurrentThread(), menu::sys::Object::cThreadPriority_CameraLowerLo);

    if (!shot) {
        // invalidate capture buffer for next capture
        DC_FlushAll();
    }

    return shot;
}

/*!
  �J�����}�l�[�W�������S�I������
 */
BOOL Manager::tryDestroy()
{
    // �X���[�v�\�ȏ����܂ő҂B
    if (!tryPrepareToSleep()) {
        return FALSE;
    }

    if (mInitFlag) {
        OS_SignalEvent(&mLowerEvent,
                       cEventMaskLower_DisableIntrAndDestroyBuffer);
        return FALSE;
    } else {
        if (mPreInitFlag) {
            OS_SignalEvent(&mLowerEvent,
                           cEventMaskLower_Destroy);

            if (OS_IsThreadTerminated(&mLowerThread)) {
                // �Đ�s���������K�v�B
                mPreInitFlag = FALSE;
                return TRUE;
            }
            return FALSE;
        }
    }
    return TRUE;
}


void Manager::activate()
{
    startCapture(mNextCmr);
}


void Manager::startCapture(CAMERASelect sel_cmr)
{
    mNextCmr = sel_cmr;
    mNextCaptureFlag = TRUE;
    // CAMERA_Init()�̏������������Ă��Ȃ���������Ȃ��̂�
    // �����Ȃ�I2C�����v���̑��M�͂ł��Ȃ��B
    OS_SignalEvent(&mLowerEvent, cEventMaskLower_Activate);
}


// TRUE���Ԃ�΁A�B�e�v�������������B
// FALSE�Ȃ�A���s�B(�B�e�����r���Ŏ��̎B�e�������Ȃ�)
BOOL Manager::shoot(BOOL keepCapture)
{
    OSIntrMode enabled;

    // �Ƃ肠�����A�˂͖����B
    // �B�e�����ɂ͎��Ԃ�������̂ŁA�B�e����ۑ���(!mCaptureFlag)�Ȃ�B�e�����Ȃ��B
    if (mNextShotFlag || mShotFlag || !mCaptureFlag) {
        return FALSE;
    }

    // �O�J������LED����(�_�����_��)�́A�Ȃ�ׂ�����LED������
    // (�c�����f�荞�ނ̂�h��)���߁A�����ōs�������B
    // �������O�J�����ɐ؂�ւ��v�����o�Ă���r���́A�܂����J�����Ȃ̂łł��Ȃ��B
    // CAMERA_RESULT_ILLEGAL_STATUS (6) ���Ԃ�B
    // �ǂ����݂��A�J������؂�ւ�������̓s���{�P����̂�1�t���[���҂�
    // ���ƂɂȂ邵�A�����ł�LED������s��Ȃ��B

    enabled = OS_DisableInterrupts();

    if (!keepCapture) {
        mNextCaptureFlag = FALSE;
    }
    mNextShotFlag = TRUE;

    OS_RestoreInterrupts(enabled);

    return TRUE;
}

BOOL Manager::tryPrepareToSleep()
{
    stopCapture();

    if (// ��s���������Ȃ�_���B
        (mPreInitFlag && mPreInitBusyFlag) ||
        // ���������Ȃ�_���B
        (mInitFlag && mInitBusyFlag) ||
        // �L���v�`�����Ȃ�_���B
        mCaptureFlag ||
        mNextCaptureFlag ||
        mpCaptureBuf ||
        // �ϊ����Ȃ�_���B
        mpConvBuf ||
        mpNextConvBuf ||
        // �B�e���Ȃ�_���B
        mShotFlag ||
        mNextShotFlag ||
        mpShotBuf ||
        // �V���b�^�[���Đ����Ȃ�_���B
//        (!isFinishedShutterSnd()) ||
        // �ۑ����Ȃ�_���B
        mEncodeBusyFlag ||
        // I2C�������Ȃ�_���B
        mI2cBusy) {
        return FALSE;
    }

    if (mSelCmr != CAMERA_SELECT_NONE ||
        mNextCmr != CAMERA_SELECT_NONE) {
        // �J�����N�����Ȃ�_���B
        mNextCmr = CAMERA_SELECT_NONE;
        return FALSE;
    }

    return TRUE;
}

#include <twl/code32.h>

// pDtcm�ɂ�u32[256*2] (2048 Bytes)�̍�Ɨ̈悪�K�v�ł��B
// ��Ɨ̈��DTCM�ȊO�̃����������蓖�Ă��ꍇ�ɂ�
// �p�t�H�[�}���X���傫���ቺ����\��������܂��B
// little endian only
asm void Manager::Shrink640Yuv422To256Rgb555( register const void* src, register void* dst, register u32 height, register void* pDtcm )
{
#define ORIGINAL_WIDTH 640
#define SHRINK_WIDTH 256 // must be even
#define SHRINK_TMP_BYTES_PER_PIXEL 4
#define SHRINK_TMP_HEIGHT 2
// loopCounters bits:
// d31-d25: (64 x 2) loop counter
// (d31-d24: dst loop counter)
// d23-d22: (2 or 3) loop counter
// d09-d00: height loop counter (0-511) << 1
#define LOOP_COUNTERS_INNER_SHIFT           25
#define LOOP_COUNTERS_DST_SHIFT             24
#define LOOP_COUNTERS_ODD_LINE_SHIFT        22
#define LOOP_COUNTERS_ODD_LINE_MASK_SHIFT   23
#define LOOP_COUNTERS_HEIGHT_BIT_WIDTH      10
// registers
// keep stack frame 8-Bytes aligned
#define SAVE_REGS r3-r11,lr
#define loopCounters height

// keep order for ldm/stm
#define t0    r12
#define t1    lr
#define yuyv0 r4
#define yuyv1 r5
#define yuyv2 r6
#define yuyv3 r7
#define uv    r8

#define CONST_REGS0 r9
#define const_0x01fe01fe      r9

// yuv2rgb
#define y0                    yuyv0
#define y1                    yuyv1
#define rgba                  yuyv2
#define const_0x80008000      const_0x01fe01fe
#define CONST_REGS1 const_H1_402_L1_772,const_Hm0_344_Lm0_714
#define const_H1_402_L1_772   r10
#define const_Hm0_344_Lm0_714 r11

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS}

    ldr     t0, =@constants1
    ldr     const_0x01fe01fe, =0x01fe01fe
    ldmia   t0, {CONST_REGS1}

    // height << 1 and adjust loop counter
    mov     loopCounters, height, lsl #1
    sub     loopCounters, loopCounters, #1

@480:
    // (480/2.5) = 192 loops

@640:
    // (64 x 2) loops

@64:
    ldmia   src!, {yuyv0, yuyv1, yuyv2, yuyv3, uv}

    and     t0, const_0x01fe01fe, yuyv0, lsr #7
    and     t1, const_0x01fe01fe, yuyv1, lsr #7
    and     yuyv0, const_0x01fe01fe, yuyv0, lsl #1
    and     yuyv1, const_0x01fe01fe, yuyv1, lsl #1
    add     yuyv0, yuyv0, yuyv0, lsl #16
    mov     yuyv0, yuyv0, lsr #16
    add     yuyv1, yuyv1, yuyv1, lsr #16
    orr     yuyv0, yuyv0, yuyv1, lsl #16 // ave. y << 1
    add     yuyv1, t0, t1 // ave. uv << 1

    and     t0, const_0x01fe01fe, yuyv2, lsr #7
    and     t1, const_0x01fe01fe, yuyv3, lsr #7
    add     t1, t1, t0, lsr #1
    and     t0, const_0x01fe01fe, uv, lsr #7

    and     yuyv2, const_0x01fe01fe, yuyv2, lsr #16-1
    and     yuyv3, const_0x01fe01fe, yuyv3, lsl #1
    add     yuyv2, yuyv2, yuyv3
    and     uv,    const_0x01fe01fe, uv,    lsl #16+1
    add     yuyv2, yuyv2, uv // ave. y << 1

    add     yuyv3, t1, t0, lsr #1 // ave. uv << 1

    stmia   pDtcm!, {yuyv0, yuyv1, yuyv2, yuyv3}

    adds    loopCounters, loopCounters, #1 << LOOP_COUNTERS_INNER_SHIFT
    bcc     @64

    sub     pDtcm, pDtcm, #SHRINK_TMP_BYTES_PER_PIXEL * SHRINK_WIDTH * SHRINK_TMP_HEIGHT
    ldr     const_0x80008000, =0x80008000
@yuv2rgb_loop:
    ldmia   pDtcm!, {y1, uv}
    ldr     t0, [pDtcm, #SHRINK_WIDTH * SHRINK_TMP_BYTES_PER_PIXEL * 1 - SHRINK_TMP_BYTES_PER_PIXEL * 2]
    ldr     t1, [pDtcm, #SHRINK_WIDTH * SHRINK_TMP_BYTES_PER_PIXEL * 1 - SHRINK_TMP_BYTES_PER_PIXEL * 2 + SHRINK_TMP_BYTES_PER_PIXEL]
    add     y1, y1, t0
    add     uv, uv, t1

    // original formula:
    // R = Y                 + 1.402*(V-128)
    // G = Y - 0.344*(U-128) - 0.714*(V-128)
    // B = Y + 1.772*(U-128)
    //
    // y1: (sum y0y1 of 4 pixels) << 1
    // uv: (sum uv   of 4 pixels) << 1
    //
    // then,
    // R = (sumY/2                            + (1.402/2)*(sumV-128*2*4)) / 4
    // G = (sumY/2 - (0.344/2)*(sumU-128*2*4) - (0.714/2)*(sumV-128*2*4)) / 4
    // B = (sumY/2 + (1.772/2)*(sumU-128*2*4)) / 4
    //
    // or
    // R = sumY/8                            + (1.402/8)*(sumV-128*2*4)
    // G = sumY/8 - (0.344/8)*(sumU-128*2*4) - (0.714/8)*(sumV-128*2*4)
    // B = sumY/8 + (1.772/8)*(sumU-128*2*4)
    //
    // sumY       max is 0xff*2*4 = 0x07f8
    // sumU, sumV max is 0xff*2*4 = 0x07f8
    // 0x07f8 << 20 =0x7f800000

    // sumY /= 8; and normalize to 0x7f800000
    mov     y0, y1, lsl #20
    mov     y1, y1, lsr #16
    mov     y1, y1, lsl #20

    // normalize sumU, sumV and -= 128*2*4
    mov     uv, uv, lsl #5
    eor     uv, uv, const_0x80008000

    // Green
    // -(0.714/8)*sumV
    smulbt  t0, const_Hm0_344_Lm0_714, uv
    // -(0.344/8)*sumU
    smultb  t1, const_Hm0_344_Lm0_714, uv
    mov     rgba, const_0x80008000
    add     t1, t0, t1
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #21 // 0x7c000000 -> 0x000003e0
    ands    t0, t1, #0xfc000000

    // Red
    // (1.402/8)*sumV
    smultt  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #21-16 // 0x7c000000 -> 0x03e00000
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #26 // 0x7c000000 -> 0x0000001f
    ands    t0, t1, #0xfc000000

    // Blue
    // (1.772/8)*sumU
    smulbb  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #26-16 // 0x7c000000 -> 0x001f0000
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #16 // 0x7c000000 -> 0x00007c00
    ands    t0, t1, #0xfc000000
    orrpl   rgba, rgba, t0 // 0x7c000000 

    str     rgba, [dst], #4
    adds    loopCounters, loopCounters, #2 << LOOP_COUNTERS_DST_SHIFT // 2 pixels at once
    bcc     @yuv2rgb_loop

    add     loopCounters, loopCounters, #1 << LOOP_COUNTERS_ODD_LINE_SHIFT
    tst     loopCounters,               #1 << LOOP_COUNTERS_ODD_LINE_MASK_SHIFT
    // skip 1 line
    addne   src, src, #ORIGINAL_WIDTH * sizeof(u16)
    bicne   loopCounters, loopCounters, #1 << LOOP_COUNTERS_ODD_LINE_MASK_SHIFT

    sub     pDtcm, pDtcm, #SHRINK_TMP_BYTES_PER_PIXEL * SHRINK_WIDTH
    ldr     const_0x01fe01fe, =0x01fe01fe

@yuv2rgb_done:
    // (-2.5) << 1
    subs    loopCounters, loopCounters, #5
    bcs     @480

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS}
    bx      lr

@constants1:
    dcd     0x59ba7168 // ((1.402 * 0x4000) << 16) | ((1.772 * 0x4000) & 0xffff)
    dcd     0xe9fcd24e // ((-0.344 * 0x4000) << 16) | ((-0.714 * 0x4000) & 0xffff)

#undef ORIGINAL_WIDTH
#undef SHRINK_WIDTH
#undef SHRINK_TMP_BYTES_PER_PIXEL
#undef SHRINK_TMP_HEIGHT
#undef LOOP_COUNTERS_INNER_SHIFT
#undef LOOP_COUNTERS_DST_SHIFT
#undef LOOP_COUNTERS_ODD_LINE_SHIFT
#undef LOOP_COUNTERS_ODD_LINE_MASK_SHIFT
#undef LOOP_COUNTERS_HEIGHT_BIT_WIDTH

#undef SAVE_REGS
#undef loopCounters
#undef t0
#undef t1

#undef yuyv0 
#undef yuyv1 
#undef yuyv2 
#undef yuyv3 
#undef uv    

#undef CONST_REGS0 
#undef const_0x01fe01fe      

#undef y0
#undef y1
#undef rgba
#undef const_0x80008000
#undef CONST_REGS1
#undef const_H1_402_L1_772
#undef const_Hm0_344_Lm0_714
}


// �܂�480 lines�ł͂Ȃ��A156*3 lines�ł����������Ă��Ȃ��B
// �����؂�̂ĂŖ��Ȃ��̂��A�؂�グ��̂��H
// pDtcm�ɂ�u32[208*3] (2496 Bytes)�̍�Ɨ̈悪�K�v�ł��B
// ��Ɨ̈��DTCM�ȊO�̃����������蓖�Ă��ꍇ�ɂ�
// �p�t�H�[�}���X���傫���ቺ����\��������܂��B
// little endian only
asm void Manager::Shrink640Yuv422To208Rgb555( register const void* src, register void* dst, register u32 height, register void* pDtcm )
{
#define ORIGINAL_WIDTH 640
#define SHRINK_WIDTH 208 // must be even
#define SHRINK_TMP_BYTES_PER_PIXEL 4
#define SHRINK_TMP_HEIGHT 3
// loopCounters bits:
// d31-d28: (6 + 7) loop counter
// d27-d24: (6 + 7) x 8 loop counter
// (d31-d24: dst loop counter)
// d23-d22: dtcm line loop counter
// d21-d09: unused
// d08-d00: height loop counter (0-511)
#define LOOP_COUNTERS_INNER_SHIFT     28
#define LOOP_COUNTERS_LINE_SHIFT      24
#define LOOP_COUNTERS_DST_SHIFT       24
#define LOOP_COUNTERS_DTCM_LINE_SHIFT 22
#define LOOP_COUNTERS_DTCM_LINE_MASK  0x3
#define LOOP_COUNTERS_HEIGHT_BIT_WIDTH 9
// registers
// keep stack frame 8-Bytes aligned
#define SAVE_REGS r3-r11,lr
#define loopCounters height

// keep order for ldm/stm
#define t0    r12
#define t1    lr
#define yuyv0 r4
#define yuyv1 r5
#define yuyv2 r6
#define yuyv3 r7
#define uv    r8

#define CONST_REGS0 r9,r10,r11
#define const_0x01fe01fe      r9
#define const_0x00001c73      r10
#define const_0x00000e39      r11

// register pair (be careful about order)
#define YYUV_REGS0 yuyv1,uv
#define YYUV_REGS1 yuyv1,t1

// yuv2rgb
#define y0                    yuyv0
#define y1                    yuyv1
#define rgba                  yuyv2
#define CONST_REGS1 const_0x000717c2,const_0x80008000,const_H1_402_L1_772,const_Hm0_344_Lm0_714
#define const_0x000717c2      yuyv3
#define const_0x80008000      const_0x01fe01fe
#define const_H1_402_L1_772   const_0x00001c73
#define const_Hm0_344_Lm0_714 const_0x00000e39

    ldr     t0, =@constants0

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS}

    ldmia   t0, {CONST_REGS0}

    // dtcm line loop
    // loopCounters and height may differ
    add     loopCounters, height, #SHRINK_TMP_HEIGHT << LOOP_COUNTERS_DTCM_LINE_SHIFT

@480:
    // (6 + 7) x 8 loops = 8,9,a,b,c,d,e,f
    // adjust (6 + 7) loop counter, from minus
    add     loopCounters, loopCounters, #(8 << LOOP_COUNTERS_LINE_SHIFT) - (1 << LOOP_COUNTERS_INNER_SHIFT)

@640:
    // 6 loops
    add     loopCounters, loopCounters, #6 << LOOP_COUNTERS_INNER_SHIFT
@6:
    ldmia   src!, {yuyv0, yuyv1, yuyv2}

    and     t0, const_0x01fe01fe, yuyv0, lsr #7
    and     t1, const_0x01fe01fe, yuyv1, lsr #7
    add     t0, t0, t1
    and     t1, const_0x01fe01fe, yuyv2, lsr #7
    add     uv, t0, t1 // sum uv << 1

    and     yuyv0, const_0x01fe01fe, yuyv0, lsl #1
    and     yuyv1, const_0x01fe01fe, yuyv1, lsl #1
    add     yuyv0, yuyv0, yuyv0, lsl #16
    add     yuyv1, yuyv1, yuyv0, lsr #16 // sum y0 << 1

    and     yuyv2, const_0x01fe01fe, yuyv2, lsl #1
    add     yuyv2, yuyv2, yuyv2, lsr #16
    add     yuyv1, yuyv1, yuyv2, lsl #16 // sum y1 << 1
    stmia   pDtcm!, {YYUV_REGS0}

    subs    loopCounters, loopCounters, #1 << LOOP_COUNTERS_INNER_SHIFT
    bpl     @6

    // 7 loops
    add     loopCounters, loopCounters, #7 << LOOP_COUNTERS_INNER_SHIFT
    ldmia   src!, {yuyv0, yuyv1, yuyv2, yuyv3}
    and     uv,    const_0x01fe01fe, yuyv0, lsr #7
    and     yuyv0, const_0x01fe01fe, yuyv0, lsl #1
@7:
    and     t0, const_0x01fe01fe, yuyv1, lsr #7
    add     t0, t0, uv, lsr #1
    and     t1, const_0x01fe01fe, yuyv2, lsr #7
    add     t0, t0, t1
    and     uv, const_0x01fe01fe, yuyv3, lsr #7
    add     t1, t0, uv, lsr #1 // sum uv << 1

    and     yuyv1, const_0x01fe01fe, yuyv1, lsl #1
    add     yuyv1, yuyv1, yuyv0, lsr #16
    add     yuyv1, yuyv1, yuyv1, lsl #16
    mov     yuyv1, yuyv1, lsr #16 // sum y0 << 1

    and     yuyv2, const_0x01fe01fe, yuyv2, lsl #1
    and     yuyv0, const_0x01fe01fe, yuyv3, lsl #1
    add     yuyv2, yuyv2, yuyv0, lsl #16
    add     yuyv2, yuyv2, yuyv2, lsr #16
    orr     yuyv1, yuyv1, yuyv2, lsl #16 // sum y1 << 1

    stmia   pDtcm!, {YYUV_REGS1}
    subs    loopCounters, loopCounters, #1 << LOOP_COUNTERS_INNER_SHIFT
    ldmplia src!, {yuyv1, yuyv2, yuyv3}
    bpl     @7

    adds    loopCounters, loopCounters, #1 << LOOP_COUNTERS_LINE_SHIFT
    bmi     @640

    sub     loopCounters, loopCounters, #1 << LOOP_COUNTERS_DTCM_LINE_SHIFT
    tst     loopCounters, #LOOP_COUNTERS_DTCM_LINE_MASK << LOOP_COUNTERS_DTCM_LINE_SHIFT
    bne     @yuv2rgb_done

    ldr     t0, =@constants1
    add     loopCounters, loopCounters, #SHRINK_TMP_HEIGHT << LOOP_COUNTERS_DTCM_LINE_SHIFT
    ldmia   t0, {CONST_REGS1}

    add     loopCounters, loopCounters, #(0x100 - SHRINK_WIDTH) << LOOP_COUNTERS_DST_SHIFT
    sub     pDtcm, pDtcm, #SHRINK_TMP_BYTES_PER_PIXEL * SHRINK_WIDTH * SHRINK_TMP_HEIGHT
@yuv2rgb_loop:
    ldmia   pDtcm!, {y0, uv}
    ldr     t0, [pDtcm, #SHRINK_WIDTH * SHRINK_TMP_BYTES_PER_PIXEL * 1 - SHRINK_TMP_BYTES_PER_PIXEL * 2]
    ldr     t1, [pDtcm, #SHRINK_WIDTH * SHRINK_TMP_BYTES_PER_PIXEL * 1 - SHRINK_TMP_BYTES_PER_PIXEL * 2 + SHRINK_TMP_BYTES_PER_PIXEL]
    add     y0, y0, t0
    ldr     t0, [pDtcm, #SHRINK_WIDTH * SHRINK_TMP_BYTES_PER_PIXEL * 2 - SHRINK_TMP_BYTES_PER_PIXEL * 2]
    add     uv, uv, t1
    ldr     t1, [pDtcm, #SHRINK_WIDTH * SHRINK_TMP_BYTES_PER_PIXEL * 2 - SHRINK_TMP_BYTES_PER_PIXEL * 2 + SHRINK_TMP_BYTES_PER_PIXEL]
    add     y0, y0, t0
    add     uv, uv, t1

    // original formula:
    // R = Y                 + 1.402*(V-128)
    // G = Y - 0.344*(U-128) - 0.714*(V-128)
    // B = Y + 1.772*(U-128)
    //
    // y0: (sum y0y1 of 9 pixels) << 1
    // uv: (sum uv   of 9 pixels) << 1
    //
    // then,
    // R = (sumY/2                            + (1.402/2)*(sumV-128*2*9)) / 9
    // G = (sumY/2 - (0.344/2)*(sumU-128*2*9) - (0.714/2)*(sumV-128*2*9)) / 9
    // B = (sumY/2 + (1.772/2)*(sumU-128*2*9)) / 9
    //
    // or
    // R = sumY/18                             + (1.402/18)*(sumV-128*2*9)
    // G = sumY/18 - (0.344/18)*(sumU-128*2*9) - (0.714/18)*(sumV-128*2*9)
    // B = sumY/18 + (1.772/18)*(sumU-128*2*9)
    //
    // sumY       max is 0xff*2*9 = 0x11ee
    // sumU, sumV max is 0xff*2*9 = 0x11ee
    // 0x11ee*0x717c2=0x7f8003fc

    // sumY /= 2*9; and normalize to 0x7f800000
    mov     t0, y0, lsr #16
    mul     y1, const_0x000717c2, t0
    eor     t0, y0, t0, lsl #16
    mul     y0, const_0x000717c2, t0

    // sumV -= 128*2*9
    sub     uv, uv, #0x900 << 16

    // Red
    // (1.402/18)*sumV
    smultt  t1, const_H1_402_L1_772, uv
    mov     rgba, const_0x80008000
    // normalize
    mov     t1, t1, lsl #8

    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #26 // 0x7c000000 -> 0x0000001f
    ands    t0, t1, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #26-16 // 0x7c000000 -> 0x001f0000

    // Green
    // -(0.714/18)*sumV
    smulbt  t0, const_Hm0_344_Lm0_714, uv
    // sumU -= 128*2*9
    sub     uv, uv, #0x900
    // -(0.344/18)*sumU
    smultb  t1, const_Hm0_344_Lm0_714, uv
    // normalize
    mov     t0, t0, lsl #8
    add     t1, t0, t1, lsl #8
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #21 // 0x7c000000 -> 0x000003e0
    ands    t0, t1, #0xfc000000

    // Blue
    // (1.772/18)*sumU
    smulbb  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #21-16 // 0x7c000000 -> 0x03e00000
    // normalize
    mov     t1, t1, lsl #8
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #16 // 0x7c000000 -> 0x00007c00
    ands    t0, t1, #0xfc000000
    orrpl   rgba, rgba, t0 // 0x7c000000 -> 0x7c000000

    str     rgba, [dst], #4
    adds    loopCounters, loopCounters, #2 << LOOP_COUNTERS_DST_SHIFT // 2 pixels at once
    bcc     @yuv2rgb_loop

    ldr     t0, =@constants0
    add     dst, dst, #(HW_LCD_WIDTH - SHRINK_WIDTH) * sizeof(u16)
    sub     pDtcm, pDtcm, #SHRINK_TMP_BYTES_PER_PIXEL * SHRINK_WIDTH
    ldmia   t0, {CONST_REGS0}

@yuv2rgb_done:
    sub     loopCounters, loopCounters, #1
    movs    t0, loopCounters, lsl #32 - LOOP_COUNTERS_HEIGHT_BIT_WIDTH
    bne     @480

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS}
    bx      lr

@constants0:
    dcd     0x01fe01fe
    dcd     0x00001c73
    dcd     0x00000e39

@constants1:
    dcd     0x000717c2
    dcd     0x80008000
    dcd     0x04fc064c // (((1.402/18) * 0x4000) << 16) | (((1.772/18) * 0x4000) & 0xffff)
    dcd     0xfec7fd77 // (((-0.344/18) * 0x4000) << 16) | (((-0.714/18) * 0x4000) & 0xffff)


#undef ORIGINAL_WIDTH
#undef SHRINK_WIDTH
#undef SHRINK_TMP_BYTES_PER_PIXEL
#undef SHRINK_TMP_HEIGHT
#undef LOOP_COUNTERS_INNER_SHIFT
#undef LOOP_COUNTERS_LINE_SHIFT
#undef LOOP_COUNTERS_DST_SHIFT
#undef LOOP_COUNTERS_DTCM_LINE_MASK
#undef LOOP_COUNTERS_DTCM_LINE_SHIFT
#undef LOOP_COUNTERS_HEIGHT_BIT_WIDTH

#undef SAVE_REGS
#undef loopCounters
#undef t0
#undef t1

#undef yuyv0 
#undef yuyv1 
#undef yuyv2 
#undef yuyv3 
#undef uv    

#undef CONST_REGS0 
#undef const_0x01fe01fe      
#undef const_0x00001c73      
#undef const_0x00000e39

#undef YYUV_REGS0
#undef YYUV_REGS1

#undef y0
#undef y1
#undef rgba
#undef CONST_REGS1
#undef const_0x000717c2
#undef const_0x80008000
#undef const_H1_402_L1_772
#undef const_Hm0_344_Lm0_714
}

// width��16�̔{��(16..640)�Aheight��2�̔{��(2..480)�ł���K�v������܂��B
// src�͍Œ�4�o�C�g�A���C�������g�ł��B
// pYCbCrBuf���Œ�4�o�C�g�A���C�������g�ł����A�e�f�o�C�X�Ƃ̘A�g��
// ������32�o�C�g�A���C�������g���Ă��������B
// pYCbCrBuf�́AY�����ACb�����ACr�����ꊇ�Ŋm�ۂ��Ă��������B�T�C�Y��
//   Y����: width*height*sizeof(u16) Bytes
//   Cb����: ((width*height)/4)*sizeof(u16)) Bytes
//   Cr����: ((width*height)/4)*sizeof(u16)) Bytes
// �ȏ�̍��v (width*height*3/2)*sizeof(u16) Bytes�ł��B
// (�܂茳�摜�f�[�^�T�C�Y��1.5�{)
// pDtcm_�ɂ�u32[16+4+4] (96 Bytes)�̍�Ɨ̈悪�K�v�ł��B
// ��Ɨ̈��DTCM�ȊO�̃����������蓖�Ă��ꍇ�ɂ�
// �p�t�H�[�}���X���傫���ቺ����\��������܂��B
// ����src��CAMERA�f�o�C�X��YUV�ŃL���v�`�������`��
// (Y 8bit,U 8bit,Y 8bit,V 8bit)�ł��B
// �o��pYCbCrBuf��DSP JPEG�G���R�[�_�ւ̓��͌`��
// (Y,Y,Y,Y,..., U,..., V,..., �e16bit)�ł��B
// �����Ńf�[�^�L���b�V���̐���͍s���Ă��Ȃ����߁A���ʂ�
// DSP���֓n���O��Flush���邢��Store����K�v������܂��B
asm void Manager::Convert8bitYuv422To16bitYuv420( register const u32* src, register u16* pYCbCrBuf, register u32 width, register u32 height, void* pDtcm_ )
{
#define BLOCK_WIDTH 16
// registers
// keep stack frame 8-Bytes aligned
#define SAVE_REGS r3-r11,lr // 10 registers = 40 Bytes
#define STACK_OFFSET_PYCBCRBUF 0  // initial value is r3
#define STACK_OFFSET_PDTCM     40
#define pCbBuf      pYCbCrBuf
#define widthHeight width
#define pDtcm       height
#define pDstY       pDtcm
#define pDstCr      pDtcm

// keep order for ldm/stm/ldrd/strd
#define FLUSH_REGS    r4-r11
#define FLUSH_REGS_CB r4,r6,r8,r10
#define FLUSH_REGS_CR r5,r7,r9,r11
#define yuyv0            r4
#define yuyv1            r5
#define yuyv2            r6
#define yuyv3            r7
#define t0               r8
#define t1               r9
#define const_0x00ff00ff r11

#define loopHeightWidth  r12
#define pDtcmStop        lr

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS}
    // pDstY initial value is pYCbCrBuf
    str     pYCbCrBuf, [sp, #STACK_OFFSET_PYCBCRBUF]

    mul     t0, width, height
    // setup widthHeight before breaking pDtcm
    orr     widthHeight, height, width, lsl #16
    add     pCbBuf, pYCbCrBuf, t0, lsl #1

    ldr     pDtcm, [sp, #STACK_OFFSET_PDTCM]
    add     pDtcmStop, pDtcm, #BLOCK_WIDTH * sizeof(u16)

    mov     loopHeightWidth, widthHeight, lsl #16

@480:
    add     loopHeightWidth, loopHeightWidth, widthHeight, lsr #16

@640:
    ldr     const_0x00ff00ff, =0x00ff00ff

@blockLoop:
    add     t0, src, widthHeight, lsr #16-1 // +1(odd) line
    ldmia   t0,   {yuyv2, yuyv3}
    ldmia   src!, {yuyv0, yuyv1}

    // even line Y
    and     t0, yuyv0, const_0x00ff00ff
    and     t1, yuyv1, const_0x00ff00ff
    strd    t0, [pDtcm, #BLOCK_WIDTH * sizeof(u16)]

#if 0
    // even line U
    and     yuyv0, const_0x00ff00ff, yuyv0, lsl #8
    and     t0,    const_0x00ff00ff, yuyv1, lsl #8
    orr     yuyv0, t0, yuyv0, lsr #16

    // odd line V
    and     yuyv1, const_0x00ff00ff, yuyv2, lsr #24
    and     t0,    const_0x00ff00ff, yuyv3, lsr #24
    orr     yuyv1, yuyv1, t0, lsl #16
#else
    // even column ave. uv (V0 U0)
    and     yuyv0, const_0x00ff00ff, yuyv0, lsr #8
    and     t0,    const_0x00ff00ff, yuyv2, lsr #8
    add     yuyv0, yuyv0, t0
    and     yuyv0, const_0x00ff00ff, yuyv0, lsr #1

    // odd column ave. uv (V1 U1)
    and     yuyv1, const_0x00ff00ff, yuyv1, lsr #8
    and     t0,    const_0x00ff00ff, yuyv3, lsr #8
    add     yuyv1, yuyv1, t0
    and     yuyv1, const_0x00ff00ff, yuyv1, lsr #1

    // V0 U0 => U1 U0
    // V1 U1    V1 V0
    eor     t0, yuyv0, yuyv1, ror #16 // V0^U1 U0^V1
    eor     yuyv1, yuyv1, t0, lsr #16 // V1    V0
    eor     yuyv0, t0, yuyv1, ror #16 // U1    U0
#endif

    strd    yuyv0, [pDtcm, #BLOCK_WIDTH * 2 * sizeof(u16)]

    // odd  line Y
    and     yuyv2, yuyv2, const_0x00ff00ff
    and     yuyv3, yuyv3, const_0x00ff00ff

    stmia   pDtcm!, {yuyv2, yuyv3}
    cmp     pDtcm, pDtcmStop
    bne     @blockLoop

// �����������ǂł��邩��
    ldr     pDstY, [sp, #STACK_OFFSET_PYCBCRBUF]
    ldmdb   pDtcmStop, {FLUSH_REGS}
    add     pDstY, pDstY, widthHeight, lsr #16-1 // +1(odd) line
    stmia   pDstY, {FLUSH_REGS}
    sub     pDstY, pDstY, widthHeight, lsr #16-1 // -1(even) line

    ldmia   pDtcmStop!, {FLUSH_REGS}
    stmia   pDstY!, {FLUSH_REGS}
    smultb  t0, widthHeight, widthHeight
    str     pDstY, [sp, #STACK_OFFSET_PYCBCRBUF]

    add     pDstCr, pCbBuf, t0, lsr #1
    ldmia   pDtcmStop, {FLUSH_REGS}
    stmia   pDstCr, {FLUSH_REGS_CR}
    stmia   pCbBuf!, {FLUSH_REGS_CB}

    sub     pDtcmStop, pDtcmStop, #8 * sizeof(u32)
    sub     pDtcm, pDtcmStop, #BLOCK_WIDTH * sizeof(u16)

    sub     loopHeightWidth, loopHeightWidth, #BLOCK_WIDTH
    movs    t0, loopHeightWidth, lsl #16
    bne     @640

    ldr     t0, [sp, #STACK_OFFSET_PYCBCRBUF]
    subs    loopHeightWidth, loopHeightWidth, #2 << 16 // 2 lines/loop
    addne   t0, t0, widthHeight, lsr #16-1 // 2 lines/loop
    strne   t0, [sp, #STACK_OFFSET_PYCBCRBUF]
    addne   src, src, widthHeight, lsr #16-1 // 2 lines/loop
    bne     @480

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS}
    bx      lr

#undef BLOCK_WIDTH
#undef SAVE_REGS
#undef STACK_OFFSET_PYCBCRBUF
#undef STACK_OFFSET_PDTCM
#undef pCbBuf      
#undef widthHeight 
#undef pDtcm       
#undef pDstY       
#undef pDstCr      

#undef FLUSH_REGS    
#undef FLUSH_REGS_CB 
#undef FLUSH_REGS_CR 
#undef yuyv0            
#undef yuyv1            
#undef yuyv2            
#undef yuyv3            
#undef t0               
#undef t1               
#undef const_0x00ff00ff 

#undef loopHeightWidth  
#undef pDtcmStop        
}

#include <twl/codereset.h>


void Manager::procVBlankIntr()
{
    if (mMode == eMode_BG) {

    } else {
        // eMode_Tex
        if (mTexSwap) {
            GX_DisableBankForTex();
            GX_SetBankForTex( GXVRamTex( mNextTexBank ) );
            mTexBank = mNextTexBank;
            mTexSwap = FALSE;
            mFirstPictureReady = TRUE;
        } else {
            // cmn�ŏ㏑�������\��������̂Ŗ��t���[���Đݒ�B
            // GX_DisableBankForTex();
            GX_SetBankForTex( GXVRamTex( mTexBank ) );
        }
    }
}


void Manager::procCameraIntr()
{
    u32 err = 0;
    u32 msg = cMsgI2c_Destroy; // ���̂܂܂̒l�����M���ꂽ��v���O�����~�X�B

#if SDK_VERSION_DATE < 20080630
    if (mIntrPositiveEdge) {
        CAMERA_StopCapture();
        mIntrPositiveEdge = FALSE;
        CAMERA_SetVsyncInterrupt(CAMERA_INTR_VSYNC_NEGATIVE_EDGE);
    }
#else
    // intr edge��CAMERA���C�u�������n���h�����O����B
#endif

    if (CAMERA_GetErrorStatus()) {
        // error
        CAMERA_StopCapture();
        CAMERA_ClearBuffer();
        MI_StopNDma(cDefaultDmaNo);
        err = 1;
        IPL_PRINT("cameraIntr: status error\n");
    } else {
        if (MI_IsNDmaBusy(cDefaultDmaNo)) {
            // error
            // �Ƃ͌����؂�Ȃ��H�Ō��NDMA�]������������Ȃ��H
            MI_StopNDma(cDefaultDmaNo);
            err = 2;
            IPL_PRINT("cameraIntr: NDMA busy\n");
        }
    }

    if (err) {
        // �L���v�`�������s�����̂ŁA���g���C����B
        if (mpCaptureBuf) {
            freeCapBufByPtr((u16*)mpCaptureBuf);
            mpCaptureBuf = NULL;
        }
        mCaptureRetryCount++;
        if (mCaptureRetryCount < cCaptureRetryMax) {
            // retry
        } else {
            // fatal error
            IPL_HALT("capture error too much!\n");
            // TWL_IPL_FINAL�ł̓X���[����[
        }
    } else {
        mCaptureRetryCount = 0;
        procPostCapture();
    }

    // I2C�������͉������Ȃ��B
    if (mI2cBusy) {
        return;
    }

    // �L���v�`��<=>�B�e�ŉ𑜓x��ύX��������ɂ��K�v�H
    if (mCaptureSkipCount) {
        mCaptureSkipCount--;
        return;
    }

    if (mSelCmr != mNextCmr) {
        // �J������؂�ւ��邽��I2C�����v�����o���B
        // �O�J������LED���������ɍs���B
        msg = cMsgI2c_Activate;
        // �J�����؂�ւ�������t���[�����̊Ԃ͉摜�����肵�Ȃ��̂ŃL���v�`����~�B
        // mSelCmr��NONE�̏ꍇ�ɂ͂����֗��Ȃ��B(�J�������荞�݂��������Ȃ�����)
        mCaptureSkipCount = 1;
        goto sendI2cRequest;
    }

    // �L���v�`�����ƎB�e���ŉ𑜓x��t�H�[�}�b�g���قȂ�ꍇ�A
    // ������I2C�����v�����o���B
    // �܂��B

    // �O�J������LED����B
    if (!mLedKeepFlag && (mSelCmr == CAMERA_SELECT_OUT)) {
        if (mNextShotFlag) {
            if (!mCurLedBlink) {
                // �B�e�Ȃ̂œ_�����_�łɂ���B
                // �V���b�^�[�X�s�[�h�𑬂����邽�ߊ��荞�ݘ_���؂�ւ��B
                // ��NDMA busy�G���[�ɂȂ��Ă��܂��̂ŁA���荞�ݘ_���͐؂�ւ��Ȃ��B
                // �c�O�Ȃ���LED���f�荞�ނ��Ƃ�����H(�}�C�R�����Â������H)

                // VBlank      Time   Intr Line Lines Ticks
                //    272    4.5640 CAMERA   13     0    26 // negative edge
                //    273    4.5754      V  192     0    17
                //    274    4.5921      V  192     0     6
                //    275    4.6089      V  192     1    33
                //    275    4.6140 CAMERA   10     1    20 // negative edge (+50.0ms from negative edge)
                //    275    4.6144 PXI_NE   16     1    19 // SetLED�֌W    ( +0.4ms from negative edge)
                //    275    4.6178 CAMERA   70     1    20 // positive edge ( +3.8ms from negative edge)
                //    276    4.6256      V  192     0    11
                //    277    4.6423      V  192     0    20
                //    277    4.6474    TM0    9     0     9
                //    278    4.6590      V  192     0     5
                //    278    4.6640 CAMERA    8     9   282 // negative edge (+50.0ms from negative edge)
                // *Warning: taking over 100 ticks (over 3 lines)
                msg = cMsgI2c_ChangeLedToBlink;
                goto sendI2cRequest;
//                goto toPositiveEdgeAndSendI2cRequest;
            }
        } else if (!mShotFlag /* mCaptureFlag */ ) {
            // �B�e�ł͂Ȃ��̂œ_�Ł��_���ɂ���B
            // �������u�L���v�`���Ȃ̂�(mCaptureFlag)�v�ɂ���ƁA
            // �G���R�[�h�A�Z�[�u���ɂ�mCaptureFlag�������Ȃ��̂ŁA
            // �_�Ŏ��Ԃ������Ȃ�B
            if (mCurLedBlink) {
                // ��ʍX�V�X�s�[�h�𑬂����邽�ߊ��荞�ݘ_���؂�ւ��B
                // VBlank      Time   Intr Line Lines Ticks
                //    281    4.7140 CAMERA    6     0    10 // negative edge
                //    282    4.7259      V  192     0     7
                //    283    4.7426      V  192     0    15
                //    284    4.7593      V  192     0     5
                //    284    4.7641 CAMERA    4     0    18 // negative edge (+50.1ms from negative edge)
                //    284    4.7645 PXI_NE   11     0    19 // SetLED�֌W    ( +0.4ms from negative edge)
                //    284    4.7678 CAMERA   63     1     7 // positive edge ( +3.7ms from negative edge)
                //    284    4.7725    TM0  137     0     8
                //    285    4.7760      V  192     0    12
                //    286    4.7927      V  192     0    15
                //    287    4.8094      V  192     0    10
                //    287    4.8140 CAMERA    1     1    14 // negative edge (+49.9ms from negative edge)
                msg = cMsgI2c_ChangeLedToOn;
                goto sendI2cRequest;
//                goto toPositiveEdgeAndSendI2cRequest;
            }
        }
    }

    if (!err) {
        mCaptureFlag = mNextCaptureFlag;
        mShotFlag = mNextShotFlag;
        // �Ƃ肠�����P���ۑ��̂݁B
        mNextShotFlag = FALSE;
    }

    // ���g���C�񐔏���ɒB�����ꍇ�̋���stop�͕K�v�H
    // �܂��B

    if (mSelCmr != CAMERA_SELECT_NONE &&
        (mCaptureFlag || mShotFlag)) {
        IPL_ASSERT( !mpCaptureBuf );
        mpCaptureBuf = procPreCapture();
        if (mpCaptureBuf) {
            // �����������A���^�C����mShot{Wdt,Hgt}<=>mShow{Wdt,Hgt}�̕ϊ����Ă����������Ȃ��ˁH
            // transferlines���Đݒ肵�Ȃ��ƁB
            // CAMERA_GetBytesAtOnce()�́Areg_CAM_CNT�𒼐ړǂ�ł���B
            // CAM_CNT�́A�J�����C�l�[�u�����̏������݂͖�������邩��A
            // ���҂����l���m���ɓ�����ۏ؂͂Ȃ��Ǝv����B
            // �ꉞ������CAMERA V�u�����N���荞�݃n���h��������A���҂����l��
            // �����Ă��Ȃ���Γ��삻�̂��̂����������Ȃ邪�A���̏ꍇ�ł�
            // DMA�Ŕ͈͊O�̃�������j�󂵂Ȃ��悤�A�v�Z�ŋ��߂����l
            // CAMERA_GET_LINE_BYTES(mShotWdt) * mTransferLines ���w��B
#if 0
// <twl/camera/ARM9/transfer.h>:
// static inline void CAMERA_DmaRecvAsync(u32 dmaNo, void *dest, u32 unit, u32 length, MINDmaCallback callback, void* arg)
// {
//     MI_Camera_NDmaRecvAsync( dmaNo, dest, unit/4, length, FALSE, callback, arg );
// }

            // SDK default NDMA parameter
            CAMERA_DmaRecvAsync(cDefaultDmaNo, (u16*)mpCaptureBuf,
                                CAMERA_GET_LINE_BYTES(mShotWdt) * mShotTransferLines,
                                CAMERA_GET_FRAME_BYTES(mShotWdt, mShotHgt),
                                NULL, NULL);
#else
// <twl/mi/common/dma.h>:
// void    MI_Camera_NDmaRecvAsync(u32 ndmaNo, void *dest, u32 unit, u32 size, BOOL contSw, MINDmaCallback callback, void* arg );
// void    MI_Camera_NDmaRecvAsyncEx(u32 ndmaNo, void *dest, u32 size, BOOL contSw, MINDmaCallback callback, void* arg, const MINDmaConfig *config );

            mDmaConfig.wordCount = (CAMERA_GET_LINE_BYTES(mShotWdt) * mShotTransferLines) / sizeof(u32);
            MI_Camera_NDmaRecvAsyncEx(cDefaultDmaNo, (u16*)mpCaptureBuf,
                                      CAMERA_GET_FRAME_BYTES(mShotWdt, mShotHgt),
                                      FALSE,
                                      NULL, NULL,
                                      &mDmaConfig);
#endif

            CAMERA_StartCapture();
        } else {
            IPL_PRINT("no memory for mpCaptureBuf\n");
            CAMERA_StopCapture();

            // �G���[���g���C�J�E���g�𑝂₷�H
            // �P��prepareBuffer()�ł��Ă��Ȃ�������������Ȃ��B
        }
    } else {
        CAMERA_StopCapture();
        if (mOneShotFlag) {
            // �J�������~������B
            // I2C�����v�����o���B
            // mNextCmr�͏㏑�������B
            mNextCmr = CAMERA_SELECT_NONE;
            msg = cMsgI2c_Activate;
            goto sendI2cRequest;
        }
    }

    return;

#if SDK_VERSION_DATE < 20080630
toPositiveEdgeAndSendI2cRequest:
    CAMERA_StopCapture();
    mIntrPositiveEdge = TRUE;
    CAMERA_SetVsyncInterrupt(CAMERA_INTR_VSYNC_POSITIVE_EDGE);
#else
    // intr edge��CAMERA���C�u�������n���h�����O����B
#endif
// thru
sendI2cRequest:
    CAMERA_StopCapture();
    if (!OS_SendMessage(&mI2cMsgQueue, (OSMessage)msg, OS_MESSAGE_NOBLOCK)) {
        // ���g���C���K�v�B
    } else {
        // �O�̂��߁A������I2C�������t���O�����ĂĂ����B
        mI2cBusy = TRUE;

        // �G���[���g���C�J�E���^�𑝂₷�K�v�����肻���B
    }
}


u16* Manager::procPreCapture()
{
    return allocCapBuf();
}


void Manager::procPostCapture()
{
    if (mpCaptureBuf) {
        if (mpConvBuf) {
            // �O���YUV2RGB�ϊ����������Ă��Ȃ��B
#ifndef TWL_IPL_FINAL
            static u32 count;
            IPL_PRINT("mConvBusy! %d, nextp %s\n", count++, mpNextConvBuf ? "exist" : "empty");
#endif
            // �B�e�f�[�^�ŁA������ϊ��\��̃o�b�t�@�����݂��Ȃ����
            // ����ϊ��\��Ƃ���B
            if (mShotFlag && !mpNextConvBuf) {
                mpNextConvBuf = mpCaptureBuf;
                // Lower�X���b�h��OS_WaitEvent()�Ńu���b�N���Ȃ��悤�A����ϊ����B�e�v�����Z�b�g���Ă����B
                OS_SignalEvent(&mLowerEvent, cEventMaskLower_NextConvertAndShot);
            } else {
                // ����̃L���v�`���ς݃f�[�^�͔j���B
                freeCapBufByPtr((u16*)mpCaptureBuf);
            }
        } else {
            // YUV2RGB�ϊ��v���Z�b�g�B
            mpConvBuf = mpCaptureBuf;
            OS_SignalEvent(&mLowerEvent,
                           (u32)(cEventMaskLower_Convert | (mShotFlag ? cEventMaskLower_Shot : 0)));
        }
        mpCaptureBuf = NULL;
    }
}


void Manager::procPostConvert(BOOL shot)
{
    if (shot) {
        // �L���v�`���ς݃f�[�^�͎B�e�p�Ɏg���B
        mpShotBuf = mpConvBuf;
    } else {
        // �L���v�`���ς݃f�[�^�͕s�v�Ȃ̂Ŏ̂Ă�B
        freeCapBufByPtr((u16*)mpConvBuf);
    }
    mpConvBuf = NULL;
}


// (mpShotBuf != NULL) �̂Ƃ��̂݌Ă΂��B
void Manager::procPostShot()
{
// #ifndef TWL_IPL_FINAL
//     mDebugPort = __LINE__;
// #endif
    // "check only urgent errors here" ���ă}�W�H���荞�݋֎~�ŌĂ΂�Ă邩��B

    freeCapBufByPtr((u16*)mpShotBuf);
}


// ���荞�݃��[�`������̂݌Ă΂��̂Ŋ��荞�݋֎~�ɂ��Ă��Ȃ��B
// �L���v�`�����A�B�e���ŃT�C�Y���ς��ꍇ�ł����Ă��A�傫������
// �T�C�Y��z�肵�ăo�b�t�@���m�ۂ���B
u16* Manager::allocCapBuf()
{
    for (u32 i = 0; i < cBufferCount; i++) {
        if (!mIsCapBufUsed[i] && mpCapBuf[i]) {
            mIsCapBufUsed[i] = TRUE;
//            OS_TPrintf("allocCapBuf[%d]=%x\n", i, mpCapBuf[i]);
            return mpCapBuf[i];
        }
    }
            IPL_PRINT("allocCapBuf: no memory! %d %d\n", mCaptureFlag, mShotFlag);
    
    return NULL;
}


void Manager::freeCapBufByPtr(const u16* pBuf)
{
    if (pBuf) {
        for (u32 i = 0; i < cBufferCount; i++) {
            if (mpCapBuf[i] == const_cast<u16*>(pBuf)) {
                mIsCapBufUsed[i] = FALSE;
                return;
            }
        }
    }
}

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
BOOL Manager::writeJpegToNand(const u8* src, u32 size)
{
    BOOL ret = FALSE;

//     const u8* path;
// 
//     path = MenuSys()->getUpPicMgr()->getSavePath();
//     if (path) {
//         cmn::sys::File file( (const char*)path );
//         ret = file.write(src, size);
//         IPL_PRINT("writeJpegToNand(%s)=%d\n", path, ret);
//         if (ret) {
//             MenuSys()->getUpPicMgr()->calcNextSavePath();
//         }
//     }

    ret = MenuSys()->getUpPicMgr()->writeTclPictureFile(src, size);

    return ret;
}
#endif


// �t�@�C�����͒��̐l�ɂ���������Ȃ��B
BOOL Manager::writeJpegToSD(const u8* src, u32 size)
{
    FSFile  file;
    FSFile* pFile = &file;
    u32     i;

    // DC_StoreAll(), DC_FlushAll() is NG?
    DC_StoreAll();

/*
    SD�J�[�h/menu/MENU####.jpg �Ƃ����t�@�C�����𐶐�����
*/
#define JPEG_DIRNAME     "sdmc:/menu"
#define JPEG_FILENAME    "menu%04d.jpg"

    if ( !pFile )
    {
        return FALSE;
    }

    /*
        �f�B���N�g���̑��݂��m���Ƃ���
        �{���́A�܂�SD�J�[�h�����݂��邩���m�F���A���̌�f�B���N�g�������ł�
        ���݂��邩�����Ė�����΍��Ƃ��������ŗǂ��̂����A����ł́A��������
        �I�[�v�����Ȃ���΂Ȃ�Ȃ��I
    */
    {
        FSFile dir;
        if ( !FS_OpenDirectory( &dir, JPEG_DIRNAME, FS_FILEMODE_R | FS_FILEMODE_W ) )
        {
            if ( !FS_CreateDirectory( JPEG_DIRNAME, FS_PERMIT_R | FS_PERMIT_W ) )
            {
                IPL_PRINT("Cannot create directory %s.\n", JPEG_DIRNAME);
                return FALSE;
            }
        }
        else
        {
            FS_CloseDirectory( &dir );
        }
    }

    FS_InitFile( pFile );

    for (i =
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
             0
#else
             1000
#endif
                  ; i < 10000; i++)
    {
        char buffer[32];
        STD_TSPrintf(buffer, JPEG_DIRNAME "/" JPEG_FILENAME, i);
        if ( FS_CreateFile( buffer, FS_PERMIT_W ) )   // ���łɂ���ꍇ���s����
        {
            if ( FS_OpenFileEx( pFile, buffer, FS_FILEMODE_W ) )    // �쐬�����Ẵt�@�C�����I�[�v������
            {
                // OK
                break;
//                 if (filename)   // �����Ƀ��������w�肳��Ă���΂����Ƀt�@�C�������i�[����
//                 {
//                     STD_CopyLString( filename, buffer, 256 );
//                 }
//                 return TRUE;
            } else {
            IPL_PRINT("Create %s, but cannot open it.\n", buffer); // �v���I�ȃG���[
            return FALSE;
            }
        }
    }
    if (i == 10000) {
    IPL_PRINT("Too many files in %s.\n", JPEG_DIRNAME);
    return FALSE;
    }

    if ( FS_WriteFile( pFile, src, static_cast<s32>(size) ) != size )
    {
        FS_CloseFile( pFile );
        return FALSE;
    }
    FS_CloseFile( pFile );
    return TRUE;
}


// /*!
//   �V���b�^�[�T�E���h�t�@�C�����Z�b�g���� �p���\��
//   @param p_wav WAV�̃w�b�_�̂Ȃ������t�@�C��
//   @param size �T�C�Y
//  */
// void Manager::setShutterSnd( const u8* p_wav , u32 size , BOOL stereo )
// {
// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
//     OSIntrMode enabled = OS_DisableInterrupts();
// 
//     mpShutterSnd    = p_wav;
//     mShutterSndSize = size;
//     mShutterStereo  = stereo;
// 
//     OS_RestoreInterrupts(enabled);
// #endif
// }


// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
// /*!
//   �V���b�^�[�T�E���h���J�n����
//  */
// void Manager::startShutterSnd()
// {
//     if ( mpShutterSnd )
//     {
// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
//         SNDEX_SetDSPMixRate( 0 );
//         SNDEX_ForceOut( TRUE );
//         DSP_PlaySound( (void*)mpShutterSnd, (u32)(mShutterSndSize / 2), mShutterStereo );
// #else
//         MenuSys()->getDspMgr()->playSound(mpShutterSnd, (u32)(mShutterSndSize / sizeof(u16)), mShutterStereo);
// #endif
//     }
// }
// 
// /*!
//   �V���b�^�[�T�E���h���I������
//  */
// void Manager::finishShutterSnd()
// {
//     if ( mpShutterSnd )
//     {
// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
//         SNDEX_SetDSPMixRate( 8 );
//         SNDEX_ForceOut( FALSE );
// #else
//         MenuSys()->getDspMgr()->stopSound();
// #endif
//     }
// }
// 
// /*!
//   �V���b�^�[�T�E���h���I�����Ă邩
//   @retval TRUE �I�����Ă�
//   @retval FALSE ���ĂȂ�
//  */
// BOOL Manager::isFinishedShutterSnd() const
// {
// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
//     return TRUE;
// #else
//     return !(MenuSys()->getDspMgr()->isSoundPlaying());
// #endif
// }
// #endif

void Manager::setCaptureRgbFlag(BOOL useRgb)
{
    mCaptureRgbFlag = (u8)useRgb;
    mEncodeOption = (u32)(SSP_JPEG_THUMBNAIL | (useRgb ? SSP_JPEG_RGB555 : SSP_JPEG_YUV422));
}

void Manager::encodeAndSave()
{
    u32 err = 0;
    OSTick tick;

    if (!mpShotBuf) {
        IPL_HALT("!mpShotBuf\n");
        // TWL_IPL_FINAL�ł̓X���[����[
        return;
    }

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
        // ���W���[�����[�h���ɁA�V���b�^�[�����~�܂�H
        // �V���b�^�[�����Đ�����O��JPEG�G���R�[�_���W���[����
        // ���[�h���Ă����K�v������悤���B
        // �V���b�^�[������I���܂�DSP���L����B
        MenuSys()->getDspMgr()->lock();
        MenuSys()->getDspMgr()->loadJpegEncoder();
#endif

//    // DSP���g���ꍇ�̂݁A�V���b�^�[���̍Đ��w�����o���B
//    startShutterSnd();

    DSPFrame_y  = mpEncodeWork->y;
    DSPFrame_cb = mpEncodeWork->cbcr;
    DSPFrame_cr = DSPFrame_cb + (mShotWdt * mShotHgt / 4);
#endif

    tick = OS_GetTick();

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
    Convert8bitYuv422To16bitYuv420((const u32*)mpShotBuf,
                                   DSPFrame_y,
                                   mShotWdt,
                                   mShotHgt,
                                   mpDtcmWork->mYuvBuf);
    // store YUV420 and invalidate encode result
    DC_FlushAll();
#endif

    IPL_PRINT("procShot: yuv422to420 time %d usec\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - tick));
    tick = OS_GetTick();

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
        // SD�J�[�h�A�N�Z�X���̃`�����΍�
        // V�u�����N���荞�݂��x������̂ŁA�A�t�B���p�����^�����W�X�^�֏������܂Ȃ��B
        // writeJpegToSD()���ĂԒ��O�̐ݒ�ł͊Ԃɍ���Ȃ��悤���B
//        MenuSys()->getUpPicMgr()->setSkipAffineFlag(TRUE);
#endif

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
    // �o�̓T�C�Y�͂����قǕK�v�H
    DSP_SetJpegEncoderBuffer(mpEncodeResult, cEncodeResultSizeMax);
    // explicit initialization
    DSP_SetJpegEncoderThumbnailBuffer(NULL, NULL, NULL);
    u32 size = DSP_StartJpegEncoder( static_cast< s16 > ( mShotWdt ) ,
                                     static_cast< s16 > ( mShotHgt ) ,
                                     DSP_JPEG_THUMBNAIL ,
                                     4 );
#else
        // �o�̓T�C�Y�͂����قǕK�v�H
        // unlock��upPicMgr�� if (event & cEventMask_WipeReplace) �ōs����B
        MenuSys()->getUpPicMgr()->lockTcl();
        MenuSys()->getUpPicMgr()->lockJpegBuf();
        u32 size = MenuSys()->getDspMgr()->encodeJpegFrom16bitYuv420(DSPFrame_y,
                                                                     mShotWdt,
                                                                     mShotHgt,
                                                                     MenuSys()->getUpPicMgr()->getJpegBufPtr(),
                                                                     MenuSys()->getUpPicMgr()->getJpegBufSize(),
                                                                     NULL,
                                                                     0,
                                                                     DSP_JPEG_THUMBNAIL);
#endif
#else
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
    // unlock��upPicMgr�� if (event & cEventMask_WipeReplace) �ōs����B
    MenuSys()->getUpPicMgr()->lockTcl();
    MenuSys()->getUpPicMgr()->lockJpegBuf();

    SSP_SetJpegEncoderMakerNote(MenuSys()->getUpPicMgr()->getMakerNote(),
                                MenuSys()->getUpPicMgr()->getMakerNoteSize());
#endif

    if (!SSP_SetJpegEncoderDateTimeNow()) {
        // �����̖��ߍ��݂Ɏ��s�����̂Ń[���Ŗ��߂�B
        // ������́uYYYY:MM:DD HH:MM:DD�v+0x00 �� 20 �����Ƃ��Ă��������B
        static const u8 zeroes[20] = {};
        IPL_PRINT("SSP_SetJpegEncoderDateTimeNow() failed!\n");
        SSP_SetJpegEncoderDateTime((u8*)zeroes);
    }
    SSP_SetJpegEncoderSignMode(TRUE);
    u32 size = SSP_StartJpegEncoder((const void*)mpShotBuf,
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
                                    mpEncodeResult,
                                    cEncodeResultSizeMax,
#else
                                    MenuSys()->getUpPicMgr()->getJpegBufPtr(),
                                    MenuSys()->getUpPicMgr()->getJpegBufSize(),
#endif
                                    mpEncodeWork,
                                    mShotWdt, mShotHgt,
                                    cEncodeQuality, cEncodeSampling, mEncodeOption);
#endif

    IPL_PRINT("procShot: encode size %d, time %d usec\n", size, (u32)OS_TicksToMicroSeconds(OS_GetTick() - tick));

        tick = OS_GetTick();

        // save to SD or NAND
        if (size) {
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
            if (!writeJpegToNand(MenuSys()->getUpPicMgr()->getJpegBufPtr(),
                                 size)) {
                err = 3;
            }
#endif

            
            if (!writeJpegToSD(
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
                mpEncodeResult,
#else
                MenuSys()->getUpPicMgr()->getJpegBufPtr(),
#endif
                size)) {
                // SD�J�[�h�ւ̕ۑ��̓e�X�g�Ȃ̂ŁA�G���[�ɂ͂��Ȃ��B
                // do nothing
            }
        } else {
            err = 4;
        }

        IPL_PRINT("procShot: save err %d, time %d usec\n", err, (u32)OS_TicksToMicroSeconds(OS_GetTick() - tick));

// #ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
//         while (!isFinishedShutterSnd()) {
//             IPL_PRINT("procShot: waiting ShutterSnd finish\n");
//             OS_Sleep(1);
//         }
//         finishShutterSnd();
// #endif

#ifdef TWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT
#else
#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
        // �V���b�^�[������I�������DSP�̐�L�������ł���B
        MenuSys()->getDspMgr()->unlock();
#endif

        MenuSys()->getUpPicMgr()->setJpegDataSize(size);

        if (MenuSys()->getUpPicMgr()->isEffectTypeNormal()) {
            // ������JPEG�f�R�[�h���s���B
            // �܂�A�J�����V�[���I���̂��߂̃t�F�[�h�A�E�g�J�n���x���B
            MenuSys()->getUpPicMgr()->decodeJpeg();
        } else {
            // ��ɌÂ��ʐ^�̃��C�v�A�E�g���J�n����B
            // �J�����V�[���I���̂��߂̃t�F�[�h�A�E�g�J�n�������Ȃ邪
            // �����`���[���C����ʂ֖߂��Ă����΂炭���ʂ������Ȃ��B
            // ���̏�ԂŃ����`���[���C����ʂ̑���͂ł��邪�A�d���B
            MenuSys()->getUpPicMgr()->wipeOut();
        }

//        MenuSys()->getUpPicMgr()->setSkipAffineFlag(FALSE);
#endif

    if (!mShotErr) {
        mShotErr = err;
    }

        {
            OSIntrMode enabled = OS_DisableInterrupts();

            procPostShot();
            mpShotBuf = NULL;
            mShotDone = TRUE;

            OS_RestoreInterrupts(enabled);
        }

    // invalidate capture buffer for next capture
    DC_FlushAll();

    IPL_PRINT("encodeAndSave done err=%d\n", mShotErr);
}



}}}
