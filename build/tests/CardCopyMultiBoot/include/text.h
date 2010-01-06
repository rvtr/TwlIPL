/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - demos - wbt-1
  File:     font.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef _TEXT_H_
#define _TEXT_H_

#define NUM_OF_SCREEN 4

#define X_LINE_MAX (32)
#define Y_LINE_MAX (24)

#define LINE_BUF_X_SIZE  32
#define LINE_BUF_Y_SIZE  24


typedef struct LINE_BUF_s
{
    int     buf[LINE_BUF_X_SIZE];
    u16     col[LINE_BUF_X_SIZE];
    int     last_count;
    int     cur_count;
    struct LINE_BUF_s *prev;
    struct LINE_BUF_s *next;
}
LINE_BUF;

typedef struct
{
    int    *num_x, *num_y;
    LINE_BUF *start;
    LINE_BUF *cur;
}
TEXT_BUF;




typedef struct
{
    TEXT_BUF text_buf;
    u16    *screen;
    u16     x_line;
    u16     y_line;
    u16     palette;
    u16     padding;
}
TEXT_CTRL;


int     init_text_buf_sys(void *heap_start, void *heap_end);


void    m_putchar(TEXT_CTRL * tc, int c);
void    m_set_palette(TEXT_CTRL * tc, u16 num);
u16     m_get_palette(TEXT_CTRL * tc);
void    init_text(TEXT_CTRL * tc, u16 *screen, u16 palette);


void    text_buf_to_vram(TEXT_CTRL * tc);


extern TEXT_CTRL *tc[NUM_OF_SCREEN];

void    mfprintf(TEXT_CTRL * tc, const char *fmt, ...);
void    mprintf(const char *fmt, ...);

#endif /* _TEXT_H_ */
