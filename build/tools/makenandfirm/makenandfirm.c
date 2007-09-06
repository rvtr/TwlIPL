/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenandfirm
  File:     makenandfirm.c

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
#include    <stdlib.h>
#include    <string.h>             // strcasecmp()
#include    <getopt.h>             // getopt()
#include    "makenandfirm.h"
#include    "format_rom.h"
#include    "path.h"
#include    "defval.h"
#include    "version.h"

static int makenandfirm(const char *specFile, const char *nandFile);

//---------------------------------------------------------------------------
//  Main
//---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int     n;
    int     narg;
    char   *nandfirmFile;

    InitAppName(argv[0]);

    while ((n = getopt(argc, argv, "D:hvpd")) != -1)
    {
        switch (n)
        {
        case 'h':
        case 'v':
            goto usage;

        case 'D':
            AddDefVal(optarg);
            break;

        case 'p':
            PrintMode = TRUE;
            break;

        case 'd':
            DebugMode = TRUE;
            break;

        default:
            break;
        }
    }

    narg = argc - optind;
    if (narg > 0)
    {
        // Make SpecFile->NandfirmFile
        nandfirmFile =
            strdup(narg >
                   1 ? argv[optind + 1] : ChangeSuffix(argv[optind], DEFAULT_NANDFIRM_SUFFIX));
        return makenandfirm(argv[optind], nandfirmFile);
    }

  usage:
    {
        char   *makenandfirm = GetAppName();

        fprintf(stderr,
                "NITRO-SDK Development Tool - %s - Make nandfirm file \n"
                "Build %lu\n\n"
                "Usage:  %s [-phv] [-DNAME=VALUE ...] SPECFILE [NANDFIRMFILE]\n\n",
                makenandfirm, SDK_DATE_OF_LATEST_FILE, makenandfirm);
    }
    return 1;
}


//---------------------------------------------------------------------------
//  makenandfirm
//---------------------------------------------------------------------------

static int makenandfirm(const char *specFile, const char *nandFile)
{
    debug_printf("makenandfirm(): '%s' -> '%s'\n", specFile, nandFile);

    // Check identical
    if (specFile && nandFile && !strcasecmp(specFile, nandFile))
    {
        error("nandfirm spec file is identical '%s'", nandFile);
        return 1;
    }

    return OutputNandfirmFile(specFile, nandFile) ? 0 : 1;
}
