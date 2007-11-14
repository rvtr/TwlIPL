/*---------------------------------------------------------------------------*
  Project:  TwlFirm - GCD - include
  File:     gcd.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef FIRM_GCD_GCD_H_
#define FIRM_GCD_GCD_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <firm/memorymap.h>
#include <firm/mi.h>
#include <firm/gcd/blowfish.h>
#include <firm/format/wram_regs.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    GCD_PRIMARY_SLOT       = 0,
    GCD_SECONDARY_SLOT     = 1, 

    GCD_NO_SLOT            = -1
}
GCDSlot;


/*****************/

/* ROM�w�b�_�E�u�[�g�p�����[�^�\���� */
typedef struct {
  u8 *romAddr;              // ROM�A�h���X
  u8 *entryAddr;                // �G���g���A�h���X
  u8 *ramAddr;              // RAM�A�h���X
  s32 romSize;              // ROM�T�C�Y
} GCDBootUsrParam;

// �J�[�h�A�N�Z�X�E�R���g���[�����W�X�^�\����
typedef struct {
    u16 latency1:13;        // ���C�e���V�P�̃T�C�N����
    u16 dataScramble_on:1;  // �f�[�^�X�N�����u�� �n�m
    u16 scrambleUnit_on:1;  // �X�N�����u����H �n�m
    u16 initScramblePN:1;   // �X�N�����u���o�m�f�[�^������
    u16 latency2:6;         // ���C�e���V�Q�̃T�C�N����
    u16 cmdScramble_on:1;   // �R�}���h�X�N�����u�� �n�m
    u16 dataReady:1;        // �f�[�^ ���f�B
    u16 pages:3;            // �y�[�W��
    u16 clockType:1;        // �N���b�N�^�C�v�i�P�T�O�����^�Q�S�O�����j
    u16 clockInLatency:1;   // ���C�e���V���ԂɃN���b�N����
    u16 reset:1;            // ���Z�b�g�M��
    u16 accessMode:1;       // �A�N�Z�X���[�h
    u16 start:1;            // �X�^�[�g
} GCDCardCnt;

// �J�[�h�R���g���[���E�p�����[�^�\����
typedef struct {
  //    u32 dmaNo;              // DMA�ԍ�
  u32 cardCnt;            // �J�[�h�R���g���[��
  u32 op[2];              // �R�}���h
} GCDCardCtrlParam;


/* ROM�w�b�_�\���� */
typedef struct {
  s8  titleName[12];            // �\�t�g�^�C�g����
  u32 initialCode;          // �C�j�V�����R�[�h

  u16 makerCode;                // ���[�J�[�R�[�h
  u8  machineCode;          // �{�̃R�[�h
  u8  deviceType;               // �f�o�C�X�^�C�v

  u8  nonVerReserved[4];        // �\��i4�o�C�g�j
  u8  verDepReserved[4];        // �\��i�o�[�W�����ˑ��A4�o�C�g�j
  u16 wirelessSerialNo;
  u16 softVersion:8;            // �\�t�g�o�[�W����
  u16 compArm9BootArea:1;       // ARM9�u�[�g�G���A���k�t���O
  u16 compArm7BootArea:1;       // ARM7�u�[�g�G���A���k�t���O
  u16 :0;

  GCDBootUsrParam arm9;         // ARM9�u�[�g�̈�p�����[�^
  GCDBootUsrParam arm7;         // ARM7�u�[�g�̈�p�����[�^

  u32 fileSysReserved[8];       // �t�@�C���V�X�e���\��i32�o�C�g�j

  GCDCardCnt romCtrl4Game;      // ROM�R���g���[�����iNORMAL & GAME���[�h�j
  GCDCardCnt romCtrl4Secure;        //                    �iSECURE���[�h�j
  u8  romCtrlReserved_l[4]; // �\��
  u16 secureCRC16;          // �Z�L���A�̈�CRC16bit
  u16 romTimerLatency;      // �^�C�}�[���C�e���V�i(�T�C�N����/256) - 2�A3D�������p�j
  u8  romReserved_h[8];     // �\��
  u32 romNormalModeKey[2];  // NORMAL���[�h����L�[

  u8  reserved_80h[0x10];   // �\��̈�i16�o�C�g�j

  u16 normalAreaOffset;     // NORMAL�̈�
  u16 twlAreaOffset;        // TWL��p�̈�

  u8  reserved_94h[0x2c];   // �\��̈�i44�o�C�g�j

  u16 nintendoLogo[0x9c/2]; // NINTENDO���S�i156�o�C�g�j
  u16 ninLogoCRC16;         // NINTENDO���SCRC16
  u16 headerCRC16;          // �w�b�_CRC16

  u8 *dbgRomAddr;               // �f�o�b�K���j�^ROM�A�h���X
  s32 dbgRomSize;               // �f�o�b�K���j�^ROM�T�C�Y
  u8 *dbgArm9RamAddr;           // �f�o�b�K���j�^ARM9-RAM�A�h���X
  u8 *dbgArm7RamAddr;           // �f�o�b�K���j�^ARM7-RAM�A�h���X

  u8  reserved_170h[0x10];  // �\��̈�i16�o�C�g�j
}
GCDRomHeaderDS;

