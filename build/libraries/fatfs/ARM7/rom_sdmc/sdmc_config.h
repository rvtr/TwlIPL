/*
** Copyright (c) 2000-2001 Matsushita Electric Industrial Co., Ltd.
** All Rights Reserved.
*/

/*
** $System      IP1.1 without C2 �T���v���\�t�g
** $Subsystem   �J�[�h�h���C�o
** $Filename    CARDDRV.H
** $Version     1.0 ��
** $Date        01/02/16
** $Log         01/02/16    rev1.0�쐬
**              �����d��Y��(��)�����̊J���{��
*/


#ifndef __SDMC_CONFIG_H__
#define __SDMC_CONFIG_H__


#include <firm.h>

#ifdef    USE_OS
#include <sd_ip_reg.h>                        /* IP �Ή����W�X�^��` */
#endif



/*********************************************
 �^�[�Q�b�gOS
*********************************************/
#define    TARGET_OS_CTR                (0)
#define    TARGET_OS_NITRO              (TARGET_OS_CTR ^ 1)


/*********************************************
 SD�h���C�o �R���t�B�O���[�V����
*********************************************/
#define    SD_DEBUG_PRINT_ON            0        /* �f�o�b�O�\�� */

#define    WP_ena                       1        /* ���C�g�v���e�N�g�̃`�F�b�N�L�� */
#define    TIMEOUT                      1        /* FPGA Timeout none = FALSE */
#define    SCR                          1        /* Send SCR Command = TRUE */
#define    RESID                        1        /* Write Error Resid enable = TRUE */
#define    ATC_ON                       0        /* ATC�]�� �g�p/���g�p */

#define    SecEnable                    1        /* SD_SECCNT���W�X�^ Enable */
#define    SecDisenable                 0        /* SD_SECCNT���W�X�^ Disable */
//#define    STANDBYMODE                  0x04    /*** 5772 standby control bit    ***/

#define    SDCARD_TIMER_ID  (OS_TIMER_3)


/*********************************************
 �^�C���A�E�g�ݒ�l(ms�P��)
*********************************************/
#define    SDCARD_RW_TIMEOUT            (2000)
#define    SDCARD_STDBY_TIMEOUT         (50)
#define    SDCARD_CLOCK_WAIT            (500)
#define    SDCARD_SDCLK_WAIT            (10)
#define    SDCARD_INITIAL_TIMEOUT       (800)
#define    SDCARD_RESET_TIMEOUT         (1500)
#define    SDCARD_ERASE_TIMEOUT         (1)
#define    SDCARD_ERRPROC_TIMEOUT       (2000)


/*********************************************
 ���g���C��(Multiple Block R/W �̂Ƃ�)
*********************************************/
#define    SDCARD_RETRY_COUNT           (3)

/*********************************************
 ���̑�
*********************************************/
#define    SECTOR_SIZE                  (512)       /* 1�Z�N�^�̃o�C�g�� */
#define    SECTOR_MAX                   (255)       /* SYSFPGA �A�N�Z�X�ő�Z�N�^�� */

/*--- ��ʃ��C���ɕԂ��X�e�[�^�X�l(SDCARD_Getstatus�Q��)�p ---*/
#define    SDCARD_FLAG_CLR              (0x3FFF)    /* �J�[�h���蕔���N���A�p */
#define    SDCARD_FLAG_SD               (0x8000)    /* �J�[�h���蕔��SD�J�[�h */
#define    SDCARD_FLAG_MMC              (0x4000)    /* �J�[�h���蕔��MMC�J�[�h */
#define    SDCARD_PORT1_CLR             (0x0007)    /* �J�[�h�|�[�g1���蕔���N���A�p */




#endif /*__SDMC_CONFIG_H__*/
