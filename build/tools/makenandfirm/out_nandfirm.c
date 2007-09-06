/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenandfirm
  File:     out_nandfirm.c

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
#include    "makenandfirm.h"
#include    "format_sign.h"
#include    "acsign_nand.h"
#include    "compress.h"

#define SDK_ASM

#include    <firm/aes/ARM7/aes_ids.h>
#include    "../acsign/aes2.h"

#define SBIN9CMD   "ARM9_SBIN"
#define SBIN7CMD   "ARM7_SBIN"
#define ELF9CMD    "ARM9_ELF"
#define ELF7CMD    "ARM7_ELF"
#define COMP9CMD   "ARM9_COMP"
#define COMP7CMD   "ARM7_COMP"
#define DECOMPCMD  "DECOMP_PROC"
#define ARM9X2CMD  "ARM9_X2"
#define VERCMD     "VERSION"
#define RSAKEYCMD  "RSA_KEY"
#define OUTKEYCMD  "OUT_KEY"
#define WREGCMD    "WRAM_RBIN"
#define ENDKEYCMD  "ENC_KEYINFO"
#define NCDCMD     "NCD_ROMOFS"
#define MIRRORCMD  "MIRROR_OFS"
#define ERRCMD     "ERROR"

static BOOL ConstructNandfirmFile(char * specFile);
static BOOL ReadSbinFile(const char *fileName, void* minfo, void* minfo2, void* hash, BOOL comp);
static BOOL ReadKeyFile(const char *fileName);
static BOOL ReadWramRegFile(const char *fileName);
static s32 GetRamAddr(const char *fileName);

static BOOL EncryptBuffer(char *buffer, int length);

static BOOL Sbin9_Command(char * line, int num);
static BOOL Sbin7_Command(char * line, int num);
static BOOL Elf9_Command(char * line, int num);
static BOOL Elf7_Command(char * line, int num);
static BOOL Comp9_Command(char * line, int num);
static BOOL Comp7_Command(char * line, int num);
static BOOL Decomp_Command(char * line, int num);
static BOOL ARM9X2_Command(char * line, int num);
static BOOL VERSION_Command(char * line, int num);
static BOOL RSAKEY_Command(char * line, int num);
static BOOL OUTKEY_Command(char * line, int num);
static BOOL WramRegs_Command(char * line, int num);
static BOOL ENCKEY_Command(char * line, int num);
static BOOL NcdOffset_Command(char * line, int num);
static BOOL MirrorOffset_Command(char * line, int num);
static BOOL ERROR_Command(char * line, int num);

static BOOL InitializeAesKey(void);
static BOOL InitializeNandfirmFile(void);
static BOOL FinalizeNandfirmFile(const char *nandFile);

static s32 Offset;                     // Current offset
static int LineNum;                    // Line number   for error message
static const char *specFileName;       // specFile name for error message

NANDHeaderEx  nandHeader;                 // Nandfirm Header Shadow
FIRMSignedContext signedContext;
u8 *keyFileBuf;
BOOL compArm9 = TRUE;
BOOL compArm7 = TRUE;
u32  mirrorOfs = 0;
tErrorFlags errFlags;

//---------------------------------------------------------------------------
//  Output - nandfirm File
//---------------------------------------------------------------------------

BOOL OutputNandfirmFile(const char *specFile, const char *nandFile)
{
    char   *buffer;
    BOOL    state;

    if (ReadFile(specFile, &buffer, READ_ALL) <= 0)
    {
        return FALSE;
    }

    if (!OpenFile(nandFile))
    {
        return FALSE;
    }

    specFileName = specFile;

    state = InitializeNandfirmFile() && ConstructNandfirmFile(buffer) &&
            FinalizeNandfirmFile(nandFile) && CloseFile();

    if (!state)
    {
        DeleteOutFile();
    }

    return state;
}


//---------------------------------------------------------------------------
//  Output - Nandfirm File
//---------------------------------------------------------------------------

static const tCommandDesc command[] = {
    {SBIN9CMD, Sbin9_Command}, {SBIN7CMD, Sbin7_Command},
    {ELF9CMD, Elf9_Command},{ELF7CMD, Elf7_Command},
    {COMP9CMD, Comp9_Command},{COMP7CMD, Comp7_Command},
    {DECOMPCMD, Decomp_Command},
    {VERCMD, VERSION_Command},
    {ARM9X2CMD, ARM9X2_Command},
    {RSAKEYCMD, RSAKEY_Command},
    {OUTKEYCMD, OUTKEY_Command},
    {WREGCMD, WramRegs_Command},
    {ENDKEYCMD, ENCKEY_Command},
    {NCDCMD, NcdOffset_Command},
    {MIRRORCMD, MirrorOffset_Command},
    {ERRCMD, ERROR_Command},
};

