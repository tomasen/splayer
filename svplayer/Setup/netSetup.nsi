!include "inc.Var.nsi"

Function .onInit
 !include "inc.OnInt.nsi"

FunctionEnd

OutFile "..\..\NetSetup.exe"

; Uninstaller

Section "Uninstall"
  
  !include "inc.uninstall.nsi"
  
SectionEnd




!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${mplayerc} "播放器主程序，必须安装"

!insertmacro MUI_FUNCTION_DESCRIPTION_END
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS