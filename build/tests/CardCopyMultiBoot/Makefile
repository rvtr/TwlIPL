#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - MB - demos - card_copy
# File:     Makefile
#
# Copyright 2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS		= banner

# MB���C�u�����̃T���v���̓}���`�u�[�g�@�\���g�����߁A
# �����ʐM��(�������邢�͗L��)�ǂ����̊J���@�ނ𕡐��K�v�Ƃ��܂��B
# $TwlSDK/bin/ARM9-TS/Release/ �f�B���N�g���ɂ���
# mb_child.bin �v���O�������ŏI���@�ɂ�����}���`�u�[�g�q�@�Ɠ�����
# �@�\��񋟂���T���v���ɂȂ��Ă��܂��̂ŁA
# ���̃o�C�i�����T���v���v���O�����Ɠ������@�ő��̋@�ނɓǂݍ��݁A
# �ꏏ�Ɏ��s���Ă��������B

#----------------------------------------------------------------------------

TARGET_PLATFORM		:= TWL
TWL_ARCHGEN	:=	HYBRID

#-------------------------
#-- NAND �A�v���̃r���h���w�肵�܂��B

TWL_NANDAPP		=	TRUE

#----------------------------------------------------------------------------
# ���̃v���O�����Ŏg�p����\�[�X�t�@�C���͈ȉ��̒ʂ�ł�.
#----------------------------------------------------------------------------

SRCDIR		 = ./src
INCDIR		 = ./include

SRCS		 = main.c		\
			   common.c		\
			   disp.c		\
			   font.c		\
			   child.c		\
			   parent.c		\
			   text.c		\

# �T���v���̊ȗ����̂��� WH ���W���[�����g�p���܂�.

WH_DIR		 = $(ROOT)/build/demos/wireless_shared/wh
SRCDIR		+= $(WH_DIR)
INCDIR		+= $(WH_DIR)
SRCS		+= $(WH_DIR)/wh.c

# �T���v���̊ȗ����̂��� MBP ���W���[�����g�p���܂�.

MBP_DIR		 = $(ROOT)/build/demos/wireless_shared/mbp
SRCDIR		+= $(MBP_DIR)
INCDIR		+= $(MBP_DIR)
SRCS		+= $(MBP_DIR)/mbp.c


#----------------------------------------------------------------------------
# �A�v���P�[�V�����̈�ʓI�Ȑݒ�͈ȉ��̒ʂ�ł�.
#----------------------------------------------------------------------------

TARGET_BIN		 = card_copy.srl

TWL_MAKEROM		 = TRUE

# �v���O�����̊e�Z�O�����g�����k����ꍇ�͂��̃t���O��L���ɂ��܂�.
# ���k���ꂽ�Z�O�����g�� CARD-ROM ��ł̃o�C�i���T�C�Y���팸����
# CARD-ROM �܂��̓��C�����X�o�R�ł̃��[�h���Ԃ��Z�k����邩����,
# ���s���͓W�J�̂��߂ɂ����炩�� CPU ���Ԃ�����܂�.
TWL_COMPRESS	 = TRUE

# ���C�����X�_�E�����[�h�q�@�Ƃ��ăI�[�o�[���C���g�p����ꍇ,
# NITRO_DIGEST ��L���ɂ���K�v������܂�.
TWL_DIGEST		 = TRUE

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	$(TARGETS)

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC		 = main.TWL.rsf
#LCFILE_SPEC	 = main.TWL.lsf
else
ROM_SPEC		 = main.rsf
#LCFILE_SPEC	 = main.lsf
endif

# MB�̃R�[���o�b�N���ŕ��G�ȏ���������ꍇ�ɂ́Alcf�t�@�C����
# IRQ�X�^�b�N�T�C�Y�������傫�߂ɐݒ肵�Ă������������S�ł��B
# ����OS_Printf()�͑�ʂ̃X�^�b�N�������̂ŁA�R�[���o�b�N���ł�
# �ł��邾���y�ʔł�OS_TPrintf()���g�p����悤�ɂ��Ă��������B

#----------------------------------------------------------------------------
# �N���[���u�[�g�e�@�Ƃ��Ă̓��ʂȐݒ�͈ȉ��̒ʂ�ł�.
#----------------------------------------------------------------------------

