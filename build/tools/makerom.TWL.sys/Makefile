#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - makerom.TWL
#  File:     Makefile
#
#  Copyright 2007 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date::            $
#  $Rev$
#  $Author$
#---------------------------------------------------------------------------
TARGET_PLATFORM		= TWL

TARGET_FIRM	= SYSTEMMENU

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

SRCDIR		= $(ROOT)/build/tools/makerom.TWL
INCDIR		= $(SRCDIR)

#SUBDIRS		= $(addprefix $(SRCDIR)/,acsign aes digest)

TARGETS		= makerom.TWL.sys.exe

SOURCES_C	= makerom.c		\
		  optarg.c		\
		  misc.c		\
		  path.c		\
		  defval.c		\
		  container.c		\
		  spec_rom.c		\
		  spec_proc.c		\
		  spec_property.c	\
		  spec_nandAppProperty.c	\
		  spec_segment.c	\
		  file_sort.c		\
		  file_offset.c 	\
		  out_romheader.c	\
		  out_filename.c 	\
		  out_overlaytable.c	\
		  out_listfile.c	\
		  out_romimage.c	\
          calc_digest_size.c	\
          set_digest.c	\
          set_signature.c \
          encrypt_aes.c \
          get_moduleParam.c

SOURCES		= $(addprefix $(SRCDIR)/,$(SOURCES_C) spec.l spec.y ) ca_keys_sys_dev.c
DEPEND_TARGETS  = $(addprefix $(SRCDIR)/,$(SOURCES_C)) ca_keys_sys_dev.c
OBJECTS		= $(SOURCES_C:.c=.o) spec.tab.o spec.yy.patched.o ca_keys_sys_dev.o

HEADERS_H	= container.h		\
		  format_nlist.h	\
		  makerom.h 		\
		  path.h		\
		  spec_proc.h		\
		  spec_rom.h 		\
		  defval.h		\
		  format_rom_private.h		\
		  misc.h		\
		  optarg.h		\
		  spec_property.h

HEADERS				=	$(addprefix $(INCDIR)/,$(HEADERS_H))

LIBACSIGN			=	$(SRCDIR)/acsign/lib/X86/libacsign_x86.a
LIBAES				=	$(SRCDIR)/aes/lib/X86/libaes_x86.a
LIBDIGEST			=	$(SRCDIR)/digest/lib/X86/libdigest_x86.a

#NITRO_DIGEST_DIR	=	$(NITRO_ADDINS)/digest/build/libraries/dgt
#MACROS				+=	-I$(NITRO_DIGEST_DIR)/include	\
#						-I$(NITRO_DIGEST_DIR)/src
MACROS				=	-I$(ROOT)/build/tools/makerom.TWL	\
						-I$(ROOT)/include	\
						-I$(ROOT)/include/twl/os/common \
						-I$(ROOT)/include/nitro/os/ARM9

#INSTALL_DIR	= $(TWL_INSTALL_TOOLSDIR)/bin
#INSTALL_TARGETS	= $(TARGETS) rom_header.template.sbin
INSTALL_DIR	= $(SYSMENU_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS) $(SRCDIR)/rom_header.template.sbin

LDIRT_CLEAN	= $(OBJECTS) $(TARGETS) version.h \
		  spec.tab.c spec.tab.h spec.yy.c \
		  spec.yy.patched.c \
		  depend.inc

include	$(TWLSDK_ROOT)/build/buildtools/twl/modulerules.x86


#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		version.h $(OBJECTS) $(LIBACSIGN) $(LIBAES) $(LIBDIGEST)
			$(CC_X86) $+ -o $@

spec.yy.c			:	$(SRCDIR)/spec.l
spec.tab.c			:	$(SRCDIR)/spec.y

spec.yy.patched.o	:	WARNING	+= -Wno-unused-label -Wno-unused-function
spec.yy.patched.c	:	$(SRCDIR)/spec.yy.c
	sed 's/^#define YY_BUF_SIZE 16384/#define YY_BUF_SIZE (1024*1024)/' $< > $@

makerom.o:	version.h

version.h:		$(SOURCES) $(HEADERS) $(MAKEFILE)
			@for i in $^ ;							\
			do								\
				date -r $$i +'#define SDK_DATE_OF_LATEST_FILE %Y%m%dUL';	\
			done | sort | tail -1 > $@


%.o:		$(SRCDIR)/%.c
ifdef SILENT
	echo $(MSG_COMPILE) $(notdir $<)
endif
	$(COMPILE_C)

%.o:		$(SRCDIR)/%.cpp
ifdef SILENT
	echo $(MSG_COMPILE) $(notdir $<)
endif
	$(COMPILE_CXX)

%.yy.c:		$(SRCDIR)/%.l
	$(LEX) -P$(basename $<)_yy -o$@ $<

%.tab.c:	$(SRCDIR)/%.y
	$(YACC) -d -p $(notdir $(basename $<))_yy -b $(notdir $(basename $<)) $<

%.tab.h:	%.tab.c;


depend.inc: version.h $(DEPEND_TARGETS)
	-@ $(RM) depend.inc
	-@ for i in $(DEPEND_TARGETS); do\
		$(CC_X86) $(MACROS) -MM -w -DSDK_WIN32 $(WARNING) -I. -c -I$(TWL_INCDIR) $$i >> depend.inc;\
	done

-include depend.inc

