#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use File::Copy;

sub main()
{
  print "Building sphash project of splayer-pkg ...\n";
  my $result = system("\"C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\Common7\\IDE\\devenv.com\" ..\\thirdparty\\pkg\\trunk\\sphash\\sphash.sln /build \"Release|Win32\"");
  if ($result != 0)
  {
    die "sphash project build failed.\n";
  }
  copy("../thirdparty/pkg/trunk/unrar/unrar.hpp", "../thirdparty/pkg/") or die "file copy failed: $!";
  copy("../thirdparty/pkg/trunk/unrar/unrar.lib", "../thirdparty/pkg/") or die "file copy failed: $!";
  copy("../thirdparty/pkg/trunk/unrar/unrar.dll", "../../") or die "file copy failed: $!";
  copy("../thirdparty/pkg/trunk/sphash/release/sphash.lib", "../thirdparty/pkg/") or die "file copy failed: $!";
  copy("../thirdparty/pkg/trunk/sphash/release/sphash.dll", "../../") or die "file copy failed: $!";
  copy("../thirdparty/pkg/trunk/sphash/sphash/sphash.h", "../thirdparty/pkg/") or die "file copy failed: $!";
  print "\n";

  print "Building sinet project of sinet ...\n";
  $result = system("\"C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\Common7\\IDE\\devenv.com\" ..\\thirdparty\\sinet\\trunk\\sinet.sln /build \"Release|Win32\"");
  if ($result != 0)
  {
    die "sinet project build failed.\n";
  }
  copy("../thirdparty/sinet/trunk/release/sinet_dyn.lib", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/release/sinet_dyn_wrapper.lib", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/api_base.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/api_refptr.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/api_types.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/config.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/pool.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/postdata.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/postdataelem.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/request.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/sinet.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/task.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/sinet/task_observer.h", "../thirdparty/sinet/") or die "file copy failed: $!";
  copy("../thirdparty/sinet/trunk/release/sinet.dll", "../../") or die "file copy failed: $!";
  print "\n";

}

main();
