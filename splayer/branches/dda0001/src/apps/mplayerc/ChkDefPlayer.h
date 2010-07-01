#pragma once

#include "PPageFormats.h"
#include "../../svplib/SVPToolBox.h"
// CChkDefPlayer dialog

class CChkDefPlayer : public CDialog
{
	DECLARE_DYNAMIC(CChkDefPlayer)

public:
	CChkDefPlayer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChkDefPlayer();
	BOOL b_isDefaultPlayer();
	void setDefaultPlayer(int ilimitime = 0);
	CStringArray szaNotExt;
	void setKeyboardNativeMediaPlayers();
	void setKeyboardNativeMediaPlayers2();
// Dialog Data
	enum { IDD = IDD_DEFAULT_PLAYER };
private:
	BOOL m_bNoMoreQuestion;
	CPPageFormats pfpage;
	void OnApplySetting();
	CSVPToolBox svpTool;
	CStringArray szaExt;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton2();
};
