/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - init - ARM7.TWL
  File:     crt0.LTD.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-28#$
  $Rev: 2690 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#include    <nitro/types.h>
#include    <nitro/hw/common/armArch.h>
#include	<nitro/os/common/emulator.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap_sp.h>
#include    <twl/hw/ARM7/mmap_wramEnv.h>
#include    "boot_sync.h"

/*---------------------------------------------------------------------------*/
void    _start(void);
void    _start_AutoloadDoneCallback(void* argv[]);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

/* �O���֐��Q�ƒ�` */
extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);

/* �����֐��v���g�^�C�v��` */
static void INITi_SetHMACSHA1ToAppParam(void);
static void INITi_DoAutoload(void);
static void INITi_ShelterLtdBinary(void);
static void detect_main_memory_size(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32* ptr);
static void INITi_CallStaticInitializers(void);
#endif

/* �����J�X�N���v�g�ɂ���`�����V���{���Q�� */
extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);

void* const _start_ModuleParams[]   =
{
    (void*)SDK_AUTOLOAD_LIST,
    (void*)SDK_AUTOLOAD_LIST_END,
    (void*)SDK_AUTOLOAD_START,
    (void*)SDK_STATIC_BSS_START,
    (void*)SDK_STATIC_BSS_END,
    (void*)0,       // CompressedStaticEnd. This fixed number will be updated by compstatic tool.
    (void*)0,       // SDK_VERSION_ID   // SDK version info /* [TODO] �r���h��ʂ����� */
    (void*)SDK_NITROCODE_BE,
    (void*)SDK_NITROCODE_LE,
};

extern void SDK_LTDAUTOLOAD_LIST(void);
extern void SDK_LTDAUTOLOAD_LIST_END(void);
extern void SDK_LTDAUTOLOAD_START(void);

void* const _start_LtdModuleParams[]    =
{
    (void*)SDK_LTDAUTOLOAD_LIST,
    (void*)SDK_LTDAUTOLOAD_LIST_END,
    (void*)SDK_LTDAUTOLOAD_START,
    (void*)0,       // CompressedLtdautoloadEnd. This fixed number will be updated by compstatic tool.
    (void*)SDK_TWLCODE_BE,
    (void*)SDK_TWLCODE_LE,
};

#ifdef	DEBUG_WRAM_SETTING
u32 const wramMapping[12] = {
	0x8d898581, 0x8c888480, 0x9c989490, 0x8c888480,
	0x9c989490, 0x00000000, 0x09403900, 0x09803940,
	0x080037c0, 0x09403900, 0x09803940, 0x0000000f
};
#endif

#define	DIGEST_HASH_BLOCK_SIZE_SHA1					(512/8)
static const u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = 
{
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24,
};

/* [TODO] 
 * ���� TWL �̃}���`�u�[�g���[�_�[���ǂ��ɏ����c���Ă���邩����̂��߁A
 * �r���h��ʂ����߂����̂ł���߂Ȓ�`�ł��B*/
#define TWLIPL_PARAM_DOWNLOAD_PARAMETER     0x02ffb000

/*---------------------------------------------------------------------------*
  Name:         ShakeHand
  Description:  ARM9 �� ShakeHand �֐��Ɠ��������B
                ���C���������łȂ���������ԂŎ��s�����K�v������B
  Arguments:    r0  -   ARM9 �����p�ϐ��ւ̃|�C���^�B
                r1  -   ARM7 �����p�ϐ��ւ̃|�C���^�B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static const u32    microcode_ShakeHand[10] =
{
    0xe1d020b0, /*      ldrh    r2, [r0]    ; �����ϐ��X�����L����������ǂ� */
    0xe1d130b0, /*      ldrh    r3, [r1]    ; �����ϐ��V�����L����������ǂ� */
    0xe2833001, /*  @1: add     r3, r3, #1  ; �����ϐ��V ++ */
    0xe1c130b0, /*      strh    r3, [r1]    ; �����ϐ��V�����L�������ɏ��� */
    0xe1d0c0b0, /*      ldrh    r12, [r0]   ; �����ϐ��X�̌�������L����������ǂ� */
    0xe152000c, /*      cmp     r2, r12     ; �����ϐ��X�̕ω��𔻒肷�� */
    0x0afffffa, /*      beq     @1          ; �ω����Ă��Ȃ���΃��[�v */
    0xe2833001, /*      add     r3, r3, #1  ; �����ϐ��V ++ */
    0xe1c130b0, /*      strh    r3, [r1]    ; �����ϐ��V�����L�������ɏ��� */
    0xe12fff1e  /*      bx      lr          ; �n���h�V�F�C�N���� */
};

