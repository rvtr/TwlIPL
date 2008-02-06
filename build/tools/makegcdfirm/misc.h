/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenorfirm
  File:     misc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef MISC_H_
#define MISC_H_
#include    <time.h>

typedef enum
{
    FALSE = 0,
    TRUE = 1
}
BOOL;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
typedef signed long long s64;
typedef volatile u8 vu8;
typedef volatile u32 vu32;
typedef vu32 REGType32v;

#define error(...)  do { fprintf(stderr, "Error: ");   \
                             fprintf(stderr, __VA_ARGS__); \
                             fprintf(stderr, "\n"); } while(0)

#define warning(...)    do { fprintf(stderr, "Warning: "); \
                             fprintf(stderr, __VA_ARGS__); \
                             fprintf(stderr, "\n"); } while(0)

BOOL    OpenFile(const char *filename);
BOOL    CloseFile(void);
void    SeekFile(long pos);
BOOL    CheckResult(void);
void    DeleteOutFile(void);
void    PutBuffer(const void *ptr, int len);
void    GetBuffer(void *ptr, int len);
void    PutByte(u8 c);
void    PutHalf(u16 c);
void    PutWord(u32 c);
void    PutString(const char *str);
void    PrintString(const char *fmt, ...);

#define READ_ALL  0
int     ReadFile(const char *filename, void *filebuffer, int size);
int     ReadFileWithPadding(const char *filename, void *filebuffer, int size, int boundary);
BOOL    WriteFile(const char *filename, void *buffer, int size);

char   *GetGMTime(const time_t time);
char   *GetTime(const time_t time);

void   *Alloc(size_t size);
void    Free(void *p);

typedef struct
{
    char   *buffer;
    int     size;
}
VBuffer;

#define VBUFFER_INITIAL_SIZE    1024
void    InitVBuffer(VBuffer * vbuf);
void    FreeVBuffer(VBuffer * vbuf);
void    PutVBuffer(VBuffer * vbuf, char c);
char   *GetVBuffer(VBuffer * vbuf);

char   *ChangeBackSlash(char *path);

u16     CalcCRC16(u16 start, u8 *data, int size);
const char *WrapNull(const char *str);

typedef struct
{
    u16     address;
    u8      align;
}
tROMAddrConvType;

tROMAddrConvType ConvertHeaderRamAddr( s32 p );
tROMAddrConvType ConvertHeaderRomOffset( s32 p );
u16 ConvertHeaderRomOffsetAlign( s32 p, u32 align );

typedef union
{
    struct
    {
        u32     sign:1;
        u32     header_hash:1;
        u32     arm9_hash:1;
        u32     arm7_hash:1;
        u32     hash_table_hash:1;
        u32     final_hash:1;
        u32     header_footer:1;
    }
    e;
    u32         raw;
}
tErrorFlags;

extern BOOL DebugMode;
extern BOOL PrintMode;
extern char *PubkeyFileName;
void    debug_printf(const char *str, ...);
void    debug_printf2(const char *str, ...);

#endif //MISC_H_
