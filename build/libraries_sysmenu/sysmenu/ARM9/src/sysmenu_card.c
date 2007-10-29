/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_card.c

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

#include <sysmenu/sysmenu_lib/ARM9/sysmenu_api.h>
#include "sysmenu_card.h"


typedef enum {
	CARD_NOT_DETECTING = 0,
	CARD_DETECTING
} CARDPullStatus;


typedef struct SYSMCardPullWork
{
    CARDPullStatus  pullStatus;
    u32     		cardCnt;
    u16     		lockID;
	u16				detectPullOut;

} SYSMCardPullWork;


static SYSMCardPullWork  cpw;


// �����֐�
void SYSMi_InitCardPulledOut(void);
BOOL SYSMi_IsDetectableCardPulledOut(void);
void SYSMi_StartReadCardID(void);
void SYSMi_ReadCardPage(void *romp, void *ramp, s32 size);
static void SYSMi_SetCardOp(const u32 *op);


//----------------------------------------------------------------------
//     					�J�[�h�������o������
//----------------------------------------------------------------------

void SYSMi_InitCardPulledOut(void)
{
	cpw.lockID = (u16 )OS_GetLockID();
	cpw.cardCnt = *(vu32 *)MROMCNT_GAME_BUF;
}


//----------------------------------------------------------------------
//     					�J�[�h�������o
//----------------------------------------------------------------------

BOOL SYSM_IsCardPulledOut(void)
{
	if (SYSMi_IsDetectableCardPulledOut() == FALSE)	return FALSE;

	switch (cpw.pullStatus) {
		case CARD_NOT_DETECTING:
				if (OS_TryLockCard( cpw.lockID ) == OS_LOCK_SUCCESS )
				{
					SYSMi_StartReadCardID();
					cpw.pullStatus = CARD_DETECTING;
				}
				break;

		case CARD_DETECTING:
				if (SYSMi_IsCardDataReady()) {
					u32 cardID = *(vu32 *)REG_CARD_DATA;
					if (cardID != GetSYSMWork()->nCardID
					  && SYSM_IsNITROCard())
					{
						cpw.detectPullOut = TRUE;
					}

					(void)OS_UnlockCard( cpw.lockID );
					cpw.pullStatus = CARD_NOT_DETECTING;
				}
				break;
	}

	return  cpw.detectPullOut;
}


//----------------------------------------------------------------------
//     					�J�[�h�������o�I������
//----------------------------------------------------------------------

void SYSM_FinalizeCardPulledOut(void)
{
	while (SYSM_IsDetectingCardPulledOut()) {
		(void)SYSM_IsCardPulledOut();
	}
}


//----------------------------------------------------------------------
//     					�J�[�h�������o��������
//----------------------------------------------------------------------

BOOL SYSM_IsDetectingCardPulledOut(void)
{
	return  (cpw.pullStatus == CARD_DETECTING);
}


//----------------------------------------------------------------------
//     				�J�[�h�������o�̏������ł��Ă��邩
//----------------------------------------------------------------------

BOOL SYSMi_IsDetectableCardPulledOut(void)
{
	return  (SYSM_GetBootFlag() & BFLG_LOAD_CARD_COMPLETED ) ? TRUE : FALSE;
}


//----------------------------------------------------------------------
//     					�J�[�hID�ǂݍ��݊J�n
//----------------------------------------------------------------------

void SYSMi_StartReadCardID(void)
{
	u32 op[2];

	op[0] = 0;													// �R�}���h�ݒ�
	op[1] = MROMOP_G_READ_ID;

	SYSMi_SetCardOp( op );
																// �R���g���[���ݒ� 
	*(vu32 *)REG_CARDCNT = cpw.cardCnt
						| CARD_READ_MODE | CARD_STATUS
						| CARD_START | CARD_RESET_HI;
}


//----------------------------------------------------------------------
//     					�J�[�hID�ǂݍ���
//----------------------------------------------------------------------

u32 SYSMi_ReadCardID(void)
{
	u32 op[2];

	op[0] = 0;													// �R�}���h�ݒ�
	op[1] = MROMOP_G_READ_ID;

	SYSMi_SetCardOp( op );
																// �R���g���[���ݒ� 
	*(vu32 *)REG_CARDCNT = *(vu32 *)MROMCNT_GAME_BUF
						| CARD_READ_MODE | CARD_STATUS
						| CARD_START | CARD_RESET_HI;

	SYSMi_WaitCardData();

	return  *(vu32 *)REG_CARD_DATA;
}


//----------------------------------------------------------------------
//     					�J�[�h�f�[�^�����ǂݍ���
//----------------------------------------------------------------------

void SYSM_ReadCard(void *romp, void *ramp, s32 size)
{
	s32  restSize = size;
	s32  blockSize = MROM_PAGE_SIZE;

	while (restSize > 0) {										// �u���b�N�����ǂݍ���
		if (restSize >= MROM_PAGE_SIZE) {	size = MROM_PAGE_SIZE;
		} else {							size = restSize;
		}

		SYSMi_ReadCardPage(romp, ramp, size);

		(u8 *)romp += size;
		(u8 *)ramp += size;
		restSize   -= size;
	}
}


//----------------------------------------------------------------------
//     					�y�[�W�ǂݍ���
//----------------------------------------------------------------------

void SYSMi_ReadCardPage(void *romp, void *ramp, s32 size)
{
	{	u32 op[2];

		op[0] = (u32 )romp <<24;								// �R�}���h�ݒ�
		op[1] = MROMOP_G_READ_PAGE | (u32 )romp >>8;

		SYSMi_SetCardOp( op );
	}

	// �y�[�W���[�h
	{	void *ramEndp;
		u32 cardCntTmp;

		*(vu32 *)REG_CARDCNT = *(vu32 *)MROMCNT_GAME_BUF		// �R���g���[���ݒ� 
							| CARD_READ_MODE | CARD_1_PAGE
							| CARD_START | CARD_RESET_HI;

		ramEndp = (u8 *)ramp + size;							// �i�[�I���A�h���X�Z�o

		do {													// CPU�ǂݍ���
			cardCntTmp = *(vu32 *)REG_CARDCNT;

			if (cardCntTmp & CARD_DATA_READY) {
				u32 dataTmp = *(vu32 *)REG_CARD_DATA;

				if (ramp < ramEndp)	
					*((vu32 *)ramp) = dataTmp;					// �w��T�C�Y�܂Ŋi�[�i�㑱�f�[�^�͓ǂݎ̂āj
				((vu32 *)ramp)++;
			}
		} while (cardCntTmp & CARD_START);
	}
}


//----------------------------------------------------------------------
//     					�R�}���h�ݒ�
//----------------------------------------------------------------------

static void SYSMi_SetCardOp(const u32 *op)
{
	int  i;

	*(vu8 *)REG_CARD_MASTER_CNT = CARDMST_SEL_ROM					// �}�X�^�[�C�l�[�u��
								| CARDMST_ENABLE;

	for (i=0; i<2; i++) {											// �R�}���h�ݒ�
		u32	opTmp = op[1 - i];
		vu8 *opDestBasep = (vu8 *)(REG_CARD_CMD + i*4);

		opDestBasep[0] = (u8 )(opTmp >>24);
		opDestBasep[1] = (u8 )(opTmp >>16);
		opDestBasep[2] = (u8 )(opTmp >>8);
		opDestBasep[3] = (u8 )(opTmp >>0);
	}
}

