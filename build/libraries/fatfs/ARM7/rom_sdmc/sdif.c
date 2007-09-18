/*
  Project:  TwlBrom SD Card driver
  File:     sd_card.c

  2007, Research and Development Department, Nintendo.
*/

#include <brom.h>
//#include "sdmc_config.h"
//#include "sdif_reg.h"       /*  IP �Ή����W�X�^��` */
//#include "sdmc.h"
//#include "sdif_ip.h"        /*  IP �Ή��t���O��` */


/*
#if (SD_DEBUG_PRINT_ON == 1)
    #if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
        #define PRINTDEBUG    osTPrintf
    #else
        #include <ctr/vlink.h>
        #define PRINTDEBUG    vlink_dos_printf
    #endif
#else
    #define PRINTDEBUG( ...) ((void)0)
#endif
*/
//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)


#define ADD_CHECK 1

/***********************************************************************
    �O���[�o��
***********************************************************************/
u16    SD_CID[8];        /* CID�ۑ��p (Card IDentification register) : ID*/
u16    SD_CSD[8];        /* CSD�ۑ��p (Card Specific Data register) : spec*/
u16    SD_OCR[2];        /* OCR�ۑ��p (Operation Condition Register) : voltage and status*/
u16    SD_SCR[4];        /* SCR�ۑ��p (Sd card Configulation Register) : bus-width, card-ver, etc*/
u16    SD_RCA;           /* RCA�ۑ��p (Relative Card Address register) : address*/

s16    SDCARD_MMCFlag;   /* MMC�J�[�h�t���O */
s16    SDCARD_SDHCFlag;  /* SDHC�J�[�h�t���O(�����ł�PhysicalLayer2.0�̈�) */
u16    SD_port_number;   /* ���݂̃|�[�g�ԍ� */

/***********************************************************************
    �O���Q�ƕϐ�
***********************************************************************/
volatile SDMC_ERR_CODE   SDCARD_ErrStatus;        /* �G���[�X�e�[�^�X */
vu32            SDCARD_Status;           /* �J�[�h�X�e�[�^�X */
//extern volatile s16    SDCARD_OutFlag;    /* �J�[�h�r�o��������t���O */
//extern void (*func_SDCARD_Out)(void);     /* �J�[�h�r�o�C�x���g�p�R�[���o�b�N�ۑ��p */

extern void SDCARD_TimerCheck( void);
extern void SYSFPGA_irq(void);





void SD_DisableInfo( void);



/*---------------------------------------------------------------------------*
  Name:         SD_Init

  Description:  reset and initialize SD card interface
                SD�J�[�hIP�̃��Z�b�g�Ə����ݒ�

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_Init(void)
{
    SD_AndFPGA( SOFT_RST,(~(SOFT_RST_SDIF_RST))); /* SD I/F ���W���[�������Z�b�g */
    SD_OrFPGA( SOFT_RST,((SOFT_RST_SDIF_RST)));   /* SD I/F ���W���[�������Z�b�g���A */

    SD_AndFPGA( SD_STOP,(~SD_STOP_STP));          /* �f�[�^�]���I���N���A */
}

/*---------------------------------------------------------------------------*
  Name:         SD_EnableInfo

  Description:  enable SD card insert and remove interrupts.
                SD�J�[�h�̑}��/���� ���荞�݂�������

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_EnableInfo( void)
{
    if(SD_port_number == SDCARD_PORT0)
    {
        SD_AndFPGA(SD_INFO1,(~(SD_INFO1_INSERT | SD_INFO1_REMOVE))); /* SD_INFO1���W�X�^�� card inserted removed���N���A */
        SD_AndFPGA(SD_INFO1_MASK,(~(SD_INFO1_MASK_INSERT | SD_INFO1_MASK_REMOVE))); /* �}/�� ���荞�݋��� */
    }
    else if(SD_port_number == SDCARD_PORT1)
    {    /* �|�[�g1��CD�[�q���q�����Ă��Ȃ��̂Ŏ������� */
        SD_AndFPGA(EXT_CD,(~(EXT_CD_PORT1_INSERT | EXT_CD_PORT1_REMOVE)));                /* EXT_CD ���W�X�^�� card inserted removed���N���A */
        SD_AndFPGA(EXT_CD_MASK,(~(EXT_CD_MASK_PORT1INSERT | EXT_CD_MASK_PORT1REMOVE)));    /* �}�� ���荞�݋��� */
    }
}
/*---------------------------------------------------------------------------*
  Name:         SD_DisableInfo

  Description:  disable SD card insert and remove interrupts.
                SD�J�[�h�̑}��/���� ���荞�݂��֎~����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_DisableInfo( void)
{
    if(SD_port_number == SDCARD_PORT0)
    {
        SD_AndFPGA(SD_INFO1,(~(SD_INFO1_INSERT | SD_INFO1_REMOVE))); /* SD_INFO1���W�X�^�� card inserted removed���N���A */
        SD_OrFPGA(SD_INFO1_MASK,(SD_INFO1_MASK_INSERT | SD_INFO1_MASK_REMOVE)); /* �}/�� ���荞�݋֎~ */
    }
    else if(SD_port_number == SDCARD_PORT1)
    {    /* �|�[�g1��CD�[�q���q�����Ă��Ȃ��̂Ŏ������� */
        SD_AndFPGA(EXT_CD,(~(EXT_CD_PORT1_INSERT | EXT_CD_PORT1_REMOVE))); /* EXT_CD ���W�X�^�� card inserted removed���N���A */
        SD_OrFPGA(EXT_CD_MASK,(EXT_CD_MASK_PORT1INSERT | EXT_CD_MASK_PORT1REMOVE)); /* �}�� ���荞�݋֎~ */
    }
}

