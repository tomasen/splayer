#pragma once


// CPlayerFloatToolBar

class CPlayerFloatToolBar : public CFrameWnd
{
	DECLARE_DYNAMIC(CPlayerFloatToolBar)

	CRgn m_rgn;
	CPoint m_lastMouseMove;
public:
	CPlayerFloatToolBar();
	virtual ~CPlayerFloatToolBar();
	
	int m_lSeekBarHeight;
	int m_lToolBarHeight;

protected:
	DECLARE_MESSAGE_MAP()
public:
	int m_nLogDPIY;
	bool m_bDraging;
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
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	CPoint m_lastpoint;
	BOOL m_mousedown;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


