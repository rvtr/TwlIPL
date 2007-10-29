/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     

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

//
//  BN系関数用のメモリ関連処理を置き換え
//
#include <nitro.h>
#include "acmemory.h"

/*
  head                                                tail
    |                  |            |                  |
    +--------+---------+----....----+---------+--------+
    |block   |check    |buffer      |check    |block   |
    +--------+---------+----....----+---------+--------+
    |4byte   |4byte    |Upper4(size)|4byte    |4byte   |
    <------------------    4 * n    ------------------->

    block = n = (4 * 4 + Upper4(size)) / 4
    check = 0x0F5555F0：未使用／0x0FAAAAF0：使用中
*/

unsigned long   aACMemoryPoolA[ 16 ];
unsigned long   aACMemoryPool[ 1024 * 32 / sizeof (unsigned long) ];    //  32K
unsigned long   aACMemoryPoolB[ 16 ];

#define     ACMEMORYPOOL_HEAD       (aACMemoryPool)
#define     ACMEMORYPOOL_TAIL       (aACMemoryPool + sizeof aACMemoryPool / sizeof (unsigned long))
#define     ACMEMORYPOOL_SIZE       ((unsigned long)ACMEMORYPOOL_TAIL - (unsigned long)ACMEMORYPOOL_HEAD)

#define     ACMEMORY_CHECK_BASE     0xF000000F
#define     ACMEMORY_CHECK_FREE     (0x00555500 | ACMEMORY_CHECK_BASE)
#define     ACMEMORY_CHECK_USED     (0x00AAAA00 | ACMEMORY_CHECK_BASE)

#define     ACMemory_Lower4( _size )                (((_size)    ) & 0xFFFFFFFC)
#define     ACMemory_Upper4( _size )                (((_size) + 3) & 0xFFFFFFFC)

#define     pACMemory_FromHeadToBody( _head )       ((unsigned long*)(((volatile unsigned long*)(_head)) + 2))
#define     pACMemory_FromBodyToHead( _body )       ((unsigned long*)(((volatile unsigned long*)(_body)) - 2))
#define     pACMemory_FromHeadToTail( _head )       ((unsigned long*)(((volatile unsigned long*)(_head)) + nACMemory_HeadBlock( _head )))
#define     pACMemory_FromTailToHead( _tail )       ((unsigned long*)(((volatile unsigned long*)(_tail)) - nACMemory_TailBlock( _tail )))
#define     nACMemory_HeadBlock( _head )            (((volatile unsigned long*)(_head))[+0])
#define     nACMemory_HeadCheck( _head )            (((volatile unsigned long*)(_head))[+1])
#define     nACMemory_TailBlock( _tail )            (((volatile unsigned long*)(_tail))[-1])
#define     nACMemory_TailCheck( _tail )            (((volatile unsigned long*)(_tail))[-2])

//#define       USE_OSALLOC
//#define       USE_ACMEMORY_DEBUGDUMP
//#define       USE_ACMEMORY_DEBUGFILL

//
#if     defined( USE_ACMEMORY_DEBUGDUMP )
static
void    acMemory_DebugDump( )
{
    unsigned long*  ptr = ACMEMORYPOOL_HEAD;

    // 
    OS_Printf( "----ACMemory_Dump [0x%.8x - 0x%.8x : 0x%.8x ]----\n", ACMEMORYPOOL_HEAD, ACMEMORYPOOL_TAIL, ACMEMORYPOOL_SIZE );

    for ( ; (unsigned long)ptr < (unsigned long)ACMEMORYPOOL_TAIL; )
    {
        OS_Printf( "  [0x%.8x - 0x%.8x] (0x%.8x) <%c>  ",
            ptr,
            pACMemory_FromHeadToTail( ptr ), 
            nACMemory_HeadBlock( ptr ),
            nACMemory_HeadCheck( ptr ) == ACMEMORY_CHECK_USED ? '*' : '-' );

        //OS_Printf( "0x%.8x 0x%.8x",  nACMemory_HeadBlock( ptr ), nACMemory_HeadCheck( ptr ) );

        //OS_Printf( " .... " );

        ptr += nACMemory_HeadBlock( ptr );

        //OS_Printf( "0x%.8x 0x%.8x",  nACMemory_TailCheck( ptr ), nACMemory_TailBlock( ptr ) );

        OS_Printf( "\n" );
    OS_PrintServer();
    }
    ptr = 0;
}

#define ACMemory_DebugDump( )                   acMemory_DebugDump( )

#else

#define ACMemory_DebugDump( )

#endif  //USE_ACMEMORY_DEBUGDUMP

//
#if     defined( USE_ACMEMORY_DEBUGFILL )
static
void    acMemory_DebugFill( void* ptr, int cnt, int val )
{
    (void)ACMemory_Memset( ptr, val, cnt );
}

#define ACMemory_DebugFill( _ptr, _cnt, _val )  acMemory_DebugFill( _ptr, _cnt, _val )

#else

#define ACMemory_DebugFill( _ptr, _cnt, _val )

#endif  //USE_ACMEMORY_DEBUGFILL

