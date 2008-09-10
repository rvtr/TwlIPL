#define cScreenWidth  256
#define cScreenHeight 192
#define cPhotoWidth   640
#define cPhotoHeight  480
// #define cThreadPriority_LauncherCameraHi (OS_THREAD_LAUNCHER_PRIORITY - 1)
// #define cThreadPriority_LauncherCameraLo (OS_THREAD_LAUNCHER_PRIORITY + 1)
// #define cThreadPriority_LauncherCameraShot (OS_THREAD_LAUNCHER_PRIORITY + 2)
#define IPL_ASSERT(exp) if ( !( exp ) ) IPL_HALT( #exp )
#define IPL_PRINT OS_TPrintf
#define IPL_HALT OS_TPanic

namespace menu { namespace sys {
class Object
{
public:
//     // スレッド優先順位。
//     enum
//     {
//         cThreadPriority_LauncherCameraHi = OS_THREAD_LAUNCHER_PRIORITY - 1,
//         cThreadPriority_LauncherCameraLo = OS_THREAD_LAUNCHER_PRIORITY + 1,
//         cThreadPriority_LauncherCameraShot = OS_THREAD_LAUNCHER_PRIORITY + 2,
//         cThreadPriority_UpPicture = OS_THREAD_LAUNCHER_PRIORITY + 3
//     };

    // 作り直し版 スレッド優先度。
    enum
    {
    // 優先度は検討が必要。
        cThreadPriority_CameraI2c     = OS_THREAD_LAUNCHER_PRIORITY - 2,
        cThreadPriority_CameraLowerHi = OS_THREAD_LAUNCHER_PRIORITY - 1,

        cThreadPriority_CameraLowerLo = OS_THREAD_LAUNCHER_PRIORITY + 10
    };
    
};
}}
