// SVPSubDownUpDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPSubDownUpDialog.h"
#include "OpenFileDlg.h"

// CSVPSubDownUpDialog dialog

//IMPLEMENT_DYNAMIC(CSVPSubDownUpDialog, CResizableDialog)

CSVPSubDownUpDialog::CSVPSubDownUpDialog(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSVPSubDownUpDialog::IDD, pParent)
{

}

CSVPSubDownUpDialog::~CSVPSubDownUpDialog()
{
}

void CSVPSubDownUpDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, cs_vfpath);
	DDX_Control(pDX, IDC_PROGRESS1, cprog);
	DDX_Control(pDX, IDC_EDIT2, ce_log);
	DDX_Control(pDX, IDOK, cb_download);
	DDX_Control(pDX, IDC_STATIC_VFILE, cs_vftitle);
	DDX_Control(pDX, IDC_BUTTON1, cb_ovfile);
	DDX_Control(pDX, IDCANCEL, cb_close);
}


BEGIN_MESSAGE_MAP(CSVPSubDownUpDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CSVPSubDownUpDialog::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CSVPSubDownUpDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDCANCEL, &CSVPSubDownUpDialog::OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


BOOL CSVPSubDownUpDialog::OnInitDialog()
{
	__super::OnInitDialog();

	cprog.SetRange(0, 1000);
	cs_vfpath.SetWindowText(szVidFilePath);

	return TRUE;
}
// CSVPSubDownUpDialog message handlers

void CSVPSubDownUpDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString szFPath;
	cs_vfpath.GetWindowText(szFPath);
	pFrame->SVPSubDownloadByVPath( szFPath , &szaMsgLogs );
	SetTimer( IDT_SVPDOWNTICK, 1000, NULL);
	cb_download.EnableWindow(FALSE);
	cs_vftitle.ShowWindow(SW_HIDE);
	cs_vfpath.ShowWindow(SW_HIDE);
	cb_ovfile.ShowWindow(SW_HIDE); 
	ce_log.ShowWindow(SW_SHOW);
	cb_close.EnableWindow(FALSE);
}

void CSVPSubDownUpDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(IDT_SVPDOWNTICK == nIDEvent){
		int pos = cprog.GetPos();
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
		if(!pFrame->m_bSubDownloading){
			pos = 1000;
			cb_close.EnableWindow(TRUE);
			KillTimer(IDT_SVPDOWNTICK);
		}
		CString szLogs ;
		if( szaMsgLogs.GetCount() > 0 ){
			POSITION pos = szaMsgLogs.GetHeadPosition();
			while(pos){
				szLogs += szaMsgLogs.GetNext(pos) + _T("\r\n");
			}
		}
		ce_log.SetWindowText(szLogs);
		cprog.SetPos(pos);
	}

	__super::OnTimer(nIDEvent);
}

void CSVPSubDownUpDialog::OnBnClickedButton1()
{
	// TODO: 手动打开视频文件
	CString filter;
	CAtlArray<CString> mask;
	AfxGetAppSettings().Formats.GetFilter(filter, mask);
	CString m_path;
	cs_vfpath.GetWindowText(m_path);
	COpenFileDlg fd(mask, true, NULL, m_path, 
		OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_ENABLEINCLUDENOTIFY, 
		filter, this);
	if(fd.DoModal() != IDOK) return;


	POSITION pos = fd.GetStartPosition();
	cs_vfpath.SetWindowText(fd.GetNextPathName(pos));


}

void CSVPSubDownUpDialog::OnBnClickedCancel()
{
	//  跳出
	if(pFrame->m_bSubDownloading && pFrame->m_ThreadSVPSub){
        TerminateThread(pFrame->m_ThreadSVPSub->m_hThread, 0);
    }
		__super::OnCancel();
	//}
}

void CSVPSubDownUpDialog::OnClose()
{
    if(pFrame->m_bSubDownloading && pFrame->m_ThreadSVPSub){
        TerminateThread(pFrame->m_ThreadSVPSub->m_hThread, 0);
    }
		__super::OnClose();
	//}
}
