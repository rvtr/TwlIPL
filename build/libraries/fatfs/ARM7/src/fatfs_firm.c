/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fatfs
  File:     fatfs_firm.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <symbols.h>

#include <firm.h>
#include <rtfs.h>
#include <devices/sdif_reg.h>
#include <devices/sdif_ip.h>
#include <devices/sdmc_config.h>
#include <twl/devices/sdmc/ARM7/sdmc.h>

extern u32 NAND_FAT_PARTITION_COUNT;

#define DMA_PIPE         2
#define DMA_RECV         3

/*
    ��pSD�֐�
*/
extern volatile SDMC_ERR_CODE   SDCARD_ErrStatus;
extern s16  SDCARD_SDHCFlag;          /* SDHC�J�[�h�t���O */
extern SDPortContext*   SDNandContext;  /* NAND�������p�����[�^ */

/*---------------------------------------------------------------------------*
  Name:         WaitFifoFull

  Description:  waiting to fill the SD FIFO

                SD�J�[�h����̓ǂݍ��݃f�[�^��FIFO����ǂݍ��߂��ԂɂȂ�܂�
                �X�g�[�����܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void WaitFifoFull( void )
{
    while( (*SDIF_CNT & SDIF_CNT_FULL) == 0 )
    {
        if ( SDCARD_ErrStatus != SDMC_NORMAL )   // an error was occurred
        {
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StartToRead

  Description:  start to read from SD I/F

                SD�J�[�h����̓ǂݍ��݂̊J�n�������s���܂��B

  Arguments:    block       begining sector to transfer
                count       number of setctors to transfer

  Returns:      None
 *---------------------------------------------------------------------------*/
static void StartToRead(u32 block, u32 count)
{
    *SDIF_FSC = count;
    *SDIF_FDS = SECTOR_SIZE;
    *SDIF_CNT = (*SDIF_CNT & ~(SDIF_CNT_FEIE | SDIF_CNT_FFIE)) | SDIF_CNT_FCLR | SDIF_CNT_USEFIFO;
    CC_EXT_MODE = CC_EXT_MODE_DMA;

    SDCARD_ErrStatus = SDMC_NORMAL;
    SD_EnableClock();
    SD_EnableSeccnt(count);
    if ( SDCARD_SDHCFlag )
    {
        SD_MultiReadBlock( block );
    }
    else
    {
        SD_MultiReadBlock( block * SECTOR_SIZE );
    }
}

/*---------------------------------------------------------------------------*
  Name:         StopToRead

  Description:  stop to read from SD I/F

                SD�J�[�h����̓ǂݍ��݂̊����������s���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void StopToRead( void )
{
    if( !SD_CheckFPGAReg( SD_STOP,SD_STOP_SEC_ENABLE ) ){
        SD_StopTransmission();      /* �J�[�h�]���I����FPGA�ɒʒm�iCMD12���s�j */
    }
    SD_TransEndFPGA();              /* �]���I������(���荞�݃}�X�N���֎~�ɖ߂�) */
    SD_DisableClock();              /* �N���b�N������~ */

    *SDIF_CNT = (*SDIF_CNT & ~SDIF_CNT_USEFIFO) | SDIF_CNT_FCLR; /* FIFO�g�p�t���OOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;  /* PIO���[�h(DMA���[�hOFF) */
}

/*
    FATFS-SDMC�̊Ԃ�AES��g�ݍ���
    �ꕔ�̐ݒ�́AFATFS���I�񂵂Đݒ肷�邱�ƂɂȂ�B
*/