typedef struct {
  // DS�݊�
  GCDRomHeaderDS l;

  // TWL�g��
  MIHeader_WramRegs w;

  u8  reserved_1b0h[0x10];  // �\��̈�i16�o�C�g�j

  GCDBootUsrParam arm9ex;       // ARM9�g���u�[�g�̈�p�����[�^
  GCDBootUsrParam arm7ex;       // ARM7�g���u�[�g�̈�p�����[�^

  u8  reserved_1e0h[0x1000 - 0x1e0 - 0x200];  // �\��̈�

  u8  acsign_contents[0x180];   // �R���e���c�ؖ���

  u8  acsign_header[0x80];      // ROM�w�b�_�d�q����
}
GCDRomHeader;


/* �Z�L���A���[�N�\���� */
typedef struct {
  u8  recvRtcBuf[8];            // RTC�f�[�^��M�o�b�t�@�i�v4�o�C�g���E�j
#if 0
  u16 flashCrc16;               // �t���b�V��������CRC16
  u16 flashCount;               // �t���b�V�������� 8�o�C�g�J�E���g
  u32 flashBuf[2];          // �t���b�V����������M�o�b�t�@�i�v4�o�C�g���E�j
  u32 flashKeyBuf[3];           // �t���b�V���������L�[�E�e���|�����o�b�t�@
#endif
  u32 unScrambleKey[2];     // �X�N�����u�������L�[�i8�o�C�g�AMakeBlowfishTable() �ŎZ�o�j
  s16 isGenUnScrambleKey;       // �X�N�����u�������L�[���������iMakeBlowfishTable() �Őݒ�j

  u32 va;                       // ����24bit�i24bit���M�j
  u32 vb;                       // ����20bit�i+����`4bit�AVBI: 32bit���M�j
  u32 vc_dummy;             // ����16bit�i16bit���M�j
  u32 vd;                       // ����24bit�i24bit���M�j

  GCDCardCnt cardCntBak4Secure; // SECURE�R�}���h�p�����[�^�E�o�b�N�A�b�v
  s16 enableReadSecure;     // SECURE�̈惊�[�h�E�C�l�[�u���iLoadSecure4Card() �Őݒ�j
  u32 cardNormalModeKey[2]; // NORMAL���[�h����L�[�iMakeBlowfishTable() �ŎZ�o�j
  u32 cardKeyBuf[3];            // �J�[�h�L�[�E�e���|�����o�b�t�@
  s32 secureSize;               // SECURE�̈�T�C�Y
  s16 sequenceNo4Secure;        // �V�[�P���X�ԍ��iSECURE�p�j
  s16 segmentTblShift;      // �Z�O�����g�ԍ��e�[�u���V�t�g�l�iSECURE�p�j
  u8  *segmentTblp;         // �Z�O�����g�e�[�u���|�C���^�iSECURE�p�j
  s16 numSecureSegment;     // SECURE�Z�O�����g���iSECURE�p�j
  s16 segmentOffset;            // �Z�O�����g�I�t�Z�b�g�iSECURE�p�j
  s16 blockOffset;          // �u���b�N�I�t�Z�b�g�iSECURE�p�j

  //  CardCtrlParam paramBak;       // �J�[�h�p�����[�^ �Í����O�o�b�N�A�b�v�iSECURE�p�j

  BLOWFISH_CTX  blowfishCardTable;  // �J�[�h�pBlowfish�e�[�u��
  BLOWFISH_CTX  blowfishFlashTable;

  u8 bufEnd[4];             // �Z�L���A���[�N�ŏI�f�[�^
} GCDSecureWork;


