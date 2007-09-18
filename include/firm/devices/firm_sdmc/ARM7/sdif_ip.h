/*
** Copyright (c) 2000-2001 Matsushita Electric Industrial Co., Ltd.
** All Rights Reserved.
*/

/*
** $Module      SD�J�[�h�A�N�Z�X���W���[���E�C���N���[�h
** $Filename    SD_CARD_IP.H
** $Version     1.0 ��
** $Date        01/02/16
** $Log         01/02/16    rev1.0�쐬
**              �����d��Y��(��)�����̊J���{��
*/


#ifndef __SD_CARD_IP_H__
#define __SD_CARD_IP_H__

//#define    IO3 0            /* Insert Remove  SW = FALSE IO3 = TRUE */

//#define    MAX_SD_CLOCK_4M        0            /* MAX SD Clock 4.608MHz */
//#define    MAX_SD_CLOCK_9M        1            /* MAX SD Clock 9.216MHz */


/*---------------------------------------------
 ���b�p�[���W�X�^�̃r�b�g����i�t���O��`�j
---------------------------------------------*/
/* SD_CNT���W�X�^ */
#define    SDIF_CNT_USEDTC                  (0x0001)    /* DTC�g�p�t���O (R/W) */
#define    SDIF_CNT_USEFIFO                 (0x0002)    /* FIFO�g�p�t���O (R/W) */
#define    SDIF_CNT_FULL                    (0x0100)    /* FIFO FULL�t���O (RO) */
#define    SDIF_CNT_NEMP                    (0x0200)    /* FIFO NOT EMPTY�t���O (RO) */
#define    SDIF_CNT_FCLR                    (0x0400)    /* FIFO �N���A�t���O (WO) */
#define    SDIF_CNT_FFIE                    (0x0800)    /* FIFO FULL�Ŋ��荞�� (R/W) */
#define    SDIF_CNT_FEIE                    (0x1000)    /* FIFO EMPTY�Ŋ��荞�� (R/W) */

#define SDCARD_UseFifoFlag                  ((*SDIF_CNT) & SDIF_CNT_USEFIFO)

/*-------------------------------------
���W�X�^�̃r�b�g����i�t���O��`�j
-------------------------------------*/
/* SD_CMD���W�X�^*/
#define    SD_CMD_CMD                       0x0000    /* SD�J�[�h�ւ̒ʏ�A�N�Z�X(CMD) */
#define    SD_CMD_ACMD                      0x0040    /* SD�J�[�h�ւ̃Z�L�����e�B�A�N�Z�X(ACMD) */

/* SD_STOP���W�X�^ */
#define    SD_STOP_STP                      0x0001    /* �f�[�^�]���I����m�点�� */
#define    SD_STOP_SEC_ENABLE               0x0100    /* SD_SECCNT���W�X�^�L���i�Z�N�^�J�E���g���W�X�^�j */

/* SD_SECCNT���W�X�^ */
#define    SD_SECCNT_END                    0x0000    /* SD_SECCNT���W�X�^ �J�E���g�`�F�b�N */

/* SD_SIZE���W�X�^ */
#define    SD_SIZE_DATA_LENGTH_1B           0x0001    /* SD�J�[�h�]���f�[�^�T�C�Y 1Bytes */
#define    SD_SIZE_DATA_LENGTH_2B           0x0002    /* SD�J�[�h�]���f�[�^�T�C�Y 2Bytes */
#define    SD_SIZE_DATA_LENGTH_4B           0x0004    /* SD�J�[�h�]���f�[�^�T�C�Y 4Bytes */
#define    SD_SIZE_DATA_LENGTH_8B           0x0008    /* SD�J�[�h�]���f�[�^�T�C�Y 8Bytes (SCR) */
#define    SD_SIZE_DATA_LENGTH_16B          0x0010    /* SD�J�[�h�]���f�[�^�T�C�Y 16Bytes */
#define    SD_SIZE_DATA_LENGTH_32B          0x0020    /* SD�J�[�h�]���f�[�^�T�C�Y 32Bytes */
#define    SD_SIZE_DATA_LENGTH_64B          0x0040    /* SD�J�[�h�]���f�[�^�T�C�Y 64Bytes (SD_Status) */
#define    SD_SIZE_DATA_LENGTH_128B         0x0080    /* SD�J�[�h�]���f�[�^�T�C�Y 128Bytes */
#define    SD_SIZE_DATA_LENGTH_256B         0x0100    /* SD�J�[�h�]���f�[�^�T�C�Y 256Bytes */
#define    SD_SIZE_DATA_LENGTH_512B         0x0200    /* SD�J�[�h�]���f�[�^�T�C�Y 512Bytes (�f�[�^) */

