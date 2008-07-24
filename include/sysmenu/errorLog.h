/*---------------------------------------------------------------------------*
  Project:  TwlIPL - ErrorLog
  File:     errorLog.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
 
 #ifndef __SYSM_ERRORLOG__
 #define __SYSM_ERRORLOG__

#include <twl.h> 
 
#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

/*-- type definition ----------------------------*/


// ���ɏ������܂ꂽ�G���[���O��\�����邽�߂̃G���g��
typedef struct ErrorLogEntry{
	// �G���[�̃^�C���X�^���v
	int year;
	int month;
	int day;
	char week[4]; // �j����3�����\��
	int hour;
	int minute;
	int second;
	// �G���[�R�[�h
	int errorCode;
} ErrorLogEntry;

typedef struct ErrorLogWork{
	// �������m�ۗp�֐�
	void* (*Alloc) ( u32 )  ;
	void (*Free) ( void* )  ;
	// �G���[���O�G���g���ێ��p�ϐ�	
	ErrorLogEntry *entry;
	// �G���[���O�̃G���g����
	int numEntry;
	// �G���[���O�̃t�@�C���|�C���^
	FSFile file;
} ErrorLogWork;


/*-- function prototype -------------------------*/
extern BOOL ERRORLOG_Write( u64 errorCode );
extern BOOL ERRORLOG_Init( void* (*AllocFunc) (u32) , void (*FreeFunc) (void*)  );
extern void ERRORLOG_End( void );
extern int ERRORLOG_getNum() ;
extern const ErrorLogEntry* ERRORLOG_Read( int idx );


	


#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif

 
 
#endif