/* ���L���[�N�\���� */
typedef struct {
  u32 nCardID;              // NORMAL�J�[�hID�iLoadCardHeader() �Ŏ擾�j
  u32 sCardID;              // SECURE�J�[�hID�iCardTimerIntr4Secure() �Ŏ擾�j
  u16 cardHeaderCrc16;      // �J�[�h�w�b�_CRC16�iLoadCardHeader() �ŎZ�o�j
  u16 cardSecureCrc16;      // �J�[�hSECURE�̈�CRC16�iLoadSecure4Card() �ŎZ�o�j
  s16 cardHeaderError;      // �J�[�h�w�b�_�G���[�iCheckCardHeader() �Őݒ�j
  s16 disableEncryptedCardData;// �J�[�hSECURE�̈�Í����f�[�^�����iDecryptObjectFile() �Őݒ�j
  s16 cardSequenceNo;           // �J�[�h�ǂݍ��݃V�[�P���X�ԍ�
  s16 enableCardNormalOnly; // �J�[�hNORMAL���[�h�̂ݗL���iLoadFlashDemo() �Őݒ�j

  //  s16 isOnDebugger;         // �f�o�b�K��œ��쒆��
  //  s16 rtcError;             // RTC�G���[

  u32 recvRtc[2];               // RTC�X�e�[�^�X1���^�C�}�[�f�[�^�i�v4�o�C�g���E�j

} GCDSharedWork;


typedef union
{
    u64 dw;
    u8  b[8];
}
GCDCmd64;

typedef struct
{
    u32 ctrl;
    s32 cmdcount;
    u32 latency;
    u32 scramble;
    u8  master;
    u8  spi;
    GCDSlot  slot;
}
GCDCtrlRegs;

typedef struct
{
    GCDCmd64 gcdOp;
    u32     dmaNo;
    u32     lastDmaNo;
    BOOL    ltckReq;
    GCDCtrlRegs *gcdRegs;
    GCDCtrlRegs nLoadTableRegs;
    GCDCtrlRegs nSendOnlyOpRegs;
    GCDCtrlRegs sSendOnlyOpRegs;
    GCDCtrlRegs nIDRegs;
    GCDCtrlRegs sIDRegs;
    GCDCtrlRegs gIDRegs;
    GCDCtrlRegs nReadShortHeaderRegs;
    GCDCtrlRegs nReadRomRegs;
    GCDCtrlRegs sReadRomRegs;
    GCDCtrlRegs gReadRomRegs;
    GCDCtrlRegs gWriteRomRegs;
}
GCDRomCtrls;

typedef void (* GCDRomFuncp)( u32 romp, void *ramp, s32 size );
typedef void (* GCDAsyncCoreFuncp)( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls );


typedef struct
{
    u32  romp;                      // ROM offset
    u8   *ramp;                     // RAM address
    s32  restSize;                  // rest size
    s32  oneShotSize;               // one shot size
    vs8  intrDone;
    s8   isAsync;
    volatile BOOL isPxiDone;
    GCDRomCtrls          ctrls;
    GCDAsyncCoreFuncp    funcp;
}
GCDWork;


#define GCD_DEFAULT_DMA_A_NO          7
#define GCD_DEFAULT_DMA_B_NO          6

#define GCD_CHATTERING_CYCLES         0x200U

#define GCD_ROM_PAGE_SIZE             0x200  // 512B
#define GCD_SECURE_SEGMENT_SIZE       0x1000 // 4KB
#define GCD_LOAD_TABLE_SIZE           0x2000 // 8KB

#define	GCD_NML_AREA_ALIGN            0x80000   // 512KB
#define	GCD_TWL_AREA_ALIGN            0x80000   // 512KB
#define GCD_SECURE2_AREA_SIZE         0x4000    // 16KB
#define	GCD_SECURE2_AREA_OFFSET       0x3000    // 12KB
#define	GCD_GAME2_AREA_OFFSET         0x7000    // 28KB


