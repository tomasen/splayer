// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXDLGS_H__
#define __AFXDLGS_H__

#pragma once

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#include <objbase.h>

#ifndef _INC_COMMDLG
	#include <commdlg.h>    // common dialog APIs
#endif

#if WINVER >= 0x0600
#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#else
#undef _WIN32_IE
#define _WIN32_IE 0x0700
#endif
#ifndef __shobjidl_h__
	#include <shobjidl.h>    // for IFileDialog/IFileOpenDialog/IFileSaveDialog
#endif
#endif

// Avoid mapping GetFileTitle to GetFileTitle[A/W]
#ifdef GetFileTitle
#undef GetFileTitle
AFX_INLINE short APIENTRY GetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf)
#ifdef UNICODE
	{ return ::GetFileTitleW(lpszFile, lpszTitle, cbBuf); }
#else
	{ return ::GetFileTitleA(lpszFile, lpszTitle, cbBuf); }
#endif
#endif

#ifndef _AFX_NO_RICHEDIT_SUPPORT
	#ifndef _RICHEDIT_
		#include <richedit.h>
	#endif
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif 

#ifndef _AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

#endif //!_AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXDLGS - MFC Standard dialogs

// Classes declared in this file

	// CDialog
		class CCommonDialog;  // implementation base class

			// modeless dialogs
			class CFindReplaceDialog; // Find/FindReplace dialog

			// modal dialogs
			class CFileDialog;    // FileOpen/FileSaveAs dialogs
			class CColorDialog;   // Color picker dialog
			class CFontDialog;    // Font chooser dialog
			class CPrintDialog;   // Print/PrintSetup dialogs
#if WINVER >= 0x0500
			class CPrintDialogEx;   // Windows 2000 Print dialog
#endif //(WINVER >= 0x0500)
			class CPageSetupDialog; // Page Setup dialog

	// CWnd
	class CPropertySheet;     // implements tabbed dialogs

	// CDialog
		class CPropertyPage;  // Used with CPropertySheet for tabbed dialogs

/////////////////////////////////////////////////////////////////////////////

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// CCommonDialog - base class for all common dialogs

class CCommonDialog : public CDialog
{
	DECLARE_DYNAMIC(CCommonDialog)
	
public:
	explicit CCommonDialog(CWnd* pParentWnd);

// Implementation
public:
    virtual ~CCommonDialog() {};

protected:
	virtual void OnOK();
	virtual void OnCancel();

	//{{AFX_MSG(CCommonDialog)
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CFileDialog - used for FileOpen... or FileSaveAs...

class CFileDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFileDialog)

public:
// Attributes
	__declspec(property(get=GetOFN)) OPENFILENAME m_ofn;
	const OPENFILENAME& GetOFN() const;
	OPENFILENAME& GetOFN();
	LPOPENFILENAME m_pOFN;

// Constructors
	explicit CFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL,
		DWORD dwSize = 0,
		BOOL bVistaStyle = TRUE);
	virtual ~CFileDialog();	

// Operations
	virtual INT_PTR DoModal();

	// Helpers for parsing file name after successful return
	// or during Overridable callbacks if OFN_EXPLORER is set
	CString GetPathName() const;  // return full path and filename
	CString GetFileName() const;  // return only filename
	CString GetFileExt() const;   // return only ext
	CString GetFileTitle() const; // return file title
	BOOL GetReadOnlyPref() const; // return TRUE if readonly checked

	// Enumerating multiple file selections
	POSITION GetStartPosition() const;
	CString GetNextPathName(POSITION& pos) const;

	// Helpers for custom templates
	void SetTemplate(UINT nWin3ID, UINT nWin4ID);
	void SetTemplate(LPCTSTR lpWin3ID, LPCTSTR lpWin4ID);

	// Other operations available while the dialog is visible
	CString GetFolderPath() const; // return full path
	void SetControlText(int nID, LPCSTR lpsz);
	#ifdef UNICODE
	void SetControlText(int nID, const wchar_t  *lpsz);
	#endif 
	void HideControl(int nID);
	void SetDefExt(LPCSTR lpsz);
