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

#include "libpng.h"
#include "SUIButton.h"

class CChildView : public CWnd
{
	CRect m_vrect;

	DWORD m_lastlmdowntime;
	CPoint m_lastlmdownpoint;

	CCritSec m_csLogo;
	CPngImage m_logo;
	CBitmap m_logo_bitmap;

	//CPngImage m_watermark;
	CPoint m_lastMouseMove;
	UINT m_nItemToTrack;
	int iBottonClicked;
	CRect m_logo_r;
	CFont m_font;
    CFont m_font_lyric;
    DWORD m_lastLyricColor;
public:
	CChildView();
	virtual ~CChildView();

	BOOL m_bMouseDown ;
	CString m_strAudioInfo;
	int m_AudioInfoCounter;
	CSUIButton* m_cover;
	CSUIBtnList m_btnList;
	CSUIBtnList m_btnBBList;
	void ReCalcBtn();
	DECLARE_DYNAMIC(CChildView)

    void SetLyricLasting(int time_sec);
public:
	void SetVideoRect(CRect r = CRect(0,0,0,0));
	CRect GetVideoRect() {return(m_vrect);}

	void LoadLogo();
	void LoadLogoFromFile(CString fnLogo);
	BOOL isUsingSkinBG;
	CSize GetLogoSize();
	
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg BOOL OnPlayPlayPauseStop(UINT nID);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
