/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makegcdfirm
  File:     out_gcdfirm.c

  Copyright 2007 Nintendo.   All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>                 // atoi()
#include    <string.h>                 // strcmp()
#include    <ctype.h>                  // isprint()
#include    <unistd.h>                 // chdir()
#include    <tchar.h>
#include    <limits.h>                 // UCHAR_MAX
#include    <time.h>
#include    <sys/stat.h>               // stat()
#include    "elf.h"
#include    "misc.h"
#include    "defval.h"
#include    "format_rom.h"
#include    "format_nlist.h"
#include    "makegcdfirm.h"
#include    "format_sign.h"
#include    "acsign_gcd.h"
#include    "compress.h"

#define SDK_ASM

#include    <firm/aes/ARM7/aes_ids.h>
#include    "../acsign/aes2.h"

#define ROMHDRCMD  "ROM_HDR"
#define SBIN9CMD   "ARM9_SBIN"
#define SBIN7CMD   "ARM7_SBIN"
#define ELF9CMD    "ARM9_ELF"
#define ELF7CMD    "ARM7_ELF"
#define GAMECODECMD  "GAMECODE"
#define ARM9X2CMD  "ARM9_X2"
#define NMLOFSCMD  "NML_OFS"
#define TWLOFSCMD  "TWL_OFS"
#define VERCMD     "VERSION"
#define RSAKEYCMD  "RSA_KEY"
#define OUTKEYCMD  "OUT_KEY"
#define WREGCMD    "WRAM_RBIN"
#define NANDFMCMD  "NANDFIRM"
#define NORFMCMD   "NORFIRM"
#define ERRCMD     "ERROR"

static BOOL ConstructGcdfirmFile(char * specFile);
static BOOL ReadRomHeaderFile(const char *fileName);
static BOOL ReadSbinFile(const char *fileName, void* minfo, void* hash, BOOL comp);
static size_t ReadFirmFile(const char *fileName);
static BOOL ReadKeyFile(const char *fileName);
static BOOL ReadWramRegFile(const char *fileName);
static s32 GetRamAddr(const char *fileName);

static BOOL EncryptBuffer(char *buffer, int length);

static BOOL RomHeader_Command(char * line, int num);
static BOOL Sbin9_Command(char * line, int num);
static BOOL Sbin7_Command(char * line, int num);
static BOOL Elf9_Command(char * line, int num);
static BOOL Elf7_Command(char * line, int num);
static BOOL GAMECODE_Command(char * line, int num);
static BOOL ARM9X2_Command(char * line, int num);
static BOOL NMLOFS_Command(char * line, int num);
static BOOL TWLOFS_Command(char * line, int num);
static BOOL VERSION_Command(char * line, int num);
static BOOL RSAKEY_Command(char * line, int num);
static BOOL OUTKEY_Command(char * line, int num);
static BOOL WramRegs_Command(char * line, int num);
static BOOL NorFirm_Command(char * line, int num);
static BOOL NandFirm_Command(char * line, int num);
static BOOL ERROR_Command(char * line, int num);

static BOOL InitializeAesKey(void);
static BOOL InitializeGcdfirmFile(void);
static BOOL FinalizeGcdfirmFile(const char *gcdFile);

static s32 Offset;                     // Current offset
static int LineNum;                    // Line number   for error message
static const char *specFileName;       // specFile name for error message

GCDHeader  gcdHeader;                 // Gcdfirm Header Shadow
FIRMSignedContext signedContext;
u8 *keyFileBuf;
BOOL compArm9 = FALSE;
BOOL compArm7 = FALSE;
tErrorFlags errFlags;

//---------------------------------------------------------------------------
//  Output - gcdfirm File
//---------------------------------------------------------------------------