BOOL ConstructNandfirmFile(char * specFile)
{
    char *line;
    char *line_top;
    char *p;
    int     i;

    LineNum = 0;
    Offset = 0x00000000;

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
        debug_printf("NANDSF Line%4d [", LineNum, line);
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

    nandHeader.o.h.w = *wram_regs;
    nandHeader.m.h.w = *wram_regs;

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
//  Output - 'Decomp' Command
//---------------------------------------------------------------------------

static BOOL Decomp_Command(char * line, int num)
{
    char*   dbg_str = "ARM7";

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    if (!strcmp(line, "ARM9"))
    {
        nandHeader.o.l.arm9_decomp = TRUE;
        nandHeader.m.l.arm9_decomp = TRUE;
        dbg_str = "ARM9";
    }

    debug_printf2("decompress processor = %s\n", dbg_str);

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
        nandHeader.o.l.arm9_x2 = TRUE;
        nandHeader.m.l.arm9_x2 = TRUE;
        dbg_str = "TRUE";
    }

    debug_printf2("arm9 x2 = %s\n", dbg_str);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'Comp9' Command
//---------------------------------------------------------------------------

static BOOL Comp9_Command(char * line, int num)
{
    char*   dbg_str = "FALSE";

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    compArm9 = FALSE;

    if (!strcmp(line, "TRUE"))
    {
        nandHeader.o.l.comp_arm9_boot_area = TRUE;
        nandHeader.m.l.comp_arm9_boot_area = TRUE;
        compArm9 = TRUE;
        dbg_str = "TRUE";
    }

    debug_printf2("arm9 compress = %s\n", dbg_str);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'Comp7' Command
//---------------------------------------------------------------------------

static BOOL Comp7_Command(char * line, int num)
{
    char*   dbg_str = "FALSE";

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    compArm7 = FALSE;

    if (!strcmp(line, "TRUE"))
    {
        nandHeader.o.l.comp_arm7_boot_area = TRUE;
        nandHeader.m.l.comp_arm7_boot_area = TRUE;
        compArm7 = TRUE;
        dbg_str = "TRUE";
    }

    debug_printf2("arm7 compress = %s\n", dbg_str);

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
        nandHeader.o.l.main_ram_address = (void*)ramAddr;
        nandHeader.m.l.main_ram_address = (void*)ramAddr;
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
        nandHeader.o.l.sub_ram_address = (void*)ramAddr;
        nandHeader.m.l.sub_ram_address = (void*)ramAddr;
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
        Offset = (sizeof(NANDHeader) + FIRM_ALIGN_MASK) & ~FIRM_ALIGN_MASK;
        if (mirrorOfs)
        {
            Offset += sizeof(NANDHeaderCore) * 2;
        }
        SeekFile(Offset);
    }
    debug_printf2("romoffset = %#x\n", Offset);
    {
        nandHeader.o.l.main_rom_offset = Offset;
        nandHeader.m.l.main_rom_offset = Offset + mirrorOfs - sizeof(NANDHeaderCore) * 2;
    }

    return ReadSbinFile(line, &nandHeader.o.l.main_rom_offset, &nandHeader.m.l.main_rom_offset, &sc->hash[FIRM_SIGNED_HASH_IDX_ARM9], compArm9);
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
        Offset = (sizeof(NANDHeader) + FIRM_ALIGN_MASK) & ~FIRM_ALIGN_MASK;
        if (mirrorOfs)
        {
            Offset += sizeof(NANDHeaderCore) * 2;
        }
        SeekFile(Offset);
    }
    debug_printf2("romoffset = %#x\n", Offset);
    {
        nandHeader.o.l.sub_rom_offset = Offset;
        nandHeader.m.l.sub_rom_offset = Offset + mirrorOfs - sizeof(NANDHeaderCore) * 2;
    }

    return ReadSbinFile(line, &nandHeader.o.l.sub_rom_offset, &nandHeader.m.l.sub_rom_offset, &sc->hash[FIRM_SIGNED_HASH_IDX_ARM7], compArm7);
}