#if WINVER >= 0x0600
	void UpdateOFNFromShellDialog();
	void ApplyOFNToShellDialog();
	IFileOpenDialog* GetIFileOpenDialog() throw();
	IFileSaveDialog* GetIFileSaveDialog() throw();
	IFileDialogCustomize* GetIFileDialogCustomize() throw();
#endif

// Overridable callbacks
protected:

#if WINVER >= 0x0600
	HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void **ppv);
#endif

	friend UINT_PTR CALLBACK _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual UINT OnShareViolation(LPCTSTR lpszPathName);
	virtual BOOL OnFileNameOK();
	virtual void OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode);

	// only called back if OFN_EXPLORER is set
	virtual void OnInitDone();
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	virtual void OnTypeChange();

// Implementation
#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	BOOL m_bVistaStyle;
	DWORD m_dwCookie;
	void* m_pIFileDialog;
	void* m_pIFileDialogCustomize;

	BOOL m_bOpenFileDialog;       // TRUE for file open, FALSE for file save
	CString m_strFilter;          // filter string
						// separate fields with '|', terminate with '||\0'
	TCHAR m_szFileTitle[64];       // contains file title after return
	TCHAR m_szFileName[_MAX_PATH]; // contains full path name after return

	OPENFILENAME*  m_pofnTemp;

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

protected:
	DECLARE_INTERFACE_MAP()

#if WINVER >= 0x0600

	BEGIN_INTERFACE_PART(FileDialogEvents, IFileDialogEvents)
		STDMETHOD(OnFileOk)(IFileDialog *);
		STDMETHOD(OnFolderChange)(IFileDialog *);
		STDMETHOD(OnFolderChanging)(IFileDialog *, IShellItem *);
		STDMETHOD(OnHelp)(IFileDialog *);
		STDMETHOD(OnSelectionChange)(IFileDialog *);
		STDMETHOD(OnShareViolation)(
			IFileDialog *pfd,
			IShellItem *psi,
			FDE_SHAREVIOLATION_RESPONSE *pResponse);
		STDMETHOD(OnTypeChange)(IFileDialog *);
		STDMETHOD(OnOverwrite)(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *);
	END_INTERFACE_PART_OPTIONAL(FileDialogEvents)

	BEGIN_INTERFACE_PART(FileDialogControlEvents, IFileDialogControlEvents)
		STDMETHOD(OnItemSelected)(IFileDialogCustomize *, DWORD, DWORD);
		STDMETHOD(OnButtonClicked)(IFileDialogCustomize *, DWORD);
		STDMETHOD(OnCheckButtonToggled)(IFileDialogCustomize *, DWORD, BOOL);
		STDMETHOD(OnControlActivating)(IFileDialogCustomize *, DWORD);
	END_INTERFACE_PART_OPTIONAL(FileDialogControlEvents)

#else
	
	BEGIN_INTERFACE_PART(FileDialogEvents, IUnknown)
	END_INTERFACE_PART_OPTIONAL(FileDialogEvents)
	
	BEGIN_INTERFACE_PART(FileDialogControlEvents, IUnknown)
	END_INTERFACE_PART_OPTIONAL(FileDialogControlEvents)

#endif
};

/////////////////////////////////////////////////////////////////////////////
// CFontDialog - used to select a font

class CFontDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFontDialog)

public:
// Attributes
	// font choosing parameter block
	CHOOSEFONT m_cf;

