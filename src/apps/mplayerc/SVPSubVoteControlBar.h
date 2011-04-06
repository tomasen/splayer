#pragma once
#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"


class CSVPSubVoteControlBar :
	public CSVPDialog
{
	DECLARE_DYNAMIC(CSVPSubVoteControlBar)
	int m_nLogDPIY;
	CFont m_font;
public:
	CSVPSubVoteControlBar(void);
	~CSVPSubVoteControlBar(void);
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void Relayout();
	void InitSettings();
	CSize getSizeOfWnd();
};