static BOOL ReadSbinFile(const char *fileName, void* minfo, void* minfo2, void* hash, BOOL comp)
{
    const NANDHeader_ModuleInfo *m  = minfo;
    u32    *size = (void*)&m->size;
    u32    *orig_size = (void*)&m->decomp_size;
    const NANDHeader_ModuleInfo *m2  = minfo2;
    u32    *size2 = (void*)&m2->size;
    u32    *orig_size2 = (void*)&m2->decomp_size;
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

    *orig_size = *orig_size2 = file_size;

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
            u32 pad_size = (FIRM_ALIGN - (file_size % FIRM_ALIGN)) % FIRM_ALIGN;
            if (pad_size)
                memset(&buffer[file_size], 0, pad_size);
            file_size += pad_size;
        }
    }
    Free(&file);
    file = buffer;

    if (size)
    {
        *size = *size2 = file_size;
    }
    Offset += file_size;

    // Encrypt file image
    EncryptBuffer(file, file_size);

    // Output file image with fitting region
    PutBuffer(file, file_size);

    Free(&file);

    if (mirrorOfs && mirrorOfs < Offset + sizeof(NANDHeaderCore) * 2)
    {
        error("mirrorOfs is too small (%ld < %ld).\n", mirrorOfs, Offset + sizeof(NANDHeaderCore) * 2);
        return FALSE;
    }

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
//  Output - 'ENC_KEYINFO' Command
//---------------------------------------------------------------------------

static BOOL ENCKEY_Command(char * line, int num)
{
    u32     key;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    key = strtoul(line, NULL, 0);
    nandHeader.g.d.ds_key = key;

    debug_printf2("keyinfo = %#x\n", key);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'NcdOffset' Command
//---------------------------------------------------------------------------

static BOOL NcdOffset_Command(char * line, int num)
{
    u32     ofs;

    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    ofs = strtoul(line, NULL, 0);
    {
        nandHeader.g.d.ncd_romAdr = ConvertHeaderRomOffsetAlign(ofs, 8);
    }

    debug_printf2("ncd romoffset = %#x\n", ofs);

    return CheckResult();
}


//---------------------------------------------------------------------------
//  Output - 'MirrorOffset' Command
//---------------------------------------------------------------------------

static BOOL MirrorOffset_Command(char * line, int num)
{
    if (num != 1)
    {
        error("Wrong format spec file '%s' line:%d", specFileName, LineNum);
        return FALSE;
    }

    mirrorOfs = strtoul(line, NULL, 0);

    debug_printf2("mirrored image offset = %#x\n", mirrorOfs);

    return CheckResult();
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

static void SetSignError(NANDHeader* nh)
{
    if ( errFlags.e.sign )
    {
         nh->sign.raw[0] ^= 1;
    }

}

static void SetFooterError(NANDHeader* nh)
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
//  Output - Initialize Nandfirm File
//---------------------------------------------------------------------------

static BOOL InitializeNandfirmFile(void)
{
    memset(&signedContext.hash[FIRM_SIGNED_HASH_IDX_HASH_TABLE], 0xff, sizeof(signedContext.hash[0]));
    nandHeader.o.h.w = wram_regs_init;
    nandHeader.m.h.w = wram_regs_init;
    InitializeAesKey();
    return TRUE;
}


//---------------------------------------------------------------------------
//  Output - Finalize Nandfirm File
//---------------------------------------------------------------------------

static void FinalizeNandfirmFileCore(NANDHeaderCore* master)
{
    NANDHeader *nh = &nandHeader.g;
    FIRMSignedContext* sc = &signedContext;
    u8* key = keyFileBuf;

    memcpy(&nh->l, &master->l, sizeof(NANDHeaderLow));
    memcpy(&nh->h, &master->h, sizeof(NANDHeaderHigh));

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

    memcpy(&master->sign, &nh->sign, sizeof(FIRMPaddedSign));
}

static BOOL FinalizeNandfirmFile(const char *nandFile)
{
    // for m header (copy to g)
    FinalizeNandfirmFileCore(&nandHeader.m);

    // for o header (copy to g)
    FinalizeNandfirmFileCore(&nandHeader.o);

    // Output file image
    SeekFile(0L);
    if (mirrorOfs)
    {
        PutBuffer(&nandHeader, sizeof(nandHeader));
    }
    else
    {
        PutBuffer(&nandHeader.g, sizeof(nandHeader.g));
    }

    // Output public key (modulus)
    if (PubkeyFileName)
    {
        WriteFile(PubkeyFileName, &keyFileBuf[ACS_RSA_PRVMOD_OFFSET], ACS_RSA_PRVMOD_LEN);
    }

    return TRUE;
}
