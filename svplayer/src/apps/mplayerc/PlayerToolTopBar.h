#pragma once
#include "SUIButton.h"

// CPlayerToolTopBar

class CPlayerToolTopBar : public CToolBar
{
	DECLARE_DYNAMIC(CPlayerToolTopBar)

public:
	CPlayerToolTopBar();
	virtual ~CPlayerToolTopBar();
	int m_nLogDPIY;


	UINT m_nItemToTrack;
	bool m_hovering;
	HCURSOR cursorHand;

	CFont m_statft;
	CString m_tooltip ;
	CPoint m_lastMouseMove;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void ReCalcBtnPos();
	CSUIBtnList m_btnList;
	CSUIBtnList* const m_pbtnList;
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP, UINT nID = AFX_IDW_TOOLBAR);
	afx_msg CSize CalcFixedLayout(BOOL bStretch,BOOL bHorz );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
};


