;
; OCSetupHlp.nsh
; --------------
;
; OpenCandy Helper Include File
;
; This file defines a few macros that need to be called
; from your main installer script in order to initialize
; and setup OpenCandy.
;
; Please consult the accompanying SDK documentation for
; integration details and contact partner support for
; assistance with any advanced integration needs.
;
; IMPORTANT:
; ----------
; Publishers should have no need to modify the content
; of this file. If you are modifying this file for any
; reason other than as directed by partner support
; you are probably making a mistake. Please contact
; partner support instead.
;
; Note: When using the Unicode NSIS compiler this file
;       must be re-saved with UTF-16LE/UCS-2LE encoding
;       for versions of Unicode NSIS up to 2.42.3,
;       and either UTF16-LE/UCS2LE or UTF8 encoding
;       for versions thereafter.
;
; Copyright (c) 2008 - 2011 OpenCandy, Inc.
;

!ifdef _OCSETUPHLP_NSH
!warning "OCSetupHlp.nsh is included more than once"
!else
!define _OCSETUPHLP_NSH



;--------------------------------
; Include and prepare NSIS libraries used
;--------------------------------

!include FileFunc.nsh
!include nsDialogs.nsh
!insertmacro GetParameters



;--------------------------------
; OpenCandy definitions
;--------------------------------

; Size of strings (including terminating character)
!define OC_STR_CHARS 1024

; Values used with OCInit2A(), OCInit2W() APIs
!define OC_INIT_SUCCESS        0
!define OC_INIT_ASYNC_ENABLE   1
!define OC_INIT_ASYNC_DISABLE  0
!define OC_INIT_MODE_NORMAL    0
!define OC_INIT_MODE_REMNANT   1

; Values used with OCGetNoCandy() API
!define OC_CANDY_ENABLED  0
!define OC_CANDY_DISABLED 1

; Offer types returned by OCGetOfferType() API
!define OC_OFFER_TYPE_NORMAL   1
!define OC_OFFER_TYPE_EMBEDDED 2

; User choice indicators returned by OCGetOfferState() API
!define OC_OFFER_CHOICE_ACCEPTED  1
!define OC_OFFER_CHOICE_DECLINED  0
!define OC_OFFER_CHOICE_NOCHOICE -1

; Values used with OCCanLeaveOfferPage() API
!define OC_OFFER_LEAVEPAGE_ALLOWED    1
!define OC_OFFER_LEAVEPAGE_DISALLOWED 0

; Values used for OCGetAsyncOfferStatus() API
!define OC_OFFER_STATUS_CANOFFER_READY    0
!define OC_OFFER_STATUS_CANOFFER_NOTREADY 1
!define OC_OFFER_STATUS_QUERYING_NOTREADY 2
!define OC_OFFER_STATUS_NOOFFERSAVAILABLE 3
!define OC_STATUS_QUERY_GENERAL 0
!define OC_STATUS_QUERY_DETERMINESOFFERENABLED 1

; Values returned by OCGetBannerInfo() API
!define OC_OFFER_BANNER_FOUNDNEITHER     0
!define OC_OFFER_BANNER_FOUNDTITLE       1
!define OC_OFFER_BANNER_FOUNDDESCRIPTION 2
!define OC_OFFER_BANNER_FOUNDBOTH        3

; Values returned by OCRunDialog() API
!define OC_OFFER_RUNDIALOG_FAILURE -1

; Values for LogDevModeMessage() API
!define OC_DEVMSG_ERROR_TRUE  1
!define OC_DEVMSG_ERROR_FALSE 0

; Values used for SetOCOfferEnabled() API
!define OC_OFFER_ENABLE_TRUE  1
!define OC_OFFER_ENABLE_FALSE 0

; Values returned by OCLoadOpenCandyDLL() API
!define OC_LOADOCDLL_FAILURE 0

; Flags used in NSIS script
!define OC_NSIS_TRUE  1
!define OC_NSIS_FALSE 0

; Values used in the sample installer script
;
; IMPORTANT:
; Do not modify these definitions or disable the warnings below.
; If you see warnings when you compile your script you must
; modify the values you set where you !insertmacro OpenCandyInit
; (i.e. in your .nsi file) before releasing your installer publicly.
!define OC_SAMPLE_PUBLISHERNAME "Open Candy Sample"
!define OC_SAMPLE_KEY "748ad6d80864338c9c03b664839d8161"
!define OC_SAMPLE_SECRET "dfb3a60d6bfdb55c50e1ef53249f1198"