/*---------------------------------------------------------------------------*
  Name:         SD_Command

  Description:  send command that the card will response only.
                �R�}���h�𑗏o����i���X�|���X���Ԃ��Ă��邾���̃R�}���h�p�j

  Arguments:    ucCommand : command number

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16 SD_Command(u16 ucCommand)
{
    PRINTDEBUG( "command\n");
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_ERR_ALLCLR));           /* SD Card I/F �� �S�G���[���N���A */
    SD_AndFPGA(SD_INFO1,(~SD_INFO1_RES_END));              /* SD_INFO1���W�X�^�� Response end �N���A */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_ALLERRMASK)); /* SD Card I/F �� �S�G���[�����݋��� */

    SD_SetFPGA(SD_CMD,(ucCommand));                        /* �R�}���h���s */

    while(!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_RES_END)){    /* Response end �҂� */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IP�G���[����?
            SYSFPGA_irq();
            break;
        }
        if( SDCARD_ErrStatus != 0) {
            break;
        }
    }

    /*ROM�ł̓G���[���荞�݂�����Ȃ�����RES_END�����ƃG���[�����������Ƃ�����̂ōŌ��INFO2�]��*/
    if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IP�G���[����?
        SYSFPGA_irq();
    }
  
    SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_ALLERRMASK));   /* SD Card I/F �̑S�G���[�����݋֎~ */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_AppCommand

  Description:  send apprication command.
                �A�v���P�[�V�����R�}���h(CMD55)�𑗏o����

  Arguments:    None

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16 SD_AppCommand(void)
{
    PRINTDEBUG( " CMD55 (APP_CMD)\n");

    /* argument���Z�b�g */
    SD_SetFPGA(SD_ARG0,(0x0000));            /* Argument(15:0) = stuff bits */
    SD_SetFPGA(SD_ARG1,(SD_RCA));            /* Argument(31:16) = RCA */

    SD_Command(SD_CMD_CMD | APP_CMD);        /* CMD55���s�A���X�|���X(R1)�҂� */

#if ADD_CHECK
    /*SD_CheckStatus���s���ƁA���O��CMD8�𔭍s���Ă����ꍇ�ASDHC�ȊO��CardStatus��
      IllegalCommand�G���[�t���O�������Ă��܂��̂ŁA�����ň��������邱�ƂɂȂ�
     �i���̃t���O���N���A�����̂�1�R�}���h�Ԃ�x��邽�߁j
      SD Physical Layer �d�l���� Card Status�Q��*/
//    SD_CheckStatus(FALSE);                 /* �R�}���h���X�|���X(R1) �� Card Status �`�F�b�N */
#endif

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_AppOpCond

  Description:  get operating condition register data of SD card.
                SD�J�[�h��ACMD41�𔭍s����OCR���擾����

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_AppOpCond(void)
{
    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));      /* �^�C���A�E�g�ȊO�̃G���[���N���A */

    while(!SDCARD_ErrStatus){                      /* �G���[���������Ȃ��Ԃ͌J��Ԃ� */
        /* Argument(31:0) = OCR without busy (0x00100000 = 3.2-3.3V) */
        /*�z�X�g���œd����I���ł���ꍇ�Ȃǂ́A�����SD_ARG1��0�ɂ��邱�Ƃɂ��
          �J�[�h���Ή����Ă���d����₢���킹�邱�Ƃ��ł���BCTR�͓d��3.3V���ߑł�
          �Ȃ̂Ŗ₢���킹�����A�����Ȃ�3.3V�Ή����J�[�h�ɗv������B�Ή��ł��Ȃ�
          �J�[�h�� Inactive Mode �Ɉڍs����B*/
        SD_SetFPGA(SD_ARG0,(0x0000));
        if( SDCARD_SDHCFlag) {
            SD_SetFPGA(SD_ARG1,(0x4010));
        }else{
            SD_SetFPGA(SD_ARG1,(0x0010));
        }

        PRINTDEBUG( " ACMD41 (SD_SEND_OP_COND)\n");
        SD_Command(SD_CMD_ACMD | SD_APP_OP_COND);  /* ACMD41���s�A���X�|���X(R3)�҂� */

        if(!SDCARD_ErrStatus){                     /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j*/
            SD_GetFPGA(SD_OCR[0],SD_RSP0);         /* ���X�|���X(R3)����OCR�擾 */
            SD_GetFPGA(SD_OCR[1],SD_RSP1);         /* ���X�|���X(R3)����OCR�擾 */
            if(SD_RSP1 & RSP_R3_OCR31){            /* OCR(���W�X�^)��31bit�ڃ`�F�b�N(busy?) */
                break;
            }
        }
        SD_AppCommand();                           /* ����ς݂�RCA��ݒ肵CMD55���s */
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendOpCond

  Description:  send CMD1 without busy for MMC
                MMC �� CMD1�𔭍s����iSD��ACMD41�Ɉʒu����R�}���h�j

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendOpCond(void)
{
    PRINTDEBUG( " CMD1  (SEND_OP_COND)\n");

    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));  /* �^�C���A�E�g�G���[���N���A */

    while(!SDCARD_ErrStatus){                  /* �G���[���������Ȃ��Ԃ͌J��Ԃ� */
        SD_SetFPGA(SD_ARG0,(0x0000));          /* Argument(15:0) for MMC (None for SD) */
        SD_SetFPGA(SD_ARG1,(0x0010));          /* Argument(31:16) for MMC (None for SD) */
        SD_Command(SD_CMD_CMD | SEND_OP_COND); /* CMD1���s�A���X�|���X(R1)�҂� */
        if(!SDCARD_ErrStatus){                 /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j */
            SD_GetFPGA(SD_OCR[0],SD_RSP0);
            SD_GetFPGA(SD_OCR[1],SD_RSP1);
            if(SD_RSP1 & RSP_R3_OCR31){        /* OCR(���W�X�^)��31bit�ڃ`�F�b�N(busy?) */
                break;
            }
        }
    }
    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         SD_SendIfCond

  Description:  send CMD8
                CMD8�𔭍s����iPhysical Layer 2.00�Œǉ����ꂽ�R�}���h�j

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendIfCond(void)
{
#if 0
    PRINTDEBUG( " CMD8  (SEND_IF_COND)\n");

    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));  /* �^�C���A�E�g�G���[���N���A */

    /* (31:12) Reserved bits, (11:8) supply voltage(VHS), (7:0) check pattern */
    SD_SetFPGA(SD_ARG0,(0x01AA));              /* Argument */
    SD_SetFPGA(SD_ARG1,(0x0000));              /* Argument */

    SD_Command(SD_CMD_CMD | SEND_IF_COND_EXT); /* CMD8���s�A���X�|���X(R7)�҂� */

    if(!SDCARD_ErrStatus){                     /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j */
        SDCARD_SDHCFlag = TRUE;                /* SDHC */
//        SD_GetFPGA(SD_R7[0],SD_RSP0);
//        SD_GetFPGA(SD_R7[1],SD_RSP1);
    }else{
#endif
        SDCARD_SDHCFlag = FALSE;
#if 0
    }
