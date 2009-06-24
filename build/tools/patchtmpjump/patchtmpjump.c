/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - patchtmpjump
  File:     patchtmpjump.c

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<stdlib.h>             // free()
#include	<string.h>
#include	<sys/stat.h>           // stat()

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG_PRINT         fprintf
#else
#define DEBUG_PRINT(...)
#endif
#define	errorf(...)	do { fprintf(stderr, "Error: ");   \
                             fprintf(stderr, __VA_ARGS__); \
                             fprintf(stderr, "\n"); } while(0)

#define	warningf(...)	do { fprintf(stderr, "Warning: "); \
                             fprintf(stderr, __VA_ARGS__); \
                             fprintf(stderr, "\n"); } while(0)

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define	SURFFIX_TMPJUMP     ".ntrtmpjump.srl"
#define NOP_CODE            0xE1A00000

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;

typedef struct ROM_Header
{
    //  0x000-
    u8      header[32];                // システム予約

    //  0x020-
    //  ARM9
    u32     main_rom_offset;           // 転送元 ROM オフセット
    u32     main_entry_address;        // 実行開始アドレス(未実装)
    u32     main_ram_address;          // 転送先 RAM アドレス
    u32     main_size;                 // 転送サイズ

    //  ARM7
    u32     sub_rom_offset;            // 転送元 ROM オフセット
    u32     sub_entry_address;         // 実行開始アドレス(未実装)
    u32     sub_ram_address;           // 転送先 RAM アドレス
    u32     sub_size;                  // 転送サイズ

}
ROM_Header;

u32 SearchSdkVersion( void *filebuffer );
s32 PatchForPMSleep( void *buffer, u32 version );
int WriteFile(const char *filename, char *buffer, int size);
int ReadFile(const char *filename, void *filebuffer);

// SDKバージョン検出用マジックコード
static const u32 sdk_ver_magic_code[] = {
    SDK_NITROCODE_BE,
    SDK_NITROCODE_LE,
};

// SDKバージョンリスト
static const u32 sdk_ver_list[] = {
    0x02004ee8,
    0x030028a0,
    0x03017530,
    0x03027531,
    0x04007530,
    0x04017531,
    0x04027530,
};
static const u32 sdk_ver_mask = 0xFFFF0000;

// 対象コード
static const u32 target_idx_400 = 3;
static const u32 target_code_400[] = {
    // if (PMi_TriggerBL & PM_TRIGGER_CARD)
    0xE3100008,  // TST      Rx, #0x8
    0x0A000001,  // BEQ      xxxx
    // OS_EnableIrqMask(OS_IE_CARD_IREQ)
    0xE3A00601,  // MOV      Rx, #0x100000
    0xEBFFD650,  // BL       EnableIrqMask <- target
    // if (PMi_TriggerBL & PM_TRIGGER_CARTRIDGE)
    0xE59F00C8,  // LDR      Rx, [PC, #xxx]
    0xE1D000B0,  // LDRH     Rx, [Rx]
    0xE3100010,  // TST      R0, #0x10
    0x0A000001,  // BEQ      xxxx
    // OS_EnableIrqMask(OS_IE_CARTRIDGE)
    0xE3A00A02,  // MOV      Rx, #0x2000
    0xEBFFD64A,  // BL       EnableIrqMask
    // OS_RestoreInterrupts(prepIntrMode)
    0xE1A00004,  // MOV      Rx, Rx
    0xEBFFE377,  // BL       OS_RestoreInterrupts
    // OS_EnableIrq()
    0,
};
static const u32 target_mask_400[] = {
    // if (PMi_TriggerBL & PM_TRIGGER_CARD)
    0xFEF0FFFF,  // TST      Rx, #0x8
    0xFF000000,  // BEQ      xxxx
    // OS_EnableIrqMask(OS_IE_CARD_IREQ)
    0xFFFF0FFF,  // MOV      Rx, #0x100000
    0xFF000000,  // BL       EnableIrqMask <- target
    // if (PMi_TriggerBL & PM_TRIGGER_CARTRIDGE)
    0xFFFF0000,  // LDR      Rx, [PC, #xxx]
    0xFFF00000,  // LDRH     Rx, [Rx]
    0xFEF0FFFF,  // TST      Rx, #0x10
    0xFF000000,  // BEQ      xxxx
    // OS_EnableIrqMask(OS_IE_CARTRIDGE)
    0xFFFF0FFF,  // MOV      Rx, #0x2000
    0xFF000000,  // BL       EnableIrqMask
    // OS_RestoreInterrupts(prepIntrMode)
    0xFFFF0000,  // MOV      Rx, Rx
    0xFF000000,  // BL       OS_RestoreInterrupts
    0,
};

// 対象コードリスト
static const u32 * const target_code_list[] = {
    target_code_400,  // 0x02004ee8
    target_code_400,  // 0x030028a0
    target_code_400,  // 0x03017530
    target_code_400,  // 0x03027531
    target_code_400,  // 0x04007530
    target_code_400,  // 0x04017531
    target_code_400,  // 0x04027530
};

static const u32 * const target_mask_list[] = {
    target_mask_400,  // 0x02004ee8
    target_mask_400,  // 0x030028a0
    target_mask_400,  // 0x03017530
    target_mask_400,  // 0x03027531
    target_mask_400,  // 0x04007530
    target_mask_400,  // 0x04017531
    target_mask_400,  // 0x04027530
};

static const u32 * const target_idx_list[] = {
    &target_idx_400,  // 0x02004ee8
    &target_idx_400,  // 0x030028a0
    &target_idx_400,  // 0x03017530
    &target_idx_400,  // 0x03027531
    &target_idx_400,  // 0x04007530
    &target_idx_400,  // 0x04017531
    &target_idx_400,  // 0x04027530
};




