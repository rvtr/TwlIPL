#!/usr/bin/perl

#
# To Do
# �EFont info table �� padding �����k��t�@�C���T�C�Y���i�[����G���g���ɕύX
# �E�t�@�C���T�C�Y���擾���ċl�ߍ��ޏ�����ǉ�
# �ESHA-1�n�b�V��������O�Ɉ��k����
#

######################################################################
#   genFontTable.pl
#
#   generate Secure Shared Font Data Table
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
#       comp_length         (  4 bytes) : length of the compression file
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
my $compprog   = "./compBLZ_modified/bin/compBLZ.exe";    # ���k�v���O����
my $compoption = "-e \"\"";
#my $compprog   = "./ntrcomp/gcc/ntrcomp.exe";
#my $compoption = "-h8 -A32 -s";

# ��n��
sub deleteTemp {
    system ("rm -f $infoFile");
    system ("rm -f $headerFile");
    system ("rm -f $digestFile");
    system ("rm -f $signFile");
    system ("rm -f $tempFile");
}

# ���ϐ�����SDK�̃��[�g���T�[�`����(ntrcomp�̏ꏊ����ɕK�v)
#foreach ( sort keys ( %ENV ) ){
#    if ($_ =~ m/TWLSDK_ROOT/s) {
#        $SDKROOT = $ENV{$_};
#    }
#}
#printf "TWLSDK_ROOT is ${SDKROOT}\n";

my $signSize   = 0x80;
my $headerSize = 0x20;
my @files;

# �v�f���Z�o
my $num = 0;
foreach ( @ARGV ) {
    next if( $_ eq $ARGV[0] );
    $files[ $num ] = $_;
    $num++;
}

# ���e�[�u���̏o��
{
    my $elementSize = 0x40;
    my $fileNameMax = 0x20;
    my $padLen      = 0x04;
    # offset length = 0x04;
    # file length   = 0x04;
    my $sha1Len     = 0x14;
    
    open INFO, ">$infoFile" or die;
    binmode INFO;
    
    # �I�t�Z�b�g�Z�o
    my $offset = $signSize + $headerSize + $num * $elementSize;
    if( ( $offset % 32 ) > 0 ) { $offset += 32 - ( $offset % 32 ); }
    
    printf "---------------------------------------------------------------\n";
    printf "filename\toffset  \torig_size\tcompressed_size\n";
    foreach ( @files ) {
        # NULL�w�莞�́ANULL�o��
        if( "NULL" eq basename( $_ ) ) {
            syswrite( INFO, pack( "x$elementSize") );
            next;
        }
        
        # �t�@�C���l�[���̏o��
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
        
        # ���k
        # SDK��compBLZ.exe�͈����o�O�����邽�߃��[�J���ɏC���ł����Ă���
        my $compfile = "$_.comp";
        system ("${compprog} $compoption $_ -o $compfile");

        # �p�f�B���O�̏o��
        #syswrite( INFO, pack( "x$padLen") );

        # ���k�t�@�C�������o��(���Ƃ��Ƃ�padding)
        $data = pack( "L", -s $compfile );
        syswrite( INFO, $data, 4 );

        # �t�@�C���I�t�Z�b�g�̏o��
        $data = pack( "L", $offset );
        syswrite( INFO, $data, 4 );
        
        # �t�@�C�����̏o��
        $data = pack( "L", -s $_ );
        syswrite( INFO, $data, 4 );
        
        # �t�@�C����SHA1�n�b�V���̏o��
        {
            my $digest;
            #system ("openssl dgst -sha1 -binary -out $digestFile $_");
            system ("openssl dgst -sha1 -binary -out $digestFile $compfile");   # ���k��̃t�@�C���Ƀn�b�V��������
            open DIGEST, $digestFile or die;
            binmode DIGEST;
            sysread( DIGEST, $digest, $sha1Len );
            close DIGEST;
            syswrite( INFO, $digest, $sha1Len );
        }
        
        printf "%s\t0x%08x\t0x%08x\t0x%08x\n", $_, $offset, -s $_, -s $compfile;
        
        # �I�t�Z�b�g���Z
        #$offset += -s $_;
        $offset += -s $compfile;
        if( ( $offset % 32 ) > 0 ) { $offset += 32 - ( $offset % 32 ); }
    }
    close INFO;
    printf "---------------------------------------------------------------\n";
}

# �w�b�_�̏o��
{
    # timestampLen  = 0x08;
    # elementNumLen = 0x02;
    my $padLen      = 0x06;
    my $sha1Len     = 0x14;
    
    open HEADER, ">$headerFile" or die;
    binmode HEADER;
    
    # �^�C���X�^���v�̏o��
#   my $timestamp = strftime "%y%m%d%H", localtime;
    my $timestamp = $ARGV[ 0 ];
    printf "timestamp = %s\n", $timestamp;
    syswrite( HEADER, pack( "N", unpack( "L", pack( "H8", $timestamp ) ) ) );
    
    # �v�f���̏o��
    syswrite( HEADER, pack( "S", $num ) );
    
    # �p�f�B���O�̏o��
    syswrite( HEADER, pack( "x$padLen") );
    
    # ���e�[�u����SHA1�n�b�V���̏o��
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

# �w�b�_�ւ̏����t��
{
    system ( "openssl dgst -sha1 -binary -out $digestFile $headerFile" );
    system ( "openssl rsautl -sign -in $digestFile -inkey $KEYROOT/keys/rsa/private_sharedFont.der -keyform DER -out $signFile" );
    system ( "cat $signFile $headerFile >$tempFile" );
    system ( "cat $tempFile $infoFile >$outFile" );
    deleteTemp();
}


# �t�H���g�̏o��
{
    open FONTTABLE, ">>$outFile" or die;
    binmode FONTTABLE;
    
    {
        # �p�f�B���O�o��
        my $fileLen = -s $outFile;
        my $padNum = ( $fileLen % 32 ) ? ( 32 - ( $fileLen % 32 ) ) : 0;
        my $padding = pack( "x$padNum" );
        syswrite( FONTTABLE, $padding, $padNum );
    }

    foreach ( @files ) {
        # NULL�w�莞�̓X�L�b�v
        if( "NULL" eq basename( $_ ) ) {
            next;
        }
        
        # �t�H���g�o��
        #my $fileLen = -s $_;
        #open TEST, $_ or die;
        my $compfile = "$_.comp";       # ���k�t�@�C�����o��
        $fileLen = -s $compfile;
        open TEST, $compfile or die;
        binmode TEST;
        sysread ( TEST, $buffer, $fileLen );
        close TEST;
        syswrite( FONTTABLE, $buffer, $fileLen );
        
        # �p�f�B���O�o��
        my $padNum = ( $fileLen % 32 ) ? ( 32 - ( $fileLen % 32 ) ) : 0;
        my $padding = pack( "x$padNum" );
        syswrite( FONTTABLE, $padding, $padNum );

        # ���k�t�@�C�����폜
        system ("rm -rf $compfile");
    }
    close FONTTABLE;
}
