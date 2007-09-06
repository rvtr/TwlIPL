/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenandfirm
  File:     misc.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <malloc.h>             // calloc()
#include    <stdlib.h>             // free(), exit()
#include    <io.h>                 // setmode()
#include    <sys/stat.h>           // stat()
#include    <fcntl.h>              // setmode()
#include    <string.h>             // strlen()
#include    <stdarg.h>             // va_start(),va_end()
#include    <time.h>               // localtime()

#include    <nitro/hw/ARM7/mmap_global.h>
#include    "misc.h"

BOOL    DebugMode = FALSE;
BOOL    PrintMode = FALSE;
char   *PubkeyFileName = NULL;

/*---------------------------------------------------------------------------*
 *   File Output Utilities
 *
 *  BOOL    OpenFile( const char* filename )
 *  void    CloseFile( void )
 *  BOOL    CheckResult( void )
 *  void    PutBuffer( const void* ptr, int len )
 *  void    PutByte( u8  c )
 *  void    PutWord( u16 c )
 *  void    PutWord( u32 c )
 *  void    PutString( const char *str )
 *---------------------------------------------------------------------------*/

static FILE *OutFile = NULL;
static const char *FileName = NULL;
static BOOL Status = FALSE;


//
//  File Open
//

BOOL OpenFile(const char *filename)
{
    if (OutFile)
        CloseFile();

    if (filename)
    {
        if (NULL == (OutFile = fopen(filename, "wb+")))
        {
            error("Can't write '%s'", filename);
            Status = FALSE;
            return FALSE;
        }
    }
    else
    {
        setmode(1, O_BINARY);
        OutFile = stdout;              // out to console if filename == NULL
    }
    FileName = filename;
    Status = TRUE;

    return TRUE;
}


//
//  File Close
//

BOOL CloseFile(void)
{
    if (OutFile)
    {
        if (FileName)
        {
            if (fclose(OutFile) == -1)
            {
                error("Can't close '%s'", FileName);
                Status = FALSE;
            }
        }
        else
        {
            setmode(1, O_TEXT);
        }
    }
    OutFile = NULL;

    return Status;
}


//
//  File Seek
//

void SeekFile(long pos)
{
    if (OutFile && fseek(OutFile, pos, SEEK_SET))
    {
        error("Can't seek '%s'", FileName ? FileName : "<stdout>");
        CloseFile();
        Status = FALSE;
    }
}


//
//  Error Check
//

BOOL CheckResult(void)
{
    return Status;
}


//
//  Delete outfile
//

void DeleteOutFile(void)
{
    // Delete outfile
    if (FileName)
    {
        debug_printf("Delete '%s'\n", FileName);
        (void)unlink(FileName);
        FileName = NULL;
    }
    return;
}


//
//  Buffer Output
//

void PutBuffer(const void *ptr, int len)        /* If error, close file */
{
    if (OutFile && len != fwrite(ptr, 1, len, OutFile))
    {
        error("Can't write buffer to '%s'", FileName ? FileName : "<stdout>");
        CloseFile();
        Status = FALSE;
    }
}

//
//  Buffer Input
//

void GetBuffer(void *ptr, int len)     /* If error, close file */
{
    if (OutFile && len != fread(ptr, 1, len, OutFile))
    {
        error("Can't read '%s'", FileName ? FileName : "<stdout>");
        CloseFile();
        Status = FALSE;
    }
}

//
//  Byte/Half/Word Output
//

void PutByte(u8 c)
{
    PutBuffer(&c, 1);
}
void PutHalf(u16 c)
{
    PutBuffer(&c, 2);
}
void PutWord(u32 c)
{
    PutBuffer(&c, 4);
}
void PutString(const char *str)
{
    PutBuffer(str, strlen(str));
}


//
//  Printf
//

void PrintString(const char *fmt, ...)
{
    char   *buffer;
    va_list va;
    int     nchars;
    int     bufsize = FILENAME_MAX;

    while (1)
    {
        buffer = Alloc(bufsize);
        va_start(va, fmt);
        nchars = vsnprintf(buffer, bufsize, fmt, va);
        va_end(va);

        if (0 <= nchars && nchars < bufsize)
        {
            break;
        }

        Free(&buffer);
        bufsize *= 2;
    }

    if (nchars > 0)
    {
        PutBuffer(buffer, nchars);
    }
    Free(&buffer);
}


