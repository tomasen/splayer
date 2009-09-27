#pragma once


// CPlayerFloatToolBar

class CPlayerFloatToolBar : public CWnd
{
	DECLARE_DYNAMIC(CPlayerFloatToolBar)

	CRgn m_rgn;
public:
	CPlayerFloatToolBar();
	virtual ~CPlayerFloatToolBar();


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	void OnRealClose();
	BOOL m_bFocused;
	int GetUIHeight();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


