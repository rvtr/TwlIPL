/*
    Project:    TwlBrom SD port driver
    File:       Carddrv.c

    2006, Research and Development Department, Nintendo.
*/

#include "sdmc_config.h"
#include "sdmc.h"
#include "sdif_reg.h"
#include "sdif_ip.h"
#include <firm.h>
//#include <twl/hw/ARM7/ioreg_MI.h>
//#include "sdmc_config.h"
//#include "sdif_reg.h"            /*  IP �Ή����W�X�^��` */
//#include <brom/rom_sdmc.h>
//#include "sdif_ip.h"            /*  IP �Ή��t���O��` */

//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)


/***********************************************************************
 �萔
***********************************************************************/
static BOOL sdmcGetErrStat( void);


/***********************************************************************
 static�֐��̐錾
***********************************************************************/
static void SDCARD_Backup_port1(void);
static void SDCARD_Restore_port1(void);

static int  MMCP_SetBusWidth( BOOL b4bit);       /* �r�b�g���̑I��(MMCplus, eMMC, moviNAND) */

static SDMC_ERR_CODE SDCARD_Layer_Init(void);
static SDMC_ERR_CODE i_sdmcMPInit( void);     /* �J�[�h�h���C�o������(�}���`�|�[�g�Ή�) */

/***********************************************************************
    �O���Q�ƕϐ�
***********************************************************************/
extern void SDCARD_Timer_irq(void* arg);      /* �^�C���A�E�g���荞�݃n���h�� */
extern void SDCARD_TimerCheck( void);
extern void SDCARD_TimerStart(u32 tim);    /* �^�C���A�E�g�v���X�^�[�g */
extern void SDCARD_TimerStop(void);        /* �^�C���A�E�g�v����~ */


extern u16  SD_CID[8];                /* CID�l�ۑ��p */
extern u16  SD_CSD[8];                /* CSD�l�ۑ��p */
extern u16  SD_OCR[2];                /* OCR�l�ۑ��p */
extern u16  SD_SCR[4];                /* SCR�l�ۑ��p */
extern u16  SD_RCA;                   /* RCA�l�ۑ��p */

extern s16  SDCARD_MMCFlag;           /* MMC�J�[�h�t���O */
extern s16  SDCARD_SDHCFlag;          /* SDHC�J�[�h�t���O */
extern u16  SD_port_number;           /* ���݃|�[�g�ԍ�  */


/***********************************************************************
    �O���[�o��
***********************************************************************/
static u16  SD_SDSTATUS[32];            /* SD_STATUS���W�X�^�ۑ��p */
static u16  SDCARD_WP_FLAG0;            /* �J�[�h���C�g�v���e�N�g�t���O�B0=�Ȃ��A1=�L��    */
static u16  SDCARD_WP_FLAG1;            /* �J�[�h���C�g�v���e�N�g�t���O�B0=�Ȃ��A1=�L��    */
static u16  SDCARD_WP_PERMANENT;        /* �J�[�h���C�g�v���e�N�g�i�v�t���O�B0=�Ȃ��A1=�L��    */
static u16  SDCARD_WP_TEMPORARY;        /* �J�[�h���C�g�v���e�N�g�ꎞ�t���O�B0=�Ȃ��A1=�L��    */

static u32  SDCARD_SectorSize;          /* �Z�N�^�T�C�Y �f�t�H���g 512bytes */

static u16  SD_INFO1_VALUE;             /* SD_INFO1���W�X�^�擾�p�ϐ� */
static u16  SD_INFO1_MASK_VALUE;        /* SD_INFO1�����݃}�X�N�p�ϐ�(0�ŋ���, 1�ŋ֎~) */
static u16  SD_INFO2_VALUE;             /* SD_INFO2���W�X�^�擾�p�ϐ� */
static u16  SD_INFO2_MASK_VALUE;        /* SD_INFO2���荞�݃}�X�N�p�ϐ�(0�ŋ���, 1�ŋ֎~) */
static u16  SD_INFO_ERROR_VALUE;        /* SD_INFO2, SD_INFO1�̃G���[�r�b�g�m�F�p�ϐ� */


/*�|�[�g��ԕۑ�*/
static SDPortContext SDPort0Context;
static SDPortContext SDPort1Context;
//SDPortContext *SDPortCurrentContext = &SDPort0Context; /*TODO*/