#ifdef	DEBUG_WRAM_SETTING
/*---------------------------------------------------------------------------*
  Name:         WaitAgreement
  Description:  ARM9 �̋N���x�N�^������̏�ԂɂȂ�܂ő҂B
                ���C���������łȂ���������ԂŎ��s�����K�v������B
  Arguments:    r0  -   �����p�t�F�[�Y�Ǘ��ϐ��ւ̃|�C���^�B
                r1  -   �ҋ@����t�F�[�Y�ԍ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static const u32    microcode_WaitAgreement[7]  =
{
    0xe1d020b0, /*  @1: ldrh    r2, [r0]    ; �t�F�[�Y�Ǘ��ϐ������L����������ǂ� */
    0xe1510002, /*      cmp     r1, r2      ; �ҋ@�t�F�[�Y�ԍ��Ɣ�r */
    0x012fff1e, /*      bxeq    lr          ; ��v���Ă���Αҋ@���� */
    0xe3a03010, /*      mov     r3, #16     ; �󃋁[�v�񐔏����� */
    0xe2533001, /*  @2: subs    r3, r3, #1  ; �󃋁[�v�� -- */
    0x1afffffd, /*      bne     @2          ; 16 �񃋁[�v */
    0xeafffff8  /*      b       @1          ; �擪�ɖ߂� */
};
#endif

/*---------------------------------------------------------------------------*
  Name:         GotoMain
  Description:  ARM9 �ɓ���̏�ԂɂȂ������Ƃ�`������ŁAMain �֐��փW�����v
                ����B���C���������łȂ���������ԂŎ��s�����K�v������B
  Arguments:    r0  -   Main �֐��̃|�C���^�BMain �֐��� Thumb �R�[�h�ł���ꍇ
                        �ɂ́Abx ���߂ŃW�����v���邽�ߍŉ��ʃr�b�g�� 1 �ɂȂ���
                        ����K�v������B
                r1  -   �����p�t�F�[�Y�Ǘ��ϐ��ւ̃|�C���^�B
                r2  -   ARM9 �ɓ`����t�F�[�Y�ԍ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static const u32    microcode_GotoMain[2]   =
{
    0xe1c120b0, /*      strh    r2, [r1]    ; �t�F�[�Y�Ǘ��ϐ����X�V */
    0xe12fff10  /*      bx  r0              ; Main �֐��փW�����v */
};

/*---------------------------------------------------------------------------*/
#include    <twl/code32.h>

