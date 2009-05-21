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
	bool m_hovering;
	HCURSOR cursorHand;

public:
	CSUIBtnList m_btnList;
	int iButtonWidth;
	void UpdateButtonStat();
	CPlayerToolBar();
	virtual ~CPlayerToolBar();

	UINT iBottonClicked;
	UINT iFastFFWCount;
	int GetVolume();
	void SetVolume(int volume);
	__declspec(property(get=GetVolume, put=SetVolume)) int Volume;

	void ArrangeControls();

	CVolumeCtrl m_volctrl;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerToolBar)
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerToolBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnInitialUpdate();
	afx_msg BOOL OnVolumeMute(UINT nID);
	afx_msg void OnUpdateVolumeMute(CCmdUI* pCmdUI);
	afx_msg BOOL OnVolumeUp(UINT nID);
	afx_msg BOOL OnVolumeDown(UINT nID);
	afx_msg void OnNcPaint();
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg CSize CalcFixedLayout(BOOL bStretch,BOOL bHorz );
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
