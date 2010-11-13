@echo off

echo Updating SPlayer SVN repo
svn_bin\svn update ..\

perl_bin\perl\bin\perl update_thirdparty.pl
perl_bin\perl\bin\perl build_thirdparty.pl
