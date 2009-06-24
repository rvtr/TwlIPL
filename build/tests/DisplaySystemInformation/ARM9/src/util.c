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
// �󂯎����������z���16�i�z��֕ϊ�
// length�͕������ł͂Ȃ��o�C�g���Ȃ̂Œ���
{
	int i;
	for( i = 0; i < length*2 ; i += 2 )
	{
		dst[i/2] = (u8) ( (ctoh( src[i] ) << 4) | ctoh( src[i+1] ) );
	}
}