/*---------------------------------------------------------------------------*
  Name:         _start
  Description:  �N���x�N�^�B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start(void)
{
@000:
        /* ���荞�݋֎~ */
        mov             r12, #HW_REG_BASE
        str             r12, [r12, #REG_IME_OFFSET]     // Use that LSB of HW_REG_BASE is 0b0

        /* �n���h�V�F�C�N�p�}�C�N���R�[�h���p WRAM �ɃR�s�[ */
        ldr             r1, =microcode_ShakeHand
        ldr             r2, =HW_PRV_WRAM
        add             r3, r2, #40
@001:   ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @001

        /* ��p WRAM ��̃R�[�h�� ARM9 �ƃn���h�V�F�C�N1 */
        ldr             r0, =HW_BOOT_SHAKEHAND_9
        ldr             r1, =HW_BOOT_SHAKEHAND_7
        ldr             r2, =HW_PRV_WRAM
        mov             lr, pc
        bx              r2

		// ���[�h���ꂽ�A�v���̃_�C�W�F�X�g���v�Z���ăA�v���ԃp�����[�^�Ɋi�[
        bl              INITi_SetHMACSHA1ToAppParam
        
@010:
        /* �X�^�b�N�|�C���^�ݒ� */
        mov             r0, #HW_PSR_SVC_MODE        // SuperVisor mode
        msr             cpsr_c, r0
        ldr             sp, =HW_PRV_WRAM_SVC_STACK_END
        mov             r0, #HW_PSR_IRQ_MODE        // IRQ mode
        msr             cpsr_c, r0
        ldr             sp, =HW_PRV_WRAM_IRQ_STACK_END
        ldr             r1, =SDK_IRQ_STACKSIZE
        sub             r1, sp, r1
        sub             sp, sp, #4                  // 4 bytes for stack check code
        mov             r0, #HW_PSR_SYS_MODE
        msr             cpsr_csfx, r0
        sub             sp, r1, #4                  // 4 bytes for stack check code

        /* �X�^�b�N�̈���N���A */
        ldr             r0, =SDK_SYS_STACKSIZE
        sub             r1, r1, r0
        ldr             r2, =HW_PRV_WRAM_IRQ_STACK_END
        mov             r0, #0
@011:   cmp             r1, r2
        strlt           r0, [r1], #4
        blt             @011
        
        /* TWL �n�[�h�E�F�A��œ��삵�Ă��邩�ǂ����𒲍� */
        ldr             r1, =REG_CLK_ADDR
        ldrh            r0, [r1]
        tst             r0, #REG_SCFG_CLK_WRAMHCLK_MASK
        /* TWL �n�[�h�E�F�A��œ��삵�Ă��Ȃ��ꍇ�͉i�v���[�v */
@012:   beq             @012

        /* �_�E�����[�h�p�����[�^�������L�G���A�ɑޔ� */
        ldr             r1, =HW_DOWNLOAD_PARAMETER
        ldr             r2, =TWLIPL_PARAM_DOWNLOAD_PARAMETER
        add             r3, r2, #HW_DOWNLOAD_PARAMETER_SIZE
@013:   ldr             r0, [r2], #4
        str             r0, [r1], #4
        cmp             r2, r3
        blt             @013

        /* �������̑���M�@�\�𓮍삳����ݒ���s�� */
        ldr             r1, =REG_WL_ADDR
        ldrh            r0, [r1]
        orr             r0, r0, #REG_SCFG_WL_OFFB_MASK
        strh            r0, [r1]
        /* ARM7 ���� DMA �ɂ͐V DMA ��H���̗p���A�T�E���h�ɂ��V DMA ��H�𗘗p */
        ldr             r1, =REG_EXT_ADDR
        ldr             r0, [r1]
        orr             r0, r0, #REG_SCFG_EXT_DMAC_MASK
        orr             r0, r0, #REG_SCFG_EXT_DSEL_MASK
        str             r0, [r1]
        /* [TODO] ARM7 ���ł����ݒ�ł��Ȃ��ǉ� I/O ���W�X�^�̏����ݒ���s�� */
        

        /* Autoload �����{ */
        bl              INITi_DoAutoload
        
@020:
        /* STATIC �u���b�N�� .bss �Z�N�V������ 0 �N���A */
        ldr             r0, =_start_ModuleParams
        ldr             r1, [r0, #12]       // SDK_STATIC_BSS_START
        ldr             r2, [r0, #16]       // SDK_STATIC_BSS_END
        mov             r0, #0
@021:   cmp             r1, r2
        strlt           r0, [r1], #4
        blt             @021

        //---- detect main memory size
        bl              detect_main_memory_size
		
        /* ���荞�݃x�N�^�ݒ� */
        ldr             r1, =HW_INTR_VECTOR_BUF
        ldr             r0, =OS_IrqHandler
        str             r0, [r1]

#ifndef SDK_NOINIT
        /* c++ �p������ */
        bl              _fp_init
        bl              TwlSpStartUp
        bl              __call_static_initializers
        bl              INITi_CallStaticInitializers
#endif

@030:
        /* Main �֐��ւ̃W�����v�p�}�C�N���R�[�h���X�^�b�N�̒�ɃR�s�[ */
        ldr             r1, =microcode_GotoMain
        ldr             r0, [r1], #4
        str             r0, [sp, #-4]
        ldr             r0, [r1]
        str             r0, [sp]

        /* �}�C�N���R�[�h���o�R���� Main �֐��փW�����v */
        ldr             r0, =TwlSpMain
        ldr             r1, =HW_BOOT_SYNC_PHASE
        mov             r2, #BOOT_SYNC_PHASE_4
        ldr             lr, =HW_RESET_VECTOR
        sub             r3, sp, #4
        bx              r3
}

#include <nitro/mi/stream.h>
#include <twl/os/common/systemCall.h>
#include <nitro/mi.h>
/*---------------------------------------------------------------------------*
  Name:         INITi_SetHMACSHA1ToAppParam
  Description:  ROM�����[�h���ꂽ�e�A�v���̈��HMACSHA1���v�Z���A����̃A�h��
                �X�ɕۑ�
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
#define UNDEF_CODE			0xe7ffdeff
#define ENCRYPT_DEF_SIZE	0x800
#define DGT_TGT_ADDR		( HW_MAIN_MEM + 0x0200 )

static void INITi_SetHMACSHA1ToAppParam(void)
{
	u32 *arm9_flx_addr = (u32 *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028));
	u32 *p_arm9encryObjVerify = (u32 *)(DGT_TGT_ADDR + 4 * 32);
	int l;
	SVCHMACSHA1Context *pCon = ( SVCHMACSHA1Context * ) 0x037c0000;
	
	// arm9_flx
	*p_arm9encryObjVerify = TRUE;
	for( l=0; l<ENCRYPT_DEF_SIZE/4; l++ )
	{
		if(arm9_flx_addr[l] != UNDEF_CODE)
		{
			if((u32)p_arm9encryObjVerify < 0x2000400)
			{
				*p_arm9encryObjVerify = arm9_flx_addr[l];
				p_arm9encryObjVerify++;
			}
		}
	}
	MI_CpuClear8( (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028)), ENCRYPT_DEF_SIZE);// �܊pMI�g����̂ŁA4�o�C�g���E�ō���Ȃ��悤��8��
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x02c)));
	SVC_HMACSHA1GetHash(pCon, (void *)DGT_TGT_ADDR);
	// arm7_flx
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x038)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x03c)));
	SVC_HMACSHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 32));
	// arm9_ltd
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1c8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1cc)));
	SVC_HMACSHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 2 * 32));
	// arm7_ltd
	SVC_HMACSHA1Init(pCon, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1);
	SVC_HMACSHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1d8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1dc)));
	SVC_HMACSHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 3 * 32));

/*
	SVCSHA1Context *pCon = ( SVCSHA1Context * ) 0x037c0000;
	// arm9_flx
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)( (*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x028)) + 0x800 ), ( *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x02c)) - 0x800 ) );
	SVC_SHA1GetHash(pCon, (void *)DGT_TGT_ADDR);
	// arm7_flx
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x038)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x03c)));
	SVC_SHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 32));
	// arm9_ltd
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1c8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1cc)));
	SVC_SHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 64));
	// arm7_ltd
	SVC_SHA1Init(pCon);
	SVC_SHA1Update(pCon, (void *)(*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1d8)), *((u32 *)(HW_TWL_ROM_HEADER_BUF + 0x1dc)));
	SVC_SHA1GetHash(pCon, (void *)(DGT_TGT_ADDR + 96));
*/
}
/*
static asm void INITi_SetHMACSHA1ToAppParam(void)
{
        mov             r0, #HW_MAIN_MEM
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        add             r0, r0, #SVC_SHA1_DIGEST_SIZE
        ldr				r1, =s_digestDefaultKey
        mov				r2, #(DIGEST_HASH_BLOCK_SIZE_SHA1)
        bl				SVC_HMACSHA1Init
        
        mov             r1, #HW_WRAM_AREA
        sub             r3, r1, 0x2000
        ldr				r1, [r3, 0x028]
        ldr				r2, [r3, 0x02c]
        bl				SVC_HMACSHA1Update
        
        mov             r1, #HW_MAIN_MEM
        bl				SVC_HMACSHA1GetHash
        
        bx				lr
}
*/


/*---------------------------------------------------------------------------*
  Name:         INITi_DoAutoload
  Description:  �����N���ɉ����āA�e�I�[�g���[�h�u���b�N�̌Œ�f�[�^���̓W�J
                �y�ѕϐ����� 0 �N���A���s���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
/*
 * < ��i�K�I�[�g���[�h >
 * 0x02f88000 �� crt0 �y�ш�i�ڃ��[�h���o�C�i�����z�u����Ă���B
 *  NITRO �Ƌ��L�\�� WRAM ��ɔz�u�����ׂ��o�C�i���f�[�^�� 0x037c0000 �Ƀ��[�h����B
 *  TWL �ł������삵�Ȃ� WRAM ��ɔz�u�����ׂ��o�C�i���f�[�^�𑱂��̃A�h���X�Ƀ��[�h����B
 * 0x02e80000 �ɓ�i�ڃ��[�h���o�C�i�����z�u����Ă���B
 *  0x04000 �o�C�g���̓J�[�h ROM ����ēǂݏo���s�Ȃ̂ŁA0x02f84000 - 0x02f88000 �ɑޔ�����B
 *  NITRO �Ƌ��L�\�� MAIN ��ɔz�u�����ׂ��o�C�i���f�[�^�� 0x02f88000 + sizeof(crt0) �Ƀ��[�h����B
 *  TWL �ł������삵�Ȃ� MAIN ��ɔz�u�����ׂ��o�C�i���f�[�^�𑱂��̃A�h���X�Ƀ��[�h����B
 */
static asm void
INITi_DoAutoload(void)
{
@000:
        stmdb           sp!, {lr}
        /* WRAM �p�u���b�N���I�[�g���[�h */
        ldr             r1, =_start_ModuleParams
        ldr             r12, [r1]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_AUTOLOAD_START
@001:   cmp             r12, r0
        bge             @010
        /* �Œ�Z�N�V���������[�h */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@002:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @002
        /* static initializer �e�[�u������ǂݏo�� */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           sp!, {r0-r3, r12}
#endif
        /* .bss �Z�N�V������ 0 �N���A */
        mov             r0, #0
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@003:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @003
@004:   ldmia           sp!, {r0}
        b               @001

@010:   /* ���C���������p�u���b�N�̑��݂��m�F */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1dc  /* ARM7 �p�g���풓���W���[�� ROM �T�C�Y */
        ldr             r0, [r1]
        cmp             r0, #0
        beq             @020

        /* �ēǂݏo���s������ޔ� */
        bl              INITi_ShelterLtdBinary

        /* ���C���������p�u���b�N���I�[�g���[�h */
        ldr             r1, =_start_LtdModuleParams
        ldr             r12, [r1]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r0, [r1, #4]        // r0 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r1, #8]        // r1 = SDK_LTDAUTOLOAD_START
@011:   cmp             r12, r0
        bge             @020
        /* �Œ�Z�N�V���������[�h */
        stmdb           sp!, {r0}
        ldr             r2, [r12], #4       // r2 = start address of destination range
        ldr             r3, [r12], #4       // r3 = size of fixed section
        add             r3, r3, r2          // r3 = end address of destination range of fixed section
@012:   cmp             r2, r3
        ldrlt           r0, [r1], #4
        strlt           r0, [r2], #4
        blt             @012
        /* static initializer �e�[�u������ǂݏo�� */
        ldr             r0, [r12], #4       // r0 = address of the table managing pointers of static initializers
#ifndef SDK_NOINIT
        stmdb           sp!, {r0-r3, r12}
        bl              INITi_ShelterStaticInitializer
        ldmia           ap!, {r0-r3, r12}
#endif
        /* .bss �Z�N�V������ 0 �N���A */
        mov             r0, #0
        ldr             r3, [r12], #4       // r3 = size of .bss section
        add             r3, r3, r2          // r3 = end address of destination range of .bss section
@013:   cmp             r2, r3
        strlt           r0, [r2], #4
        blt             @013
@014:   ldmia           sp!, {r0}
        b               @011

@020:   /* �I�[�g���[�h�����R�[���o�b�N�֐��Ăяo�� */
        ldr             r0, =_start_ModuleParams
        ldr             r1, =_start_LtdModuleParams
        ldmia           sp!, {lr}
        b               _start_AutoloadDoneCallback
}

/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterLtdBinary
  Description:  TWL ��p�̃I�[�g���[�h���o�C�i���f�[�^�̓��A�J�[�h ROM ����
                �ēǂݏo���ł��Ȃ��̈�̃f�[�^��ޔ��G���A�ɑޔ�����B
                �ēǂݏo���ł��Ȃ��̈�̃f�[�^�� ARM7 �p�� ARM9 �p�̊g���풓
                ���W���[���̂Q�ɕ�����Ă���\��������̂ŁA�璷�ł͂��邪
                �����̐擪���� 0x4000 �������ꂼ��ޔ�����B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_ShelterLtdBinary(void)
{
        /* �ޔ����E��A�h���X�𒲍� */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1d8  /* ARM7 �p�g���풓���W���[�� RAM �A�h���X */
        ldr             r1, [r1]
        ldr             r3, =HW_TWL_ROM_HEADER_BUF + 0x038  /* ARM7 �p�풓���W���[�� RAM �A�h���X */
        ldr             r3, [r3]
        sub             r2, r3, #0x4000                     /* �ēǂݏo���s�̈�T�C�Y */ /* ARM7 �p�ޔ��G���A */

        /* �R�s�[ */
@loop:  ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @loop

        bx              lr
}

#ifndef SDK_NOINIT
/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterStaticInitializer
  Description:  �e�I�[�g���[�h�Z�O�����g���� static initializer �ւ̃|�C���^
                �e�[�u���� IRQ �X�^�b�N�̍ŏ㕔�ɑޔ�����B
  Arguments:    ptr     -   �Z�O�����g���̃|�C���^�e�[�u���ւ̃|�C���^�B
                            �e�[�u���� NULL �ŏI�[����Ă���K�v������B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_ShelterStaticInitializer(u32* ptr)
{
        /* �����m�F */
        cmp             r0, #0
        bxeq            lr

        /* �ޔ��ꏊ�擪�A�h���X���v�Z */
        ldr             r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr             r2, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* �ޔ��ꏊ�擪����󂫏ꏊ�𒲍� */
@001:   ldr             r2, [r1]
        cmp             r2, #0
        addne           r1, r1, #4
        bne             @001

        /* �󂫏ꏊ�Ƀe�[�u�����R�s�[ */
@002:   ldr             r2, [r0], #4
        str             r2, [r1], #4
        cmp             r2, #0
        bne             @002

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CallStaticInitializers
  Description:  �e�I�[�g���[�h�Z�O�����g���� static initializer ���Ăяo���B
                �I�[�g���[�h�����ɂ���� IRQ �X�^�b�N�̍ŏ㕔�ɑޔ�����Ă���
                �֐��|�C���^�e�[�u��������Ăяo���B
  Arguments:    �Ȃ��B
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static asm void
INITi_CallStaticInitializers(void)
{
        stmdb           sp!, {lr}

        /* �e�[�u���ޔ��ꏊ�擪�A�h���X���v�Z */
        ldr             r1, =HW_PRV_WRAM_IRQ_STACK_END
        ldr             r2, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* �e�[�u���ɊǗ�����Ă���|�C���^������Ăяo�� */
@001:   ldr             r0, [r1]
        cmp             r0, #0
        beq             @002
        stmdb           sp!, {r1}
        mov             lr, pc
        bx              r0
        ldmia           sp!, {r1}
        /* ��U�Ăяo�����|�C���^�̓[���N���A (IRQ�X�^�b�N���Ԏ؂肵�Ă����) */
        mov             r0, #0
        str             r0, [r1], #4
        b               @001

@002:   ldmia           sp!, {lr}
        bx              lr
}
#endif

/*---------------------------------------------------------------------------*
  Name:         _start_AutoloadDoneCallback
  Description:  �I�[�g���[�h�����R�[���o�b�N�B
  Arguments:    argv    -   �I�[�g���[�h�p�����[�^��ێ����Ă���z��B
                    argv[0] =   SDK_AUTOLOAD_LIST
                    argv[1] =   SDK_AUTOLOAD_LIST_END
                    argv[2] =   SDK_AUTOLOAD_START
                    argv[3] =   SDK_STATIC_BSS_START
                    argv[4] =   SDK_STATIC_BSS_END
  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start_AutoloadDoneCallback(void* argv[])
{
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         detect_main_memory_size

  Description:  detect main memory size.
                result is written into (u32*)HW_MMEMCHECKER_SUB.
                value is [OS_CONSOLE_SIZE_4MB|OS_CONSOLE_SIZE_8MB|
				OS_CONSOLE_SIZE_16MB|OS_CONSOLE_SIZE_32MB]

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define OSi_IMAGE_DIFFERENCE  0x400000
#define OSi_IMAGE_DIFFERENCE2 0xb000000
#define OSi_DETECT_NITRO_MASK  (REG_SND_SMX_CNT_E_MASK | REG_SND_SMX_CNT_FSEL_MASK)
#define OSi_DETECT_NITRO_VAL   (REG_SND_SMX_CNT_E_MASK)

static asm void detect_main_memory_size( void )
{
//################ temp: this process will be done in IPL
	    // SCFG enable?
	    ldr     r2, =REG_EXT_ADDR
		ldr     r0, [r2]
		tst     r0, #0x80000000
		beq     @9

        ldr     r2, =HW_PRV_WRAM_SYSRV
		//OPT(bonding option)
        ldr     r3, =REG_OP_ADDR
        ldrh    r0, [r3]
		strh    r0, [r2, #8]
		//OPT(JTAG info)
		ldr  	r3, =REG_JTAG_ADDR
		ldrb	r0, [r3]
		//CLK(only wram clock)
		ldr     r3, =REG_CLK_ADDR
		ldrh    r1, [r3]
		and     r1, r1, #0x80
		orr     r0, r0, r1, LSR 1
		strb	r0, [r2, #9]
@9:
//################

	    //---- copy scfg setting
		ldr     r2, =HW_PRV_WRAM_SYSRV
		ldr     r3, =HW_SYS_CONF_BUF
		ldr     r0, [r2, #HWi_WSYS04_WRAMOFFSET]
		str     r0, [r3, #HWi_WSYS04_OFFSET]
		ldrh    r0, [r2, #HWi_WSYS08_WRAMOFFSET]
		strh    r0, [r3, #HWi_WSYS08_OFFSET]

		//---- detect memory size
        mov     r0, #OS_CONSOLE_SIZE_4MB
        mov     r1, #0

        ldr     r2, =HW_MMEMCHECKER_SUB
        sub     r3, r2, #OSi_IMAGE_DIFFERENCE
@1:
        strh    r1, [r2]
        ldrh    r12, [r3]
        cmp     r1, r12
        bne     @2

        add     r1, r1, #1
        cmp     r1, #2 // check 2 loop
        bne     @1

		//---- 4MB
		b       @4

		//---- 8MB or 16MB or 32MB
@2:
		// check if running on twl/nitro
		ldr     r1, =HW_SYS_CONF_BUF
		ldrb    r12, [r1,#HWi_WSYS09_OFFSET]
		tst		r12, #HWi_WSYS09_CLK_WRAMHCLK_MASK
        moveq   r0, #OS_CONSOLE_SIZE_8MB
		beq		@4

		//---- 16MB or 32MB
        mov     r1, #0
        add     r3, r2, #OSi_IMAGE_DIFFERENCE2
@3:
        strh    r1, [r2]
        ldrh    r12, [r3]
        cmp     r1, r12

        movne   r0, #OS_CONSOLE_SIZE_32MB
        bne     @4

        add     r1, r1, #1
        cmp     r1, #2 // check 2 loop
        bne     @3
        mov     r0, #OS_CONSOLE_SIZE_16MB
@4:
		//---- check SMX_CNT
		ldr     r3, =REG_SMX_CNT_ADDR
		ldrh    r1, [r3]
		and     r1, r1, #OSi_DETECT_NITRO_MASK
		cmp     r1, #OSi_DETECT_NITRO_VAL
		orreq   r0, r0, #OS_CHIPTYPE_SMX_MASK

        strb    r0, [r2]
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         NitroSpStartUp
  Description:  Hook for user start up.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void
NitroSpStartUp(void)
{
}

#include    <twl/codereset.h>
