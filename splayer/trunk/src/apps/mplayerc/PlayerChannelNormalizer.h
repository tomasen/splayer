#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

#include "..\..\filters\switcher\audioswitcher\AudioSwitcher.h"

class CPlayerChannelNormalizer :
	public CSVPDialog
{
	DECLARE_DYNAMIC(CPlayerChannelNormalizer)
	CSVPSliderCtrl csl_trans[MAX_INPUT_CHANNELS];
	CSVPStatic cslLabel[MAX_INPUT_CHANNELS];
	CComQIPtr<IAudioSwitcherFilter> m_pASF;
	int m_nLogDPIY;
	CFont m_font;
	CPoint m_lastMousePoint;
public:
	CPlayerChannelNormalizer(void);
	~CPlayerChannelNormalizer(void);
	enum 
	{
		TIMER_SETCHANNELMAPING = 12176
	};
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Relayout();
	CSize getSizeOfWnd();
	bool InitChannels();
	CString GetChannelDescByID(int iChannelID);
	int m_nChannels;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
