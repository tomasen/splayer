#pragma once

#include "afxwin.h"
#include "afxcmn.h"

#include "MainFrm.h"
// CSVPSubUploadDlg dialog

class CSVPSubUploadDlg : public CResizableDialog
{
	//DECLARE_DYNAMIC(CSVPSubUploadDlg)

public:
	CSVPSubUploadDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSVPSubUploadDlg();

// Dialog Data
	enum { IDD = IDD_SVPSUBUPLOAD };
	enum { IDT_SVPUPTICK };
	CMainFrame* pFrame;
	CString m_szVideoPath;
	CString m_szSubPath;
	int m_iDelayMs;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CProgressCtrl cp_prg;
	afx_msg void OnBnClickedOk();
	CEdit ce_vpath;
	CEdit ce_subpath;

	CString m_szChsName;
	CString m_szEngName;
	CString m_szVidVersion;
	CString m_szProducer;
	CButton cb_close;
	CStatic cs_donemsg;
	afx_msg void OnBnClickedButton1();
	CButton cb_upload;
	afx_msg void OnClose();
};
