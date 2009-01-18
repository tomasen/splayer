
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SVPlayer"
  DeleteRegKey HKLM "SOFTWARE\SVPlayer"
  DeleteRegKey HKCU "SOFTWARE\SVPlayer"


  ; Remove files and uninstaller
  Delete /REBOOTOK $INSTDIR\svp*.*
  Delete /REBOOTOK $INSTDIR\mplayerc.*
  Delete /REBOOTOK $INSTDIR\Updater.exe
  Delete /REBOOTOK $INSTDIR\unrar.dll
  RMDir  /r /REBOOTOK $INSTDIR\codecs
  RMDir  $INSTDIR

  ; Remove shortcuts, if any
  Delete /REBOOTOK "$SMPROGRAMS\射手影音播放器\*.*"
  RMDir /REBOOTOK "$SMPROGRAMS\射手影音播放器"
  Delete /REBOOTOK "$QUICKLAUNCH\射手影音.lnk"
  Delete /REBOOTOK "$DESKTOP\射手影音.lnk"

  ; Remove directories used

  RMDir "$INSTDIR"