;--------------------------------
; OpenCandy global variables
;--------------------------------

; IMPORTANT:
; Never modify or reference these directly, they are used
; completely internally to this helper script.
Var OCHasBeenInitialized
Var OCNoCandy
Var OCUseOfferPage
Var OCProductInstallSuccess
Var OCAttached
Var OCHasReachedOCPage


;--------------------------------
; OpenCandy functions and macros
;--------------------------------

;
; OpenCandy API check
; -------------------
; Perform a basic sanity check to ensure that your script
; inserts all mandatory API calls. Any failures will be
; shown as warnings from the compiler at compile time.
;
; To perform this check insert the OpenCandyAPIDoChecks
; macro at the very bottom of your .nsi file.
;
; The _OpenCandyAPIInserted and _OpenCandyAPICheckInserted
; macros are internal to this helper script, do not insert
; them in your code.
;
; Usage:
;
;   # Have the compiler perform some basic OpenCandy API implementation checks
;   # This macro must be inserted at the very end of the .nsi script,
;   # outside any other code blocks.
;   !insertmacro OpenCandyAPIDoChecks
;

!macro _OpenCandyAPIInserted APINAME
	; Add a definition to note insertion of API name
	!ifndef _OC_APICHECK_INSERTED_${APINAME}
		!define _OC_APICHECK_INSERTED_${APINAME}
	!endif
!macroend

!macro _OpenCandyAPICheckInserted APINAME WARNMSG
	; Check an API name was inserted, show a warning otherwise
	!ifndef _OC_APICHECK_INSERTED_${APINAME}
		!if "${WARNMSG}" != ""
			!warning "Did not find reference to required API ${APINAME}. ${WARNMSG}"
		!else
			!warning "Did not find reference to required API ${APINAME}."
		!endif
	!endif
!macroend

!macro OpenCandyAPIDoChecks
	; Check only for mandatory API names
	!insertmacro _OpenCandyAPICheckInserted _OpenCandyInitInternal "Check that you have inserted OpenCandyAsyncInit in your .onInit callback function, after any language selection code."
	!insertmacro _OpenCandyAPICheckInserted OpenCandyOfferPage     "Check that you have inserted OpenCandyOfferPage in your list of installer pages (e.g. before instfiles)."
	!insertmacro _OpenCandyAPICheckInserted OpenCandyOnInstSuccess "Check that you have inserted OpenCandyOnInstSuccess in your .onInstSuccess callback function."
	!insertmacro _OpenCandyAPICheckInserted OpenCandyOnGuiEnd      "Check that you have inserted OpenCandyOnGuiEnd in your .onGUIEnd callback function."
	!insertmacro _OpenCandyAPICheckInserted OpenCandyInstallDll    "Check that you have inserted OpenCandyInstallDll in a section that will run during your product installation."
!macroend



;
; OpenCandyReserveFile
; --------------------
; Insert this macro as close to the top of your .nsi file as possible
; after including this header, before other functions and sections that
; include "File" directives. This will reserve a place early in the
; file datablock for OCSetupHlp.dll. Because the DLL is required in
; the .onInit callback function the reservation will make your installer
; launch faster and allow more time for fetching offer information.
;
;   ; Improve startup performance and increase offer fetch time by
;   ; reserving an early place in the file data block for OpenCandy DLL.
;   !insertmacro OpenCandyReserveFile
;

!macro OpenCandyReserveFile
	ReserveFile "${OC_OCSETUPHLP_FILE_PATH}"
!macroend



;
; _OpenCandyDevModeMsg
; --------------------
; This macro is internal to this helper script. Do not
; insert it in your own code.
;
; The macro code is intentionally unguarded with respect
; to $OCHasBeenInitialized and $OCHasBeenInitialized. Calling
; code is responsible for ensuring appropriate conditions.
;
; Parameters:
;
;   OC_DEV_MSG   : Message to display (string)
;   OC_DEV_ERROR : The message represents an error OC_NSIS_TRUE or OC_NSIS_FALSE
;   OC_FAQ_ID    : ID of the FAQ associated with the message, or 0 if there is no FAQ associated (int)
;
; Usage:
;
;   !insertmacro _OpenCandyDevModeMsg "This is an error with associated FAQ #500" ${OC_NSIS_TRUE} 500
;

