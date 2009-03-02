#pragma once

#include "afxwin.h"
#include "afxcmn.h"

#include "MainFrm.h"

// CSVPSubDownUpDialog dialog

class CSVPSubDownUpDialog : public CResizableDialog
{
	//DECLARE_DYNAMIC(CSVPSubDownUpDialog)

public:
	CSVPSubDownUpDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSVPSubDownUpDialog();

// Dialog Data
	enum { IDD = IDD_SVPSUBDOWNUPDLG };
	enum { IDT_SVPDOWNTICK };

	CString szVidFilePath;
	CMainFrame* pFrame;
	CAtlList<CString> szaMsgLogs;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit cs_vfpath;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CProgressCtrl cprog;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCancel();
	CEdit ce_log;
	CButton cb_download;
	CStatic cs_vftitle;
	CButton cb_ovfile;
	CButton cb_close;
	afx_msg void OnClose();
};