// ROM area

#define GCD_LOAD_TABLE_SIZE           0x2000 // 8KB
#define GCD_DS_ROM_HEADER_SIZE        0x200  // 512B
#define GCD_ROM_HEADER_SIZE           0x1000 // 4KB
#define GCD_SECURE_AREA_SIZE          0x4000 // 16KB

#define GCD_SECURE_AREA_OFFSET        0x4000
#define GCD_GAME_AREA_OFFSET          0x8000


// ROM command

#define GCDOP_BIT_NUM                 64

// NORMAL mode

#define GCDOP_N_OP_MASK               0xff00000000000000ULL
#define GCDOP_N_OP_SIZE               8

#define GCDOP_N_OP_RD_ROM_ID          0x9000000000000000ULL
#define GCDOP_N_OP_RD_PAGE            0x0000000000000000ULL
#define GCDOP_N_OP_WR_PAGE            0x8000000000000000ULL
#define GCDOP_N_OP_LD_TABLE           0x9f00000000000000ULL
#define GCDOP_N_OP_CHG_MODE           0x3c00000000000000ULL
#define GCDOP_N_OP_CHG2_MODE          0x3d00000000000000ULL

#define GCDOP_N_RD_ROM_ID_PAD         0x00ffffffffffffffULL
#define GCDOP_N_RD_PAGE_PAD           0x00fffffeff00ffffULL
#define GCDOP_N_CHG_MODE_PAD          0x00000000f00000ffULL
#define GCDOP_N_CHG2_MODE_PAD         GCDOP_N_CHG_MODE_PAD

#define GCDOP_N_RD_PAGE_ADDR_SHIFT    33
#define GCDOP_N_RD_PAGE_ADDR_SIZE     23
#define GCDOP_N_RD_PAGE_ADDR_MASK     0x00fffffe00000000ULL

#define GCDOP_N_VAE_SHIFT             32
#define GCDOP_N_VAE_SIZE              24
#define GCDOP_N_VAE_MASK              0x00ffffff00000000ULL

#define GCDOP_N_VBI_SHIFT             8
#define GCDOP_N_VBI_SIZE              20
#define GCDOP_N_VBI_MASK              0x000000000fffff00ULL


// SECURE mode

#define GCDOP_S_OP_MASK               0xf000000000000000ULL
#define GCDOP_S_OP_SIZE               4

#define GCDOP_S_OP_RD_ROM_ID          0x1000000000000000ULL
#define GCDOP_S_OP_RD_SEGMENT         0x2000000000000000ULL
#define GCDOP_S_OP_PNG_ON             0x4000000000000000ULL
#define GCDOP_S_OP_PNG_OFF            0x6000000000000000ULL
#define GCDOP_S_OP_CHG_MODE           0xa000000000000000ULL

#define GCDOP_S_VA_SHIFT              GCDOP_S_VB_SIZE
#define GCDOP_S_VA_SIZE               24
#define GCDOP_S_VA_MASK               0x00000ffffff00000ULL

#define GCDOP_S_VB_SHIFT              0
#define GCDOP_S_VB_SIZE               20
#define GCDOP_S_VB_MASK               0x00000000000fffffULL

#define GCDOP_S_VC_SHIFT              (GCDOP_S_VA_SIZE + GCDOP_S_VB_SIZE)
#define GCDOP_S_VC_SIZE               16
#define GCDOP_S_VC_MASK               0x0ffff00000000000ULL

#define GCDOP_S_VD_SHIFT              GCDOP_S_VA_SHIFT
#define GCDOP_S_VD_SIZE               GCDOP_S_VA_SIZE
#define GCDOP_S_VD_MASK               GCDOP_S_VA_MASK


// GAME mode

#define GCDOP_G_OP_MASK               0xff00000000000000ULL
#define GCDOP_G_OP_SIZE               8

#define GCDOP_G_OP_RD_ROM_ID          0xb800000000000000ULL
#define GCDOP_G_OP_RD_ROM_UID         0xb900000000000000ULL
#define GCDOP_G_OP_RD_PAGE            0xb700000000000000ULL
#define GCDOP_G_OP_RD_CACHE_START     0x5800000000000000ULL
#define GCDOP_G_OP_RD_CACHE           0x6000000000000000ULL
#define GCDOP_G_OP_RD_CACHE_LAST      0x6800000000000000ULL
#define GCDOP_G_OP_WR_PAGE            0x8000000000000000ULL

