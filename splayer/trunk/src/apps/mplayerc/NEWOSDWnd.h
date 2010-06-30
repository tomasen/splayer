#pragma once


// CNEWOSDWnd
#include "SUIButton.h"

class CNEWOSDWnd : public CWnd
{
	DECLARE_DYNAMIC(CNEWOSDWnd)
	enum {IDT_HIDE = 1};
	CRgn m_rgn;
	CDC m_dc;
public:
	CNEWOSDWnd();
	virtual ~CNEWOSDWnd();
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnPaint();
	int SendOSDMsg(CString szMsg, int lTime = 3000);
	CSize mSize ;
	CFont m_statft;
	CString m_osdStr;
	CStringArray m_szaOsd;
	CUIntArray m_sziOsd;
	void CountSize();
	CWnd* m_wndView;
	void OnRealClose();
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnClose();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};


