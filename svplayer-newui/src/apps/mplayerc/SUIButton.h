
#pragma once

#include "stdafx.h"

#include <atlbase.h>

class CSUIButton {
	CSize m_btnSize; //按钮的大小尺寸
	int m_iAlign; //与窗口对齐方向
	CRect m_marginTownd; //按钮相对于窗口的位置， 0为不强制 负数为百分比
	CRect m_rcHitest; //按钮的有效范围，用于hittest
	CBitmap m_bitmap; //图片存储位置
	int m_stat; //0 normal ; 1 hove ; 2 clicked ; 3 disabled; 4 hide

	
	CSUIButton * m_relativeto;
	CRect m_marginToBtn;//相对于另一个按钮的位置

	CSUIButton(LPCTSTR szBmpName , int iAlign, CRect marginTownd , CSUIButton * relativeto = NULL);

	void LoadImage(LPCTSTR szBmpName);

	void PreMultiplyBitmap( CBitmap& bmp );

	void OnSize(CRect WndRect);
};