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
//     // �X���b�h�D�揇�ʁB
//     enum
//     {
//         cThreadPriority_LauncherCameraHi = OS_THREAD_LAUNCHER_PRIORITY - 1,
//         cThreadPriority_LauncherCameraLo = OS_THREAD_LAUNCHER_PRIORITY + 1,
//         cThreadPriority_LauncherCameraShot = OS_THREAD_LAUNCHER_PRIORITY + 2,
//         cThreadPriority_UpPicture = OS_THREAD_LAUNCHER_PRIORITY + 3
//     };

    // ��蒼���� �X���b�h�D��x�B
    enum
    {
    // �D��x�͌������K�v�B
        cThreadPriority_CameraI2c     = OS_THREAD_LAUNCHER_PRIORITY - 2,
        cThreadPriority_CameraLowerHi = OS_THREAD_LAUNCHER_PRIORITY - 1,

        cThreadPriority_CameraLowerLo = OS_THREAD_LAUNCHER_PRIORITY + 10
    };
    
};
}}
