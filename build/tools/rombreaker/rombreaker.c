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
  �R�}���h���C�������Ŏw�肳�ꂽROM�t�@�C����ǂݍ��ށB
  �j��I�v�V�����Ŏw�肳�ꂽ�̈���ł��邾�����S�ɔj�󂷂�B
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

// 1MB���炢����
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
    // �ʓ|�Ȃ̂�in��out�������ꍇ�͑Ή����Ȃ��i�v�]������ΑΉ��j
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
  FLX��LTD�͂ł��邾���R�[�h�ɉe����^���Ȃ��悤��납��1�o�C�g�j��B
  HEADER�͏����͈͓��ŒN���g���Ă��Ȃ��\��̈��1�o�C�g�j��B
*/

  /* ROM Header���擾 */
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
    // �j��I�v�V�����w��~�X
    fprintf(stderr,"Error: invalid break opt %s\n",breakOpt);
    return -1;
  }
  
  // input�̃t�@�C���|�C���^��擪�ɖ߂��i�ʓ|�Ȃ̂Łj
  fseek( in_fp, 0, SEEK_SET );

  // �󂷏ꏊ�����������Ȃ���R�s�[
  while( 1 ) {
    read_size = fread((void *)file_read_buffer, 1, FILE_READ_BUFFER_LENGTH, in_fp);
    if( read_size ) {
      fcounter += read_size;
      // ��������
      if( fcounter > breakoffset )
      {
        file_read_buffer[ breakoffset - ( fcounter - read_size ) ] += 1; // 1���������ŉ���
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