/* SD_OPTION���W�X�^ */
#define    SD_OPTION_WIDTH_1BIT             0x8000    /* �r�b�g���̑I�� 1bit�� */
#define    SD_OPTION_MSEL_C2NOUSE           0x4000    /* C2���W���[�����g�p */
#define    SD_CD_DETECT_TIME                0xFFF0    /* CD ���o�^�C���������N���A���邽�߂̃}�X�N */

/* SD_INFO2���W�X�^ */
#define    SD_INFO2_ERR_ILA                 0x8000    /* �C���[�K���A�N�Z�X�G���[ */
#define    SD_INFO2_BWE                     0x0200    /* SD�J�[�h����512byte�̃f�[�^�����ݗv�� */
#define    SD_INFO2_BRE                     0x0100    /* SD�J�[�h����512byte�̃f�[�^�Ǎ��ݗv�� */
#define    SD_INFO2_ERR_ALLCLR              0x807F    /* SD Card �G���[���W�X�^�N���A */
#define    SD_INFO2_ERR_SDDAT0              0x0080    /* SD Card Busy bit */
#define    SD_INFO2_ERR_RESTIMEOUT          0x0040    /* ���X�|���X�^�C���A�E�g�G���[ */
#define    SD_INFO2_ERR_UNDERFLOW           0x0020    /* FIFO �A���_�[�t���[�G���[ */
#define    SD_INFO2_ERR_OVERFLOW            0x0010    /* FIFO �I�[�o�[�t���[�G���[ */
#define    SD_INFO2_ERR_TIMEOUT             0x0008    /* ���X�|���X�ȊO�̃^�C���A�E�g�G���[ */
#define    SD_INFO2_ERR_END                 0x0004    /* �t���[���I���F���ł��Ȃ��Ƃ��́iEND�j�G���[ */
#define    SD_INFO2_ERR_CRC                 0x0002    /* CRC �G���[ */
#define    SD_INFO2_ERR_CMD                 0x0001    /* CMD�G���[ */

#define    SD_INFO2_RW_SET                  0x0300    /* SD�J�[�h Read/Write �v�������ݗv���`�F�b�N */
#define    SD_INFO2_ERROR_SET               0x807F    /* SD�J�[�h �G���[�����ݗv���`�F�b�N */

/* SD_INFO2_MASK���W�X�^ */
#define    SD_INFO2_MASK_ILA                0x8000    /* �C���[�K���A�N�Z�X�G���[�����݃}�X�N */
#define    SD_INFO2_MASK_BWE                0x0200    /* SD�J�[�h����̃f�[�^�����ݗv�������݋֎~ */
#define    SD_INFO2_MASK_BRE                0x0100    /* SD�J�[�h����̃f�[�^�Ǎ��ݗv�������݋֎~ */
#define    SD_INFO2_MASK_ALLERRMASK         0x807F    /* �S�G���[���荞�݋֎~ */
#define    SD_INFO2_MASK_EXCEPT_OVERFLOW    0x802F    /* �S�G���[���荞�݋֎~ FIFO Overflow Error������ */
#define    SD_INFO2_MASK_RESTIMEOUT         0x0040    /* Time out ������Enable */
#define    SD_INFO2_MASK_UNDERFLOW          0x0020    /* FIFO �A���_�[�t���[ ������Enable */
#define    SD_INFO2_MASK_OVERFLOW           0x0010    /* FIFO �I�[�o�[�t���[ ������Enable */
#define    SD_INFO2_MASK_TIMEOUT            0x0008    /* Time out ������Enable */
#define    SD_INFO2_MASK_END                0x0004    /* END �G���[ ������Enable */
#define    SD_INFO2_MASK_CRC                0x0002    /* CRC �G���[ ������Enable */
#define    SD_INFO2_MASK_CMD                0x0001    /* CMD �G���[ ������Enable */
#define    SD_INFO2_MASK_ERRSET             0x807F    /* SD�J�[�h �G���[�����ݗv���`�F�b�N */

