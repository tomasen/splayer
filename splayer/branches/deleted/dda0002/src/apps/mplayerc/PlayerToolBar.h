/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "SUIButton.h"

#include "VolumeCtrl.h"

// CPlayerToolBar

class CPlayerToolBar : public CToolBar
{
	DECLARE_DYNAMIC(CPlayerToolBar)

private:
	bool IsMuted();
	void SetMute(bool fMute = true); 
	CImageList m_ToolBarImages;
	CImageList m_ToolBarDisabledImages;
	
	UINT m_nItemToTrack;
	UINT m_nItemToTrackR;
	bool m_hovering;
	HCURSOR cursorHand;
	CSUIButton* m_btnVolTm ;
	CSUIButton* m_btnVolBG;
	CSUIButton* btnLogo;
	CFont m_statft;
	CString m_tooltip ;
	CPoint m_lastMouseMove;
	int m_lastLeftText;
	CSUIButton* btnSubSwitch;
public:
	CString m_timerstr;
	CString m_timerqueryedstr;
	CString m_buffering;
	BOOL holdStatStr;
	int m_nLogDPIY;
	CSUIBtnList m_btnList;
	 CSUIBtnList* const m_pbtnList;
	int iButtonWidth;
	void UpdateButtonStat();
	CPlayerToolBar();
	virtual ~CPlayerToolBar();
	BOOL m_bMouseDown ;
    int m_nHeight;

	UINT iBottonClicked;
	UINT iFastFFWCount;
	int GetVolume();
	void SetVolume(int volume);
	__declspec(property(get=GetVolume, put=SetVolume)) int Volume;

	void SetStatusTimer(CString str, UINT timer = 0);
	void SetStatusTimer(REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, bool fHighPrecision, const GUID* pTimeFormat = &TIME_FORMAT_MEDIA_TIME, double playRate = 1);

	void ArrangeControls();
	void ReCalcBtnPos();
	CVolumeCtrl m_volctrl;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerToolBar)
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	afx_msg CSize CalcFixedLayout(BOOL bStretch,BOOL bHorz );

// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerToolBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	
	afx_msg void OnInitialUpdate();
	afx_msg BOOL OnVolumeMute(UINT nID);
	afx_msg void OnUpdateVolumeMute(CCmdUI* pCmdUI);
	afx_msg BOOL OnVolumeUp(UINT nID);
	afx_msg BOOL OnVolumeDown(UINT nID);
	afx_msg void OnNcPaint();
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	bool OnSetVolByMouse(CPoint point, BOOL byClick = false);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	virtual INT_PTR OnToolHitTest(	CPoint point,TOOLINFO* pTI 	) const;
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};
