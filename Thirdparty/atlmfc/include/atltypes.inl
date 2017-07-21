// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __ATLTYPES_INL__
#define __ATLTYPES_INL__

#pragma once

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

#ifndef __ATLTYPES_H__
	#error atltypes.inl requires atltypes.h to be included first
#endif	// __ATLTYPES_H__

// CSize
ATLTYPES_INLINE CSize::CSize() throw()
	{ /* random filled */ }
ATLTYPES_INLINE CSize::CSize(int initCX, int initCY) throw()
	{ cx = initCX; cy = initCY; }
ATLTYPES_INLINE CSize::CSize(SIZE initSize) throw()
	{ *(SIZE*)this = initSize; }
ATLTYPES_INLINE CSize::CSize(POINT initPt) throw()
	{ *(POINT*)this = initPt; }
ATLTYPES_INLINE CSize::CSize(DWORD dwSize) throw()
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}
ATLTYPES_INLINE BOOL CSize::operator==(SIZE size) const throw()
	{ return (cx == size.cx && cy == size.cy); }
ATLTYPES_INLINE BOOL CSize::operator!=(SIZE size) const throw()
	{ return (cx != size.cx || cy != size.cy); }
ATLTYPES_INLINE void CSize::operator+=(SIZE size) throw()
	{ cx += size.cx; cy += size.cy; }
ATLTYPES_INLINE void CSize::operator-=(SIZE size) throw()
	{ cx -= size.cx; cy -= size.cy; }
ATLTYPES_INLINE void CSize::SetSize(int CX, int CY) throw()
	{ cx = CX; cy = CY; }	
ATLTYPES_INLINE CSize CSize::operator+(SIZE size) const throw()
	{ return CSize(cx + size.cx, cy + size.cy); }
ATLTYPES_INLINE CSize CSize::operator-(SIZE size) const throw()
	{ return CSize(cx - size.cx, cy - size.cy); }
ATLTYPES_INLINE CSize CSize::operator-() const throw()
	{ return CSize(-cx, -cy); }
ATLTYPES_INLINE CPoint CSize::operator+(POINT point) const throw()
	{ return CPoint(cx + point.x, cy + point.y); }
ATLTYPES_INLINE CPoint CSize::operator-(POINT point) const throw()
	{ return CPoint(cx - point.x, cy - point.y); }
ATLTYPES_INLINE CRect CSize::operator+(const RECT* lpRect) const throw()
	{ return CRect(lpRect) + *this; }
ATLTYPES_INLINE CRect CSize::operator-(const RECT* lpRect) const throw()
	{ return CRect(lpRect) - *this; }

// CPoint
ATLTYPES_INLINE CPoint::CPoint() throw()
	{ /* random filled */ }
ATLTYPES_INLINE CPoint::CPoint(int initX, int initY) throw()
	{ x = initX; y = initY; }
ATLTYPES_INLINE CPoint::CPoint(POINT initPt) throw()
	{ *(POINT*)this = initPt; }
ATLTYPES_INLINE CPoint::CPoint(SIZE initSize) throw()
	{ *(SIZE*)this = initSize; }
ATLTYPES_INLINE CPoint::CPoint(LPARAM dwPoint) throw()
	{
		x = (short)GET_X_LPARAM(dwPoint);
		y = (short)GET_Y_LPARAM(dwPoint);
	}
ATLTYPES_INLINE void CPoint::Offset(int xOffset, int yOffset) throw()
	{ x += xOffset; y += yOffset; }
ATLTYPES_INLINE void CPoint::Offset(POINT point) throw()
	{ x += point.x; y += point.y; }
ATLTYPES_INLINE void CPoint::Offset(SIZE size) throw()
	{ x += size.cx; y += size.cy; }
ATLTYPES_INLINE void CPoint::SetPoint(int X, int Y) throw()
	{ x = X; y = Y; }
ATLTYPES_INLINE BOOL CPoint::operator==(POINT point) const throw()
	{ return (x == point.x && y == point.y); }
