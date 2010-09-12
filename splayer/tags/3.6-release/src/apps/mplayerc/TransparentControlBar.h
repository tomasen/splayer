#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

// CTransparentControlBar

class CTransparentControlBar : public CSVPDialog
{
	DECLARE_DYNAMIC(CTransparentControlBar)
	CSVPSliderCtrl csl_trans;
public:
	CTransparentControlBar();
	virtual ~CTransparentControlBar();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Relayout();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};


