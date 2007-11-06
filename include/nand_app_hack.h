
// とにかく.appファイル内のSRLを直接読み込むコールバック
//#define WAD_SRL_OFFSET 0x12C0
#define WAD_SRL_OFFSET 0
static FSResult ReadFromWad(FSArchive *arc, void *buffer, u32 offset, u32 length)
{
    FSFile *file = (FSFile *)FS_GetArchiveBase(arc);
    (void)FS_SeekFile(file, (int)(WAD_SRL_OFFSET + offset), FS_SEEK_SET);
    return (FS_ReadFile(file, buffer, (int)length) >= 0) ?
            FS_RESULT_SUCCESS : FS_GetResultCode(file);
}
// 指定のタイトルIDで起動したNANDアプリであると仮定して"rom"アーカイブを置換
static BOOL FS_IdentifyTitle(u32 titleLo)
{
    BOOL    retval = FALSE;
    static struct
    {
        BOOL    initialized;
        FSFile  file[1];
    }
    context;
    if (!context.initialized)
    {
        context.initialized = TRUE;
//        if (OS_GetBootType() == OS_BOOTTYPE_NAND)
        {
            char    path[FS_ENTRY_LONGNAME_MAX];
            STD_TSPrintf(path, "nand:/title_e/00010001/%02X%02X%02X%02X/content/12123434.app",
                        ((titleLo / 1000) % 10) + '0',
                        ((titleLo / 100) % 10) + '0',
                        ((titleLo / 10) % 10) + '0',
                        ((titleLo / 1) % 10) + '0');
            if (!FS_IsAvailable())
            {
                FS_Init(FS_DMA_NOT_USE);
            }
            if (FS_OpenFileEx(context.file, path, FS_FILEMODE_R))
            {
                CARDRomHeader   header[1];
                (void)FS_SeekFile(context.file, (int)(WAD_SRL_OFFSET + 0), FS_SEEK_SET);
                if (FS_ReadFile(context.file, header, sizeof(header)) == sizeof(header))
                {
                    FSArchive      *arc = FS_FindArchive("rom", 3);
                    (void)FS_UnloadArchive(arc);
                    if (FS_LoadArchive(arc, (u32)context.file,
                                header->fat.offset, header->fat.length,
                                header->fnt.offset, header->fnt.length,
                                ReadFromWad, NULL))
                    {
                        retval = TRUE;
                    }
                }
                if (!retval)
                {
                    (void)FS_CloseFile(context.file);
                }
            }
        }
    }
    return retval;
}
