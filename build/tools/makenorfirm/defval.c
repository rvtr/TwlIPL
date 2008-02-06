/*---------------------------------------------------------------------------*
  Project:  NitroSDK - tools - makerom
  File:     defval.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: defval.c,v $
  Revision 1.10  2006/01/18 02:11:19  kitase_hirotake
  do-indent

  Revision 1.9  2005/02/28 05:26:03  yosizaki
  do-indent.

  Revision 1.8  2004/08/05 13:50:13  yasu
  Support -M option

  Revision 1.7  2004/06/29 04:55:40  yasu
  Use VBuffer to resolve variables

  Revision 1.6  2004/06/23 07:51:02  yasu
  fix a bug as illegal memory freeing at ResolveDefVal()

  Revision 1.5  2004/05/27 00:40:49  yasu
  care also about current directory (dot ".")

  Revision 1.4  2004/05/27 00:25:46  yasu
  care about double-dots ".." for defvalue option :r, :e

  Revision 1.3  2004/05/27 00:11:19  yasu
  fix a error when searching a "dot" of file extension

  Revision 1.2  2004/05/26 12:02:47  yasu
  support :h, :t, :r, :e option for variable name

  Revision 1.1  2004/03/26 05:06:45  yasu
  support variables like as -DNAME=VALUE

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<stdlib.h>             // getenv()
#include	<string.h>             // strcasecmp()
#include	<getopt.h>             // getopt()
#include	"misc.h"
#include	"defval.h"

typedef struct tValdef
{
    struct tValdef *next;
    char   *name;
    char   *value;
}
tValdef;

tValdef *valdef_top = NULL;


//
//  Add new define value via file
//
//      opt : "DEFINE=VALUE"
//
BOOL AddDefValFromFile(char *filename)
{
    char   *buffer;
    int     buffer_size;
    int     read_size;
    FILE   *fp;

    if (filename[0] == '-' && filename[1] == '\0')
    {
        fp = stdin;
    }
    else if (NULL == (fp = fopen(filename, "rb")))
    {
        fprintf(stderr, "Cannot open file \"%s\".\n", filename);
        return FALSE;
    }

    buffer_size = DEFVAL_DEFAULT_BUFFER_SIZE;

    if (NULL == (buffer = malloc(buffer_size)))
    {
        fprintf(stderr, "Cannot allocate memory.\n");
        return FALSE;
    }

    read_size = 0;

    while (NULL != fgets(buffer + read_size, buffer_size - read_size, fp))
    {
        read_size = strlen(buffer);

        if (read_size == buffer_size - 1 && buffer[read_size - 1] != '\n')
        {
            buffer_size *= 2;

            if (NULL == (buffer = realloc(buffer, buffer_size)))
            {
                fprintf(stderr, "Cannot allocate memory.\n");
                return FALSE;
            }
            continue;
        }

        AddDefVal(buffer);
        read_size = 0;
    }

    if (fp != stdin)
    {
        fclose(fp);
    }
    free(buffer);

    return TRUE;
}


//
//  Add new define value
//
//      opt : "DEFINE=VALUE"
//
void AddDefVal(char *opt)
{
    int     i;
    tValdef *t;

    for (i = 0;; i++)
    {
        if ('=' == opt[i] || '\0' == opt[i])
        {
            break;
        }
    }

    if (i > 0)
    {
        t = Alloc(sizeof(tValdef));
        t->name = strncpy(Alloc(i + 1), opt, i);
        t->name[i] = '\0';

        if (opt[i] == '=')
        {
            i++;
        }
        t->value = strdup(opt + i);

        t->next = valdef_top;
        valdef_top = t;

        debug_printf("DEFINE:$(%s)=\"%s\"\n", t->name, t->value);
    }
    return;
}

//
//  Search define value
//
//      Return: value of specified name
//
char   *SearchDefVal(char *name)
{
    tValdef *t;

    for (t = valdef_top; t; t = t->next)
    {
        if (!strcmp(t->name, name))
        {
            return t->value;
        }
    }

    return getenv(name);
}


//
//  Search define value and Modify it by : option
//
//      Return: duplicated value of specified name modified by :x option
//
char   *SearchDefValWithOption(char *name)
{
    int     len_name = strlen(name);
    char   *value;
    char    option = '\0';

    if (len_name > 2 && name[len_name - 2] == ':')
    {
        name[len_name - 2] = '\0';
        option = name[len_name - 1];
    }

    value = SearchDefVal(name);

    if (value)
    {
        int     value_len = strlen(value);
        int     col_dot = value_len;
        int     col_filename = 0;
        int     i;

        for (i = 0; i < value_len; i++)
        {
            switch (value[i])
            {
            case '.':
                if (col_filename == i &&
                    (value[i + 1] == '\0' || (value[i + 1] == '.' && value[i + 2] == '\0')))
                {
                    i = value_len;     // exit loop if last entry is . or ..
                }
                else
                {
                    col_dot = i;       // Save the last dot column
                }
                break;

            case '/':
            case '\\':
            case ':':
                col_filename = i + 1;  // Save the last filename
                col_dot = value_len;   // Reset dot position
                break;

            default:
                ;
            }
        }

        switch (option)
        {
        case 'h':                     // Dirname with the last slash
            value = strdup(value);
            value[col_filename] = '\0';
            break;

        case 't':                     // Filename
            value = strdup(value + col_filename);
            break;

        case 'r':                     // All without . file extension
            value = strdup(value);
            value[col_dot] = '\0';
            break;

        case 'e':                     // File extension
            value = strdup(value + col_dot + 1);
            break;

        default:
            value = strdup(value);
        }
    }
    return value;
}


//
//  Resolve define value
//
//      Return: new string
//
char   *ResolveDefVal(char *str)
{
    int     i, j;
    char   *val;
    VBuffer buf;

    InitVBuffer(&buf);

    for (i = 0; '\0' != str[i]; i++)
    {
        // search $(XXX)
        if ('$' == str[i] && '(' == str[i + 1])
        {
            for (j = i + 2; '\0' != str[j]; j++)
            {
                if (')' == str[j])
                {
                    str[j] = '\0';

                    // get value of XXX
                    val = SearchDefValWithOption(&str[i + 2]);

                    // copy value of XXX
                    if (val)
                    {
                        char   *s = val;

                        while (*s)
                        {
                            PutVBuffer(&buf, *s);
                            s++;
                        }
                        free(val);
                    }
                    i = j;
                    goto next;
                }
            }
        }
        PutVBuffer(&buf, str[i]);
      next:;
    }
    return GetVBuffer(&buf);           // pass allocated buffer, should be freed by caller
}
