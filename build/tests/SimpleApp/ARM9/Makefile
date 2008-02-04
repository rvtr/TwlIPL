#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-1
# File:     Makefile
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: $
# $Rev:  $
# $Author:  $
#----------------------------------------------------------------------------

SUBDIRS					=	

#----------------------------------------------------------------------------

TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	SMPL

TARGET_BIN				=	simpleapp.srl

ROM_SPEC				=	main.rsf

SRCS					=	main.c SimpleApp.c misc_simple.c data.c

LINCLUDES	=	$(TWLSDK_ROOT)/build/demos/gx/UnitTours/DEMOLib/include

LLIBRARY_DIRS	+=	$(TWLSDK_ROOT)/build/demos/gx/UnitTours/DEMOLib/lib/$(TWL_BUILDTYPE)

LLIBRARIES				+=	libboc$(TWL_LIBSUFFIX).a \
							libes$(TWL_LIBSUFFIX).a \
							libnam$(TWL_LIBSUFFIX).a \
							libDEMO$(TWL_LIBSUFFIX).a

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include	$(TWLSDK_ROOT)/add-ins/es/commondefs.es
include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====