#endif
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendRelativeAddr

  Description:  send relative addr
                CMD3�𔭍s���ASD�J�[�h�̏ꍇ�̓��X�|���X��RCA���擾����
                �iRCA�͑��̃R�}���h���s����Argument�Ƃ��ĕK�v�j

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendRelativeAddr(void)
{
    PRINTDEBUG( " CMD3  (SEND_RELATIVE_ADDR)\n");

    if(SDCARD_MMCFlag){                          /* MMC�J�[�h�t���O ON �H */
        SD_SetFPGA(SD_ARG0,(0x0000));            /* Argument(15:0) */
        SD_SetFPGA(SD_ARG1,(0x0001));            /* Argument(31:16) = 0x0001 (����drv�͂Ȃ���0x0100�ɂ��Ă���) */
    }                            /* SD�J�[�h�̂Ƃ���Argument(31:0) = stuff bits */

    SD_Command(SD_CMD_CMD | SEND_RELATIVE_ADDR); /* CMD3���s�A���X�|���X(R6)�҂� */

    if(!SDCARD_ErrStatus){                       /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j */
        if(SDCARD_MMCFlag){                      /* MMC�J�[�h�t���O ON �H */
            SD_RCA = 0x0001;                     /* RCA <- 1 (����drv�͂Ȃ���0x0100�ɂ��Ă���) */
        }else{
            SD_GetFPGA(SD_RCA,SD_RSP1);          /* ���X�|���X���W�X�^����RCA���擾 */
        }
    }
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SelectCard

  Description:  toggle card between the stand-by and transfer states.
                CMD7�𔭍s���ăX�^���o�C���[�h�Ɠ]�����[�h��؂�ւ���

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SelectCard(void)
{
    PRINTDEBUG( " CMD7  (SELECT#/DESELECT_CARD)\n");

    SD_SetFPGA(SD_ARG0,(0x0000));         /* Argument(15:0) = stuff bits*/
    SD_SetFPGA(SD_ARG1,(SD_RCA));         /* Argument(31:16) = RCA */

    SD_Command(SD_CMD_CMD | SELECT_CARD); /* CMD7���s�A���X�|���X(R1b)�҂� */

#if ADD_CHECK
    SD_CheckStatus(FALSE);                /* �R�}���h���X�|���X(R1) �� Card Status �`�F�b�N */
#endif

    return SDCARD_ErrStatus;

}

/*---------------------------------------------------------------------------*
  Name:         SD_SetBlockLength

  Description:  set of block length.
                SD�J�[�h�̃u���b�N�����O�X��ݒ肷��B
                ���ӁFmultiple read �̂Ƃ��� SD_SIZE���W�X�^��512�ȊO�̒l��
                �ݒ肷��Ɛ��������삵�Ȃ�(IP reg spec�������)�B

  Arguments:    ulBlockLength : bytes of a block ( must be multiplier of 2)
                (memo : the default block length is as specified in the CSD.)
                1�u���b�N�̒����F2�̏搔���w�肷�邱��

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SetBlockLength(u32 ulBlockLength)
{
    u16 usValue;

    PRINTDEBUG( " CMD16 (SET_BLOCKLEN)\n");

    /*------- IP�� transfer length ��ݒ� -------*/
    if(ulBlockLength == 512){
        SD_SetFPGA(SD_SIZE,(SD_SIZE_DATA_LENGTH_512B)); /* SD�J�[�h�f�[�^�]���T�C�Y 512Bytes �ݒ� */
    }else{
        usValue = (u16)ulBlockLength;                   /* 16bit�ɕϊ� */
        SD_SetFPGA(SD_SIZE,(usValue));                  /* IP��SD�J�[�h�f�[�^�]���T�C�Y��ݒ� */
    }/*------------------------------------------*/

    /*------- �J�[�h�̐ݒ� -------*/
    /* argument(31:0) = block length */
    SD_SetFPGA(SD_ARG0,(((LELONG *)&ulBlockLength)->dt2word.low));
    SD_SetFPGA(SD_ARG1,(((LELONG *)&ulBlockLength)->dt2word.high));

    SD_Command(SD_CMD_CMD | SET_BLOCKLEN);              /* CMD16���s�A���X�|���X(R1)�҂� */
#if ADD_CHECK
    SD_CheckStatus(FALSE);                              /* �R�}���h���X�|���X(R1) �� Card Status �`�F�b�N */
#endif
    /*----------------------------*/
    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         SD_SendCID

  Description:  get card identification data (128bits).
                SD�J�[�h��CID�l���擾����

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendCID(void)
{
    PRINTDEBUG( " CMD2  (ALL_SEND_CID)\n");
                                            /* Argument(31:0) = stuff bits */
    SD_Command(SD_CMD_CMD | ALL_SEND_CID);  /* CMD2���s�A���X�|���X(R2)�҂� */

    if(!SDCARD_ErrStatus){                  /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j */
        SD_GetFPGA(SD_CID[0],SD_RSP0);
        SD_GetFPGA(SD_CID[1],SD_RSP1);
        SD_GetFPGA(SD_CID[2],SD_RSP2);
        SD_GetFPGA(SD_CID[3],SD_RSP3);
        SD_GetFPGA(SD_CID[4],SD_RSP4);
        SD_GetFPGA(SD_CID[5],SD_RSP5);
        SD_GetFPGA(SD_CID[6],SD_RSP6);
        SD_GetFPGA(SD_CID[7],SD_RSP7);
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendCSD

  Description:  get card specific data
                SD�J�[�h��CSD�l���擾����

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendCSD(void)
{
    PRINTDEBUG( " CMD9  (SEND_CSD)\n");

    SD_SetFPGA(SD_ARG0,(0x0000));       /* Argument(15:0) = stuff bits */
    SD_SetFPGA(SD_ARG1,(SD_RCA));       /* Argument(31:16) = RCA */

    SD_Command(SD_CMD_CMD | SEND_CSD);  /* CMD9���s�A���X�|���X(R2)�҂� */

    if(!SDCARD_ErrStatus){              /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j */
        SD_GetFPGA(SD_CSD[0],SD_RSP0);
        SD_GetFPGA(SD_CSD[1],SD_RSP1);
        SD_GetFPGA(SD_CSD[2],SD_RSP2);
        SD_GetFPGA(SD_CSD[3],SD_RSP3);
        SD_GetFPGA(SD_CSD[4],SD_RSP4);
        SD_GetFPGA(SD_CSD[5],SD_RSP5);
        SD_GetFPGA(SD_CSD[6],SD_RSP6);
        SD_GetFPGA(SD_CSD[7],SD_RSP7);
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendStatus

  Description:  send status read command (compatible to the MMC protocol).
                �}���`���f�B�A�J�[�h�݊������ł�status�擾�R�}���h���s

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendStatus(void)
{
    PRINTDEBUG( " CMD13 (Send STATUS)\n");

    SD_SetFPGA(SD_ARG0,(0x0000));                          /* Argument(15:0) = stuff bits */
    SD_SetFPGA(SD_ARG1,(SD_RCA));                          /* Argument(31:16) = RCA */

    /* �J�[�h�r�o�̏d����h������ */
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_ERR_ALLCLR));           /* SD Card I/F �� �S�ẴG���[���N���A */
    SD_AndFPGA(SD_INFO1,(~SD_INFO1_RES_END));              /* SD_INFO1���W�X�^�� Response end �N���A */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_ALLERRMASK)); /* SD Card I/F �� �S�G���[�����݋��� */

    SD_SetFPGA(SD_CMD,(SD_CMD_CMD | SD_SEND_STATUS));      /* CMD13���s */

    while(!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_RES_END)){    /* Response end (R1)�҂� */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IP�G���[����?
            SYSFPGA_irq();
            break;
        }
        if( SDCARD_ErrStatus != 0) {
            break;
        }
    }

    /*ROM�ł̓G���[���荞�݂�����Ȃ�����RES_END�����ƃG���[�����������Ƃ�����̂ōŌ��INFO2�]��*/
    if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IP�G���[����?
        SYSFPGA_irq();
    }
    SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_ALLERRMASK));   /* SD Card I/F �� �S�G���[�����݋֎~ */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendSCR

  Description:  get condition data.
                SD�J�[�h����SCR���W�X�^�l�擾�R�}���h���s�B���̌�J�[�h��
                DAT���C���o�R��1�u���b�N(8Bytes�ɐݒ肵�Ă�������)���M���Ă���B
                MultiBlock R/W �ƈقȂ�ADAT���C���o�R�œ]������Ă���SD�J�[�h��
                ���W�X�^�́AMSB�����ɑ����Ă��邱�Ƃɒ��ӁB
                (Physical Layer Specification 2.00 p12-13�Q��)

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendSCR(void)
{
    PRINTDEBUG( " ACMD51 (Send SCR)\n");
                                                    /* Argument(31:0) = stuff bits */
#if ADD_CHECK
    SD_TransReadyFPGA();                            /* �֘A���W�X�^������ */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE)); /* SD�J�[�h����̃f�[�^�Ǐo���v�������݋���*/

    SD_TransCommand((SD_CMD_ACMD | SEND_SCR));      /* SCR�擾�R�}���h���s�A���X�|���X(R1)�҂� */
#endif

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SDStatus

  Description:  send status read command (for SD card only).
                SD�J�[�h��p�����ł�status�擾�R�}���h���s�B���̌�J�[�h��
                DAT���C���o�R��1�u���b�N(64Bytes�ɐݒ肵�Ă�������)���M���Ă���B
                MultiBlock R/W �ƈقȂ�ADAT���C���o�R�œ]������Ă���SD�J�[�h��
                ���W�X�^�́AMSB�����ɑ����Ă��邱�Ƃɒ��ӁB
                (Physical Layer Specification 2.00 p12-13�Q��)

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SDStatus(void)
{
    PRINTDEBUG( " ACMD13 (SD_SD STATUS)\n");
                                                    /* Argument(31:0) = stuff bits */
    SD_TransReadyFPGA();                            /* �֘A���W�X�^������ */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE)); /* SD�J�[�h����̃f�[�^�Ǐo���v�������݋���*/

    SD_TransCommand((SD_CMD_ACMD | SD_STATUS));     /* ACMD13 Send the SD_CARD status �R�}���h���s�A���X�|���X(R1)�҂� */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_MultiReadBlock

  Description:  send multiple block read command.
                �}���`�u���b�N���[�h�R�}���h���s�B���̌�J�[�h��
                DAT���C���o�R�Ńf�[�^�𑗐M���Ă���B

  Arguments:    ulOffset : offset address to read(BYTE).

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_MultiReadBlock(u32 ulOffset)
{
    PRINTDEBUG( " CMD18 (READ_MULTIPLE_BLOCK)\n");

    SD_TransReadyFPGA();                                /* INFO���W�X�^������ */
    if( !SDCARD_UseFifoFlag) {                          /* FIFO���g��Ȃ��Ƃ� */
        SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE)); /* SD�J�[�h����̃f�[�^�Ǐo���v�������݋���*/
    }

    /* �ǂݍ��݊J�n�A�h���X�i�I�t�Z�b�g�j�ݒ� */
    SD_SetFPGA(SD_ARG0,(((LELONG *)&ulOffset)->dt2word.low));
    SD_SetFPGA(SD_ARG1,(((LELONG *)&ulOffset)->dt2word.high));

    SD_TransCommand((READ_MULTIPLE_BLOCK));             /* CMD18(�}���`�Z�N�^���[�h�R�}���h)���s�A���X�|���X(R1)�҂� */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_ClockDivSet

  Description:  set clock speed into the SD card.
                (notice : clock into IP is 33.51MHz.)
                (memo : default clock is 262KHz.)
                SD�J�[�h�ւ̃N���b�N���x��ݒ肷��BCTR����IMCLK�[�q��IP�ɋ���
                ������{�N���b�N��33.51MHz�B
                �N������̃f�t�H���g�ł�128����(262KHz)���ݒ肳��Ă���B

  Arguments:    CSD responce include "TRAN_SPEED".
                �\��CSD��SD�J�[�h����ǂݏo���Ă������ATRAN_SPEED�������Ă���
                ������CSD(CSD[5])�BSD�J�[�h���őΉ��\�Ȏ��g�����Q�Ƃ��邽�߁B

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 SD_ClockDivSet(u16 usTranSpeed)
{
    u16 usTranTime;

    //CTR�ł́AIMCLK�ɓ��͂����N���b�N�� 33.51Mhz
    //SD_CLK_CTRL �� default�l�� 0x0020( 128���� = 262kHz)
    /*------*/
    usTranTime = (u16)((usTranSpeed >> 11) & 0x000F); /* CSD[103:96] �� time value(=4bit) �擾 */

    usTranSpeed &= CSD_TRANSFER_RATE;                 /* CSD[103:96] �� transfer rate unit ���擾 */
    usTranSpeed = (u16)(usTranSpeed >> 8);            /* transfer rate unit ������8�r�b�g�ɐݒ肷�� */
    /*------*/


    switch( usTranSpeed) {
      /*--- 100kbit/s(one dat line) = 100KHz, �̔{���̂Ƃ� ---*/
      case CSD_TRAN_SPEED_100K:
        if(usTranTime > 0x000C){                               /* time value �� 5.5 ���傫��? */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_64));          /* 523KHz */
        }
        else{
            if(usTranTime > 0x0006){                           /* time value �� 2.5 ���傫��? */
                SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128));     /* 262KHz */
            }else{
                if( usTranTime == 1) {                         /* time value �� 1.0 ? */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_512)); /* 65KHz */
                }else{                                         /* time value �� 1.2�`2.5 �̂Ƃ� */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_256)); /* 131KHz */
                }
            }
        }
        break;
      /*--- 1Mbit/s(one dat line) = 1MHz, �̔{���̂Ƃ� ---*/
      case CSD_TRAN_SPEED_1M:
        if(usTranTime == 0x0001){                              /* time value �� 1.0 ? */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_64));          /* 523KHz */
        }
        else{
            if(usTranTime <= 0x0005){                          /* time value �� 2.0 �ȉ�? */
                SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_32));      /* 1.05MHz */
            }
            else{
                if(usTranTime <= 0x0009){                      /* time value �� 4.0 �ȉ�? */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_16));  /* 2.095MHz */
                }else{                                         /* time value �� 4.5 �ȏ�̂Ƃ� */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_8));   /* 4.18MHz */
                }
            }
        }
        break;
      /*--- 10Mbit/s(one dat line) = 10MHz, �̔{���̂Ƃ� ---*/
      case CSD_TRAN_SPEED_10M:
        if(usTranTime > 0x0004){                        /* time value �� 1.5 ���傫��? */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_2));    /* 16.76MHz */
        }else{                                          /* time value �� 1.5 �ȉ��̂Ƃ� */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_4));    /* 8.38MHz */
        }
        break;
      /*--- 100Mbit/s(one dat line) = 100MHz, �̔{���̂Ƃ� ---*/
      case CSD_TRAN_SPEED_100M:                         /* time value ���ǂ�Ȓl�ł����Ă� */
           SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_2));     /* 16.76MHz */
        break;
      default:
        if( usTranSpeed != 7) {                         /* reserved�l(=7)�ȊO��? */
               SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_2)); /* 16.76MHz */
        }                                               /* (����drv�ł́A4�ȏ�Ȃ�288KHz�ɂ��Ă���) */
        break;                                          /* transfer rate unit �� 7 �̂Ƃ��͉������Ȃ� */
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_EnableClock

  Description:  enable clock

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_EnableClock( void)
{
    SD_OrFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_SDCLKEN));
}

