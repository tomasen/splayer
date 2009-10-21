#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

class CPlayerChannelNormalizer :
	public CSVPDialog
{
	DECLARE_DYNAMIC(CPlayerChannelNormalizer)
	CSVPSliderCtrl csl_trans[18];
public:
	CPlayerChannelNormalizer(void);
	~CPlayerChannelNormalizer(void);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Relayout();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