// Constructors
	CFontDialog(LPLOGFONT lplfInitial = NULL,
		DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	CFontDialog(const CHARFORMAT& charformat,
		DWORD dwFlags = CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
#endif
// Operations
	virtual INT_PTR DoModal();

	// Get the selected font (works during DoModal displayed or after)
	void GetCurrentFont(LPLOGFONT lplf);

	// Helpers for parsing information after successful return
	CString GetFaceName() const;  // return the face name of the font
	CString GetStyleName() const; // return the style name of the font
	int GetSize() const;          // return the pt size of the font
	COLORREF GetColor() const;    // return the color of the font
	int GetWeight() const;        // return the chosen font weight
	BOOL IsStrikeOut() const;     // return TRUE if strikeout
	BOOL IsUnderline() const;     // return TRUE if underline
	BOOL IsBold() const;          // return TRUE if bold font
	BOOL IsItalic() const;        // return TRUE if italic font
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	void GetCharFormat(CHARFORMAT& cf) const;
#endif

// Implementation
	LOGFONT m_lf; // default LOGFONT to store the info
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	DWORD FillInLogFont(const CHARFORMAT& cf);
#endif

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	TCHAR m_szStyleName[64]; // contains style name after return
};

/////////////////////////////////////////////////////////////////////////////
// CColorDialog - used to select a color

class CColorDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CColorDialog)

public:
// Attributes
	// color chooser parameter block
	CHOOSECOLOR m_cc;

// Constructors
	CColorDialog(COLORREF clrInit = 0, DWORD dwFlags = 0,
			CWnd* pParentWnd = NULL);

// Operations
	virtual INT_PTR DoModal();

	// Set the current color while dialog is displayed
	void SetCurrentColor(COLORREF clr);

	// Helpers for parsing information after successful return
	COLORREF GetColor() const;
	static COLORREF* PASCAL GetSavedCustomColors();

// Overridable callbacks
protected:
	friend UINT_PTR CALLBACK _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual BOOL OnColorOK();       // validate color

// Implementation

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif
};

// for backward compatibility clrSavedCustom is defined as GetSavedCustomColors
#define clrSavedCustom GetSavedCustomColors()

/////////////////////////////////////////////////////////////////////////////
// Page Setup dialog

class CPageSetupDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CPageSetupDialog)

public:
// Attributes
	PAGESETUPDLG m_psd;

// Constructors
	CPageSetupDialog(DWORD dwFlags = PSD_MARGINS | PSD_INWININIINTLMEASURE,
		CWnd* pParentWnd = NULL);

// Attributes
	LPDEVMODE GetDevMode() const;   // return DEVMODE
	CString GetDriverName() const;  // return driver name
	CString GetDeviceName() const;  // return device name
	CString GetPortName() const;    // return output port name
	HDC CreatePrinterDC();
	CSize GetPaperSize() const;
	void GetMargins(LPRECT lpRectMargins, LPRECT lpRectMinMargins) const;

// Operations
	virtual INT_PTR DoModal();

// Overridables
	virtual UINT PreDrawPage(WORD wPaper, WORD wFlags, LPPAGESETUPDLG pPSD);
	virtual UINT OnDrawPage(CDC* pDC, UINT nMessage, LPRECT lpRect);

// Implementation
protected:
	static UINT CALLBACK PaintHookProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam);

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CPrintDialog - used for Print... and PrintSetup...

class CPrintDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CPrintDialog)

public:
// Attributes
	// print dialog parameter block (note this is a reference)
	PRINTDLG& m_pd;

// Constructors
	CPrintDialog(BOOL bPrintSetupOnly,
		// TRUE for Print Setup, FALSE for Print Dialog
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
			| PD_HIDEPRINTTOFILE | PD_NOSELECTION,
		CWnd* pParentWnd = NULL);