!macro _OpenCandyDevModeMsg OC_DEV_MSG OC_DEV_ERROR OC_FAQ_ID
	Push $0
	StrCpy $0 "${OC_DEV_MSG}"
	!if ${OC_DEV_ERROR} == ${OC_NSIS_TRUE}
		!define OC_TMP_DEV_ERROR ${OC_DEVMSG_ERROR_TRUE}
		StrCpy $0 "{\rtf1 {\colortbl;\red0\green0\blue0;\red255\green0\blue0;}\cf2Status ERROR! \cf1 $0\par}"
	!else if ${OC_DEV_ERROR} == ${OC_NSIS_FALSE}
		!define OC_TMP_DEV_ERROR ${OC_DEVMSG_ERROR_FALSE}
	!else
		!error "Value for OC_DEV_ERROR must be either OC_NSIS_TRUE or OC_NSIS_FALSE!"
	!endif
	!ifdef NSIS_UNICODE
		System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403LogDevModeMessageW(w, i, i)(r0, ${OC_TMP_DEV_ERROR}, ${OC_FAQ_ID})? c"
	!else
		System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403LogDevModeMessage(m, i, i)(r0, ${OC_TMP_DEV_ERROR}, ${OC_FAQ_ID})? c"
	!endif
	Pop $0
	!undef OC_TMP_DEV_ERROR
!macroend


;
; _OpenCandyInitChecksAndDefaults
; -------------------------------
; This macro is internal to this helper script. Do not
; insert it in your own code.
;

!macro _OpenCandyInitChecksAndDefaults
	!ifdef _OpenCandyInitChecksAndDefaults
		!error "_OpenCandyInitChecksAndDefaults inserted more than once"
	!else
		!define _OpenCandyInitChecksAndDefaults

		; Compile-time checks and defaults
		!if "${PublisherName}" == "${OC_SAMPLE_PUBLISHERNAME}"
			!warning "Do not forget to change the product name from '${OC_SAMPLE_PUBLISHERNAME}' to your company's product name before releasing this installer."
		!endif
		!if "${Key}" == "${OC_SAMPLE_KEY}"
			!warning "Do not forget to change the sample key '${OC_SAMPLE_KEY}' to your company's product key before releasing this installer."
		!endif
		!if "${Secret}" == "${OC_SAMPLE_SECRET}"
			!warning "Do not forget to change the sample secret '${OC_SAMPLE_SECRET}' to your company's product secret before releasing this installer."
		!endif

		; DLL relocation support
		!ifndef OC_OCSETUPHLP_FILE_PATH
			!warning "Do not forget to define OC_OCSETUPHLP_FILE_PATH in your script. Defaulting to 'OCSetupHlp.dll'."
			!define OC_OCSETUPHLP_FILE_PATH ".\OCSetupHlp.dll"
		!endif

		; OC_MAX_INIT_TIME is the maximum time in milliseconds that OCInit may block when fetching offers.
		; Note that under normal network conditions OCInit may return sooner. Setting this value too low
		; may reduce offer rate. Values of 5000 or greater are recommended. If you intend to override this
		; default do so by defining it in your .nsi file before #include'ing this header. Be certain to
		; make OpenCandy partner support aware of any override you apply because this can affect your metrics.
		!ifndef OC_MAX_INIT_TIME
			!define OC_MAX_INIT_TIME 8000
		!endif

	!endif
!macroend



;
; _OpenCandyInitInternal
; ----------------------
; This macro is internal to this helper script. Do not
; insert it in your own code. Instead see OpenCandyInit
; and OpenCandyAsyncInit.
;

