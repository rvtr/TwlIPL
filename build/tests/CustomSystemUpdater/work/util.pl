
package util;

#-----------------------------------------------------------------------
# name     : get_title_version
# function : get the title version from the specified tad file
#-----------------------------------------------------------------------
sub get_title_version {

  my $titleVersion = 0 ;
  my $cmdPython = "/usr/bin/python.exe";
  my $scrpPython = "/usr/local/bin/mkcls.py";
  $tad = $_[0];

  open(PY, "$cmdPython $scrpPython $tad | ") or die "$!";
  $titleVersion = $1 if ( <PY> =~ /([0-9]{1,})/) ;
  close(PY);

  $titleVersion;
}

#-----------------------------------------------------------------------
# name     : get_nlist
# function : get the list of the specified directory except "." and ".."
#-----------------------------------------------------------------------
sub get_nlist {
  my $dirname = ( $#_ < 0) ? "." : $_[0];
  my @list;

  unless ( opendir(DIR, "$dirname") ) {
    print "Can't open directory: $dirname\n";
    return "";
  }

  foreach $node ( readdir(DIR) ) {
    push(@list, $node) unless ( $node =~ /^\.{1,2}$/ );
  }
  closedir(DIR);

  @list;
}


#-----------------------------------------------------------------------
# name     : fecheck
# function : check the specified file existence (Error if no file)
#-----------------------------------------------------------------------
sub fecheck {
  my $filename = $_[0];
  unless ( -e $filename ) {
print <<ERROR_NO_FILE;
[ # Error] No such file : $filename
ERROR_NO_FILE
    return 0;
  }
  1;
}

#-----------------------------------------------------------------------
# name     : fwcheck
# function : check the specified file existence (Warning if no file)
#-----------------------------------------------------------------------
sub fwcheck {
  my $filename = $_[0];
  unless ( -e $filename ) {
print <<WARN_NO_FILE;
[ = Warn ] No such file : $filename
WARN_NO_FILE
    return 0;
  }
  1;
}

sub get_filebody {
  my ($filename_body) = @_;
  $filename_body = $1 if ( $filename_body =~ /(.{1,})\.[^.]{1,}$/);
  $filename_body;
}

sub get_fileext {
  my ($filename_ext) = @_;
  $filename_ext = $1 if ( $filename_ext =~ /.{1,}\.([^.]{1,})$/);
  $filename_ext;
}

#-----------------------------------------------------------------------
# name     : cnv_str2hex
# function : check the specified file existence (Warning if no file)
#-----------------------------------------------------------------------
sub cnv_str2hex {
  my $instr = $_[0];
  my $outstr = "" ;
  my $i;

  my $len = ($#_<1) ? 1 : $_[1];

  #print (substr($instr, 0, 1));
  #print (substr($instr, 1, 1));
  for ( $i=0; $i<$len; $i++) {
    $outstr .= chr ( (hex(substr($_[0], $i*2, 1))<<4) 
                |    (hex(substr($_[0], $i*2+1, 1)) ) );
    #print $outstr."\n";
  } 
  return $outstr;
}

return 1;

__END__