// Operations
	virtual INT_PTR DoModal();

	// GetDefaults will not display a dialog but will get
	// device defaults
	BOOL GetDefaults();

	// Helpers for parsing information after successful return
	int GetCopies() const;          // num. copies requested
	BOOL PrintCollate() const;      // TRUE if collate checked
	BOOL PrintSelection() const;    // TRUE if printing selection
	BOOL PrintAll() const;          // TRUE if printing all pages
	BOOL PrintRange() const;        // TRUE if printing page range
	int GetFromPage() const;        // starting page if valid
	int GetToPage() const;          // starting page if valid
	LPDEVMODE GetDevMode() const;   // return DEVMODE
	CString GetDriverName() const;  // return driver name
	CString GetDeviceName() const;  // return device name
	CString GetPortName() const;    // return output port name
	HDC GetPrinterDC() const;       // return HDC (caller must delete)

	// This helper creates a DC based on the DEVNAMES and DEVMODE structures.
	// This DC is returned, but also stored in m_pd.hDC as though it had been
	// returned by CommDlg.  It is assumed that any previously obtained DC
	// has been/will be deleted by the user.  This may be
	// used without ever invoking the print/print setup dialogs.

	HDC CreatePrinterDC();

// Implementation

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	PRINTDLG m_pdActual; // the Print/Print Setup need to share this
protected:
	// The following handle the case of print setup... from the print dialog
	CPrintDialog(PRINTDLG& pdInit);
	virtual CPrintDialog* AttachOnSetup();

	//{{AFX_MSG(CPrintDialog)
	afx_msg void OnPrintSetup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CPrintDialogEx - Windows 2000 Print Dialog

#if WINVER >= 0x0500

#ifndef __ocidl_h__
	#include <ocidl.h>
#endif

class CPrintDialogEx :
			public CCommonDialog,
			public IPrintDialogCallback,
			public IObjectWithSite
{

	DECLARE_DYNAMIC(CPrintDialogEx)

public:
// Attributes
	// print dialog parameter block
	PRINTDLGEX m_pdex;

// Constructors
	CPrintDialogEx(DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
			| PD_HIDEPRINTTOFILE | PD_NOSELECTION | PD_NOCURRENTPAGE,
		CWnd* pParentWnd = NULL);

// Operations
	virtual INT_PTR DoModal();

	// GetDefaults will not display a dialog but will get
	// device defaults
	BOOL GetDefaults();

	// Helpers for parsing information after successful return
	int GetCopies() const;          // num. copies requested
	BOOL PrintCollate() const;      // TRUE if collate checked
	BOOL PrintSelection() const;    // TRUE if printing selection
	BOOL PrintCurrentPage() const;	// TRUE if printing current page
	BOOL PrintAll() const;          // TRUE if printing all pages
	BOOL PrintRange() const;        // TRUE if printing page range
	LPDEVMODE GetDevMode() const;   // return DEVMODE
	CString GetDriverName() const;  // return driver name
	CString GetDeviceName() const;  // return device name
	CString GetPortName() const;    // return output port name
	HDC GetPrinterDC() const;       // return HDC (caller must delete)

	// This helper creates a DC based on the DEVNAMES and DEVMODE structures.
	// This DC is returned, but also stored in m_pdex.hDC as though it had been
	// returned by CommDlg.  It is assumed that any previously obtained DC
	// has been/will be deleted by the user.  This may be
	// used without ever invoking the print dialog.

	HDC CreatePrinterDC();

// Implementation

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

// IPrintDialogCallback
	STDMETHOD(InitDone)();
	STDMETHOD(SelectionChange)();
	STDMETHOD(HandleMessage)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

// IObjectWithSite
	STDMETHOD(SetSite)(IUnknown *pUnkSite);
	STDMETHOD(GetSite)(REFIID riid, void **ppvSite);

	IUnknown* m_pUnkSite;

	// implementation helpers
	HWND PreModal();
	void PostModal();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	//{{AFX_MSG(CPrintDialogEx)
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif //(WINVER >= 0x0500)

/////////////////////////////////////////////////////////////////////////////
// Find/FindReplace modeless dialogs

class CFindReplaceDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFindReplaceDialog)

public:
// Attributes
	FINDREPLACE m_fr;

