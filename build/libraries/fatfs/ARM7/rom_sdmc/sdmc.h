
#ifndef __SDMC_H__
#define __SDMC_H__

//#include <brom.h>
//#include <rtfs.h>

#include <firm/devices/firm_sdmc/ARM7/sdmc_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************
 RTFS�p�h���C�o�C���^�t�F�[�X
*********************************************/
#if 0
BOOL sdmcRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
int  sdmcRtfsCtrl( int driveno, int opcode, void* pargs);
BOOL sdmcRtfsAttach( int driveno);
#endif

BOOL sdmcCheckMedia( void);


/*********************************************
 �f�[�^�]���֐��̓o�^�֘A
*********************************************/
typedef void (*sdmcTransferFunction)( void* sd_adr, u32 size, BOOL read_flag);

//void sdmcSetTransferFunction( sdmcTransferFunction usr_func);


/*********************************************
 ��{API
*********************************************/
void sdmcClearPortContext( SDPortContext* buf_adr);
SDMC_ERR_CODE sdmcCheckPortContext( SDPortContext* buf_adr);


SDMC_ERR_CODE    sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(),void (*func2)());/* �J�[�h�h���C�o������ */
SDMC_ERR_CODE    sdmcReset( void);                                    /* �J�[�h���Z�b�g */

SDMC_ERR_CODE    sdmcGetStatus(u16 *status);           /* �J�[�h�h���C�o�̌��݂̏�Ԃ��擾���� */
u32              sdmcGetCardSize(void);                /* �J�[�h�S�T�C�Y�̎擾 */

/*SD I/F��FIFO���g���ă��[�h����i�����j*/
SDMC_ERR_CODE    sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
SDMC_ERR_CODE    sdmcReadFifoDirect( sdmcTransferFunction usr_func,
                                     u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);

SDMC_ERR_CODE sdmcReadStreamBegin( u32 offset, SdmcResultInfo *info);
SDMC_ERR_CODE sdmcReadStreamEnd( SdmcResultInfo *info);


/*���[�h����*/
//SDMC_ERR_CODE    sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);

/*SD I/F��FIFO���g���ă��C�g����i�����j*/
SDMC_ERR_CODE    sdmcWriteFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
SDMC_ERR_CODE    sdmcWriteFifoDirect(sdmcTransferFunction usr_func,
                                     u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
/*���C�g����*/
//SDMC_ERR_CODE    sdmcWrite(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);

/*�|�[�g�I��*/
u16           sdmcSelectedNo(void);
SDMC_ERR_CODE sdmcSelect(u16 select);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__SDMC_H__*/