BOOL OutputGcdfirmFile(const char *specFile, const char *gcdFile)
{
    char   *buffer;
    BOOL    state;

    if (ReadFile(specFile, &buffer, READ_ALL) <= 0)
    {
        return FALSE;
    }

    if (!OpenFile(gcdFile))
    {
        return FALSE;
    }

    specFileName = specFile;

    state = InitializeGcdfirmFile() && ConstructGcdfirmFile(buffer) &&
            FinalizeGcdfirmFile(gcdFile) && CloseFile();

    if (!state)
    {
        DeleteOutFile();
    }

    return state;
}


//---------------------------------------------------------------------------
//  Output - Gcdfirm File
//---------------------------------------------------------------------------

static const tCommandDesc command[] = {
    {ROMHDRCMD, RomHeader_Command},
    {SBIN9CMD, Sbin9_Command}, {SBIN7CMD, Sbin7_Command},
    {ELF9CMD, Elf9_Command},{ELF7CMD, Elf7_Command},
    {VERCMD, VERSION_Command},
    {GAMECODECMD, GAMECODE_Command},
    {ARM9X2CMD, ARM9X2_Command},
    {NMLOFSCMD, NMLOFS_Command},
    {TWLOFSCMD, TWLOFS_Command},
    {RSAKEYCMD, RSAKEY_Command},
    {OUTKEYCMD, OUTKEY_Command},
    {WREGCMD, WramRegs_Command},
    {NANDFMCMD, NandFirm_Command},
    {NORFMCMD, NorFirm_Command},
    {ERRCMD, ERROR_Command},
};

BOOL ConstructGcdfirmFile(char * specFile)
{
    char *line;
    char *line_top;
    char *p;
    int     i;

    LineNum = 0;
    Offset = 0x4000;
    SeekFile(Offset);

    line = specFile++;

    while (*line != '\0')
    {
        LineNum++;

        // Get command line
        line_top = line;
        while (*line != '\0')
        {
            if (*line++ == '\n')
            {
                break;
            }
        }

        // Print for debug
        debug_printf("GCDSF Line%4d [", LineNum, line);
        for (p = line_top; p != line; p++)
        {
            if (isprint(*p))
            {
                debug_printf("%c", *p);
            }
        }
        debug_printf("]\n");

        if (*line_top == '#')
        {
        }
        else
        {
            for (i = 0; i < (sizeof(command) / sizeof(command[0])); i++)
            {
                if (!strncmp(line_top, command[i].string, strlen(command[i].string)))
                {
                    if (command[i].funcp != NULL)
                    {
                        char  line_cmd[FILENAME_MAX];
                        char  line_scan[FILENAME_MAX];
                        char* line_conv;
                        int num;

                        num = sscanf( line_top,
                              "%1024[^ :] : %1024[^ \x0d:#\n]",
                              line_cmd, line_scan
                              );
                        line_conv = ResolveDefVal(line_scan);

                        debug_printf("line_cmd = %s, line_conv = %s\n", line_cmd, line_conv);

                        if (!command[i].funcp(line_conv, num - 1))
                            return FALSE;
                    }
                }
            }
        }

    }
    return TRUE;
}


//---------------------------------------------------------------------------
//  Output - 'RomHeader' Command
//---------------------------------------------------------------------------

static BOOL RomHeader_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("hom header file = %s\n", line);

    return ReadRomHeaderFile(line);
}


static BOOL ReadRomHeaderFile(const char *fileName)
{
    char   *file;
    int     file_size;
    struct stat st;

    if (FILESTATUS_FILE != GetFileStatus(&st, fileName))
    {
        error("'%s' is not regular file.", fileName);
        return FALSE;
    }

    if ((file_size = ReadFile(fileName, &file, READ_ALL)) < 0)
        return FALSE;

    gcdHeader = *(GCDHeader*)file;

    // Output file image with fitting region
    SeekFile(0L);
    PutBuffer(file, file_size);

    Free(&file);

    return CheckResult();
}

//---------------------------------------------------------------------------
//  Output - 'WramRegs' Command
//---------------------------------------------------------------------------

extern MIHeader_WramRegs wram_regs_init;
MIHeader_WramRegs* wram_regs;