// Constructors
	CFindReplaceDialog();
	// Note: you must allocate these on the heap.
	//  If you do not, you must derive and override PostNcDestroy()

	virtual BOOL Create(BOOL bFindDialogOnly, // TRUE for Find, FALSE for FindReplace
			LPCTSTR lpszFindWhat,
			LPCTSTR lpszReplaceWith = NULL,
			DWORD dwFlags = FR_DOWN,
			CWnd* pParentWnd = NULL);

	// find/replace parameter block
	static CFindReplaceDialog* PASCAL GetNotifier(LPARAM lParam);

// Operations
	// Helpers for parsing information after successful return
	CString GetReplaceString() const;// get replacement string
	CString GetFindString() const;   // get find string
	BOOL SearchDown() const;         // TRUE if search down, FALSE is up
	BOOL FindNext() const;           // TRUE if command is find next
	BOOL MatchCase() const;          // TRUE if matching case
	BOOL MatchWholeWord() const;     // TRUE if matching whole words only
	BOOL ReplaceCurrent() const;     // TRUE if replacing current string
	BOOL ReplaceAll() const;         // TRUE if replacing all occurrences
	BOOL IsTerminating() const;      // TRUE if terminating dialog

// Implementation
protected:
	virtual void PostNcDestroy();

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	TCHAR m_szFindWhat[128];
	TCHAR m_szReplaceWith[128];
};

////////////////////////////////////////////////////////////////////////////
// CPropertyPage -- one page of a tabbed dialog

class CPropertyPage : public CDialog
{
	DECLARE_DYNAMIC(CPropertyPage)

// Construction
public:
	// simple construction
	CPropertyPage();
	explicit CPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0, DWORD dwSize = sizeof(PROPSHEETPAGE));
	explicit CPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0, DWORD dwSize = sizeof(PROPSHEETPAGE));
	void Construct(UINT nIDTemplate, UINT nIDCaption = 0);
	void Construct(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);

	// extended construction
	CPropertyPage(UINT nIDTemplate, UINT nIDCaption, 
		UINT nIDHeaderTitle, UINT nIDHeaderSubTitle = 0, DWORD dwSize = sizeof(PROPSHEETPAGE));
	CPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption, 
		UINT nIDHeaderTitle, UINT nIDHeaderSubTitle = 0, DWORD dwSize = sizeof(PROPSHEETPAGE));
	void Construct(UINT nIDTemplate, UINT nIDCaption, 
		UINT nIDHeaderTitle, UINT nIDHeaderSubTitle = 0);
	void Construct(LPCTSTR lpszTemplateName, UINT nIDCaption, 
		UINT nIDHeaderTitle, UINT nIDHeaderSubTitle = 0);

// Attributes
	__declspec(property(get=GetPSP)) PROPSHEETPAGE m_psp;
	const PROPSHEETPAGE & GetPSP() const;
	PROPSHEETPAGE & GetPSP();
	LPPROPSHEETPAGE m_pPSP;

// Operations
	void CancelToClose();
	void SetModified(BOOL bChanged = TRUE);
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam);

// Overridables
public:
	virtual BOOL OnApply();
	virtual void OnReset();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnQueryCancel();

	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnWizardFinish();	
	virtual HWND OnWizardFinishEx();

// Implementation
public:
	virtual ~CPropertyPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void EndDialog(int nEndID); // called for error scenarios

protected:
	// private implementation data
	CString m_strCaption;
	BOOL m_bFirstSetActive;
	CString m_strHeaderTitle;    // this is displayed in the header
	CString m_strHeaderSubTitle; //

	// implementation helpers
	void AllocPSP(DWORD dwSize);
	void CommonConstruct(LPCTSTR lpszTemplateName, UINT nIDCaption);
	void CommonConstruct(LPCTSTR lpszTemplateName, UINT nIDCaption, 
		UINT nIDHeaderTitle, UINT nIDHeaderSubTitle);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG*);
	LRESULT MapWizardResult(LRESULT lToMap);
	BOOL IsButtonEnabled(int iButton);

	void PreProcessPageTemplate(PROPSHEETPAGE& psp, BOOL bWizard);
