/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     loadSharedFont.c

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
#include "loadSharedFont.h"

// ���k�t�H���g���𓀂���Ƃ���1�ɂ���
#define  UNCOMPRESS_FONTS    1

// extern data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
BOOL g_isSucceededLoad[ OS_SHARED_FONT_MAX ];

// static variable-------------------------------------------------------------
static u8 *s_pFontBuffer[ OS_SHARED_FONT_MAX ];     // �ǂݍ��݂̓e�X�g�Ȃ̂Ń��[�h���static�ϐ��ɂ��Ă���B
static BOOL s_isStarted = FALSE;
static u8 *s_pCompressedBuffer[ OS_SHARED_FONT_MAX ];   // ���k�pTempBuffer

// const data------------------------------------------------------------------


// ============================================================================
// ���L�t�H���g���[�h
// ============================================================================
void LoadSharedFont( STicks *pTicks )
{
	u8 *pBuffer;
	int size;
	OSSharedFontIndex i;
    OSTick  starttick;

    pTicks->all  = 0;
    pTicks->comp = 0;

	if( s_isStarted ) return;
	
	starttick = OS_GetTick();
	
	// �t�H���g���[�h����
	if( !OS_InitSharedFont() ) {
		OS_TPrintf( "OS_InitSharedFont failed.\n" );
		return;
	}
	size = OS_GetSharedFontTableSize();
	if( size < 0 ) {
		OS_TPrintf( "OS_GetSharedTableSize failed.\n" );
		return;
	}
	
	pBuffer = OS_Alloc( (u32)size );
	if( pBuffer == NULL ) {
		OS_TPrintf( "malloc failed.\n" );
		return;
	}
	if( !OS_LoadSharedFontTable( pBuffer ) ) {
		OS_TPrintf( "OS_LoadSharedTable failed.\n" );
		return;
	}
	
    for( i = OS_SHARED_FONT_WW_S; i < OS_SHARED_FONT_MAX; i++ )
    {
        int origsize;
#if (UNCOMPRESS_FONTS==1)
        int    compsize;
        int    comperr;
        OSTick comptick;
#endif

        OS_TPrintf( "%s read.\n", OS_GetSharedFontName( i ) );

        // �t�H���g�̌���(�𓀌��)�T�C�Y
        origsize = OS_GetSharedFontSize( i );
        if( origsize < 0 )
        {
            OS_TPrintf( "    get font size failed.\n" );
        }
        origsize = MATH_ROUNDUP( origsize, 32 );		// FS�̃L���b�V�������������Ȃ̂ŁA�Ƃ肠�����A���C�������g���Ƃ��Ă����B
        OS_TPrintf( "    original size   = %d\n", origsize );

#if (UNCOMPRESS_FONTS==1)
        compsize = OS_GetSharedFontCompressedSize( i );
        if( compsize < 0 )
        {
            OS_TPrintf( "    get compressed font size failed.\n" );
        }
        OS_TPrintf( "    compressed size = %d\n", compsize );
#endif

        s_pFontBuffer[ i ] = OS_Alloc( (u32)origsize ); // ���̃t�H���g�T�C�Y�Ńo�b�t�@�m��
        if( s_pFontBuffer[ i ] == NULL )
        {
            OS_TPrintf( "    malloc failed.\n" );
        }

        // ���k��̃t�H���g�����[�h
//        s_pCompressedBuffer[i] = OS_Alloc( (u32)MATH_ROUNDUP( compsize, 32 ) );
//        if( OS_LoadSharedFont( i, s_pCompressedBuffer[ i ] ) )
        if( OS_LoadSharedFont( i, s_pFontBuffer[ i ] ) )
        {
            OS_TPrintf( "    load succeeded.\n" );
            g_isSucceededLoad[ i ] = TRUE;
        }
        else
        {
            OS_TPrintf( "    load failed.\n" );
            g_isSucceededLoad[ i ] = FALSE;
        }

        // ��
#if (UNCOMPRESS_FONTS==1)
        comptick = OS_GetTick();
        OS_TPrintf( "    uncompress font\n" );
//        comperr = MI_SecureUncompressLZ( s_pCompressedBuffer[i], (u32)compsize, s_pFontBuffer[i], (u32)origsize );
        comperr = MI_SecureUncompressBLZ( s_pFontBuffer[i], (u32)compsize, (u32)origsize );
        if( MI_ERR_SUCCESS == comperr  )
        {
            OS_TPrintf( "    uncompression succeeded\n" );
        }
        else
        {
            OS_TPrintf( "    uncompression failed %d\n", comperr );
        }
        pTicks->comp += (OS_GetTick() - comptick);  // �𓀂ɂ�����������
#endif
    }

    // �S���̏����ɂ�����������
    pTicks->all = OS_GetTick() - starttick;

    s_isStarted = TRUE;
}

void WriteFontIntoSD( void )
{
    u8                *pFont = NULL;
    int                size;
    OSSharedFontIndex  i;
    char               str[100];
    FSFile             fp[1];

    FS_InitFile( fp );

    for( i = OS_SHARED_FONT_WW_S; i < OS_SHARED_FONT_MAX; i++ )
    {
        pFont = s_pFontBuffer[i];
        size = OS_GetSharedFontSize(i);
        if( pFont != NULL)
        {
            STD_TSPrintf( str, "sdmc:/%s.uncomp", OS_GetSharedFontName(i) );
            FS_DeleteFile( str );
            if( !FS_CreateFile( str, FS_PERMIT_R|FS_PERMIT_W ) )
            {
                OS_TPrintf( "sd create failed %d\n", FS_GetArchiveResultCode(str) );
            }
            if( FS_OpenFileEx( fp, str, FS_FILEMODE_W ) )
            {
                FS_WriteFile( fp, pFont, size );
                FS_CloseFile( fp );
                OS_TPrintf( "%s is written\n", str );
            }
            else
            {
                OS_TPrintf( "sd write failed %d\n", FS_GetArchiveResultCode(str) );
            }
        }
    }
}