static BOOL WramRegs_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("wram regs file = %s\n", line);

    return ReadWramRegFile(line);
}

static BOOL ReadWramRegFile(const char *fileName)
{
    int     file_size;
    struct stat st;

    if (FILESTATUS_FILE != GetFileStatus(&st, fileName))
    {
        error("'%s' is not regular file.", fileName);
        return FALSE;
    }

    if ((file_size = ReadFile(fileName, &wram_regs, READ_ALL)) < 0)
        return FALSE;

    gcdHeader.h.w = *wram_regs;

    return CheckResult();
}

//---------------------------------------------------------------------------
//  Output - 'RSAKEY' Command
//---------------------------------------------------------------------------

static BOOL RSAKEY_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("rsa key = %s\n", line);

    return ReadKeyFile(line);
}

static BOOL ReadKeyFile(const char *fileName)
{
    int     file_size;
    struct stat st;

    if (FILESTATUS_FILE != GetFileStatus(&st, fileName))
    {
        error("'%s' is not regular file.", fileName);
        return FALSE;
    }

    if ((file_size = ReadFile(fileName, &keyFileBuf, READ_ALL)) < 0)
        return FALSE;

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'OUTKEY' Command
//---------------------------------------------------------------------------

static BOOL OUTKEY_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("out key = %s\n", line);

    PubkeyFileName = line;

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'VERSION' Command
//---------------------------------------------------------------------------

static u8 ConvToBCD8(int x)
{
    u8 bcd = 0;

    x %= 100;
    bcd |= (x / 10)<<4;
    bcd |= (x % 10);

    return bcd;
}

static BOOL VERSION_Command(char * line, int num)
{
    char   scan[FILENAME_MAX];
    u64    version = -1;

    // rescan
    num = sscanf( line,
          "0x%1024[^\x0d\n]",
          scan
          );

    if (num == 1)
    {
        // convert version info
        version = strtoull(scan, NULL, 16);
    }
    else if (num == 0)
    {
        // generate version info
        u8* ver8 = (u8*)&version;
        time_t timer;
        struct tm *t_st;

        time(&timer);
        t_st = localtime(&timer);

        ver8[0] = ConvToBCD8(t_st->tm_min);
        ver8[1] = ConvToBCD8(t_st->tm_hour);
        ver8[2] = ConvToBCD8(t_st->tm_mday);
        ver8[3] = ConvToBCD8(t_st->tm_mon+1);
        ver8[4] = ConvToBCD8(t_st->tm_year);
        ver8[5] = 0xff;
        ver8[6] = 0xff;
        ver8[7] = 0xff;
    }

    debug_printf2("version = %08llx\n", version);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'NMLOFS' Command
//---------------------------------------------------------------------------

static BOOL NMLOFS_Command(char * line, int num)
{
    u32    offset = 0;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    offset = strtoul(line, NULL, 0);

    gcdHeader.l.normal_area_offset = offset / NML_AREA_ALIGN;

    debug_printf2("normal area offset = %#x\n", offset);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'TWLOFS' Command
//---------------------------------------------------------------------------

static BOOL TWLOFS_Command(char * line, int num)
{
    u32    offset = 0;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    offset = strtoul(line, NULL, 0);

    gcdHeader.l.twl_area_offset = offset / TWL_AREA_ALIGN;

    debug_printf2("twl area offset = %#x\n", offset);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'ARM9_X2' Command
//---------------------------------------------------------------------------

static BOOL ARM9X2_Command(char * line, int num)
{
    char*   dbg_str = "FALSE";

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    if (!strcmp(line, "TRUE"))
    {
        gcdHeader.l.arm9_x2 = TRUE;
        dbg_str = "TRUE";
    }

    debug_printf2("arm9 x2 = %s\n", dbg_str);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'GAMECODE' Command
//---------------------------------------------------------------------------

static BOOL GAMECODE_Command(char * line, int num)
{
    u32     key;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    key = strtoul(line, NULL, 0);

    debug_printf2("keyinfo = %#x\n", key);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'Elf9' Command
//---------------------------------------------------------------------------

static BOOL Elf9_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("arm9 elf = %s\n", line);

    {
        s32 ramAddr = GetRamAddr(line);
        gcdHeader.l.main_ram_address = (void*)ramAddr;
    }

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'Elf7' Command
//---------------------------------------------------------------------------

static BOOL Elf7_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("arm7 elf = %s\n", line);

    {
        s32 ramAddr = GetRamAddr(line);
        gcdHeader.l.sub_ram_address = (void*)ramAddr;
    }

    return CheckResult();
}


static s32 GetRamAddr(const char *fileName)
{
    Elf32_Ehdr *ehdr;
    s32    ramAddr;
    int     file_size;
    struct stat st;

    if (FILESTATUS_FILE != GetFileStatus(&st, fileName))
    {
        error("'%s' is not regular file.", fileName);
        return FALSE;
    }

    if ((file_size = ReadFile(fileName, &ehdr, sizeof(Elf32_Ehdr))) < 0)
        return FALSE;

    ramAddr = ehdr->e_entry;

    Free(&ehdr);

    debug_printf2("ramaddr = 0x%08x\n", ramAddr);

    return ramAddr;
}


//---------------------------------------------------------------------------
//  Output - 'NANDFIRM' Command
//---------------------------------------------------------------------------

static BOOL NandFirm_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("nandfirm = %s\n", line);

    // Set NANDFIRM ROM Offset
    if (!Offset)
    {
        Offset = sizeof(GCDHeader);
        SeekFile(Offset);
    }
    debug_printf2("romoffset = %#x\n", Offset);
    {
        gcdHeader.l.nandfirm_offset = Offset;
        gcdHeader.l.nandfirm_size = ReadFirmFile(line);
    }

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'NORFIRM' Command
//---------------------------------------------------------------------------

static BOOL NorFirm_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("norfirm = %s\n", line);

    // Set NORFIRM ROM Offset
    if (!Offset)
    {
        Offset = sizeof(GCDHeader);
        SeekFile(Offset);
    }
    debug_printf2("romoffset = %#x\n", Offset);
    {
        gcdHeader.l.norfirm_offset = Offset;
        gcdHeader.l.norfirm_size = ReadFirmFile(line);
    }

    return CheckResult();
}


static size_t ReadFirmFile(const char *fileName)
{
    char   *file;
    int     file_size;
    struct stat st;

    if (FILESTATUS_FILE != GetFileStatus(&st, fileName))
    {
        error("'%s' is not regular file.", fileName);
        return FALSE;
    }

    if ((file_size = ReadFileWithPadding(fileName, &file, READ_ALL, FIRM_ALIGN)) < 0)
        return FALSE;

    Offset += file_size;

    // Output file image with fitting region
    PutBuffer(file, file_size);

    Free(&file);

    return file_size;
}


//---------------------------------------------------------------------------
//  Output - 'Sbin9' Command
//---------------------------------------------------------------------------

static BOOL Sbin9_Command(char * line, int num)
{
    FIRMSignedContext* sc = &signedContext;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("arm9 sbin = %s\n", line);

    // Set ARM9 ROM Offset
    if (!Offset)

    {
        Offset = sizeof(GCDHeader);
        SeekFile(Offset);
    }
    debug_printf2("romoffset = %#x\n", Offset);
    {
        gcdHeader.l.main_rom_offset = Offset;
    }

    return ReadSbinFile(line, &gcdHeader.l.main_rom_offset, &sc->hash[FIRM_SIGNED_HASH_IDX_ARM9], compArm9);
}

//---------------------------------------------------------------------------
//  Output - 'Sbin7' Command
//---------------------------------------------------------------------------

static BOOL Sbin7_Command(char * line, int num)
{
    FIRMSignedContext* sc = &signedContext;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    debug_printf2("arm7 sbin = %s\n", line);

    // Set ARM7 ROM Offset
    if (!Offset)
    {
        Offset = sizeof(GCDHeader);
        SeekFile(Offset);
    }
    debug_printf2("romoffset = %#x\n", Offset);
    {
        gcdHeader.l.sub_rom_offset = Offset;
    }

    return ReadSbinFile(line, &gcdHeader.l.sub_rom_offset, &sc->hash[FIRM_SIGNED_HASH_IDX_ARM7], compArm7);
}


static BOOL ReadSbinFile(const char *fileName, void* minfo, void* hash, BOOL comp)
{
    const GCDHeader_ModuleInfo *m  = minfo;
    u32    *size = (void*)&m->size;
    u32    *orig_size = (void*)&m->decomp_size;
    char   *buffer;
    char   *file;
    int     file_size;
    struct stat st;

    if (FILESTATUS_FILE != GetFileStatus(&st, fileName))
    {
        error("'%s' is not regular file.", fileName);
        return FALSE;
    }

    if ((file_size = ReadFile(fileName, &file, READ_ALL)) < 0)
        return FALSE;

    *orig_size = file_size;

    // Digest file image
    if (hash)
    {
        ACSign_DigestUnit(hash, file, file_size);
    }

    // Compress file image with fitting region
    buffer = Alloc(file_size * 2);
    if ( comp )
    {
        file_size = LZCompWrite(file, file_size, buffer, FIRM_ALIGN);
    }
    else
    {
        memcpy(&buffer[0], file, file_size);
        {
            u32 pad_size = (DEFAULT_ALIGN - (file_size % FIRM_ALIGN)) % FIRM_ALIGN;
            if (pad_size)
                memset(&buffer[file_size], 0, pad_size);
            file_size += pad_size;
        }
    }
    Free(&file);
    file = buffer;

    if (size)
    {
        *size = file_size;
    }
    Offset += file_size;

    // Encrypt file image
    EncryptBuffer(file, file_size);

    // Output file image with fitting region
    PutBuffer(file, file_size);

    Free(&file);

    return CheckResult();
}

typedef struct
{
    unsigned long e[4];
} u128;

static BOOL EncryptBuffer(char *buffer, int length)
{
    const u128  id = {{ AES_IDS_ID2_A, AES_IDS_ID2_B, AES_IDS_ID2_C, AES_IDS_ID2_D }};
    u128        iv = {{ length, -length, ~length, 0 }};
    FIRMSignedContext* sc = &signedContext;
    char *buffer2 = Alloc(length);
    AES_KEY key;
    if (!buffer2)
        return FALSE;
    AES_SetKey(&key, sc->aes_key, (unsigned char*)&id);
    AES_Ctr(&key, buffer2, buffer, length, (unsigned char*)&iv);
    memcpy(buffer, buffer2, length);
    memset(buffer2, 0, length);
    Free(buffer2);
    return TRUE;
}


//---------------------------------------------------------------------------
//  Output - 'ERROR' Command
//---------------------------------------------------------------------------
static char* error_type[] =
{
    "SIGN",
    "HEADER_HASH",
    "ARM9_HASH",
    "ARM7_HASH",
    "HASH_TABLE_HASH",
    "FINAL_HASH",
    "HEADER_FOOTER",
};

static BOOL ERROR_Command(char * line, int num)
{
    char*   dbg_str = "UNKNOWN";
    int i;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    for (i=0; i<sizeof(error_type)/sizeof(error_type[0]); i++)
    {
        char* str = error_type[i];

        if (!strcmp(line, str))
        {
            errFlags.raw |= 1 << i;
            dbg_str = str;
        }
    }

    debug_printf2("error type = %s\n", dbg_str);

    return CheckResult();
}

static void SetUnitHashErrors(FIRMSignedContext* sc)
{
    if ( errFlags.e.header_hash )
    {
         sc->hash[FIRM_SIGNED_HASH_IDX_HEADER][0] ^= 1;
    }
    if ( errFlags.e.arm9_hash )
    {
         sc->hash[FIRM_SIGNED_HASH_IDX_ARM9][0] ^= 1;
    }
    if ( errFlags.e.arm7_hash )
    {
         sc->hash[FIRM_SIGNED_HASH_IDX_ARM7][0] ^= 1;
    }
    if ( errFlags.e.hash_table_hash )
    {
         sc->hash[FIRM_SIGNED_HASH_IDX_HASH_TABLE][0] ^= 1;
    }
}

static void SetFinalHashError(FIRMSignedContext* sc)
{
    if ( errFlags.e.final_hash )
    {
         sc->hash[FIRM_SIGNED_HASH_IDX_FINAL][0] ^= 1;
    }
}

static void SetSignError(GCDHeader* nh)
{
    if ( errFlags.e.sign )
    {
         nh->sign.raw[0] ^= 1;
    }

}

static void SetFooterError(GCDHeader* nh)
{
    if ( errFlags.e.header_footer )
    {
         nh->h.reserved_footer[sizeof(nh->h.reserved_footer)-1] ^= 1;
    }

}

//---------------------------------------------------------------------------
//  Output - Initialize AES Key
//---------------------------------------------------------------------------
static BOOL InitializeAesKey(void)
{
    FIRMSignedContext* sc = &signedContext;
    struct stat specstat;
    time_t spectime;
    if (stat(specFileName, &specstat) != 0)
        return FALSE;
    time(&spectime);
    memcpy(&sc->aes_key[0], &specstat.st_atime, 4);
    memcpy(&sc->aes_key[4], &specstat.st_mtime, 4);
    memcpy(&sc->aes_key[8], &specstat.st_ctime, 4);
    memcpy(&sc->aes_key[12], &spectime, 4);
    ACSign_GetKey(sc->aes_key, sizeof(sc->aes_key), sc->aes_key, sizeof(sc->aes_key));
    return TRUE;
}


//---------------------------------------------------------------------------
//  Output - Initialize Gcdfirm File
//---------------------------------------------------------------------------

static BOOL InitializeGcdfirmFile(void)
{
    ReadRomHeaderFile( GetSrcPath(GetAppBaseName(), DEFAULT_ROMHEADER_TEMPLATE) );

    memset(&signedContext.hash[FIRM_SIGNED_HASH_IDX_HASH_TABLE], 0xff, sizeof(signedContext.hash[0]));
    gcdHeader.h.w = wram_regs_init;
    InitializeAesKey();
    return TRUE;
}

//---------------------------------------------------------------------------
//  Output - Finalize Gcdfirm File
//---------------------------------------------------------------------------

static BOOL FinalizeGcdfirmFile(const char *gcdFile)
{
    GCDHeader*  nh = &gcdHeader;
    FIRMSignedContext* sc = &signedContext;
    u8* key = keyFileBuf;
    int size;

    // set rom size
    size = ROM_SIZE_MIN;
    nh->l.rom_size = 0;
    while (size < Offset)
    {
        size <<= 1;
        nh->l.rom_size++;
    }

    ACSign_DigestHeader(&sc->hash[FIRM_SIGNED_HASH_IDX_HEADER], nh);

    SetUnitHashErrors(sc);

    ACSign_DigestUnit(&sc->hash[FIRM_SIGNED_HASH_IDX_FINAL], sc, FIRM_HEADER_2ND_HASH_AREA_LEN);

    SetFinalHashError(sc);

    if (key)
    {
        ACSign_Final(nh, sc, key);
    }

    SetFooterError(nh);

    SetSignError(nh);

    // Output file image
    SeekFile(0L);
    PutBuffer(&gcdHeader, sizeof(gcdHeader));

    // Output public key (modulus)
    if (PubkeyFileName)
    {
        WriteFile(PubkeyFileName, &keyFileBuf[ACS_RSA_PRVMOD_OFFSET], ACS_RSA_PRVMOD_LEN);
    }

    return TRUE;
}
