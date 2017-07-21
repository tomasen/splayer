OutFile ColorfulErrors.exe
 
;includes 
 !include nsDialogs.nsh
 !include LogicLib.nsh
 
;variables 
 Var Dialog
 Var Directory
 Var ButtonClear
 Var ButtonReset
 
;definitions
 !define YELLOW "0xfff799"
 !define RED "0xf69679"
 
;pages
 Page custom nsDialogsPage nsDialogsLeave
 Page InstFiles
 
;functions
 Function nsDialogsPage
	nsDialogs::Create /NOUNLOAD 1018
	Pop $Dialog
 
	${If} $Dialog == error
		Abort
	${EndIf}
 
	${NSD_CreateLabel} 0 0 100% 70% "This example uses colors to indicate a state of a text-control. To understand the behaviour, please try out the following:$\n$\n$\t1. add or delete characters from the path$\n$\t2. click the clear button$\n$\t3. click reset$\n$\nAs you can see, the background color of the control changes depending on the input. A wrong path will indicate red, a missing input will indicate yellow. Only the correct path of $$DESKTOP will show the default color."
 
 	${NSD_CreateText} 0 70% 75% 20 "$DESKTOP"
	Pop $Directory
	${NSD_OnChange} $Directory OnChange
 
	${NSD_CreateButton} 78% 70% 10% 20 "Clear"
	Pop $ButtonClear
	${NSD_OnClick} $ButtonClear OnClickClear
 
	${NSD_CreateButton} 90% 70% 10% 20 "Reset"
	Pop $ButtonReset
	${NSD_OnClick} $ButtonReset OnClickReset
 
	${NSD_CreateLabel} 0 85% 100% 10% "PS: Unfortunately you *might* have to leave a control to see the effect!"
 
	nsDialogs::Show
 FunctionEnd 
 
 Function nsDialogsLeave
	${NSD_GetText} $Directory $0
 
	${If} $0 == ""
		MessageBox MB_YESNO|MB_ICONEXCLAMATION "Enter the path to your Desktop. Ignore?" IDYES +2
		Abort
	${ElseIfNot} $0 == "$DESKTOP"
		MessageBox MB_YESNO|MB_ICONEXCLAMATION "Not the path to your Desktop. Ignore?" IDYES +2
		Abort
	${EndIf}
 FunctionEnd
 
 Function OnChange
	Pop $0 # HWND
	${NSD_GetText} $Directory $0
 
	${If} $0 == ""
		SetCtlColors $Directory "" ${YELLOW}
	${Else}		
		${If} ${FileExists} "$0\*.*"
			SetCtlColors $Directory "" "" ;reset colors
		${Else}
			SetCtlColors $Directory "" ${RED}
		${EndIf}
	${EndIf}	
 FunctionEnd
 
 Function OnClickClear
	Pop $0 # HWND
	${NSD_SetText} $Directory ""
 FunctionEnd
 
 Function OnClickReset
	Pop $0 # HWND
	${NSD_SetText} $Directory "$DESKTOP"
 FunctionEnd
 
;(dummy) section
 Section
	Quit
 SectionEnd