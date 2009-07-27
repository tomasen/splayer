#pragma once

#include "SUIButton.h"

// CSeekBarTip

class CSeekBarTip : public CWnd
{
	DECLARE_DYNAMIC(CSeekBarTip)
	CFont m_statft;
	CRgn m_rgn;
public:
	CSeekBarTip();
	virtual ~CSeekBarTip();
	CString m_text;
	CSize CountSize();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	void OnRealClose();
};


