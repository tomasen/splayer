#pragma once

#include "SUIButton.h"

// CSeekBarTip

class CSeekBarTip : public CWnd
{
	DECLARE_DYNAMIC(CSeekBarTip)
	CFont m_statft;
	CRgn m_rgn;
	enum{IDT_CLOSTTIPS, IDT_DELAYOPEN};
	CString m_delayText;
	BOOL m_delayMove  ;
	CPoint m_delayPoint ;
public:
	CSeekBarTip();
	virtual ~CSeekBarTip();
	CString m_text;
	CSize CountSize();
	void SetTips(CString szText, BOOL bMove = TRUE , CPoint* mPoint = NULL, UINT delayOpen = 0);
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	void ClearStat();
	void OnRealClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};


