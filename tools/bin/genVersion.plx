#!/usr/bin/perl

######################################################################
#	genVersion.pl
#
#	generate SystemMenu Version Data
#   
#   [[ FILE FORMAT ]]
#
#   Security code           (128 bytes) : RSA signature of Header
#   Header + Data                       : NitroSystemArchive Format
#
######################################################################

use POSIX 'strftime';
use File::Basename;

if ($#ARGV < 6) {
    printf STDOUT ("Usage: %s [genVersion] timestamp strVersion majorVersion minorVersion userAreaSize NUPHostName EULAURL\n", $0);
    exit(-1);
}

# �A�[�J�C�u�ɂ܂Ƃ߂�f�[�^�t�@�C��
my $dataDir                   = "archive_data";
my $versionFile               = "$dataDir/version.bin";
my $timeStampFile             = "$dataDir/time_stamp.bin";
my $userAreaSizeFile          = "$dataDir/user_area_size.bin";
my $nupHostFile               = "$dataDir/nup_host.bin";
my $urlEULAFile               = "$dataDir/eula_url.bin";

# �ؖ�������ь���IPL_private�ō쐬�������̂����̂܂܎g���̂ł����ł͍쐬���Ȃ�
#my $cert4SSLClientFile        = "$dataDir/cert4ssl";
#my $privKeyCert4SSLClientFile = "$dataDir/priv_key_cert4ssl";
#my $rootCACert4SSLFile1       = "$dataDir/root_ca_cert4ssl1";
#my $rootCACert4SSLFile2       = "$dataDir/root_ca_cert4ssl2";
#my $rootCACert4SSLFile3       = "$dataDir/root_ca_cert4ssl3";
#my $rootCACert4SSLFile4       = "$dataDir/root_ca_cert4ssl4";
#my $rootCACert4SSLFile5       = "$dataDir/root_ca_cert4ssl5";
#my $rootCACert4SSLFile6       = "$dataDir/root_ca_cert4ssl6";

# �o�̓t�@�C��
my $outFile          = "SysmenuVersion.dat";

# ���ԃt�@�C��
my $archiveFile      = "archive.narc";
my $digestFile       = "digest.bin";
my $signFile         = "sign.bin";

# ��n��
sub deleteTemp {
	system ("rm -f $digestFile");
	system ("rm -f $signFile");
	system ("rm -f $archiveFile");
}

# ���ϐ��T�[�`
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWL_IPL_RED_PRIVATE_ROOT/s) {
        $KEYROOT = $ENV{$_};
    }
}
if (!$KEYROOT) {
	die "No TWL_IPL_RED_PRIVATE_ROOT is found.\n";
}

my $TWLIPLRED_ROOT;

# ���ϐ��̃L�[��TWL_IPL_RED_ROOT�����邩
if (exists($ENV{"TWL_IPL_RED_ROOT"}) ){
    $TWLIPLRED_ROOT = $ENV{"TWL_IPL_RED_ROOT"};
}else
{
	die "No TWL_IPL_RED_ROOT is found.\n";
}

# �V�X�e�����j���[�o�[�W�����̏o��
{
	my $length = 0x20;
	my $sysMenuVersion = $ARGV[1];
	if( length $sysMenuVersion >= $length ) {
		printf "ERROR: SystemMenu version length less than %d.\n", $length;
		die;
	}
	open VERSION, ">$versionFile" or die "File Open Error.\n";
	binmode VERSION;
#	my $ver = ( ( $ARGV[ 1 ] & 0xffff ) << 16 ) | ($ARGV[ 2 ] & 0xffff);
#	printf "version = %d.%d\n", ($ver >> 16), ($ver & 0xffff);
#	syswrite( VERSION, pack( "L", $ver ) );
	printf "SysMenu version = %s\n", $sysMenuVersion;
    syswrite( VERSION, pack( "a$length", $sysMenuVersion) );
	close VERSION;
}

# �^�C���X�^���v�̏o��
{
	open TIMESTAMP, ">$timeStampFile" or die "File Open Error.\n";
	binmode TIMESTAMP;
#	my $timestamp = strftime "%y%m%d%H", localtime;
	my $timestamp = $ARGV[ 0 ];
	printf "timestamp = %s\n", $timestamp;
	syswrite( TIMESTAMP, pack( "N", unpack( "L", pack( "H8", $timestamp ) ) ) );
	close TIMESTAMP;
}

# ���[�U�G���A�T�C�Y�̏o��
{
	open USERAREA, ">$userAreaSizeFile" or die "File Open Error.\n";
	binmode USERAREA;
	my $userAreaSize = $ARGV[ 4 ];
	printf "userAreaSize = $userAreaSize\n";
	syswrite( USERAREA, pack( "L", $userAreaSize ) );
	close USERAREA;
}

# NUP_HOSTNAME�̏o��
{
	my $length = 0x40;
	my $nupHostName = $ARGV[ 5 ];
	if( length $nupHostName >= $length ) {
		printf "ERROR: NUP Host Name length less than %d.\n", $length;
		die;
	}
	open NUPHOSTNAME, ">$nupHostFile" or die "File Open Error.\n";
	binmode NUPHOSTNAME;
	printf "NUPHostName = $nupHostName\n";
	syswrite( NUPHOSTNAME, pack( "a$length", $nupHostName ) );
	close NUPHOSTNAME;
}

# EULA_URL�̏o��
{
	my $length = 0x80;
	my $urlEULA = $ARGV[ 6 ];
	if( length $urlEULA >= $length ) {
		printf "ERROR: EULA URL length less than %d.\n", $length;
		die;
	}
	open EULAURL, ">$urlEULAFile" or die "File Open Error.\n";
	binmode EULAURL;
	printf "EULAURL = $urlEULA\n";
	syswrite( EULAURL, pack( "a$length", $urlEULA ) );
	close EULAURL;
}

my $TWLSYSTEM_ROOT;

# ���ϐ��̃L�[��TWLSYSTEM_ROOT�����邩
if (exists($ENV{"TWLSYSTEM_ROOT"}) ){
    $TWLSYSTEM_ROOT = $ENV{"TWLSYSTEM_ROOT"};
}else
{
	deleteTemp();
	die "No TWLSYSTEM_ROOT is found.\n";
}

# �A�[�J�C�u�쐬
{
    system ( "$TWLSYSTEM_ROOT/tools/bin/nnsarc.exe -c $archiveFile -A 16 $dataDir -s -E .svn" );
}

# �A�[�J�C�u�̃n�b�V���̏o��
{
	my $digest;
	system ("openssl dgst -sha1 -binary -out $digestFile $archiveFile");
}

# �����t��
{
    system ( "openssl rsautl -sign -in $digestFile -inkey $KEYROOT/keys/rsa/private_sharedFont.der -keyform DER -out $signFile" );
	system ( "cat $signFile $archiveFile >$outFile" );
	deleteTemp();
}
