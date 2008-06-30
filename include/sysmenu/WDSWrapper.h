//**********************************************************************
//
//	@file		WDSWrapper.h
//	@brief		WDS���C�u�����̃��b�p�[
//
//	@author		S.Nakata
//	@date		2008/06/21
//	@version	0.0
//
//*********************************************************************/
#ifndef	__WDSWRAPPER_H__
#define	__WDSWRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------
//	Include
//-----------------------------------------------------
#include <sysmenu/WDS.h>

//-----------------------------------------------------
//	Macros
//-----------------------------------------------------
#define WDSWRAPPER_STACKSIZE	4096	///< WDS���b�p�[���g�p����X���b�h�X�^�b�N�̐����T�C�Y
#define WDSWRAPPER_SCANPERIOD	3000	///< WDS���b�p�[���X�L�������p�����鎞��[msec]
#define WDSWRAPPER_WAITPERIOD	3000	///< WDS���b�p�[���X�L�����𒆒f���鎞��[msec]

//#define WDSWRAPPER_DEBUGPRINT	///< �f�o�b�O�\���t���O

//-----------------------------------------------------
//	Types of WDS Wrapper Library
//-----------------------------------------------------

/**
	@brief	�������ʂ�����킷�񋓌^
*/
typedef enum WDSWrapperErrCode
{
	WDSWRAPPER_ERRCODE_SUCCESS		= 0,	///< ��������
	WDSWRAPPER_ERRCODE_FAILURE		= 1,	///< �������s
	WDSWRAPPER_ERRCODE_OPERATING	= 2,	///< �������s��
	WDSWRAPPER_ERRCODE_INITIALIZED	= 3,	///< WDS���b�p�[���C�u�����������ς�
	WDSWRAPPER_ERRCODE_UNINITIALIZED= 4		///< WDS���b�p�[���C�u������������
} WDSWrapperErrCode;

/**
	@brief	�R�[���o�b�N�����v��������킷�񋓌^
*/
typedef enum WDSWrapperCallback
{
	WDSWRAPPER_CALLBACK_INITIALIZE	= 0,	//!< WDS_WrapperInitialize�֐��ɂ��R�[���o�b�N
	WDSWRAPPER_CALLBACK_CLEANUP		= 1,	//!< WDS_WrapperCleanup�֐��ɂ��R�[���o�b�N
	WDSWRAPPER_CALLBACK_STARTSCAN	= 2,	//!< WDS_WrapperStartScan�֐��ɂ��R�[���o�b�N
	WDSWRAPPER_CALLBACK_STARTSCAN2	= 3,	//!< WDS_WrapperStartScan�֐��ɂ��R�[���o�b�N(�Ԍ��X�L����1��I��)
	WDSWRAPPER_CALLBACK_STOPSCAN	= 4		//!< WDS_WrapperStopScan�֐��ɂ��R�[���o�b�N
} WDSWrapperCallback;

/**
	@brief	WDS���b�p�[�̂̃X�e�[�g������킷�񋓌^
*/
typedef enum WDSWrapperStateThreadState {
	WDSWRAPPER_STATE_BEFOREINIT,			//!< �������J�n�҂��X�e�[�g
	WDSWRAPPER_STATE_INIT,				//!< �������J�n�X�e�[�g
	WDSWRAPPER_STATE_WAITINIT,			//!< �����������҂��X�e�[�g
	WDSWRAPPER_STATE_SCAN,				//!< �X�L�����J�n�X�e�[�g
	WDSWRAPPER_STATE_WAITSCAN,			//!< �X�L���������҂��X�e�[�g
	WDSWRAPPER_STATE_ENDSCAN,			//!< �X�L�������f�X�e�[�g
	WDSWRAPPER_STATE_WAITENDSCAN,		//!< �X�L�������f�����҂��X�e�[�g
	WDSWRAPPER_STATE_END,				//!< WDS����J�n�X�e�[�g
	WDSWRAPPER_STATE_WAITEND,			//!< WDS��������҂��X�e�[�g
	WDSWRAPPER_STATE_IDLE,				//!< �A�C�h���X�e�[�g
	WDSWRAPPER_STATE_TERMINATE			//!< WDS���b�p�[����X�e�[�g
} WDSWrapperStateThreadState;

/**
	@brief	WDSWrapper���C�u�������g�p����R�[���o�b�N�֐��^
	@note
		�E����arg�͕K��WDSWrapperCallback�^�ϐ��ւ̃|�C���^
*/
typedef void ( *WDSWrapperCallbackFunc )( void *arg );

/**
	@brief	WDS���b�p�[�����Ŏg�p����A���P�[�^�[�^
*/
typedef void *( *WDSWrapperAlloc )( u32 size );

/**
	@brief	WDS���b�p�[�����Ŏg�p����A���P�[�^�[�^
*/
typedef void ( *WDSWrapperFree )( void *ptr );

//-----------------------------------------------------
//	Structs of WDS Wrapper Library
//-----------------------------------------------------

/**
	@brief	WDS_WrapperInitialize�֐��ɗ^���鏉�����p�����[�^
*/
typedef struct WDSWrapperInitializeParam
{
	u32						threadprio;	///< WDS�R���g���[���X���b�h�̗D��x
	u16						dmano;		///< WM���C�u�������g�p����DMA�ԍ�
	
	WDSWrapperCallbackFunc	callback;	///< WDS�R���g���[���X���b�h����Ăяo�����R�[���o�b�N�֐��ւ̃|�C���^
	WDSWrapperAlloc			alloc;		///< WDS���b�p�[���g�p����A���P�[�^
	WDSWrapperFree			free;		///< WDS���b�p�[���g�p����A���P�[�^
} WDSWrapperInitializeParam;

/**
	@brief	WDS���b�p�[���Ăяo���R�[���o�b�N�֐��ɗ^����������p�̍\����
*/
typedef struct WDSWrapperCallbackParam
{
	WDSWrapperCallback	callback;	///< �R�[���o�b�N�����v��
	WDSWrapperErrCode	errcode;	///< �G���[�R�[�h
} WDSWrapperCallbackParam;

//-----------------------------------------------------
//	Prototypes of WDS Wrapper Library
//-----------------------------------------------------

WDSWrapperErrCode WDS_WrapperInitialize( WDSWrapperInitializeParam param );
WDSWrapperErrCode WDS_WrapperCleanup( void );
WDSWrapperErrCode WDS_WrapperStartScan( void );
WDSWrapperErrCode WDS_WrapperStopScan( void );
WDSWrapperErrCode WDS_WrapperCheckValidBeacon( void );
WDSWrapperErrCode WDS_WrapperSetArgumentParam( void );
WDSWrapperErrCode WDS_WrapperCheckThreadRunning( void );
OSThread *WDS_WrapperGetOSThread( void );

#ifdef __cplusplus
}
#endif

#endif	//EOF __WDSWRAPPER_H__