!macro _OpenCandyInitInternal
!insertmacro _OpenCandyAPIInserted _OpenCandyInitInternal
${If} $OCHasBeenInitialized <> ${OC_NSIS_TRUE}

	Exch $0 ; Publisher name
	Exch 1
	Exch $1 ; Key
	Exch 2
	Exch $2 ; Secret
	Exch 3
	Exch $3 ; Enable Async
	Exch 4
	Exch $4 ; Init mode
	Push $5 ; Language
	StrCpy $5 $LANGUAGE
	
	; Check macro definitions and enforce defaults
	!insertmacro _OpenCandyInitChecksAndDefaults

	; Initialize OpenCandy variables
	StrCpy $OCNoCandy ${OC_NSIS_FALSE}
	StrCpy $OCUseOfferPage ${OC_NSIS_FALSE}
	StrCpy $OCHasReachedOCPage ${OC_NSIS_FALSE}
	StrCpy $OCHasBeenInitialized ${OC_NSIS_FALSE}
	StrCpy $OCProductInstallSuccess ${OC_NSIS_FALSE}
	StrCpy $OCAttached ${OC_NSIS_FALSE}

	; Unpack and load the OpenCandy SDK DLL
	InitPluginsDir
	IfFileExists "$PLUGINSDIR\OCSetupHlp.dll" OCDLLExists
	File "/oname=$PLUGINSDIR\OCSetupHlp.dll" "${OC_OCSETUPHLP_FILE_PATH}"
	OCDLLExists:
	ClearErrors
	Push $0
	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403LoadOpenCandyDLL()i().r0? c"
	IfErrors OCDLLoadFailed               ; System plugin failed
	${if} $0 == error                     ; API call failed
	${orif} $0 == ${OC_LOADOCDLL_FAILURE} ; API returned failure
		Goto OCDLLoadFailed
	${endif}
	Goto OCDLLoadSuccess
	OCDLLoadFailed:
	StrCpy $OCNoCandy ${OC_NSIS_TRUE}
	OCDLLoadSuccess:
	Pop $0

	; Handle command line options and silent installations
	${if} $OCNoCandy == ${OC_NSIS_FALSE}
		Push $0
		Push $1
		${GetParameters} $0
		IfSilent 0 OCNotSilent ; Force /NOCANDY if this is a silent install
		StrCpy $0 "/NOCANDY"
		OCNotSilent:

		!ifdef NSIS_UNICODE
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403SetCmdLineValuesW(w)i(r0).r1? c"
		!else
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403SetCmdLineValues(m)i(r0).r1? c"
		!endif

		System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetNoCandy()i().r1? c"
		${if} $1 == ${OC_CANDY_DISABLED}
			StrCpy $OCNoCandy ${OC_NSIS_TRUE}
		${endif}
		Pop $1
		Pop $0
	${endif}

	; Pass publisher details to OpenCandy API
	${if} $OCNoCandy == ${OC_NSIS_FALSE}
		${if} $4 == ${OC_NSIS_TRUE}
			StrCpy $4 ${OC_INIT_ASYNC_ENABLE}
		${elseif} $4 == ${OC_NSIS_FALSE}
			StrCpy $4 ${OC_INIT_ASYNC_DISABLE}
		${else}
			!insertmacro _OpenCandyDevModeMsg "OCSetupHlp.nsh - Init: Value for Async must be either OC_NSIS_TRUE or OC_NSIS_FALSE." ${OC_NSIS_TRUE} 0
			StrCpy $4 ${OC_INIT_ASYNC_ENABLE} ; Failsafe
		${endif}

		!ifdef NSIS_UNICODE
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403Init2W(w, w, w, w, i, i, i)i(r0, r1, r2, r5, r3, ${OC_MAX_INIT_TIME}, r4).s? c"
		!else
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403Init2A(m, m, m, m, i, i, i)i(r0, r1, r2, r5, r3, ${OC_MAX_INIT_TIME}, r4).s? c"
		!endif

		Exch $0
		${if} $0 == ${OC_INIT_SUCCESS}
			StrCpy $OCHasBeenInitialized ${OC_NSIS_TRUE}
		${endif}
		Pop $0
	${endif}
	
	Pop $5
	Pop $4
	Pop $0
	Pop $1
	Pop $2
	Pop $3
${endif}
!macroend



