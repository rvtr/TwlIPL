/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenorfirm
  File:     makenorfirm.c

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
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>             // strcasecmp()
#include	<getopt.h>             // getopt()
#include	"makenorfirm.h"
#include	"format_rom.h"
#include	"path.h"
#include	"defval.h"
#include	"version.h"

static int makenorfirm(const char *specFile, const char *norFile);

//---------------------------------------------------------------------------
//  Main
//---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int     n;
    int     narg;
    char   *norfirmFile;

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
        // Make SpecFile->NorfirmFile
        norfirmFile =
            strdup(narg >
                   1 ? argv[optind + 1] : ChangeSuffix(argv[optind], DEFAULT_NORFIRM_SUFFIX));
        return makenorfirm(argv[optind], norfirmFile);
    }

  usage:
    {
        char   *makenorfirm = GetAppName();

        fprintf(stderr,
                "NITRO-SDK Development Tool - %s - Make norfirm file \n"
                "Build %lu\n\n"
                "Usage:  %s [-phv] [-DNAME=VALUE ...] SPECFILE [NORFIRMFILE]\n\n",
                makenorfirm, SDK_DATE_OF_LATEST_FILE, makenorfirm);
    }
    return 1;
}


//---------------------------------------------------------------------------
//  makenorfirm
//---------------------------------------------------------------------------

static int makenorfirm(const char *specFile, const char *norFile)
{
    debug_printf("makenorfirm(): '%s' -> '%s'\n", specFile, norFile);

    // Check identical
    if (specFile && norFile && !strcasecmp(specFile, norFile))
    {
        error("norfirm spec file is identical '%s'", norFile);
        return 1;
    }

    return OutputNorfirmFile(specFile, norFile) ? 0 : 1;
}
