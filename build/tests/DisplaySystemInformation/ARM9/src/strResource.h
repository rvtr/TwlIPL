/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     strResource.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef __DISPLAY_INFO_RESOURCE__
#define __DISPLAY_INFO_RESOURCE__

// 各項目のページごとのオフセット値
extern const int s_pageOffset[ROOTMENU_SIZE][MAXPAGE];

extern const u8 s_numMenu[];
extern const char *s_strRootMenu[];
extern const char *s_strOwnerMenu[];
extern const char *s_strParentalMenu[];
extern const char *s_strNormalHWMenu[];
extern const char *s_strSecureHWMenu[];
extern const char *s_strSCFGARM7Menu[];
extern const char *s_strSCFGARM9Menu[];
extern const char *s_strVersionMenu[];
extern const char **s_strMetaMenu[];
extern const char *s_strARM7RegisterName[];
extern const char *s_strARM9RegisterName[];
extern const char *s_strSCFGViewMode[];
extern char *s_strEnable[];
extern char *s_strJoint[];
extern char *s_strSupply[];
extern char *s_strRomMode[];
extern char *s_strPSRAM[];
extern char *s_strCpuSpeed[];
extern char *s_strBool[];
extern char *s_strAccess[];
extern char *s_strRomApp[];
extern char *s_strRomForm[];
extern char *s_strMCMode[];
extern char *s_strRatingOrg[];
extern char *s_strRegion[];
extern char *s_strUserColor[];
extern char *s_strLanguage[];
extern char *s_strCountry[];
extern char s_strNA[];	

#endif