/*---------------------------------------------------------------------------*
  Name:         SD_DisableClock

  Description:  disable clock (for power save).

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_DisableClock( void)
{
    SD_AndFPGA(SD_CLK_CTRL,(~SD_CLK_CTRL_SDCLKEN));
}


/*---------------------------------------------------------------------------*
  Name:         SD_SelectBitWidth

  Description:  set bus width for SD Card ( Not for MMC Card).
                (The allowed data bus widths are given in SCR register.)
                SD�J�[�h��DAT���C���o�X����1bit�܂���4bit�ɐݒ肷��B
                SD�J�[�h��4bit�ɑΉ����Ă��Ȃ����AMMC�J�[�h�̏ꍇ�̓o�X����4bit��
                �ݒ肷�邱�Ƃ͂ł��Ȃ��BSD�J�[�h�̋��e����o�X���́A�\�ߎ擾�ς݂�
                SCR���W�X�^�ɂ���ĎQ�Ƃ��Ă���B

  Arguments:    b4bit : TRUE = 4bit, FALSE = 1bit

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 SD_SelectBitWidth(s16 b4bit)
{
    /*--- �J�[�h�̎d�l(SCR)���Q�� ---*/
#if ADD_CHECK
    if( b4bit){                                         /* SCR���W�X�^�����[�h�ςł��邱��! */
        if( !(SD_SCR[0] & SCR_DAT_BUS_WIDTH_4BIT)) {    /* �J�[�h��4bit���ɑΉ����Ă��Ȃ���? */
            b4bit = FALSE;                              /* �Ή����Ă��Ȃ��ꍇ��1bit�����w�肵�Ă��� */
        }
    }/*------------------------------*/
