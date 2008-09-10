#!/usr/bin/perl

######################################################################
#	genFontTable.pl
#
#	generate Secure Shared Font Data Table
#
######################################################################

if ($#ARGV < 1) {
    printf STDOUT ("Usage: %s SRL_file tgtDir\n", $0);
    exit(-1);
}

open SRL, "<$ARGV[0]" or die;
binmode SRL;


# TitleID_Lo取得
sysseek SRL, 0x230, SEEK_SET;
my $data;
sysread( SRL, $data, 4 );

my @id = unpack( "cccc", $data);
my $titleID_Lo = pack( "cccc", $id[3], $id[2], $id[1], $id[0] );
#printf "%s\n", $titleID_Lo;


# ROMバージョン取得
sysseek SRL, 0x1e, SEEK_SET;
sysread( SRL, $data, 1 );
my $version = sprintf "%d\n", unpack( "c", $data );
#printf "%s\n", $version;

system( "mkdir -p $ARGV[1]/$titleID_Lo" );
system( "mkdir -p $ARGV[1]/$titleID_Lo/V$version" );

#TAD格納ディレクトリprint
printf "$ARGV[1]/$titleID_Lo/V$version";

close SRL