;
; OpenCandyInit (Deprecated) / OpenCandyAsyncInit
; -----------------------------------------------
; Performs initialization of the OpenCandy DLL
; and checks for offers to present.
;
; Parameters:
;
;   PublisherName : Your publisher name (will be provided by OpenCandy)
;   Key           : Your product key (will be provided by OpenCandy)
;   Secret        : Your product secret (will be provided by OpenCandy)
;   InitMode      : The operating mode. Pass OC_INIT_MODE_NORMAL for normal operation
;                   or OC_INIT_MODE_REMNANT if OpenCandy should not show offers. Do not
;                   use InitMode to handle /NOCANDY, this is done automatically for you.
;
; Usage (Using sample values for internal testing purposes only):
;
;   ; Initialize OpenCandy, check for offers
;   ;
;   ; Note: If you use a language selection system,
;   ; e.g. MUI_LANGDLL_DISPLAY or calls to LangDLL, you must insert
;   ; this macro after the language selection code in order for
;   ; OpenCandy to detect the user-selected language.
;   !insertmacro OpenCandyAsyncInit "Open Candy Sample" 748ad6d80864338c9c03b664839d8161 dfb3a60d6bfdb55c50e1ef53249f1198 ${OC_INIT_MODE_NORMAL}
;

!macro OpenCandyInit PublisherName Key Secret InitMode
	Push ${InitMode}
	Push ${OC_NSIS_FALSE} ; Enable Async?
	Push ${Secret}
	Push ${Key}
	Push "${PublisherName}" 
	!insertmacro _OpenCandyInitInternal
!macroend

!macro OpenCandyAsyncInit PublisherName Key Secret InitMode
	Push ${InitMode}
	Push ${OC_NSIS_TRUE} ; Enable Async?
	Push ${Secret}
	Push ${Key}
	Push "${PublisherName}" 
	!insertmacro _OpenCandyInitInternal
!macroend



;
; SetOCOfferEnabled
; -----------------
; Allows you to disable the OpenCandy offer screen easily from your
; installer code. Note that this is not the recommended method - you
; ought to determine during initialization whether OpenCandy should be
; disabled and specify an appropriate mode when inserting OpenCandyInit
; or OpenCandyAsyncInit in that case. If you must use this method please
; be sure to inform the OpenCandy partner support team. Never directly
; place logical conditions around other OpenCandy functions and macros because
; this can have unforseen consequences. You should call this procedure only
; after calling OpenCandyInit / OpenCandyAsyncInit.
;
; Usage:
;
;  # This turns off offers from the OpenCandy network
;  !insertmacro SetOCOfferEnabled ${OC_NSIS_FALSE}
;

!macro SetOCOfferEnabled OC_OFFER_ENABLED
${If} $OCHasBeenInitialized == ${OC_NSIS_TRUE}
${AndIf} $OCNoCandy == ${OC_NSIS_FALSE}
	!if ${OC_OFFER_ENABLED} == ${OC_NSIS_TRUE}
		!define OC_TMP_OFFER_ENABLED ${OC_OFFER_ENABLE_TRUE}
	!else if ${OC_OFFER_ENABLED} == ${OC_NSIS_FALSE}
		!define OC_TMP_OFFER_ENABLED ${OC_OFFER_ENABLE_FALSE}
	!else
		!error "Value for OC_OFFER_ENABLED must be either OC_NSIS_TRUE or OC_NSIS_FALSE!"
	!endif
	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403SetOCOfferEnabled(i)(${OC_TMP_OFFER_ENABLED})? c"
	!undef OC_TMP_OFFER_ENABLED
${EndIf}
!macroend



;
; GetOCOfferStatus
; ----------------
; Allows you to determine if an offer is currently available. This is
; done automatically for you before the offer screen is shown. Typically
; you do not need to call this function from your own code directly.
;
; The offer status is placed on the stack and may be one of:
; ${OC_OFFER_STATUS_CANOFFER_READY}    - An OpenCandy offer is available and ready to be shown
; ${OC_OFFER_STATUS_CANOFFER_NOTREADY} - An offer is available but is not yet ready to be shown
; ${OC_OFFER_STATUS_QUERYING_NOTREADY} - The remote API is still being queried for offers
; ${OC_OFFER_STATUS_NOOFFERSAVAILABLE} - No offers are available
;
; When using the macro you must indicate whether the information returned
; will be used to decide whether the OpenCandy offer screen will be shown, e.g.
; if the information may result in use of the SetOCOfferEnabled macro. This helps
; to optimize future OpenCandy SDKs for better performance with your product.
;
; Usage:
;
;   # Test if OpenCandy is ready to show an offer.
;   # Indicate the result is informative only and does not directly
;   # determine whether offers from OpenCandy are enabled.
;   !insertmacro GetOCOfferStatus ${OC_NSIS_FALSE}
;   Pop <result>
;

