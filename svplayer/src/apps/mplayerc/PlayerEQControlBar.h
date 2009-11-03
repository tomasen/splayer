#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

#define MAX_EQ_BRAND 10

class CPlayerEQControlBar :
	public CSVPDialog
{
	DECLARE_DYNAMIC(CPlayerEQControlBar)
	CSVPSliderCtrl csl_trans[MAX_EQ_BRAND];
	CComQIPtr<IAudioSwitcherFilter> m_pASF;
	int m_nLogDPIY;
	CFont m_font;
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
	CSize getSizeOfWnd();
	bool InitSettings();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
