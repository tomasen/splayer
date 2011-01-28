#pragma once


// CSearchSubDlg dialog

class CSearchSubDlg : public CDialog
{
	DECLARE_DYNAMIC(CSearchSubDlg)

public:
	CSearchSubDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchSubDlg();

// Dialog Data
	enum { IDD = IDD_SEARCHSUBDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_skeywords;
};
