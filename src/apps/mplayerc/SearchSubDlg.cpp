// SearchSubDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SearchSubDlg.h"


// CSearchSubDlg dialog

IMPLEMENT_DYNAMIC(CSearchSubDlg, CDialog)

CSearchSubDlg::CSearchSubDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSearchSubDlg::IDD, pParent)
	, m_skeywords(_T(""))
{

}

CSearchSubDlg::~CSearchSubDlg()
{
}

void CSearchSubDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_skeywords);
}


BEGIN_MESSAGE_MAP(CSearchSubDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSearchSubDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSearchSubDlg::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// CSearchSubDlg message handlers

void CSearchSubDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	CString url = _T("http://shooter.cn/sub/?searchword=");
	url += m_skeywords;

	ShellExecute(m_hWnd, _T("open"), url, NULL, NULL, SW_SHOWDEFAULT);

	//OnOK();
}
