/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tools - rombreaker
  File:     rombreaker.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::
  $Rev:
  $Author:
 *---------------------------------------------------------------------------*/
 
/*
  コマンドライン引数で指定されたROMファイルを読み込む。
  破壊オプションで指定された領域をできるだけ安全に破壊する。
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "misc.h"
#include <twl/os/common/format_rom.h>

/*
  rombreaker.exe -i input.srl -o output.srl -b HEADER
*/

static ROM_Header_Short *rom_header_short;

#define MATH_ROUNDUP(x, base)   (((x) + ((base)-1)) & ~((base)-1))

// 1MBぐらいずつ
#define FILE_READ_BUFFER_LENGTH ( 1 * 1024 * 1024 )

int main(int argc, char *argv[])
{
  char   *inFile = NULL;
  char   *outFile = NULL;
  char   *breakOpt = NULL;
  FILE   *in_fp;
  FILE   *out_fp;
  int     n;
  int read_size;
  unsigned char file_read_buffer[FILE_READ_BUFFER_LENGTH];
  u32 breakoffset;
  u32 fcounter;

  while ((n = getopt(argc, argv, "hiob")) != -1) {
    switch (n) {
    case 'h':
      printf("Usage:\n"
             " %s -b breakopt -i inputfile -o outputfile\n"
             "  -b HEADER/ARM9FLX/ARM9LTD/ARM7FLX/ARM7LTD ( BreakOption )\n"
             "  -i XXXX ( input srl file )\n"
             "  -o XXXX ( output srl file )\n\n",
             argv[0]);
      return 0;

    case 'i':
      inFile =        strdup( argv[optind] );
      printf("input srl file = %s\n", inFile);
      break;
    case 'o':
      outFile =        strdup( argv[optind] );
      printf("output srl file = %s\n", outFile);
      break;
    case 'b':
      breakOpt = strdup( argv[optind] );
      printf("break opt = %s\n", breakOpt);
      break;
    default:
      break;
    }
  }

  if( inFile && outFile ) {
    // 面倒なのでinとoutが同じ場合は対応しない（要望があれば対応）
    if( strncmp( inFile, outFile, 256 ) == 0 )
    {
      fprintf(stderr,"Error:output is input.\n");
      return -1;
    }
    if( NULL == (in_fp = fopen(inFile, "rb")) ) {
      fprintf(stderr,"Error:input srl file open error %s\n",inFile);
      return -1;
    }
    if( NULL == (out_fp = fopen(outFile, "wb+")) ) {
      fprintf(stderr,"Error:output srl file open error %s\n",outFile);
      return -1;
    }
  }
  else {
    fprintf(stderr,"Error:not specify input or output\n");
    return -1;
  }

/*
  FLXとLTDはできるだけコードに影響を与えないよう後ろから1バイト破壊。
  HEADERは署名範囲内で誰も使っていない予約領域を1バイト破壊。
*/

  /* ROM Header情報取得 */
  rom_header_short = (ROM_Header_Short *)file_read_buffer;
  read_size = fread( (void *)rom_header_short, 1, sizeof(ROM_Header_Short), in_fp );
  if( read_size != sizeof(ROM_Header_Short) )
  {
    fprintf(stderr,"Error:rom_header_short read error\n");
    return -1;
  }

  if( strncmp( "HEADER", breakOpt, 7 ) == 0 )
  {
    breakoffset = 0x180 - 1;
  }else if( strncmp( "ARM9FLX", breakOpt, 8 ) == 0 )
  {
    breakoffset = rom_header_short->main_rom_offset + rom_header_short->main_size - 1;
  }else if( strncmp( "ARM9LTD", breakOpt, 8 ) == 0 )
  {
    breakoffset = rom_header_short->main_ltd_rom_offset + rom_header_short->main_ltd_size - 1;
  }else if( strncmp( "ARM7FLX", breakOpt, 8 ) == 0 )
  {
    breakoffset = rom_header_short->sub_rom_offset + rom_header_short->sub_size - 1;
  }else if( strncmp( "ARM7LTD", breakOpt, 8 ) == 0 )
  {
    breakoffset = rom_header_short->sub_ltd_rom_offset + rom_header_short->sub_ltd_size - 1;
  }else
  {
    // 破壊オプション指定ミス
    fprintf(stderr,"Error: invalid break opt %s\n",breakOpt);
    return -1;
  }
  
  // inputのファイルポインタを先頭に戻す（面倒なので）
  fseek( in_fp, 0, SEEK_SET );

  // 壊す場所を書き換えながらコピー
  while( 1 ) {
    read_size = fread((void *)file_read_buffer, 1, FILE_READ_BUFFER_LENGTH, in_fp);
    if( read_size ) {
      fcounter += read_size;
      // 書き換え
      if( fcounter > breakoffset )
      {
        file_read_buffer[ breakoffset - ( fcounter - read_size ) ] += 1; // 1足すだけで壊れる
      }
      if( read_size != fwrite((void *)&file_read_buffer, 1, read_size, out_fp) ) {
        fprintf(stderr,"Error:file write error\n");
        return -1;
      }
    }
    else {
      break;
    }
  }

  fclose(in_fp);
  fclose(out_fp);

  printf("success!\n");
  return 0;
}