#ifndef _AFX_NO_OCC_SUPPORT
	void Cleanup();
	const DLGTEMPLATE* InitDialogInfo(const DLGTEMPLATE* pTemplate);
#endif

	// Generated message map functions
	//{{AFX_MSG(CPropertyPage)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CPropertySheet;
};

#define CPropertyPageEx CPropertyPage

////////////////////////////////////////////////////////////////////////////
// CPropertySheet -- a tabbed "dialog" (really a popup-window)

class CTabCtrl; // forward reference (see afxcmn.h)

class CPropertySheet : public CWnd
{
	DECLARE_DYNAMIC(CPropertySheet)

// Construction
public:
	// simple construction
	CPropertySheet();
	explicit CPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	explicit CPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	void Construct(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	void Construct(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);

	// extended construction
	CPropertySheet(UINT nIDCaption, CWnd* pParentWnd,
		UINT iSelectPage, HBITMAP hbmWatermark,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);
	CPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd,
		UINT iSelectPage, HBITMAP hbmWatermark,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);
	void Construct(UINT nIDCaption, CWnd* pParentWnd,
		UINT iSelectPage, HBITMAP hbmWatermark,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);
	void Construct(LPCTSTR pszCaption, CWnd* pParentWnd,
		UINT iSelectPage, HBITMAP hbmWatermark,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);

	// for modeless creation
	virtual BOOL Create(CWnd* pParentWnd = NULL, DWORD dwStyle = (DWORD)-1,
		DWORD dwExStyle = 0);
	// the default style, expressed by passing -1 as dwStyle, is actually:
	// WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | DS_CONTEXT_HELP | WS_VISIBLE

// Attributes
public:
	PROPSHEETHEADER m_psh;

	int GetPageCount() const;
	CPropertyPage* GetActivePage() const;
	int GetActiveIndex() const;
	CPropertyPage* GetPage(int nPage) const;
	int GetPageIndex(CPropertyPage* pPage);
	BOOL SetActivePage(int nPage);
	BOOL SetActivePage(CPropertyPage* pPage);
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0);
	CTabCtrl* GetTabControl() const;

	void SetWizardMode();
	void SetFinishText(LPCTSTR lpszText);
	void SetWizardButtons(DWORD dwFlags);

	void EnableStackedTabs(BOOL bStacked);

// Operations
public:
	virtual INT_PTR DoModal();
	void AddPage(CPropertyPage* pPage);
	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);
	void EndDialog(int nEndID); // used to terminate a modal dialog
	void PressButton(int nButton);
	void MapDialogRect(LPRECT lpRect) const;

// Overridables (special message map entries)
	virtual BOOL OnInitDialog();

// Implementation
public:
	virtual ~CPropertySheet();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void CommonConstruct(CWnd* pParentWnd, UINT iSelectPage);
	void CommonConstruct(CWnd* pParentWnd, UINT iSelectPage,
		HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void BuildPropPageArray();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL ContinueModal();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	BOOL IsWizard() const;
	BOOL IsModeless() const;

protected:
	CPtrArray m_pages;      // array of CPropertyPage pointers
	CString m_strCaption;   // caption of the pseudo-dialog
	CWnd* m_pParentWnd;     // parent window of property sheet
	BOOL m_bStacked;        // EnableStackedTabs sets this
	BOOL m_bModeless;       // TRUE when Create called instead of DoModal

	// Generated message map functions
	//{{AFX_MSG(CPropertySheet)
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM);
	afx_msg LRESULT OnSetDefID(WPARAM, LPARAM);
	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CPropertyPage;
};

#define CPropertySheetEx CPropertySheet

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDLGS_INLINE AFX_INLINE
#include <afxdlgs.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#endif //__AFXDLGS_H__

/////////////////////////////////////////////////////////////////////////////