!macro GetOCOfferStatus OC_DETERMINES_OFFER_ENABLED
${If} $OCHasBeenInitialized == ${OC_NSIS_TRUE}
${AndIf} $OCNoCandy == ${OC_NSIS_FALSE}
	!if ${OC_DETERMINES_OFFER_ENABLED} == ${OC_NSIS_TRUE}
		!define OC_TMP_DETERMINES_OFFER_ENABLED ${OC_STATUS_QUERY_DETERMINESOFFERENABLED}
	!else if ${OC_DETERMINES_OFFER_ENABLED} == ${OC_NSIS_FALSE}
		!define OC_TMP_DETERMINES_OFFER_ENABLED ${OC_STATUS_QUERY_GENERAL}
	!else
		!error "Value for OC_DETERMINES_OFFER_ENABLED must be either OC_NSIS_TRUE or OC_NSIS_FALSE!"
	!endif
	Push $0
	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetAsyncOfferStatus(i)i(${OC_TMP_DETERMINES_OFFER_ENABLED}).r0? c"
	Exch $0
	!undef OC_TMP_DETERMINES_OFFER_ENABLED
${Else}
	Push ${OC_OFFER_STATUS_NOOFFERSAVAILABLE}
${EndIf}
!macroend



;
; OpenCandyOfferPage
; ------------------
; Inserts the OpenCandy offer page. The offer page will show
; only if an offer is available and ready to be shown when
; the user reaches the page.
;
; Usage:
;
;   # Insert the OpenCandy offer page
;   !insertmacro OpenCandyOfferPage
;

!macro OpenCandyOfferPage
	!insertmacro _OpenCandyAPIInserted OpenCandyOfferPage
	PageEx custom
		PageCallbacks _OpenCandyPageStartFn _OpenCandyPageLeaveFn
	PageExEnd
!macroend


;
; _OpenCandyPageStartFn
; ---------------------
; Decides if there is an offer to show and if so sets up
; the offer page for NSIS.
;
; Do not call this function directly, it is a callback used
; by the PageEx that is added to your .nsi script.
;
; Usage:
;
;   # Insert the OpenCandy offer page
;   !insertmacro OpenCandyOfferPage
;

