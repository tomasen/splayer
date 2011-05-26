#pragma once
#include "stdafx.h"
#include "mplayerc.h"
#include "SUIButton.h"


//歌词显示面板
//mouseover时显示半透明玻璃背景、关闭按钮和变色按钮，out后仅显示文字
//默认在屏幕底部区域显示，可以通过拖拽改变位置
//关闭后改为在主窗口界面内显示

class SVPLycShowBox :
	public CFrameWnd
{
private:

	//背景应该为透明
	//ChangeBackgroundColor( COLORREF rgb_color );
	int ChangeFontColor( COLORREF rgb_color );


	/*
	拖拽来改变背景颜色 , 方向决定色彩变化
	每60度一个方向
	左上+R 上+G 右上+B
	左下+R 下-G 右下-B
	like a invisible color picker
	*/
	int ChangeFontColorByMouse(CPoint movement);

public:
	SVPLycShowBox(void);
	~SVPLycShowBox(void);

	/* 
	显示歌词文字和最长保持时间
	*/

	int ShowLycLine(CString szLycLine, int iLastingTime);

//Standard Message Loop Shits
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);

	void DoUpdateWindow();

	CBitmap m_bmpWnd;
	SIZE m_sizeBmpWnd;
	int m_nBmpWndPadding;
	CFont m_f;

	long m_nCurrentMouseAction;
	BOOL m_bMouseInAction;
	POINT m_ptMouse;

	//测试 POPUP 的 Layered Window
	HWND m_wndNewOSD;
};