//
void    ACMemory_Clear( )
{
    unsigned long*  head = ACMEMORYPOOL_HEAD;
    unsigned long*  tail = ACMEMORYPOOL_TAIL;
    unsigned long   block;
    unsigned long   check;


    block = ACMEMORYPOOL_SIZE / 4;
    check = ACMEMORY_CHECK_FREE;

    nACMemory_HeadBlock( head ) = block;
    nACMemory_HeadCheck( head ) = check;
    nACMemory_TailBlock( tail ) = block;
    nACMemory_TailCheck( tail ) = check;

    ACMemory_DebugFill( pACMemory_FromHeadToBody( head ), (block - 4) * 4, 0xFF );
    ACMemory_DebugDump( );
}

//
void*   ACMemory_Alloc( u32 size )
{
  #if       defined( USE_OSALLOC )
    OSIntrMode  nOSIntrMode;
    void*       alloc = NULL;

    nOSIntrMode = OS_DisableInterrupts( );
    alloc = OS_Alloc( size );
    (void)OS_RestoreInterrupts( nOSIntrMode );
    return alloc;

  #else

    unsigned long*  head = ACMEMORYPOOL_HEAD;
    unsigned long*  tail = ACMEMORYPOOL_TAIL;
    unsigned long   block, oldblock, newblock;
    unsigned long   check, oldcheck, newcheck;

    if ( !size )    return 0;

    //
    if ( size < 16 )    size = 16;
    block = 4 + ACMemory_Upper4( size ) / 4;
    check = ACMEMORY_CHECK_USED;
    
    // 
    for ( ; ; )
    {
        if ( (unsigned long)head >= (unsigned long)tail )
        {
            head = 0;
            break;
        }

        newblock = nACMemory_HeadBlock( head );
        newcheck = nACMemory_HeadCheck( head );
        if ( newcheck == check )
        {
            head += newblock;
            continue ;
        }
        if ( newblock <  block + 4 + 4 )
        {
            head += newblock;
            continue ;
        }

        break;
    }

    if ( !head )    return 0;

    //  
    oldblock = nACMemory_HeadBlock( head );
    oldcheck = nACMemory_HeadCheck( head );

    nACMemory_HeadBlock( head ) = block;
    nACMemory_HeadCheck( head ) = check;

    tail = pACMemory_FromHeadToTail( head );
    nACMemory_TailBlock( tail ) = block;
    nACMemory_TailCheck( tail ) = check;

    if ( (unsigned long)tail < (unsigned long)ACMEMORYPOOL_TAIL )
    {
        nACMemory_HeadBlock( tail ) = oldblock - block;
        nACMemory_HeadCheck( tail ) = oldcheck;
        tail = pACMemory_FromHeadToTail( tail );
        nACMemory_TailBlock( tail ) = oldblock - block;
        nACMemory_TailCheck( tail ) = oldcheck;
    }

    //
   (void)ACMemory_Memset( pACMemory_FromHeadToBody( head ), 0x00, (block - 4) * 4 );

    ACMemory_DebugFill( pACMemory_FromHeadToBody( head ), (block - 4) * 4, 0x00 );
    ACMemory_DebugDump( );

    return (void*)pACMemory_FromHeadToBody( head );
  #endif
}

//
void    ACMemory_Free( void* adrs )
{
  #if       defined( USE_OSALLOC )
    OSIntrMode  nOSIntrMode;

    nOSIntrMode = OS_DisableInterrupts( );
    OS_Free( adrs );
    (void)OS_RestoreInterrupts( nOSIntrMode );
  #else

    unsigned long*  work;
    unsigned long*  head;
    unsigned long*  tail;
    unsigned long   block;
    unsigned long   check;


    if ( !adrs )    return ;

    //  
    head = pACMemory_FromBodyToHead( adrs );
    tail = pACMemory_FromHeadToTail( head );
    block = nACMemory_HeadBlock( head );
    check = ACMEMORY_CHECK_FREE;

    //
    if ( nACMemory_HeadBlock( head ) != nACMemory_TailBlock( tail ) )
        return ;
    if ( nACMemory_HeadCheck( head ) != nACMemory_TailCheck( tail ) )
        return ;


    //  
    if ( (unsigned long)head != (unsigned long)ACMEMORYPOOL_HEAD )
    {
        work = pACMemory_FromTailToHead( head );
        if ( nACMemory_HeadCheck( work ) == check )
        {
            block += nACMemory_HeadBlock( work );
            head   = work;
        }
    }
    //
    if ( (unsigned long)tail != (unsigned long)ACMEMORYPOOL_TAIL )
    {
        work = pACMemory_FromHeadToTail( tail );
        if ( nACMemory_TailCheck( work ) == check )
        {
            block += nACMemory_TailBlock( work );
            tail   = work;
        }
    }

    //
    nACMemory_HeadBlock( head ) = nACMemory_TailBlock( tail ) = block;
    nACMemory_HeadCheck( head ) = nACMemory_TailCheck( tail ) = check;

    ACMemory_DebugFill( pACMemory_FromHeadToBody( head ), (block - 4) * 4, 0xFF );
    ACMemory_DebugDump( );

  #endif
}


//
void*   ACMemory_Memset( void* adrs, u32 val, u32 cnt )
{
    if ( !adrs )    return 0;
    MI_CpuFill8( (void*)adrs, (u8)val, (u32)cnt );
    return adrs;
}

//
void*   ACMemory_Memcpy( void* dst, void* src, u32 cnt )
{
    if ( !dst || !src ) return 0;
    MI_CpuCopy8( (const void*)src, (void*)dst, (u32)cnt );
    return dst;
}