/*---------------------------------------------------------------------------*
 *   File Read/Write Utilities
 *
 *  int ReadFile( const char* filename, void** buffer )
 *
 *      Read a file to buffer allocated internally
 *              Return read size
 *              Add '\0' at tail of file for help to handle text file
 *
 *  BOOL    WriteFile( const char* filename, void** buffer, int size )
 *---------------------------------------------------------------------------*/

int ReadFile(const char *filename, void *filebuffer, int size)
{
    FILE   *fp;
    struct stat fileStat;
    int     fileSize, readSize;
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

    if (size && (size < fileSize))
    {
        readSize = size;
    }
    else
    {
        readSize = fileSize;
    }

    /* Read file */
    *buffer = Alloc(readSize + 1);     /* error handle is done in Alloc()       */
    /* size+1 for '\0' to handle text file */
    if (readSize != fread(*buffer, sizeof(char), readSize, fp))
    {
        goto error_free_close;
    }

    (*(char **)buffer)[readSize] = '\0';        /* Works as terminater if file is text file */

    /* Close file */
    fclose(fp);
    return readSize;

  error_free_close:
    Free(buffer);
    fclose(fp);
  error:
    error("Can't read '%s'", filename);
    return -1;
}


int ReadFileWithPadding(const char *filename, void *filebuffer, int size, int boundary)
{
    FILE   *fp;
    struct stat fileStat;
    int     fileSize, readSize, padSize;
    void  **buffer = (void **)filebuffer;

    if (!boundary)
    {
        return ReadFile(filename, buffer, size);
    }

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

    if (size && (size < fileSize))
    {
        readSize = size;
    }
    else
    {
        readSize = fileSize;
    }

    padSize = (boundary - (readSize & (boundary-1))) & (boundary-1);

    /* Read file */
    *buffer = Alloc(readSize + padSize);     /* error handle is done in Alloc()       */
    /* size+1 for '\0' to handle text file */
    if (readSize != fread(*buffer, sizeof(char), readSize, fp))
    {
        goto error_free_close;
    }

    memset((char*)*buffer + readSize, 0, padSize);

    /* Close file */
    fclose(fp);
    return readSize + padSize;

  error_free_close:
    Free(buffer);
    fclose(fp);
  error:
    error("Can't read '%s'", filename);
    return -1;
}


BOOL WriteFile(const char *filename, void *buffer, int size)
{
    debug_printf("WriteFile %s\n", filename);

    (void)OpenFile(filename);
    PutBuffer(buffer, size);
    CloseFile();
    Free(&buffer);
    return CheckResult();
}


/*---------------------------------------------------------------------------*
 *    Time Format Utilities
 *
 *  char*   GetGMTime( const time_t time )  Show GMT
 *  char*   GetTime( const time_t time )    Show local Time
 *---------------------------------------------------------------------------*/

char   *GetGMTime(const time_t time)
{
    static char timebuffer[32];
    strftime(timebuffer, sizeof(timebuffer), "%y/%m/%d-%H:%M:%S", gmtime(&time));
    return timebuffer;
}


char   *GetTime(const time_t time)
{
    static char timebuffer[32];
    strftime(timebuffer, sizeof(timebuffer), "%y/%m/%d-%H:%M:%S", localtime(&time));
    return timebuffer;
}


/*---------------------------------------------------------------------------*
 *   Memory Allocation Utilities
 *
 *  void*   Alloc( size_t size )
 *---------------------------------------------------------------------------*/

void   *Alloc(size_t size)
{
    void   *t = calloc(1, size);

    if (t == NULL)
    {
        error("Can't allocate memory.");
        exit(10);
    }
    return t;
}


void Free(void *p)
{
    void  **ptr = (void **)p;

    if (*ptr)
    {
        free(*ptr);
        (*ptr) = NULL;
    }
}