# �Z�L�����e�B�ړI�̐e�@��p�̈� .parent �Z�N�V������z�u���邽��
# �N���[���u�[�g�p�����N�ݒ�e���v���[�g���w�肵�܂�.
#
# .parent �Z�N�V�����̓��e��DS�_�E�����[�h�v���C�q�@�ɂ͑��M����܂���.
# ���̃Z�N�V�����ɐe�@��p���K�{�̊֐���z�u���邱�Ƃɂ����,
# �T�󂳂ꂽ��M���e����e�@���g���Č����邱�Ƃ�s�\�ɂ��܂�.
# �R�[�h�̔z�u�ɂ������Ă�, �ȉ��̓_�ɒ��ӂ��Ă�������.
#   �E.text/.rodata �Z�N�V�����ȊO���܂܂Ȃ�����
#   �E�e�@�݂̂��K�v�Ƃ���d�v�ȏ������܂ނ���
# 
# .parent �Z�N�V������, �Y������R�[�h�ɂ��Ĉȉ��̂悤�ɔz�u���܂�.
# 
#     #include <nitro/parent_begin.h>
#
#     void  ForParentOnly(void)
#     {
#         /* �����e�@�ŗL�̏��� */
#     }
#
#     #include <nitro/parent_end.h>
#
# ���̃T���v���f���ł� parent.c �̑S�Ă̊֐��� .parent �Z�N�V������
# �z�u���Ă��܂�. �ڍׂ̓\�[�X�R�[�h���Q�Ƃ�������.

ifeq	($(TARGET_PLATFORM),TWL)
LCFILE_TEMPLATE	= $(TWL_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template
else
LCFILE_TEMPLATE	= $(NITRO_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template
endif

# ���i�ŃA�v���P�[�V�����̂��߂Ɋe�X�֔z�z���ꂽ
# �����ȃV�X�e���R�[�����C�u�����������Ŏw�肵�܂�.
# ���̃T���v���ł͎b��p�Ƃ��āA���炩���߂��̃f�B���N�g����,
# SDK �t����(lib/ARM9-TS/etc/libsyscall.a)�̃R�s�[��u���Ă���܂��D
# ���̃f�������ɂ��ăA�v���P�[�V�����������ꍇ�́A���ЃT�|�[�g���
# �z�z�����Ă��������Ă��� libsyscal.a �ł��̎b��V�X�e���R�[����
# �㏑�����Ă��������D

LIBSYSCALL			 = ./etc/libsyscall.a
LIBSYSCALL_CHILD	 = ./etc/libsyscall_child.bin

#----------------------------------------------------------------------------
# �A�v���P�[�V�����𐻕i�łŎ��s�����邽�߂̎��O�F�؎葱���͈ȉ��̒ʂ�ł�.
#----------------------------------------------------------------------------

# ���O�F�؎葱���͂����悻�ȉ��̗���ɂȂ�܂�.
# �ڍׂ́uDS�_�E�����[�h�v���C����v���Q�Ƃ�������.
# (/docs/TechnicalNotes/AboutMultiBoot.doc)
#
# (0) �N���[���u�[�g�Ȃ� emuchild �R�}���h�ő��M�p�v���O������p�ӂ��܂�.
#     �����łȂ���� main.srl �����̂܂ܗp�ӂ��܂�.
#     ���� makefile �ł�, �p�ӂ������̂� bin/sign.srl �Ƃ��܂�.
#
# (1) �F�؃R�[�h�擾�̂���, (0) �ŗp�ӂ��� tmp/sign.srl ���T�[�o�֑��t���܂�.
#     ���̍�Ƃ͂��g�p�̃u���E�U������Ƃōs���K�v������܂�.
#     ���� makefile �ł�, �擾�����F�؃R�[�h�� bin/sign.sgn �Ƃ��܂�.
#
# (2) attachsign �R�}���h���g�p����, (1) �Ŏ擾���� bin/sign.sgn ��
#     main.srl �����������ŏI�v���O�����𐶐����܂�.
#     ���� makefile �ł�, �������� main_with_sign.srl �Ƃ��܂�.
#

# �N���[���u�[�g�̏ꍇ�͒ʏ��DS�_�E�����[�h�v���C�v���O�����Ǝ኱�قȂ�,
# ���g�̃v���O�����łȂ� emuchild �ŉ��H�����v���O�������T�[�o�֑��t���܂�.

presign:
	$(EMUCHILD) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		$(LIBSYSCALL_CHILD) \
		bin/sign.srl

# ����ꂽ�F�؃R�[�h���o�C�i���Ɋ܂߂�葱���̓N���[���u�[�g���ʏ�Ɠ��l�ł�.
postsign:
	$(ATTACHSIGN) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		bin/sign.sgn  \
		main_with_sign.srl && \
	$(MAKETAD) \
		main_with_sign.srl \
		$(MAKETAD_OPTION) \
		-o bin/$(TWL_BUILDTYPE)/card_copy.tad

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN


#===== End of Makefile =====