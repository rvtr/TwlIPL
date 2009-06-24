
#include <twl.h>
#include <nitro/std.h>
#include "sd_log_wrapper.h"

#include "sd_log.c"

/* Function Prototype */
static void OS_FPutStringSD(int console, const char *str);
static void SDLOGi_WriteThreadEx(void* arg);

/* Static */
static void (*HookedFPutString)(int console, const char *string);

void TwlStartUp(void)
{
    OS_InitPrintServer();              // Should be called before PXI_Init()
}

/*---------------------------------------------------------------------------*
  Name:         SDLOG_Init

  Description:  Initialize

  Arguments:    None
                
  Returns:      ファイルの作成に失敗した場合は FALSE を返す
 *---------------------------------------------------------------------------*/
BOOL SDLOG_InitEx(const char* path, const char* filename, char* buffer, u16 bufferSize, u16 partitionSize, SDLOGWriteType writeType)
{
    if( isInitialized )
    {
        return FALSE;
    }
    
    if( !FS_IsAvailable() )
    {
        FS_Init(FS_DMA_NOT_USE);
    }

    OS_InitLog();
    
    {
        int i = 0;
        FS_InitFile(&work.logInfo[i].file);
        
        /* ファイルシステムの初期化 */
        {
            char tmpPath[256];

            STD_TSPrintf(tmpPath, "%s/%s", path, filename);
            
            if( writeType == SDLOG_WRITE_TYPE_OVERWRITE )
            {
                /* 上書き設定の場合はファイルを削除 */
                (void)FS_DeleteFileAuto(tmpPath);
            }
            
            if( !FS_CreateDirectoryAuto( path, FS_PERMIT_W | FS_PERMIT_R ) )
            {
                //OS_TPrintf("%s L:%04d Error(%d)\n", __FILE__, __LINE__, FS_GetArchiveResultCode(tmpPath) );
            }
            
            if( !FS_CreateFile( tmpPath, FS_PERMIT_W | FS_PERMIT_R ) )
            {
                //OS_TPrintf("%s L:%04d Error(%d)\n", __FILE__, __LINE__, FS_GetArchiveResultCode(tmpPath) );
            }
            
            if(!FS_OpenFileEx(&work.logInfo[i].file, tmpPath, FS_PERMIT_R | FS_PERMIT_W))
            {
                //OS_TPrintf("%s L:%04d Error(%d)\n", __FILE__, __LINE__, FS_GetArchiveResultCode(tmpPath) );
                goto file_error;
            }
            
            if( writeType == SDLOG_WRITE_TYPE_CONTINUANCE )
            {
                /* 追記設定の場合はファイル端までシーク */
                FS_SeekFileToEnd(&work.logInfo[i].file);
            }
        }
        
        /* OS_Log の諸設定 */
        work.logInfo[i].buffer = buffer;
        work.logInfo[i].current = 0;
        work.logInfo[i].bufferSize = bufferSize;
        work.logInfo[i].partitionSize = partitionSize;
        work.logInfo[i].writeType = writeType;

        OS_InitLogBuffer(&work.logInfo[i].info, buffer, partitionSize);
        OS_SetLogWarningOffset(&work.logInfo[i].info, OS_LOG_WARNING_OFFSET_FULL );
        OS_SetLogAnnounceType(&work.logInfo[i].info, OS_LOG_ANNOUNCE_IMM);

        OS_SetLogCallback(&work.logInfo[i].info, SDlogCallback, (void *)i );
    }

    /* カレントは IS_TWL_DEBBUGER と同じく出力ゼロ */
    OS_SetLogCurrentInfo(&work.logInfo[0].info);
    
    /* Thread */
    if( OS_IsThreadAvailable() == FALSE )
    {
        /*スレッドシステムが立ち上がっていない場合は立ち上げる*/
        OS_InitThread();
    }

    OS_InitMessageQueue(&work.writeQueue, work.writeMsg, WRITE_MESSAGE_COUNT );
    OS_CreateThread(&work.writeThread, SDLOGi_WriteThreadEx, NULL,
                    work.writeThreadStack+SDLOG_THREAD_STACK_SIZE/sizeof(u32), SDLOG_THREAD_STACK_SIZE, 15);
    OS_WakeupThreadDirect(&work.writeThread);

    HookedFPutString = OS_FPutString;
    OS_FPutString = OS_FPutStringSD;

    SDLOG_Printf("------------ START ------------\n");

    isInitialized =TRUE;
    
    return TRUE;

file_error:
    return FALSE;
}


void OS_FPutStringSD(int console, const char *str)
{
#pragma unused (console)
    OS_LockMutex( &work.logInfo[0].info.mutex );
    OS_LogFPutString( &work.logInfo[0].info, str );
    OS_UnlockMutex( &work.logInfo[0].info.mutex );
}


void SDLOG_FinishEx(void)
{
    SDLOG_Finish();

    if (OS_FPutString == OS_FPutStringSD) {
        OS_FPutString = HookedFPutString;
    }
}


static void SDLOGi_WriteThreadEx(void* arg)
{
#pragma unused(arg)

    u32         num;
    const char* buffer;
    u32         dataSize;

    OSMessage   msg;

    while(TRUE)
    {

        (void)OS_ReceiveMessage(&work.writeQueue, &msg, OS_MESSAGE_BLOCK);
        num =(u32) msg;
        (void)OS_ReceiveMessage(&work.writeQueue, &msg, OS_MESSAGE_BLOCK);
        buffer =(const char*) msg;
        (void)OS_ReceiveMessage(&work.writeQueue, &msg, OS_MESSAGE_BLOCK);
        dataSize =(u32) msg;
        
        if(FS_WriteFile(&work.logInfo[num].file, buffer, (u16)dataSize) < 0)
        {
            if (OS_FPutString == OS_FPutStringSD) {
                OS_FPutString = HookedFPutString;
                OS_TPrintf("%s L:%d SD Write Error\n",__FILE__, __LINE__);
            }
        } else {
            if (OS_FPutString == HookedFPutString) {
                OS_FPutString = OS_FPutStringSD;
            }
        }

        FS_FlushFile(&work.logInfo[num].file);

    }
}