#include <twl.h>
#include "util.h"

u8 ctoh( const char c );

void putBinary( u8 *src, u32 size )
{
	int i;
	for( i = 0; i < size; i++ )
	{
		OS_TPrintf( "%x", src[i] );
	}
	
	OS_TPrintf( "\n" );
}

u8 ctoh( const char c )
{
	if( '0' <= c && c <= '9' )
	{
		return (u8) (c - '0');
	}
	
	if( 'a' <= c && c <= 'f' )
	{
		return (u8) ((c - 'a') + 10);
	}
	
	if( 'A' <= c && c <= 'F' )
	{
		return (u8) ((c - 'A') + 10);
	}
	
	return 0;
}

void strToHexa( const char *src, u8 *dst, u32 length )
// 受け取った文字列配列を16進配列へ変換
// lengthは文字数ではなくバイト数なので注意
{
	int i;
	for( i = 0; i < length*2 ; i += 2 )
	{
		dst[i/2] = (u8) ( (ctoh( src[i] ) << 4) | ctoh( src[i+1] ) );
	}
}
