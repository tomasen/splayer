#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDlgChkUpdater dialog

class CDlgChkUpdater : public CDialog
{
	DECLARE_DYNAMIC(CDlgChkUpdater)

public:
	CDlgChkUpdater(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgChkUpdater();

// Dialog Data
	enum { IDD = IDD_CHECKUPDATE };
	enum {IDT_CHECK_TICK = 74 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CProgressCtrl cprog_chker;
	int moreTick;
	BOOL mChk;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_lostPos;
	afx_msg void OnBnClickedCancel();
	CStatic cs_stat;
	CButton cb_close;
};