extern volatile SDMC_ERR_CODE    SDCARD_ErrStatus;    /* �G���[�X�e�[�^�X */
extern volatile u32    SDCARD_Status;       /* �J�[�h�X�e�[�^�X */

static s16             SDCARD_SDFlag;           /* SD�J�[�h�t���O */

static volatile s16    SDCARD_OutFlag = 0;          /* �J�[�h�r�o��������t���O */
static SdmcResultInfo  *pSDCARD_info = NULL;           /* �ۑ��p���s���ʍ\���̃|�C���^ */


static SDMC_ERR_CODE i_sdmcSavePortContext( SDPortContext* buf_adr, u16 port_no);
static SDMC_ERR_CODE i_sdmcLoadPortContext( SDPortContext* buf_adr, u16* port_no);


/*---------------------------------------------------------------------------*
  Name:         sdmcSavePortContext

  Description:  �|�[�g0�̃��W�X�^��ϐ������[�U�o�b�t�@�ɑޔ�����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcSavePortContext( SDPortContext* buf_adr, u16 port_no)
{
    if( buf_adr == NULL) {
        return( SDMC_ERR_PARAM);
    }
    switch( port_no) {
      case 0:
        MI_CpuCopy8( &SDPort0Context, buf_adr, sizeof(SDPortContext));
        buf_adr->port_no = 0;
        break;
      case 1:
        MI_CpuCopy8( &SDPort1Context, buf_adr, sizeof(SDPortContext));
        buf_adr->port_no = 1;
        break;
      default: return( SDMC_ERR_PARAM);
    }
   return( SDMC_NORMAL);
}

/*---------------------------------------------------------------------------*
  Name:         sdmcLoadPortContext

  Description:  �|�[�g0�̃��W�X�^��ϐ������[�U�o�b�t�@���畜�A����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcLoadPortContext( SDPortContext* buf_adr, u16* port_no)
{
    if( buf_adr == NULL) {
        return( SDMC_ERR_PARAM);
    }
    switch( buf_adr->port_no) {
      case 0:
        MI_CpuCopy8( buf_adr, &SDPort0Context, sizeof(SDPortContext));
        *port_no = 0;
        break;
      case 1:
        MI_CpuCopy8( buf_adr, &SDPort1Context, sizeof(SDPortContext));
        *port_no = 1;
        break;
      default: return( SDMC_ERR_PARAM);
    }
   return( SDMC_NORMAL);
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Backup_port1

  Description:  backup registers and variables of port1.
                �|�[�g1�̃��W�X�^��ϐ����o�b�N�A�b�v����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Backup_port1(void)
{
    /* registers */
    SD_GetFPGA(SDPort1Context.SD_CLK_CTRL_VALUE,SD_CLK_CTRL);
    SD_GetFPGA(SDPort1Context.SD_OPTION_VALUE,  SD_OPTION);

    /* variables */
    SDPort1Context.SD_RCA       = SD_RCA;
    SDPort1Context.ErrStatus = SDCARD_ErrStatus;
    SDPort1Context.Status    = SDCARD_Status;
    SDPort1Context.MMCFlag   = SDCARD_MMCFlag;
    SDPort1Context.SDHCFlag  = SDCARD_SDHCFlag;
    SDPort1Context.SDFlag    = SDCARD_SDFlag;

    SDPort1Context.OutFlag   = SDCARD_OutFlag;

    /*media registers*/
    MI_CpuCopy8( SD_CID, SDPort1Context.SD_CID, 16);
    MI_CpuCopy8( SD_CSD, SDPort1Context.SD_CSD, 16);
    MI_CpuCopy8( SD_OCR, SDPort1Context.SD_OCR,  4);
    MI_CpuCopy8( SD_SCR, SDPort1Context.SD_SCR,  8);
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Restore_port1

  Description:  restore registers and variables of port0.
                �|�[�g1�̃��W�X�^��ϐ��𕜋A����B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Restore_port1(void)
{
    /* registers */
    SD_SetFPGA( SD_CLK_CTRL, SDPort1Context.SD_CLK_CTRL_VALUE);
    SD_SetFPGA( SD_OPTION,   SDPort1Context.SD_OPTION_VALUE);

    /* variables */
    SD_RCA           = SDPort1Context.SD_RCA;
    SDCARD_ErrStatus = SDPort1Context.ErrStatus;
    SDCARD_Status    = SDPort1Context.Status;
    SDCARD_MMCFlag   = SDPort1Context.MMCFlag;
    SDCARD_SDHCFlag  = SDPort1Context.SDHCFlag;
    SDCARD_SDFlag    = SDPort1Context.SDFlag;

    SDCARD_OutFlag = SDPort1Context.OutFlag;
    pSDCARD_info   = NULL;

    /*media registers*/
    MI_CpuCopy8( SDPort1Context.SD_CID, SD_CID, 16);
    MI_CpuCopy8( SDPort1Context.SD_CSD, SD_CSD, 16);
    MI_CpuCopy8( SDPort1Context.SD_OCR, SD_OCR,  4);
    MI_CpuCopy8( SDPort1Context.SD_SCR, SD_SCR,  8);

    /*recalc*/
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));
}


