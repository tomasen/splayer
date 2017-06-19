Function PinToTaskbar
        SetOutPath $TEMP
        File ..\Setup\pin.vbs
        ExecShell "open" "cscript" "/b /nologo $TEMP\pin.vbs pin $0" SW_HIDE
        ;ExecWait "cscript /b /nologo $TEMP\pin.vbs pin $0"
        ;delete pin.vbs
FunctionEnd

Function UnpinFromTaskbar
        SetOutPath $TEMP
        File ..\Setup\pin.vbs
        ExecShell "open" "cscript" "/b /nologo $TEMP\pin.vbs unpin $0" SW_HIDE
        ;ExecWait "cscript /b /nologo $TEMP\pin.vbs unpin $0"
        ;delete pin.vbs
FunctionEnd

Function un.UnpinFromTaskbar
        SetOutPath $TEMP
        File ..\Setup\pin.vbs
        ExecShell "open" "cscript" "/b /nologo $TEMP\pin.vbs unpin $0" SW_HIDE
        ;ExecWait "cscript /b /nologo $TEMP\pin.vbs unpin $0"
        ;delete pin.vbs
FunctionEnd