ATLTYPES_INLINE BOOL CPoint::operator!=(POINT point) const throw()
	{ return (x != point.x || y != point.y); }
ATLTYPES_INLINE void CPoint::operator+=(SIZE size) throw()
	{ x += size.cx; y += size.cy; }
ATLTYPES_INLINE void CPoint::operator-=(SIZE size) throw()
	{ x -= size.cx; y -= size.cy; }
ATLTYPES_INLINE void CPoint::operator+=(POINT point) throw()
	{ x += point.x; y += point.y; }
ATLTYPES_INLINE void CPoint::operator-=(POINT point) throw()
	{ x -= point.x; y -= point.y; }
ATLTYPES_INLINE CPoint CPoint::operator+(SIZE size) const throw()
	{ return CPoint(x + size.cx, y + size.cy); }
ATLTYPES_INLINE CPoint CPoint::operator-(SIZE size) const throw()
	{ return CPoint(x - size.cx, y - size.cy); }
ATLTYPES_INLINE CPoint CPoint::operator-() const throw()
	{ return CPoint(-x, -y); }
ATLTYPES_INLINE CPoint CPoint::operator+(POINT point) const throw()
	{ return CPoint(x + point.x, y + point.y); }
ATLTYPES_INLINE CSize CPoint::operator-(POINT point) const throw()
	{ return CSize(x - point.x, y - point.y); }
ATLTYPES_INLINE CRect CPoint::operator+(const RECT* lpRect) const throw()
	{ return CRect(lpRect) + *this; }
ATLTYPES_INLINE CRect CPoint::operator-(const RECT* lpRect) const throw()
	{ return CRect(lpRect) - *this; }

// CRect
ATLTYPES_INLINE CRect::CRect() throw()
	{ /* random filled */ }
ATLTYPES_INLINE CRect::CRect(int l, int t, int r, int b) throw()
	{ left = l; top = t; right = r; bottom = b; }
ATLTYPES_INLINE CRect::CRect(const RECT& srcRect) throw()
	{ ::CopyRect(this, &srcRect); }
ATLTYPES_INLINE CRect::CRect(LPCRECT lpSrcRect) throw()
	{ ::CopyRect(this, lpSrcRect); }
ATLTYPES_INLINE CRect::CRect(POINT point, SIZE size) throw()
	{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }
ATLTYPES_INLINE CRect::CRect(POINT topLeft, POINT bottomRight) throw()
	{ left = topLeft.x; top = topLeft.y;
		right = bottomRight.x; bottom = bottomRight.y; }
ATLTYPES_INLINE int CRect::Width() const throw()
	{ return right - left; }
ATLTYPES_INLINE int CRect::Height() const throw()
	{ return bottom - top; }
ATLTYPES_INLINE CSize CRect::Size() const throw()
	{ return CSize(right - left, bottom - top); }
ATLTYPES_INLINE CPoint& CRect::TopLeft() throw()
	{ return *((CPoint*)this); }
ATLTYPES_INLINE CPoint& CRect::BottomRight() throw()
	{ return *((CPoint*)this+1); }
ATLTYPES_INLINE const CPoint& CRect::TopLeft() const throw()
	{ return *((CPoint*)this); }
ATLTYPES_INLINE const CPoint& CRect::BottomRight() const throw()
	{ return *((CPoint*)this+1); }
ATLTYPES_INLINE CPoint CRect::CenterPoint() const throw()
	{ return CPoint((left+right)/2, (top+bottom)/2); }
ATLTYPES_INLINE void CRect::SwapLeftRight() throw()
	{ SwapLeftRight(LPRECT(this)); }
ATLTYPES_INLINE void WINAPI CRect::SwapLeftRight(LPRECT lpRect) throw()
	{ LONG temp = lpRect->left; lpRect->left = lpRect->right; lpRect->right = temp; }
ATLTYPES_INLINE CRect::operator LPRECT() throw()
	{ return this; }
