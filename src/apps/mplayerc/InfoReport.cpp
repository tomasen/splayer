// InfoReport.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "InfoReport.h"
#include "revision.h"

// CInfoReport dialog

IMPLEMENT_DYNAMIC(CInfoReport, CDialog)

CInfoReport::CInfoReport(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoReport::IDD, pParent)
	, m_rptText2(_T(""))

{

}

CInfoReport::~CInfoReport()
{
}

void CInfoReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, ce_rpt);
}


BEGIN_MESSAGE_MAP(CInfoReport, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CInfoReport::OnBnClickedButton1)
END_MESSAGE_MAP()


// CInfoReport message handlers

BOOL CInfoReport::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	CString szVersion;
	szVersion.Format( _T("Version %d \n") , SVP_REV_NUMBER);
	ce_rpt.SetWindowText(szVersion + m_rptText2 +  AfxGetAppSettings().szFGMLog);

	// Set WS_EX_LAYERED on this window 
	::SetWindowLong(this->m_hWnd , GWL_EXSTYLE, ::GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	// Make this window 70% alpha
	SetLayeredWindowAttributes( 0, (255 * 70) / 100, LWA_ALPHA);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CInfoReport::PreTranslateMessage(MSG* pMsg)
{
	

	return CDialog::PreTranslateMessage(pMsg);
}

void CInfoReport::OnBnClickedButton1()
{
	if (!OpenClipboard())
	{
		AfxMessageBox(_T("Cannot open the Clipboard"));
		return;
	}
	// Remove the current Clipboard contents  
	if(!EmptyClipboard())
	{
		AfxMessageBox(_T("Cannot empty the Clipboard"));
		return;  
	}

	// Get the currently selected data, hData handle to 
	// global memory of data
	CString str;
	ce_rpt.GetWindowText(str);
	
	size_t cbStr = (str.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, cbStr);
	memcpy_s(GlobalLock(hData), cbStr, str.LockBuffer(), cbStr);
	GlobalUnlock(hData);
	str.UnlockBuffer();

	// For the appropriate data formats...
	UINT uiFormat = (sizeof(TCHAR) == sizeof(WCHAR)) ? CF_UNICODETEXT : CF_TEXT;
	if (::SetClipboardData(uiFormat, hData) == NULL)  
	{
		AfxMessageBox(_T("Unable to set Clipboard data"));    
		CloseClipboard();
		return;  
	}  

	CloseClipboard();

}
