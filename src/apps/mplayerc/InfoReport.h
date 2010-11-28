#pragma once
#include "afxwin.h"


// CInfoReport dialog

class CInfoReport : public CDialog
{
	DECLARE_DYNAMIC(CInfoReport)

public:
	CInfoReport(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInfoReport();

// Dialog Data
	enum { IDD = IDD_INFOREPORT };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_rptText2;
	virtual BOOL OnInitDialog();
	
	CEdit ce_rpt;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButton1();
};
