#!/usr/bin/perl
use strict;
use warnings;
use utf8;

sub main()
{
  print "Updating splayer-pkg project ...\n";
  my $result = system("hg_bin/hg pull -u ../thirdparty/pkg");
  my $result2 = system("hg_bin/hg update -c -R ../thirdparty/pkg");
  if ($result != 0 || $result2 != 0)
  {
    die "splayer-pkg project update failed.\n";
  }
  print "\n";

  print "Updating sinet project ...\n";
  $result = system("hg_bin/hg pull -u ../thirdparty/sinet");
  $result2 = system("hg_bin/hg update -c -R ../thirdparty/sinet");
  if ($result != 0 || $result2 != 0)
  {
    die "sinet project update failed.\n";
  }
  print "\n";
}

main();