/* SD_INFO1���W�X�^ */
#define    SD_INFO1_DAT3DETECT              0x0400    /* (IO3���o) card detect(���o=1) : CTR�ł͎g�p�ł��Ȃ�*/
#define    SD_INFO1_DAT3INSERT              0x0200    /* (IO3���o) card inserted(�}��=1) : CTR�ł͎g�p�ł��Ȃ� */
#define    SD_INFO1_DAT3REMOVE              0x0100    /* (IO3���o) card removed(����=1) : CTR�ł͎g�p�ł��Ȃ� */
#define    SD_INFO1_DAT3INIT                0x0300    /* (IO3���o) �̏����� */
#define    SD_INFO1_WRITEPROTECT            0x0080    /* write protect(�������݋֎~=1) */
#define    SD_INFO1_DETECT                  0x0020    /* card detect(���o=1) */

#define    SD_INFO1_INSERT                  0x0010    /* card inserted(�}��=1) */
#define    SD_INFO1_REMOVE                  0x0008    /* card removed(����=1) */

#define    SD_INFO1_ALL_END                 0x0004    /* R/W access all end */
#define    SD_INFO1_RES_END                 0x0001    /* Response end */
#define    SD_INFO1_INIT                    0x0005    /* SD Card�̏�Ԃ������� */

#define    SD_INFO1_SET                     0x031D    /* SD�J�[�h �}�� and RW�A�N�Z�X�I�� and ���X�|���X�I�� �v�������ݗv���`�F�b�N */

/* SD_INFO1_MASK���W�X�^ (0:���荞�݋��A1:���荞�݋֎~)*/
#define    SD_INFO1_MASK_DAT3INSERT         0x0200    /* (IO3���o) card inserted(�}��) �����݋֎~ */
#define    SD_INFO1_MASK_DAT3REMOVE         0x0100    /* (IO3���o) card removed(����) �����݋֎~ */
#define    SD_INFO1_MASK_INSERT             0x0010    /* card inserted(�}��) �����݋֎~ */
#define    SD_INFO1_MASK_REMOVE             0x0008    /* card removed(����) �����݋֎~ */
#define    SD_INFO1_MASK_ALL_END            0x0004    /* R/W access all end �����݋֎~ */
#define    SD_INFO1_MASK_RES_END            0x0001    /* Response end �����݋֎~ */

/* CC_EXT_MODE���W�X�^ */
#define CC_EXT_MODE_PIO                     0x0000    /* PIO���[�h */
#define CC_EXT_MODE_DMA                     0x0002    /* DMA���[�h */

/* SOFT_RST���W�X�^ */
#define    SOFT_RST_SDIF_RST                0x0001    /* SD I/F���W���[�������Z�b�g */

