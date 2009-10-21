#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

class CPlayerEQControlBar :
	public CSVPDialog
{
	DECLARE_DYNAMIC(CPlayerEQControlBar)
	CSVPSliderCtrl csl_trans[10];
public:
	CPlayerEQControlBar(void);
	~CPlayerEQControlBar(void);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Relayout();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
