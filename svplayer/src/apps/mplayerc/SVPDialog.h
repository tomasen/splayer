#pragma once

#include "SUIButton.h"
#include "SVPButton.h"
// CSVPDialog

class CSVPDialog : public CWnd
{
	DECLARE_DYNAMIC(CSVPDialog)

	CFont m_statft;
	CRgn m_rgn;
	CBrush m_brushBorder;
	CSVPButton m_btnClose;
	CRgn m_rgnBorder;
	DWORD m_bgColor , m_borderColor;
public:
	CSVPDialog();
	virtual ~CSVPDialog();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnButtonClose();
	void OnRealClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMove(int x, int y);
};