#define GCDOP_G_RD_ROM_ID_PAD         0x00ffffffffffffffULL
#define GCDOP_G_RD_ROM_UID_PAD        0x00ffffffffffffffULL
#define GCDOP_G_RD_PAGE_PAD           0x00f0000000ffffffULL

#define GCDOP_G_RD_PAGE_ADDR_SHIFT    33
#define GCDOP_G_RD_PAGE_ADDR_SIZE     23
#define GCDOP_G_RD_PAGE_ADDR_MASK     0x000ffffe00000000ULL


// ROM ID

#define GCD_ROMID_1TROM_MASK          0x80000000UL
#define GCD_ROMID_TWLROM_MASK         0x40000000UL
#define GCD_ROMID_BADBLK_MASK         0x20000000UL
#define GCD_ROMID_SIZE_MASK           0x0000ff00UL



/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadRomID

  Description:  read rom ID
                sync version

  Arguments:    None

  Returns:      rom ID
 *---------------------------------------------------------------------------*/
u32 GCDi_ReadRomID( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadRom

  Description:  read rom data
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadRomAsync

  Description:  read rom data
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadNormalModeRom

  Description:  read rom data on normal mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadNormalModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCD_ReadNormalModeRomAsync

  Description:  read rom data on normal mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ReadNormalModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadGameModeRom

  Description:  read rom data on game mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadGameModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_ReadGameModeRomAsync

  Description:  read rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_ReadGameModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_WriteGameModeRom

  Description:  write rom data on game mode
                sync version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WriteGameModeRom( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_WriteGameModeRomAsync

  Description:  write rom data on game mode
                async version

  Arguments:    romp  : rom offset
                ramp  : ram destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WriteGameModeRomAsync( GCDSlot slot, u32 romp, void *ramp, s32 size );

/*---------------------------------------------------------------------------*
  Name:         GCDi_Enable

  Description:  Enable game card master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Enable( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_Disable

  Description:  disable game card master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Disable( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCD_SetPrimarySlot

  Description:  set primary game card slot

  Arguments:    primary slot

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_SetPrimarySlot( u8 slot );

/*---------------------------------------------------------------------------*
  Name:         GCD_GetPrimarySlot

  Description:  get primary game card slot

  Arguments:    None

  Returns:      primary slot
 *---------------------------------------------------------------------------*/
u8 GCD_GetPrimarySlot(void );

/*---------------------------------------------------------------------------*
  Name:         GCD_Reset

  Description:  reset game card

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_Reset( GCDSlot slot, u32 chat_cycles );

/*---------------------------------------------------------------------------*
  Name:         GCD_Reset

  Description:  reset game card

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCD_ResetAll( u32 chat_cycles );

/*---------------------------------------------------------------------------*
  Name:         GCDi_SelectRom

  Description:  select game card spi

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SelectRom( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_SelectSpi

  Description:  select game card spi

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SelectSpi( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_SetDmaNo

  Description:  set card dma number

  Arguments:    dmaNo : dma channel number

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_SetDmaNo( GCDSlot slot, u32 dmaNo );

/*---------------------------------------------------------------------------*
  Name:         GCDi_GetDmaNo

  Description:  get card dma number

  Arguments:    None

  Returns:      dmaNo : dma channel number
 *---------------------------------------------------------------------------*/
u32 GCDi_GetDmaNo( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_InterruptHandler

  Description:  interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_InterruptHandlerPRIME( void );
void GCDi_InterruptHandlerSECOND( void );
void GCDi_InterruptHandlerCommon( GCDSlot slot );

//================================================================================
//       WAIT/STOP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         GCDi_IsBusy

  Description:  check whether game card is busy or not

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL GCDi_IsBusy( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_IsDataReady

  Description:  check whether data is ready or not

  Arguments:    None

  Returns:      TRUE if game card is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL GCDi_IsDataReady( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_Stop

  Description:  stop game card access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_Stop( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitCtrl

  Description:  wait while game card is busy

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitCtrl( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitData

  Description:  wait until data is ready

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitData( GCDSlot slot );

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitDma

  Description:  wait for stopping game card DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitDma( GCDSlot slot, u32 dmaNo );

/*---------------------------------------------------------------------------*
  Name:         GCDi_WaitInterrupt

  Description:  wait for game card interrupt

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GCDi_WaitInterrupt( GCDSlot slot );


// internal

BOOL GCDi_IsSecureInitialized( GCDSlot slot );
BOOL GCDi_IsCtrApp( GCDSlot slot );
BOOL GCDi_IsDsApp( GCDSlot slot );

u32 GCDi_ReadNormalModeID( GCDSlot slot );
u32 GCDi_ReadSecureModeID( GCDSlot slot );
u32 GCDi_ReadGameModeID( GCDSlot slot );

u32 GCDi_ReadRomIDCore( GCDSlot slot, GCDRomCtrls *ctrls );
void GCDi_ReadRomCore( GCDSlot slot, void *ramp, s32 size, GCDRomCtrls *ctrls );
BOOL GCDi_ReadRomCommonPreCore( GCDSlot slot, u32 romp, void *ram, s32 size );
void GCDi_WriteRomCore( GCDSlot slot, void *ramp, s32 size, GCDRomCtrls *ctrls );

void GCDi_ReadNormalModeRomCore( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls );

void GCDi_InitSecureParam( GCDSlot slot, BOOL twl_ex );
void GCDi_InitPngIntf( GCDSlot slot );
void GCDi_SetInitCardOpBlowfish( void (*p)( void ) );
void GCDi_SetDecryptObjectFilep( void (*p)( void ) );
void GCDi_SetSetPngIntf( void (*p)( u32 pnA_l, u8 pnA_h, u32 pnB_l, u8 pnB_h ) );
void GCDi_SetReadSecureModeIDCorep( u32 (p)( GCDRomCtrls *ctrls ) );
void GCDi_SetReadSecureModeRomCorep( void (p)( u32 romp, void *ramp, s32 size, GCDRomCtrls *ctrls ) );
void GCDi_SetGetOneShotSizeFromCtrlp( s32 (p)( u32 ctrl ) );

void GCDi_SetSendOnlyCardOpCorep( void (*p)( GCDRomCtrls *ctrls ) );
void GCDi_ChangeIntoSecureMode( GCDSlot slot );
void GCDi_ChangeIntoSecure2Mode( GCDSlot slot );
void GCDi_SendPngON( GCDSlot slot );
void GCDi_SendPngOFF( GCDSlot slot );
void GCDi_ChangeIntoGameMode( GCDSlot slot );
void GCDi_ReadCardSegmentAsync4Secure( GCDSlot slot, BOOL twl_ex );

void GCDi_InitCardOpBlowfish( GCDSlot slot );
void GCDi_InitCardOpBlowfishDS( GCDSlot slot );
void GCDi_DecryptObjectFile( GCDSlot slot );
void GCDi_DecryptObjectFileDS( GCDSlot slot );

void GCD_SetInterrupt( GCDSlot slot, u32 romp, void *ramp, s32 size, GCDWork *wp );

void* GCDi_GetRomHeaderAddr( GCDSlot slot );


extern u32 GCDi_HeaderBuf[2][ GCD_ROM_HEADER_SIZE/sizeof(u32) ];
extern u32 GCDi_SecureAreaBuf[2][ GCD_SECURE_AREA_SIZE/sizeof(u32) ];
extern u32 GCDi_Secure2AreaBuf[2][ GCD_SECURE_AREA_SIZE/sizeof(u32) ];

extern GCDSharedWork GCDi_SharedWork[2];
extern GCDWork  GCDi_Work[2];

// �������e�[�u��
extern const BLOWFISH_CTX  GCDi_BlowfishInitTableDS;
extern const BLOWFISH_CTX  GCDi_BlowfishInitTableGCDFIRM;


// ��SecondarySlot�̃J�[�h���b�N���p�ӂ���Ă��Ȃ��̂ŁA�Ƃ肠����PrimarySlot�Ƌ��p�ŁB
s32 OS_LockExCard( u16 lockID );
s32 OS_UnlockExCard( u16 lockID );

#ifdef __cplusplus
} /* extern "C" */

#endif

/* FIRM_GCD_GCD_H_ */
#endif