#endif
    /*--- 4bit(TRUE) �w��̂Ƃ� ---*/
    if(b4bit){
        if(!SDCARD_MMCFlag){                            /* MMC�J�[�h�t���O OFF�H(ON�Ȃ牽�����Ȃ�) */
            if(SD_AppCommand()){                        /* RCA�ݒ�� CMD55���s */
                return SDCARD_ErrStatus;                /* CMD55������I�����Ȃ�������G���[�I�� */
            }
            SD_SetFPGA(SD_ARG0,(0x0002));                /* Argument(1:0) = (10) to 4bit,(00) to 1bit */
            SD_SetFPGA(SD_ARG1,(0x0000));                /* Argument(31:2) = stuff bits */
            PRINTDEBUG( " ACMD6 (SET_BUS_WIDTH:4bit)\n");
            SD_Command(SD_CMD_ACMD | SET_BUS_WIDTH);     /* ACMD6�i�r�b�g���I���R�}���h�j���s�A���X�|���X(R1)�҂� */
#if ADD_CHECK
    SD_CheckStatus(FALSE);                               /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
#endif
            if(!SDCARD_ErrStatus){                       /* �G���[�Ȃ�? */
                SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IP�Ƀr�b�g���̐ݒ�(4bit��) */
            }
        }
    }/*--- 1bit(FALSE) �w��̂Ƃ� ---*/
    else{
        if(!SDCARD_MMCFlag){                            /* MMC�J�[�h�t���O OFF�H(ON�Ȃ牽�����Ȃ�) */
            if(SD_AppCommand()){                        /* RCA�ݒ�� CMD55���s���� */
                return SDCARD_ErrStatus;                /* CMD55������I�����Ȃ�������G���[�I�� */
            }
            SD_SetFPGA(SD_ARG0,(0x0000));                /* Argument(1:0) = bus width : 1bit */
            SD_SetFPGA(SD_ARG1,(0x0000));                /* Argument(31:2) = stuff bits */
            PRINTDEBUG( " ACMD6 (SET_BUS_WIDTH:1bit)\n");
            SD_Command(SD_CMD_ACMD | SET_BUS_WIDTH);     /* ACMD6�i�r�b�g���I���R�}���h�j���s�A���X�|���X(R1)�҂� */
#if ADD_CHECK
    SD_CheckStatus(FALSE);                               /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
#endif
        }
        if(!SDCARD_ErrStatus){                           /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j */
            SD_OrFPGA(SD_OPTION,(SD_OPTION_WIDTH_1BIT)); /* IP�Ƀr�b�g���̐ݒ�(1bit��) */
        }
    }/*------------------------------*/

    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         MMCP_WriteBusWidth

  Description:  set bus width for SD Card ( Not for MMC Card).
                (The allowed data bus widths are given in SCR register.)
                MMCplus�J�[�h��DAT���C���o�X����1bit�܂���4bit�ɐݒ肷��B
                SD�܂���MMC�J�[�h�̏ꍇ�̓o�X����4bit�ɐݒ肷�邱�Ƃ͂ł��Ȃ��B

  Arguments:    b4bit : TRUE = 4bit, FALSE = 1bit

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 MMCP_WriteBusWidth(s16 b4bit)
{
    if( !SDCARD_MMCFlag) {
        return 1;
    }

    /* �������݊J�n�I�t�Z�b�g�ݒ� */
    if( b4bit) {
        SD_SetFPGA(SD_ARG0,(0x0100));    /* Argument(15:8)=(0x1) to 4bit,(0x0) to 1bit */
    }else{
        SD_SetFPGA(SD_ARG0,(0x0000));    /* Argument(15:8)=(0x1) to 4bit,(0x0) to 1bit */
    }
    SD_SetFPGA(SD_ARG1,(0x03B7));        /* Argument(25:24)=(0x3)Write, (23:16)=(183)Index */

    /* CMD6�i�r�b�g���I���R�}���h�j���s�A���X�|���X(R1)�҂� */
    SD_Command(SD_CMD_CMD | EXT_CSD_ACCESS);

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         MMCP_BusTest

  Description:  �o�X�̃e�X�g���s���B
                ���̌�J�[�h��DAT���C���o�R�Ńf�[�^�𑗐M����K�v������B

  Arguments:    readflag : ���[�h��TRUE, ���C�g��FALSE

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 MMCP_BusTest( BOOL readflag)
{
    /**/
    SD_TransReadyFPGA();                            /* INFO���W�X�^������ */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BWE)); /* SD�J�[�h����̃f�[�^�����ݗv�������݋��� */

    /**/
    SD_SetFPGA( SD_ARG0, 0);
    SD_SetFPGA( SD_ARG1, 0);

    if( readflag) {
        SD_TransCommand( 14);
    }else{
        SD_TransCommand( 19);
    }

    return( SDCARD_ErrStatus);
}


/*---------------------------------------------------------------------------*
  Name:         SD_FPGA_irq

  Description:  handler of data transfer request interrupt from card,
                check and clear interrupt register of IP.
                �J�[�h����̓]���v���iBRE�܂���BWE�j���荞�ݔ�������
                �ǂ��炩�𒲂ׂĕԂ��AINFO2�̓��Y���荞�ݗv���t���O���N���A����B

  Arguments:    None

  Returns:      TRUE : BRE���荞�ݔ���
                FALSE : BWE���荞�ݔ���
 *---------------------------------------------------------------------------*/
BOOL SD_FPGA_irq(void)
{
    /*--- FIFO���g���Ƃ� ---*/
    if( SDCARD_UseFifoFlag) {
        if( SD_CheckFPGAReg( *SDIF_CNT_L, (u16)SDIF_CNT_FFIE)) {   /* FULL���荞�݋��̂Ƃ� */
            return TRUE;
        }else{
            if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BWE)){ /* SD�J�[�h����̃f�[�^�����ݗv������H */
                SD_AndFPGA(SD_INFO2,(~SD_INFO2_BWE));              /* SD�p�o�b�t�@���� Write Enable ���Z�b�g*/
            }
            return FALSE;
        }
    }else{    /*--- FIFO���g��Ȃ��Ƃ� ---*/
        if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BRE)){ /* SD�J�[�h����̃f�[�^�Ǐo���v������H */
            SD_AndFPGA(SD_INFO2,(~SD_INFO2_BRE));              /* SD�p�o�b�t�@���� Read Enable ���Z�b�g*/
            return TRUE;
        }
        if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BWE)){ /* SD�J�[�h����̃f�[�^�����ݗv������H */
            SD_AndFPGA(SD_INFO2,(~SD_INFO2_BWE));              /* SD�p�o�b�t�@���� Write Enable ���Z�b�g*/
            return FALSE;
        }
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         SD_StopTransmission

  Description:  manual send CMD12 to terminate translate.
                �蓮��CMD12�𔭍s���A�]���I����FPGA�ɒʒm����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_StopTransmission(void)
{
    SD_OrFPGA(SD_STOP,(SD_STOP_STP));        /* �f�[�^�]���I���ݒ� */
}

