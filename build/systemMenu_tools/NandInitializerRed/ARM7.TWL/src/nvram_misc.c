/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     nvram_misc.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include    "nvram.h"
#include    "spi_sp.h"
#include    "nvram_sp.h"

/*---------------------------------------------------------------------------*
  Name:         NvramCheckReadyToRead

  Description:  NVRAM��ǂݏo���\�ȏ�Ԃ��ǂ����m�F����B

  Arguments:    None.

  Returns:      BOOL    -   �ǂݏo���\�ȏ�Ԃ̏ꍇ��TRUE��Ԃ��B
                            FALSE�̏ꍇ�͓ǂݏo���֎~�̏�ԁB
 *---------------------------------------------------------------------------*/
static BOOL NvramCheckReadyToRead(void)
{
    u16     tempStatus;

    // �X�e�[�^�X���W�X�^�ǂݏo��
    NVRAM_ReadStatusRegister((u8 *)(&tempStatus));
    // �������݂������͏������쒆�����m�F
    if (tempStatus & NVRAM_STATUS_REGISTER_WIP)
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NvramCheckReadyToWrite

  Description:  NVRAM���������݉\�ȏ�Ԃ��ǂ����m�F����B

  Arguments:    None.

  Returns:      BOOL    -   �������݉\�ȏ�Ԃ̏ꍇ��TRUE��Ԃ��B
                            FALSE�̏ꍇ�͏������݋֎~�̏�ԁB
 *---------------------------------------------------------------------------*/
static BOOL NvramCheckReadyToWrite(void)
{
    u16     tempStatus;

    // �X�e�[�^�X���W�X�^�ǂݏo��
    NVRAM_ReadStatusRegister((u8 *)(&tempStatus));
    // �������݂������͏������쒆�����m�F
    if (tempStatus & NVRAM_STATUS_REGISTER_WIP)
    {
        return FALSE;
    }
    // �������݋�����Ă��邩���m�F
    if (!(tempStatus & NVRAM_STATUS_REGISTER_WEL))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_WaitOperation

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NVRAM_WaitOperation(void)
{
  while( NvramCheckReadyToRead() == FALSE ) {
  }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_WaitWriteEnable

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NVRAM_WaitWriteEnable(void)
{
  while( NvramCheckReadyToWrite() == FALSE ) {
  }
}


/*---------------------------------------------------------------------------*
  Name:         NVRAMi_Read

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NVRAMi_Read(u32 address, void *buf, u32 size)
{
  NVRAM_WaitOperation();
  NVRAM_ReadDataBytes(address, size, buf);
  return;
}


/*---------------------------------------------------------------------------*
  Name:         NVRAMi_Write

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NVRAMi_Write(u32 address, void *buf, u32 size)
{
  u32 i;
  u32 page_start;
  u32 page_end;
  u32 offset_start;
  u32 offset_end;
  u8 *src_ptr;
  u8 temp_buffer[SPI_NVRAM_PAGE_SIZE];

  src_ptr = (u8 *)buf;
  page_start = (address / SPI_NVRAM_PAGE_SIZE) * SPI_NVRAM_PAGE_SIZE;
  page_end = ((address+size-1) / SPI_NVRAM_PAGE_SIZE ) * SPI_NVRAM_PAGE_SIZE;
  offset_start = address % SPI_NVRAM_PAGE_SIZE;
  offset_end = (address+size-1) % SPI_NVRAM_PAGE_SIZE;

  while( page_start <= page_end ) {
    if( offset_start != 0 ) {
      NVRAMi_Read(page_start, temp_buffer, offset_start);
    }
    if( page_start != page_end ) {
      for( i  = offset_start ; i < SPI_NVRAM_PAGE_SIZE ; i++ ) {
          temp_buffer[i] = *src_ptr++;
      }
    }
    else {
      for( i  = offset_start ; i <= offset_end ; i++ ) {
          temp_buffer[i] = *src_ptr++;
      }
      if( offset_end != (SPI_NVRAM_PAGE_SIZE-1) ) {
          NVRAMi_Read(page_start+offset_end+1, &(temp_buffer[offset_end+1]), SPI_NVRAM_PAGE_SIZE - (offset_end+1) );
      }
    }

    NVRAM_WriteEnable();
    NVRAM_WaitWriteEnable();

    NVRAM_PageErase((u32)page_start);

    NVRAM_WaitOperation();

    NVRAM_WriteEnable();

    NVRAM_PageWrite((u32)page_start, (u16)SPI_NVRAM_PAGE_SIZE, temp_buffer);
    NVRAM_WaitOperation();

    page_start += SPI_NVRAM_PAGE_SIZE;
    offset_start = 0;
  }
}
