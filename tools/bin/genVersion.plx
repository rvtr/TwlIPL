#!/usr/bin/perl

######################################################################
#	genVersion.pl
#
#	generate SystemMenu Version Data
#   
#   [[ FILE FORMAT ]]
#   security code           (128 bytes) : RSA signature of Version Data
#
#   Version Data            ( 32 bytes)
#       TimeStamp           (  4 bytes) : date %y%m%d%H
#       Version             (  4 bytes) : 0 ~ 
#       padding             ( 24 bytes) : 
#
#   note: each section image is aligned to 32 bytes.
#
######################################################################

use POSIX 'strftime';
use File::Basename;

if ($#ARGV < 1) {
    printf STDOUT ("Usage: %s [genVersion] timestamp version\n", $0);
    exit(-1);
}

my $outFile    = "SysmenuVersion.dat";
my $versionFile = "version.bin";
my $signFile   = "sign.bin";

# 後始末
sub deleteTemp {
	system ("rm -f $versionFile");
	system ("rm -f $signFile");
}

my $signSize   = 0x80;
my $versionSize = 0x20;
my @files;

# バージョン情報の出力
{
	# timestampLen  = 0x08;
	# elementNumLen = 0x02;
	my $padLen      = 0x18;
	
	open VERSION, ">$versionFile" or die "File Open Error.\n";
	binmode VERSION;
	
	# タイムスタンプの出力
#	my $timestamp = strftime "%y%m%d%H", localtime;
	my $timestamp = $ARGV[ 0 ];
	my $ver = $ARGV[ 1 ];
	printf "timestamp = %s\n", $timestamp;
	syswrite( VERSION, pack( "N", unpack( "L", pack( "H8", $timestamp ) ) ) );
	
	# バージョンの出力
	syswrite( VERSION, pack( "L", $ver ) );
	
	# パディングの出力
	syswrite( VERSION, pack( "x$padLen") );
	
	close VERSION;
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

# バージョン情報に署名付加
{
    system ( "openssl rsautl -sign -in $versionFile  -inkey $KEYROOT/keys/rsa/private_sharedFont.der -keyform DER -out $signFile" );
	system ( "cat $signFile $versionFile >$outFile" );
	deleteTemp();
}
