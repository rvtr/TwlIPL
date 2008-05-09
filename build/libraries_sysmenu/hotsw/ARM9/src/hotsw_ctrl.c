/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     hotsw_ctrl.c

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
#include <twl.h>
#include <sysmenu.h>


// ===========================================================================
// 	Function Describe
// ===========================================================================

/*---------------------------------------------------------------------------*
  Name:         HOTSW_EnableHotSWAsync
  
  Description:  PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm
 *---------------------------------------------------------------------------*/
void HOTSW_EnableHotSWAsync( BOOL enable )
{
	HotSwPxiMessage msg;
    
    enable = enable ? 1 : 0;
    
	// ���݂̒l�Ɠ����Ȃ牽���������^�[��
	if( SYSMi_GetWork()->flags.hotsw.isEnableHotSW == enable ) {
		return;
	}
	
	msg.msg.value = enable;
	msg.msg.ctrl  = TRUE;

	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
	{
		// do nothing
	}
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_FinalizeHotSW
  
  Description:  PXI�ʐM��ARM7�Ɋ����}��Finalize������ʒm
 *---------------------------------------------------------------------------*/
void HOTSW_FinalizeHotSWAsync( HotSwApliType apliType )
{
	HotSwPxiMessage msg;

    msg.msg.finalize = TRUE;
    msg.msg.bootType = (u8)apliType;

	while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    {
    	// do nothing
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isEnableHotSW
  
  Description:  �����}���̋���/�}���̏�Ԃ�Ԃ�
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isEnableHotSW(void)
{
    return SYSMi_GetWork()->flags.hotsw.isEnableHotSW ? TRUE : FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_isCardLoadCompleted
  
  Description:  �J�[�h�A�v���̃��[�h���������Ă��邩��Ԃ�
 *---------------------------------------------------------------------------*/
BOOL HOTSW_isCardLoadCompleted(void)
{
    return SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted ? TRUE : FALSE;
}