/* SD_CLK_CTRL���W�X�^ */
#define    SD_CLK_CTRL_SDCLKEN              0x0100    /* SD�J�[�h�N���b�N�o�̓C�l�[�u�� */
#define    SD_CLK_CTRL_512                  0x0180    /* SD�N���b�N�̎��g���i������512�j*/
#define    SD_CLK_CTRL_256                  0x0140    /* SD�N���b�N�̎��g���i������256�j*/
#define    SD_CLK_CTRL_128                  0x0120    /* SD�N���b�N�̎��g���i������128�j*/
#define    SD_CLK_CTRL_64                   0x0110    /* SD�N���b�N�̎��g���i������ 64�j*/
#define    SD_CLK_CTRL_32                   0x0108    /* SD�N���b�N�̎��g���i������ 32�j*/
#define    SD_CLK_CTRL_16                   0x0104    /* SD�N���b�N�̎��g���i������ 16�j*/
#define    SD_CLK_CTRL_8                    0x0102    /* SD�N���b�N�̎��g���i������  8�j*/
#define    SD_CLK_CTRL_4                    0x0101    /* SD�N���b�N�̎��g���i������  4�j*/
#define    SD_CLK_CTRL_2                    0x0100    /* SD�N���b�N�̎��g���i������  2�j*/


/*-------------------------------------
�}���`�|�[�g�Ή��r�b�g����i�t���O��`�j
-------------------------------------*/
#define    SDCARD_PORT_NO                   0x0300    /* �J�[�h�|�[�g�I��         */
#define    SDCARD_PORT_NO_MAX               0x04//0x02    /* �J�[�h�|�[�g�ő�I��    */
#define    SDCARD_PORT_NO_MIN               0x01      /* �J�[�h�|�[�g�ŏ��I��    */
#define    SDCARD_PORT_SELECT_NO            0x0001    /* �J�[�h�|�[�g�ԍ��r�b�g     */
#define    SDCARD_PORT0                     0x0000    /* �J�[�h�|�[�g�ԍ��r�b�g     */
#define    SDCARD_PORT1                     0x0001    /* �J�[�h�|�[�g�ԍ��r�b�g     */

/* EXT_WP���W�X�^�i�|�[�g1�ȍ~�̃��C�g�v���e�N�g�j */
#define    EXT_WP_PORT1                     0x0001    /* �|�[�g1write protect(�������݋֎~=1)*/

/* EXT_CD���W�X�^ */
#define    EXT_CD_PORT1_REMOVE              0x0001    /* �|�[�g1 card detect�i���o=1�j */
#define    EXT_CD_PORT1_INSERT              0x0002    /* �|�[�g1 card inserted�i�}��=1�j */
#define    EXT_CD_PORT1_DETECT              0x0004    /* �|�[�g1 card removed�i����=1�j */

/* EXT_CD_DAT3���W�X�^ */
//#define EXT_CD_PORT1_DAT3INIT        0x0003    /* �|�[�g1 card �̏�� (IO3���o) �̏����� */
#define    EXT_CD_PORT1_DAT3REMOVE          0x0001    /* �|�[�g1 dat3 card detect�i���o=1�j */
#define    EXT_CD_PORT1_DAT3INSERT          0x0002    /* �|�[�g1 dat3 card inserted�i�}��=1�j */
#define    EXT_CD_PORT1_DAT3DETECT          0x0004    /* �|�[�g1 dat3 card removed�i����=1) */

/* EXT_CD_DAT3_MASK���W�X�^ */
#define    EXT_CD_MASK_PORT1INSERT          0x0002    /* port1 card inserted(�}��) �����݋֎~ */
#define    EXT_CD_MASK_PORT1REMOVE          0x0001    /* port1 card removed(����) �����݋֎~ */
#define    EXT_CD_MASK_PORT1DAT3INSERT      0x0002    /* port1 (IO3���o) card inserted(�}��) �����݋֎~ */
#define    EXT_CD_MASK_PORT1DAT3REMOVE      0x0001    /* port1 (IO3���o) card removed(����) �����݋֎~ */

/*-------------------------------------
�J�[�h�̑I��
--------------------------------------*/
#define    SDCARD_DETECT_VISUAL_BIT         0x0400    /***    upper layer card detected visual bit                     ***/

