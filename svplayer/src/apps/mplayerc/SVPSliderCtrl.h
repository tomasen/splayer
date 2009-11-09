#pragma once

#include "SUIButton.h"

// CSVPSliderCtrl

class CSVPSliderCtrl : public CSliderCtrl
{
	DECLARE_DYNAMIC(CSVPSliderCtrl)

public:
	CSVPSliderCtrl();
	virtual ~CSVPSliderCtrl();
	CString imgTM;
	CString imgTBG;
	DWORD colorBackGround;
	DWORD m_style;

	BOOL m_bVertical ;
	CSUIButton* m_btnVolTm;
	CSUIButton* m_btnVolBG;
	CSUIBtnList m_btnList;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	bool SetThumb(const CPoint& pt);
	void PostMessageToParent(const int nTBCode) const;
	bool m_bDragging;
	bool m_bDragChanged;
};


