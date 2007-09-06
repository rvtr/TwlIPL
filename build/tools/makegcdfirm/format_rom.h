/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makegcdfirm
  File:     format_rom.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef FORMAT_ROM_H_
#define FORMAT_ROM_H_

#include "misc.h"
#include <firm/format/gcdfirm.h>


#define	DEFAULT_ALIGN			0x200
#define FIRM_ALIGN              DEFAULT_ALIGN
#define FIRM_ALIGN_MASK         (FIRM_ALIGN - 1)

#define	NML_AREA_ALIGN			0x80000  // 512KB
#define	TWL_AREA_ALIGN			0x80000  // 512KB
#define	SEC2_AREA_OFFSET		0x3000   // 12KB
#define	GAME2_AREA_OFFSET		0x7000   // 28KB

#define DEFAULT_HOSTROOT        "."
#define DEFAULT_ROOT            "/"

#define DEFAULT_REJECT_1        "CVS"
#define DEFAULT_REJECT_2        "vssver.scc"

#define FORMAT_VERSION          "1.0"

#define ENTRYNAME_MAX           127

#define	ROM_SIZE_MIN			0x20000

#define	DEFAULT_ROMHEADER_TEMPLATE	"gcdfirm_header_twlj.template.sbin"
#define DEFAULT_LISTFILE        "default.nlf"

#define DEFAULT_NORFIRM_SUFFIX      ".gcd"
#define DEFAULT_SPECFILE_SUFFIX     ".gcdsf"

/*===========================================================================*
 *  ROM FORMAT
 *===========================================================================*/

//---------------------------------------------------------------------------
//  ROM HEADER
//---------------------------------------------------------------------------

#endif //FORMAT_ROM_H_