/*-------------------------------------
���C�g�v���e�N�g
-------------------------------------*/
#define    SDCARD_WP_PERMANENT_BIT          0x0020    /*** permanent write protection        ***/
#define    SDCARD_WP_TEMPORARY_BIT          0x0010    /*** temporary write protection        ***/

/*-------------------------------------
SD or MMC�J�[�h�R�}���h
-------------------------------------*/
#define    GO_IDLE_STATE            (0)    /* resets all cards to idle state */
#define    SEND_OP_COND             (1)    /* Asks all cards in idle state to send their operation conditions */
#define    ALL_SEND_CID             (2)    /* send CID numbers */
#define    SEND_RELATIVE_ADDR       (3)    /* ask the card to publish a new relative address(RCA) */
#define    SET_BUS_WIDTH            (6)    /* �r�b�g���̑I�� */
#define    SELECT_CARD              (7)    /* Command toggles acard between thr Stand-by and Transfer states */
#define    SEND_CSD                 (9)    /* addressed card sends its card-specific data(CSD) */
#define    STOP_TRANSMISSION        (12)   /* forces the card to stop transmission */
#define    SD_SEND_STATUS           (13)   /* addressed card sends its status register */
#define    SET_BLOCKLEN             (16)   /* sets the block length */
#define    READ_MULTIPLE_BLOCK      (18)   /* �}���`�u���b�N���[�h */
#define    WRITE_MULTIPLE_BLOCK     (25)   /* �}���`�u���b�N���C�g */
#define    APP_CMD                  (55)   /* CMD55 */
#define    SD_STATUS                (13)   /* ACMD13 Send the SD_CARD status */
#define    SEND_NUM_WR_SECTORS      (22)   /* ACMD22 �������݊����Z�N�^���擾 */
#define    SD_APP_OP_COND           (41)   /* ACMD41 */
#define    SEND_SCR                 (51)   /* SD configuration register (SCR) */

/* MMCplus, eMMC�̒�` */
#define    EXT_CSD_ACCESS           (6)

/* Extended Command�̒�` */
#define    EXT_NORMAL               (0)
#define    EXT_SDIO                 (0x4000)

#define    EXT_COM_R3               (0x0700)

#define    EXT_CMD                  (0x00C0)

#define    SEND_IF_COND             (8)    /* Physical Layer 2.0 �Œǉ����ꂽ�R�}���h */
#define    SEND_IF_COND_EXT         (EXT_SDIO | EXT_COM_R3 | EXT_CMD | SEND_IF_COND)


/*-------------------------------------
IP ���W�X�^�A�N�Z�X �}�N���֐�
-------------------------------------*/
#define    SD_OrFPGA(reg,value)      ((reg) |= (value)); 
#define    SD_AndFPGA(reg,value)     ((reg) &= (value));
#define    SD_SetFPGA(reg,value)     ((reg) = (value));
#define    SD_GetFPGA(dest,reg)      ((dest) = (reg));

/***********************************************************************
    �\���̒萔
***********************************************************************/
typedef    union{
    u32    dat;
    struct    {
        u16    low;
        u16    high;
    }    dt2word;
}    LELONG;

/*-------------------------------------
���̑��i�r�b�g����j
-------------------------------------*/
#define    RSP_R3_OCR31                 0x8000        /* Use OCR Busy bit Check */
#define    RSP_R3_OCR_VDD               0x0030        /* Use OCR VDD bit Check (3.2-3.3v,3.3-3.4v is OK)*/

/*--- SCR[0]�ɑ΂���4bit�o�X���Ή��t���O�̃}�X�N ---*/
//SCR��bit50�ɑ������邪�ASCR��SD_STATUS��MSB���瑗���Ă��邱�Ƃ��l�����Ă���
#define    SCR_DAT_BUS_WIDTH_4BIT       0x0400        /* SCR DAT Bus width supported 4bit */

