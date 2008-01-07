#!/bin/perl

use strict;


if( @ARGV != 8 ) {
	die "usage: makeHWInfo.pl -r <RegionName> -s <SerialNo> -k <RSA PrivKey> -o <outName>\n";
}

#引数チェック
my $cmd = &analyze_argv;

if( !exists($cmd->{"-r"}) or
	!exists($cmd->{"-s"}) or
	!exists($cmd->{"-k"}) or
	!exists($cmd->{"-o"}) ) {
	die "parameter error.\n";
}


#リージョン
my @regionNameList = qw/ JAPAN AMERICA EUROPE AUSTRALIA CHINA KOREA /;
my $i = 0;
my $region = 0xff;
foreach ( @regionNameList ) {
	if( $cmd->{"-r"} eq $_ ) {
		$region	= $i;
		last;
	}
	$i++;
}
if( $region == 0xff ) {
	die "region error.\n", $cmd->{"-r"};
}


#シリアルNo.
my $serialNo = $cmd->{"-s"};
if( !( 11 == length($serialNo) or ( 12 == length($serialNo) ) ) ) {
	die "serialNo length error. needs length \"11\" or \"12\"\n";
}


#Bodyファイル
open(OUT, ">body.bin");
binmode(OUT);
print OUT pack( "C", $region );
print OUT pack( "A16", $serialNo );
close(OUT);


#Headerファイル
my $version = 1;
my $length  = 17;
open(OUT, ">header.bin");
binmode(OUT);
print OUT pack( "C", $version );
print OUT pack( "CCC", 0, 0, 0 );
print OUT pack( "L", $length );
close(OUT);

#コンソール出力
print "[Header]\n";
printf " version  : %d\n", $version;
printf " length   : %d\n", $length;
print "[Body]\n";
printf " region   : %d (%s)\n", $region, $regionNameList[$region];
printf " serialNo : %s (%d)\n", $serialNo, length($serialNo);

#RSA署名
my $key = $cmd->{"-k"};
my $outName = $cmd->{"-o"};
system "openssl dgst -sha1 -binary -out tgt.dgst body.bin";
system "openssl rsautl -sign -in tgt.dgst -inkey $key -out tgt.sgn";
system "cat tgt.sgn header.bin body.bin >$outName";
system "rm header.bin body.bin tgt.dgst tgt.sgn";

exit;


sub analyze_argv
{
	my $opt;
	my %cmd = ();

	foreach my $token ( @ARGV )
	{
		if( substr($token, 0, 1) eq "-" )
		{
			$opt = $token;
		}
		elsif( $opt )
		{
			$cmd{$opt} = $token;
			$opt = "";
		}
		else
		{
			push @{$cmd{""}}, $token;
		}
	}

	return \%cmd;
}

