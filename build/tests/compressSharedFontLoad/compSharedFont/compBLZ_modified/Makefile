#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - compBLZ
#  File:     Makefile
#
#  Copyright 2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
# $Date::            $
# $Rev$
# $Author$
#---------------------------------------------------------------------------

TARGET_PLATFORM    =   TWL NITRO

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#---------------------------------------------------------------------------

TARGET_BIN   =   compBLZ.exe
TARGETS      =   $(BINDIR)/$(TARGET_BIN)

SRCS         =   main.c                  \
                 compress.c              \
                 file.c                  \
                 version.c
                 
HEADERS      =   common.h    \
                 compress.h  \
                 file.h
                 
INCDIR      +=  ../../../include
OBJDIR       =   obj
BINDIR       =   bin

OBJS         =   $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

MACROS          +=  -DSDK_TWL $(addprefix -I,$(INCDIR))
NEWDIRS          =  $(OBJDIR) $(BINDIR)
LDIRT_CLEAN     +=  $(NEWDIRS) version.c

#INSTALL_DIR      =   $(TWL_INSTALL_TOOLSDIR)/bin
#INSTALL_TARGETS  =   $(TARGETS)

#---------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules.x86

do-build:   $(TARGETS)

$(TARGETS): $(OBJS) $(LIBDGT) $(MAKEFILE)
	$(CC_X86) $(OBJS) $(LIBDGT) -o $@

$(OBJS):%.o:
	$(COMPILE_C)

$(OBJDIR)/main.o:                   main.c file.h version.c
$(OBJDIR)/compress.o:               compress.c compress.h
$(OBJDIR)/file.o:                   file.c file.h
$(OBJDIR)/version.o:                version.c

version.c:  $(filter-out version.c,$(SRCS)) $(HEADERS) $(MAKEFILE)
	@for i in $^ ;                                                  \
    do                                                              \
        date -r $$i +'const unsigned long SDK_DATE_OF_LATEST_FILE=%Y%m%dUL;';    \
    done | sort | tail -1 > $@

#===== End of Makefile =====