/*--- 127bitCSD (CSD[0]�`CSD[7])�֘A ---*/
#define    CSD_VDD_R_CURR_MIN           0x0038        /* "VDD_R_CURR_MIN" (for CSD[3]) */
#define    CSD_VDD_R_CURR_MAX           0x0007        /* "VDD_R_CURR_MAX" (for CSD[3]) */
#define    CSD_VDD_W_CURR_MIN           0xE000        /* "VDD_W_CURR_MIN" (for CSD[2]) */
#define    CSD_VDD_W_CURR_MAX           0x1C00        /* "VDD_W_CURR_MAX" (for CSD[2]) */

#define    CSD_READ_BL_LEN              0x0F00        /* "READ_BL_LEN" (for CSD[4])*/
#define    CSD_WRITE_BL_LEN_BIT_25_24   0x0003        /* "WRITE_BL_LEN" (for CSD[1])*/
#define    CSD_WRITE_BL_LEN_BIT_23_22   0xC000        /* "WRITE_BL_LEN" (for CSD[0]*/
#define    CSD_READ_BL_PARTIAL          0x0080        /* "READ_BL_PARTIAL" (for CSD[4]) */
#define    CSD_TRANSFER_RATE            0x0700        /* "Transfer rate unit" of "TRAN_SPEED" (for CSD[5]) */
#define    CSD_TRAN_SPEED_100K          0x0000        /* 100Kbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_1M            0x0001        /* 1Mbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_10M           0x0002        /* 10Mbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_100M          0x0003        /* 100Mbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_OTHER         0x0004        /* Reserve (for CSD Transfer rate) */

#define    CSD_C_SIZE_MULT              0x0380        /* RSP2 �� bit[49:47] */
#define    CSD_C_SIZE_BIT_73_72         0x0003        /* RSP3 �� bit[73:72](C_SIZE) */
#define    CSD_C_SIZE_BIT_71_62         0xFFC0        /* RSP3 �� bit[71:62](C_SIZE) */
//SDHC(CSD format version2)�̏ꍇ
#define    CSD_C_SIZE_BIT_69_56         0x3FFF        /* SD_CSD[3] */
#define    CSD_C_SIZE_BIT_55_48         0xFF00        /* SD_CSD[2] */

#define    CSD_STRUCT_BIT_127_126       0x00C0        /* SD_CSD[7] */

//#define    VDD_R_CURR_MIN                0x0000        /* CSD max read current@VDD min */
//#define    VDD_R_CURR_MAX                0x0007        /* CSD max read current@VDD max */
//#define    VDD_W_CURR_MIN                0x0000        /* CSD max write current@VDD min */
//#define    VDD_W_CURR_MAX                0x0007        /* CSD max write current@VDD max */

/*--- R1���X�|���X�� card status(32bit)�ɑ΂���}�X�N ---*/
#define    RSP_R1_STATUS_ERR            0xF9FF0008    /* R1�i���X�|���X�j�̃J�[�h�X�e�[�^�X�̃`�F�b�N */
#define    SDCARD_STATUS_OUT_OF_RANGE   0x80000000    /* Card Status OUT_OF_RANGE �̃`�F�b�N */
#define    RSP_R1_CURRENT_STATE         0x1E00        /* CARD current state */
/*-------------------------------------------------------*/

/* ���X�|���X��RSP0 & RSP_R1_CURRENT_STATE��1�r�b�g�E�V�t�g�����l�ɑ΂���t���O */
#define    CURRENT_STATE_DATA           0x0500        /* CARD current state data */
#define    CURRENT_STATE_RCV            0x0600        /* CARD current state rcv */

/* �J�[�h�X�e�[�^�X */
#define    SD_MEMORY_CARD               0x00FF        /* SD_CARD_TYPE SD memory card */


