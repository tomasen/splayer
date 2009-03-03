// SVPSubUploadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPSubUploadDlg.h"


// CSVPSubUploadDlg dialog

//IMPLEMENT_DYNAMIC(CSVPSubUploadDlg, CDialog)

CSVPSubUploadDlg::CSVPSubUploadDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSVPSubUploadDlg::IDD, pParent)
{

}

CSVPSubUploadDlg::~CSVPSubUploadDlg()
{
}

void CSVPSubUploadDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, cp_prg);
	DDX_Control( pDX, IDC_EDIT1,ce_vpath );
	DDX_Control( pDX, IDC_EDIT3,ce_subpath );

	DDX_CBString( pDX, IDC_EDIT2,m_szChsName );
	DDX_CBString( pDX, IDC_EDIT4,m_szEngName );
	DDX_CBString( pDX, IDC_EDIT5,m_szVidVersion );
	DDX_CBString( pDX, IDC_EDIT6,m_szProducer );

	DDX_Control(pDX, IDCANCEL, cb_close);
	DDX_Control(pDX, IDC_STATIC_DONE, cs_donemsg);
	DDX_Control(pDX, IDOK, cb_upload);
}


BEGIN_MESSAGE_MAP(CSVPSubUploadDlg, CResizableDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CSVPSubUploadDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CSVPSubUploadDlg::OnBnClickedButton1)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CSVPSubUploadDlg message handlers
BOOL CSVPSubUploadDlg::OnInitDialog()
{
	__super::OnInitDialog();

	cp_prg.SetRange(0, 1000);

	ce_vpath.SetWindowText(m_szVideoPath);
	ce_subpath.SetWindowText(m_szSubPath);

	return TRUE;
}

void CSVPSubUploadDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(IDT_SVPUPTICK == nIDEvent){
		int pos = cp_prg.GetPos();
		if(pos < 500){
			pos += 70;
		}else if(pos < 780){
			pos += 30;
		}else if(pos < 930){
			pos += 10;
		}else{
			pos += 1;
		}
		if(pos >= 1000){
			pos = 931;
		}
		if(!pFrame->m_bSubUploading){
			pos = 1000;
			cb_close.EnableWindow(TRUE);
			KillTimer(IDT_SVPUPTICK);
		}
		
		cp_prg.SetPos(pos);
		if(!pFrame->m_bSubUploading){
			Sleep(800);
			cs_donemsg.ShowWindow(SW_SHOW);
			cp_prg.ShowWindow(SW_HIDE);
		}
	}
	__super::OnTimer(nIDEvent);
}

void CSVPSubUploadDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	cb_close.EnableWindow(FALSE);
	cb_upload.EnableWindow(FALSE);
	pFrame->SVP_UploadSubFileByVideoAndSubFilePath( m_szVideoPath, m_szSubPath, m_iDelayMs, NULL, NULL );
	SetTimer( IDT_SVPUPTICK, 1000, NULL);
	
}

void CSVPSubUploadDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here 
	CString url = _T("http://shooter.cn/sub/upload.html?videohash=");
	ShellExecute(m_hWnd, _T("open"), CString(url), NULL, NULL, SW_SHOWDEFAULT);
}

void CSVPSubUploadDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if(!pFrame->m_bSubUploading){
		__super::OnClose();
	}
	
}
