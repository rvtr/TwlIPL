/*
    Project:    CTR SD port driver
    File:       sdmc.c

    2006-2007, Research and Development Department, Nintendo.
*/

#include <firm.h>
#include <firm/sdmc.h>

/*#if (SD_DEBUG_PRINT_ON == 1)
    #if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
        #define PRINTDEBUG    osTPrintf
    #else
        #include <ctr/vlink.h>
        #define PRINTDEBUG    vlink_dos_printf
    #endif
#else
    #define PRINTDEBUG( ...) ((void)0)
#endif*/

//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)


/***********************************************************************
 �萔
***********************************************************************/
#define SD_STACK_SIZE        (4096*2)
#define SD_THREAD_PRIO       (10)
#define SD_INTR_THREAD_PRIO  (SD_THREAD_PRIO - 1)

#define SD_OPERATION_INIT               (0)
#define SD_OPERATION_READ               (1)
#define SD_OPERATION_READ_WITH_FIFO     (2)
#define SD_OPERATION_WRITE              (3)
#define SD_OPERATION_WRITE_WITH_FIFO    (4)

/*
TODO:
SD_INTR_THREAD_PRIO����������Sleep���Ȃ����Γ����B
Sleep���g���ɂ́AWakeup�̑O��Sleep�����邽��SD_INTR_THREAD_PRIO��Ⴍ
����K�v�����邪�ASleep�̑O��SD���荞�݂�����Ȃ������ꍇ�Ȃ���������
���荞�݂�����Ȃ��Ȃ蓮���Ȃ��Ȃ�BSleep�̑O�Ɋ��荞�݂�����Γ����B
*/


/***********************************************************************
 extern�ϐ�
***********************************************************************/
extern u16 BgBak[32*32];
//u16 sdcard_request_flag;            //�J�[�h����̃f�[�^�]���v���̗L���t���O
static BOOL    thread_flag;


/***********************************************************************
 global�ϐ�
***********************************************************************/
BOOL    sdmc_tsk_created = FALSE;
#if (TARGET_OS_CTR == 1)
ER_ID   sdmc_tsk_id;        //SD�^�X�NID
ER_ID   sdmc_dtq_id;
ER_ID   sdmc_result_dtq_id;
ER_ID   sdmc_alm_id;
ER_ID   sdmc_intr_tsk_id;
#else
OSThread       sdmc_tsk;
OSMessageQueue sdmc_dtq;
OSMessage      sdmc_dtq_array[1];
OSMessageQueue sdmc_result_dtq;
OSMessage      sdmc_result_dtq_array[1];
OSAlarm        sdmc_alm;
OSThread       sdmc_intr_tsk;
#endif

u16 sdmc_wakeup_count = 0; //OS_WakeupThread�̃L���[�C���O����
u16 sdmc_intr_wakeup_count = 0;

#if (TARGET_OS_CTR == 1)
#else
u64            sd_stack[SD_STACK_SIZE / sizeof(u64)];
u64            sd_intr_stack[SD_STACK_SIZE / sizeof(u64)];
#endif

/* drsdmc.c�ł��Q�� */
SdmcSpec    sdmc_current_spec;    //TODO:�|�[�g�؂�ւ����APort0��Port1�ɕۑ�����悤��

typedef struct {                  //OSMessage
    void*           buf;
    u32             bufsize;
    u32             offset;
    void            (*func)(void);
    SdmcResultInfo* info;
    u32             operation;
    void            (*func2)(void);
} SDCARDMsg;


/***********************************************************************
 static�֐��̐錾
***********************************************************************/
void i_sdmcCalcSize( void);
static void SDCARD_Backup_port0(void);
void SDCARD_Backup_port1(void);
static void SDCARD_Restore_port0(void);
void SDCARD_Restore_port1(void);

static SDMC_ERR_CODE SDCARDi_ReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
static SDMC_ERR_CODE SDCARDi_WriteFifo(void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info);

