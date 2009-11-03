#pragma once


// CSVPStatic

class CSVPStatic : public CStatic
{
	DECLARE_DYNAMIC(CSVPStatic)

public:
	CSVPStatic();
	virtual ~CSVPStatic();
	DWORD m_dwAlign;
	DWORD m_bgColor, m_textColor;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	virtual void PreSubclassWindow();
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


