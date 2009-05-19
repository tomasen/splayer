
#pragma once

#include "stdafx.h"

#include <atlbase.h>


class CMemoryDC : public CDC
{
public:
	// Data members
	CDC* m_hDCOriginal;
	RECT m_rcPaint;
	CBitmap m_bmp;
	HBITMAP m_hBmpOld;

	// Constructor/destructor
	CMemoryDC(CDC* hDC, RECT& rcPaint) : m_hDCOriginal(hDC), m_hBmpOld(NULL)
	{
		m_rcPaint = rcPaint;
		CreateCompatibleDC(m_hDCOriginal);
		ATLASSERT(m_hDC != NULL);
		m_bmp.CreateCompatibleBitmap( m_hDCOriginal, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top);
		ATLASSERT( (HBITMAP)m_bmp != NULL);
		m_hBmpOld = (HBITMAP)SelectObject(m_bmp);
		SetViewportOrg(-m_rcPaint.left, -m_rcPaint.top);
	}

	~CMemoryDC()
	{
		m_hDCOriginal->BitBlt( m_rcPaint.left, m_rcPaint.top, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top, CDC::FromHandle(m_hDC), m_rcPaint.left, m_rcPaint.top, SRCCOPY);
		SelectObject(m_hBmpOld);
	}
};

class CBtnAlign{
public:
	CBtnAlign(UINT ia, INT_PTR pb){
		iAlign = ia;
		bBtn = pb;
	}
	UINT iAlign;
	INT_PTR bBtn;
};

#define ALIGN_TOPLEFT 1
#define ALIGN_TOPRIGHT 2
#define ALIGN_BOTTOMLEFT 3
#define ALIGN_BOTTOMRIGHT 4

#define ALIGN_TOP 1
#define ALIGN_LEFT 2
#define ALIGN_RIGHT 4
#define ALIGN_BOTTOM 8

#define DEFAULT_MARGIN 3

#include "libpng.h"

class CSUIButton {

public:
	CSize m_btnSize; //按钮的大小尺寸
	int m_iAlign; //与窗口对齐方向
	CRect m_marginTownd; //按钮相对于窗口的位置， 0为不强制 负数为百分比
	CRect m_rcHitest; //按钮的有效范围，用于hittest
	CBitmap m_bitmap; //图片存储位置
	int m_stat; //0 normal ; 1 hove ; 2 clicked ; 3 disabled; 4 hide
	UINT m_htMsgID;
	BOOL m_hide;
	CRect m_marginToBtn;//相对于另一个按钮的位置
	CString m_szBmpName;

	CList<CBtnAlign*> btnAlignList;

	CSUIButton(LPCTSTR szBmpName,  int iAlign, CRect marginTownd 
		, BOOL bNotButton = false, UINT htMsgID = NULL , BOOL bHide = FALSE, 
		UINT alignToButton = 0 , CSUIButton * relativeToButton = 0);
	CSUIButton(UINT Imgid,  int iAlign, CRect marginTownd 
		, BOOL bNotButton = false, UINT htMsgID = NULL , BOOL bHide = FALSE, 
		UINT alignToButton = 0 , CSUIButton * relativeToButton = 0);

	CPngImage m_png;

	
	void LoadImage(LPCTSTR szBmpName);
	void Attach(HBITMAP bmp);

	void addAlignRelButton(UINT alignToButton  , CSUIButton * relativeToButton );

	void OnSize(CRect WndRect);

	void OnPaint(CMemoryDC *hDC, CRect rc);
	
	BOOL OnHitTest(CPoint pt , BOOL bLBtnDown);

	BOOL m_NotButton;

private:
	void PreMultiplyBitmap( CBitmap& bmp );
	LONG CalcRealMargin(LONG Mlen, LONG bW, LONG wW);
	
};



class CSUIBtnList : public CList<CSUIButton*>
{

public:
	CSUIBtnList();
	virtual ~CSUIBtnList();
	BOOL HTRedrawRequired ;

	void PaintAll(CMemoryDC *hDC, CRect rc);
	
	void OnSize(CRect WndRect);

	UINT OnHitTest(CPoint pt , CRect rc);

	void SetHideStat(POSITION pos, BOOL bHide); //By Position
	void SetHideStat(UINT iMsgID, BOOL bHide); //By HT MSG ID
	void SetHideStat(LPCTSTR szBmpName, BOOL bHide); //By BitMapName

	void ClearStat();
};