Function _OpenCandyPageStartFn
${If} $OCHasBeenInitialized == ${OC_NSIS_TRUE}
${AndIf} $OCNoCandy == ${OC_NSIS_FALSE}

	Push $0

	${If} $OCAttached == ${OC_NSIS_TRUE}
		System::Call "OCSetupHlp::OCDetach()i.r0? c"
		StrCpy $OCAttached ${OC_NSIS_FALSE}
	${EndIf}

	${If} $OCHasReachedOCPage == ${OC_NSIS_FALSE}
		System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetAsyncOfferStatus(i)i(${OC_STATUS_QUERY_DETERMINESOFFERENABLED}).r0? c"
		${If} $0 == ${OC_OFFER_STATUS_CANOFFER_READY}
			StrCpy $OCUseOfferPage ${OC_NSIS_TRUE}
		${EndIf}
	${EndIf}
	
	StrCpy $OCHasReachedOCPage ${OC_NSIS_TRUE}
	
	${If} $OCUseOfferPage == ${OC_NSIS_TRUE}
		; Call OCGetBannerInfo to get title and description strings for the offer screen
		!ifdef NSIS_UNICODE
			!define OC_TMP_STRTYPE w
		!else
			!define OC_TMP_STRTYPE m
		!endif
		Push $1
		Push $2
		Push $3
		Push $4
		System::Call "*(&${OC_TMP_STRTYPE}${OC_STR_CHARS} '') i .r3" ; $1 points to OC_STR_CHARS chars of initialized memory
		System::Call "*(&${OC_TMP_STRTYPE}${OC_STR_CHARS} '') i .r4" ; $2 points to OC_STR_CHARS chars of initialized memory
		!ifdef NSIS_UNICODE
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetBannerInfoW(i, i)i(r3, r4).r0? c"
		!else
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetBannerInfo(i, i)i(r3, r4).r0? c"
		!endif

		; Override defaults based on what OCGetBannerInfo has filled
		StrCpy $1 " "
		StrCpy $2 " "
		${If} $0 == ${OC_OFFER_BANNER_FOUNDBOTH}
			System::Call "*$3(&${OC_TMP_STRTYPE}${OC_STR_CHARS} .s)"
			Pop $1
			System::Call "*$4(&${OC_TMP_STRTYPE}${OC_STR_CHARS} .s)"
			Pop $2
		${ElseIf} $0 == ${OC_OFFER_BANNER_FOUNDTITLE}
			System::Call "*$3(&${OC_TMP_STRTYPE}${OC_STR_CHARS} .s)"
			Pop $1
		${ElseIf} $0 == ${OC_OFFER_BANNER_FOUNDDESCRIPTION}
			System::Call "*$4(&${OC_TMP_STRTYPE}${OC_STR_CHARS} .s)"
			Pop $2
		${EndIf}
		System::Free $4
		System::Free $3
		Pop $4
		Pop $3
		!undef OC_TMP_STRTYPE

		; Create and show offer page
		nsDialogs::Create /NOUNLOAD 1018
		Exch $3
		${If} $3 == error
			StrCpy $OCUseOfferPage ${OC_NSIS_FALSE}
			Pop $3
			Pop $2
			Pop $1
			Pop $0
			Abort
		${Else}
			!insertmacro MUI_HEADER_TEXT $1 $2
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403NSISAdjust(i, i, i, i, i)i(r3, 14, 70, 470, 228).r0? c"
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403RunDialog(i)i(r3).r0? c"
			${If} $0 != ${OC_OFFER_RUNDIALOG_FAILURE}
				StrCpy $OCAttached ${OC_NSIS_TRUE}
				nsDialogs::Show
			${EndIf}
		${EndIf}
		Pop $3
		Pop $2
		Pop $1
	${Else}
		; No offer was not ready to be shown when the user reached the offer page
		StrCpy $OCUseOfferPage ${OC_NSIS_FALSE}
	${EndIf}

	Pop $0

${Else}
	StrCpy $OCUseOfferPage ${OC_NSIS_FALSE}
	Abort
${EndIf}
FunctionEnd



;
; _OpenCandyPageLeaveFn
; ---------------------
; Decides if it is ok for the end user to leave the offer
; page and continue with setup.
;
; Do not call this function directly, it is a callback used
; by the PageEx that is added to your .nsi script.
;
; Usage:
;
;   # Insert the OpenCandy offer page
;   !insertmacro OpenCandyOfferPage
;

Function _OpenCandyPageLeaveFn
	Push $0
	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403CanLeaveOfferPage()i.r0? c"
	${If} $0 == ${OC_OFFER_LEAVEPAGE_DISALLOWED} ; If user must choose an option before proceeding
		Pop $0
		Abort
	${EndIf}
	${If} $OCAttached == ${OC_NSIS_TRUE}
		System::Call "OCSetupHlp::OCDetach()i.r0? c"
		StrCpy $OCAttached ${OC_NSIS_FALSE}
	${EndIf}
	Pop $0
FunctionEnd



;
; _OpenCandyExecOfferInternal
; ---------------------------
; This macro is internal to this helper script. Do not
; insert it in your own code. Instead see OpenCandyOnInstSuccess
; and OpenCandyInstallEmbedded.
;
; Parameters:
;
;   OC_OFFER_TYPE     : The offer type, OC_OFFER_TYPE_NORMAL or OC_OFFER_TYPE_EMBEDDED
;
; Usage:
;
;   # DO NOT USE _OpenCandyExecOfferInternal directly!
;   !insertmacro _OpenCandyExecOfferInternal ${OC_OFFER_TYPE_NORMAL}
;

!macro _OpenCandyExecOfferInternal OC_OFFER_TYPE
!if "${OC_OFFER_TYPE}" != "${OC_OFFER_TYPE_NORMAL}"
	!if "${OC_OFFER_TYPE}" != "${OC_OFFER_TYPE_EMBEDDED}"
		!error "Unsupported offer type '${OC_OFFER_TYPE}' in _OpenCandyExecOfferInternal"
	!endif
