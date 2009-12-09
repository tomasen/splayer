del /F /S /Q bin
msbuild svplayer.wixproj /property:Configuration=Release /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.log;verbosity=detailed /t:Clean,Build 
rem msbuild svplayer.wixproj /property:Configuration=ReleaseJPN /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.jp.log;verbosity=detailed /t:Clean,Build 
msbuild svplayer.wixproj /property:Configuration=ReleaseCHS /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.ch.log;verbosity=detailed /t:Clean,Build 
