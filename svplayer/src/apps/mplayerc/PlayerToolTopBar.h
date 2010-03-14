#pragma once
#include "SUIButton.h"

// CPlayerToolTopBar

class CPlayerToolTopBar : public CWnd
{
	DECLARE_DYNAMIC(CPlayerToolTopBar)
	CRgn m_rgn;
	int iLeftBorderPos;
	CSUIButton* btnClose;
	enum {IDT_CLOSE, IDT_TIPS};
public:
	CPlayerToolTopBar();
	virtual ~CPlayerToolTopBar();
	int m_nLogDPIY;
    int m_nHeight;
	
	CToolTipCtrl m_toolTip;
	TOOLINFO	m_ti;

	UINT m_nItemToTrack;
	UINT m_lastTipItem;
	bool m_hovering;
	HCURSOR cursorHand;
	HCURSOR cursorArrow;

	CFont m_statft;
	CPoint m_lastMouseMove;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);


	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	CSUIBtnList m_btnList;
	CSUIBtnList* const m_pbtnList;
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
public:
	void UpdateButtonStat();
	void ReCalcBtnPos();
	void OnResizeRgn();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnClose();
	void OnRealClose();
	afx_msg void OnDestroy();
	afx_msg void OnMouseLeave();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
};