/*-------------------------------------
�v���g�^�C�v�錾
-------------------------------------*/
void    SD_Init(void);                          /* SD Card�C���^�[�t�F�[�X�������Z�b�g&�����ݒ� */
void    SD_EnableInfo(void);                    /* SD Card �}�� ���荞�݃C�l�[�u���E�f�B�X�G�[�u�� */
u16     SD_Command(u16 ucCommand);              /* SD�J�[�h�R�}���h���o */
u16     SD_AppCommand(void);                    /* SD�J�[�h RCA = 1���Z�b�g�� CMD55 ���s */
u16     SD_AppOpCond(void);                     /* ACMD41 ���s busy�łȂ��Ȃ�܂ŌJ��Ԃ� */
u16     SD_SendOpCond(void);                    /* CMD1 ���s busy�łȂ��Ȃ�܂ŌJ��Ԃ� */
u16     SD_SendIfCond(void);                    /* CMD8 ���s (SDHC�̂ݔ������Ă���) */
u16     SD_SendRelativeAddr(void);              /* CMD3 ���s ����I���� RCA<-Res��RCA */
u16     SD_SelectCard(void);                    /* CMD7 ���s Command toggles acard between the Stand-by and Transfer states */
u16     SD_SetBlockLength(u32 ulBlockLength);   /* �u���b�N�����O�X(1�Z�N�^�̓]����)�̐ݒ� */
u16     SD_SendCID(void);                       /* card identification data �̎擾�R�}���h���s */
u16     SD_SendCSD(void);                       /* card-specific data �̎擾�R�}���h���s */
u16     SD_SendSCR(void);                       /* SD register �̎擾�R�}���h���s */
u16     SD_SDStatus(void);                      /* SD STATUS �̎擾�R�}���h���s */
u16     SD_SendStatus(void);                    /* SD status register �̎擾�R�}���h���s */
u16     SD_MultiReadBlock(u32 ulOffset);        /* �}���`�Z�N�^���[�h�R�}���h���s */
u16     SD_ClockDivSet(u16 usTranSpeed);        /* �J�[�h�̓���N���b�N�ݒ� */

void    SD_EnableClock( void);                  /* SD�J�[�h�̃N���b�N�L�� */
void    SD_DisableClock( void);                 /* SD�J�[�h�̃N���b�N�����i�ȓd�́j */

u16     SD_SelectBitWidth(s16 b4bit);           /* �r�b�g���̑I�� */

u16     MMCP_WriteBusWidth(s16 b4bit);
u16     MMCP_BusTest( BOOL readflag);


BOOL    SD_FPGA_irq(void);                      /* �J�[�h�]���v������FPGA�̐��� */
void    SD_StopTransmission(void);              /* �J�[�h�]���I����FPGA�ɒʒm����B */
void    SD_TransEndFPGA(void);                  /* �J�[�h�]���̏I������(���荞�݃}�X�N��߂�) */
u16     SD_CheckStatus(BOOL bRead);             /* Normal response command �J�[�h�X�e�[�^�X�̃`�F�b�N */
u16     SD_SwapByte(u16 *data);                 /* ���byte�A����byte�����ւ���֐� */

void    SD_EnableSeccnt( u32 ulSDCARD_SectorCount); /* SD_SECCNT���W�X�^�L����&�l�ݒ� */
void    SD_DisableSeccnt( void);                    /* SD_SECCNT���W�X�^������ */

void    SD_SetErr(u16 Error);                   /* �G���[�X�e�[�^�X��ݒ肷�� */
void    SD_ClrErr(u16 Error);                   /* �G���[�X�e�[�^�X���N���A���� */

BOOL    SD_CheckFPGAReg(u16 reg,u16 value);     /* IP���W�X�^�Ƀt���O�������Ă��邩���� */

void    SD_TransReadyFPGA(void);                /* �]����������FPGA�ݒ� */
u16     SD_TransCommand(u16 ucCommand);         /* ���ߔ��s���� */

u16     SD_MultiWriteBlock(u32 ulOffset);       /* �}���`�Z�N�^���C�g�R�}���h���s */
u16     SD_SendNumWRSectors(void);              /* ACMD22 �������݊����Z�N�^���擾�R�}���h���s */


#endif /* __SD_CARD_IP_H__ */
