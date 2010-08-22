#pragma once


// CSVPButton

class CSVPButton : public CButton
{
	DECLARE_DYNAMIC(CSVPButton)
public:
	
	CSVPButton();
	virtual ~CSVPButton();
	DWORD m_textColor,m_btnBgColor, m_pushedColor, m_borderColor, m_bgColor, m_textGrayColor;
	int m_btnMode;

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


