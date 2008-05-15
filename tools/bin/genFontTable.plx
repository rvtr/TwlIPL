#!/usr/bin/perl

######################################################################
#	genFontTable.pl
#
#	generate Secure Shared Font Data Table
#   
#   [[ HEADER FORMAT ]]
#   security code           (128 bytes) : RSA signature of Header
#
#   Header                  ( 32 bytes)
#       TimeStamp           (  4 bytes) : date %y%m%d%H
#       number              (  2 bytes) : number of font files
#       padding             (  6 bytes) : 
#       padding             ( 20 bytes) : SHA1 digest of Font info table
# 
#   Font info table         ( 64 bytes * number)
#       fileName            ( 32 bytes) : font file name
#       padding             (  4 bytes) : 
#       file offset         (  4 bytes) : file offset of Font data
#       length              (  4 bytes) : length of file (bytes)
#       digest              ( 20 bytes) : SHA1 digest of Font data
#
#   note: each section image is aligned to 32 bytes.
#
######################################################################

use POSIX 'strftime';
use File::Basename;

if ($#ARGV < 2) {
    printf STDOUT ("Usage: %s [genFontTable] timestamp [Target font files...]\n", $0);
    exit(-1);
}

my $outFile    = "TWLFontTable.dat";
my $infoFile   = "info.bin";
my $headerFile = "header.bin";
my $digestFile = "sha1.bin";
my $signFile   = "sign.bin";
my $tempFile   = "temp.bin";

# 後始末
sub deleteTemp {
	system ("rm -f $infoFile");
	system ("rm -f $headerFile");
	system ("rm -f $digestFile");
	system ("rm -f $signFile");
	system ("rm -f $tempFile");
}

my $signSize   = 0x80;
my $headerSize = 0x20;
my @files;

# 要素数算出
my $num = 0;
foreach ( @ARGV ) {
	next if( $_ eq $ARGV[0] );
	$files[ $num ] = $_;
	$num++;
}

# 情報テーブルの出力
{
	my $elementSize = 0x40;
	my $fileNameMax = 0x20;
	my $padLen      = 0x04;
	# offset length = 0x04;
	# file length   = 0x04;
	my $sha1Len     = 0x14;
	
	open INFO, ">$infoFile" or die;
	binmode INFO;
	
	# オフセット算出
	my $offset = $signSize + $headerSize + $num * $elementSize;
	if( ( $offset % 32 ) > 0 ) { $offset += 32 - ( $offset % 32 ); }
	
	foreach ( @files ) {
		# NULL指定時は、NULL出力
		if( "NULL" eq basename( $_ ) ) {
			syswrite( INFO, pack( "x$elementSize") );
			next;
		}
		
		# ファイルネームの出力
		if( !( -e $_ ) ) {
			close( INFO );
			deleteTemp();
			die "file not exist. : $_\n";
		}
		my $name = basename( $_ );
		if( length $name >= $fileNameMax ) {
			close( INFO );
			deleteTemp();
			die "file name length must be smaller than $fileNameMax. : $_\n";
		}
		my $data = pack( "a$fileNameMax", $name );
		syswrite( INFO, $data, $fileNameMax );
		
		# パディングの出力
		syswrite( INFO, pack( "x$padLen") );
		
		# ファイルオフセットの出力
		$data = pack( "L", $offset );
		syswrite( INFO, $data, 4 );
		
		# ファイル長の出力
		$data = pack( "L", -s $_ );
		syswrite( INFO, $data, 4 );
		
		# ファイルのSHA1ハッシュの出力
		{
			my $digest;
			system ("openssl dgst -sha1 -binary -out $digestFile $_");
			open DIGEST, $digestFile or die;
			binmode DIGEST;
			sysread( DIGEST, $digest, $sha1Len );
			close DIGEST;
			syswrite( INFO, $digest, $sha1Len );
		}
		
		printf "%s\t0x%08x\t0x%08x\n", $_, $offset, -s $_;
		
		# オフセット加算
		$offset += -s $_;
		if( ( $offset % 32 ) > 0 ) { $offset += 32 - ( $offset % 32 ); }
	}
	close INFO;
}

# ヘッダの出力
{
	# timestampLen  = 0x08;
	# elementNumLen = 0x02;
	my $padLen      = 0x06;
	my $sha1Len     = 0x14;
	
	open HEADER, ">$headerFile" or die;
	binmode HEADER;
	
	# タイムスタンプの出力
#	my $timestamp = strftime "%y%m%d%H", localtime;
	my $timestamp = $ARGV[ 0 ];
	printf "timestamp = %s\n", $timestamp;
	syswrite( HEADER, pack( "N", unpack( "L", pack( "H8", $timestamp ) ) ) );
	
	# 要素数の出力
	syswrite( HEADER, pack( "S", $num ) );
	
	# パディングの出力
	syswrite( HEADER, pack( "x$padLen") );
	
	# 情報テーブルのSHA1ハッシュの出力
	{
		my $digest;
		system ("openssl dgst -sha1 -binary -out $digestFile $infoFile");
		open DIGEST, $digestFile or die;
		binmode DIGEST;
		sysread( DIGEST, $digest, $sha1Len );
		close DIGEST;
		syswrite( HEADER, $digest, $sha1Len );
	}
	close HEADER;
}

# 環境変数サーチ
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWL_IPL_RED_PRIVATE_ROOT/s) {
        $KEYROOT = $ENV{$_};
    }
}
if (!$KEYROOT) {
	deleteTemp();
	die "No TWL_IPL_RED_PRIVATE_ROOT is found.\n";
}

# ヘッダへの署名付加
{
    system ( "openssl dgst -sha1 -binary -out $digestFile $headerFile" );
    system ( "openssl rsautl -sign -in $digestFile -inkey $KEYROOT/keys/rsa/private_sharedFont.der -keyform DER -out $signFile" );
	system ( "cat $signFile $headerFile >$tempFile" );
	system ( "cat $tempFile $infoFile >$outFile" );
	deleteTemp();
}


# フォントの出力
{
	open FONTTABLE, ">>$outFile" or die;
	binmode FONTTABLE;
	
	{
		# パディング出力
		my $fileLen = -s $outFile;
		my $padNum = ( $fileLen % 32 ) ? ( 32 - ( $fileLen % 32 ) ) : 0;
		my $padding = pack( "x$padNum" );
		syswrite( FONTTABLE, $padding, $padNum );
	}

	foreach ( @files ) {
		# NULL指定時はスキップ
		if( "NULL" eq basename( $_ ) ) {
			next;
		}
		
		# フォント出力
		my $fileLen = -s $_;
		open TEST, $_ or die;
		binmode TEST;
		sysread ( TEST, $buffer, $fileLen );
		close TEST;
		syswrite( FONTTABLE, $buffer, $fileLen );
		
		# パディング出力
		my $padNum = ( $fileLen % 32 ) ? ( 32 - ( $fileLen % 32 ) ) : 0;
		my $padding = pack( "x$padNum" );
		syswrite( FONTTABLE, $padding, $padNum );
	}
	close FONTTABLE;
}