!endif
	Push $0
	${If} $OCUseOfferPage == ${OC_NSIS_TRUE}
		System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetOfferType() i.r0? c"
		${If} $0 == ${OC_OFFER_TYPE}
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403PrepareDownload()i().r0? c"
			System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403GetOfferState()i().r0? c"
			${If} $0 == ${OC_OFFER_CHOICE_ACCEPTED}
				System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403StartDLMgr2Download()? c"
			${EndIf}
		${EndIf}
	${EndIf}
	Pop $0
!macroend



;
; OpenCandyOnInstSuccess
; ----------------------
;
; This macro needs to be inserted in the .onInstSuccess callback function
; to signal a successful installation of the product and launch installation
; of recommended software accepted by the end user.
;
; If you do not use the .onInstSuccess NSIS callback function in your
; script already you must add it.
;
; Usage:
;
;   Function .onInstSuccess
;     # Signal succesfull installation, download and install accepted offers
;     !insertmacro OpenCandyOnInstSuccess
;   FunctionEnd
;

!macro OpenCandyOnInstSuccess
!insertmacro _OpenCandyAPIInserted OpenCandyOnInstSuccess
${If} $OCHasBeenInitialized == ${OC_NSIS_TRUE}
${AndIf} $OCNoCandy == ${OC_NSIS_FALSE}
	Push $0
	!insertmacro _OpenCandyExecOfferInternal ${OC_OFFER_TYPE_NORMAL}
	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403SignalProductInstalled()i.r0? c"
	StrCpy $OCProductInstallSuccess ${OC_NSIS_TRUE}
	Pop $0
${EndIf}
!macroend



;
; OpenCandyInstallEmbedded
; ------------------------
; This macro needs to be inserted in an install section that always runs
; to launch installation of recommended software accepted by the end user.
;
; Usage:
;
;   Section "-OpenCandyEmbedded"
;     # Handle offers the user accepted
;     # This section is hidden. It will always execute during installation
;     # but it won't appear on the component selection screen.
;     !insertmacro OpenCandyInstallEmbedded
;   SectionEnd
;

!macro OpenCandyInstallEmbedded
!insertmacro _OpenCandyAPIInserted OpenCandyInstallDll
${If} $OCHasBeenInitialized == ${OC_NSIS_TRUE}
${AndIf} $OCNoCandy == ${OC_NSIS_FALSE}
	!insertmacro _OpenCandyExecOfferInternal ${OC_OFFER_TYPE_EMBEDDED}
${EndIf}
!macroend



;
; OpenCandyOnGuiEnd
; -----------------
; This macro needs to be inserted in the .onGUIEnd callback function
; to properly unload the OpenCandy DLL. The DLL needs to remain loaded
; until then so that it can start the recommended software setup at the
; very end of the NSIS install process (depending on offer type).
;
; If you do not use the .onGUIEnd NSIS callback function in your
; script already you must add it.
;
; Usage:
;
;   Function .onGUIEnd
;     # Inform the OpenCandy API that the installer is about to exit
;     !insertmacro OpenCandyOnGuiEnd
;   FunctionEnd
;

!macro OpenCandyOnGuiEnd
!insertmacro _OpenCandyAPIInserted OpenCandyOnGuiEnd
${If} $OCHasBeenInitialized == ${OC_NSIS_TRUE}
${AndIf} $OCNoCandy == ${OC_NSIS_FALSE}
	${If} $OCProductInstallSuccess == ${OC_NSIS_FALSE}
	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403SignalProductFailed()i.r0? c"
	${EndIf}
	; Clean up and unload the OpenCandy DLL
	Push $0
	${If} $OCAttached == ${OC_NSIS_TRUE}
		System::Call "OCSetupHlp::OCPID403Detach()i.r0? c"
		StrCpy $OCAttached ${OC_NSIS_FALSE}
	${EndIf}	
   	System::Call /NOUNLOAD "$PLUGINSDIR\OCSetupHlp.dll::OCPID403Shutdown()i.r0? cu"
	Pop $0
${EndIf}
!macroend



!endif # !ifdef _OCSETUPHLP_NSH

; END of OpenCandy Helper Include file