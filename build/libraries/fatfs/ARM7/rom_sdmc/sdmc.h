
#ifndef __SDMC_H__
#define __SDMC_H__

//#include <brom.h>
//#include <rtfs.h>

#include <firm/devices/firm_sdmc/ARM7/sdmc_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************
 RTFS用ドライバインタフェース
*********************************************/
#if 0
BOOL sdmcRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
int  sdmcRtfsCtrl( int driveno, int opcode, void* pargs);
BOOL sdmcRtfsAttach( int driveno);
#endif

BOOL sdmcCheckMedia( void);


/*********************************************
 データ転送関数の登録関連
*********************************************/
typedef void (*sdmcTransferFunction)( void* sd_adr, u32 size, BOOL read_flag);

//void sdmcSetTransferFunction( sdmcTransferFunction usr_func);


/*********************************************
 基本API
*********************************************/
void sdmcClearPortContext( SDPortContext* buf_adr);
SDMC_ERR_CODE sdmcCheckPortContext( SDPortContext* buf_adr);


SDMC_ERR_CODE    sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(),void (*func2)());/* カードドライバ初期化 */
SDMC_ERR_CODE    sdmcReset( void);                                    /* カードリセット */

SDMC_ERR_CODE    sdmcGetStatus(u16 *status);           /* カードドライバの現在の状態を取得する */
u32              sdmcGetCardSize(void);                /* カード全サイズの取得 */

/*SD I/FのFIFOを使ってリードする（高速）*/
SDMC_ERR_CODE    sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
SDMC_ERR_CODE    sdmcReadFifoDirect( sdmcTransferFunction usr_func,
                                     u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);

SDMC_ERR_CODE sdmcReadStreamBegin( u32 offset, SdmcResultInfo *info);
SDMC_ERR_CODE sdmcReadStreamEnd( SdmcResultInfo *info);


/*リードする*/
//SDMC_ERR_CODE    sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);

/*SD I/FのFIFOを使ってライトする（高速）*/
SDMC_ERR_CODE    sdmcWriteFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
SDMC_ERR_CODE    sdmcWriteFifoDirect(sdmcTransferFunction usr_func,
                                     u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
/*ライトする*/
//SDMC_ERR_CODE    sdmcWrite(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);

/*ポート選択*/
u16           sdmcSelectedNo(void);
SDMC_ERR_CODE sdmcSelect(u16 select);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__SDMC_H__*/
