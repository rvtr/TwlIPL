#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "misc.h"
#include <twl/os/common/format_rom.h>

/*
  attach_dummyromheader.exe -s -l -i nwm_firm.bin -c WFW0 -o m.srl
  maketad m.srl -s
*/

static ROM_Header rom_header;

#define FILE_READ_BUFFER_LENGTH 0x1000

/* maketadのソースから引用 */
#define TAD_BOOT_IDX          0
#define TITLE_NAME_OFFSET     0x0
#define PUBLISHER_CODE        0x3

#define CHANNEL_SYSTEM_BIT    0x1
#define CHANNEL_SHOW_BIT      0x2
#define CHANNEL_CARD_BIT      0x4


int main(int argc, char *argv[])
{
  char   *binFile = NULL;
  char   *srlFile = NULL;
  char   *g_code = NULL;
  FILE   *bin_fp;
  FILE   *srl_fp;
  int     n;
  int read_size;
  BOOL isSystem = FALSE;
  BOOL isLaunch = FALSE;
  unsigned char file_read_buffer[FILE_READ_BUFFER_LENGTH];

  while ((n = getopt(argc, argv, "hiocsl")) != -1) {
    switch (n) {
    case 'h':
      printf("Usage:\n"
	     " %s -s -l -c -i inputfile -o outputfile\n"
	     "  -s (Systemfile) \n"
	     "  -l (Not Launch) \n"
	     "  -c XXXX ( GameCode )\n"
	     "  -i XXXX ( input file )\n"
	     "  -o XXXX ( output file )\n\n",
	     argv[0]);
      return 0;

    case 'i':
      binFile =	strdup( argv[optind] );
      printf("input file = %s\n", binFile);
      break;
    case 'o':
      srlFile =	strdup( argv[optind] );
      printf("srl file = %s\n", srlFile);
      break;
      
    case 'c':
      g_code = strdup( argv[optind] );
      printf("game code = %s\n", g_code);
      if( 4 != strlen(g_code) ) {
	fprintf(stderr,"Error: invalid game code %s\n",g_code);
	return -1;
      }

      break;
    case 's':
      isSystem = TRUE;
      printf("system file\n");
      break;
    case 'l':
      isLaunch = TRUE;
      printf("Not Launch file\n");
      break;
    default:
      break;
    }

  }
  
  if( binFile && srlFile ) {
    if( NULL == (bin_fp = fopen(binFile, "rb")) ) {
      fprintf(stderr,"Error:binFile open error %s\n",binFile);
      return -1;
    }
    if( NULL == (srl_fp = fopen(srlFile, "wb+")) ) {
      fprintf(stderr,"Error:srlFile open error %s\n",srlFile);
      return -1;
    }
  }
  else {
    fprintf(stderr,"Error:not specify binFile or srlFile\n");
    return -1;
  }

  /* ROM Header情報生成 */
  memset((void *)&rom_header, 0 , sizeof(ROM_Header));

  //  strncpy((void *)&rom_header.s.title_name, "WLANFIRMWARE", TITLE_NAME_MAX);
  strncpy((void *)&rom_header.s.title_name, g_code, TITLE_NAME_MAX);

  // strncpy((void *)&rom_header.s.game_code,"WFW0",GAME_CODE_MAX);   // Game code
  strncpy((void *)&rom_header.s.game_code, g_code ,GAME_CODE_MAX);   // Game code

  rom_header.s.maker_code[0] = 0;    // Maker code
  rom_header.s.maker_code[1] = 3;    // Maker code
  rom_header.s.platform_code = 0x03; /* Platform code bit0: not support NTR,  bit1: support TWL
				     ( NTR_only=0x00, NTR/TWL=0x03, TWL_only=0x02 ) */

  // rom_header.s.rom_type;
  // rom_header.s.rom_size;                /* Rom size (2のrom_size乗 Mbit: ex. 128Mbitのときrom_size = 7) */
  
  // rom_header.s.enable_signature:1;        // enable ROM Header signature
  // rom_header.s.enable_aes:1;              // enable AES encryption
  // rom_header.s.developer_encrypt:1;       // 開発用セキュリティがかかっている場合に"1"。製品版では"0"

  //	u32     arm7_scfg_ext;                  // SCFG-EXT
  //	u8      arm7_scfg_clk;                  // SCFG-CLK


  // 0x1BF - TWL expansion flags
  // u8		codec_mode:1;					// 0:NTR mode, 1:TWL mode		// undeveloped
  // u8		otherParentalControls:1;		// 1: Enable OtherParentalControls
  // u8		subBannerFile:1;				// 1: Enable SubBannerFile
  // u8		wifiConnection:1;				// 1: Use WiFiConnection
  rom_header.s.codec_mode = 1;			// 0:NTR mode, 1:TWL mode		// undeveloped
  rom_header.s.otherParentalControls = 0;	// 1: Enable OtherParentalControls
  rom_header.s.subBannerFile = 0;		// 1: Enable SubBannerFile
  rom_header.s.wifiConnection = 1;		// 1: Use WiFiConnection  


  //    u8		titleID_Lo[ 4 ];
  //  u32		titleID_Hi;

  rom_header.s.titleID_Lo[ 3 ] = *(g_code);
  rom_header.s.titleID_Lo[ 2 ] = *(g_code+1);
  rom_header.s.titleID_Lo[ 1 ] = *(g_code+2);
  rom_header.s.titleID_Lo[ 0 ] = *(g_code+3);

  /*
    ビットフィールド:
    [0]: AppType ( 0: User, 1: System )
    [1]: Launch ( 0: Launch, 1: Not launch )
    [2]: Media ( 0: GameCard, 1: NAND )
    [3-15]: rsv.
    [16-31]: PublisherCode( 2:Broadon, 3:Nintendo)
  */

  rom_header.s.titleID_Hi =  ( 3 /* Nintendo */ << 16) | 4 | (isLaunch? 2:0) | (isSystem? 1:0);

  printf("titleID = 0x%08X%08x\n", (unsigned int)rom_header.s.titleID_Hi, (unsigned int)*((u32 *)&(rom_header.s.titleID_Lo[0])));

  // 0x02f0 - 0x0300 Parental Controls Rating Info
  // u8		ParentalControlsRatingInfo[ 0x10 ];

  rom_header.s.ParentalControlsRatingInfo[ 0x0 ] = 0;
  rom_header.s.ParentalControlsRatingInfo[ 0x1 ] = 1;
  rom_header.s.ParentalControlsRatingInfo[ 0x2 ] = 2;
  rom_header.s.ParentalControlsRatingInfo[ 0x3 ] = 3;
  rom_header.s.ParentalControlsRatingInfo[ 0x4 ] = 4;
  rom_header.s.ParentalControlsRatingInfo[ 0x5 ] = 5;
  rom_header.s.ParentalControlsRatingInfo[ 0x6 ] = 6;
  rom_header.s.ParentalControlsRatingInfo[ 0x7 ] = 7;
  rom_header.s.ParentalControlsRatingInfo[ 0x8 ] = 8;
  rom_header.s.ParentalControlsRatingInfo[ 0x9 ] = 9;
  rom_header.s.ParentalControlsRatingInfo[ 0xa ] = 0xa;
  rom_header.s.ParentalControlsRatingInfo[ 0xb ] = 0xb;
  rom_header.s.ParentalControlsRatingInfo[ 0xc ] = 0xc;
  rom_header.s.ParentalControlsRatingInfo[ 0xd ] = 0xd;
  rom_header.s.ParentalControlsRatingInfo[ 0xe ] = 0xe;
  rom_header.s.ParentalControlsRatingInfo[ 0xf ] = 0xf;

  /* ＳＲＬファイルの書き出し */
  if(1 != fwrite((void *)&rom_header, sizeof(ROM_Header), 1, srl_fp) ) {
    fprintf(stderr,"Error:file write error(ROM_Header)\n");
    return -1;
  }
  
  
  while( 1 ) {
    read_size = fread((void *)file_read_buffer, 1, FILE_READ_BUFFER_LENGTH, bin_fp);
    if( read_size ) {
      if( 1 != fwrite((void *)&file_read_buffer, read_size, 1, srl_fp) ) {
	fprintf(stderr,"Error:file write error(bin file)\n");	
	return -1;
      }
    }
    else {
      break;
    }
  }

  fclose(bin_fp);
  fclose(srl_fp);

  printf("success!\n");
  return 0;
}