/*---------------------------------------------------------------------------*
  Name:         SD_TransEndFPGA

  Description:  clear the transfer interrupt.
                �J�[�h�]���I�����ɓ]���֘A�̊��荞�݂��֎~�ɖ߂��Ă���

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_TransEndFPGA(void)
{
    SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_ALLERRMASK));   /* SD Card I/F �� �S�G���[�����݋֎~ */
    SD_OrFPGA(SD_INFO1_MASK,(SD_INFO1_MASK_ALL_END));      /* R/W �A�N�Z�X�I�������݋֎~ */

    if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BRE)){ /* SD�J�[�h����̃f�[�^�Ǎ��ݗv�������݋���? */
        SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_BRE));      /* SD�J�[�h����̃f�[�^�Ǎ��ݗv�������݋֎~ */
    }
    if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BWE)){ /* SD�J�[�h����̃f�[�^�����ݗv�������݋���? */
        SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_BWE));      /* SD�J�[�h����̃f�[�^�����ݗv�������݋֎~ */
    }
}

/*---------------------------------------------------------------------------*
  Name:         SD_CheckStatus

  Description:  check the card status in the R1.
                R1���X�|���X�̃J�[�h�X�e�[�^�X���`�F�b�N����

  Arguments:    bRead : �ǂݍ��ݏ����� = TRUE

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16 SD_CheckStatus(BOOL bRead)
{
    /* �R�}���h���X�|���X(R1)��[39:8] ���� Card status ���擾 */
    SD_GetFPGA((((LELONG *)&SDCARD_Status)->dt2word.low),SD_RSP0);
    SD_GetFPGA((((LELONG *)&SDCARD_Status)->dt2word.high),SD_RSP1);

    /*--- �����T���v���h���C�o�ł���Ă��鏈�� ---*/
    if(bRead){                                            /* ���[�h����? */
        if(!(SDCARD_ErrStatus & SDMC_ERR_TIMEOUT)){       /* �^�C���A�E�g�������Ă��Ȃ���? */
            SDCARD_Status &= ~SDCARD_STATUS_OUT_OF_RANGE; /* OUT_OF_RANGE�t���O�𗎂Ƃ� */
        }
    }/*-------------------------------------------*/

    PRINTDEBUG( " SD_CheckStatus ======== 0x%x\n", SDCARD_Status);

    if(SDCARD_Status & RSP_R1_STATUS_ERR){ /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[���m�F */
        SD_SetErr(SDMC_ERR_R1_STATUS);     /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X �G���[ */
    }
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SwapByte

  Description:  swap bytes in a 16bit data.
                16bit�f�[�^�̏�ʂƉ��ʂ���ꊷ����

  Arguments:    data : 16bit data

  Returns:      swapped data
 *---------------------------------------------------------------------------*/
u16    SD_SwapByte(u16 *data)
{
    u16    usDATA;

    usDATA = *data;
    usDATA = (u16)(((usDATA & 0x00FF) << 8) | ((usDATA & 0xFF00) >> 8));

    return usDATA;
}

/*---------------------------------------------------------------------------*
  Name:         SD_EnableSeccnt

  Description:  enable SD_SECCNT register.
                SD_SECCNT ��L�������Ēl���Z�b�g����B

  Arguments:    ulSDCARD_SectorCount : �Z�N�^�J�E���g�l�i1�Z�N�^=512byte�j

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_EnableSeccnt( u32 ulSDCARD_SectorCount)
{
    u16    usSector;

    usSector = (u16)ulSDCARD_SectorCount;
    SD_OrFPGA(SD_STOP,SD_STOP_SEC_ENABLE); /* SD_SECCNT���W�X�^��L���ɂ��� */
    SD_SetFPGA(SD_SECCNT,usSector);        /* SD_SECCNT���W�X�^�ɓ]���Z�N�^�J�E���g��ݒ� */
}

/*---------------------------------------------------------------------------*
  Name:         SD_DisableSeccnt�����݂͖��g�p�֐���

  Description:  disable SD_SECCNT register.
                SD_SECCNT �𖳌�������B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_DisableSeccnt( void)
{
    u16    i;

    /*--- �����h���C�o�ł���Ă��邱�� ---*/
    for ( i=0; i<50000; i++)                   /* �J�E���g�l50000�ł͑���Ȃ����Ƃ��� */
    {    /* SD_INFO1���W�X�^�� R/W access all end �҂� */
        if(SD_CheckFPGAReg(SD_INFO1,SD_INFO1_ALL_END)) {
            break;
        }
    }/*-----------------------------------*/
    SD_AndFPGA(SD_STOP,(~SD_STOP_SEC_ENABLE)); /* SD_SECCNT���W�X�^�𖳌��ɂ��� */
}

/*---------------------------------------------------------------------------*
  Name:         SD_SetErr

  Description:  set error flag
                �w��̃G���[�t���O���Z�b�g����

  Arguments:    Error : error flag to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void    SD_SetErr(u16 Error)
{
    OSIntrMode irq_core_flag;

#if (TARGET_OS_CTR == 1)
    irq_core_flag = osDisableInterrupts(); /* �����݋֎~ */
        SDCARD_ErrStatus |= Error;         /* �G���[�X�e�[�^�X��ݒ� */
    osRestoreInterrupts( irq_core_flag);   /* ���荞�ݐݒ�����ɖ߂� */
#else
    irq_core_flag = OS_DisableInterrupts();
        SDCARD_ErrStatus |= Error;         /* �G���[�X�e�[�^�X��ݒ� */
    OS_RestoreInterrupts( irq_core_flag);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SD_ClrErr

  Description:  clear error flag
                �w��̃G���[�t���O���N���A����

  Arguments:    Error : error flag to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
void    SD_ClrErr(u16 Error)
{
    OSIntrMode irq_core_flag;

#if (TARGET_OS_CTR == 1)
    irq_core_flag = osDisableInterrupts(); /* �����݋֎~ */
        SDCARD_ErrStatus &= ~(Error);      /* �G���[�X�e�[�^�X���N���A */
    osRestoreInterrupts( irq_core_flag);   /* ���荞�ݐݒ�����ɖ߂� */
#else
    irq_core_flag = OS_DisableInterrupts(); /* �����݋֎~ */
        SDCARD_ErrStatus &= ~(Error);       /* �G���[�X�e�[�^�X���N���A */
    OS_RestoreInterrupts( irq_core_flag);   /* ���荞�ݐݒ�����ɖ߂� */
#endif
}


/*---------------------------------------------------------------------------*
  Name:         SD_TransReadyFPGA

  Description:  setup for the command that the card will response and request data transfer.
                DAT���C���ł̃f�[�^�]������������R�}���h���s�O�̓]������
                �iBRE, BWE���荞�݂̋��͕ʓr�s�����Ɓj

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_TransReadyFPGA(void)
{
    /* �֘A���W�X�^������ */
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_ERR_ALLCLR));                    /* SD Card I/F �� �S�ẴG���[���N���A */
    SD_AndFPGA(SD_INFO1,(~(SD_INFO1_RES_END | SD_INFO1_ALL_END)));    /* SD_INFO1���W�X�^�� Response end �� access all end �N���A */
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_BRE));                            /* SD�p�o�b�t�@���� Read Enable �N���A*/
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_BWE));                            /* SD�p�o�b�t�@���� Write Enable �N���A*/
    SD_AndFPGA(SD_STOP,(~SD_STOP_STP));                                /* �f�[�^�]���I���N���A */

    /* ���荞�݊֘A���� */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_ALLERRMASK));            /* SD Card I/F �� �S�G���[�����݋��� */
    SD_AndFPGA(SD_INFO1_MASK,(~SD_INFO1_MASK_ALL_END));                /* R/W access all end �����݋���(Response�̓|�[�����O�Ō��邽�ߋ����Ȃ�) */
}