/*---------------------------------------------------------------------------*
  Name:         sdmcNandInit

  Description:  Initialize SD interface and SD card.
                ������

  Arguments:    dma_no : �g�p����DMA�ԍ�
                func1 : �J�[�h�}�����R�[���o�b�N�֐�
                func2 : �J�[�h�r�o���R�[���o�b�N�֐�

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcNandInit( void)
{
    /* SD������ */
    SDCARD_ErrStatus = sdmcReset();

    if(!SDCARD_ErrStatus) {
        SDCARD_ErrStatus = i_sdmcMPInit();
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcReset

  Description:  reset SD card.
                ���Z�b�g

  Arguments:

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE sdmcReset( void)
{
    OSIntrMode irq_core_flag;
    SDCARD_MMCFlag  = FALSE;           /* MMC�J�[�h����t���O�N���A */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag   = FALSE;           /* SD�J�[�h����t���O�N���A */

    /*** �J�[�h�X�e�[�^�X���N���A ***/
    SDCARD_ErrStatus = SDMC_NORMAL;
    SDCARD_Status    = SDMC_NORMAL;

    /*** �J�[�hCSD WP�r�b�g���N���A ***/
    SDCARD_WP_FLAG0 = 0;
    SDCARD_WP_FLAG1 = 0;
    SDCARD_WP_PERMANENT = 0;
    SDCARD_WP_TEMPORARY = 0;

    irq_core_flag = OS_DisableInterrupts();    /* �����݋֎~ */

        *SDIF_CNT_L = 0x0402;                  //���b�p�[���W�X�^
        *SDIF_CNT_L = 0x0000;                  //���b�p�[���W�X�^
        *SDIF_FDS_L = 0;
        *SDIF_FSC_L = 1;
        SD_Init();                                 /* SD Card I/F ���������� */
        SD_AndFPGA( SD_OPTION, SD_CD_DETECT_TIME); /* CD ���o�^�C�����[���N���A */

        SD_port_number = SDCARD_PORT1;             /*** ���݂̃|�[�g�ԍ����f�t�H���g�ɐݒ�      ***/

//        SDCARD_Backup_port1();                   /* port1 backup    */

    OS_RestoreInterrupts( irq_core_flag);      /* ���荞�ݐݒ�����ɖ߂� */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Layer_Init

  Description:  initialize sequence for SD card.
                SD�J�[�h�K��̏������菇

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARD_Layer_Init(void)
{
    u32     ulCSize;
    u16     read_block_len_val, mult_val;

//    u16 memory_exist, function_number;
    SDCARD_Status = SDMC_NORMAL;    /* �J�[�h�X�e�[�^�X���N���A */
    SDCARD_MMCFlag = FALSE;         /* MMC�J�[�h����t���O�N���A */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag = FALSE;          /* SD�J�[�h����t���O�N���A */

    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128)); /* SD�N���b�N�̎��g�� 261KHz(����������100�`400khz) */
    SD_EnableClock();                          /* SD�J�[�h�̃N���b�N���C�l�[�u���ɂ��� */

    /* SD I/F���_�~�[80�N���b�N(1mSec)�]���҂��i�^�C�}�[�ő҂����������Ă��ǂ��j */
    SVC_WaitByLoop( 17900); //179*4�T�C�N��=716�T�C�N��=10024ns=10us

    SDCARD_ErrStatus = SDMC_NORMAL;            /* �G���[�X�e�[�^�X���N���A */

    SDCARD_TimerStop();                        /* �^�C���A�E�g����p�^�C�}�X�g�b�v */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RESET_TIMEOUT);    /* �^�C���A�E�g����p�^�C�}�X�^�[�g */
#endif

    PRINTDEBUG( " CMD0(GO_IDLE_STATE)\n");
    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));   /* �^�C���A�E�g�ȊO�̃G���[���N���A */
    SD_Command(SD_CMD_CMD | GO_IDLE_STATE);     /* CMD0���s�A���X�|���X�m�F */
    if(SDCARD_ErrStatus){                       /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }

    /*------- idle state -------*/
    SVC_WaitByLoop( 17900); //179*4�T�C�N��=716�T�C�N��=10024ns=10us

    while(!(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT)){ /* �^�C���A�E�g�ɂȂ����甲���� */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));

        SD_RCA = 0;                             /* RCA = 0���Z�b�g */

        if(!SDCARD_MMCFlag){                    /* MMC�J�[�h�t���O�� 0(OFF) ��? */

            if( SD_AppCommand() == SDMC_NORMAL) {    /* CMD55 ���s���������킩? */

                SDCARD_MMCFlag = FALSE;         /* MMC�J�[�h�t���O�N���A */
                if(!SD_AppOpCond()){            /* ACMD41���s���������킩?(OCR31bit = L �̎� No Response) */

                    SDCARD_SDFlag = TRUE;       /* SD�J�[�h�t���O�Z�b�g */
                    break;
                }
            }else{                                        /* CMD55 ������I�����Ȃ� */
                if(SDCARD_ErrStatus == SDMC_ERR_TIMEOUT){ /* �^�C���A�E�g(==No Response)��? */
                    SDCARD_MMCFlag = TRUE;                /* MMC�J�[�h�t���O�Z�b�g */
                }else{
//                    break;                //�R�����g�A�E�g���Ȃ���SD�J�[�h�̏������Ɏ��s����
                }
            }
        }
        if(SDCARD_MMCFlag){                     /* MMC�J�[�h�t���O�� 1(ON) �̂Ƃ� */
            SD_RCA = 1;                         /* RCA = 1���Z�b�g */
            if(!SD_SendOpCond()){               /* CMD1���s���������킩? */
                break;
            }
        }
    }

    SDCARD_TimerStop();                               /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
    if(SDCARD_ErrStatus){                             /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j*/
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���`�F�b�N */
            SD_ClrErr(SDMC_ERR_FPGA_TIMEOUT);         /* �^�C���A�E�g�G���[�̐ݒ�N���A */
            SD_SetErr(SDMC_ERR_RESET);                /* �������J�[�h���Z�b�g�R�}���h��1.5�b�^�C���A�E�g�G���[�̐ݒ� */
        }
        SDCARD_MMCFlag = FALSE;                       /* MMC�J�[�h�t���O�N���A */
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_INITIAL_TIMEOUT); /* �^�C���A�E�g����p�^�C�}�X�^�[�g */
#endif

    SD_SendCID();                            /* CMD2���s ���X�|���X�m�F */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }

    while(1){
        /******************/
        SDCARD_TimerCheck();
        /******************/
        SD_SendRelativeAddr();               /* CMD3���s ���X�|���X�m�F ����I���� RCA<-Res��RCA */
        if(SDCARD_ErrStatus){                /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
            return SDCARD_ErrStatus;
        }
        if(SD_RCA != 0){
            break;
        }
    }

    /*------- standby state -------*/
    SD_SendCSD();                            /* CMD9���s ���X�|���X�m�F */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));

    /* �]�����x�ݒ� */
    SD_ClockDivSet(SD_RSP5);                 /* SD�J�[�h�̓���N���b�N�ݒ� (CSD[5]) */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }

    /* Command toggles acard between the Stand-by and Transfer states */
    SD_SelectCard();                         /* CMD7���s ���X�|���X�m�F */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }

    /*------- translate state -------*/

    SDCARD_SectorSize = SECTOR_SIZE;         /* �Z�N�^�T�C�Y �f�t�H���g 512bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* CMD16 �u���b�N�T�C�Y�̐ݒ� */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }

  SD_SCR[0] |= SCR_DAT_BUS_WIDTH_4BIT; //SCR�̕K�v�ȕ����̂�

    SD_EnableClock();                        /* SD-CLK Enable */

    if(SDCARD_MMCFlag){                      /* MMC�J�[�h ON ���`�F�b�N */
        if( ((SD_CSD[7] & 0x3C)>>2) >= 4) {
            MMCP_SetBusWidth( TRUE);
        }
//        SD_SelectBitWidth(FALSE);            /* CMD55->ACMD6 �r�b�g���̑I�� 1bit */
    }else{
        SD_SelectBitWidth(TRUE);             /* CMD55->ACMD6 �r�b�g���̑I�� 4bit */
    }

    SDCARD_TimerStop();                      /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcMPInit

  Description:  initialize SD card in multi ports.
                �}���`�|�[�g��SD�J�[�h������

  Arguments:

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcMPInit( void)
{
    u16 load_port_no;
    SDPortContext* SDNandContext;

    //�\��̈�̃|�[�g�R���e�L�X�g�Q��
    SDNandContext = (SDPortContext*)&(((OSFromBromBuf*)OSi_GetFromBromAddr())->SDNandContext);

    //NAND�X���b�g�̏�����
    SD_SetFPGA( SD_PORTSEL, SDMC_PORT_NAND); /* NAND�|�[�g�I�� */

//    OS_TPrintf( "SDNandContext : 0x%x\n", SDNandContext);
    //�������ς݂łȂ��Ƃ�����������
    if( sdmcCheckPortContext( SDNandContext) != SDMC_NORMAL) {
//        OS_TPrintf( "sdmcCheckPortContext : ERR!\n");
        SDCARD_ErrStatus = SDCARD_Layer_Init();

        SDCARD_Backup_port1();
        //�|�[�g�R���e�L�X�g�̕ۑ�
        if( i_sdmcSavePortContext( SDNandContext, 1) != SDMC_NORMAL) {
//            OS_TPrintf( "i_sdmcSavePortContext failed\n");
            return( SDMC_ERR_PARAM);
        }
    }else{ //�|�[�g�R���e�L�X�g�̕��A
//        OS_TPrintf( "sdmcCheckPortContext : NORMAL\n");

        /*SDCARD_Layer_Init()�̑���*/
        SDCARD_SectorSize = SECTOR_SIZE;         /* �Z�N�^�T�C�Y �f�t�H���g 512bytes */

        if( i_sdmcLoadPortContext( SDNandContext, &load_port_no) != SDMC_NORMAL) {
//            OS_TPrintf( "i_sdmcLoadPortContext failed\n");
            return( SDMC_ERR_PARAM);
        }
        SDCARD_Restore_port1(); //TODO:load_port_no�l����
    }

    SDCARD_OutFlag = FALSE;     /* �r�o�t���O�����Z�b�g */

    SDCARD_TimerStop();         /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
    SD_DisableClock();          /* SD-CLK Disable */
    SD_EnableInfo();            /* SD Card  �}�� ���荞�݋��� */

    return SDCARD_ErrStatus;
}

#if 0
/*---------------------------------------------------------------------------*
  Name:         sdmcGetStatus

  Description:  get card status
                �J�[�h�̏�Ԃ��擾����
                bit15 SD�J�[�h���ʃr�b�g(���o������1)
                bit14 MMC�J�[�h���ʃr�b�g(���o������1)
                bit10 IO3 card detect(���o������1)     ��CTR�ł̓v���A�b�v�̂��ߎg���Ȃ�
                bit9 IO3 card inserted(�}�������1) ��CTR�ł̓v���A�b�v�̂��ߎg���Ȃ�
                bit8 IO3 card removed(�E�����1)     ��CTR�ł̓v���A�b�v�̂��ߎg���Ȃ�
                bit7 write protect(�������݋֎~�̏ꍇ1)
                bit5 card detect(���o������1)
                bit4 card inserted(�}�������1)
                bit3 card removed(�E�����1)
                bit2 R/W access all end
                bit0 Response end

  Arguments:    *status : �J�[�h�̏�Ԃ��i�[����ϐ��ւ̃|�C���^

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------
SD_INFO1���W�X�^
bit[10,9,8,7] = DAT3CD, DAT3IN, DAT3OUT, WP
bit[5, 4, 3, 2] = CD, INS, REM, ALLEND
bit0 = RESEND
-------------------------------------------------
EXT_CD���W�X�^
bit[2,1,0] = P1CD, P1INS, P1REM
-------------------------------------------------
EXT_CD_DAT3���W�X�^
bit[2, 1, 0] = P1DCD, P1DINS, P1DREM
-------------------------------------------------
EXT_WP���W�X�^
bit0 = P1WP
-----------------------------------------------*/
SDMC_ERR_CODE sdmcGetStatus(u16 *status)
{
    u16 SD_INFO1_STATUS;

    SD_INFO1_STATUS = SD_INFO1;         /* SD_INFO1���W�X�^�ǂݏo�� */
    *status = SD_INFO1_STATUS;          /* �_�����] */

    /*--- �|�[�g0�̂Ƃ� ---*/
    if(SD_port_number == SDCARD_PORT0)
    {
        *status &= SDCARD_FLAG_CLR;     /* SD/MMC�t���O�N���A */
    }
    /*--- �|�[�g1�̂Ƃ� ---*/
    else if (SD_port_number == SDCARD_PORT1)
    {
        *status &= SDCARD_PORT1_CLR;    /* port1�Ɋ֌W�Ȃ��������N���A */
        SD_INFO1_STATUS = (u16)((EXT_CD & 0x0007) << 3);
        SD_INFO1_STATUS |= ((EXT_CD_DAT3 & 0x0007) << 8);
        SD_INFO1_STATUS |= ((EXT_WP & 0x0001) << 7);
        *status |= SD_INFO1_STATUS;     /* �J�[�hport1�t���O�ݒ�   */
    }
    /*--- SD/MMC�t���O���Z�b�g ---*/
    if( SDCARD_MMCFlag) {               /* ���o�����J�[�h��MMC�J�[�h�̎� */
        *status |= SDCARD_FLAG_MMC;     /* �J�[�h���蕔��MMC�J�[�h */
    }
    if( SDCARD_SDFlag) {                /* ���o�����J�[�h��SD�J�[�h�̎� */
        *status |= SDCARD_FLAG_SD;      /* �J�[�h���蕔��SD�J�[�h */
    }

    return SDMC_NORMAL;
}
#endif


/*******************************************************************************/
static int MMCP_SetBusWidth( BOOL b4bit)
{
    SD_EnableClock();                        /* SD-CLK Enable */

    /*�R�}���h6���s*/
    MMCP_WriteBusWidth( b4bit);
    SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IP�Ƀr�b�g���̐ݒ�(4bit��) */

    return( 0);
}
/*******************************************************************************/



/*---------------------------------------------------------------------------*
  Name:         sdmcGetErrStat

  Description:  �G���[�擾

  Arguments:

  Returns:      TRUE : success
                FALSE : error
 *---------------------------------------------------------------------------*/
static BOOL sdmcGetErrStat( void)
{
    PRINTDEBUG( "SD_INFO2:0x%x\n", SD_INFO2);

    /*--- �擾 ---*/
    SD_GetFPGA( SD_INFO2_VALUE,            SD_INFO2);
    /*------------------------------------------*/

    /*--- SD_INFO2�̃G���[�t���O�쐬 ---*/
    SD_INFO_ERROR_VALUE = (u16)(SD_INFO2_VALUE & 0x807F);//
    /*--- �G���[�X�e�[�^�X�쐬 (RESTIMEOUT��ILA�G���[�̃t���O�͔��f���Ȃ�) ---*/
    SDCARD_ErrStatus |= SD_INFO_ERROR_VALUE & (~(SD_INFO2_ERR_RESTIMEOUT)) &
                        (~(SD_INFO2_ERR_ILA)) & SD_INFO2_MASK_ERRSET;

    /*--- RESTIMEOUT��ILA�G���[�̓t���O�̈ʒu�����炵�Ĕ��f���� ---*/
    if( SD_INFO_ERROR_VALUE & SD_INFO2_ERR_ILA) {
        SDCARD_ErrStatus |= SDMC_ERR_ILA;                       /* �C���[�K���A�N�Z�X�G���[���� */
    }
    if( SD_INFO_ERROR_VALUE & SD_INFO2_ERR_RESTIMEOUT) {
        SDCARD_ErrStatus |= SDMC_ERR_TIMEOUT;                   /* Response Time out �G���[���� */
    }/*------------------------------------------------------------*/

    SD_AndFPGA( SD_INFO2,(~(SD_INFO2_ERROR_SET)));              /* SD_INFO2�̃G���[�t���O��S�ė��Ƃ� */

    if( (SD_INFO_ERROR_VALUE) != 0) {
        return( FALSE);
    }else{
        return( TRUE);
    }
}
