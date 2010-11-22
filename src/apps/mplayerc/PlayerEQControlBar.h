#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"



class CPlayerEQControlBar :
	public CSVPDialog
{
	DECLARE_DYNAMIC(CPlayerEQControlBar)
	CSVPSliderCtrl csl_trans[MAX_EQ_BAND];
	CSVPStatic cslLabel[MAX_EQ_BAND];
	CSVPButton cb_reset;
	CSVPButton cb_CurrentPerset;
	CComQIPtr<IAudioSwitcherFilter> m_pASF;
	int m_nLogDPIY;
	CFont m_font;
	CPoint m_lastMousePoint;
	enum 
	{
		TIMER_SETEQCONTROL = 12177
	};
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
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnButtonReset();
	afx_msg void OnPersetMenu();
};