ATLTYPES_INLINE CRect::operator LPCRECT() const throw()
	{ return this; }
ATLTYPES_INLINE BOOL CRect::IsRectEmpty() const throw()
	{ return ::IsRectEmpty(this); }
ATLTYPES_INLINE BOOL CRect::IsRectNull() const throw()
	{ return (left == 0 && right == 0 && top == 0 && bottom == 0); }
ATLTYPES_INLINE BOOL CRect::PtInRect(POINT point) const throw()
	{ return ::PtInRect(this, point); }
ATLTYPES_INLINE void CRect::SetRect(int x1, int y1, int x2, int y2) throw()
	{ ::SetRect(this, x1, y1, x2, y2); }
ATLTYPES_INLINE void CRect::SetRect(POINT topLeft, POINT bottomRight) throw()
	{ ::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }
ATLTYPES_INLINE void CRect::SetRectEmpty() throw()
	{ ::SetRectEmpty(this); }
ATLTYPES_INLINE void CRect::CopyRect(LPCRECT lpSrcRect) throw()
	{ ::CopyRect(this, lpSrcRect); }
ATLTYPES_INLINE BOOL CRect::EqualRect(LPCRECT lpRect) const throw()
	{ return ::EqualRect(this, lpRect); }
ATLTYPES_INLINE void CRect::InflateRect(int x, int y) throw()
	{ ::InflateRect(this, x, y); }
ATLTYPES_INLINE void CRect::InflateRect(SIZE size) throw()
	{ ::InflateRect(this, size.cx, size.cy); }
ATLTYPES_INLINE void CRect::DeflateRect(int x, int y) throw()
	{ ::InflateRect(this, -x, -y); }
ATLTYPES_INLINE void CRect::DeflateRect(SIZE size) throw()
	{ ::InflateRect(this, -size.cx, -size.cy); }
ATLTYPES_INLINE void CRect::OffsetRect(int x, int y) throw()
	{ ::OffsetRect(this, x, y); }
ATLTYPES_INLINE void CRect::OffsetRect(POINT point) throw()
	{ ::OffsetRect(this, point.x, point.y); }
ATLTYPES_INLINE void CRect::OffsetRect(SIZE size) throw()
	{ ::OffsetRect(this, size.cx, size.cy); }
ATLTYPES_INLINE void CRect::MoveToY(int y) throw()
	{ bottom = Height() + y; top = y; }
ATLTYPES_INLINE void CRect::MoveToX(int x) throw()
	{ right = Width() + x; left = x; }
ATLTYPES_INLINE void CRect::MoveToXY(int x, int y) throw()
	{ MoveToX(x); MoveToY(y); }
ATLTYPES_INLINE void CRect::MoveToXY(POINT pt) throw()
	{ MoveToX(pt.x); MoveToY(pt.y); }
ATLTYPES_INLINE BOOL CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2) throw()
	{ return ::IntersectRect(this, lpRect1, lpRect2);}
ATLTYPES_INLINE BOOL CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2) throw()
	{ return ::UnionRect(this, lpRect1, lpRect2); }
ATLTYPES_INLINE void CRect::operator=(const RECT& srcRect) throw()
	{ ::CopyRect(this, &srcRect); }
ATLTYPES_INLINE BOOL CRect::operator==(const RECT& rect) const throw()
	{ return ::EqualRect(this, &rect); }
ATLTYPES_INLINE BOOL CRect::operator!=(const RECT& rect) const throw()
	{ return !::EqualRect(this, &rect); }
ATLTYPES_INLINE void CRect::operator+=(POINT point) throw()
	{ ::OffsetRect(this, point.x, point.y); }
ATLTYPES_INLINE void CRect::operator+=(SIZE size) throw()
	{ ::OffsetRect(this, size.cx, size.cy); }
ATLTYPES_INLINE void CRect::operator+=(LPCRECT lpRect) throw()
	{ InflateRect(lpRect); }
