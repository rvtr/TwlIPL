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

# ��n��
sub deleteTemp {
	system ("rm -f $versionFile");
	system ("rm -f $signFile");
}

my $signSize   = 0x80;
my $versionSize = 0x20;
my @files;

# �o�[�W�������̏o��
{
	# timestampLen  = 0x08;
	# elementNumLen = 0x02;
	my $padLen      = 0x18;
	
	open VERSION, ">$versionFile" or die "File Open Error.\n";
	binmode VERSION;
	
	# �^�C���X�^���v�̏o��
#	my $timestamp = strftime "%y%m%d%H", localtime;
	my $timestamp = $ARGV[ 0 ];
	my $ver = $ARGV[ 1 ];
	printf "timestamp = %s\n", $timestamp;
	syswrite( VERSION, pack( "N", unpack( "L", pack( "H8", $timestamp ) ) ) );
	
	# �o�[�W�����̏o��
	syswrite( VERSION, pack( "L", $ver ) );
	
	# �p�f�B���O�̏o��
	syswrite( VERSION, pack( "x$padLen") );
	
	close VERSION;
}

# ���ϐ��T�[�`
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWL_IPL_RED_PRIVATE_ROOT/s) {
        $KEYROOT = $ENV{$_};
    }
}
if (!$KEYROOT) {
	deleteTemp();
	die "No TWL_IPL_RED_PRIVATE_ROOT is found.\n";
}

# �o�[�W�������ɏ����t��
{
    system ( "openssl rsautl -sign -in $versionFile  -inkey $KEYROOT/keys/rsa/private_sharedFont.der -keyform DER -out $signFile" );
	system ( "cat $signFile $versionFile >$outFile" );
	deleteTemp();
}