/*---------------------------------------------------------------------------*
  Name:         SD_TransCommand

  Description:  send command that the card will response and request data transfer.
                �R�}���h�𑗏o����iDAT���C���ł̃f�[�^�]������������R�}���h�p�j

  Arguments:    ucCommand : command number

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16    SD_TransCommand(u16 ucCommand)
{
    SD_SetFPGA(SD_CMD,(ucCommand));                     /* �R�}���h���s */

    while(!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_RES_END)){ /* Response end �҂� */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IP�G���[����?
            SYSFPGA_irq();
            break;
        }
        if( SDCARD_ErrStatus != 0) {
            break;
        }
    }
  
    /*ROM�ł̓G���[���荞�݂�����Ȃ�����RES_END�����ƃG���[�����������Ƃ�����̂ōŌ��INFO2�]��*/
    if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IP�G���[����?
        SYSFPGA_irq();
    }
  
    SD_CheckStatus(FALSE);                              /* R1���X�|���X��card status�`�F�b�N*/

    if(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS){
        SD_StopTransmission();                          /* �J�[�h�]���I����FPGA�ɒʒm */
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_CheckFPGAReg

  Description:  check register bits.
                ���W�X�^�̏�Ԃ��`�F�b�N����

  Arguments:    reg : 16bit register data
                value : mask data

  Returns:      TRUE : some of mask data bits are 1 on the register.
                FALSE : no mask data bits is 1 on the register.
 *---------------------------------------------------------------------------*/
BOOL    SD_CheckFPGAReg(u16 reg,u16 value)
{
    if( reg & value) {
        return TRUE;
    }else{
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendNumWRSectors

  Description:  send "number of well written blocks" command (for SD card only).
                SD�J�[�h��p�����ł̃��C�g�ς݃Z�N�^���擾�R�}���h���s�B���̌�J�[�h��
                DAT���C���o�R��1�u���b�N(4Bytes�ɐݒ肵�Ă�������)���M���Ă���B
                MultiBlock R/W �ƈقȂ�ADAT���C���o�R�œ]������Ă���SD�J�[�h��
                ���W�X�^�́AMSB�����ɑ����Ă��邱�Ƃɒ��ӁB
                (Physical Layer Specification 2.00 p12-13�Q��)

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendNumWRSectors(void)
{
    PRINTDEBUG( " ACMD22 (SEND_NUM_WR_SECTORS)\n");
                                                          /* Argument(31:0) = stuff bits */
    SD_TransReadyFPGA();                                  /* �֘A���W�X�^������ */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE));       /* SD�J�[�h����̃f�[�^�Ǎ��ݗv�������݋��� */

    SD_TransCommand((SD_CMD_ACMD | SEND_NUM_WR_SECTORS)); /* ACMD22�i�������݊����Z�N�^���擾�R�}���h�j���s�A���X�|���X(R1)�҂� */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_MultiWriteBlock

  Description:  send multiple block write command.
                �}���`�u���b�N���C�g�R�}���h���s�B���̌�J�[�h��
                DAT���C���o�R�Ńf�[�^�𑗐M����K�v������B

  Arguments:    ulOffset : offset address to write(BYTE).

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_MultiWriteBlock(u32 ulOffset)
{
    PRINTDEBUG( " CMD25 (WRITE_MULTIPLE_BLOCK)\n");

    SD_TransReadyFPGA();                                /* INFO���W�X�^������ */
//    if( !SDCARD_UseFifoFlag) {                        /* FIFO���g��Ȃ��Ƃ� */
        SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BWE)); /* SD�J�[�h����̃f�[�^�����ݗv�������݋��� */
//    }

    /* �������݊J�n�I�t�Z�b�g�ݒ� */
    SD_SetFPGA(SD_ARG0,(((LELONG *)&ulOffset)->dt2word.low));
    SD_SetFPGA(SD_ARG1,(((LELONG *)&ulOffset)->dt2word.high));

    SD_TransCommand(WRITE_MULTIPLE_BLOCK);              /* CMD25�i�}���`�Z�N�^���C�g�R�}���h�j���s */

    return SDCARD_ErrStatus;
}