/*---------------------------------------------------------------------------*
 *   VBuffer
 *
 *  void    PutVBuffer( VBuffer* vbuf, char c )
 *---------------------------------------------------------------------------*/

void PutVBuffer(VBuffer * vbuf, char c)
{
    int     size;
    char   *tmp_buffer;

    if (vbuf->buffer == 0)
    {
        vbuf->size = VBUFFER_INITIAL_SIZE;
        vbuf->buffer = Alloc(vbuf->size);       // buffer is CALLOCed
    }
    size = strlen(vbuf->buffer);

    if (size >= vbuf->size - 1)
    {
        // Need buffer expansion
        vbuf->size *= 2;
        tmp_buffer = Alloc(vbuf->size); // buffer is CALLOCed
        strcpy(tmp_buffer, vbuf->buffer);
        Free(&vbuf->buffer);
        vbuf->buffer = tmp_buffer;
    }
    vbuf->buffer[size] = c;
    return;
}

char   *GetVBuffer(VBuffer * vbuf)
{
    return vbuf->buffer;
}

void InitVBuffer(VBuffer * vbuf)
{
    vbuf->buffer = 0;
    vbuf->size = 0;
}

void FreeVBuffer(VBuffer * vbuf)
{
    Free(&vbuf->buffer);
}


/*---------------------------------------------------------------------------*
 *   File Path Utilities
 *
 *  char*  ChangeBackSlash( char* path )
 *---------------------------------------------------------------------------*/

char   *ChangeBackSlash(char *path)
{
    char   *p = path;

    while (*p)
    {
        if (*p == '\\')
        {
            *p = '/';
        }
        p++;
    }
    return path;
}


/*---------------------------------------------------------------------------*
 *   Math
 *
 *  u16 CalcCRC16( u16 start, u8 *data, int size )
 *---------------------------------------------------------------------------*/

static u16 crc16_table[16] = {
    0x0000, 0xCC01, 0xD801, 0x1400,
    0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401,
    0x5000, 0x9C01, 0x8801, 0x4400
};

u16 CalcCRC16(u16 start, u8 *data, int size)
{
    u16     r1;
    u16     total = start;

    while (size-- > 0)
    {
        // ‰ºˆÊ4bit
        r1 = crc16_table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ crc16_table[*data & 0xf];

        // ãˆÊ4bit
        r1 = crc16_table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ crc16_table[(*data >> 4) & 0xf];

        data++;
    }
    return total;
}


/*---------------------------------------------------------------------------*
 *   for firm header
 *
 *---------------------------------------------------------------------------*/

static u8 ConvertAlign( u32 ofs )
{
    u8 i;

    ofs /= 4;
    for (i=0; i<7; i++)
    {
        if ( ofs & 1 )
        {
            break;
        }
        ofs >>= 1;
    }

    return  i;
}

tROMAddrConvType ConvertHeaderRamAddr( s32 p )
{
    tROMAddrConvType retval;

    retval.align = ConvertAlign( p );
    retval.address = (u16)((p - HW_WRAM)/(4<<retval.align));

    return  retval;
}

tROMAddrConvType ConvertHeaderRomOffset( s32 p )
{
    tROMAddrConvType retval;

    retval.align = ConvertAlign( p );
    retval.address = (u16)(p/(4<<retval.align));

    return  retval;
}

u16 ConvertHeaderRomOffsetAlign( s32 p, u32 align )
{
    return  (u16)(p/align);
}


/*---------------------------------------------------------------------------*
 *   String
 *
 *  char*  WrapNull( char* str )
 *---------------------------------------------------------------------------*/

const char *WrapNull(const char *str)
{
    return str ? str : "*";            // return NULL String if NULL pointer
}


/*---------------------------------------------------------------------------*
 *  Debug print
 *  void   debug_printf( char* fmt, ... )
 *---------------------------------------------------------------------------*/

void debug_printf(const char *fmt, ...)
{
    va_list va;

    if (DebugMode)
    {
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}

void debug_printf2(const char *fmt, ...)
{
    va_list va;

    if (PrintMode || DebugMode)
    {
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}
