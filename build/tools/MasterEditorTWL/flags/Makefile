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
TARGET_PLATFORM	 =	TWL

#----------------------------------------------------------------------------

SUBDIRS			 =

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

TARGET_LIB		 =	libflags_x86.a
TARGETS			 =	$(LIBDIR)/$(TARGET_LIB)

SRCS			 =	flags.c

INCDIR			 =	include \
					../

OBJDIR			 =	obj/X86
LIBDIR			 =	lib/X86

MACROS			+=	$(addprefix -I,$(INCDIR))	\
					-DSTANDALONE \
					-DOPT_32_BIT \
					-DNO_SPLIT \
					-DNO_FP_API \
					-DNO_R_DIAG \
					-DNO_STDIO_H \
					-DNO_STDLIB_H

VPATH				= $(SRCDIR) $(INCDIR)
OBJS				= $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

#INSTALL_TARGETS	=	$(TARGETS)
#INSTALL_DIR		=	.

NEWDIRS				= $(OBJDIR) $(LIBDIR)
LDIRT_CLEAN			= $(OBJDIR) $(LIBDIR)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJS)
			$(AR_X86) $@ $(OBJS)

$(OBJS):%.o:
			$(COMPILE_C)

$(OBJDIR)/flags.o:	flags.c


#===== End of Makefile =====
