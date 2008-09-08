#!/usr/bin/perl -w -I/usr/local/bin
use File::Copy;
use File::Copy::Recursive qw(fcopy rcopy dircopy);
use Class::Struct;
use POSIX 'strftime';
use File::Find;

require "util.pl";

my @apps = ("download_play", 
            "full_browser", 
            "nintendo_spot", 
            "photo", 
            "pictchat", 
            "shop", 
            "sound",
	    "sysmenu");

my $flgDate = 0;

my $i;
my $date_dir = "";
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
my $str_today = sprintf("%04d%02d%02d", $year+1900, $mon+1, $mday);
my $flghelp = 0;
my $err;
my @list_tad;


for ($i = 0; $i < $#ARGV+1; $i++) {

  my $opt = $ARGV[$i];

  if ( $opt eq "-d" ) {
    $flgDate = 1;
    unless ( $#ARGV <= $i ) {
      #$date_dir = $aqua_dir."/SD_".$ARGV[$i+1];
      $str_today = $ARGV[$i+1];
      $err = [ 1, "Not date (\"$date_dir\") !!"]  if ( $date_dir !~ /[0-9]{8}/ );
      $i++;

    } else {
      $err = [1,"No argument after -d !!"];
    }
  } elsif ( $opt eq "-h" ) {
    &show_help;
    exit 0;
  } else {
    print "Unknown option: $opt\n";
    &show_help;
    exit 1;
  }
  do {
    print "*** Error *** : $err->[1]";
    exit 1;
  } if ( $err->[0] );
}

# each application directory
foreach $app ( @apps ) 
{
  my $dir_date;
  #$dir_app = sprintf('//Aqua/TWL_debug/%s/rom/tad', $app);
$dir_app = sprintf('//10.116.1.5/TWL_debug/%s/rom/tad', $app);
  $dir_app_date = &search_date_dir($dir_app);

  print "* $dir_app\n   - $dir_app_date\n\n";
  
  find( sub 
        {
	  do {
	push @list_tad, $File::Find::name }if (/\.tad$/);
      }, $dir_app_date
    );
  #foreach ( @list_tad ) { print "   * $_\n"; }
}

#print "Date: $date_dir\n";

# Actually not today
mkdir($str_today);

# copy to the local date directory
foreach ( @list_tad ) {
  print("$_\n");
  copy($_, $str_today);
}

# 
exit 0;

#-----------------------------------------------------------------------
# name     : show_help
# function :
#-----------------------------------------------------------------------
sub show_help 
{
print <<__MSG_HELP__;
Usage:
\$ perl pickup_tad.pl [-d yyyymmdd]
__MSG_HELP__
}

#-----------------------------------------------------------------------
# name     : search_date_dir
# function :
#-----------------------------------------------------------------------
sub search_date_dir 
{
  my $root_dir;
  my $date_dir;
  my $prefix = ($#_<1) ? "" : $_[1];
  my $full_prefix;
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

  $year += 1900;
  $mon  += 1;

  $root_dir =  ( $#_ < 0 ) ? "." : $_[0];

  $full_prefix = $root_dir."/".$prefix;
  $date_dir = $full_prefix.sprintf("%04d%02d%02d", $year, $mon, $mday);

  # search directory on going back date
  while ( $year >= 2000 ) { 
    if ( -e "$date_dir" and -d "$date_dir") {
      return "$date_dir";
    } else {
      $mday -= 1;
      $date_dir = $full_prefix.sprintf("%04d%02d%02d", $year, $mon, $mday);
    }
    do {
      $mday = 31; # constant
      $mon -= 1;
      unless ( $mon ) {
        $mon = 12;
	$year -= 1;
      }
    } unless ( $mday ) ;
  }
  print "*** Error *** : Can't find the date directory !!\n";
  return 0;
}

__END__