/*---------------------------------------------------------------------------*
  Name:         main

  Description:  patch NitroSDK ROM for tmp jump

  Arguments:    file name of binary

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int     i;
    int     fsize;
    char   *buffer;
    char   *fname;
    char    outputname[FILENAME_MAX];
    u32     version;

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            // Read entire file image
            fname = argv[i];

            fsize = ReadFile(fname, &buffer);
            if (fsize < 0)
            {
                return 1;
            }

            // Search SDK version
            version = SearchSdkVersion(buffer);
            DEBUG_PRINT(stdout, "SDK version %08X\n", (int)version);

            // Patch ARM7 component
            if ( PatchForPMSleep(buffer, version) )
            {
                return 1;
            }

            // Output patched binaries
            sprintf(outputname, "%s%s", fname, SURFFIX_TMPJUMP);
            if (fsize != WriteFile(outputname, buffer, fsize))
            {
                return 1;
            }

            free(buffer);
        }
    }
    else
    {
        fprintf(stderr,
                "Usage: patchtmpjump [ROM IMAGE...]\n\n"
                "Patch NITRO ROM for tmp jump\n");
    }
    return 0;
}

/*---------------------------------------------------------------------------*
 *  search SDK version
 *---------------------------------------------------------------------------*/
u32 SearchSdkVersion( void *buffer )
{
    ROM_Header* h = buffer;
    const u32 *magic = sdk_ver_magic_code;

    int offset = h->main_entry_address - h->main_ram_address;
    int size = h->main_size - offset;
    u32 *cur = (void*)((u8*)buffer + h->main_rom_offset + offset);
    u32 *end = (void*)((u8*)cur + size);

    while( cur < end )
    {
        if( cur[0] == magic[0] && cur[1] == magic[1] )
        {
            return cur[-1];
        }
        cur++;
    }
    return 0;
}

/*---------------------------------------------------------------------------*
 *  patch PMi_DoSleep
 *---------------------------------------------------------------------------*/
s32 PatchForPMSleep( void *buffer, u32 version )
{
    ROM_Header* h = buffer;
    int size = h->sub_size;
    u32 *cur = (void*)((u8*)buffer + h->sub_rom_offset);
    u32 *end = (void*)((u8*)cur + size);
    u32 count = 0;
    int i;

    for ( i=0; i<sizeof(sdk_ver_list)/sizeof(u32); i++ )
    {
        if ( (version & sdk_ver_mask) == (sdk_ver_list[i] & sdk_ver_mask) )
        {
            while( cur < end )
            {
                const u32* s = target_code_list[i];
                const u32* m = target_mask_list[i];
                u32  idx = *target_idx_list[i];
                u32* d = cur;
                int ii = 0;
                // compare
                while ( s[ii] )
                {
                    if ( (d[ii] & m[ii]) != (s[ii] & m[ii]) )
                    {
                        break;
                    }
                    ii++;
                }
                // patch
                if ( ! s[ii] )
                {
                    DEBUG_PRINT(stdout, "%X: d = %X, s = %X, m = %X\n", (int)&cur[idx]-(int)buffer, (int)d[idx], (int)s[idx], (int)m[idx]);
                    d[idx] = NOP_CODE;
                    count++;
                }
                cur++;
            }
            switch ( count )
            {
                case 1:
                    return 0;
                    break;
                case 0:
                    errorf("Can't find target code.");
                    return -1;
                    break;
                default:
                    errorf("Find too many target codes.");
                    return -1;
                    break;
            }
        }
    }
    errorf("SDK version %08X isn't supported yet.", (int)version);
    return -1;
}


/*---------------------------------------------------------------------------*
 *   File Read Utility
 *
 *    int   ReadFile( const char* filename, void** buffer )
 *---------------------------------------------------------------------------*/
int ReadFile(const char *filename, void *filebuffer)
{
    FILE   *fp;
    struct stat fileStat;
    int     fileSize;
    void  **buffer = (void **)filebuffer;

    /* Check file */
    if (stat(filename, &fileStat) || !S_ISREG(fileStat.st_mode))
    {
        goto error;
    }

    fileSize = fileStat.st_size;
    if (fileSize < 0)
    {
        goto error;
    }

    /* Open file */
    fp = fopen(filename, "rb");
    if (!fp)
    {
        goto error;
    }

    /* Get Memory */
    *buffer = malloc(fileSize);
    if (*buffer == NULL)
    {
        fclose(fp);
        goto error;
    }

    /* Read file */
    if (fileSize != fread(*buffer, sizeof(char), fileSize, fp))
    {
        free(*buffer);
        fclose(fp);
        goto error;
    }

    /* Close file */
    fclose(fp);
    return fileSize;

  error:
    errorf("Can't read '%s'", filename);
    return -1;
}

/*---------------------------------------------------------------------------*
 *   File Write Utility
 *
 *    int   WriteFile( const char* filename, void* buffer, int size )
 *---------------------------------------------------------------------------*/
int WriteFile(const char *filename, char *buffer, int size)
{
    FILE   *fp;

    /* Open file */
    fp = fopen(filename, "wb");
    if (!fp)
    {
        goto error;
    }

    /* Write file */

    if (size != fwrite(buffer, sizeof(char), size, fp))
    {
        fclose(fp);
        goto error;
    }

    /* Close file */
    fclose(fp);
    return size;

  error:
    errorf("Can't write '%s'", filename);
    return -1;
}