static SDMC_ERR_CODE SDCARDi_Read(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
static SDMC_ERR_CODE SDCARDi_Write(void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info);

int     MMCP_SetBusWidth( BOOL b4bit);       /* �r�b�g���̑I��(MMCplus, eMMC, moviNAND) */

static void SDCARD_Thread( void* arg);        //SD�X���b�h
static void SDCARD_Intr_Thread( void* arg);        //SD���荞�ݏ����X���b�h

SDMC_ERR_CODE sdmcGoIdle( void (*func1)(),void (*func2)());
static SDMC_ERR_CODE i_sdmcInit( void);
SDMC_ERR_CODE SDCARD_Layer_Init(void);
static SDMC_ERR_CODE i_sdmcMPInit( void);     /* �J�[�h�h���C�o������(�}���`�|�[�g�Ή�) */

static u16 i_sdmcErrProcess(void);            /* �G���[���̏��� */
static u16 i_sdmcGetResid(u32 *pResid);       /* �������݊����Z�N�^���̎擾 */
static u16 i_sdmcCheckWP(void);               /* SDCARD ���C�g�v���e�N�g�`�F�b�N */

static void i_sdmcEnable( void);              /* SD�g�p�̂��߂�OS���� */
static void i_sdmcDisable( void);

static u16 i_sdmcSendSCR(void);               /* SCR�̓ǂݏo�� */
static u16 SDCARD_SD_Status(void);            /* SD_STATUS�̎擾 */
static u32 SDCARD_GetR1Status(void);          /* Normal response command �J�[�h�X�e�[�^�X���擾 */

static void SDCARD_Dmy_Handler( void);        /* �������Ȃ� */
static void SDCARD_Timer_irq(void* arg);      /* �^�C���A�E�g���荞�݃n���h�� */
static void SDCARD_irq_Handler( void);        /* SD-IP����̊��荞�݃n���h�� */
static void SDCARD_FPGA_irq(void);            /* �J�[�h���[�h���C�g���荞�ݏ��� */
static void SDCARD_ATC0_irq(void);            /* ATC0�]���������荞�ݏ��� */
static void SYSFPGA_irq(void);                /* SYSFPGA�G���[���荞�ݏ��� */

/*�|�[�g1�͖����Œ�Ȃ̂Ń|�[�g�I���֐��͌��J���Ȃ�*/
static u16 i_sdmcSelectedNo(void);            /* �J�[�h�|�[�g�̑I�� */
static u16 i_sdmcSelect(u16 select);          /* ���݂̃J�[�h�|�[�g�ԍ��̃`�F�b�N */




/***********************************************************************
    �O���Q�ƕϐ�
***********************************************************************/
static void SDCARD_TimerStart(u32 tim);    /* �^�C���A�E�g�v���X�^�[�g */
void SDCARD_TimerStop(void);        /* �^�C���A�E�g�v����~ */

extern u16  SD_CID[8];                /* CID�l�ۑ��p */
extern u16  SD_CSD[8];                /* CSD�l�ۑ��p */
extern u16  SD_OCR[2];                /* OCR�l�ۑ��p */
extern u16  SD_SCR[4];                /* SCR�l�ۑ��p */
extern u16  SD_RCA;                   /* RCA�l�ۑ��p */

extern s16  SDCARD_MMCFlag;           /* MMC�J�[�h�t���O */
extern s16  SDCARD_SDHCFlag;          /* SDHC�J�[�h�t���O */
extern u16  SD_port_number;           /* ���݃|�[�g�ԍ�  */

//static BOOL init_io_exist;            /* SDIO���݃t���O */
//static BOOL init_mem_exist;           /* ���������݃t���O */


/***********************************************************************
    �O���[�o��
***********************************************************************/
u16  SD_SDSTATUS[32];            /* SD_STATUS���W�X�^�ۑ��p */
u16  SDCARD_WP_FLAG0;            /* �J�[�h���C�g�v���e�N�g�t���O�B0=�Ȃ��A1=�L��    */
u16  SDCARD_WP_FLAG1;            /* �J�[�h���C�g�v���e�N�g�t���O�B0=�Ȃ��A1=�L��    */
u16  SDCARD_WP_PERMANENT;        /* �J�[�h���C�g�v���e�N�g�i�v�t���O�B0=�Ȃ��A1=�L��    */
u16  SDCARD_WP_TEMPORARY;        /* �J�[�h���C�g�v���e�N�g�ꎞ�t���O�B0=�Ȃ��A1=�L��    */

u16* pSDCARD_BufferAddr;         /* �ۑ��p�f�[�^�i�[�o�b�t�@�A�h���X */

u32  ulSDCARD_SectorCount;       /* �]���Z�N�^�� */
u32  ulSDCARD_RestSectorCount;   /* �c��]���Z�N�^�� */
u32  SDCARD_SectorSize;          /* �Z�N�^�T�C�Y �f�t�H���g 512bytes */

u16  SD_INFO1_VALUE;             /* SD_INFO1���W�X�^�擾�p�ϐ� */
u16  SD_INFO1_MASK_VALUE;        /* SD_INFO1�����݃}�X�N�p�ϐ�(0�ŋ���, 1�ŋ֎~) */
u16  SD_INFO2_VALUE;             /* SD_INFO2���W�X�^�擾�p�ϐ� */
u16  SD_INFO2_MASK_VALUE;        /* SD_INFO2���荞�݃}�X�N�p�ϐ�(0�ŋ���, 1�ŋ֎~) */
u16  SD_INFO_ERROR_VALUE;        /* SD_INFO2, SD_INFO1�̃G���[�r�b�g�m�F�p�ϐ� */

u16  SD_port_en_numbers;         /* �T�|�[�g����|�[�g�� */



/*�|�[�g��ԕۑ�*/
SDPortContext SDPort0Context;
SDPortContext SDPort1Context;
//SDPortContext *SDPortCurrentContext = &SDPort0Context; /*TODO*/



u16    TransCount;                       /* R/W�]���J�E���g�ϐ� */

u32    ulSDCARD_Size;                    /* �J�[�h�S�Z�N�^�� */

volatile s16    SDCARD_ATC0_Flag;        /* �SATC�����t���O */
volatile s16    SDCARD_FPGA_Flag;        /* FPGA���������t���O */
volatile s16    SDCARD_EndFlag;          /* �]�����������t���O */

SDMC_ERR_CODE   SDCARD_ErrStatus;        /* �G���[�X�e�[�^�X */
vu32            SDCARD_Status;           /* �J�[�h�X�e�[�^�X */
s16             SDCARD_SDFlag;           /* SD�J�[�h�t���O */

volatile s16    SDCARD_OutFlag;          /* �J�[�h�r�o��������t���O */
SdmcResultInfo  *pSDCARD_info;           /* �ۑ��p���s���ʍ\���̃|�C���^ */

u16             SDCARD_IO_Port;          /* �J�[�h�}��/�r�o���荞�ݔ������̃|�[�g�ԍ� */

void (*func_SDCARD_In)(void);            /* �J�[�h�}���C�x���g�p�R�[���o�b�N�ۑ��p */
void (*func_SDCARD_Out)(void);           /* �J�[�h�r�o�C�x���g�p�R�[���o�b�N�ۑ��p */
/* void (*func_SDCARD_CallBack)(SdmcResultInfo *info);     �������ʒʒm�p�R�[���o�b�N�ۑ��p */



extern SDMC_ERR_CODE i_sdmcMPInitFirm( void);


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Backup_port0

  Description:  backup registers and variables of port0.
                �|�[�g0�̃��W�X�^��ϐ����o�b�N�A�b�v����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Backup_port0(void)
{
    /* registers */
    SD_GetFPGA(SDPort0Context.SD_CLK_CTRL_VALUE,SD_CLK_CTRL);
    SD_GetFPGA(SDPort0Context.SD_OPTION_VALUE,  SD_OPTION);

    /* variables */
    SDPort0Context.SD_RCA    = SD_RCA;
    SDPort0Context.ErrStatus = SDCARD_ErrStatus;
    SDPort0Context.Status    = SDCARD_Status;
    SDPort0Context.MMCFlag   = SDCARD_MMCFlag;
    SDPort0Context.SDHCFlag  = SDCARD_SDHCFlag;
    SDPort0Context.SDFlag    = SDCARD_SDFlag;

    SDPort0Context.OutFlag   = SDCARD_OutFlag;

    /*media registers*/
    MI_CpuCopy8( SD_CID, SDPort0Context.SD_CID, 16);
    MI_CpuCopy8( SD_CSD, SDPort0Context.SD_CSD, 16);
    MI_CpuCopy8( SD_OCR, SDPort0Context.SD_OCR,  4);
    MI_CpuCopy8( SD_SCR, SDPort0Context.SD_SCR,  8);
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Backup_port1

  Description:  backup registers and variables of port1.
                �|�[�g1�̃��W�X�^��ϐ����o�b�N�A�b�v����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SDCARD_Backup_port1(void)
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
  Name:         i_sdmcEnable

  Description:  assign OS resouce for using SD memory card.
                SD�J�[�h���g�����߂̏���������

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void i_sdmcEnable( void)
{
#if (TARGET_OS_CTR == 1)
    /*SD interrupt setting*/
//    osInitIntrFlag();
//    osClearInterruptPendingID( OS_INTR_ID_SD);
//    *(vu32*)CTR_INT_IF = CTR_IE_SD_MASK;
    osSetInterruptHandler( OS_INTR_ID_SD, SDCARD_irq_Handler);
    osEnableInterruptID( OS_INTR_ID_SD);
//    *(vu32*)CTR_INT_SE = CTR_IE_SD_MASK;    //���荞��(IRQ)��������
//    *(vu32*)CTR_INT_IE = CTR_IE_SD_MASK;
//    osEnableInterrupts();
//    *(vu16*)0x04000208 = 1;
#else
    /*SD���荞�݂�IF����*/
//    *SDIF_CNT_L = (SDIF_CNT_FCLR | SDIF_CNT_USEFIFO); //���b�p�[���W�X�^(sdmcReset�ĂԂȂ炢��Ȃ�)
//    SD_AndFPGA( SD_OPTION, SD_CD_DETECT_TIME); /* CD ���o�^�C�����[���N���A(sdmcReset�ĂԂȂ炢��Ȃ�) */

    *(vu16*)CTR_INT_IF = CTR_IE_SD_MASK;
    OS_SetIrqFunction( OS_IE_SD1, SDCARD_irq_Handler);
    OS_EnableIrqMask( OS_IE_SD1);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcDisable

  Description:  under construction
                �H����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void i_sdmcDisable( void)
{
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_irq_Handler

  Description:  SD interrupt handler
                SD���荞�݃n���h��

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SDCARD_irq_Handler( void)
{
#if (TARGET_OS_CTR == 1)
    iwup_tsk( sdmc_intr_tsk_id);
#else
    PRINTDEBUG( "SD irq!\n");
//  OS_DumpThreadList();
    OS_SetIrqCheckFlag( OS_IE_SD1);
//  sdmc_intr_wakeup_count++;
//    OS_WakeupThreadDirect( &sdmc_intr_tsk);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Dmy_Handler

  Description:  dmy handler for timer interrupt.
                �^�C�}�[���荞�ݗp�_�~�[�n���h��

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Dmy_Handler( void)
{
}

/*---------------------------------------------------------------------------*
  Name:         sdmcInit

  Description:  Initialize SD interface and SD card.
                ������

  Arguments:    dma_no : DMA�ԍ�
                func1 : �J�[�h�}�����R�[���o�b�N�֐�
                func2 : �J�[�h�r�o���R�[���o�b�N�֐�

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(),void (*func2)())
{
#if (TARGET_OS_CTR == 1)
    T_CALM    calm;
    T_CTSK    ctsk;
    T_CDTQ    cdtq;
#endif
//    SDCARDMsg SdMsg;
//    u32            init_msg;
    SDMC_ERR_CODE    api_result;

    if( sdmc_tsk_created == FALSE) {
        /*---------- OS���� ----------*/
        /* �A���[���n���h���o�^ */
#if (TARGET_OS_CTR == 1)
        calm.almatr     = TA_HLNG;          // set attribution : for high level language
        calm.exinf      = 0;                // set argument for alarm handler
        calm.almhdr     = SDCARD_Timer_irq; // set alarm handler
        sdmc_alm_id = acre_alm(&calm);
        if (sdmc_alm_id < 0)
        {
            PRINTDEBUG("create_alarm_simple: Cannot create new alarm handler (%d).\n", sdmc_alm_id);
        }

        /* ���b�Z�[�W������ */
        // setup dataqueue structure
        cdtq.dtqatr     = TA_TFIFO;         // set attribution : normal FIFO
        cdtq.dtqcnt     = 1;                // there are 2 datas in queue
        cdtq.dtq        = NULL;             // set data queue address : NULL means automatically allocated by kernel
        sdmc_dtq_id = acre_dtq(&cdtq);
        if (sdmc_dtq_id < 0)
        {
            PRINTDEBUG("create_dataqueue_simple: Cannot create new data queue.\n");
        }
        /**/
        cdtq.dtqatr     = TA_TFIFO;         // set attribution : normal FIFO
        cdtq.dtqcnt     = 1;                // there are 2 datas in queue
        cdtq.dtq        = NULL;             // set data queue address : NULL means automatically allocated by kernel
        sdmc_result_dtq_id = acre_dtq(&cdtq);
        if (sdmc_result_dtq_id < 0)
        {
            PRINTDEBUG("create_dataqueue_simple: Cannot create new data queue.\n");
        }

//        OS_InitThread();    //�����̗D��x��16�ɂȂ�
//        chg_pri( (ID)0, (PRI)12);

        /* SD�^�X�N�̗����グ */
        ctsk.tskatr     = TA_HLNG | TA_ACT; // set attribution : for high level language and running now
        ctsk.task       = SDCARD_Thread;    // set task routine
        ctsk.exinf      = (void*)0;         // set argument for task routine
        ctsk.itskpri    = SD_THREAD_PRIO;   // set priority
        ctsk.stksz      = SD_STACK_SIZE;    // set stack size
        ctsk.stk        = NULL;             // set stack address : NULL means automatically allocated by kernel
        sdmc_tsk_id = acre_tsk(&ctsk);
        if (sdmc_tsk_id < 0)
        {
            PRINTDEBUG("create_task_sd: Cannot create new task.\n");
        }else{
            if( (sdmc_tsk_id == E_NOID)||(sdmc_tsk_id == E_NOMEM)||(sdmc_tsk_id == E_RSATR)||
                (sdmc_tsk_id == E_PAR)||(sdmc_tsk_id == E_OBJ)) {
                PRINTDEBUG("create_task_sd: Cannot create new task.\n");
            }
            PRINTDEBUG("create_task_sd: 0x%x\n", sdmc_tsk_id);
        }
        /*----------------------------*/

        /* SD���荞�ݏ����^�X�N�̗����グ */
        ctsk.tskatr     = TA_HLNG | TA_ACT;     // set attribution : for high level language and running now
        ctsk.task       = SDCARD_Intr_Thread;   // set task routine
        ctsk.exinf      = (void*)0;             // set argument for task routine
        ctsk.itskpri    = SD_INTR_THREAD_PRIO;  // set priority
        ctsk.stksz      = SD_STACK_SIZE;        // set stack size
        ctsk.stk        = NULL;                 // set stack address : NULL means automatically allocated by kernel
        sdmc_intr_tsk_id = acre_tsk(&ctsk);
        if (sdmc_intr_tsk_id < 0)
        {
            PRINTDEBUG("create_intr_task_sd: Cannot create new task.\n");
        }else{
            if( (sdmc_intr_tsk_id == E_NOID)||(sdmc_intr_tsk_id == E_NOMEM)||(sdmc_intr_tsk_id == E_RSATR)||
                (sdmc_intr_tsk_id == E_PAR)||(sdmc_intr_tsk_id == E_OBJ)) {
                PRINTDEBUG("create_intr_task_sd: Cannot create new task.\n");
            }
            PRINTDEBUG("create_intr_task_sd: 0x%x\n", sdmc_intr_tsk_id);
        }

#else //(TARGET_OS_NITRO = 1)
        /*---------- OS���� ----------*/
        if( !OS_IsAlarmAvailable()) {   /* �A���[���`�F�b�N(OS_InitAlarm�ς݂�?) */
            SDCARD_ErrStatus |= SDMC_ERR_END;
        }else{
            OS_CreateAlarm( &sdmc_alm); //�g�p�\�ł���Ώ�����
        }

        /* ���b�Z�[�W������ */
        OS_InitMessageQueue( &sdmc_dtq,        &sdmc_dtq_array[0],        1);
        OS_InitMessageQueue( &sdmc_result_dtq, &sdmc_result_dtq_array[0], 1);

//        OS_InitThread();  //�����̗D��x��16�ɂȂ�

        /* SD�X���b�h�̗����グ */
        OS_CreateThread( &sdmc_tsk, SDCARD_Thread, NULL,
                         (sd_stack+SD_STACK_SIZE / sizeof(u64)), SD_STACK_SIZE, SD_THREAD_PRIO);
        OS_WakeupThreadDirect( &sdmc_tsk);
        PRINTDEBUG( "sdmc_tsk:0x%x\n", &sdmc_tsk);


        /* SD���荞�ݏ����X���b�h�̗����グ */
        (void)OS_ClearIrqCheckFlag( OS_IE_SD1);
        OS_CreateThread( &sdmc_intr_tsk, SDCARD_Intr_Thread, NULL,
                         (sd_intr_stack+SD_STACK_SIZE / sizeof(u64)), SD_STACK_SIZE, SD_INTR_THREAD_PRIO);
        OS_WakeupThreadDirect( &sdmc_intr_tsk);
        PRINTDEBUG( "sdmc_intr_tsk:0x%x\n", &sdmc_intr_tsk);
        /*----------------------------*/
#endif
        /**/
        sdmc_tsk_created = TRUE;
    }

    /**/
    func_SDCARD_In = func1;         /* �J�[�h�}���C�x���g�p�֐��̃A�h���X��ݒ� */
    func_SDCARD_Out = func2;        /* �J�[�h�r�o�C�x���g�p�֐��̃A�h���X��ݒ� */
    api_result = sdmcGoIdle( func1, func2);
//    api_result = SDMC_NORMAL;

    return api_result;
}

/* �J�[�h�����ꊷ������Ƃ��Ȃǂɏ������iRTFS�p�j*/
SDMC_ERR_CODE sdmcGoIdle( void (*func1)(),void (*func2)())
{
    SDCARDMsg        SdMsg;
#if (TARGET_OS_CTR == 1)
    u32              init_msg;
#else
    OSMessage        init_msg;
#endif
    SDMC_ERR_CODE    api_result;

    func_SDCARD_In  = func1;        /* �J�[�h�}���C�x���g�p�֐��̃A�h���X��ݒ� */
    func_SDCARD_Out = func2;        /* �J�[�h�r�o�C�x���g�p�֐��̃A�h���X��ݒ� */

    /*----- SD�X���b�h�ƒʐM -----*/
#if (TARGET_OS_CTR == 1)
    SdMsg.operation =    SD_OPERATION_INIT;

    snd_dtq( sdmc_dtq_id, (VP_INT)&SdMsg);

    /* �Ԃ�l�҂� */
    rcv_dtq( sdmc_result_dtq_id, (VP_INT*)&init_msg);
    api_result = (SDMC_ERR_CODE)init_msg;
#else
    SdMsg.operation = SD_OPERATION_INIT;
//    SdMsg.func  = func1;
//    SdMsg.func2 = func2;

    init_msg = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, init_msg, OS_MESSAGE_BLOCK);

    /* �Ԃ�l�҂� */
    OS_ReceiveMessage( &sdmc_result_dtq, &init_msg, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)init_msg;
#endif
    /*----------------------------------*/

    return api_result;
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcInit

  Description:  Initialize SD interface and SD card.
                ������

  Arguments:

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcInit( void)
{
    i_sdmcEnable();

    /* SD������ */
    SDCARD_ErrStatus = sdmcReset();

    if(!SDCARD_ErrStatus) {
        SDCARD_ErrStatus = i_sdmcMPInitFirm();
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
SDMC_ERR_CODE sdmcReset( void)
{
    OSIntrMode irq_core_flag;
    ulSDCARD_Size   = 0;               /* �J�[�h�S�Z�N�^���N���A */
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

    pSDCARD_info   = NULL;
    SDCARD_OutFlag = FALSE;            /* �J�[�h�r�o��������t���O�N���A */

#if (TARGET_OS_CTR == 1)
    irq_core_flag = osDisableInterrupts();     /* �����݋֎~ */
#else
    irq_core_flag = OS_DisableInterrupts();    /* �����݋֎~ */
#endif
        *SDIF_CNT_L = (SDIF_CNT_FCLR | SDIF_CNT_USEFIFO); //���b�p�[���W�X�^
        *SDIF_CNT_L = 0x0000;                  //���b�p�[���W�X�^
        *SDIF_FDS_L = 0;
        *SDIF_FSC_L = 1;
        SD_Init();                                 /* SD Card I/F ���������� */
        SD_AndFPGA( SD_OPTION, SD_CD_DETECT_TIME); /* CD ���o�^�C�����[���N���A */

        SD_port_en_numbers = SDCARD_PORT_NO_MAX; /*** �T�|�[�g����|�[�g�����f�t�H���g�ɐݒ� ***/
        SD_port_number = 0;                      /*** ���݂̃|�[�g�ԍ����f�t�H���g�ɐݒ�      ***/

        SDCARD_Backup_port0();                   /* port0 backup    */
        SDCARD_Backup_port1();                   /* port1 backup    */

#if (TARGET_OS_CTR == 1)
    osRestoreInterrupts( irq_core_flag);       /* ���荞�ݐݒ�����ɖ߂� */
#else
    OS_RestoreInterrupts( irq_core_flag);      /* ���荞�ݐݒ�����ɖ߂� */
#endif

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Layer_Init

  Description:  initialize sequence for SD card.
                SD�J�[�h�K��̏������菇

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE SDCARD_Layer_Init(void)
{
//    SYSTIM  wait_tim, limit_tim;

//    u16 memory_exist, function_number;
    SDCARD_Status = SDMC_NORMAL;    /* �J�[�h�X�e�[�^�X���N���A */
/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    SDCARD_EndFlag = TRUE;          /* �]�����������t���O�Z�b�g */
    SDCARD_MMCFlag = FALSE;         /* MMC�J�[�h����t���O�N���A */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag = FALSE;          /* SD�J�[�h����t���O�N���A */
    SDCARD_OutFlag = FALSE;         /* �J�[�h�r�o��������t���O�N���A */
    ulSDCARD_Size = 0;              /* �J�[�h�S�Z�N�^���N���A */
    TransCount = 0;                 /* �]���J�E���g�ϐ��N���A */
//    init_io_exist = 0;
//    init_mem_exist = 0;

    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128)); /* SD�N���b�N�̎��g�� 261KHz(����������100�`400khz) */
    SD_EnableClock();                          /* SD�J�[�h�̃N���b�N���C�l�[�u���ɂ��� */

    /* SD I/F���_�~�[80�N���b�N(1mSec)�]���҂��i�^�C�}�[�ő҂����������Ă��ǂ��j */
#if (TARGET_OS_CTR == 1)
    dly_tsk( 1);
#else
//    OS_Sleep( 1);
    SVC_WaitByLoop( 17900); //179*4�T�C�N��=716�T�C�N��=10024ns=10us
#endif

    SDCARD_ErrStatus = SDMC_NORMAL;            /* �G���[�X�e�[�^�X���N���A */

    SDCARD_TimerStop();                        /* �^�C���A�E�g����p�^�C�}�X�g�b�v */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RESET_TIMEOUT);   /* �^�C���A�E�g����p�^�C�}�X�^�[�g */
#endif

    PRINTDEBUG( " CMD0(GO_IDLE_STATE)\n");
    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));   /* �^�C���A�E�g�ȊO�̃G���[���N���A */
    {
/*PRINTDEBUG( "SD_INFO1      : 0x%x\n", SD_INFO1);
PRINTDEBUG( "SD_INFO2      : 0x%x\n", SD_INFO2);
PRINTDEBUG( "SD_INFO1_MASK : 0x%x\n", SD_INFO1_MASK);
PRINTDEBUG( "SD_INFO2_MASK : 0x%x\n", SD_INFO2_MASK);
PRINTDEBUG( "SD_CLK_CTRL   : 0x%x\n", SD_CLK_CTRL);
PRINTDEBUG( "SD_SIZE       : 0x%x\n", SD_SIZE);

PRINTDEBUG( "SD_INFO1_MASK : 0x%x\n", (*(vu32 *)(SD_IP_BASE + 0x20)));*/
    }
    SD_Command(SD_CMD_CMD | GO_IDLE_STATE);     /* CMD0���s�A���X�|���X�m�F */
    if(SDCARD_ErrStatus){                       /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
        return SDCARD_ErrStatus;
    }

    /*------- idle state -------*/
#if (TARGET_OS_CTR == 1)
    dly_tsk( 1);            /* 1ms�҂� */
#else
//    OS_Sleep( 1);
    SVC_WaitByLoop( 17900); //179*4�T�C�N��=716�T�C�N��=10024ns=10us
#endif
    SD_SendIfCond();                            /* CMD8���s�A���X�|���X�m�F */
    if( !SDCARD_SDHCFlag) {                     /* SDHC�ȊO�͎��s���Ă�͂��Ȃ̂� */
        SDCARD_ErrStatus = SDMC_NORMAL;         /* �G���[�t���O���N���A���Ă��� */
    } //��:CMD8�ɂ�肱���Ŋ��荞�݂�����I

    while(!(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT)){ /* �^�C���A�E�g�ɂȂ����甲���� */
        SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));

        SD_RCA = 0;                             /* RCA = 0���Z�b�g */
        if(!SDCARD_MMCFlag){                    /* MMC�J�[�h�t���O�� 0(OFF) ��? */
            if(!SD_AppCommand()){               /* CMD55 ���s���������킩? */
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
/*
    if( SDCARD_SDHCFlag) {
        SD_ReadOCR();
    }
*/
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

#if SCR_ON
    SD_SelectBitWidth(FALSE);                /* CMD55->ACMD6 �r�b�g���̑I�� 1bit */

    /* ACMD51 ���s SD configuration register (SCR) */
    if(SDCARD_SDFlag){                       /* SD�J�[�h�t���O ON ���`�F�b�N */
        SDCARD_TimerStop();                  /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
        i_sdmcSendSCR();
        if(SDCARD_ErrStatus){                /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j */
            return SDCARD_ErrStatus;
        }
#if TIMEOUT
        SDCARD_TimerStart(SDCARD_CLOCK_WAIT);/* �^�C���A�E�g����p�^�C�}�X�^�[�g */
#endif
    }
#endif
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

    if(SDCARD_SDFlag){                       /* SD�J�[�h�t���O ON ���`�F�b�N */
        if (SDCARD_SD_Status())              /* CMD55->ACMD13 �J�[�h�X�e�[�^�X���擾 */
            return SDCARD_ErrStatus;
        if(SD_SDSTATUS[1] & SD_MEMORY_CARD){
            SDCARD_SDFlag = FALSE;           /* SD�J�[�h�t���O�N���A */
        }
    }

    i_sdmcCalcSize();

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcCalcSize

  Description:

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
void i_sdmcCalcSize( void)
{
    u32     ulCSize;
    u16     read_block_len_val, mult_val;

    /*--------------�J�[�h�T�C�Y�̎Z�o---------------*/
    if( ((SD_CSD[7] & CSD_STRUCT_BIT_127_126) >> 6) == 0x1) {    //SDHC�̂Ƃ�
        sdmc_current_spec.csd_ver2_flag = 1;
        ulCSize = (u32)((((u32)(SD_CSD[3]) & CSD_C_SIZE_BIT_69_56) << 8) +
                    ((SD_CSD[2] & CSD_C_SIZE_BIT_55_48) >> 8) + 1);
        ulCSize = ulCSize * 1024;    //���Ƃ���512KByte�P�ʂȂ̂�512Byte�P�ʂɂ���
        /* �f�[�^�̈�T�C�Y�Z�o */
        sdmc_current_spec.memory_capacity = ulCSize;
        ulSDCARD_Size = ulCSize;
        /* �v���e�N�g�̈�T�C�Y�Z�o */
        sdmc_current_spec.protected_capacity = (((SD_SwapByte( &SD_SDSTATUS[2])) << 16) +
                                            (SD_SwapByte( &SD_SDSTATUS[3]))) / 0x200;
        /*�g�[�^���T�C�Y�Z�o */
        sdmc_current_spec.card_capacity = sdmc_current_spec.memory_capacity +
                                            sdmc_current_spec.protected_capacity;

    }else{                                                      //�]��SD�J�[�h�̂Ƃ�
        sdmc_current_spec.csd_ver2_flag = 0;
        ulCSize = (u32)(((SD_CSD[3] & CSD_C_SIZE_BIT_71_62) >> 6) +
                    ((SD_CSD[4] & CSD_C_SIZE_BIT_73_72) << 10) + 1);
        mult_val = ((SD_CSD[2] & CSD_C_SIZE_MULT) >> 7) + 2;    //2�̏搔
        ulCSize = ulCSize << mult_val;
        if(SDCARD_MMCFlag){                        /* MMC�J�[�h�t���OON ���`�F�b�N */
            read_block_len_val = ((SD_CSD[4] & CSD_READ_BL_LEN) >> 8);
            ulCSize = (ulCSize << read_block_len_val);
        }else{                                    /* SD�J�[�h�t���O(SDCARD_SDFlag)ON �̂͂� */
            read_block_len_val = (((SD_CSD[1] & CSD_WRITE_BL_LEN_BIT_25_24) << 2) |
                        ((SD_CSD[0] & CSD_WRITE_BL_LEN_BIT_23_22) >> 14));
            ulCSize = (ulCSize << read_block_len_val);
        }
        /* �f�[�^�̈�T�C�Y�Z�o */
        ulCSize /= SDCARD_SectorSize;            /* �S�Z�N�^���̎Z�o */
        sdmc_current_spec.memory_capacity = ulCSize;
        ulSDCARD_Size += ulCSize;                /* �S�Z�N�^���̃Z�b�g */
        /* �v���e�N�g�̈�T�C�Y�Z�o */
        sdmc_current_spec.protected_capacity = ((SD_SwapByte( &SD_SDSTATUS[2])) << 16) +
                                            (SD_SwapByte( &SD_SDSTATUS[3]));
        sdmc_current_spec.protected_capacity <<= mult_val;
        sdmc_current_spec.protected_capacity <<= read_block_len_val;
        sdmc_current_spec.protected_capacity /= SDCARD_SectorSize;    //TODO:�\���̂ɂ܂Ƃ߂邱��
        /*�g�[�^���T�C�Y�Z�o */
        sdmc_current_spec.card_capacity = sdmc_current_spec.memory_capacity +
                                            sdmc_current_spec.protected_capacity;
    }
    sdmc_current_spec.SS = SDCARD_SectorSize;

    PRINTDEBUG( "SD memory capacity : 0x%x\n", sdmc_current_spec.memory_capacity);
    PRINTDEBUG( "SD protected capacity : 0x%x\n", sdmc_current_spec.protected_capacity);
    PRINTDEBUG( "SD total capacity : 0x%x\n", sdmc_current_spec.card_capacity);
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
    if(((SD_port_number == SDCARD_PORT0) && (!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_DETECT))) ||
        ((SD_port_number == SDCARD_PORT1) && (!SD_CheckFPGAReg(EXT_CD,EXT_CD_PORT1_DETECT))) ||
          (SD_port_number > SDCARD_PORT1))
    {
        SDCARD_ErrStatus = SDMC_NORMAL;         /* �G���[�X�e�[�^�X���N���A */
        SDCARD_OutFlag = TRUE;                  /* �r�o�t���O���Z�b�g */
    }else{
        if( SD_CheckFPGAReg( SD_INFO1, SD_INFO1_DETECT)) {
            sdmcSelect( (u16)SDMC_PORT_CARD);
            SDCARD_ErrStatus = SDCARD_Layer_Init();
        }
        if( SD_CheckFPGAReg( EXT_CD, EXT_CD_PORT1_DETECT)) {
            sdmcSelect( (u16)SDMC_PORT_NAND);
            SDCARD_ErrStatus = SDCARD_Layer_Init();
        }
        SDCARD_OutFlag = FALSE;                 /* �r�o�t���O�����Z�b�g */
    }

    SDCARD_TimerStop();         /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
    SD_DisableClock();          /* SD-CLK Disable */
    SD_EnableInfo();            /* SD Card  �}�� ���荞�݋��� */

    return SDCARD_ErrStatus;
}


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


/*---------------------------------------------------------------------------*
  Name:         SDCARD_GetR1Status

  Description:  get the card status of R1 response.
                R1���X�|���X�̃J�[�h�X�e�[�^�X���擾����B

  Arguments:    None

  Returns:      SDCARD_Status : R1��[39:8]
 *---------------------------------------------------------------------------*/
static u32 SDCARD_GetR1Status(void)
{
    /* SD_CheckStatus��SDCARD_Status�ɒl������ */
    return SDCARD_Status;
}


/*---------------------------------------------------------------------------*
  Name:         sdmcReadFifo

  Description:  read from card.
                ���b�p�[��FIFO���g�p���ăJ�[�h����̓ǂݏo���B

  Arguments:    buf : �ǂݏo�����f�[�^���i�[���邽�߂̃o�b�t�@�̃A�h���X
                bufsize : �ǂݏo���T�C�Y�i�Z�N�^���j
                offset : �ǂݏo���J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
#if (TARGET_OS_CTR == 1)
    u32           recv_dat;
#else
    OSMessage     recv_dat;
#endif
    SDMC_ERR_CODE api_result;                    //SDCARD�֐��̕Ԃ�l

    SdMsg.buf       = buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_READ_WITH_FIFO;

#if (TARGET_OS_CTR == 1)
    PRINTDEBUG( "readfifo : snd_dtq begin\n");
    snd_dtq( sdmc_dtq_id, (VP_INT)&SdMsg);

    /* �Ԃ�l�҂� */
    PRINTDEBUG( "readfifo : rcv_dtq begin\n");
    rcv_dtq( sdmc_result_dtq_id, (VP_INT*)&recv_dat);

    api_result = (SDMC_ERR_CODE)recv_dat;
#else
    recv_dat = (OSMessage)&SdMsg; //SdMsg�̃A�h���X��`����
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);

    /* �Ԃ�l�҂� */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;
#endif

    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_ReadFifo

  Description:  read from card.
                ���b�p�[��FIFO���g�p���ăJ�[�h����̓ǂݏo���B

  Arguments:    buf : �ǂݏo�����f�[�^���i�[���邽�߂̃o�b�t�@�̃A�h���X
                bufsize : �ǂݏo���T�C�Y�i�Z�N�^���j
                offset : �ǂݏo���J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_ReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    SDMC_ERR_CODE result;

    /* FIFO Empty���荞�ݖ����AFIFO Full���荞�ݗL�� */
    *(SDIF_CNT) = (*(SDIF_CNT) & (~SDIF_CNT_FEIE)) | SDIF_CNT_FFIE;
    *(SDIF_FDS) = (u16)SDCARD_SectorSize;   /* FIFO�̃f�[�^�T�C�Y */
    *(SDIF_FSC) = bufsize;
    *(SDIF_CNT) |= (SDIF_CNT_USEFIFO | SDIF_CNT_FCLR);        /* FIFO�g�p�t���OON */
    CC_EXT_MODE = CC_EXT_MODE_DMA;          /* DMA���[�hON */

    result = SDCARDi_Read( buf, bufsize, offset, func, info);

    /* FIFO������ */
    *(SDIF_CNT) &= (~SDIF_CNT_USEFIFO);     /* FIFO�g�p�t���OOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;          /* PIO���[�h(DMA���[�hOFF) */

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRead

  Description:  read from card.
                �J�[�h����̓ǂݏo���B

  Arguments:    buf : �ǂݏo�����f�[�^���i�[���邽�߂̃o�b�t�@�̃A�h���X
                bufsize : �ǂݏo���T�C�Y�i�Z�N�^���j
                offset : �ǂݏo���J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
#if (TARGET_OS_CTR == 1)
    u32           recv_dat;
#else
    OSMessage     recv_dat;
#endif
    SDMC_ERR_CODE api_result;

    SdMsg.buf       = buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_READ;

#if (TARGET_OS_CTR == 1)
    snd_dtq( sdmc_dtq_id, (VP_INT)&SdMsg);

    /* �Ԃ�l�҂� */
    rcv_dtq( sdmc_result_dtq_id, (VP_INT*)&recv_dat);

    api_result = (SDMC_ERR_CODE)recv_dat;
#else
    recv_dat = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);

    /* �Ԃ�l�҂� */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;
#endif

    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_Read

  Description:  read from card.
                �J�[�h����̓ǂݏo���B

  Arguments:    buf : �ǂݏo�����f�[�^���i�[���邽�߂̃o�b�t�@�̃A�h���X
                bufsize : �ǂݏo���T�C�Y�i�Z�N�^���j
                offset : �ǂݏo���J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_Read(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    s16           nRetryCount;                  /* ���g���C�񐔃J�E���g */
    SDMC_ERR_CODE SaveErrStatus;                /* �G���[�X�e�[�^�X�ۑ��p */
    u32           SaveStatus;                   /* �J�[�h�X�e�[�^�X�ۑ��p */

    for( nRetryCount=0; nRetryCount<SDCARD_RETRY_COUNT; nRetryCount++) {

        SD_EnableClock();                       /* �N���b�N���� */

    /*    func_SDCARD_CallBack = func;    */
        pSDCARD_info = info;                    /* �ۑ��p���s���ʍ\���̂̃A�h���X��ݒ� */
        ulSDCARD_SectorCount = bufsize;         /* �]���Z�N�^���̐ݒ� */
        ulSDCARD_RestSectorCount = bufsize;     /* �c��]���Z�N�^���̐ݒ� */
        pSDCARD_BufferAddr = buf;               /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */

        SDCARD_ATC0_Flag = FALSE;               /* �SATC���������t���O�N���A */
        SDCARD_FPGA_Flag = FALSE;               /* FPGA���������t���O�N���A */
        SDCARD_EndFlag = FALSE;                 /* �]�����������t���O�N���A */
        SDCARD_ErrStatus = SDMC_NORMAL;         /* �G���[�X�e�[�^�X�̃N���A */

#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);   /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif
        /* IP��SD_SECCNT���W�X�^�L�����A�]���Z�N�^���ݒ�(����CMD12���s�̂���) */
        SD_EnableSeccnt( ulSDCARD_RestSectorCount);

        thread_flag = TRUE;
        /*--- ���[�h�R�}���h���s ---*/
        if( SDCARD_SDHCFlag) {
            SD_MultiReadBlock( offset);         /* �����Foffset(SDHC�̓Z�N�^�T�C�Y512Byte�Œ�) */
        }else{
            SD_MultiReadBlock( offset * SDCARD_SectorSize); /* �����Foffset �~ �Z�N�^�T�C�Y */
        }
        /*--------------------------*/

        /**/
        PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
        //can_wup( 0);
        slp_tsk();
#else
        /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
        /*--------------------*/
#endif
        PRINTDEBUG( "waked\n");

        thread_flag = FALSE;
        while( !SDCARD_EndFlag) {                          /* SD�J�[�h R/W �A�N�Z�X�I���t���O�� ON�i�I���j�҂� */
            if( SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[��? */
                return SDCARD_ErrStatus;                   /* �G���[�I�� */
            }
        }
        /* �G���[���������Ă��Ȃ����A�^�C���A�E�g�ȊO�̃G���[�̏ꍇ */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){      /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ���? */
            SD_CheckStatus(TRUE);                          /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
            if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){  /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ���? */
                SD_SendStatus();                           /* �J�[�h�X�e�[�^�X�̎擾�R�}���h���s */
                SD_CheckStatus(TRUE);                      /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
            }
        }
        SaveStatus = SDCARD_Status;                   /* �J�[�h�X�e�[�^�X�̕ۑ� */
        SaveErrStatus = SDCARD_ErrStatus;             /* �G���[�X�e�[�^�X�̕ۑ� */

        if( SDCARD_ErrStatus) {                       /* �G���[�X�e�[�^�X�̊m�F(�G���[�L��?) */
            i_sdmcErrProcess();                       /* �G���[���̏���(status�擾�A������~) */
        }
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���m�F */
            return SDCARD_ErrStatus;                  /* �G���[�I�� */
        }
        SDCARD_Status = SaveStatus;                   /* �J�[�h�X�e�[�^�X�̕��A */
        SDCARD_ErrStatus = SaveErrStatus;             /* �G���[�X�e�[�^�X�̕��A */


        if(SDCARD_ErrStatus == SDMC_NORMAL){          /* �G���[�X�e�[�^�X�̊m�F�i�G���[�����H�j*/
            break;                                    /* ���g���C���[�v���甲���� */
        }
    }

    SD_DisableClock();                                /* �N���b�N������~ */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_FPGA_irq

  Description:  read/write between Card, when BRE or BWE interrupt.
                �J�[�h����BRE(���[�h�v��)�܂���BWE(���C�g�v��)���荞�݂�����
                �����Ƃ��A���ۂɃ��[�h�܂��̓��C�g���s���B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_FPGA_irq(void)
{
    BOOL    bRead;

    /* FIFO���g���Ƃ��FFIFO���荞�݋���Ԃ����āARead��Write�����ʂ���*/
    /* FIFO���g��Ȃ��Ƃ��FBRE��BWE���𒲂ׁAINFO2�̓��Y���荞�ݗv���t���O���N���A���� */
    bRead = SD_FPGA_irq();

    if( ulSDCARD_RestSectorCount) {                 /* �c��Z�N�^�����L�鎞 */
        ulSDCARD_RestSectorCount--;                 /* �c��Z�N�^�����f�N�������g */

        if( TransCount==0) {                        /* �]���J�E���g����0��? (R/W�����ݗv������ 1��̂ݐݒ�)*/
            if( SDCARD_UseFifoFlag) {
                TransCount = (u16)(SDCARD_SectorSize/4); /* �]���J�E���g�ݒ� (32bit�]���Ȃ̂ŃZ�N�^�T�C�Y��1/4) */
            }else{
                TransCount = (u16)(SDCARD_SectorSize/2); /* �]���J�E���g�ݒ� (16bit�]���Ȃ̂ŃZ�N�^�T�C�Y�̔���) */
            }
        }

        /*--- SD�J�[�h����̃��[�h�� ---*/
        if(bRead){
            while(TransCount != 0){                                      /* �]���J�E���g���̃��[�v */
                if( SDCARD_UseFifoFlag) {                                /*--- FIFO���g���Ƃ� ---*/
                    *((u32*)pSDCARD_BufferAddr) = *(SDIF_FI);            /* 32bit�ǂݏo�� */
                }else{                                                   /*--- FIFO���g��Ȃ��Ƃ� ---*/
                    *(pSDCARD_BufferAddr) = *(SD_BUF0);                  /* 16bit�ǂݏo�� */
                }
                TransCount = (u16)(TransCount-1);                        /* �]���J�E���g�̃f�N�������g */
                if(TransCount == 0){                                     /* �]���J�E���g���I��? */
                    if( ulSDCARD_RestSectorCount <= 0) {                 /* �v���Z�N�^�����[�h����������? */
                        if(SD_CheckFPGAReg(SD_STOP,SD_STOP_SEC_ENABLE)){ /* SD_SECCNT���W�X�^��Enable��? */
//                            SD_DisableSeccnt();                        /* SD_SECCNT���W�X�^�����ݒ� */
                        }else{                                           /* SD_SECCNT���W�X�^��Disable�̂Ƃ� */
                            SD_StopTransmission();                       /* �J�[�h�]���I����FPGA�ɒʒm�iCMD12���s�j */
                        }
                    }
                    SDCARD_ATC0_irq();                                   /* �]��������̏��� */
                }
                if( SDCARD_UseFifoFlag) {
                    pSDCARD_BufferAddr+=2;                               /* �Ǎ��݃f�[�^�̃o�b�t�@�A�h���X���C���N�������g */
                }else{
                    pSDCARD_BufferAddr++;                                /* �Ǎ��݃f�[�^�̃o�b�t�@�A�h���X���C���N�������g */
                }
            }
        }else{    /*--- SD�J�[�h�ւ̃��C�g�� ---*/
            while(TransCount != 0){                                      /* �]���J�E���g���̃��[�v */
                if( SDCARD_UseFifoFlag) {                                /*--- FIFO���g���Ƃ� ---*/
#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
                    *(vu16*)0x08030200 = 1;                              /* �u���b�h�{�[�h�ŗL�̐�ǂݖ��� */
#endif
                    *(SDIF_FI) = *((u32*)pSDCARD_BufferAddr);            /* 32bit�������� */
#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
                    *(vu16*)0x08030200 = 0;                              /* �u���b�h�{�[�h�ŗL�̐�ǂݗL�� */
#endif
                }else{                                                   /*--- FIFO���g��Ȃ��Ƃ� ---*/
#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
                    *(vu16*)0x08030200 = 1;                              /* �u���b�h�{�[�h�ŗL�̐�ǂݖ��� */
#endif
                    *(SD_BUF0) = *(pSDCARD_BufferAddr);                  /* 16bit�������� */
#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
                    *(vu16*)0x08030200 = 0;                              /* �u���b�h�{�[�h�ŗL�̐�ǂݗL�� */
#endif
                }
                TransCount = (u16)(TransCount-1);                        /* �]���J�E���g�̃f�N�������g */
                if(TransCount == 0){                                     /* �]���J�E���g���I��? */
                    if( ulSDCARD_RestSectorCount <= 0){                  /* �v���Z�N�^�����C�g����? */
                        if( SDCARD_UseFifoFlag) {                        /* FIFO���g�p����Ƃ��� */
                            while( (*SDIF_CNT & SDIF_CNT_NEMP)) {};      /* FIFO�Ƀf�[�^���c���Ă��邤���͏I�����Ȃ� */
                        }
                        if(SD_CheckFPGAReg(SD_STOP,SD_STOP_SEC_ENABLE)){ /* SD_SECCNT���W�X�^��Enable��? */
//                            SD_DisableSeccnt();                        /* SD_SECCNT���W�X�^�����ݒ� */
                        }else{                                           /* SD_SECCNT���W�X�^��Disable�̂Ƃ� */
                            SD_StopTransmission();                       /* �J�[�h�]���I����FPGA�ɒʒm�iCMD12���s�j */
                        }
                    }
                    SDCARD_ATC0_irq();                                   /* �]��������̏��� */
                }
                if( SDCARD_UseFifoFlag) {
                    pSDCARD_BufferAddr+=2;                               /* �����݃f�[�^�̃o�b�t�@�A�h���X���C���N�������g */
                }else{
                    pSDCARD_BufferAddr++;                                /* �����݃f�[�^�̃o�b�t�@�A�h���X���C���N�������g */
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_ATC0_irq

  Description:
                �J�[�h�Ƃ̓]���I�����ɍs�������B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_ATC0_irq(void)
{
    /*--- �c��Z�N�^���� 0 �̂Ƃ� ---*/
    if( ulSDCARD_RestSectorCount == 0) {
        SDCARD_ATC0_Flag = TRUE;            /* �SATC�����t���O�Z�b�g */

        /* �]������ */
        if( SDCARD_FPGA_Flag) {             /* �]���I�����̊��荞�ݏ���(SYSFPGA_irq)���������Ă���? */
            SDCARD_TimerStop();             /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
            SD_TransEndFPGA();              /* �]���I������(���荞�݃}�X�N���֎~�ɖ߂�) */
            if( SDCARD_EndFlag == FALSE) {  /* �]�����������t���O�������Ă��Ȃ�? */
                SDCARD_EndFlag = TRUE;      /* �]�����������t���O���Z�b�g */
                if( pSDCARD_info) {
                    pSDCARD_info->result = SDCARD_ErrStatus; /* SdmcResultInfo �ɏ��ݒ� */
                    pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                        SDCARD_SectorSize;                   /* SdmcResultInfo �ɏ����Z�N�^���ݒ� */
                }
            }
        }
    }else{    /*--- �c��Z�N�^���� 0 �łȂ��Ƃ� ---*/
        SDCARD_TimerStop();                 /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);   /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         SYSFPGA_irq

  Description:  insert/remove/error/access end interrupt handler.
                BRE��BWE���荞�݂��������荞�݂̃n���h���B�}���A�G���[�����A
                �A�N�Z�X�I���̊��荞�ݔ������ɂ��ꂼ��̏������s���B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SYSFPGA_irq(void)
{
    /*--- �|�[�g0�̑}�����荞�݃`�F�b�N�ƃR�[���o�b�N�N�� ---*/
    if(!SD_CheckFPGAReg( SD_INFO1_MASK, SD_INFO1_MASK_REMOVE)){ /* SD Card ���� �����݋���Ԃ�? */
        if( SD_CheckFPGAReg( SD_INFO1, SD_INFO1_REMOVE)){       /* SD Card ���� ������? */
            SD_AndFPGA( SD_INFO1,(~SD_INFO1_REMOVE));           /* INFO1�̔����t���O�𗎂Ƃ� */
            SDCARD_OutFlag = TRUE;                              /* �r�o�t���O�Z�b�g */
            if(func_SDCARD_Out){                                /* �R�[���o�b�N�֐���Null�`�F�b�N */
                SDCARD_IO_Port = SDCARD_PORT0;                  /* �J�[�h�����|�[�g�ԍ���ݒ� */
                func_SDCARD_Out();                              /* �J�[�h�����R�[���o�b�N�֐��Ăяo�� */
            }
        }
    }
    if(!SD_CheckFPGAReg( SD_INFO1_MASK, SD_INFO1_MASK_INSERT)){ /* SD Card �}�� �����݋���Ԃ�? */
        if( SD_CheckFPGAReg( SD_INFO1, SD_INFO1_INSERT)){       /* SD Card �}�� ������? */
            SD_AndFPGA( SD_INFO1, (~SD_INFO1_INSERT));          /* INFO1�̑}���t���O�𗎂Ƃ� */
            SDCARD_OutFlag = FALSE;                             /* �r�o�t���O���Z�b�g */
            if(func_SDCARD_In){                                 /* �R�[���o�b�N�֐���Null�`�F�b�N */
                SDCARD_IO_Port = SDCARD_PORT0;                  /* �J�[�h�}���|�[�g�ԍ���ݒ� */
                func_SDCARD_In();                               /* �J�[�h�}���R�[���o�b�N�֐��Ăяo�� */
            }
        }
    }
    /* (CTR�̓|�[�g1��CD�[�q�����ڑ�(��ɑ}�����)�Ȃ̂ŁA�|�[�g1�̑}���`�F�b�N�͍s��Ȃ�) */

    /*--- ���荞�ݗv���Ɗ��荞�݃}�X�N��ۑ� ---*/
    SD_GetFPGA( SD_INFO1_VALUE,            SD_INFO1);
    SD_GetFPGA( SD_INFO1_MASK_VALUE,    SD_INFO1_MASK);
    SD_GetFPGA( SD_INFO2_VALUE,            SD_INFO2);
    SD_GetFPGA( SD_INFO2_MASK_VALUE,    SD_INFO2_MASK);
    /*------------------------------------------*/

    /*--- SD_INFO2�̃G���[�t���O�쐬 ---*/
    SD_INFO_ERROR_VALUE = (u16)(SD_INFO2_VALUE & (~SD_INFO2_MASK_VALUE));
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
    if ( SDCARD_ErrStatus) {                                    /* ���炩�̃G���[���������Ă��邩? */
        SD_OrFPGA( SD_INFO2_MASK, SD_INFO2_MASK_ERRSET);        /* �S�ẴG���[���荞�݂��֎~ */
    }
    /*--- SD_INFO2�̃G���[�`�F�b�N�I�� ---*/


    /*--- SD_INFO1�̊��荞�ݔ����󋵃t���O�쐬 ---*/
    SD_INFO_ERROR_VALUE = (u16)(SD_INFO1_VALUE & (~SD_INFO1_MASK_VALUE));

    if( SD_INFO_ERROR_VALUE & SD_INFO1_MASK_ALL_END) {          /* R/W access all end �����ݔ�����? */
        SD_OrFPGA( SD_INFO1_MASK, SD_INFO1_MASK_ALL_END);       /* INFO1�� access all end �����݋֎~ */
        SDCARD_FPGA_Flag = TRUE;                                /* R/W�A�N�Z�X�I��(IP��������)�t���O�Z�b�g */
        if( SDCARD_ATC0_Flag) {                                 /* �]����������(SDCARD_ATC0_irq)���������Ă��邩? */
            SDCARD_TimerStop();                                 /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
            SD_TransEndFPGA();                                  /* �]���I������(���荞�݃}�X�N���֎~�ɖ߂�) */
            if( SDCARD_EndFlag == FALSE) {                      /* �]�����I�����Ă��Ȃ���? */
                SDCARD_EndFlag = TRUE;                          /* �]�����������t���O�Z�b�g */
                if( pSDCARD_info) {                             /* Null�`�F�b�N */
                    pSDCARD_info->result = SDCARD_ErrStatus;    /* SdmcResultInfo �ɏ��ݒ� */
                    pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                                            SDCARD_SectorSize;  /* SdmcResultInfo �ɏ����Z�N�^���ݒ� */
                }
            }    /* �]�����I���ς݂̂Ƃ� */
            return;
        }        /* �SATC�����t���O OFF �̏ꍇ */
        if( SDCARD_ErrStatus != SDMC_NORMAL) {                  /* �G���[���������Ă���ꍇ */
            SDCARD_TimerStop();                                 /* �^�C���A�E�g����p�^�C�}�X�g�b�v */
            if( SDCARD_EndFlag == FALSE) {                      /* �]�����I�����Ă��Ȃ���? */
                SD_TransEndFPGA();                              /* �J�[�h�]���̏I������ */
                SDCARD_EndFlag = TRUE;                          /* �]�����������t���O�Z�b�g */
            /*    SD_Init();    */                              /* SD Card�C���^�[�t�F�[�X�������Z�b�g&�����ݒ� */
                if( pSDCARD_info) {                             /* Null�`�F�b�N */
                    pSDCARD_info->result = SDCARD_ErrStatus;    /* SdmcResultInfo �ɏ��ݒ� */
                    pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                                            SDCARD_SectorSize;  /* SdmcResultInfo �ɏ����Z�N�^���ݒ� */
                }
            }
        }
    }                                                           /* R/W access all end ���荞�ݖ����� */
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_TimerStart

  Description:  start timer for measure timeout.
                �^�C���A�E�g�v�����J�n����

  Arguments:    tim : ms�P�ʂ̃^�C���A�E�g����
                (50ms�𒴂���l�̏ꍇ��50ms�P�ʂɂȂ�)

  Returns:      None
 *---------------------------------------------------------------------------*/
static    void    SDCARD_TimerStart(u32 tim)
{
#if (TARGET_OS_CTR == 1)
    sta_alm( sdmc_alm_id, tim);
    PRINTDEBUG( "Timer Started.\n");
#else
    OSTick tim_tick;

    tim_tick = OS_MilliSecondsToTicks( tim);        //us�P�ʂ���Tick�P�ʂ�

    OS_CancelAlarm( &sdmc_alm);                     //�A���[���j��
    OS_SetAlarm( &sdmc_alm, tim_tick, SDCARD_Timer_irq, NULL);   //�A���[���Z�b�g
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_TimerStop

  Description:  stop timer
                �^�C���A�E�g�v�����~����

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    SDCARD_TimerStop(void)
{
#if (TARGET_OS_CTR == 1)
    stp_alm( sdmc_alm_id);
    PRINTDEBUG( "Timer Stopped.\n");

    //memo:���荞�݋֎~��Ԃɂ���stp_alm���Ă�ł���~���Ȃ��̂Œ���
#else
    OS_DisableIrq();
    OS_CancelAlarm( &sdmc_alm);
    OS_EnableIrq();
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Timer_irq

  Description:  timer interrupt handler.
                �^�C�}�[���荞�݃n���h��

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void    SDCARD_Timer_irq(void* arg)
{
#if (SD_DEBUG_PRINT_ON == 1)
    u16 tmp;

    PRINTDEBUG( ">>>Timer intr(Timeout)\n");

    SDCARD_ErrStatus |= SDMC_ERR_FPGA_TIMEOUT;    /* �^�C���A�E�g�G���[�r�b�g�̐ݒ� */

    tmp = SD_INFO1;
    PRINTDEBUG( "SD_INFO1      : 0x%x\n", tmp);
    tmp = SD_INFO1_MASK;
    PRINTDEBUG( "SD_INFO1_MASK : 0x%x\n", tmp);
    tmp = SD_INFO2;
    PRINTDEBUG( "SD_INFO2      : 0x%x\n", tmp);
    tmp = SD_INFO2_MASK;
    PRINTDEBUG( "SD_INFO2_MASK : 0x%x\n", tmp);
    tmp = SD_ERR_STS1;
    PRINTDEBUG( "SD_ERR_STS1   : 0x%x\n", tmp);
#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
    *(vu16*)0x08030200 = 1;
#endif
    tmp = SD_ERR_STS2;
#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
    *(vu16*)0x08030200 = 0;
#endif
    PRINTDEBUG( "SD_ERR_STS2   : 0x%x\n", tmp);
    tmp = *(vu16 *)(SD_IF_BASE+0x00);
    PRINTDEBUG( "SD_CNT        : 0x%x\n", tmp);
    tmp = SD_SECCNT;
    PRINTDEBUG( "SD_SECCNT     : 0x%x\n", tmp);
#endif


    if(SDCARD_EndFlag == FALSE){                    /* �]�����������t���O�̊m�F�i�N���A�H�j*/
        SDCARD_EndFlag = TRUE;                      /* �]�����������t���O���Z�b�g */
    /*    SD_TransEndFPGA();    */                  /* �J�[�h�]���̏I������ */
    /*    SD_StopTransmission();    */              /* �J�[�h�]���I���ݒ� */
    /*    SD_Init();    */                          /* SD Card�C���^�[�t�F�[�X�������Z�b�g&�����ݒ� */
        if(pSDCARD_info){                           /* Null�`�F�b�N */
            pSDCARD_info->result = SDCARD_ErrStatus;/* SdmcResultInfo �ɏ��ݒ� */
            pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                SDCARD_SectorSize;                  /* SdmcResultInfo �ɏ����Z�N�^���ݒ� */
        }
    }

    /**/
    PRINTDEBUG( "--Wup sdTsk(Time)--\n");
#if (TARGET_OS_CTR == 1)
    iwup_tsk( sdmc_tsk_id);
    PRINTDEBUG( "id : 0x%x\n", sdmc_tsk_id);
#else
    sdmc_wakeup_count++;
    PRINTDEBUG( "wakeup\n");
    OS_WakeupThreadDirect( &sdmc_tsk);
#endif
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcSendSCR

  Description:  get SCR register.
                SCR���擾����iDAT���C���o�R��8�o�C�g�����Ă���j�B
                MultiBlock R/W �ƈقȂ�ADAT���C���o�R�œ]������Ă���SD�J�[�h��
                ���W�X�^�́AMSB�����ɑ����Ă��邱�Ƃɒ��ӁB
                (Physical Layer Specification 2.00 p12-13�Q��)

  Arguments:    None

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 i_sdmcSendSCR(void)
{
    SDMC_ERR_CODE SaveErrStatus;          /* �G���[�X�e�[�^�X�ۑ��p */
    u32           SaveStatus;             /* �J�[�h�X�e�[�^�X�ۑ��p */
    u32           ulSave_SectorSize;      /* �Z�N�^�T�C�Y�ۑ��p */

    SD_EnableClock();                        /* SD-CLK Enable */

    /*    �u���b�N�T�C�Y�̐ݒ� */
    ulSave_SectorSize = SDCARD_SectorSize;   /* �Z�N�^�T�C�Y�̕ۑ� */
    SDCARD_SectorSize = 8;                   /* SCR ���W�X�^ �]���T�C�Y 8bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* SD�J�[�h�f�[�^�]���T�C�Y 8byte �ݒ� */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j*/
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);    /* �^�C���A�E�g����p�^�C�}�X�^�[�g */
#endif

    if(SD_AppCommand()){                  /* RCA�ݒ�� CMD55���s����������I�����Ȃ�? */
        SD_DisableClock();                /* SD-CLK Disable */
        return SDCARD_ErrStatus;          /* �G���[�I�� */
    }

/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1; /* �c��Z�N�^�T�C�Y�A�Z�N�^�J�E���g�ɂP��ݒ� */
    pSDCARD_BufferAddr = SD_SCR;                         /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */

    /* �]���O�̏������� */
    SDCARD_ATC0_Flag = FALSE;                /* �SATC�����t���O�N���A */
    SDCARD_FPGA_Flag = FALSE;                /* FPGA���������t���O�N���A */
    SDCARD_EndFlag = FALSE;                  /* �]�����������t���O�N���A */
    SDCARD_ErrStatus = SDMC_NORMAL;          /* �G���[�X�e�[�^�X�̃N���A */

#if SCR_ON
    thread_flag = TRUE;
    SD_SendSCR();                            /*    SCR�̎擾�R�}���h���s */
    PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
    //can_wup( 0);
    slp_tsk();
#else
    /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
    /*--------------------*/
#endif
    PRINTDEBUG( "waked\n");
    thread_flag = FALSE;

    while(!SDCARD_EndFlag){                            /* �J�[�h�A�N�Z�X�I���҂� */
      PRINTDEBUG( "k\n");
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){  /* �^�C���A�E�g�G���[���m�F */
            return SDCARD_ErrStatus;
        }
    }

    if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){     /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ����m�F */
        SD_CheckStatus(FALSE);                        /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){ /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ����m�F */
            SD_SendStatus();                      /* �J�[�h�X�e�[�^�X�̎擾�R�}���h���s���� */
            SD_CheckStatus(FALSE);                /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
        }
    }
    SaveStatus = SDCARD_Status;                   /* �J�[�h�X�e�[�^�X�̕ۑ� */
    SaveErrStatus = SDCARD_ErrStatus;             /* �G���[�X�e�[�^�X�̕ۑ� */
    if(SDCARD_ErrStatus){                         /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j*/
        i_sdmcErrProcess();                       /* �G���[���̏��� */
    }
    if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���m�F */
        return SDCARD_ErrStatus;
    }
    SDCARD_Status = SaveStatus;                   /* �J�[�h�X�e�[�^�X�̕��A */
    SDCARD_ErrStatus = SaveErrStatus;             /* �G���[�X�e�[�^�X�̕��A */
#endif

    SDCARD_SectorSize = ulSave_SectorSize;        /* �ۑ����Ă����Z�N�^�T�C�Y��߂��idefault:512bytes�j*/
    SD_SetBlockLength(SDCARD_SectorSize);         /* SD�J�[�h�f�[�^�]���T�C�Y 512bytes �ݒ� */

    SD_DisableClock();                            /* SD-CLK Disable */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcErrProcess

  Description:  when error is occured, get Card Status to check and stop the
                transfer.
                �G���[�������̏����B�J�[�h�X�e�[�^�X���擾���A�f�[�^�]������
                ����΃X�g�b�v������B

  Arguments:    None

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 i_sdmcErrProcess(void)
{
    u16 usRSP0;

    SDCARD_ErrStatus = SDMC_NORMAL;    /* �G���[�X�e�[�^�X�̃N���A */

    SDCARD_TimerStop();                /* �^�C���A�E�g����p�^�C�}�X�g�b�v */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_ERRPROC_TIMEOUT); /* �^�C���A�E�g����p�^�C�}�X�^�[�g(2000msec) */
#endif

    SD_SendStatus();                   /* CMD13 addressed card sends its status register ���s�A���X�|���X�҂� */

    if(!SDCARD_ErrStatus){             /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j*/
        SD_GetFPGA( usRSP0, SD_RSP0);
        usRSP0 = (u16)(( usRSP0 & RSP_R1_CURRENT_STATE) >> 1);               /* �J�����g�X�e�[�g�����o�� */
        if((usRSP0 == CURRENT_STATE_DATA) || (usRSP0 == CURRENT_STATE_RCV)){ /* SDCARD Status �� data rcv �̎� */
            SD_Command(SD_CMD_CMD | STOP_TRANSMISSION);                      /* CMD12�iStopTransmission�j���s���� */
        }
    }

    SDCARD_TimerStop();                /* �^�C���A�E�g����p�^�C�}�X�g�b�v */

    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         sdmcGetCardSize

  Description:  get card size (number of sectors).
                �J�[�h�̃Z�N�^�����擾����

  Arguments:    None

  Returns:      number of sectors in the SD card which inserted.
 *---------------------------------------------------------------------------*/
u32 sdmcGetCardSize(void)
{
    /* �J�[�h�S�Z�N�^�� (SDCARD_Layer_Init�֐����ŎZ�o�����) */
    return ulSDCARD_Size;
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_SD_Status

  Description:  get SD Status.
                SD�X�e�[�^�X���擾����iDAT���C���o�R��64�o�C�g�����Ă���j�B
                �J�[�h�X�e�[�^�X�ł͂Ȃ����Ƃ𗯈ӁB
                MultiBlock R/W �ƈقȂ�ADAT���C���o�R�œ]������Ă���SD�J�[�h��
                ���W�X�^�́AMSB�����ɑ����Ă��邱�Ƃɒ��ӁB
                (Physical Layer Specification 2.00 p12-13�Q��)

  Arguments:    None

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 SDCARD_SD_Status(void)
{
    SDMC_ERR_CODE SaveErrStatus;             /* �G���[�X�e�[�^�X�ۑ��p */
    u32           SaveStatus;                /* �J�[�h�X�e�[�^�X�ۑ��p */
    u32           ulSave_SectorSize;         /* �Z�N�^�T�C�Y�ۑ��p */
    u32           ulSaveRestSectorCount;     /* �c��Z�N�^�T�C�Y�ۑ��p */

    SD_EnableClock();                        /* SD-CLK Enable */

    /* �u���b�N�T�C�Y�̐ݒ� */
    ulSave_SectorSize = SDCARD_SectorSize;   /* �Z�N�^�T�C�Y�̕ۑ� */
    SDCARD_SectorSize = 64;                  /* SD_STATUS �]���T�C�Y 64bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* SD�J�[�h�f�[�^�]���T�C�Y 64byte �ݒ� */
    if(SDCARD_ErrStatus){                    /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j*/
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);    /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif

    if(SD_AppCommand()){                     /* RCA�ݒ�� CMD55���s���� ������I�����Ȃ�?*/
        SD_DisableClock();                   /* SD-CLK Disable */
        return SDCARD_ErrStatus;             /* �G���[�I�� */
    }

/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    ulSaveRestSectorCount = ulSDCARD_RestSectorCount;    /* �c��Z�N�^�T�C�Y��ۑ�(TODO:����Ȃ�?�j */
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1; /* �c��Z�N�^�T�C�Y�A�Z�N�^�J�E���g���P�ɐݒ� */
    pSDCARD_BufferAddr = SD_SDSTATUS;                    /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */

    /* �]���O�̏������� */
    SDCARD_ATC0_Flag = FALSE;                /* �SATC�����t���O�N���A */
    SDCARD_FPGA_Flag = FALSE;                /* FPGA���������t���O�N���A */
    SDCARD_EndFlag   = FALSE;                /* �]�����������t���O�N���A */
    SDCARD_ErrStatus = SDMC_NORMAL;          /* �G���[�X�e�[�^�X�̃N���A */

    thread_flag = TRUE;
    SD_SDStatus();                           /* ACMD13 SD_STATUS�̎擾�R�}���h���s���� */
    PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
    //can_wup( 0);
    slp_tsk();
#else
    /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
    /*--------------------*/
#endif
    PRINTDEBUG( "waked\n");
    thread_flag = FALSE;

    while(!SDCARD_EndFlag){                           /* �J�[�h�A�N�Z�X�I���҂� */
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���m�F */
            return SDCARD_ErrStatus;
        }
    }

    if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){     /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X�����炩�̃G���[�łȂ����m�F */
        SD_CheckStatus(FALSE);                        /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){ /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ����m�F */
            SD_SendStatus();                      /* �J�[�h�X�e�[�^�X�擾�R�}���h���s�A���X�|���X(R1)�҂� */
            SD_CheckStatus(FALSE);                /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
        }
    }
    SaveStatus = SDCARD_Status;                   /* �J�[�h�X�e�[�^�X�̕ۑ� */
    SaveErrStatus = SDCARD_ErrStatus;             /* �G���[�X�e�[�^�X�̕ۑ� */
    if(SDCARD_ErrStatus){                         /* �G���[�X�e�[�^�X�̊m�F�i�G���[�L��H�j*/
        i_sdmcErrProcess();                       /* �G���[���̏��� */
    }
    if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���m�F */
        return SDCARD_ErrStatus;
    }
    SDCARD_Status = SaveStatus;                   /* �J�[�h�X�e�[�^�X�̕��A */
    SDCARD_ErrStatus = SaveErrStatus;             /* �G���[�X�e�[�^�X�̕��A */

    SDCARD_SectorSize = ulSave_SectorSize;            /* �ۑ����Ă����Z�N�^�T�C�Y��߂��idefault:512bytes�j*/
    SD_SetBlockLength(SDCARD_SectorSize);             /* SD�J�[�h�f�[�^�]���T�C�Y 512bytes �ݒ� */
    ulSDCARD_RestSectorCount = ulSaveRestSectorCount; /* �ۑ����Ă����c��Z�N�^�T�C�Y��߂�(TODO:����Ȃ�?)*/

    SD_DisableClock();                            /* SD-CLK Disable */

    return SDCARD_ErrStatus;
}

/*******************************************************************************/
int MMCP_SetBusWidth( BOOL b4bit)
{
    u32 ulSave_SectorSize;                          /* �Z�N�^�T�C�Y�ۑ��p */
//    u16 TestData;
    u16 Resid;

    SD_EnableClock();                        /* SD-CLK Enable */

#if 0
    TestData = 0x5A;//0xA5;

    /* �u���b�N�T�C�Y�̐ݒ� */
    ulSave_SectorSize = SDCARD_SectorSize;             /* �Z�N�^�T�C�Y�̕ۑ� */
    SDCARD_SectorSize = 4;                             /* �]���T�C�Y 1�o�C�g */
    SD_SetBlockLength( SDCARD_SectorSize);             /* SD�J�[�h�f�[�^�]���T�C�Y 1byte �ݒ� */
#endif
    /*�R�}���h6���s*/
    MMCP_WriteBusWidth( b4bit);
    SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IP�Ƀr�b�g���̐ݒ�(4bit��) */

#if 0
    /**/
    pSDCARD_info = NULL;
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1;
    pSDCARD_BufferAddr = &TestData;            /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */
    SDCARD_ATC0_Flag = FALSE;                  /* �SATC�����t���O�N���A */
    SDCARD_FPGA_Flag = FALSE;                  /* FPGA���������t���O�N���A */
    SDCARD_EndFlag   = FALSE;                  /* �]�����������t���O�N���A */
    SDCARD_ErrStatus = SDMC_NORMAL;            /* �G���[�X�e�[�^�X�̃N���A */

#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);      /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif

    /* IP��SD_SECCNT���W�X�^�L�����A�]���Z�N�^���ݒ�(����CMD12���s�̂���) */
//    SD_EnableSeccnt( ulSDCARD_RestSectorCount);

    /*�o�X�e�X�g*/
    MMCP_BusTest( FALSE);

    /**/
    while( !SDCARD_EndFlag) {                         /* �J�[�h�A�N�Z�X�I���҂� */
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[��? */
            return SDCARD_ErrStatus;                  /* �G���[�I�� */
        }
    }


    /**/
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1;/* �c��Z�N�^�T�C�Y�A�Z�N�^�J�E���g�ɂP��ݒ� */
    pSDCARD_BufferAddr = &Resid;                         /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */
    SDCARD_ATC0_Flag = FALSE;                          /* �SATC�����t���O�N���A */
    SDCARD_FPGA_Flag = FALSE;                          /* FPGA���������t���O�N���A */
    SDCARD_EndFlag   = FALSE;                          /* �]�����������t���O�N���A */
    SDCARD_ErrStatus = SDMC_NORMAL;                    /* �G���[�X�e�[�^�X�̃N���A */

#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);      /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif

    /**/
    MMCP_BusTest( TRUE);

    /**/
    while( !SDCARD_EndFlag) {                          /* �J�[�h�A�N�Z�X�I���҂� */
        if( SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[��? */
            return SDCARD_ErrStatus;                   /* �G���[�I�� */
        }
    }

    /*�o�X�e�X�g���i�Ȃ�o�X����4bit�Ɋg��*/
//    if( TestData == (~(Resid))) {
    if( Resid == 0xA5) {
        SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IP�Ƀr�b�g���̐ݒ�(4bit��) */
    }else{
        SD_OrFPGA(SD_OPTION,(SD_OPTION_WIDTH_1BIT)); /* IP�Ƀr�b�g���̐ݒ�(1bit��) */
    }

    SDCARD_SectorSize = ulSave_SectorSize;             /* �ۑ����Ă����Z�N�^�T�C�Y��ݒ�i�f�t�H���g 512bytes�ɖ߂��j*/
    SD_SetBlockLength( SDCARD_SectorSize);             /* SD�J�[�h�f�[�^�]���T�C�Y 512bytes �ݒ� */
#endif
    return( 0);
}
/*******************************************************************************/


/*---------------------------------------------------------------------------*
  Name:         i_sdmcCheckWP

  Description:  check the write protect bit in the SD_INFO1 register.
                SD_INFO1���W�X�^�̃��C�g�v���e�N�g�r�b�g�𒲂ׂ�
                �i0:���C�g�v���e�N�g����Ă���A1:���C�g�v���e�N�g����Ă��Ȃ��j�B
                CTR�ł̓|�[�g1�͏��1�B

  Arguments:    None

  Returns:      number of sectors in the SD card which inserted.
 *---------------------------------------------------------------------------*/
static u16  i_sdmcCheckWP(void)
{
    if (SD_port_number == SDCARD_PORT0)            /* �|�[�g0�̂Ƃ� */
    {
        if(!(SD_CheckFPGAReg(SD_INFO1,SD_INFO1_WRITEPROTECT))) { //WP�t���O�������Ă��Ȃ���?
            SDCARD_ErrStatus |= SDMC_ERR_WP;    //�G���[�t���O��WP�G���[�r�b�g�𗧂Ă�
        }else{                                  //WP�t���O�������Ă����Ƃ�
            SDCARD_ErrStatus &= ~SDMC_ERR_WP;   //�G���[�t���O��WP�G���[�r�b�g�𗎂Ƃ�
        }
        return    SDCARD_ErrStatus;
    }
    else if (SD_port_number == SDCARD_PORT1)       /* �|�[�g1�̂Ƃ� */
    {  /*TWL�̃|�[�g1�̓��C�g�v���e�N�g�r�b�g�����0�i�v���e�N�g��ԁj�Ȃ̂Ŕ��]���ĕ]��*/
        if((SD_CheckFPGAReg(EXT_WP,EXT_WP_PORT1))) { //WP�t���O�������Ă��Ȃ���?
            SDCARD_ErrStatus |= SDMC_ERR_WP;          //�G���[�t���O��WP�G���[�r�b�g�𗧂Ă�
        }else{                                        //WP�t���O�������Ă����Ƃ�
            SDCARD_ErrStatus &= ~SDMC_ERR_WP;         //�G���[�t���O��WP�G���[�r�b�g�𗎂Ƃ�
        }
        return    SDCARD_ErrStatus;
    }
    return SDMC_ERR_END;    //�����ɂ͗��Ȃ�
}

/*---------------------------------------------------------------------------*
  Name:         sdmcWriteFifo

  Description:  write to card.
                ���b�p�[��FIFO���g�p���ăJ�[�h�ւ̏������݁B

  Arguments:    buf : �������݃f�[�^���i�[����Ă���o�b�t�@�̃A�h���X
                bufsize : �������ރT�C�Y�i�Z�N�^���j
                offset : �������݊J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcWriteFifo(void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
#if (TARGET_OS_CTR == 1)
    u32           recv_dat;
#else
    OSMessage     recv_dat;
#endif
    SDMC_ERR_CODE api_result;

    SdMsg.buf       = buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_WRITE_WITH_FIFO;

#if (TARGET_OS_CTR == 1)
    snd_dtq( sdmc_dtq_id, (VP_INT)&SdMsg);

    /* �Ԃ�l�҂� */
    rcv_dtq( sdmc_result_dtq_id, (VP_INT*)&recv_dat);

    api_result = (SDMC_ERR_CODE)recv_dat;
#else
    recv_dat = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);

    /* �Ԃ�l�҂� */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;
#endif

    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_WriteFifo

  Description:  write to card.
                ���b�p�[��FIFO���g�p���ăJ�[�h�ւ̏������݁B

  Arguments:    buf : �������݃f�[�^���i�[����Ă���o�b�t�@�̃A�h���X
                bufsize : �������ރT�C�Y�i�Z�N�^���j
                offset : �������݊J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_WriteFifo(void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    SDMC_ERR_CODE result;

    /* FIFO���荞�݋֎~ */
    *(SDIF_CNT) = (*(SDIF_CNT) & (~(SDIF_CNT_FFIE | SDIF_CNT_FEIE)));
    *(SDIF_FDS) = (u16)SDCARD_SectorSize;   /* FIFO�̃f�[�^�T�C�Y */
    *(SDIF_FSC) = bufsize;
    *(SDIF_CNT) |= SDIF_CNT_USEFIFO;        /* FIFO�g�p�t���OON */
    CC_EXT_MODE = CC_EXT_MODE_DMA;          /* DMA���[�hON */

    result = SDCARDi_Write( buf, bufsize, offset, func, info);

    /* FIFO������ */
    *(SDIF_CNT) &= (~SDIF_CNT_USEFIFO);     /* FIFO�g�p�t���OOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;          /* PIO���[�h(DMA���[�hOFF) */

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcWrite

  Description:  write to card.
                �J�[�h�ւ̏������݁B

  Arguments:    buf : �������݃f�[�^���i�[����Ă���o�b�t�@�̃A�h���X
                bufsize : �������ރT�C�Y�i�Z�N�^���j
                offset : �������݊J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcWrite(void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
#if (TARGET_OS_CTR == 1)
    u32           recv_dat;
#else
    OSMessage     recv_dat;
#endif
    SDMC_ERR_CODE api_result;

    SdMsg.buf       = buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_WRITE;

#if (TARGET_OS_CTR == 1)
    snd_dtq( sdmc_dtq_id, (VP_INT)&SdMsg);

    /* �Ԃ�l�҂� */
    rcv_dtq( sdmc_result_dtq_id, (VP_INT*)&recv_dat);

    api_result = (SDMC_ERR_CODE)recv_dat;
#else
    recv_dat = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);

    /* �Ԃ�l�҂� */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;
#endif

    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_Write

  Description:  write to card.
                �J�[�h�ւ̏������݁B

  Arguments:    buf : �������݃f�[�^���i�[����Ă���o�b�t�@�̃A�h���X
                bufsize : �������ރT�C�Y�i�Z�N�^���j
                offset : �������݊J�n�I�t�Z�b�g�i�Z�N�^�ԍ��j
                info : ���s���ʂ��i�[���邽�߂̍\���̂ւ̃A�h���X

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_Write(void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    s16           nRetryCount;
    u32           ulResid;
    SDMC_ERR_CODE SaveErrStatus;  /* �G���[�X�e�[�^�X�ۑ��p */
    u32           SaveStatus;     /* �J�[�h�X�e�[�^�X�ۑ��p */

    if( func != NULL){            /* �R�[���o�b�N�֐���Null�`�F�b�N */
        return SDMC_ERR_PARAM;    /* �R�}���h�p�����[�^�G���[ */
    }

#if WP_ena
    if( i_sdmcCheckWP()) {
        return SDMC_ERR_WP;       /*** ���C�g�v���e�N�g�̃`�F�b�N    ***/
    }
#endif
    for(nRetryCount = 0;nRetryCount < SDCARD_RETRY_COUNT;nRetryCount++){

    SD_EnableClock();             /* SD-CLK Enable */

/*    func_SDCARD_CallBack = func;    */
        pSDCARD_info = info;
        ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = bufsize;
        pSDCARD_BufferAddr = buf;                  /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */

        SDCARD_ATC0_Flag = FALSE;                  /* �SATC�����t���O�N���A */
        SDCARD_FPGA_Flag = FALSE;                  /* FPGA���������t���O�N���A */
        SDCARD_EndFlag = FALSE;                    /* �]�����������t���O�N���A */
        SDCARD_ErrStatus = SDMC_NORMAL;            /* �G���[�X�e�[�^�X�̃N���A */

#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);      /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif

        /* IP��SD_SECCNT���W�X�^�L�����A�]���Z�N�^���ݒ�(����CMD12���s�̂���) */
        SD_EnableSeccnt( ulSDCARD_RestSectorCount);

        /*--- ���C�g�R�}���h���s ---*/
        if( SDCARD_SDHCFlag) {
            SD_MultiWriteBlock( offset);           /* ���C�g�R�}���h���s(�����F�I�t�Z�b�g) */
        }else{
            SD_MultiWriteBlock( offset * SDCARD_SectorSize); /* ���C�g�R�}���h���s(�����F�I�t�Z�b�g*�Z�N�^�T�C�Y) */
        }
        /*--------------------------*/

        /**/
        while( !SDCARD_EndFlag) {                         /* �J�[�h�A�N�Z�X�I���҂� */
            if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[��? */
                return SDCARD_ErrStatus;                  /* �G���[�I�� */
            }
        }

        /* �G���[���������Ă��Ȃ����A�^�C���A�E�g�ȊO�̃G���[�̏ꍇ */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){     /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ����m�F */
            SD_CheckStatus(FALSE);                        /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
            if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){ /* �R�}���h���X�|���X(R1)�̃J�[�h�X�e�[�^�X���G���[�łȂ����m�F */
                SD_SendStatus();                      /* �J�[�h�X�e�[�^�X�̎擾�R�}���h���s���� */
                SD_CheckStatus(FALSE);                /* �R�}���h���X�|���X(R1)�� Card Status �`�F�b�N */
            }
        }
        SaveStatus = SDCARD_Status;                   /* �J�[�h�X�e�[�^�X�̕ۑ� */
        SaveErrStatus = SDCARD_ErrStatus;             /* �G���[�X�e�[�^�X�̕ۑ� */
        if( SDCARD_ErrStatus) {                       /* �G���[�X�e�[�^�X�̊m�F(�G���[�L��?)*/
            i_sdmcErrProcess();                       /* �G���[���̏���(status�擾�A������~) */
        }
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���m�F */
            return SDCARD_ErrStatus;                  /* �G���[�I�� */
        }
#if RESID
        if( SDCARD_SDFlag) {                          /* SD�J�[�h(MMC�łȂ�)�̏ꍇ */
            if( SDCARD_UseFifoFlag) {                 /*--- FIFO���g�p���Ă���Ƃ��� ---*/
                *(SDIF_CNT) &= (~SDIF_CNT_USEFIFO);   /* �ꎞ�I��FIFO���g�p���[�h�ɂ��� */
                CC_EXT_MODE = CC_EXT_MODE_PIO;
                i_sdmcGetResid(&ulResid);             /* �������݊����Z�N�^���̎擾 */
                *(SDIF_CNT) |= SDIF_CNT_USEFIFO;      /* FIFO�g�p���[�h�ɖ߂� */
                CC_EXT_MODE = CC_EXT_MODE_DMA;
            }else{                                    /*--- FIFO���g�p���Ă��Ȃ��Ƃ� ---*/
                i_sdmcGetResid(&ulResid);             /* �������݊����Z�N�^���̎擾 */
            }
            if(info){
                info->resid = ulResid * SDCARD_SectorSize;/*** pSDCARD_info->resid ��info->resid �ɏC���@���@2000.08.31. ***/
            }
            if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[���m�F */
                return SDCARD_ErrStatus;
            }
        }
#endif
        SDCARD_Status = SaveStatus;                   /* �J�[�h�X�e�[�^�X�̕��A */
        SDCARD_ErrStatus = SaveErrStatus;             /* �G���[�X�e�[�^�X�̕��A */
#if RESID
        if( SDCARD_SDFlag) {                          /* SD�J�[�h(MMC�łȂ�)�̏ꍇ */
            if( bufsize != ulResid){                  /* ���C�g�ς݃Z�N�^�����������Ȃ���? */
                SD_SetErr( SDMC_ERR_NUM_WR_SECTORS);  /* �G���[�t���O�Z�b�g */
            }
        }
#endif
        if( SDCARD_ErrStatus == SDMC_NORMAL) {        /* �G���[�X�e�[�^�X�̊m�F(�G���[����?)*/
            break;
        }
    }

    SD_DisableClock();                                /* �N���b�N������~ */

    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcGetResid

  Description:  get the numbers of the well written(without errors) blocks.
                �������݂����������Z�N�^�̐����擾����B


  Arguments:    pResid : �������݊����Z�N�^����Ԃ��ϐ��ւ̃|�C���^

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 i_sdmcGetResid(u32 *pResid)
{
    u16    Resid[2];
    u32    ulSave_SectorSize;                          /* �Z�N�^�T�C�Y�ۑ��p */

    SDCARD_ErrStatus = SDMC_NORMAL;                    /* �G���[�X�e�[�^�X���N���A */

    /* �u���b�N�T�C�Y�̐ݒ� */
    ulSave_SectorSize = SDCARD_SectorSize;             /* �Z�N�^�T�C�Y�̕ۑ� */
    SDCARD_SectorSize = 4;                             /* �]���T�C�Y 4�o�C�g */
    SD_SetBlockLength( SDCARD_SectorSize);             /* SD�J�[�h�f�[�^�]���T�C�Y 4byte �ݒ� */
    if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){      /* �^�C���A�E�g�G���[��? */
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);              /* �^�C���A�E�g����p�^�C�}�X�^�[�g(4000msec) */
#endif

    if( SD_AppCommand()) {                             /* RCA�ݒ�� CMD55���s���� */
        return SDCARD_ErrStatus;                       /* �G���[����(CMD55������I�����Ȃ�)�Ȃ�߂� */
    }

/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1;/* �c��Z�N�^�T�C�Y�A�Z�N�^�J�E���g�ɂP��ݒ� */
    pSDCARD_BufferAddr = Resid;                         /* �f�[�^�i�[�o�b�t�@�̃A�h���X��ݒ� */

    SDCARD_ATC0_Flag = FALSE;                          /* �SATC�����t���O�N���A */
    SDCARD_FPGA_Flag = FALSE;                          /* FPGA���������t���O�N���A */
    SDCARD_EndFlag   = FALSE;                          /* �]�����������t���O�N���A */
    SDCARD_ErrStatus = SDMC_NORMAL;                    /* �G���[�X�e�[�^�X�̃N���A */

    thread_flag = TRUE;
    /*--- ACMD22 ���C�g�ς݃Z�N�^���擾�R�}���h���s ---*/
    SD_SendNumWRSectors();
    /*-------------------------------------------------*/
    PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
//    can_wup( 0);
    slp_tsk();
#else
    /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
    /*--------------------*/
#endif
    PRINTDEBUG( "waked\n");
    thread_flag = FALSE;

    while( !SDCARD_EndFlag) {                          /* �J�[�h�A�N�Z�X�I���҂� */
        if( SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* �^�C���A�E�g�G���[��? */
            return SDCARD_ErrStatus;                   /* �G���[�I�� */
        }
    }
    /* �G���[���������Ă��Ȃ����A�^�C���A�E�g�ȊO�̃G���[�̏ꍇ */
    if( SDCARD_ErrStatus) {                            /* (�^�C���A�E�g�ȊO��)�G���[������? */
        i_sdmcErrProcess();                            /* �G���[���̏��� */
        *pResid = 0L;                                  /* ���C�g�ς݃Z�N�^���� 0 ��ݒ� */
    }else{                                             /* �G���[���������Ă��Ȃ��ꍇ */
        /* SD�J�[�h�̃��W�X�^��MSB���瑗���Ă��邽�ߕ��בւ����s�� */
        Resid[1] = SD_SwapByte(&Resid[1]);             /* ��� 1byte �Ɖ��� 1byte �����ւ��� */
        (((LELONG *)pResid)->dt2word.low) = Resid[1];  /* Resid[1]�̐ݒ� */
        Resid[0] = SD_SwapByte(&Resid[0]);             /* ��� 1byte �Ɖ��� 1byte �����ւ��� */
        (((LELONG *)pResid)->dt2word.high) = Resid[0]; /* Resid[0]�̐ݒ� */
    }

    SDCARD_SectorSize = ulSave_SectorSize;             /* �ۑ����Ă����Z�N�^�T�C�Y��ݒ�i�f�t�H���g 512bytes�ɖ߂��j*/
    SD_SetBlockLength( SDCARD_SectorSize);             /* SD�J�[�h�f�[�^�]���T�C�Y 512bytes �ݒ� */
    /* SetBlockLength�ŃG���[���o����ǂ�����? */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Restore_port0

  Description:  restore registers and variables of port0.
                �|�[�g0�̃��W�X�^��ϐ��𕜋A����B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Restore_port0(void)
{
    /* registers */
    SD_SetFPGA( SD_CLK_CTRL, SDPort0Context.SD_CLK_CTRL_VALUE);
    SD_SetFPGA( SD_OPTION,   SDPort0Context.SD_OPTION_VALUE);

    /* variables */
    SD_RCA           = SDPort0Context.SD_RCA;
    SDCARD_ErrStatus = SDPort0Context.ErrStatus;
    SDCARD_Status    = SDPort0Context.Status;
    SDCARD_MMCFlag   = SDPort0Context.MMCFlag;
    SDCARD_SDHCFlag  = SDPort0Context.SDHCFlag;
    SDCARD_SDFlag    = SDPort0Context.SDFlag;

    SDCARD_OutFlag = SDPort0Context.OutFlag;
    pSDCARD_info   = NULL;

    /*media registers*/
    MI_CpuCopy8( SDPort0Context.SD_CID, SD_CID, 16);
    MI_CpuCopy8( SDPort0Context.SD_CSD, SD_CSD, 16);
    MI_CpuCopy8( SDPort0Context.SD_OCR, SD_OCR,  4);
    MI_CpuCopy8( SDPort0Context.SD_SCR, SD_SCR,  8);

    /*recalc*/
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Restore_port1

  Description:  restore registers and variables of port0.
                �|�[�g1�̃��W�X�^��ϐ��𕜋A����B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SDCARD_Restore_port1(void)
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
  Name:         sdmcSelectedNo

  Description:  get selected port number.
                �I������Ă���|�[�g�ԍ����擾����

  Arguments:    None

  Returns:      [15:8]port numbers which supported�i�T�|�[�g����Ă���|�[�g���j
                [7:0]port number which selected now�i�I������Ă���|�[�g�ԍ��j
 *---------------------------------------------------------------------------*/
u16 sdmcSelectedNo(void)
{
    u16 i_sdmcSelect_Value;

    SD_GetFPGA(i_sdmcSelect_Value,SD_PORTSEL);    /* SD_PORTSEL���W�X�^�l���擾 */

    return i_sdmcSelect_Value;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcSelect

  Description:  select port.
                �|�[�g��I������

  Arguments:    select : [15:8]port numbers which supported�i�T�|�[�g����Ă���|�[�g���j
                         [7:0]port number which selected now�i�I������|�[�g�ԍ��j
  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcSelect(u16 select)
{
    union
    {
        u16    Val;
        struct
        {
            u8    L;        /* �w�肵�����|�[�g */
            u8    H;        /* �T�|�[�g����Ă���|�[�g */
        }    PSel;
    }    SDCARD_PSel;

    //TODO : transfer���[�h����stand-by���[�h�ɂ���?
/*    if( !SDCARD_EndFlag) {                      // �]�����c���Ă���ꍇ�̓G���[
        return SDMC_ERR_END;
    }
*/
    SDCARD_PSel.Val = select;
    if ((SDCARD_PSel.PSel.H > SDCARD_PORT_NO_MAX)
        | (SDCARD_PSel.PSel.H < SDCARD_PORT_NO_MIN)
        | (SDCARD_PSel.PSel.L > SDCARD_PORT_SELECT_NO))
    {
        return SDMC_ERR_PARAM;                    /* �R�}���h�p�����[�^�G���[        */
    }else{
        SD_port_en_numbers = SDCARD_PSel.PSel.H;  /* �V�����T�|�[�g����|�[�g����ۑ� */
        SD_port_number = SDCARD_PSel.PSel.L;      /* �V�����I������|�[�g�ԍ���ۑ� */

        SD_GetFPGA(SDCARD_PSel.Val,SD_PORTSEL);   /* ���݂̃��W�X�^���擾 */

        if (SDCARD_PSel.PSel.L == 0)              /* ���ݑI������Ă���|�[�g�� 0 ? */
        {
            SDCARD_Backup_port0();
        }else{
            SDCARD_Backup_port1();
        }

        SDCARD_PSel.Val= select;

        if (SDCARD_PSel.PSel.L == 0)              /* �V�����I�����ꂽ�|�[�g�� 0 ? */
        {
            SDCARD_Restore_port0();
        }else{
            SDCARD_Restore_port1();
        }

        SD_SetFPGA(SD_PORTSEL,select);            /* ���W�X�^�փZ�b�g    */
        return SDMC_NORMAL;                       /* 0�@���^�[�� */
    }
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Thread

  Description:  SD�������J�[�h���C���^�X�N

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Thread( void* arg)
{
    SDCARDMsg*    SdMsg;
#if (TARGET_OS_CTR == 1)
    u32           current_dat;
#else
    OSMessage     current_dat;
#endif
    SDMC_ERR_CODE api_result;

    while( TRUE) {
        /* ���b�Z�[�W�҂� */
        PRINTDEBUG( "rcv mes sdThread\n");
#if (TARGET_OS_CTR == 1)
        rcv_dtq( sdmc_dtq_id, (VP_INT*)&current_dat);
#else
        OS_ReceiveMessage( &sdmc_dtq, &current_dat, OS_MESSAGE_BLOCK);
#endif
        SdMsg = (SDCARDMsg*)current_dat;
        PRINTDEBUG( "sd task : receive command : %d\n", SdMsg->operation);

        switch( SdMsg->operation) {
          case SD_OPERATION_INIT:
            api_result = i_sdmcInit();
            break;
          case SD_OPERATION_READ:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_Read( SdMsg->buf, SdMsg->bufsize,
                                       SdMsg->offset, SdMsg->func,
                                       SdMsg->info);
            break;
          case SD_OPERATION_READ_WITH_FIFO:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_ReadFifo( SdMsg->buf, SdMsg->bufsize,
                                           SdMsg->offset, SdMsg->func,
                                           SdMsg->info);
            break;
          case SD_OPERATION_WRITE:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_Write( SdMsg->buf, SdMsg->bufsize,
                                        SdMsg->offset, SdMsg->func,
                                        SdMsg->info);
            break;
          case SD_OPERATION_WRITE_WITH_FIFO:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_WriteFifo( SdMsg->buf, SdMsg->bufsize,
                                            SdMsg->offset, SdMsg->func,
                                            SdMsg->info);
            break;
          default:
            PRINTDEBUG( "sdmc-thread error : undefined command.\n");
            api_result = SDMC_ERR_COMMAND;
            break;
        }

        PRINTDEBUG( "sd task : operation ends(result : 0x%x), send message begin\n", api_result);

        /*���b�Z�[�W�ԑ�*/
        current_dat = (OSMessage)api_result;
#if (TARGET_OS_CTR == 1)
        snd_dtq( sdmc_result_dtq_id, (VP_INT)api_result);
#else
        OS_SendMessage( &sdmc_result_dtq, current_dat, OS_MESSAGE_BLOCK);
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Intr_Thread

  Description:  SD�������J�[�h���荞�ݏ����^�X�N

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Intr_Thread( void* arg)
{
    u16        sd_info1;//, sd_info2;
    OSIntrMode enabled;

    while( 1) {
        PRINTDEBUG( "next_tsk:0x%x\n", OS_SelectThread());
        PRINTDEBUG( "Slp sdIntr\n");

        OS_WaitIrq( FALSE, OS_IE_SD1);
//      OS_DisableIrqMask( OS_IE_SD1);
//      if( sdmc_intr_wakeup_count == 0) {
//        OS_SleepThread( NULL);
//      }
//      sdmc_intr_wakeup_count--;
//      OS_EnableIrqMask( OS_IE_SD1);
//        enabled = OS_DisableInterrupts();
        (void)OS_ClearIrqCheckFlag( OS_IE_SD1);
        /*SD���荞�݂�IF����*/
        *(vu16*)CTR_INT_IF = CTR_IE_SD_MASK;
//        (void)OS_RestoreInterrupts( enabled);
        PRINTDEBUG( "sdIntr waked\n");

        /*--- FIFO���g���Ƃ� ---*/
        if( SDCARD_UseFifoFlag) {
            sd_info1 = SD_INFO1;
            if( ((*SDIF_CNT & SDIF_CNT_FULL)&&(*SDIF_CNT & SDIF_CNT_FFIE)) ||
                ((!(*SDIF_CNT & SDIF_CNT_NEMP))&&(*SDIF_CNT & SDIF_CNT_FEIE))) {

                PRINTDEBUG( ">>>SD Intr(FIFO)\n");// Full or Empty)\n");
                OS_DisableIrqMask( OS_IE_SD1);
                SDCARD_FPGA_irq();                        /*�J�[�h����̃��[�h���C�g�v�����荞��*/
                OS_EnableIrqMask( OS_IE_SD1);
                   /* FIFO���荞�݂�ALLEND���荞�݂��قړ����̏ꍇ�ɑΉ� */
                if( SD_CheckFPGAReg( sd_info1, SD_INFO1_ALL_END)) {
        (void)OS_ClearIrqCheckFlag( OS_IE_SD1);
        /*SD���荞�݂�IF����*/
        *(vu16*)CTR_INT_IF = CTR_IE_SD_MASK;
                    OS_DisableIrqMask( OS_IE_SD1);
                    SYSFPGA_irq();
                    OS_EnableIrqMask( OS_IE_SD1);
                    if( thread_flag) {
                        PRINTDEBUG( "--Wup sdThread!--\n");
                        sdmc_wakeup_count++;
                        PRINTDEBUG( "wakeup\n");
                        OS_WakeupThreadDirect( &sdmc_tsk);
                    }
                }
            }else{
                if( SD_CheckFPGAReg( SD_INFO2, (SD_INFO2_MASK_BRE | SD_INFO2_MASK_BWE))) {
                   PRINTDEBUG ( ">>>SD Intr(R/W Req)\n");
                    //�����Ŏ����I�Ƀ��b�p�[��FIFO<->SD_BUF0�ԂŒʐM���s����
    //                if((!(*SDIF_CNT & SDIF_CNT_NEMP))&&(*SDIF_CNT & SDIF_CNT_FEIE)) {
                    OS_DisableIrqMask( OS_IE_SD1);
                    SDCARD_FPGA_irq();
                    OS_EnableIrqMask( OS_IE_SD1);
                }else{
                    PRINTDEBUG( ">>>SD Intr(End or Err)\n");
                    OS_DisableIrqMask( OS_IE_SD1);
                    SYSFPGA_irq();                            /*�����܂��̓G���[���荞��*/
                    OS_EnableIrqMask( OS_IE_SD1);
                    /**/
                    if( thread_flag) {
                        PRINTDEBUG( "--Wup sdThread!--\n");
                        sdmc_wakeup_count++;
                        OS_WakeupThreadDirect( &sdmc_tsk);
                    }
                }
            }
        /*--- FIFO���g��Ȃ��Ƃ� ---*/
        }else{
            if( SD_CheckFPGAReg( SD_INFO2, (SD_INFO2_MASK_BRE | SD_INFO2_MASK_BWE))) {
                PRINTDEBUG( ">>>SD Intr(R/W Req)\n");
                OS_DisableIrqMask( OS_IE_SD1);
                SDCARD_FPGA_irq();                        /*�J�[�h����̃��[�h���C�g�v�����荞��*/
                OS_EnableIrqMask( OS_IE_SD1);
            }else{
                PRINTDEBUG( ">>SD Intr(End or Err)\n");
                OS_DisableIrqMask( OS_IE_SD1);
                SYSFPGA_irq();                            /*�����܂��̓G���[���荞��*/
                OS_EnableIrqMask( OS_IE_SD1);
                /**/
                if( thread_flag) {
                    PRINTDEBUG( "--Wup sdThread!--\n");
                    sdmc_wakeup_count++;
                    OS_WakeupThreadDirect( &sdmc_tsk);
                }
            }
        }
    }
}
