#!/usr/bin/perl

######################################################################
#	genVersion.pl
#
#	generate SystemMenu Version Data
#   
#   [[ FILE FORMAT ]]
#   security code           (128 bytes) : RSA signature of Version Data
#
#   Header                  ( 96 bytes)
#       TimeStamp           (  4 bytes) : date %y%m%d%H
#       Version             (  4 bytes) : 0.0 ~ 65535.65535
#       UserAreaSize        (  4 bytes) : Size of UserArea
#       Data1Offset         (  4 bytes) : File offset of Data1
#       Data1Size           (  4 bytes) : Size of Data1
#       padding             ( 12 bytes) : 
#       Data1Hash           ( 20 bytes) : SHA1 digest of Data1
#       padding             ( 44 bytes) : 
#
#   Data1                   (flexible length) : 
#       padding             ( 32 bytes) : 
#
#   note: each section image is aligned to 32 bytes.
#
######################################################################

use POSIX 'strftime';
use File::Basename;

if ($#ARGV < 3) {
    printf STDOUT ("Usage: %s [genVersion] timestamp majorVersion minorVersion userAreaSize\n", $0);
    exit(-1);
}

my $outFile    = "SysmenuVersion.dat";
my $data1File  = "data1.bin";
my $headerFile = "header.bin";
my $digestFile = "digest.bin";
my $tmpFile    = "tmp.bin";
my $signFile   = "sign.bin";

# ��n��
sub deleteTemp {
	system ("rm -f $tmpFile");
	system ("rm -f $digestFile");
	system ("rm -f $data1File");
	system ("rm -f $headerFile");
	system ("rm -f $signFile");
}

my $signSize   = 0x80;
my $headerSize = 0x80;
my $data1Len = 0x20;

# Data1���̏o��
{
	open DATA1, ">$data1File" or die "File Open Error.\n";
	binmode DATA1;
	
	# �_�~�[�f�[�^�̏o��
	syswrite( DATA1, pack( "x$data1Len") );
	
	close DATA1;
}

# �w�b�_���̏o��
{
	# timestampLen   = 0x08;
	# elementNumLen  = 0x02;
	my $pad1Len      = 0x0c;
	my $pad2Len      = 0x2c;
	my $data1Off     = $signSize + $headerSize;
	my $sha1Len     = 0x14;

	open HEADER, ">$headerFile" or die "File Open Error.\n";
	binmode HEADER;
	
#	my $timestamp = strftime "%y%m%d%H", localtime;
	my $timestamp = $ARGV[ 0 ];
	my $ver = ( ( $ARGV[ 1 ] & 0xffff ) << 16 ) | ($ARGV[ 2 ] & 0xffff);
	my $userAreaSize = $ARGV[ 3 ];

	# �^�C���X�^���v�̏o��
	printf "timestamp = %s\n", $timestamp;
	syswrite( HEADER, pack( "N", unpack( "L", pack( "H8", $timestamp ) ) ) );
	
	# �o�[�W�����̏o��
	printf "version = %d.%d\n", ($ver >> 16), ($ver & 0xffff);
	syswrite( HEADER, pack( "L", $ver ) );
	
	# ���[�U�̈�T�C�Y�̏o��
	printf "userAreaSize = $userAreaSize\n";
	syswrite( HEADER, pack( "L", $userAreaSize ) );
	# Data1�I�t�Z�b�g�̏o��
	printf "data1Offset = $data1Off\n";
	syswrite( HEADER, pack( "L", $data1Off ) );
	# Data1�T�C�Y�̏o��
	printf "data1Size = $data1Len\n";
	syswrite( HEADER, pack( "L", $data1Len ) );
	
	# �p�f�B���O1�̏o��
	syswrite( HEADER, pack( "x$pad1Len") );

	# Data1�n�b�V���̏o��
	{
		my $digest;
		system ("openssl dgst -sha1 -binary -out $digestFile $data1File");
		open DIGEST, $digestFile or die "File Open Error.\n";
		binmode DIGEST;
		sysread( DIGEST, $digest, $sha1Len );
		close DIGEST;
		syswrite( HEADER, $digest, $sha1Len );
	}

	# �p�f�B���O2�̏o��
	syswrite( HEADER, pack( "x$pad2Len") );
	
	close HEADER;
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

# �����t��
{
    system ( "openssl rsautl -sign -in $headerFile  -inkey $KEYROOT/keys/rsa/private_sharedFont.der -keyform DER -out $signFile" );
	system ( "cat $signFile $headerFile >$tmpFile" );
	system ( "cat $tmpFile $data1File > $outFile" );
	deleteTemp();
}