#define AES_GET_CNT_BITS(regValue, name)                        \
    ((regValue) & (REG_AES_AES_CNT_##name##_MASK))

static BOOL useAES = FALSE;
static AESCounter   aesCounter;

/*---------------------------------------------------------------------------*
  Name:         FATFS_EnableAES

  Description:  enable AES data path

                ���ɓǂݍ��ރf�[�^��AES�Í�������Ă��邱�Ƃ��A�O������
                IO�֐��ɒʒm���邽�߂�API�ł��B

                ����API���Ăяo��������ɓǂݍ��ރf�[�^��AES�̏����l���w��
                ���Ă����΁A�Ȍ�̃V�[�P���V�����ȌĂяo���ł̏����l�͎���
                �v�Z����܂��B
                �����_���A�N�Z�X���s���ꍇ�́A���̂��тɂ���API���Ăяo��
                �K�v������܂��B

  Arguments:    counter     initial counter value

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_EnableAES( const AESCounter* pCounter )
{
    useAES = TRUE;
    aesCounter = *pCounter;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_DisableAES

  Description:  bypass AES

                ���ɓǂݍ��ރf�[�^��AES�Í�������Ă��Ȃ����Ƃ��A�O������
                IO�֐��ɒʒm���邽�߂�API�ł��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_DisableAES( void )
{
    useAES = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ReadNormal

  Description:  normal read

                ���ʂ�NAND/SD�J�[�h��ǂݍ��݂܂��B

                ���荞�݋֎~��Ԃō����ɓ��삷��d�l�ɂȂ��Ă��܂��B

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
static u16 ReadNormal(u32 block, void *dest, u16 count)
{
    MINDmaConfig config =
    {
        MI_NDMA_NO_INTERVAL,
        MI_NDMA_INTERVAL_PS_1,
        MI_NDMA_BWORD_128,
        SECTOR_SIZE/4
    };
//    OS_TPrintf("ReadNormal(0x%X, 0x%08X, 0x%X) is calling.\n", block, dest, count);
    MI_NDmaRecvExAsync_Dev( DMA_PIPE, SDIF_FI, dest, (u32)(count * SECTOR_SIZE), NULL, NULL, &config, MI_NDMA_TIMING_SD_1 );
    StartToRead( block, count );
    MI_WaitNDma( DMA_PIPE );
    StopToRead();

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         ReadAES

  Description:  AES read

                AES�������Ȃ���NAND/SD�J�[�h��ǂݍ��݂܂��B
                AES�̌��̐ݒ�͂��炩���ߍs���Ă����K�v������܂��B
                AES�̏����l�̐ݒ�́AFATFS_EnableAES�̈�������v�Z����܂��B

                ���荞�݋֎~��Ԃō����ɓ��삷��d�l�ɂȂ��Ă��܂��B

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
#define PIPE_SIZE   64
static u16 ReadAES(u32 block, void *dest, u16 count)
{
    u32 offset = 0; // in bytes

//    OS_TPrintf("ReadAES(0x%X, 0x%08X, 0x%X) is calling.\n", block, dest, count);
    MI_NDmaPipeAsync_SetUp( DMA_PIPE, (void*)SDIF_FI, (void*)REG_AES_IFIFO_ADDR, PIPE_SIZE, NULL, NULL );

/*
    AES�̃Z�b�g�A�b�v���o��DMA�ݒ�
*/
    AESi_Reset();
    AESi_Reset();
    AESi_DmaRecv( DMA_RECV, dest, (u32)(count * SECTOR_SIZE), NULL, NULL );
    AESi_SetCounter( &aesCounter );
    AESi_Run( AES_MODE_CTR, 0, (u32)(count * SECTOR_SIZE / AES_BLOCK_SIZE), NULL, NULL );

    // update for next read
    AESi_AddCounter( &aesCounter, (u32)(count * SECTOR_SIZE / AES_BLOCK_SIZE) );

    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        goto err;
    }

    while ( count * SECTOR_SIZE > offset )
    {
        while ( AES_GET_CNT_BITS( reg_AES_AES_CNT, IFIFO_CNT ) )
        {
        }
        if ( (offset & (SECTOR_SIZE-1)) == 0 )
        {
            WaitFifoFull();
            if ( SDCARD_ErrStatus != SDMC_NORMAL )
            {
                goto err;
            }
        }
        MI_NDmaRestart( DMA_PIPE );
        offset += PIPE_SIZE;
    }
    MI_WaitNDma( DMA_PIPE );
    StopToRead();
    MI_WaitNDma( DMA_RECV );
    return SDCARD_ErrStatus;

err:
    MI_StopNDma( DMA_RECV );
    MI_StopNDma( DMA_PIPE );
    StopToRead();
    AESi_Reset();
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsIoFirm

  Description:  ��ʑw����̃Z�N�^���[�h�^���C�g�v�����󂯂�

                Read�ɑ΂��Ă̂݁A�Ǝ��̊֐����g�p����h���C�o��IO�֐��ł��B
                AES�̗L���̔���͒f�Љ����ɖ����؂ł��B
                (�Ԃɘ_���̈�̓ǂݍ��݂����܂�\�������邩��)

  Arguments:    driveno : �h���C�u�ԍ�
                block : �J�n�u���b�N�ԍ�
                buffer :
                count : �u���b�N��
                reading : ���[�h�v������TRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
static BOOL nandRtfsIoFirm( int driveno, u32 block, void* buffer, u16 count, BOOL reading )
{
    u16 result;
#pragma unused( driveno)

    sdmcSelect( (u16)SDMC_PORT_NAND);

    if( reading )
    {
        result = useAES ?
                ReadAES( block, buffer, count ) :
                ReadNormal( block, buffer, count );
    }
    else
    {
        SdmcResultInfo SdResult;
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult );
    }

    return result ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsIoFirm

  Description:  ��ʑw����̃Z�N�^���[�h�^���C�g�v�����󂯂�

                Read�ɑ΂��Ă̂݁A�Ǝ��̊֐����g�p����h���C�o��IO�֐��ł��B

  Arguments:    driveno : �h���C�u�ԍ�
                block : �J�n�u���b�N�ԍ�
                buffer :
                count : �u���b�N��
                reading : ���[�h�v������TRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
static BOOL sdmcRtfsIoFirm( int driveno, u32 block, void* buffer, u16 count, BOOL reading )
{
    u16 result;
#pragma unused( driveno)

    sdmcSelect( (u16)SDMC_PORT_CARD );

    if( reading )
    {
        result = useAES ?
                ReadAES( block, buffer, count ) :
                ReadNormal( block, buffer, count );
    }
    else
    {
        SdmcResultInfo SdResult;
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult );
    }

    return result ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsAttachFirm

  Description:  sdmc�h���C�o���h���C�u�Ɋ��蓖�Ă�

                �Ǝ���IO�֐����g�p����悤�ɏ��������܂��B

  Arguments:    driveno : �h���C�u�ԍ�

  Returns:
 *---------------------------------------------------------------------------*/
#define nandRtfsCtrl                    FATFSi_nandRtfsCtrl
extern int nandRtfsCtrl( int driveno, int opcode, void* pargs);
static BOOL nandRtfsAttachFirm( int driveno, int partition_no)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    pdr.dev_table_drive_io     = nandRtfsIoFirm;
    pdr.dev_table_perform_device_ioctl = nandRtfsCtrl;
    pdr.register_file_address  = (dword) 0;    /* Not used */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = (DRIVE_FLAGS_VALID | DRIVE_FLAGS_PARTITIONED);//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = partition_no; /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    switch( partition_no )
    {
    case 0:
        result = rtfs_attach( driveno, &pdr, "SD1p0" ); //�\���̂�FS���C�u�������ɃR�s�[�����
        break;
    case 1:
        result = rtfs_attach( driveno, &pdr, "SD1p1" ); //�\���̂�FS���C�u�������ɃR�s�[�����
        break;
    case 2:
        result = rtfs_attach( driveno, &pdr, "SD1p2" ); //�\���̂�FS���C�u�������ɃR�s�[�����
        break;
    case 3:
        result = rtfs_attach( driveno, &pdr, "SD1p3" ); //�\���̂�FS���C�u�������ɃR�s�[�����
        break;
    default:
        result = FALSE;
        break;
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsAttachFirm

  Description:  sdmc�h���C�o���h���C�u�Ɋ��蓖�Ă�

                �Ǝ���IO�֐����g�p����悤�ɏ��������܂��B

  Arguments:    driveno : �h���C�u�ԍ�

  Returns:
 *---------------------------------------------------------------------------*/
#define sdmcRtfsCtrl                    FATFSi_sdmcRtfsCtrl
extern int sdmcRtfsCtrl( int driveno, int opcode, void* pargs);
static BOOL sdmcRtfsAttachFirm( int driveno)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    pdr.dev_table_drive_io     = sdmcRtfsIoFirm;
    pdr.dev_table_perform_device_ioctl = sdmcRtfsCtrl;
    pdr.register_file_address  = (dword) 0; /* Not used  */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = 0;//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = 0;            /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    result = rtfs_attach( driveno, &pdr, "SD0" );    //�\���̂�FS���C�u�������ɃR�s�[�����

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_InitFIRM

  Description:  init file system

                FATFS�����������܂��B
                �W����FATFS_Init�̎g�p���Ȃ��������ȗ����Ă��邾���ł��B
                �ȑO��NAND�R���e�L�X�g���c���Ă���Ȃ�A���������Ԃ𒘂���
                �Z�k�ł��܂��B
                FATFS�p�̃������q�[�v(OS_ARENA_MAIN_SUBPRIV�̊֘A�ƃq�[�v)��
                ���炩���ߊm�ۂ��Ă����Ă��������B

  Arguments:    nandContext     context of nand driver's previous life

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_InitFIRM( void* nandContext )
{
    /* RTFS���C�u������������ */
    if( !FATFSi_rtfs_init() )
    {
        return FALSE;
    }

    /* NAND�������p�����[�^�̐ݒ� */
    SDNandContext = (SDPortContext*)nandContext;

    /* SD�h���C�o������ */
    if ( FATFSi_sdmcInit( SDMC_NOUSE_DMA ) != SDMC_NORMAL )
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountDriveFIRM

  Description:  mount specified partition

                �w�肵���f�o�C�X�̎w�肵���p�[�e�B�V�������w�肵���h���C�u��
                �}�E���g���܂��BNAND�܂���SD�J�[�h�̂ݑΉ����Ă��܂��B
                ����API�Ń}�E���g�����ꍇ�A�Ǝ���IO�֐����g�p����悤�ɂȂ�܂��B

  Arguments:    driveno         drive number "A:" is 0
                media           media type
                partition_no    pertition number

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountDriveFIRM( int driveno, FATFSMediaType media, int partition_no )
{
    if ( media == FATFS_MEDIA_TYPE_NAND )
    {
        // CAUTION!: �����֐���2��Ăяo�����R�ɂ��ėv�m�F�B
        if ( !nandRtfsAttachFirm( driveno, partition_no ) || nandRtfsAttachFirm( driveno, partition_no ) )
        {
            return FALSE;
        }
    }
    else
    {
        if ( partition_no )   // support only 0
        {
            return FALSE;
        }
        // CAUTION!: �����֐���2��Ăяo�����R�ɂ��ėv�m�F�B
        if ( !sdmcRtfsAttachFirm( driveno ) || sdmcRtfsAttachFirm( driveno ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}
