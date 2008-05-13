#!/usr/bin/perl

######################################################################
#	genFontTable.pl
#
#	attach signature to firmware file
#   
#   [[ HEADER FORMAT ]]
#   Header
#       TimeStamp           (  4 bytes) : number of file sections
#       number              (  2 bytes) : number of font files
#       padding             ( 26 bytes) : 
#
#   security code           (128 bytes) : RSA signature of Header
#
#   section information     ( 48 bytes * number) 
#       fileName            ( 24 bytes) : font file name
#       length              (  4 bytes) : length of file (bytes)
#       digest              ( 20 bytes) : WLAN FW=2 DATAPATCH=5 signature = 128
#
#   note: each section image is aligned to 48 bytes.
#
######################################################################

use POSIX 'strftime';
use File::Basename;

if ($#ARGV < 1) {
    printf STDOUT ("Usage: %s [genFontTable] [Target font files...]\n", $0);
    exit(-1);
}

my $outFile    = "TWLFontTable.dat";
my $headerFile = "header.bin";
my $digestFile = "sha1.bin";
my $signFile   = "sign.bin";

# 後始末
sub deleteTemp {
	system ("rm -f $headerFile");
	system ("rm -f $digestFile");
	system ("rm -f $signFile");
}

# ヘッダの出力
{
	
	open HEADER, ">$headerFile" or die;
	binmode HEADER;
	
	# タイムスタンプの出力
	my $date = strftime "%y%m%d%H", localtime;
	syswrite( HEADER, pack( "H8", $date ) );
	
	# 要素数の出力
	my $num = 0;
	foreach ( @ARGV ) {
		$num++;
	}
	syswrite( HEADER, pack( "S", $num ) );
	
	# パディングの出力
	syswrite( HEADER, pack( "x26") );
	
	foreach ( @ARGV ) {
		# ファイルネームの出力
		my $fileNameMax = 0x18;
		if( !( -e $_ ) ) {
			close( HEADER );
			deleteTemp();
			die "file not exist. : $_\n";
		}
		my $name = basename( $_ );
		if( length $name >= $fileNameMax ) {
			close( HEADER );
			deleteTemp();
			die "file name length must be smaller than $fileNameMax. : $_\n";
		}
		my $data = pack( "a$fileNameMax", $name );
		syswrite( HEADER, $data, $fileNameMax );
		
		# ファイル長の出力
		$data = pack( "L", -s $_ );
		syswrite( HEADER, $data, 4 );
		
		# ファイルのSHA1ハッシュの出力
		{
			my $sha1Len = 20;
			my $digest;
			system ("openssl dgst -sha1 -binary -out $digestFile $_");
			open DIGEST, $digestFile or die;
			binmode DIGEST;
			sysread( DIGEST, $digest, $sha1Len );
			close DIGEST;
			syswrite( HEADER, $digest, $sha1Len );
		}
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
    system ( "openssl rsautl -sign -in $digestFile -inkey $KEYROOT/keys/rsa/private9_1.der -keyform DER -out $signFile" );
	system ( "cat $signFile $headerFile >$outFile" );
	deleteTemp();
}