ATLTYPES_INLINE void CRect::operator-=(POINT point) throw()
	{ ::OffsetRect(this, -point.x, -point.y); }
ATLTYPES_INLINE void CRect::operator-=(SIZE size) throw()
	{ ::OffsetRect(this, -size.cx, -size.cy); }
ATLTYPES_INLINE void CRect::operator-=(LPCRECT lpRect) throw()
	{ DeflateRect(lpRect); }
ATLTYPES_INLINE void CRect::operator&=(const RECT& rect) throw()
	{ ::IntersectRect(this, this, &rect); }
ATLTYPES_INLINE void CRect::operator|=(const RECT& rect) throw()
	{ ::UnionRect(this, this, &rect); }
ATLTYPES_INLINE CRect CRect::operator+(POINT pt) const throw()
	{ CRect rect(*this); ::OffsetRect(&rect, pt.x, pt.y); return rect; }
ATLTYPES_INLINE CRect CRect::operator-(POINT pt) const throw()
	{ CRect rect(*this); ::OffsetRect(&rect, -pt.x, -pt.y); return rect; }
ATLTYPES_INLINE CRect CRect::operator+(SIZE size) const throw()
	{ CRect rect(*this); ::OffsetRect(&rect, size.cx, size.cy); return rect; }
ATLTYPES_INLINE CRect CRect::operator-(SIZE size) const throw()
	{ CRect rect(*this); ::OffsetRect(&rect, -size.cx, -size.cy); return rect; }
ATLTYPES_INLINE CRect CRect::operator+(LPCRECT lpRect) const throw()
	{ CRect rect(this); rect.InflateRect(lpRect); return rect; }
ATLTYPES_INLINE CRect CRect::operator-(LPCRECT lpRect) const throw()
	{ CRect rect(this); rect.DeflateRect(lpRect); return rect; }
ATLTYPES_INLINE CRect CRect::operator&(const RECT& rect2) const throw()
	{ CRect rect; ::IntersectRect(&rect, this, &rect2);
		return rect; }
ATLTYPES_INLINE CRect CRect::operator|(const RECT& rect2) const throw()
	{ CRect rect; ::UnionRect(&rect, this, &rect2);
		return rect; }
ATLTYPES_INLINE BOOL CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2) throw()
	{ return ::SubtractRect(this, lpRectSrc1, lpRectSrc2); }

ATLTYPES_INLINE void CRect::NormalizeRect() throw()
	{
		int nTemp;
		if (left > right)
		{
			nTemp = left;
			left = right;
			right = nTemp;
		}
		if (top > bottom)
		{
			nTemp = top;
			top = bottom;
			bottom = nTemp;
		}
	}

ATLTYPES_INLINE void CRect::InflateRect(LPCRECT lpRect) throw()
	{
		left -= lpRect->left;		top -= lpRect->top;
		right += lpRect->right;		bottom += lpRect->bottom;
	}

ATLTYPES_INLINE void CRect::InflateRect(int l, int t, int r, int b) throw()
	{
		left -= l;			top -= t;
		right += r;			bottom += b;
	}

ATLTYPES_INLINE void CRect::DeflateRect(LPCRECT lpRect) throw()
	{
		left += lpRect->left;	top += lpRect->top;
		right -= lpRect->right;	bottom -= lpRect->bottom;
	}

ATLTYPES_INLINE void CRect::DeflateRect(int l, int t, int r, int b) throw()
	{
		left += l;		top += t;
		right -= r;		bottom -= b;
	}

ATLTYPES_INLINE CRect CRect::MulDiv(int nMultiplier, int nDivisor) const throw()
	{
		return CRect(
			::MulDiv(left, nMultiplier, nDivisor),
			::MulDiv(top, nMultiplier, nDivisor),
			::MulDiv(right, nMultiplier, nDivisor),
			::MulDiv(bottom, nMultiplier, nDivisor));
	}


#endif	// __ATLTYPES_INL__
