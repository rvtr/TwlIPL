/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compBLZ
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include     <stdio.h>
#include     <stdlib.h>
#include     <string.h>
#include     <unistd.h>                // getopt()
#include     "file.h"
#include     "compress.h"

extern const unsigned long SDK_DATE_OF_LATEST_FILE;

static void usage(void);

/*---------------------------------------------------------------------------*
  Name:         main

  Description:
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int     n;
    int     result;
    int     iptfilesize;
    
    char*   optfilename;
    char*   suffix;
    u8*     filebuf;
    
    BOOL    optfname_flag = FALSE;    // if optfilename input, this flag is true.
    
    optfilename = suffix = NULL;

    while ((n = getopt(argc, argv, "o:e:dhv")) != -1)
    {
        switch (n)
        {
        case 'o':                     // output file name
            optfilename = optarg;
            optfname_flag = TRUE;
            break;
        
        case 'e':
            suffix = optarg;
            break;
        
        case 'd':                     // Show debug message
            bDebugMode = TRUE;
            break;

        case 'h':
        case 'v':
        default:
            usage();                   // Never returns
            break;
        }

        DebugPrintf("option -%c: %s\n", n, optarg ? optarg : "No ARG");
    }

    argc -= optind;
    argv += optind;

    if (bDebugMode)
    {
        int     i;

        DebugPrintf("argc=%d  optind=%d\n", argc, optind);
        for (i = 0; i < argc; i++)
        {
            DebugPrintf("argv[%d] = [%s]\n", i, argv[i]);
        }
    }

    if (argc == 1)
    {
        if (optfilename == NULL)
        {
            optfilename = argv[0];
        }
        iptfilesize = ReadFile(argv[0], &filebuf);
    }
    else
    {
        usage();                         // Never returns
    }
    
    if (iptfilesize < 0)
    {
        ConsolePrintf("exit...\n");
        return 1;
    }
    
    if ((result = Compress(filebuf, iptfilesize)) < 0)
    {
        switch (result)
        {
        case COMPRESS_LARGER_ORIGINAL:
            ConsolePrintf("Inputdata ..... Not compressed (enlarged or same size as before)\n");
            break;
        case COMPRESS_FATAL_ERROR:
            ConsolePrintf("Fatal error occured\n");
            break;
        }
        ConsolePrintf("exit...\n");
        return 1;
    }
    
    // cut file path and suffix of input file
    if (!optfname_flag)
    {
        optfilename = StrCutFname(optfilename);
    }
    
    // create output filename
    if (suffix == NULL)
    {
        optfilename = StrCat(2, optfilename, "_BLZ.bin");
    }
    else
    {
        optfilename = StrCat(2, optfilename, suffix);
    }
    
    // output file
    if (WriteFile(optfilename, filebuf, result) < 0)
    {
        ConsolePrintf("exit...\n\n");
        return 1;
    }
        
    ConsolePrintf("Inputdata ..... Compressed ... %9d -> %9d\n", iptfilesize, result);
    
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         usage

  Description:
 *---------------------------------------------------------------------------*/
static void usage(void)
{
    fprintf(stderr,
            "TWL-SDK Development Tool - compBLZ - Compress data\n"
            "Build %lu\n"
            "\n"
            "Usage: compBLZ [-d] [-o outputFile] [-e suffix] inputFile\n"
            "\n"
            "  Compress data (backward LZ)\n"
            "\n"
            "    -o outputFile FILENAME for output file   (default:input filename)\n"
            "    -e suffix     SUFFIX for output file (default:\"_BLZ\")\n"
            "    -d            Show debug messages (for test purpose)\n"
            "    -h            Show this  message\n" "\n", SDK_DATE_OF_LATEST_FILE);

    exit(1);
}
